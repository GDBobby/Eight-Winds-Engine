#pragma once

//#include "DescriptorHandler.h"
#include "Device.hpp"
#include "Descriptors.h"
#include "DescriptorHandler.h"
#include "EWEngine/Data/ShaderBuilder.h"
#include "EWEngine/Data/EngineDataTypes.h"

#include <glm/glm.hpp>
#include <map>



#define PIPELINE_DERIVATIVES 0 //pipeline derivatives are not currently recommended by hardware vendors
/* https://developer.nvidia.com/blog/vulkan-dos-donts/ */

//#define DYNAMIC_PIPE_LAYOUT_COUNT 24 //MAX_TEXTURE_COUNT * 4 //defined in descriptorhandler.h

namespace EWE {


	enum Pipeline_Enum : PipelineID {
		Pipe_pointLight,
		Pipe_textured,
		Pipe_alpha,
		//Pipe_material,
		Pipe_2d,
		Pipe_NineUI,
		//Pipe_fbx,
		Pipe_skybox,
		Pipe_grid,
		//Pipe_bobTrans,
		Pipe_sprite,
		Pipe_boneWeapon,
		Pipe_lightning,
		Pipe_spikyBall,
		Pipe_visualEffect,
		Pipe_grass,
		Pipe_loading,

		Pipe_MAX_COUNT,
	};
	enum PipeLayout_Enum : uint32_t { //uint32_t is the same as PipelineID, but using uint32_t to avoid confusion
		PL_pointLight,
		PL_lightning,
		PL_spikyBall,
		PL_grass,
		PL_textured,
		//PL_material,
		PL_2d,
		PL_boned,
		//PL_fbx,
		PL_skybox,
		PL_sprite,
		//PL_boneWeapon,
		PL_visualEffect,
		//PL_metalRough,
		PL_loading,

		PL_nineUI,

		PL_MAX_COUNT,
	};

	//typedef uint8_t MaterialFlags; this in engine/data/enginedatatypes.h

	namespace Pipeline_Helper_Functions {
		void createShaderModule(EWEDevice& device, std::string const& file_path, VkShaderModule* shaderModule);
		std::vector<char> readFile(const std::string& filepath);

		void createShaderModule(EWEDevice& device, const std::vector<uint32_t>& data, VkShaderModule* shaderModule);
		void createShaderModule(EWEDevice& device, const char* data, size_t dataSize, VkShaderModule* shaderModule);

		template <typename T>
		void createShaderModule(EWEDevice& device, const std::vector<T>& data, VkShaderModule* shaderModule);
	}

	class EWE_Compute_Pipeline {
	public:
		VkPipelineLayout pipe_layout;
		VkPipeline pipeline;
		//could do some shit to prevent compute path being reused, idk

		static EWE_Compute_Pipeline createPipeline(EWEDevice& device, std::vector<VkDescriptorSetLayout> computeDSL, std::string compute_path);
		static EWE_Compute_Pipeline createPipeline(EWEDevice& device, VkPipelineLayout pipe_layout, std::string compute_path);
		void bind(VkCommandBuffer cmdBuf) {
			vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
		}
	private:
		VkShaderModule shader;
	};

	class EWEPipeline {
	public:


		struct PipelineConfigInfo {
			PipelineConfigInfo() = default;
			PipelineConfigInfo(const PipelineConfigInfo&) = delete;
			PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

			VkViewport viewport;
			VkRect2D scissor;

			std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

			VkPipelineViewportStateCreateInfo viewportInfo{};
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
			VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
			VkPipelineMultisampleStateCreateInfo multisampleInfo{};
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
			VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

			std::vector<VkDynamicState> dynamicStateEnables{};
			VkPipelineDynamicStateCreateInfo dynamicStateInfo{};

			VkPipelineLayout pipelineLayout = nullptr;
			//VkPipelineRenderingCreateInfo const& pipeRenderInfo = nullptr;
			VkPipelineRenderingCreateInfo pipelineRenderingInfo;
			uint32_t subpass = 0;
		#if PIPELINE_DERIVATIVES
			int32_t basePipelineIndex = -1;
			EWEPipeline* basePipelineHandle = nullptr;
			VkPipelineCreateFlags flags = 0;
		#endif

			VkPipelineCache cache = VK_NULL_HANDLE;


			static VkPipelineRenderingCreateInfo* pipelineRenderingInfoStatic;
		};

		EWEPipeline(EWEDevice& device, std::string const& vertFilepath, std::string const& fragFilepath, PipelineConfigInfo const& configInfo);
		EWEPipeline(EWEDevice& device, VkShaderModule vertShaderModu, VkShaderModule fragShaderModu, PipelineConfigInfo const& configInfo);
		EWEPipeline(EWEDevice& device, std::string const& vertFilePath, MaterialFlags flags, PipelineConfigInfo const& configInfo, bool hasBones);
		EWEPipeline(EWEDevice& device, uint16_t boneCount, MaterialFlags flags, PipelineConfigInfo const& configInfo);

		~EWEPipeline();

		EWEPipeline(EWEPipeline const&) = delete;
		EWEPipeline& operator=(EWEPipeline const&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void enable2DConfig(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);

		static void cleanShaderModules(EWEDevice& device) {
			for (auto iter = shaderModuleMap.begin(); iter != shaderModuleMap.end(); iter++) {
				vkDestroyShaderModule(device.device(), iter->second, nullptr);
			}
			shaderModuleMap.clear();
		}


	private:
		//static materials
		static std::map<std::string, VkShaderModule> shaderModuleMap;

		EWEDevice& eweDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		void createGraphicsPipeline(PipelineConfigInfo const& configInfo);

	};





	/*
	class PipelineManager {
		PipelineManager() {}
	public:
		//better to pass in references every time i want something, or make an object then have the object store the reference, with static accessors? singleton?
		//is static reference even a thing?
		static std::map<PipeLayout_Enum, VkPipelineLayout> pipeLayouts;
		static std::map<Pipeline_Enum, std::unique_ptr<EWEPipeline>> pipelines;
		static std::map<MaterialFlags, std::unique_ptr<EWEPipeline>> dynamicMaterialPipeline;

#ifdef _DEBUG
		static std::vector<MaterialFlags> dynamicBonePipeTracker;
		static std::vector<std::pair<uint16_t, MaterialFlags>> dynamicInstancedPipeTracker;
#endif
		static VkPipelineLayout dynamicMaterialPipeLayout[DYNAMIC_PIPE_LAYOUT_COUNT];

		static VkShaderModule loadingVertShaderModule;
		static VkShaderModule loadingFragShaderModule;
		static std::unique_ptr<EWEPipeline> loadingPipeline;

		static void createLoadingPipeline(EWEDevice& device, VkPipelineRenderingCreateInfo const& pipeRenderInfo);

		static VkPipelineLayout getPipelineLayout(PipeLayout_Enum ple, EWEDevice& eweDevice);

		static void initDynamicPipeLayout(uint16_t dynamicPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced, EWEDevice& device);
		static void updateMaterialPipe(MaterialFlags flags, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device);

		//this should ALWAYS have bones
		static std::unique_ptr<EWEPipeline> createInstancedRemote(MaterialFlags flags, uint16_t boneCount, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device);
		static std::unique_ptr<EWEPipeline> createBoneRemote(MaterialFlags flags, VkPipelineRenderingCreateInfo const& pipeRenderInfo, EWEDevice& device);

		static void initStaticVariables() {
			for (int i = 0; i < DYNAMIC_PIPE_LAYOUT_COUNT; i++) {
				dynamicMaterialPipeLayout[i] = VK_NULL_HANDLE;
			}
		}
		static void cleanupStaticVariables(EWEDevice& device);


		static void initPipelines(VkPipelineRenderingCreateInfo const& pipeRenderInfo, Pipeline_Enum pipesNeeded, EWEDevice& eweDevice);


	private:
		//uint8_t daddyPipeKey = 0; //this is for derivatives
		//uint8_t boneDaddyPipeKey = 0;

		static VkPipelineCache materialPipelineCache;
		static VkPipelineCache boneMaterialPipelineCache;
		static VkPipelineCache instanceMaterialPipelineCache;
	};
	*/


}