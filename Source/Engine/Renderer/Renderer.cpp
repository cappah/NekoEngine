/* NekoEngine
 *
 * Renderer.cpp
 * Author: Alexandru Naiman
 *
 * Forward+ Renderer
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

#include <set>

#include <Renderer/GUI.h>
#include <Renderer/SSAO.h>
#include <Renderer/Debug.h>
#include <Renderer/VKUtil.h>
#include <Renderer/Texture.h>
#include <Renderer/Renderer.h>
#include <Renderer/Swapchain.h>
#include <Renderer/PostProcessor.h>
#include <Renderer/PipelineManager.h>
#include <Renderer/RenderPassManager.h>
#include <Engine/Engine.h>
#include <Engine/SceneManager.h>
#include <Engine/CameraManager.h>

#define RENDERER_MODULE "VulkanRenderer"

VKUTIL_OBJS;

using namespace std;
using namespace glm;

const vector<const char*> _ValidationLayers =
{
	"VK_LAYER_LUNARG_standard_validation"
};

const vector<const char *> _DeviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

QueueFamilyIndices _queueIndices;
NString _vkVersion;
static uint _wkGroupsX = 0, _wkGroupsY = 0, _numTiles = 0;
static Light *_lights;
static Renderer *_rendererInstance = nullptr;
static VkImageView _depthImageView = VK_NULL_HANDLE, _stencilImageView = VK_NULL_HANDLE;
char _deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];

Renderer *Renderer::GetInstance()
{
	if (!_rendererInstance)
		_rendererInstance = new Renderer();
	return _rendererInstance;
}

void Renderer::Release()
{
	delete _rendererInstance;
	_rendererInstance = nullptr;
}

Renderer::Renderer()
{
	_buffer = nullptr;
	_stagingBuffer = nullptr;

	_instance = VK_NULL_HANDLE;
	_device = VK_NULL_HANDLE;
	_graphicsQueue = _computeQueue = _transferQueue = _presentQueue = VK_NULL_HANDLE;
	_physicalDevice = VK_NULL_HANDLE;
	_graphicsCommandPool = _computeCommandPool = VK_NULL_HANDLE;
	_surface = VK_NULL_HANDLE;
	_allocator = VK_NULL_HANDLE;
	_debugCB = VK_NULL_HANDLE;
	memset(&_swapchainInfo, 0x0, sizeof(SwapchainInfo));
	_swapchain = nullptr;

	_colorTarget = _depthTarget = nullptr;
	_msaaColorTarget = nullptr;
	_normalBrightTarget = _msaaNormalBrightTarget = nullptr;
	_framebuffer = _depthFramebuffer = _guiFramebuffer = VK_NULL_HANDLE;
	_depthSampler = VK_NULL_HANDLE;
	_nearestSampler = VK_NULL_HANDLE;
	_textureSampler = VK_NULL_HANDLE;

	_descriptorPool = _computeDescriptorPool = VK_NULL_HANDLE;
	_sceneDescriptorSetLayout = _cullingDescriptorSetLayout = VK_NULL_HANDLE;
	_sceneDescriptorSet = _cullingDescriptorSet = VK_NULL_HANDLE;

	_imageAvailableSemaphore = _depthFinishedSemaphore = _cullingFinishedSemaphore = \
		_sceneFinishedSemaphore = _renderFinishedSemaphore = _aoFinishedSemaphore = \
		_aoReadySemaphore = VK_NULL_HANDLE;
}

const char *Renderer::GetAPIVersion() { return *_vkVersion; }
const char *Renderer::GetDeviceName() { return _deviceName; }

int Renderer::Initialize(PlatformWindowType window, bool enableValidation, bool debug)
{
	Logger::Log(RENDERER_MODULE, LOG_INFORMATION, "Initializing...");

	_nFrames = 0;
	_depthSampler = VK_NULL_HANDLE;

	_sceneData =
	{
		mat4(),
		mat4(),
		vec4(.2f, .2f, .2f, 1.f),
		vec4(-40.f, 10.f, 0.f, 0.f),
		ivec2(), 0, 0, vec4(), mat4()
	};

	if (!_CreateInstance(enableValidation))
		return ENGINE_INSTANCE_CREATE_FAIL;

	if (!Platform::CreateSurface(_instance, _surface, window, _allocator))
		return ENGINE_SURFACE_CREATE_FAIL;

	if (!_CreateDevice(enableValidation, debug))
		return ENGINE_DEVICE_CREATE_FAIL;

	if (debug)
		DebugMarker::Initialize(_device);

	if (debug && enableValidation && !_SetupDebugCallback())
		return ENGINE_DEBUG_INIT_FAIL;

	if ((_swapchain = new Swapchain(_swapchainInfo)) == nullptr)
		return ENGINE_SWAPCHAIN_CREATE_FAIL;

	if (!_CreateCommandPools())
		return ENGINE_CMDPOOL_CREATE_FAIL;

	VKUtil::Initialize(_device, _physicalDevice, _graphicsQueue, _graphicsCommandPool, _computeQueue, _computeCommandPool, _allocator);	

	RenderPassManager::Initialize();
	if (PipelineManager::Initialize() != ENGINE_OK)
		return ENGINE_PIPELINE_INIT_FAIL;

	if (!_CreateFramebuffers())
		return ENGINE_FRAMEUBFFER_CREATE_FAIL;

	if (!VKUtil::CreateSampler(_depthSampler, VK_FILTER_NEAREST, VK_FILTER_NEAREST,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 0.f, VK_SAMPLER_MIPMAP_MODE_LINEAR,
		0.f, 0.f, 0.f, VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE))
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create depth sampler");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_depthSampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, "Depth sampler");

	if (!VKUtil::CreateSampler(_nearestSampler, VK_FILTER_NEAREST, VK_FILTER_NEAREST,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 0.f, VK_SAMPLER_MIPMAP_MODE_LINEAR,
		0.f, 0.f, 0.f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK))
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create depth sampler");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_nearestSampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, "Nearest sampler");

	if (!VKUtil::CreateSampler(_textureSampler, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, 16.f, VK_SAMPLER_MIPMAP_MODE_LINEAR,
		0.f, 0.f, 0.f, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK))
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create depth sampler");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_textureSampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, "Filtered sampler");

	_lights = (Light *)calloc(Engine::GetConfiguration().Renderer.MaxLights, sizeof(Light));

	_sceneDescriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_Scene);
	_cullingDescriptorSetLayout = PipelineManager::GetDescriptorSetLayout(DESC_LYT_Culling);

	_wkGroupsX = (Engine::GetScreenWidth() + (Engine::GetScreenWidth() % 16)) / 16;
	_wkGroupsY = (Engine::GetScreenHeight() + (Engine::GetScreenHeight() % 16)) / 16;
	++_wkGroupsY;
	_numTiles = _wkGroupsX * _wkGroupsY;
	_sceneData.NumberOfTilesX = (int32_t)_wkGroupsX;

	if (!_CreateBuffer())
		return ENGINE_FAIL;

	if (Engine::GetConfiguration().PostProcessor.Enable)
	{
		if (PostProcessor::Initialize() != ENGINE_OK)
		{
			Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to initialize the post processor");
			return ENGINE_FAIL;
		}
	}

	if (Engine::GetConfiguration().Renderer.SSAO.Enable)
	{
		if (SSAO::Initialize() != ENGINE_OK)
		{
			Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to initialize SSAO");
			return ENGINE_FAIL;
		}
	}

	if (!_CreateDescriptorSets())
		return ENGINE_DESCRIPTOR_SET_CREATE_FAIL;

	if (!_CreateCommandBuffers())
		return ENGINE_CMDBUFFER_CREATE_FAIL;

	if (!_CreateSemaphores())
		return ENGINE_SEMAPHORE_CREATE_FAIL;

	for (int i = 0; i < MAX_INFLIGHT_COMMAND_BUFFERS; ++i)
		_depthCommandBuffers[i] = _sceneCommandBuffers[i] = _guiCommandBuffers[i] = VK_NULL_HANDLE;

	_currentDepthCB = -1;
	_currentSceneCB = -1;
	_currentGUICB = -1;

	_rebuildDepthCB = _rebuildSceneCB = _rebuildGUICB = true;

	for (uint32_t i = 0; i < _swapchain->GetImageCount(); ++i)
		VKUtil::TransitionImageLayout(_swapchain->GetImage(i), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	Logger::Log(RENDERER_MODULE, LOG_INFORMATION, "Initialized");

	return ENGINE_OK;
}

bool Renderer::_CreateBuffer()
{
	VkDeviceSize bufferSize = sizeof(SceneData) + (sizeof(Light) * Engine::GetConfiguration().Renderer.MaxLights) + (_numTiles * Engine::GetConfiguration().Renderer.MaxLights * sizeof(int32_t));

	_stagingBuffer = new Buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	_buffer = new Buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, nullptr, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	DBG_SET_OBJECT_NAME((uint64_t)_buffer->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, "Scene data buffer");
	DBG_SET_OBJECT_NAME((uint64_t)_stagingBuffer->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, "Scene data staging buffer");
	DBG_SET_OBJECT_NAME((uint64_t)_buffer->GetMemoryHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, "Scene data buffer memory");
	DBG_SET_OBJECT_NAME((uint64_t)_stagingBuffer->GetMemoryHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, "Scene data staging buffer memory");

	_sceneData.View = mat4();
	_sceneData.Projection = mat4();
	_sceneData.ScreenSize = ivec2(Engine::GetScreenWidth(), Engine::GetScreenHeight());

	uint8_t *ptr = _stagingBuffer->Map();
	{
		memcpy(ptr, &_sceneData, sizeof(SceneData));
		memcpy(ptr + sizeof(SceneData), _lights, sizeof(Light) * Engine::GetConfiguration().Renderer.MaxLights);
	}
	_stagingBuffer->Unmap();

	VKUtil::CopyBuffer(_stagingBuffer->GetHandle(), _buffer->GetHandle(), bufferSize, 0, 0);

	return true;
}

VkCommandBuffer Renderer::CreateMeshCommandBuffer()
{
	VkCommandBuffer ret = VKUtil::CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY, _graphicsCommandPool);
	DBG_SET_OBJECT_NAME((uint64_t)ret, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Mesh command buffer");
	return ret;
}

void Renderer::FreeMeshCommandBuffer(VkCommandBuffer buffer)
{
	VKUtil::FreeCommandBuffer(buffer, _graphicsCommandPool);
}

VkDescriptorPool Renderer::CreateMeshDescriptorPool()
{
	VkDescriptorPool pool;

	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &pool) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
		return VK_NULL_HANDLE;
	}
	DBG_SET_OBJECT_NAME((uint64_t)pool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, "Mesh descriptor pool");

	return pool;
}

VkDescriptorPool Renderer::CreateAnimatedMeshDescriptorPool()
{
	VkDescriptorPool pool;

	VkDescriptorPoolSize poolSize[2]{};
	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = 1;
	poolSize[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSize[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSize;
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(VKUtil::GetDevice(), &poolInfo, VKUtil::GetAllocator(), &pool) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
		return VK_NULL_HANDLE;
	}
	DBG_SET_OBJECT_NAME((uint64_t)pool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, "Animated mesh descriptor pool");

	return pool;
}

void Renderer::FreeMeshDescriptorPool(VkDescriptorPool pool)
{
	if(pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(_device, pool, _allocator);
}

VkImage Renderer::GetRenderTargetImage()
{
	return _colorTarget->GetImage();
}

VkImage Renderer::GetDepthStencilImage()
{
	return _depthTarget->GetImage();
}

VkImage Renderer::GetNormalBrightImage()
{
	return _normalBrightTarget->GetImage();
}

VkImage Renderer::GetMSAANormalBrightImage()
{
	return _msaaNormalBrightTarget->GetImage();
}

VkImageView Renderer::GetRenderTargetImageView()
{
	return _colorTarget->GetImageView();
}

VkImageView Renderer::GetDepthStencilImageView()
{
	return _depthTarget->GetImageView();
}

VkImageView Renderer::GetNormalBrightImageView()
{
	return _normalBrightTarget->GetImageView();
}

VkImageView Renderer::GetMSAANormalBrightImageView()
{
	return _msaaNormalBrightTarget->GetImageView();
}

VkImageView Renderer::GetDepthImageView()
{
	return _depthImageView;
}

VkImageView Renderer::GetStencilImageView()
{
	return _stencilImageView;
}

Buffer *Renderer::GetStagingBuffer(VkDeviceSize size)
{
	Buffer *ret = new Buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	DBG_SET_OBJECT_NAME((uint64_t)ret->GetHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, "Staging buffer");
	DBG_SET_OBJECT_NAME((uint64_t)ret->GetMemoryHandle(), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, "Staging buffer memory");
	return ret;
}

void Renderer::FreeStagingBuffer(Buffer *stagingBuffer)
{
	delete stagingBuffer;
}

Light *Renderer::AllocLight()
{
	return &_lights[_sceneData.LightCount++];
}

Light *Renderer::GetLight(uint32_t lightId)
{
	return &_lights[lightId];
}

void Renderer::FreeLight(Light *light)
{
	//
}

void Renderer::Update(double deltaTime)
{
	if (!SceneManager::IsSceneLoaded())
	{
		return;
	}

	Camera *cam = CameraManager::GetActiveCamera();
	_sceneData.View = cam->GetView();
	_sceneData.Projection = cam->GetProjectionMatrix();
	_sceneData.CameraPosition = vec4(cam->GetPosition(), 1.f);
	_sceneData.ScreenSize = ivec2(Engine::GetScreenWidth(), Engine::GetScreenHeight());

	VkCommandBuffer updateBuffer = VKUtil::CreateOneShotCmdBuffer();

	DBG_MARKER_BEGIN(updateBuffer, "Update data", vec4(0.83, 0.63, 0.56, 1.0));

	DBG_MARKER_INSERT(updateBuffer, "Update objects", vec4(0.83, 0.73, 0.56, 1.0));

	if (SceneManager::IsSceneLoaded())
		SceneManager::GetActiveScene()->UpdateData(updateBuffer);

	uint8_t *ptr = _stagingBuffer->Map();
	{
		memcpy(ptr, &_sceneData, sizeof(SceneData));
		memcpy(ptr + sizeof(SceneData), _lights, sizeof(Light) * Engine::GetConfiguration().Renderer.MaxLights);
	}
	_stagingBuffer->Unmap();

	DBG_MARKER_INSERT(updateBuffer, "Update scene", vec4(0.83, 0.73, 0.56, 1.0));

	VKUtil::CopyBuffer(_stagingBuffer->GetHandle(), _buffer->GetHandle(), sizeof(SceneData) + sizeof(Light) * _sceneData.LightCount, 0, 0, updateBuffer);

	DBG_MARKER_INSERT(updateBuffer, "Update GUI", vec4(0.83, 0.73, 0.56, 1.0));

	if (Engine::GetConfiguration().Renderer.SSAO.Enable)
		SSAO::UpdateData(updateBuffer);

	GUI::UpdateData(updateBuffer);

	DBG_MARKER_END(updateBuffer);

	VKUtil::ExecuteOneShotCmdBuffer(updateBuffer);
}

void Renderer::Draw()
{
	if (!SceneManager::IsSceneLoaded())
	{
		return;
	}

	uint32_t imageIndex = _swapchain->AcquireNextImage(_imageAvailableSemaphore);

	if (imageIndex == UINT32_MAX)
	{
		_RecreateSwapchain();
		return;
	}

	ResetDepthCommandBuffers();
	ResetSceneCommandBuffers();

	SceneManager::GetActiveScene()->PrepareCommandBuffers();

	_BuildDepthCommandBuffer();
	_BuildSceneCommandBuffer();

	if (_rebuildGUICB)
	{
		_BuildGUICommandBuffer();
		_rebuildGUICB = false;
	}
	
	if(Engine::GetConfiguration().Renderer.EnableAsyncCompute)
		_SubmitAsync(imageIndex);
	else
		_Submit(imageIndex);

	if (_swapchain->Present(_renderFinishedSemaphore, imageIndex, _presentQueue) == UINT32_MAX)
		_RecreateSwapchain();
}

void Renderer::_Submit(uint32_t imageIndex)
{
	VkResult result;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkCommandBuffer buffers[]{ _depthCommandBuffers[_currentDepthCB], SSAO::GetCommandBuffer() };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = Engine::GetConfiguration().Renderer.SSAO.Enable ? 2 : 1;
	submitInfo.pCommandBuffers = buffers;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_depthFinishedSemaphore;

	if ((result = vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit depth draw buffer: %d", result);
		DIE("Failed to submit depth draw buffer");
	}

	submitInfo.pWaitSemaphores = &_depthFinishedSemaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_cullingCommandBuffer;
	submitInfo.pSignalSemaphores = &_cullingFinishedSemaphore;

	if (vkQueueSubmit(_computeQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit culling compute buffer: %d", result);
		DIE("Failed to submit culling compute  buffer");
	}

	submitInfo.pWaitSemaphores = &_cullingFinishedSemaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_sceneCommandBuffers[_currentSceneCB];
	submitInfo.pSignalSemaphores = &_sceneFinishedSemaphore;

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit scene draw buffer: %d", result);
		DIE("Failed to submit scene draw buffer");
	}

	submitInfo.pWaitSemaphores = &_sceneFinishedSemaphore;

	VkCommandBuffer cBuffers[3];
	if (Engine::GetConfiguration().PostProcessor.Enable)
	{
		cBuffers[0] = PostProcessor::GetCommandBuffer();
		cBuffers[1] = _guiCommandBuffers[_currentGUICB];
		cBuffers[2] = _presentCommandBuffers[imageIndex];
		submitInfo.commandBufferCount = 3;
	}
	else
	{
		cBuffers[0] = _guiCommandBuffers[_currentGUICB];
		cBuffers[1] = _presentCommandBuffers[imageIndex];
		submitInfo.commandBufferCount = 2;
	}

	submitInfo.pCommandBuffers = cBuffers;
	submitInfo.pSignalSemaphores = &_renderFinishedSemaphore;

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit present buffer: %d", result);
		DIE("Failed to submit present buffer");
	}
}

void Renderer::_SubmitAsync(uint32_t imageIndex)
{
	VkResult result;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	VkSemaphore depthSignalSemaphores[]{ _depthFinishedSemaphore, _aoReadySemaphore };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_depthCommandBuffers[_currentDepthCB];
	submitInfo.signalSemaphoreCount = Engine::GetConfiguration().Renderer.SSAO.Enable ? 2 : 1;
	submitInfo.pSignalSemaphores = depthSignalSemaphores;

	if ((result = vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE)) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit depth draw buffer: %d", result);
		DIE("Failed to submit depth draw buffer");
	}

	submitInfo.pWaitSemaphores = &_depthFinishedSemaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_cullingCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_cullingFinishedSemaphore;

	if (vkQueueSubmit(_computeQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit culling compute buffer: %d", result);
		DIE("Failed to submit culling compute  buffer");
	}

	if (Engine::GetConfiguration().Renderer.SSAO.Enable)
	{
		VkCommandBuffer ssaoCommandBuffer = SSAO::GetCommandBuffer();

		submitInfo.pWaitSemaphores = &_aoReadySemaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &ssaoCommandBuffer;
		submitInfo.pSignalSemaphores = &_aoFinishedSemaphore;

		if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
		{
			Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit SSAO buffer: %d", result);
			DIE("Failed to submit SSAO  buffer");
		}
	}

	VkSemaphore sceneWaitSemaphores[]{ _cullingFinishedSemaphore, _aoFinishedSemaphore };
	VkPipelineStageFlags sceneWaitStages[] = { VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.pWaitDstStageMask = sceneWaitStages;
	submitInfo.waitSemaphoreCount = Engine::GetConfiguration().Renderer.SSAO.Enable ? 2 : 1;
	submitInfo.pWaitSemaphores = sceneWaitSemaphores;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_sceneCommandBuffers[_currentSceneCB];
	submitInfo.pSignalSemaphores = &_sceneFinishedSemaphore;

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit scene draw buffer: %d", result);
		DIE("Failed to submit scene draw buffer");
	}

	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &_sceneFinishedSemaphore;

	VkCommandBuffer cBuffers[3];
	if (Engine::GetConfiguration().PostProcessor.Enable)
	{
		cBuffers[0] = PostProcessor::GetCommandBuffer();
		cBuffers[1] = _guiCommandBuffers[_currentGUICB];
		cBuffers[2] = _presentCommandBuffers[imageIndex];
		submitInfo.commandBufferCount = 3;
	}
	else
	{
		cBuffers[0] = _guiCommandBuffers[_currentGUICB];
		cBuffers[1] = _presentCommandBuffers[imageIndex];
		submitInfo.commandBufferCount = 2;
	}

	submitInfo.pCommandBuffers = cBuffers;
	submitInfo.pSignalSemaphores = &_renderFinishedSemaphore;

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to submit present buffer: %d", result);
		DIE("Failed to submit present buffer");
	}
}

void Renderer::ScreenResized()
{
	// swapchain
	
	// Recreate framebuffers
	_DestroyFramebuffers();
	_CreateFramebuffers();

	// Recreate command buffers
	_DestroyCommandBuffers();
	_CreateCommandBuffers();
}

void Renderer::_RecreateSwapchain()
{
	vkDeviceWaitIdle(_device);

	/*_swapchain->Resize(_width, _height);

	vkDestroyImage(_device, _fbImage, _allocator);
	vkFreeMemory(_device, _fbImageMemory, _allocator);
	vkDestroyImage(_device, _fbDepthImage, _allocator);
	vkFreeMemory(_device, _fbDepthImageMemory, _allocator);

	vkDestroyImageView(_device, _fbImageView, _allocator);
	vkDestroyImageView(_device, _fbDepthImageView, _allocator);

	vkDestroyFramebuffer(_device, _framebuffer, _allocator);
	vkDestroyFramebuffer(_device, _depthFramebuffer, _allocator);
	vkDestroyFramebuffer(_device, _guiFramebuffer, _allocator);

	vkFreeCommandBuffers(_device, _commandPool, 1, &_depthCommandBuffer);
	vkFreeCommandBuffers(_device, _commandPool, 1, &_cmdBuffer);
	vkFreeCommandBuffers(_device, _computeCommandPool, 1, &_cullingCommandBuffer);

	_CreateFramebufferImage();
	_CreateFramebuffer();

	_CreateSwapchainFramebuffers();
	_CreateSwapchainCommandBuffers();

	_BuildCommandBuffer();

	vkDeviceWaitIdle(_device);*/
}

bool _checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	vector<VkLayerProperties> layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	for (const char *name : _ValidationLayers)
	{
		bool found = false;

		size_t len = strlen(name);
		for (VkLayerProperties &properties : layers)
		{
			if (!strncmp(name, properties.layerName, len))
			{
				found = true;
				break;
			}
		}

		if (!found) return false;
	}

	return true;
}

static VkBool32 debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj,
	size_t location, int32_t code, const char *layerPrefix, const char *msg, void *user)
{
	Logger::Log(RENDERER_MODULE, LOG_DEBUG, "Vaidaton [%s]: %s", layerPrefix, msg);
	return VK_FALSE;
}

bool Renderer::_CheckDeviceExtension(const char *name)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);

	vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, extensions.data());

	size_t len = strlen(name);

	for (VkExtensionProperties &ext : extensions)
		if (!strncmp(ext.extensionName, name, len))
			return true;
	return false;
}

bool Renderer::_CreateInstance(bool debug)
{
	if (debug && !_checkValidationLayerSupport())
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Validation requested, but validation layers not present");
		return false;
	}

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "NekoEngine";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 4, 0);
	appInfo.pEngineName = "NekoEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 4, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 20);

	VkInstanceCreateInfo instInfo = {};
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instInfo.pApplicationInfo = &appInfo;

	vector<const char*> extensions = Platform::GetRequiredExtensions(debug);

	instInfo.enabledExtensionCount = (uint32_t)extensions.size();
	instInfo.ppEnabledExtensionNames = extensions.data();

	if (debug)
	{
		instInfo.enabledLayerCount = (uint32_t)_ValidationLayers.size();
		instInfo.ppEnabledLayerNames = _ValidationLayers.data();
	}
	else
		instInfo.enabledLayerCount = 0;

	VkResult result;
	if ((result = vkCreateInstance(&instInfo, _allocator, &_instance)) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create instance %d", result);
		return false;
	}

	return true;
}

bool Renderer::_SetupDebugCallback()
{
	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)debugCallback;

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugReportCallbackEXT");

	if (vkCreateDebugReportCallbackEXT(_instance, &createInfo, nullptr, &_debugCB) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create debug callback");
		return false;
	}

	return true;
}

bool Renderer::_CreateDevice(bool enableValidation, bool debug)
{
	uint32_t deviceCount = 0;

	if (vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkEnumeratePhysicalDevices call failed");
		return false;
	}

	if (!deviceCount)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "No physical devices found");
		return false;
	}

	vector<VkPhysicalDevice> devices(deviceCount);
	if (vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data()) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkEnumeratePhysicalDevices call failed");
		return false;
	}

	for (VkPhysicalDevice &device : devices)
	{
		uint32_t max_mem = 0;

		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);

		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);

		uint32_t familyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, nullptr);

		vector<VkQueueFamilyProperties> families(familyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familyCount, families.data());

		int graphicsQueueIndex = -1, presentQueueIndex = -1, computeQueueIndex = -1, i = 0;
		for (VkQueueFamilyProperties familyProperties : families)
		{
			VkBool32 presentSupport = false;
			if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport) != VK_SUCCESS)
				continue;

			if (familyProperties.queueCount > 0 && familyProperties.queueFlags & VK_QUEUE_COMPUTE_BIT)
				computeQueueIndex = i;

			if (familyProperties.queueCount > 0 && familyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				graphicsQueueIndex = i;
			else
				continue;

			if (familyProperties.queueCount > 0 && presentSupport)
				presentQueueIndex = i;

			if (graphicsQueueIndex > 0 && presentQueueIndex > 0)
				break;

			++i;
		}

		if (graphicsQueueIndex < 0)
			continue;

		uint32_t extensionCount;
		if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) != VK_SUCCESS)
			continue;

		vector<VkExtensionProperties> extensions(extensionCount);
		if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data()) != VK_SUCCESS)
			continue;

		set<string> requiredExtensions(_DeviceExtensions.begin(), _DeviceExtensions.end());

		for (VkExtensionProperties extProperties : extensions)
			requiredExtensions.erase(extProperties.extensionName);

		if (!requiredExtensions.empty())
			continue;

		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &_swapchainInfo.capabilities) != VK_SUCCESS)
			continue;

		uint32_t formatCount = 0;
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr) != VK_SUCCESS)
			continue;

		_swapchainInfo.formats.resize(formatCount);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, _swapchainInfo.formats.data()) != VK_SUCCESS)
			continue;

		uint32_t presentModeCount = 0;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr) != VK_SUCCESS)
			continue;

		_swapchainInfo.presentModes.resize(presentModeCount);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, _swapchainInfo.presentModes.data()) != VK_SUCCESS)
			continue;

		if (_swapchainInfo.formats.empty() || _swapchainInfo.presentModes.empty())
			continue;

		if (properties.vendorID == 0x10DE) // prefer NVIDIA
		{
			_physicalDevice = device;
			_queueIndices.graphicsFamily = graphicsQueueIndex;
			_queueIndices.presentFamily = presentQueueIndex;
			_queueIndices.computeFamily = computeQueueIndex;
		
			memset(_deviceName, 0x0, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
			memcpy(_deviceName, properties.deviceName, strlen(properties.deviceName));

			break;
		}

		if (properties.limits.maxMemoryAllocationCount > max_mem)
		{
			max_mem = properties.limits.maxMemoryAllocationCount;
			_physicalDevice = device;
			_queueIndices.graphicsFamily = graphicsQueueIndex;
			_queueIndices.presentFamily = presentQueueIndex;
			_queueIndices.computeFamily = computeQueueIndex;
		}

		memset(_deviceName, 0x0, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
		memcpy(_deviceName, properties.deviceName, strlen(properties.deviceName));
	}

	if (!_physicalDevice)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "No suitable device found");
		return false;
	}

	float queuePriority = 1.f;
	vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	set<int> uniqueQueueFamilies = { _queueIndices.graphicsFamily, _queueIndices.presentFamily, _queueIndices.computeFamily };

	for (int family : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = family;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.independentBlend = VK_TRUE;
	deviceFeatures.depthBounds = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.geometryShader = VK_TRUE;
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.textureCompressionBC = VK_TRUE;
	deviceFeatures.fullDrawIndexUint32 = VK_TRUE;

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(_physicalDevice, &properties);
	_vkVersion = NString::StringWithFormat(10, "%d.%d.%d", (properties.apiVersion >> 22),
		((properties.apiVersion >> 12) & 0x3FF), (properties.apiVersion & 0xFFFF));

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
	createInfo.pEnabledFeatures = &deviceFeatures;

	vector<const char *> extensions;
	for (const char *ext : _DeviceExtensions)
		extensions.push_back(ext);

	if (debug && _CheckDeviceExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
		extensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);

	createInfo.enabledExtensionCount = (uint32_t)extensions.size();
	createInfo.ppEnabledExtensionNames = extensions.data();

	if (enableValidation)
	{
		createInfo.enabledLayerCount = (uint32_t)_ValidationLayers.size();
		createInfo.ppEnabledLayerNames = _ValidationLayers.data();
	}
	else
		createInfo.enabledLayerCount = 0;

	Logger::Log(RENDERER_MODULE, LOG_INFORMATION, "Device: %s", _deviceName);

	if (vkCreateDevice(_physicalDevice, &createInfo, _allocator, &_device) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkCreateDevice call failed");
		return false;
	}

	vkGetDeviceQueue(_device, _queueIndices.graphicsFamily, 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, _queueIndices.presentFamily, 0, &_presentQueue);
	vkGetDeviceQueue(_device, _queueIndices.computeFamily, 0, &_computeQueue);

	_swapchainInfo.device = _device;
	_swapchainInfo.surface = _surface;
	_swapchainInfo.allocator = nullptr;
	_swapchainInfo.graphicsFamily = _queueIndices.graphicsFamily;
	_swapchainInfo.presentFamily = _queueIndices.presentFamily;

	return true;
}

bool Renderer::_CreateCommandPools()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = _queueIndices.graphicsFamily;
	poolInfo.flags = 0;

	if (vkCreateCommandPool(_device, &poolInfo, _allocator, &_graphicsCommandPool) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create graphics command pool");
		return false;
	}

	/*poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	if (vkCreateCommandPool(_device, &poolInfo, _allocator, &_resetableGraphicsCommandPool) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create graphics command pool");
		return false;
	}*/

	poolInfo.flags = 0;
	poolInfo.queueFamilyIndex = _queueIndices.computeFamily;

	if (vkCreateCommandPool(_device, &poolInfo, _allocator, &_computeCommandPool) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create compute command pool");
		return false;
	}

	return true;
}

bool Renderer::_CreateFramebuffers()
{
	//*****************
	//* Images & views
	//*****************

	_colorTarget = new Texture(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_TILING_OPTIMAL, Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true, VK_NULL_HANDLE, VK_SAMPLE_COUNT_1_BIT);
	DBG_SET_OBJECT_NAME((uint64_t)_colorTarget->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Color render target image");
	_colorTarget->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
	DBG_SET_OBJECT_NAME((uint64_t)_colorTarget->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Color render target image view");
	VKUtil::TransitionImageLayout(_colorTarget->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	_normalBrightTarget = new Texture(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_TILING_OPTIMAL, Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true, VK_NULL_HANDLE, VK_SAMPLE_COUNT_1_BIT);
	DBG_SET_OBJECT_NAME((uint64_t)_normalBrightTarget->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Normal & brightness render target image");
	_normalBrightTarget->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
	DBG_SET_OBJECT_NAME((uint64_t)_normalBrightTarget->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Normal & brightness render target image view");
	VKUtil::TransitionImageLayout(_normalBrightTarget->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

	NArray<VkImageView> attachments(5);
	NArray<VkImageView> depthAttachments(2);

	if (Engine::GetConfiguration().Renderer.Multisampling)
	{
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		switch (Engine::GetConfiguration().Renderer.Samples)
		{
			case 2: samples = VK_SAMPLE_COUNT_2_BIT; break;
			case 4: samples = VK_SAMPLE_COUNT_4_BIT; break;
			case 8: samples = VK_SAMPLE_COUNT_8_BIT; break;
			case 16: samples = VK_SAMPLE_COUNT_16_BIT; break;
			case 32: samples = VK_SAMPLE_COUNT_32_BIT; break;
			case 64: samples = VK_SAMPLE_COUNT_64_BIT; break;
		}

		_msaaColorTarget = new Texture(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL,
			Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true, VK_NULL_HANDLE, samples);
		DBG_SET_OBJECT_NAME((uint64_t)_msaaColorTarget->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "MSAA color render target image");	
		_msaaColorTarget->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
		DBG_SET_OBJECT_NAME((uint64_t)_msaaColorTarget->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "MSAA color render target image view");
		VKUtil::TransitionImageLayout(_msaaColorTarget->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

		_depthTarget = new Texture(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL,
			Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true, VK_NULL_HANDLE, samples);
		DBG_SET_OBJECT_NAME((uint64_t)_depthTarget->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Depth target image");
		_depthTarget->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		DBG_SET_OBJECT_NAME((uint64_t)_depthTarget->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Depth target image view");
		VKUtil::TransitionImageLayout(_depthTarget->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

		_msaaNormalBrightTarget = new Texture(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL,
			Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true, VK_NULL_HANDLE, samples);
		DBG_SET_OBJECT_NAME((uint64_t)_msaaNormalBrightTarget->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "MSAA normal & brightness render target image");
		_msaaNormalBrightTarget->CreateView(VK_IMAGE_ASPECT_COLOR_BIT);
		DBG_SET_OBJECT_NAME((uint64_t)_msaaNormalBrightTarget->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "MSAA normal & brightness render target image view");
		VKUtil::TransitionImageLayout(_msaaNormalBrightTarget->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);

		attachments.Add(_msaaColorTarget->GetImageView());
		attachments.Add(_depthTarget->GetImageView());
		attachments.Add(_msaaNormalBrightTarget->GetImageView());
		attachments.Add(_colorTarget->GetImageView());
		attachments.Add(_normalBrightTarget->GetImageView());

		depthAttachments.Add(_depthTarget->GetImageView());
		depthAttachments.Add(_msaaNormalBrightTarget->GetImageView());
	}
	else
	{
		_depthTarget = new Texture(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TYPE_2D, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_TILING_OPTIMAL,
			Engine::GetScreenWidth(), Engine::GetScreenHeight(), 1, true, VK_NULL_HANDLE, VK_SAMPLE_COUNT_1_BIT);
		DBG_SET_OBJECT_NAME((uint64_t)_depthTarget->GetImage(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Depth target image");
		_depthTarget->CreateView(VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);
		DBG_SET_OBJECT_NAME((uint64_t)_depthTarget->GetImageView(), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Depth target image view");
		VKUtil::TransitionImageLayout(_depthTarget->GetImage(), VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

		attachments.Add(_colorTarget->GetImageView());
		attachments.Add(_depthTarget->GetImageView());
		attachments.Add(_normalBrightTarget->GetImageView());

		depthAttachments.Add(_depthTarget->GetImageView());
		depthAttachments.Add(_normalBrightTarget->GetImageView());
	}

	if (!VKUtil::CreateImageView(_depthImageView, _depthTarget->GetImage(), VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT))
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create depth image view");
		return false;
	}

	if (!VKUtil::CreateImageView(_stencilImageView, _depthTarget->GetImage(), VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_STENCIL_BIT))
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create stencil image view");
		return false;
	}

	//**********************
	//* Default framebuffer
	//**********************

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = RenderPassManager::GetRenderPass(RP_Graphics);
	createInfo.attachmentCount = (uint32_t)attachments.Count();
	createInfo.pAttachments = *attachments;
	createInfo.width = Engine::GetScreenWidth();
	createInfo.height = Engine::GetScreenHeight();
	createInfo.layers = 1;

	if (vkCreateFramebuffer(_device, &createInfo, _allocator, &_framebuffer) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create framebuffer");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, "Default framebuffer");

	createInfo.renderPass = RenderPassManager::GetRenderPass(RP_Depth);
	createInfo.attachmentCount = (uint32_t)depthAttachments.Count();
	createInfo.pAttachments = *depthAttachments;

	if (vkCreateFramebuffer(_device, &createInfo, _allocator, &_depthFramebuffer) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create depth framebuffer");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_depthFramebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, "Depth framebuffer");

	VkImageView guiImageViews[]{ _colorTarget->GetImageView() };
	createInfo.renderPass = RenderPassManager::GetRenderPass(RP_GUI);
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = guiImageViews;

	if (vkCreateFramebuffer(_device, &createInfo, _allocator, &_guiFramebuffer) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create gui framebuffer");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_guiFramebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, "GUI framebuffer");

	return true;
}

bool Renderer::_CreateDescriptorSets()
{
	if (_descriptorPool == VK_NULL_HANDLE)
	{
		VkDescriptorPoolSize poolSizes[3]{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[1].descriptorCount = 2;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 3;
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(_device, &poolInfo, _allocator, &_descriptorPool) != VK_SUCCESS)
		{
			Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
			return false;
		}
		DBG_SET_OBJECT_NAME((uint64_t)_descriptorPool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, "Scene descriptor pool");
	}

	if (_computeDescriptorPool == VK_NULL_HANDLE)
	{
		VkDescriptorPoolSize sizes[3] = {};
		sizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		sizes[0].descriptorCount = 2;
		sizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		sizes[1].descriptorCount = 1;
		sizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sizes[2].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 3;
		poolInfo.pPoolSizes = sizes;
		poolInfo.maxSets = 1;

		if (vkCreateDescriptorPool(_device, &poolInfo, _allocator, &_computeDescriptorPool) != VK_SUCCESS)
		{
			Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create descriptor pool");
			return false;
		}
		DBG_SET_OBJECT_NAME((uint64_t)_computeDescriptorPool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, "Compute descriptor pool");
	}
	
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_sceneDescriptorSetLayout;

	if (vkAllocateDescriptorSets(VKUtil::GetDevice(), &allocInfo, &_sceneDescriptorSet) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate descriptor sets");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_sceneDescriptorSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, "Scene descriptor set");

	VkDescriptorBufferInfo sceneDataInfo{};
	sceneDataInfo.buffer = _buffer->GetHandle();
	sceneDataInfo.offset = 0;
	sceneDataInfo.range = sizeof(SceneData);

	VkDescriptorBufferInfo lightBufferInfo{};
	lightBufferInfo.buffer = _buffer->GetHandle();
	lightBufferInfo.offset = sceneDataInfo.offset + sceneDataInfo.range;
	lightBufferInfo.range = sizeof(Light) * Engine::GetConfiguration().Renderer.MaxLights;

	VkDescriptorBufferInfo visibleIndicesInfo{};
	visibleIndicesInfo.buffer = _buffer->GetHandle();
	visibleIndicesInfo.offset = lightBufferInfo.offset + lightBufferInfo.range;
	visibleIndicesInfo.range = sizeof(int32_t) * Engine::GetConfiguration().Renderer.MaxLights * _numTiles;

	VkDescriptorImageInfo aoImageInfo{};
	aoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	aoImageInfo.imageView = Engine::GetConfiguration().Renderer.SSAO.Enable ? SSAO::GetAOImageView() : VK_NULL_HANDLE;
	aoImageInfo.sampler = _nearestSampler;

	VkWriteDescriptorSet writeBuffer{};
	writeBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeBuffer.dstSet = _sceneDescriptorSet;
	writeBuffer.dstBinding = 0;
	writeBuffer.dstArrayElement = 0;
	writeBuffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeBuffer.descriptorCount = 1;
	writeBuffer.pBufferInfo = &sceneDataInfo;
	writeBuffer.pImageInfo = nullptr;
	writeBuffer.pTexelBufferView = nullptr;

	vector<VkWriteDescriptorSet> writeSets;

	writeSets.push_back(writeBuffer);

	writeBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

	writeBuffer.dstBinding = 1;
	writeBuffer.pBufferInfo = &lightBufferInfo;
	writeSets.push_back(writeBuffer);

	writeBuffer.dstBinding = 2;
	writeBuffer.pBufferInfo = &visibleIndicesInfo;
	writeSets.push_back(writeBuffer);

	writeBuffer.dstBinding = 3;
	writeBuffer.pBufferInfo = &visibleIndicesInfo;
	writeBuffer.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeBuffer.pBufferInfo = nullptr;
	writeBuffer.pImageInfo = Engine::GetConfiguration().Renderer.SSAO.Enable ? &aoImageInfo : nullptr;
	writeBuffer.descriptorCount = Engine::GetConfiguration().Renderer.SSAO.Enable ? 1 : 0;
	writeSets.push_back(writeBuffer);

	vkUpdateDescriptorSets(VKUtil::GetDevice(), (uint32_t)writeSets.size(), writeSets.data(), 0, nullptr);

	// Culling

	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _computeDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &_cullingDescriptorSetLayout;

	if (vkAllocateDescriptorSets(_device, &allocInfo, &_cullingDescriptorSet) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate descriptor sets");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_cullingDescriptorSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, "Culling descriptor set");

	VkDescriptorBufferInfo lightBlockInfo = {};
	lightBlockInfo.buffer = _buffer->GetHandle();
	lightBlockInfo.offset = sizeof(SceneData);
	lightBlockInfo.range = Engine::GetConfiguration().Renderer.MaxLights * sizeof(Light);

	VkDescriptorBufferInfo visibleIndicesBlockInfo = {};
	visibleIndicesBlockInfo.buffer = _buffer->GetHandle();
	visibleIndicesBlockInfo.offset = lightBlockInfo.offset + lightBlockInfo.range;
	visibleIndicesBlockInfo.range = Engine::GetConfiguration().Renderer.MaxLights * sizeof(int32_t) * _numTiles;

	VkDescriptorBufferInfo dataBlockInfo = {};
	dataBlockInfo.buffer = _buffer->GetHandle();
	dataBlockInfo.offset = 0;
	dataBlockInfo.range = sizeof(SceneData);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	imageInfo.imageView = _depthImageView;
	imageInfo.sampler = _depthSampler;

	VkWriteDescriptorSet descriptorWrite[4] = {};
	memset(descriptorWrite, 0x0, sizeof(VkWriteDescriptorSet) * 4);

	descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[0].dstSet = _cullingDescriptorSet;
	descriptorWrite[0].dstBinding = 0;
	descriptorWrite[0].dstArrayElement = 0;
	descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite[0].descriptorCount = 1;
	descriptorWrite[0].pBufferInfo = &lightBlockInfo;
	descriptorWrite[0].pImageInfo = nullptr;
	descriptorWrite[0].pTexelBufferView = nullptr;

	descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[1].dstSet = _cullingDescriptorSet;
	descriptorWrite[1].dstBinding = 1;
	descriptorWrite[1].dstArrayElement = 0;
	descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descriptorWrite[1].descriptorCount = 1;
	descriptorWrite[1].pBufferInfo = &visibleIndicesBlockInfo;
	descriptorWrite[1].pImageInfo = nullptr;
	descriptorWrite[1].pTexelBufferView = nullptr;

	descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[2].dstSet = _cullingDescriptorSet;
	descriptorWrite[2].dstBinding = 2;
	descriptorWrite[2].dstArrayElement = 0;
	descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite[2].descriptorCount = 1;
	descriptorWrite[2].pBufferInfo = &dataBlockInfo;
	descriptorWrite[2].pImageInfo = nullptr;
	descriptorWrite[2].pTexelBufferView = nullptr;

	descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite[3].dstSet = _cullingDescriptorSet;
	descriptorWrite[3].dstBinding = 3;
	descriptorWrite[3].dstArrayElement = 0;
	descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite[3].descriptorCount = 1;
	descriptorWrite[3].pBufferInfo = nullptr;
	descriptorWrite[3].pImageInfo = &imageInfo;
	descriptorWrite[3].pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(_device, 4, descriptorWrite, 0, nullptr);

	return true;
}

bool Renderer::_CreateCommandBuffers()
{
	// Culling

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _computeCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(_device, &allocInfo, &_cullingCommandBuffer) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate culling command buffer");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_cullingCommandBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Culling command buffer");

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;
	
	if (vkBeginCommandBuffer(_cullingCommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (culling) call failed");
		return false;
	}

	DBG_MARKER_BEGIN(_cullingCommandBuffer, "Light culling", vec4(0.34, 0.82, 0.2, 1.0));

	vkCmdBindPipeline(_cullingCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineManager::GetPipeline(PIPE_Culling));
	vkCmdBindDescriptorSets(_cullingCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, PipelineManager::GetPipelineLayout(PIPE_LYT_Culling), 0, 1, &_cullingDescriptorSet, 0, nullptr);
	vkCmdDispatch(_cullingCommandBuffer, _wkGroupsX, _wkGroupsY, 1);

	DBG_MARKER_END(_cullingCommandBuffer);

	if (vkEndCommandBuffer(_cullingCommandBuffer) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (culling) call failed");
		return false;
	}

	// Present command buffers

	if (_presentCommandBuffers.Count() > 0)
		vkFreeCommandBuffers(_device, _graphicsCommandPool, (uint32_t)_presentCommandBuffers.Size(), *_presentCommandBuffers);

	_presentCommandBuffers.Resize(_swapchain->GetImageCount());
	_presentCommandBuffers.Fill();

	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)_presentCommandBuffers.Size();

	if (vkAllocateCommandBuffers(_device, &allocInfo, *_presentCommandBuffers) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate present command buffers");
		return false;
	}

	for (size_t i = 0; i < _presentCommandBuffers.Size(); ++i)
	{
		DBG_SET_OBJECT_NAME((uint64_t)_presentCommandBuffers[i], VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Present command buffer");

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(_presentCommandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (present) call failed");
			return false;
		}

		DBG_MARKER_BEGIN(_presentCommandBuffers[i], "Present", vec4(0.9, 0.85, 0.0, 1.0));

		VKUtil::TransitionImageLayout(_colorTarget->GetImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _presentCommandBuffers[i]);
		VKUtil::TransitionImageLayout(_swapchain->GetImage((uint32_t)i), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _presentCommandBuffers[i]);
		VKUtil::BlitImage(_colorTarget->GetImage(), _swapchain->GetImage((uint32_t)i),
			Engine::GetScreenWidth(), Engine::GetScreenHeight(), Engine::GetConfiguration().Engine.ScreenWidth, Engine::GetConfiguration().Engine.ScreenHeight,
			Engine::GetConfiguration().Renderer.Supersampling ? VK_FILTER_LINEAR : VK_FILTER_NEAREST, _presentCommandBuffers[i]);
		VKUtil::TransitionImageLayout(_swapchain->GetImage((uint32_t)i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT, _presentCommandBuffers[i]);
		VKUtil::TransitionImageLayout(_colorTarget->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, _presentCommandBuffers[i]);

		DBG_MARKER_END(_presentCommandBuffers[i]);

		if (vkEndCommandBuffer(_presentCommandBuffers[i]) != VK_SUCCESS)
		{
			Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (present) call failed");
			return false;
		}
	}

	return true;
}

bool Renderer::_CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(_device, &semaphoreInfo, _allocator, &_imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(_device, &semaphoreInfo, _allocator, &_depthFinishedSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(_device, &semaphoreInfo, _allocator, &_cullingFinishedSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(_device, &semaphoreInfo, _allocator, &_sceneFinishedSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(_device, &semaphoreInfo, _allocator, &_aoReadySemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(_device, &semaphoreInfo, _allocator, &_aoFinishedSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(_device, &semaphoreInfo, _allocator, &_renderFinishedSemaphore) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to create semaphore");
		return false;
	}

	return true;
}

bool Renderer::_BuildDepthCommandBuffer()
{
	int nextCB = (_currentDepthCB + 1) % MAX_INFLIGHT_COMMAND_BUFFERS;

	if (_depthCommandBuffers[nextCB] != VK_NULL_HANDLE)
		vkFreeCommandBuffers(_device, _graphicsCommandPool, 1, &_depthCommandBuffers[nextCB]);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(_device, &allocInfo, &_depthCommandBuffers[nextCB]) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate depth command buffer");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_depthCommandBuffers[nextCB], VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Depth command buffer");

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(_depthCommandBuffers[nextCB], &beginInfo) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (depth) call failed");
		return false;
	}

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = RenderPassManager::GetRenderPass(RP_Depth);
	renderPassInfo.framebuffer = _depthFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { Engine::GetScreenWidth(), Engine::GetScreenHeight() };

	VkClearValue clearValues[2]{};
	clearValues[0].depthStencil = { 1.0f, 0 };
	clearValues[1].color = { { 0.f, 0.f, 0.f, 0.f } };

	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearValues;

	DBG_MARKER_BEGIN(_depthCommandBuffers[nextCB], "Depth pass", vec4(0.03, 0.48, 0.64, 1.0));

	vkCmdBeginRenderPass(_depthCommandBuffers[nextCB], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	vkCmdExecuteCommands(_depthCommandBuffers[nextCB], (uint32_t)_secondaryDepthCommandBuffers.Count(), *_secondaryDepthCommandBuffers);
	vkCmdEndRenderPass(_depthCommandBuffers[nextCB]);

	DBG_MARKER_END(_depthCommandBuffers[nextCB]);

	if (vkEndCommandBuffer(_depthCommandBuffers[nextCB]) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (depth) call failed");
		return false;
	}

	_currentDepthCB = nextCB;

	return true;
}

bool Renderer::_BuildSceneCommandBuffer()
{
	int nextCB = (_currentSceneCB + 1) % MAX_INFLIGHT_COMMAND_BUFFERS;

	if (_sceneCommandBuffers[nextCB] != VK_NULL_HANDLE)
		vkFreeCommandBuffers(_device, _graphicsCommandPool, 1, &_sceneCommandBuffers[nextCB]);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(_device, &allocInfo, &_sceneCommandBuffers[nextCB]) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate scene command buffer");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_sceneCommandBuffers[nextCB], VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Scene command buffer");

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(_sceneCommandBuffers[nextCB], &beginInfo) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (scene) call failed");
		return false;
	}

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = RenderPassManager::GetRenderPass(RP_Graphics);
	renderPassInfo.framebuffer = _framebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { Engine::GetScreenWidth() , Engine::GetScreenHeight() };

	VkClearValue clearValues[3]{}; // value 1 not used
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

	renderPassInfo.clearValueCount = 3;
	renderPassInfo.pClearValues = clearValues;	

	DBG_MARKER_BEGIN(_sceneCommandBuffers[nextCB], "Scene pass", vec4(0.79, 0.2, 0.0, 1.0));

	vkCmdBeginRenderPass(_sceneCommandBuffers[nextCB], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	vkCmdExecuteCommands(_sceneCommandBuffers[nextCB], (uint32_t)_secondarySceneCommandBuffers.Count(), *_secondarySceneCommandBuffers);
	vkCmdEndRenderPass(_sceneCommandBuffers[nextCB]);

	DBG_MARKER_END(_sceneCommandBuffers[nextCB]);

	if (vkEndCommandBuffer(_sceneCommandBuffers[nextCB]) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (scene) call failed");
		return false;
	}

	_currentSceneCB = nextCB;

	return true;
}

bool Renderer::_BuildGUICommandBuffer()
{
	int nextCB = (_currentGUICB + 1) % MAX_INFLIGHT_COMMAND_BUFFERS;

	if (_guiCommandBuffers[nextCB] != VK_NULL_HANDLE)
		vkFreeCommandBuffers(_device, _graphicsCommandPool, 1, &_guiCommandBuffers[nextCB]);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(_device, &allocInfo, &_guiCommandBuffers[nextCB]) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "Failed to allocate gui command buffer");
		return false;
	}
	DBG_SET_OBJECT_NAME((uint64_t)_guiCommandBuffers[nextCB], VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "GUI command buffer");

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(_guiCommandBuffers[nextCB], &beginInfo) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkBeginCommandBuffer (gui) call failed");
		return false;
	}

	DBG_MARKER_BEGIN(_guiCommandBuffers[nextCB], "GUI pass", vec4(0.66, 0.2, 0.82, 1.0));

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = RenderPassManager::GetRenderPass(RP_GUI);
	renderPassInfo.framebuffer = _guiFramebuffer;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = { Engine::GetScreenWidth(), Engine::GetScreenHeight() };
	renderPassInfo.clearValueCount = 0;
	renderPassInfo.pClearValues = nullptr;

	vkCmdBeginRenderPass(_guiCommandBuffers[nextCB], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
	vkCmdExecuteCommands(_guiCommandBuffers[nextCB], (uint32_t)_secondaryGuiCommandBuffers.Count(), *_secondaryGuiCommandBuffers);
	vkCmdEndRenderPass(_guiCommandBuffers[nextCB]);

	DBG_MARKER_END(_guiCommandBuffers[nextCB]);

	if (vkEndCommandBuffer(_guiCommandBuffers[nextCB]) != VK_SUCCESS)
	{
		Logger::Log(RENDERER_MODULE, LOG_CRITICAL, "vkEndCommandBuffer (gui) call failed");
		return false;
	}

	_currentGUICB = nextCB;

	return true;
}

void Renderer::_DestroyFramebuffers()
{
	if (_depthImageView != VK_NULL_HANDLE)
		vkDestroyImageView(_device, _depthImageView, _allocator);

	if (_stencilImageView != VK_NULL_HANDLE)
		vkDestroyImageView(_device, _stencilImageView, _allocator);

	delete _colorTarget;
	delete _depthTarget;
	delete _normalBrightTarget;
	delete _msaaColorTarget;
	delete _msaaNormalBrightTarget;

	if (_framebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(_device, _framebuffer, _allocator);
	if (_depthFramebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(_device, _depthFramebuffer, _allocator);
	if (_guiFramebuffer != VK_NULL_HANDLE)
		vkDestroyFramebuffer(_device, _guiFramebuffer, _allocator);
}

void Renderer::_DestroyDescriptorSets()
{
	//
}

void Renderer::_DestroyCommandBuffers()
{

}

Renderer::~Renderer()
{
	free(_lights);

	if (_device == VK_NULL_HANDLE)
		return;

	delete _buffer;
	delete _stagingBuffer;

	vkDestroySemaphore(_device, _imageAvailableSemaphore, _allocator);
	vkDestroySemaphore(_device, _depthFinishedSemaphore, _allocator);
	vkDestroySemaphore(_device, _cullingFinishedSemaphore, _allocator);
	vkDestroySemaphore(_device, _sceneFinishedSemaphore, _allocator);
	vkDestroySemaphore(_device, _renderFinishedSemaphore, _allocator);
	vkDestroySemaphore(_device, _aoFinishedSemaphore, _allocator);
	vkDestroySemaphore(_device, _aoReadySemaphore, _allocator);

	_DestroyCommandBuffers();

	_DestroyFramebuffers();

	if (_depthSampler != VK_NULL_HANDLE)
		vkDestroySampler(_device, _depthSampler, _allocator);

	if (_nearestSampler != VK_NULL_HANDLE)
		vkDestroySampler(_device, _nearestSampler, _allocator);

	if (_textureSampler != VK_NULL_HANDLE)
		vkDestroySampler(_device, _textureSampler, _allocator);

	if (_descriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(_device, _descriptorPool, _allocator);

	if (_computeDescriptorPool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(_device, _computeDescriptorPool, _allocator);

	PipelineManager::Release();
	RenderPassManager::Release();

	if (_graphicsCommandPool != VK_NULL_HANDLE)
		vkDestroyCommandPool(_device, _graphicsCommandPool, _allocator);
	/*if (_resetableGraphicsCommandPool != VK_NULL_HANDLE)
		vkDestroyCommandPool(_device, _resetableGraphicsCommandPool, _allocator);*/
	if (_computeCommandPool != VK_NULL_HANDLE)
		vkDestroyCommandPool(_device, _computeCommandPool, _allocator);

	delete _swapchain;

	if (_debugCB != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackEXT(_instance, _debugCB, _allocator);
	}

	if (_device != VK_NULL_HANDLE)
		vkDestroyDevice(_device, _allocator);
	_device = VK_NULL_HANDLE;

	if (_surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(_instance, _surface, _allocator);
	_surface = VK_NULL_HANDLE;

	if(_instance != VK_NULL_HANDLE)
		vkDestroyInstance(_instance, _allocator);
	_instance = VK_NULL_HANDLE;

	Logger::Log(RENDERER_MODULE, LOG_INFORMATION, "Released");

	/*vkDestroySampler(_device, _depthSampler, _allocator);

	vkDestroySemaphore(_device, _depthFinishedSemaphore, _allocator);
	vkDestroySemaphore(_device, _cullingFinishedSemaphore, _allocator);
	vkDestroySemaphore(_device, _sceneFinishedSemaphore, _allocator);

	vkFreeCommandBuffers(_device, _commandPool, 1, &_depthCommandBuffer);
	vkFreeCommandBuffers(_device, _commandPool, 1, &_cmdBuffer);
	vkFreeCommandBuffers(_device, _computeCommandPool, 1, &_cullingCommandBuffer);

	vkDestroyImage(_device, _depthImage, _allocator);
	vkFreeMemory(_device, _depthImageMemory, _allocator);
	vkDestroyImageView(_device, _depthImageView, _allocator);

	vkDestroyFramebuffer(_device, _framebuffer, _allocator);
	vkDestroyFramebuffer(_device, _depthFramebuffer, _allocator);
	vkDestroyFramebuffer(_device, _guiFramebuffer, _allocator);

	vkDestroyDescriptorPool(_device, _descriptorPool, _allocator);
	vkDestroyDescriptorPool(_device, _computeDescriptorPool, _allocator);
	vkDestroyDescriptorSetLayout(_device, _sceneDescriptorSetLayout, _allocator);
	vkDestroyDescriptorSetLayout(_device, _cullingDescriptorSetLayout, _allocator);

	delete _buffer;
	delete _sponza;
	delete _camera;*/
}
