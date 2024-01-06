
#include <EWEngine/imgui/imGuiHandler.h>


namespace EWE {
	ImGUIHandler::ImGUIHandler(GLFWwindow* window, EWEDevice& device, uint32_t imageCount, VkPipelineRenderingCreateInfo const& pipeRenderInfo) : device{ device } {
		//printf("imgui handler constructor \n");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();

		//descriptorPool
		createDescriptorPool();

		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = device.getInstance();
		init_info.PhysicalDevice = device.getPhysicalDevice();
		init_info.Device = device.device();
		init_info.QueueFamily = device.getGraphicsIndex();
		init_info.Queue = device.graphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.poolID = imguiPoolID;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = imageCount;
		init_info.ImageCount = imageCount;
		init_info.CheckVkResultFn = check_vk_result;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		

		//if an issue, address this first
		init_info.UseDynamicRendering = true;
		init_info.pipeRenderInfo = pipeRenderInfo;

		ImGui_ImplVulkan_Init(&init_info, VK_NULL_HANDLE);

		//uploadFonts();
		//printf("end of imgui constructor \n");
	}
	ImGUIHandler::~ImGUIHandler() {
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		EWEDescriptorPool::DestructPool(imguiPoolID);
		printf("imguihandler deconstructed \n");
	}

	void ImGUIHandler::beginRender() {
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	void ImGUIHandler::endRender(VkCommandBuffer cmdBuf) {
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuf);
	}


	void ImGUIHandler::createDescriptorPool() {
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.maxSets = 0;
		for (int i = 0; i < pool_info.poolSizeCount; i++) {
			pool_info.maxSets += pool_sizes[i].descriptorCount;
		}
		//pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;
		EWEDescriptorPool::AddPool(DescriptorPool_imgui, device, pool_info);
	}
}
