#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <unordered_map>

enum RenderPassId : uint8_t
{
	RP_Depth,
	RP_Graphics,
	RP_GUI,
	RP_PostProcess,
	RP_SSAO
};

class RenderPassManager
{
public:

	static bool Initialize();

	static VkRenderPass GetRenderPass(uint8_t name) { return _renderPasses[name]; }

	static void Release();

private:
	static std::unordered_map<uint8_t, VkRenderPass> _renderPasses;

	static bool _CreateRenderPass();
	static bool _CreateDepthRenderPass();
	static bool _CreateGUIRenderPass();
	static bool _CreateSSAORenderPass();
};