/* NekoEngine
 *
 * DeferredBuffer.cpp
 * Author: Alexandru Naiman
 *
 * Deffered rendering buffer
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2016, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Engine/Console.h>
#include <Engine/SceneManager.h>
#include <Engine/CameraManager.h>
#include <Engine/DeferredBuffer.h>
#include <Engine/ResourceManager.h>
#include <Scene/Components/StaticMeshComponent.h>

#define DR_MODULE	"DeferredBuffer"

using namespace glm;
using namespace std;

RFramebuffer* DeferredBuffer::_fbos[4]{ 0, 0, 0, 0 };
RTexture* DeferredBuffer::_gbTextures[8]{ 0, 0, 0, 0, 0, 0 };
uint64_t DeferredBuffer::_gbTexHandles[7]{ 0, 0, 0, 0, 0, 0, 0 };
uint32_t DeferredBuffer::_fboWidth = 1280;
uint32_t DeferredBuffer::_fboHeight = 720;
Shader* DeferredBuffer::_geometryShader = nullptr;
Shader* DeferredBuffer::_lightingShader = nullptr;
SSAO* DeferredBuffer::_ssao = nullptr;
HBAO* DeferredBuffer::_hbao = nullptr;
int DeferredBuffer::_samples = 8;
Object* DeferredBuffer::_lightSphere = nullptr;
ShadowMap* DeferredBuffer::_shadow = nullptr;
RBuffer* DeferredBuffer::_sceneLightUbo = nullptr;
RBuffer* DeferredBuffer::_lightUbo = nullptr;
RBuffer* DeferredBuffer::_lightMatrixUbo = nullptr;

static bool _enableHBAO = false, _enableSSAO = false;
NString _drCVar_GetHBAO() { return _enableHBAO ? "true" : "false"; }
void _drCVar_SetHBAO(NString str) { _enableHBAO = (bool)str; DeferredBuffer::EnableAO(_enableHBAO); }
NString _drCVar_GetSSAO() { return _enableSSAO ? "true" : "false"; }
void _drCVar_SetSSAO(NString str) { _enableSSAO = (bool)str; DeferredBuffer::EnableAO(_enableSSAO); }

static inline bool _initSSAO(SSAO **ssao, int width, int height)
{
	if ((*ssao = new SSAO(width, height)) == nullptr)
		return false;

	/*if (!(SSAO*)(*ssao)->Initialize())
	{
		Logger::Log(DR_MODULE, LOG_WARNING, "Failed to initialize SSAO.");
		delete *ssao;
		*ssao = nullptr;
		return false;
	}*/

	return true;
}

static inline bool _initHBAO(HBAO **hbao, int width, int height)
{
	if ((*hbao = new HBAO(width, height)) == nullptr)
		return false;

	if (!(HBAO*)(*hbao)->Initialize())
	{
		Logger::Log(DR_MODULE, LOG_WARNING, "Failed to initialize HBAO+.");
		delete *hbao;
		*hbao = nullptr;
		return false;
	}

	return true;
}

int DeferredBuffer::Initialize() noexcept
{
	_geometryShader = (Shader*)ResourceManager::GetResourceByName("sh_geometry", ResourceType::RES_SHADER);
	_lightingShader = (Shader*)ResourceManager::GetResourceByName("sh_lighting", ResourceType::RES_SHADER);

	if(_geometryShader == nullptr || _lightingShader == nullptr)
		return ENGINE_FAIL;

	_fboWidth = Engine::GetScreenWidth();
	_fboHeight = Engine::GetScreenHeight();

	if (Engine::GetConfiguration().Renderer.Supersampling)
	{
		_fboWidth *= 2;
		_fboHeight *= 2;
	}
	
	if((_fbos[GB_FBO_GEOMETRY] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if((_fbos[GB_FBO_LIGHT] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if((_fbos[GB_FBO_BRIGHT] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if((_fbos[GB_FBO_LIGHT_ACCUM] = Engine::GetRenderer()->CreateFramebuffer(_fboWidth, _fboHeight)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	
	if (Engine::GetConfiguration().Renderer.Multisampling)
	{
		_samples = Engine::GetConfiguration().Renderer.Samples;
		int maxSamples = Engine::GetRenderer()->GetMaxSamples();

		if (maxSamples < _samples)
		{
			Logger::Log(DR_MODULE, LOG_WARNING, "%d samples requested, but the maximum supported is %d.", _samples, maxSamples);
			_samples = maxSamples;
		}
	}
	else
		_samples = 1;

	if(!_GenerateTextures())
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}

	if (!_AttachTextures())
	{
		Release();
		return ENGINE_FAIL;
	}
	
	DrawAttachment drawAttachments[4] { DrawAttachment::Color0, DrawAttachment::Color1, DrawAttachment::Color2, DrawAttachment::Color3 };
	_fbos[GB_FBO_GEOMETRY]->SetDrawBuffers(4, drawAttachments);

	_enableHBAO = Engine::GetConfiguration().Renderer.HBAO;
	_enableSSAO = Engine::GetConfiguration().Renderer.SSAO;

	if (_enableHBAO)
		_initHBAO(&_hbao, _fboWidth, _fboHeight);
	
	if (_enableSSAO && !_hbao)
		_initSSAO(&_ssao, _fboWidth, _fboHeight);
		
	ObjectInitializer lsInitializer;

	if((_lightSphere = Engine::NewObject("Object", &lsInitializer)) == nullptr)
	{
		Release();
		return ENGINE_FAIL;
	}

	ComponentInitializer mcInitializer;
	mcInitializer.parent = _lightSphere;
	mcInitializer.arguments.insert(make_pair("mesh", "stm_light_sphere"));

	StaticMeshComponent *meshComponent = (StaticMeshComponent *)Engine::NewComponent("StaticMeshComponent", &mcInitializer);
	if (!meshComponent)
	{
		Release();
		return ENGINE_FAIL;
	}
	meshComponent->Load();
    
    if(meshComponent->GetMesh()->CreateBuffers(false) != ENGINE_OK)
    {
        Release();
        return ENGINE_FAIL;
    }

	_lightSphere->AddComponent("Mesh", meshComponent);
	_lightSphere->SetId(0xB000B5);

	if (_lightSphere->Load() != ENGINE_OK)
	{
		Release();
		return ENGINE_FAIL;
	}
    
	if((_shadow = new ShadowMap(Engine::GetConfiguration().Renderer.ShadowMapSize)) == nullptr)
	{
		Release();
		return ENGINE_FAIL;
	}

	if((_sceneLightUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	_sceneLightUbo->SetStorage(sizeof(LightSceneData), nullptr);

	if((_lightUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	_lightUbo->SetStorage(sizeof(LightData), nullptr);

	if((_lightMatrixUbo = Engine::GetRenderer()->CreateBuffer(BufferType::Uniform, true, false)) == nullptr)
	{
		Release();
		return ENGINE_OUT_OF_RESOURCES;
	}
	_lightMatrixUbo->SetStorage(sizeof(mat4), nullptr);

	RShader *lightShader = _lightingShader->GetRShader();

	lightShader->VSUniformBlockBinding(0, "MatrixBlock");
	lightShader->VSSetUniformBuffer(0, 0, sizeof(mat4), _lightMatrixUbo);

	lightShader->FSUniformBlockBinding(0, "SceneLightData");
	lightShader->FSSetUniformBuffer(0, 0, sizeof(LightSceneData), _sceneLightUbo);

	lightShader->FSUniformBlockBinding(1, "LightData");
	lightShader->FSSetUniformBuffer(1, 0, sizeof(LightData), _lightUbo);

	vec3 data;
	data.x = (float)_fboWidth;
	data.y = (float)_fboHeight;
	data.z = _ssao || _hbao ? 1.f : 0.f;

	_sceneLightUbo->UpdateData(sizeof(vec4) * 3, sizeof(vec3), &data.x);

	RShader *geomShader = _geometryShader->GetRShader();
	
	geomShader->VSUniformBlockBinding(0, "MatrixBlock");
	geomShader->VSUniformBlockBinding(1, "MaterialBlock");
	geomShader->VSUniformBlockBinding(2, "BoneBlock");
	
	geomShader->FSUniformBlockBinding(0, "ObjectBlock");
	geomShader->FSUniformBlockBinding(1, "MaterialBlock");

	_RegisterCVars();

	return ENGINE_OK;
}

void DeferredBuffer::SetAmbientColor(vec3 &color, float intensity) noexcept
{
	_sceneLightUbo->UpdateData(sizeof(vec3), sizeof(float), &intensity);
	_sceneLightUbo->UpdateData(sizeof(vec4), sizeof(vec3), &color.x);
}

void DeferredBuffer::SetFogColor(vec3 &color) noexcept
{
	_sceneLightUbo->UpdateData(sizeof(vec4) * 2, sizeof(vec3), &color.x);
}

void DeferredBuffer::SetFogProperties(float clear, float start) noexcept
{
	_sceneLightUbo->UpdateData(sizeof(vec4) + sizeof(vec3), sizeof(float), &clear);
	_sceneLightUbo->UpdateData(sizeof(vec4) * 2 + sizeof(vec3), sizeof(float), &start);
}

void DeferredBuffer::BindGeometry() noexcept
{
	_Bind();
	_geometryShader->Enable();
	
	Engine::GetRenderer()->SetViewport(0, 0, _fboWidth, _fboHeight);
	Engine::GetRenderer()->SetMinSampleShading(_samples);
	Engine::GetRenderer()->EnableBlend(false);
}

void DeferredBuffer::BindLighting() noexcept
{
	_fbos[GB_FBO_LIGHT_ACCUM]->Bind(FB_DRAW);
	Engine::GetRenderer()->SetMinSampleShading(_samples);
}

void DeferredBuffer::Unbind() noexcept
{
	Engine::GetRenderer()->BindDefaultFramebuffer();
	_geometryShader->Disable();
}

void DeferredBuffer::RenderLighting() noexcept
{
	Renderer* r = Engine::GetRenderer();
	Scene *s = SceneManager::GetActiveScene();
	Camera *cam = CameraManager::GetActiveCamera();
	RShader *lightShader = _lightingShader->GetRShader();

	if (_enableHBAO && _hbao)
	{
		mat4 worldToView = inverse(cam->GetView());
		_hbao->SetProjection(value_ptr(cam->GetProjectionMatrix()));
		_hbao->SetWorldToView(value_ptr(worldToView));
		_hbao->SetViewport(0, 0, _fboWidth, _fboHeight, cam->GetNear(), cam->GetFar());
		_hbao->Render();
	}
	else if (_enableSSAO && _ssao)
		_ssao->Render();
	
	DrawAttachment drawAttachments[2] { DrawAttachment::Color0, DrawAttachment::Color1 };
	_fbos[GB_FBO_LIGHT]->SetDrawBuffers(2, drawAttachments);

	r->Clear(R_CLEAR_COLOR);

	lightShader->Enable();
	lightShader->BindUniformBuffers();

	lightShader->SetTexture(U_TEXTURE0, _gbTextures[GB_TEX_POSITION]);
	lightShader->SetTexture(U_TEXTURE1, _gbTextures[GB_TEX_NORMAL]);
	lightShader->SetTexture(U_TEXTURE2, _gbTextures[GB_TEX_COLOR_SPECULAR]);
	lightShader->SetTexture(U_TEXTURE3, _gbTextures[GB_TEX_MATERIAL_INFO]);	
	lightShader->SetTexture(U_TEXTURE5, _gbTextures[GB_TEX_LIGHT_ACCUM]);

	if (_enableHBAO && _hbao)
		lightShader->SetTexture(U_TEXTURE4, _hbao->GetTexture());
	else if (_enableSSAO && _ssao)
		lightShader->SetTexture(U_TEXTURE4, _ssao->GetTexture());

	if (_shadow)
		lightShader->SetTexture(U_TEXTURE6, _shadow->GetTexture());

	lightShader->SetTexture(U_TEXTURE7, _gbTextures[GB_TEX_DEPTH_STENCIL]);

	_sceneLightUbo->UpdateData(0, sizeof(float) * 3, &cam->GetPosition().x);
	_lightMatrixUbo->UpdateData(0, sizeof(mat4), (void *)value_ptr(mat4()));

	for (size_t i = 0; i < s->GetNumLights(); i++)
	{
		Light *l = s->GetLight(i);
		LightData data;

		data.LightPosition = l->GetPosition();
		data.LightColor = l->GetColor();
		data.LightAttenuationAndData = vec4(l->GetAttenuation(), l->GetIntensity(), (float)l->GetType());

		if (_shadow && l->CastShadows())
		{
			_shadow->Render(l);
			
			lightShader->Enable();
			lightShader->BindUniformBuffers();
			BindLighting();

			data.LightDirectionAndShadow = vec4(l->GetDirection(), 1.f);
			data.CameraToLight = ((_shadow->GetProjection() * _shadow->GetView()) * _shadow->GetModel()) * inverse((cam->GetProjectionMatrix() * cam->GetView()) * cam->GetModel());
			_lightUbo->UpdateData(0, sizeof(LightData), &data);
		}
		else
		{
			data.LightDirectionAndShadow = vec4(l->GetDirection(), 0.f);
			_lightUbo->UpdateData(0, sizeof(LightData) - sizeof(mat4), &data);
		}
		
		uint32_t subroutine = (uint32_t)l->GetType();
		lightShader->SetSubroutines(ShaderType::Fragment, 1, &subroutine);

		if (l->GetType() == LightType::Point)
		{
			_fbos[GB_FBO_LIGHT_ACCUM]->SetDrawBuffer(DrawAttachment::None);

			r->EnableFaceCulling(false);
			r->EnableStencilTest(true);
			r->SetStencilMask(1);

			r->Clear(R_CLEAR_STENCIL);

			r->SetStencilFunc(TestFunc::Always, 0, 0);
			r->SetStencilOpSeparate(PolygonFace::Back, TestOp::Keep, TestOp::IncrementWrap, TestOp::Keep);
			r->SetStencilOpSeparate(PolygonFace::Front, TestOp::Keep, TestOp::DecrementWrap, TestOp::Keep);

			_lightSphere->SetPosition(l->GetPosition());
			vec3 scale(l->GetAttenuation().y);
			_lightSphere->SetScale(scale);
			mat4 mvpMatrix = (cam->GetProjectionMatrix() * cam->GetView()) * _lightSphere->GetModelMatrix();
			_lightMatrixUbo->UpdateData(0, sizeof(mat4), (void *)value_ptr(mvpMatrix));

			_lightSphere->Update(0);
			_lightSphere->Draw(lightShader, cam);

			_fbos[GB_FBO_LIGHT_ACCUM]->SetDrawBuffers(1, drawAttachments);

			r->SetStencilMask(0);
			r->SetStencilFunc(TestFunc::NotEqual, 0, 0xFF);
			r->EnableFaceCulling(true);
		}
		
		r->EnableBlend(true);
		r->SetBlendEquation(BlendEquation::Add);
		r->SetBlendFunc(BlendFactor::One, BlendFactor::One);

		Engine::BindQuadVAO();
		_lightMatrixUbo->UpdateData(0, sizeof(mat4), (void *)value_ptr(mat4()));
		r->DrawArrays(PolygonMode::TriangleStrip, 0, 4);

		r->EnableBlend(false);
		r->EnableStencilTest(false);
		r->EnableDepthTest(true);
	}

	_fbos[GB_FBO_LIGHT]->Bind(FB_DRAW);

	r->SetMinSampleShading(_samples);

	uint32_t subroutine = LT_AMBIENTAL;
	lightShader->SetSubroutines(ShaderType::Fragment, 1, &subroutine);

	int v = LT_AMBIENTAL;
	_lightUbo->UpdateData(sizeof(LightData) - sizeof(mat4) - sizeof(int), sizeof(int), &v);

	Engine::BindQuadVAO();
	_lightMatrixUbo->UpdateData(0, sizeof(mat4), (void *)value_ptr(mat4()));
	r->DrawArrays(PolygonMode::TriangleStrip, 0, 4);

	_lightingShader->Disable();

	_fbos[GB_FBO_LIGHT]->SetDrawBuffers(1, drawAttachments);
}

void DeferredBuffer::ScreenResized(int width, int height) noexcept
{
	_fboWidth = width;
	_fboHeight = height;

	if (Engine::GetConfiguration().Renderer.Supersampling)
	{
		_fboWidth *= 2;
		_fboHeight *= 2;
	}

	_fbos[GB_FBO_GEOMETRY]->Resize(_fboWidth, _fboHeight);
	_fbos[GB_FBO_LIGHT]->Resize(_fboWidth, _fboHeight);
	_fbos[GB_FBO_BRIGHT]->Resize(_fboWidth, _fboHeight);
	_fbos[GB_FBO_LIGHT_ACCUM]->Resize(_fboWidth, _fboHeight);
		
	if (_hbao)
		_hbao->Resize(_fboWidth, _fboHeight);

	if (_ssao)
		_ssao->Resize(_fboWidth, _fboHeight);

	vec2 data;
	data.x = (float)_fboWidth;
	data.y = (float)_fboHeight;

	_sceneLightUbo->BeginUpdate();
	_sceneLightUbo->UpdateData(sizeof(vec4) * 3, sizeof(vec2), &data.x);
	_sceneLightUbo->EndUpdate();
}

void DeferredBuffer::_Bind() noexcept
{
	_fbos[GB_FBO_GEOMETRY]->Bind(FB_DRAW);
}

bool DeferredBuffer::_GenerateTextures() noexcept
{
	TextureSizedFormat internalFormat = TextureSizedFormat::RGBA_32F;

	if (Engine::GetConfiguration().Renderer.Quality == RENDER_QUALITY_LOW)
		internalFormat = TextureSizedFormat::RGBA_16F;

	Renderer *r = Engine::GetRenderer();

	if (r->HasCapability(RendererCapability::MultisampledFramebuffer) && Engine::GetConfiguration().Renderer.Multisampling)
	{
		_gbTextures[GB_TEX_POSITION] = r->CreateTexture(TextureType::Tex2DMultisample);
		if(!_gbTextures[GB_TEX_POSITION])
			return false;
		_gbTextures[GB_TEX_POSITION]->SetStorage2DMS(_samples, _fboWidth, _fboHeight, internalFormat, true);

		_gbTextures[GB_TEX_NORMAL] = r->CreateTexture(TextureType::Tex2DMultisample);
		if(!_gbTextures[GB_TEX_NORMAL])
			return false;
		_gbTextures[GB_TEX_NORMAL]->SetStorage2DMS(_samples, _fboWidth, _fboHeight, internalFormat, true);

		_gbTextures[GB_TEX_COLOR_SPECULAR] = r->CreateTexture(TextureType::Tex2DMultisample);
		if(!_gbTextures[GB_TEX_COLOR_SPECULAR])
			return false;
		_gbTextures[GB_TEX_COLOR_SPECULAR]->SetStorage2DMS(_samples, _fboWidth, _fboHeight, TextureSizedFormat::RGBA_16F, true);

		_gbTextures[GB_TEX_MATERIAL_INFO] = r->CreateTexture(TextureType::Tex2DMultisample);
		if(!_gbTextures[GB_TEX_MATERIAL_INFO])
			return false;
		_gbTextures[GB_TEX_MATERIAL_INFO]->SetStorage2DMS(_samples, _fboWidth, _fboHeight, TextureSizedFormat::RGBA_16F, true);

		_gbTextures[GB_TEX_DEPTH_STENCIL] = r->CreateTexture(TextureType::Tex2DMultisample);
		if(!_gbTextures[GB_TEX_DEPTH_STENCIL])
			return false;
		_gbTextures[GB_TEX_DEPTH_STENCIL]->SetStorage2DMS(_samples, _fboWidth, _fboHeight, TextureSizedFormat::DEPTH24_STENCIL8, true);

		_gbTextures[GB_TEX_LIGHT] = r->CreateTexture(TextureType::Tex2DMultisample);
		if(!_gbTextures[GB_TEX_LIGHT])
			return false;
		_gbTextures[GB_TEX_LIGHT]->SetStorage2DMS(_samples, _fboWidth, _fboHeight, TextureSizedFormat::RGB_16F, true);

		_gbTextures[GB_TEX_LIGHT_ACCUM] = r->CreateTexture(TextureType::Tex2DMultisample);
		if(!_gbTextures[GB_TEX_LIGHT_ACCUM])
			return false;
		_gbTextures[GB_TEX_LIGHT_ACCUM]->SetStorage2DMS(_samples, _fboWidth, _fboHeight, TextureSizedFormat::RGB_16F, true);

		_gbTextures[GB_TEX_BRIGHT] = r->CreateTexture(TextureType::Tex2DMultisample);
		if(!_gbTextures[GB_TEX_BRIGHT])
			return false;
		_gbTextures[GB_TEX_BRIGHT]->SetStorage2DMS(_samples, _fboWidth, _fboHeight, TextureSizedFormat::RGB_8U, true);
	}
	else
	{
		_gbTextures[GB_TEX_POSITION] = r->CreateTexture(TextureType::Tex2D);
		if(!_gbTextures[GB_TEX_POSITION])
			return false;
		_gbTextures[GB_TEX_POSITION]->SetStorage2D(1, internalFormat, _fboWidth, _fboHeight);

		_gbTextures[GB_TEX_NORMAL] = r->CreateTexture(TextureType::Tex2D);
		if(!_gbTextures[GB_TEX_NORMAL])
			return false;
		_gbTextures[GB_TEX_NORMAL]->SetStorage2D(1, internalFormat, _fboWidth, _fboHeight);

		_gbTextures[GB_TEX_COLOR_SPECULAR] = r->CreateTexture(TextureType::Tex2D);
		if(!_gbTextures[GB_TEX_COLOR_SPECULAR])
			return false;
		_gbTextures[GB_TEX_COLOR_SPECULAR]->SetStorage2D(1, TextureSizedFormat::RGBA_16F, _fboWidth, _fboHeight);

		_gbTextures[GB_TEX_MATERIAL_INFO] = r->CreateTexture(TextureType::Tex2D);
		if(!_gbTextures[GB_TEX_MATERIAL_INFO])
			return false;
		_gbTextures[GB_TEX_MATERIAL_INFO]->SetStorage2D(1, TextureSizedFormat::RGBA_16F, _fboWidth, _fboHeight);

		_gbTextures[GB_TEX_DEPTH_STENCIL] = r->CreateTexture(TextureType::Tex2D);
		if(!_gbTextures[GB_TEX_DEPTH_STENCIL])
			return false;
		_gbTextures[GB_TEX_DEPTH_STENCIL]->SetStorage2D(1, TextureSizedFormat::DEPTH24_STENCIL8, _fboWidth, _fboHeight);

		_gbTextures[GB_TEX_LIGHT] = r->CreateTexture(TextureType::Tex2D);
		if(!_gbTextures[GB_TEX_LIGHT])
			return false;
		_gbTextures[GB_TEX_LIGHT]->SetStorage2D(1, TextureSizedFormat::RGB_16F, _fboWidth, _fboHeight);

		_gbTextures[GB_TEX_LIGHT_ACCUM] = r->CreateTexture(TextureType::Tex2D);
		if(!_gbTextures[GB_TEX_LIGHT_ACCUM])
			return false;
		_gbTextures[GB_TEX_LIGHT_ACCUM]->SetStorage2D(1, TextureSizedFormat::RGB_16F, _fboWidth, _fboHeight);

		_gbTextures[GB_TEX_BRIGHT] = r->CreateTexture(TextureType::Tex2D);
		if(!_gbTextures[GB_TEX_BRIGHT])
			return false;
		_gbTextures[GB_TEX_BRIGHT]->SetStorage2D(1, TextureSizedFormat::RGB_8U, _fboWidth, _fboHeight);
	}
	
	return true;
}

bool DeferredBuffer::_AttachTextures() noexcept
{
	_fbos[GB_FBO_GEOMETRY]->AttachTexture(DrawAttachment::Color0, _gbTextures[GB_TEX_POSITION]);	
	_fbos[GB_FBO_GEOMETRY]->AttachTexture(DrawAttachment::Color1, _gbTextures[GB_TEX_NORMAL]);
	_fbos[GB_FBO_GEOMETRY]->AttachTexture(DrawAttachment::Color2, _gbTextures[GB_TEX_COLOR_SPECULAR]);
	_fbos[GB_FBO_GEOMETRY]->AttachTexture(DrawAttachment::Color3, _gbTextures[GB_TEX_MATERIAL_INFO]);
	_fbos[GB_FBO_GEOMETRY]->AttachDepthStencilTexture(_gbTextures[GB_TEX_DEPTH_STENCIL]);
	
	if (_fbos[GB_FBO_GEOMETRY]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(DR_MODULE, LOG_CRITICAL, "Failed to create geometry framebuffer");
		return false;
	}

	_fbos[GB_FBO_LIGHT]->AttachTexture(DrawAttachment::Color0, _gbTextures[GB_TEX_LIGHT]);
	_fbos[GB_FBO_LIGHT]->AttachTexture(DrawAttachment::Color1, _gbTextures[GB_TEX_BRIGHT]);
	_fbos[GB_FBO_LIGHT]->AttachDepthStencilTexture(_gbTextures[GB_TEX_DEPTH_STENCIL]);

	if (_fbos[GB_FBO_LIGHT]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(DR_MODULE, LOG_CRITICAL, "Failed to create light framebuffer");
		return false;
	}

	_fbos[GB_FBO_BRIGHT]->AttachTexture(DrawAttachment::Color0, _gbTextures[GB_TEX_BRIGHT]);

	if (_fbos[GB_FBO_BRIGHT]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(DR_MODULE, LOG_CRITICAL, "Failed to create brightness framebuffer");
		return false;
	}

	_fbos[GB_FBO_LIGHT_ACCUM]->AttachTexture(DrawAttachment::Color0, _gbTextures[GB_TEX_LIGHT_ACCUM]);
	_fbos[GB_FBO_LIGHT_ACCUM]->AttachDepthStencilTexture(_gbTextures[GB_TEX_DEPTH_STENCIL]);

	if (_fbos[GB_FBO_LIGHT_ACCUM]->CheckStatus() != FramebufferStatus::Complete)
	{
		Logger::Log(DR_MODULE, LOG_CRITICAL, "Failed to create light acummulation framebuffer");
		return false;
	}

	return true;
}

void DeferredBuffer::_DeleteTextures() noexcept
{
	delete _gbTextures[GB_TEX_POSITION];
	delete _gbTextures[GB_TEX_NORMAL];
	delete _gbTextures[GB_TEX_COLOR_SPECULAR];
	delete _gbTextures[GB_TEX_MATERIAL_INFO];
	delete _gbTextures[GB_TEX_DEPTH_STENCIL];
	delete _gbTextures[GB_TEX_LIGHT_ACCUM];
	delete _gbTextures[GB_TEX_BRIGHT];
	delete _gbTextures[GB_TEX_LIGHT];
}

void DeferredBuffer::CopyLight(RFramebuffer* destFbo) noexcept
{
	_fbos[GB_FBO_LIGHT]->Blit(destFbo, 0, 0, _fboWidth, _fboHeight, 0, 0, _fboWidth, _fboHeight);
}

void DeferredBuffer::CopyColor(RFramebuffer* destFbo) noexcept
{
	_fbos[GB_FBO_LIGHT]->CopyColor(destFbo, TextureFilter::Linear);
}

void DeferredBuffer::CopyBrightness(RFramebuffer* destFbo) noexcept
{
	_fbos[GB_FBO_BRIGHT]->CopyColor(destFbo, TextureFilter::Nearest);
}

void DeferredBuffer::CopyDepth(RFramebuffer* destFbo) noexcept
{
	_fbos[GB_FBO_GEOMETRY]->CopyDepth(destFbo);
}

void DeferredBuffer::CopyStencil(RFramebuffer* destFbo) noexcept
{
	_fbos[GB_FBO_GEOMETRY]->CopyStencil(destFbo);
}

void DeferredBuffer::EnableAO(bool enable) noexcept
{
	float f = enable ? 1.f : 0.f;
	_sceneLightUbo->BeginUpdate();
	_sceneLightUbo->UpdateData((sizeof(vec4) * 3) + (sizeof(float) * 2), sizeof(float), &f);
	_sceneLightUbo->EndUpdate();
}

void DeferredBuffer::Release() noexcept
{
	_DeleteTextures();

	ResourceManager::UnloadResourceByName("sh_geometry", ResourceType::RES_SHADER);
	ResourceManager::UnloadResourceByName("sh_lighting", ResourceType::RES_SHADER);
	
	delete _fbos[GB_FBO_GEOMETRY]; _fbos[GB_FBO_GEOMETRY] = nullptr;
	delete _fbos[GB_FBO_LIGHT]; _fbos[GB_FBO_LIGHT] = nullptr;
	delete _fbos[GB_FBO_BRIGHT]; _fbos[GB_FBO_BRIGHT] = nullptr;
	delete _fbos[GB_FBO_LIGHT_ACCUM]; _fbos[GB_FBO_LIGHT_ACCUM] = nullptr;

	delete _shadow; _shadow = nullptr;
	delete _ssao; _ssao = nullptr;
	delete _hbao; _hbao = nullptr;

	if (_lightSphere)
	{
		_lightSphere->Unload();
		delete _lightSphere;
		_lightSphere = nullptr;
	}
	
	delete _lightMatrixUbo; _lightMatrixUbo = nullptr;
	delete _sceneLightUbo; _sceneLightUbo = nullptr;
	delete _lightUbo; _lightUbo = nullptr;
	
	Logger::Log(DR_MODULE, LOG_INFORMATION, "Released");
}

void DeferredBuffer::_RegisterCVars()
{
	REGISTER_CVAR_STATIC(enableHBAO, _drCVar_GetHBAO, _drCVar_SetHBAO);
	REGISTER_CVAR_STATIC(enableSSAO, _drCVar_GetSSAO, _drCVar_SetSSAO);
}
