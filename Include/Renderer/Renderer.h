/* NekoEngine
 *
 * Renderer.h
 * Author: Alexandru Naiman
 *
 * Forward+ Renderer
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2017, Alexandru Naiman
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

#pragma once

#include <unordered_map>
#include <algorithm>

#include <Engine/Defs.h>
#include <Engine/Engine.h>
#include <Runtime/Runtime.h>
#include <Renderer/Buffer.h>
#include <Renderer/Swapchain.h>

#define MAX_INFLIGHT_COMMAND_BUFFERS	12
#define TEMPORARY_BUFFER_SIZE			2097152		// 2 MB

#ifdef ENGINE_INTERNAL
struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;
	int computeFamily = -1;

	bool complete() { return graphicsFamily > 0 && presentFamily > 0 && computeFamily > 0; }
};
#endif

enum LightType : uint32_t
{
	LT_Directional = 0,
	LT_Point = 1,
	LT_Spot = 2
};

typedef struct LIGHT
{
	glm::vec4 position;
	glm::vec4 direction;
	glm::vec4 color;
	glm::vec4 data;
} Light;

typedef struct OBJECT_DATA
{
	glm::mat4 model;
	glm::mat4 modelViewProjection;
	glm::mat4 normal;
	uint32_t objectId;
	// padding
	glm::vec3 p0;
	glm::vec4 p1, p2, p3;
} ObjectData;

typedef struct SCENE_DATA
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::vec4 ambient;
	glm::vec4 cameraPosition;
	glm::ivec2 screenSize;
	int32_t lightCount;
	int32_t numberOfTilesX;
	int32_t numSamples;
	float gamma;
	// padding
	glm::vec2 p0;
	glm::mat4 p1;
} SceneData;

class Renderer
{
public:
	static Renderer *GetInstance();
	static void Release();

	const char *GetAPIName() { return "Vulkan"; }
	const char *GetAPIVersion();
	const char *GetDeviceName();

	virtual int Initialize(PlatformWindowType window, bool enableValidation = false, bool debug = false);

	virtual void Update(double deltaTime);
	virtual void Draw();

	virtual void ScreenResized();

	void WaitIdle() { if (_device != VK_NULL_HANDLE) vkDeviceWaitIdle(_device); }

	VkCommandBuffer CreateMeshCommandBuffer();
	void FreeMeshCommandBuffer(VkCommandBuffer buffer);

	VkDescriptorPool CreateMeshDescriptorPool();
	VkDescriptorPool CreateAnimatedMeshDescriptorPool();
	void FreeMeshDescriptorPool(VkDescriptorPool pool);

	Buffer *GetSceneDataBuffer() { return _buffer; }
	class Texture *GetBlankTexture() { return _blankTexture; }
	VkDescriptorSet GetBlankTextureDescriptorSet() { return _blankTextureDescriptorSet; }

	VkImage GetRenderTargetImage();
	VkImage GetDepthStencilImage();
	VkImage GetNormalImage();
	VkImage GetMSAANormalImage();
	VkImage GetBrightnessImage();
	VkImage GetMSAABrightnessImage();
	VkImageView GetRenderTargetImageView();
	VkImageView GetDepthStencilImageView();
	VkImageView GetNormalImageView();
	VkImageView GetMSAANormalImageView();
	VkImageView GetBrightnessImageView();
	VkImageView GetMSAABrightnessImageView();

	VkImageView GetDepthImageView();
	VkImageView GetStencilImageView();

	VkSampler GetTextureSampler() { return _textureSampler; }
	VkSampler GetNearestSampler() { return _nearestSampler; }
	VkSampler GetDepthSampler() { return _depthSampler; }

	VkFramebuffer GetDepthFramebuffer() { return _depthFramebuffer; }
	VkFramebuffer GetDrawFramebuffer() { return _framebuffer; }
	VkFramebuffer GetGUIFramebuffer() { return _guiFramebuffer; }

	VkDescriptorSet GetSceneDescriptorSet() { return _sceneDescriptorSet; }

	void AddDepthCommandBuffer(VkCommandBuffer buffer) { _secondaryDepthCommandBuffers.Add(buffer); }
	void AddSceneCommandBuffer(VkCommandBuffer buffer) { _secondarySceneCommandBuffers.Add(buffer); }
	void AddGUICommandBuffer(VkCommandBuffer buffer) { _secondaryGuiCommandBuffers.Add(buffer); }

	void AddParticleDrawCommandBuffer(VkCommandBuffer buffer) { _particleDrawCommandBuffers.Add(buffer); }
	void RemoveParticleDrawCommandBuffer(VkCommandBuffer buffer) { size_t id = _particleDrawCommandBuffers.Find(buffer); if(id != NArray<VkCommandBuffer>::NotFound) _particleDrawCommandBuffers.Remove(id); }

	void AddComputeCommandBuffer(VkCommandBuffer buffer) { _computeCommandBuffers.Add(buffer); }
	void RemoveComputeCommandBuffer(VkCommandBuffer buffer) { size_t id = _computeCommandBuffers.Find(buffer); if(id != NArray<VkCommandBuffer>::NotFound) _computeCommandBuffers.Remove(id); }

	void ResetComputeCommandBuffers() { _computeCommandBuffers.Clear(false); _computeCommandBuffers.Add(_cullingCommandBuffer); }
	void ResetDepthCommandBuffers() { _secondaryDepthCommandBuffers.Clear(false); }
	void ResetSceneCommandBuffers() { _secondarySceneCommandBuffers.Clear(false); }
	void ResetGUICommandBuffers() { _secondaryGuiCommandBuffers.Clear(false); }

	void DrawBounds(const NBounds &bounds) { _drawBoundsList.Add(&bounds); }

	// WARNING: THE RETURNED BUFFER MUST NOT BE FREED. IT WILL BE FREED WHEN THE NEXT FRAME STARTS
	Buffer *GetTemporaryBuffer(VkDeviceSize size);

	Buffer *GetStagingBuffer(VkDeviceSize size);
	void FreeStagingBuffer(Buffer *buffer);

	int32_t AllocLight();
	Light *GetLight(int32_t id);
	void FreeLight(int32_t id);
	int32_t GetNumLights() { return _sceneData.lightCount; }

	void SetAmbientColor(float r, float g, float b, float intensity) { _sceneData.ambient = glm::vec4(r, g, b, intensity); };

protected:
	Renderer();

	virtual ~Renderer();

private:
	// Vulkan
	VkInstance _instance;
	VkDevice _device;
	VkQueue _graphicsQueue, _computeQueue, _transferQueue, _presentQueue;
	VkPhysicalDevice _physicalDevice;
	VkCommandPool _graphicsCommandPool, _computeCommandPool;
	VkSurfaceKHR _surface;
	VkAllocationCallbacks *_allocator;
	VkDebugReportCallbackEXT _debugCB;
	SwapchainInfo _swapchainInfo;
	Swapchain *_swapchain;

	VkFramebuffer _framebuffer, _depthFramebuffer, _guiFramebuffer;
	VkSampler _textureSampler, _nearestSampler, _depthSampler;

	class Texture *_colorTarget, *_depthTarget, *_normalTarget, *_brightnessTarget;
	class Texture *_msaaColorTarget, *_msaaNormalTarget, *_msaaBrightnessTarget;

	VkDescriptorPool _descriptorPool, _computeDescriptorPool;
	VkDescriptorSetLayout _sceneDescriptorSetLayout, _cullingDescriptorSetLayout, _loadDescriptorSetLayout;
	VkDescriptorSet _sceneDescriptorSet, _cullingDescriptorSet, _loadDescriptorSet, _blankTextureDescriptorSet;

	VkCommandBuffer _cullingCommandBuffer, _loadingCommandBuffer;
	NArray<VkCommandBuffer> _presentCommandBuffers, _computeCommandBuffers;
	NArray<VkCommandBuffer> _secondaryDepthCommandBuffers, _secondarySceneCommandBuffers, _secondaryGuiCommandBuffers;

	VkCommandBuffer _shadowCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS], _depthCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS], _sceneCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS],
		_guiCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS], _updateCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS], _drawBoundsCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS];
	Buffer *_temporaryBuffer, *_tempBuffers[MAX_INFLIGHT_COMMAND_BUFFERS];
	VkDeviceSize _tempBufferOffsets[MAX_INFLIGHT_COMMAND_BUFFERS];
	NArray<Buffer *> _allocatedBuffers[MAX_INFLIGHT_COMMAND_BUFFERS];

	NArray<VkCommandBuffer> _particleDrawCommandBuffers;

	int _currentBufferIndex;

	VkSemaphore _imageAvailableSemaphore,
		_depthFinishedSemaphore,
		_cullingFinishedSemaphore,
		_sceneFinishedSemaphore,
		_renderFinishedSemaphore,
		_aoReadySemaphore,
		_aoFinishedSemaphore;

	Buffer *_buffer, *_stagingBuffer;
	SceneData _sceneData;
	class Texture *_blankTexture;

	uint32_t _nFrames;

	NArray<const NBounds *> _drawBoundsList;

	virtual void _RecreateSwapchain();

	// ----

	bool _CheckDeviceExtension(const char *name);

	bool _CreateInstance(bool debug);
	bool _SetupDebugCallback();
	bool _CreateDevice(bool enableValidation, bool debug);
	bool _CreateCommandPools();
	bool _CreateFramebuffers();
	bool _CreateDescriptorSets();
	bool _CreateBuffer();
	bool _CreateCommandBuffers();
	bool _CreateSemaphores();

	bool _BuildDepthCommandBuffer();
	bool _BuildSceneCommandBuffer();
	bool _BuildGUICommandBuffer();
	bool _BuildBoundsDrawCommandBuffer();

	void _UpdateDescriptorSets();

	void _DestroyFramebuffers();
	void _DestroyCommandBuffers();

	void _Submit(uint32_t imageIndex);
	void _SubmitAsync(uint32_t imageIndex);
};
