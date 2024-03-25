#include "EWEngine/Systems/Ocean/Ocean.h"
#include "EWEngine/Data/TransformInclude.h"

namespace EWE {
	namespace Ocean {
		Ocean::Ocean(EWEDevice& device) :
			device{ device }
		{
			
		}
		Ocean::~Ocean() {

		}
		void Ocean::InitialiseCascades(VkCommandBuffer cmdBuf) {
			
		}

		void Ocean::ComputeUpdate(std::array<VkCommandBuffer, 5> oceanBuffers, float dt) {

		}

		void Ocean::initializeDSLs() {
			
		}

		void Ocean::createRenderPipeline(VkPipelineRenderingCreateInfo const& pipeRenderInfo) {
			
		}

		void Ocean::createBuffers() {

			ocean_time_buffer = std::make_unique<EWEBuffer>(
				device, sizeof(Ocean_Time_Struct), 1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			ocean_time_buffer->map();

			spectrum_parameter_buffer = std::make_unique<EWEBuffer>(
				device, sizeof(Spectrum_Settings) * 2, 1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			spectrum_parameter_buffer->map();
		}

		void Ocean::RenderUpdate(FrameInfo frameInfo) {

		}

		void Ocean::getGaussNoise() {

		}
	}//ocean namespace
}