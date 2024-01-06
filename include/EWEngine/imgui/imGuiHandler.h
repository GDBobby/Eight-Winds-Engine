
#pragma once

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "EWEngine/graphics/EWE_Device.hpp"
#include "EWEngine/graphics/EWE_Descriptors.h"

#include "EWEngine/graphics/EWE_Texture.h"


#include <stdexcept>

//#define IMGUI_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif


#ifdef IMGUI_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData) {
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}
#endif // IMGUI_VULKAN_DEBUG_REPORT

namespace EWE {
	class ImGUIHandler {
		static void check_vk_result(VkResult err) {
			if (err == 0) {
				return;
			}
			fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
			if (err < 0) {
				abort();
			}
		}
	public:
		ImGUIHandler(GLFWwindow* window, EWEDevice& eweDevice, uint32_t imageCount, VkPipelineRenderingCreateInfo const& pipeRenderInfo);
        ~ImGUIHandler();

		void beginRender();
		void endRender(VkCommandBuffer cmdBuf);

		void rebuild() {
			//ImGui_ImplVulkanH_CreateWindow(device.getInstance(), device.getPhysicalDevice(), device.device(), &g_MainWindowData, g_QueueFamily, nullptr, g_SwapChainResizeWidth, g_SwapChainResizeHeight, g_MinImageCount);
		}
        void addTexture(TextureID eweTexID) {

        }

	private:
		//float tempFloat{0.f};
		void createDescriptorPool();
		/*
		void creatingARenderPass(VkFormat imageFormat) {
			VkAttachmentDescription attachment = {};
			attachment.format = imageFormat;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference color_attachment = {};
			color_attachment.attachment = 0;
			color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 1;
			info.pAttachments = &attachment;
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 1;
			info.pDependencies = &dependency;
			if (vkCreateRenderPass(device.device(), &info, nullptr, &imGuiRenderPass) != VK_SUCCESS) {
				throw std::runtime_error("Could not create Dear ImGui's render pass");
			}
		}
		*/
		EWEDevice& device;
        DescriptorPool_ID imguiPoolID;
		VkRenderPass imGuiRenderPass;

	};
}
