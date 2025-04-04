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
			skybox,
			grid,
			loading,
			lightning,

			ENGINE_MAX_COUNT,
		};
	} //namespace Pipe
	//namespace PL{
	//enum PipeLayout_Enum : uint32_t { //uint32_t is the same as PipelineID, but using uint32_t to avoid confusion
	//	pointLight,
	//	lightning,
	//	spikyBall,
	//	grass,
	//	textured,
	//	//PL_material,
	//	twod,
	//	boned,
	//	//PL_fbx,
	//	skybox,
	//	sprite,
	//	//PL_boneWeapon,
	//	visualEffect,
	//	//PL_metalRough,
	//	loading,

	//	nineUI,

	//	MAX_COUNT,
	//};
	//} //namespace PL

	//typedef uint8_t MaterialFlags; this in engine/data/enginedatatypes.h

	namespace Pipeline_Helper_Functions {
		void CreateShaderModule(std::string const& file_path, VkShaderModule* shaderModule);
		std::vector<char> ReadFile(const std::string& filepath);

		void CreateShaderModule(const std::vector<uint32_t>& data, VkShaderModule* shaderModule);
		void CreateShaderModule(const char* data, std::size_t dataSize, VkShaderModule* shaderModule);

		template <typename T>
		void CreateShaderModule(const std::vector<T>& data, VkShaderModule* shaderModule);
	}

	class EWE_Compute_Pipeline {
	public:
		VkPipelineLayout pipe_layout;
		VkPipeline pipeline;

		static EWE_Compute_Pipeline CreatePipeline(std::vector<VkDescriptorSetLayout> computeDSL, std::string compute_path);
		static EWE_Compute_Pipeline CreatePipeline(VkPipelineLayout pipe_layout, std::string compute_path);
		void Bind(CommandBuffer cmdBuf) {
			EWE_VK(vkCmdBindPipeline, cmdBuf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
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

			void AddGeomShaderModule(std::string const& geomFilepath);

			VkViewport viewport;
			VkRect2D scissor;

			VkShaderModule geomShaderModule{ VK_NULL_HANDLE };

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

		void Bind();
		static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		static void Enable2DConfig(PipelineConfigInfo& configInfo);
		static void EnableAlphaBlending(PipelineConfigInfo& configInfo);

		static void CleanShaderModules() {
			for (auto iter = shaderModuleMap.begin(); iter != shaderModuleMap.end(); iter++) {
				EWE_VK(vkDestroyShaderModule, VK::Object->vkDevice, iter->second, nullptr);
			}
			shaderModuleMap.clear();
		}
#if DEBUG_NAMING
		void SetDebugName(std::string const& name);
#endif

	private:
		//static materials
		static std::map<std::string, VkShaderModule> shaderModuleMap;

		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		void CreateGraphicsPipeline(PipelineConfigInfo const& configInfo);

	};
}