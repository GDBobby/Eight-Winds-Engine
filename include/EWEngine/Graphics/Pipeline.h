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

	namespace Pipe {
		enum Pipeline_Enum : PipelineID {
			pointLight,
			textured,
			alpha,
			//material,
			twod,
			NineUI,
			//fbx,
			skybox,
			grid,
			//bobTrans,
			sprite,
			boneWeapon,
			lightning,
			spikyBall,
			visualEffect,
			grass,
			loading,

			ENGINE_MAX_COUNT,
		};
	} //namespace Pipe
	namespace PL{
	enum PipeLayout_Enum : uint32_t { //uint32_t is the same as PipelineID, but using uint32_t to avoid confusion
		pointLight,
		lightning,
		spikyBall,
		grass,
		textured,
		//PL_material,
		twod,
		boned,
		//PL_fbx,
		skybox,
		sprite,
		//PL_boneWeapon,
		visualEffect,
		//PL_metalRough,
		loading,

		nineUI,

		MAX_COUNT,
	};
	} //namespace PL

	//typedef uint8_t MaterialFlags; this in engine/data/enginedatatypes.h

	namespace Pipeline_Helper_Functions {
		void createShaderModule(std::string const& file_path, VkShaderModule* shaderModule);
		std::vector<char> readFile(const std::string& filepath);

		void createShaderModule(const std::vector<uint32_t>& data, VkShaderModule* shaderModule);
		void createShaderModule(const char* data, std::size_t dataSize, VkShaderModule* shaderModule);

		template <typename T>
		void createShaderModule(const std::vector<T>& data, VkShaderModule* shaderModule);
	}

	class EWE_Compute_Pipeline {
	public:
		VkPipelineLayout pipe_layout;
		VkPipeline pipeline;
		//could do some shit to prevent compute path being reused, idk

		static EWE_Compute_Pipeline createPipeline(std::vector<VkDescriptorSetLayout> computeDSL, std::string compute_path);
		static EWE_Compute_Pipeline createPipeline(VkPipelineLayout pipe_layout, std::string compute_path);
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

		EWEPipeline(std::string const& vertFilepath, std::string const& fragFilepath, PipelineConfigInfo const& configInfo);
		EWEPipeline(VkShaderModule vertShaderModu, VkShaderModule fragShaderModu, PipelineConfigInfo const& configInfo);
		EWEPipeline(std::string const& vertFilePath, MaterialFlags flags, PipelineConfigInfo const& configInfo, bool hasBones);
		EWEPipeline(uint16_t boneCount, MaterialFlags flags, PipelineConfigInfo const& configInfo);

		~EWEPipeline();

		EWEPipeline(EWEPipeline const&) = delete;
		EWEPipeline& operator=(EWEPipeline const&) = delete;

		void bind(VkCommandBuffer commandBuffer);
		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void enable2DConfig(PipelineConfigInfo& configInfo);
		static void enableAlphaBlending(PipelineConfigInfo& configInfo);

		static void cleanShaderModules() {
			VkDevice const& vkDevice = EWEDevice::GetVkDevice();
			for (auto iter = shaderModuleMap.begin(); iter != shaderModuleMap.end(); iter++) {
				vkDestroyShaderModule(vkDevice, iter->second, nullptr);
			}
			shaderModuleMap.clear();
		}


	private:
		//static materials
		static std::map<std::string, VkShaderModule> shaderModuleMap;

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
		static std::map<Pipeline_Enum, EWEPipeline*> pipelines;
		static std::map<MaterialFlags, EWEPipeline*> dynamicMaterialPipeline;

#ifdef _DEBUG
		static std::vector<MaterialFlags> dynamicBonePipeTracker;
		static std::vector<std::pair<uint16_t, MaterialFlags>> dynamicInstancedPipeTracker;
#endif
		static VkPipelineLayout dynamicMaterialPipeLayout[DYNAMIC_PIPE_LAYOUT_COUNT];

		static VkShaderModule loadingVertShaderModule;
		static VkShaderModule loadingFragShaderModule;
		static EWEPipeline* loadingPipeline;

		static void createLoadingPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo);

		static VkPipelineLayout getPipelineLayout(PipeLayout_Enum ple);

		static void initDynamicPipeLayout(uint16_t dynamicPipeLayoutIndex, uint8_t textureCount, bool hasBones, bool instanced);
		static void updateMaterialPipe(MaterialFlags flags, VkPipelineRenderingCreateInfo const& pipeRenderInfo);

		//this should ALWAYS have bones
		static EWEPipeline* createInstancedRemote(MaterialFlags flags, uint16_t boneCount, VkPipelineRenderingCreateInfo const& pipeRenderInfo);
		static EWEPipeline* createBoneRemote(MaterialFlags flags, VkPipelineRenderingCreateInfo const& pipeRenderInfo);

		static void initStaticVariables() {
			for (int i = 0; i < DYNAMIC_PIPE_LAYOUT_COUNT; i++) {
				dynamicMaterialPipeLayout[i] = VK_NULL_HANDLE;
			}
		}
		static void cleanupStaticVariables();


		static void initPipelines(VkPipelineRenderingCreateInfo const& pipeRenderInfo, Pipeline_Enum pipesNeeded);


	private:
		//uint8_t daddyPipeKey = 0; //this is for derivatives
		//uint8_t boneDaddyPipeKey = 0;

		static VkPipelineCache materialPipelineCache;
		static VkPipelineCache boneMaterialPipelineCache;
		static VkPipelineCache instanceMaterialPipelineCache;
	};
	*/


}