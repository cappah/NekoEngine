/* NekoEngine
 *
 * Renderer.h
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

#pragma once

#include <unordered_map>
#include <algorithm>

#include <Engine/Defs.h>
#include <Engine/Engine.h>
#include <Runtime/Runtime.h>
#include <Renderer/Buffer.h>
#include <Renderer/Swapchain.h>

#define MAX_INFLIGHT_COMMAND_BUFFERS	6

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
	glm::mat4 Model;
	glm::mat4 ModelViewProjection;
	glm::mat4 Normal;
	// padding
	glm::mat4 p2;
} ObjectData;

typedef struct SCENE_DATA
{
	glm::mat4 View;
	glm::mat4 Projection;
	glm::vec4 Ambient;
	glm::vec4 CameraPosition;
	glm::ivec2 ScreenSize;
	int32_t LightCount;
	int32_t NumberOfTilesX;
	// padding
	glm::vec4 p0;
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

	VkImage GetRenderTargetImage();
	VkImage GetDepthStencilImage();
	VkImage GetNormalBrightImage();
	VkImage GetMSAANormalBrightImage();
	VkImageView GetRenderTargetImageView();
	VkImageView GetDepthStencilImageView();
	VkImageView GetNormalBrightImageView();
	VkImageView GetMSAANormalBrightImageView();

	VkImageView GetDepthImageView();
	VkImageView GetStencilImageView();

	VkSampler GetTextureSampler() { return _textureSampler; }
	VkSampler GetNearestSampler() { return _nearestSampler; }
	VkSampler GetDepthSampler() { return _depthSampler; }

	VkFramebuffer GetDepthFramebuffer() { return _depthFramebuffer; }
	VkFramebuffer GetDrawFramebuffer() { return _framebuffer; }
	VkFramebuffer GetGUIFramebuffer() { return _guiFramebuffer; }

	VkDescriptorSet GetSceneDescriptorSet() { return _sceneDescriptorSet; }

	void AddDepthCommandBuffer(VkCommandBuffer buffer) { _secondaryDepthCommandBuffers.Add(buffer); _rebuildDepthCB = true; }
	void AddSceneCommandBuffer(VkCommandBuffer buffer) { _secondarySceneCommandBuffers.Add(buffer); _rebuildSceneCB = true; }
	void AddGUICommandBuffer(VkCommandBuffer buffer) { _secondaryGuiCommandBuffers.Add(buffer); _rebuildGUICB = true; }

//	void RemoveDepthCommandBuffer(VkCommandBuffer buffer)
//	{ _secondaryDepthCommandBuffers.erase(std::remove(_secondaryDepthCommandBuffers.begin(), _secondaryDepthCommandBuffers.end(), buffer), _secondaryDepthCommandBuffers.end()); _rebuildDepthCB = true; }
//	void RemoveSceneCommandBuffer(VkCommandBuffer buffer)
//	{ _secondarySceneCommandBuffers.erase(std::remove(_secondarySceneCommandBuffers.begin(), _secondarySceneCommandBuffers.end(), buffer), _secondarySceneCommandBuffers.end()); _rebuildSceneCB = true; }
//	void RemoveGUICommandBuffer(VkCommandBuffer buffer)
//	{ _secondaryGuiCommandBuffers.erase(std::remove(_secondaryGuiCommandBuffers.begin(), _secondaryGuiCommandBuffers.end(), buffer), _secondaryGuiCommandBuffers.end()); _rebuildGUICB = true; }

	void ResetDepthCommandBuffers() { _secondaryDepthCommandBuffers.Clear(); }
	void ResetSceneCommandBuffers() { _secondarySceneCommandBuffers.Clear(); }
	void ResetGUICommandBuffers() { _secondaryGuiCommandBuffers.Clear(); }

	Buffer *GetStagingBuffer(VkDeviceSize size);
	void FreeStagingBuffer(Buffer *buffer);

	Light *AllocLight();
	Light *GetLight(uint32_t id);
	void FreeLight(Light *light);
	int32_t GetNumLights() { return _sceneData.LightCount; }

	void SetAmbientColor(float r, float g, float b, float intensity) { _sceneData.Ambient = glm::vec4(r, g, b, intensity); };

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

	class Texture *_colorTarget, *_depthTarget, *_normalBrightTarget;
	class Texture *_msaaColorTarget, *_msaaNormalBrightTarget;

	VkDescriptorPool _descriptorPool, _computeDescriptorPool;
	VkDescriptorSetLayout _sceneDescriptorSetLayout, _cullingDescriptorSetLayout, _loadDescriptorSetLayout;
	VkDescriptorSet _sceneDescriptorSet, _cullingDescriptorSet, _loadDescriptorSet;

	VkCommandBuffer _cullingCommandBuffer, _loadingCommandBuffer;
	NArray<VkCommandBuffer> _presentCommandBuffers;
	NArrayTS<VkCommandBuffer> _secondaryDepthCommandBuffers, _secondarySceneCommandBuffers, _secondaryGuiCommandBuffers;

	VkCommandBuffer _depthCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS], _sceneCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS], _guiCommandBuffers[MAX_INFLIGHT_COMMAND_BUFFERS];

	bool _rebuildDepthCB, _rebuildSceneCB, _rebuildGUICB;
	int _currentDepthCB, _currentSceneCB, _currentGUICB;

	VkSemaphore _imageAvailableSemaphore,
		_depthFinishedSemaphore,
		_cullingFinishedSemaphore,
		_sceneFinishedSemaphore,
		_renderFinishedSemaphore,
		_aoReadySemaphore,
		_aoFinishedSemaphore;

	Buffer *_buffer, *_stagingBuffer;
	SceneData _sceneData;

	uint32_t _nFrames;

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

	void _DestroyFramebuffers();
	void _DestroyDescriptorSets();
	void _DestroyCommandBuffers();

	void _Submit(uint32_t imageIndex);
	void _SubmitAsync(uint32_t imageIndex);
};
