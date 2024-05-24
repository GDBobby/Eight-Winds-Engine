#include "EWEngine/Graphics/VkDebugDeviceLost.h"

#include <cstdio>
#include <cassert>
#include <set>
#include <string>

//there's a license in there, not sure if i need to copy it or not
//https://github.com/ConfettiFX/The-Forge/blob/23483a282ddc8a917f8b292b0250dec122eab6a9/Common_3/Graphics/Vulkan/Vulkan.cpp#L741
//im pretty much copy and pasting with minor adjustments

PFN_vkGetDeviceFaultInfoEXT func;
PFN_vkGetQueueCheckpointDataNV vkGetQueueCheckpointDataNVX;
PFN_vkCmdSetCheckpointNV vkCmdSetCheckpointNVX;
PFN_vkCmdWriteBufferMarkerAMD vkCmdWriteBufferMarkerAMDX;


namespace EWE::VKDEBUG {
	VkDevice device;

	DeviceLostDebugStructure::DeviceLostDebugStructure() {

	}
	void DeviceLostDebugStructure::Initialize(VkDevice vkDevice) {
		device = vkDevice;
	}

	bool DeviceLostDebugStructure::GetVendorDebugExtension(VkPhysicalDevice physDevice) {
		//double checking device extensions, which isn't great, shouldnt be completely awful tho
		uint32_t extensionCount;
		VkResult result = vkEnumerateDeviceExtensionProperties(physDevice, nullptr, &extensionCount, nullptr);
		if (result != VK_SUCCESS) {
			printf("VK_ERROR : %s(%d) : %s - %d \n", __FILE__, __LINE__, __FUNCTION__, result);
		}

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		result = vkEnumerateDeviceExtensionProperties(
			physDevice,
			nullptr,
			&extensionCount,
			availableExtensions.data()
		);
		if (result != VK_SUCCESS) {
			printf("VK_ERROR : %s(%d) : %s - %d \n", __FILE__, __LINE__, __FUNCTION__, result);
		}

		std::set<std::string> requestedExtensions(debugExtensions.begin(), debugExtensions.end());

		printf("available extensions: \n");
		for (const auto& extension : availableExtensions) {
			printf("\t%s\n", extension.extensionName);
			requestedExtensions.erase(extension.extensionName);
		}
		const uint32_t inSize = debugExtensions.size();
		//remainign extensions, not supported
		for (uint8_t i = 0; i < debugExtensions.size(); i++) {
			const std::string tempString{ debugExtensions[i] };

			if (requestedExtensions.find(tempString) != requestedExtensions.end()) {
				debugExtensions.erase(debugExtensions.begin() + i);
				i--;
			}
		}
		for (const auto& extension : debugExtensions) {
			if (strcmp(extension, VK_NV_DEVICE_DIAGNOSTIC_CHECKPOINTS_EXTENSION_NAME) == 0) {
				NVIDIAdebug = true;
				InitNvidiaDebug();
			}
			else if (strcmp(extension, VK_AMD_BUFFER_MARKER_EXTENSION_NAME)) {
				AMDdebug = true;
				InitAMDDebug();
			}
		}


		return debugExtensions.size() != inSize;
	}


	void (*checkpointPtr)(DeviceLostDebugStructure*, VkCommandBuffer, const char*, GFX_vk_checkpoint_type);

	void DeviceLostDebugStructure::InitNvidiaDebug() {
		vkCmdSetCheckpointNVX = reinterpret_cast<PFN_vkCmdSetCheckpointNV>(vkGetDeviceProcAddr(device, "vkCmdSetCheckpointNV"));
		checkpoints.reserve(50);

		checkpointPtr = &DeviceLostDebugStructure::AddNvidiaCheckpoint;
	}
	void DeviceLostDebugStructure::InitAMDDebug() {
		vkCmdWriteBufferMarkerAMDX = reinterpret_cast<PFN_vkCmdWriteBufferMarkerAMD>(vkGetDeviceProcAddr(device, "vkCmdWriteBufferMarkerAMDX"));
		checkpointPtr = &DeviceLostDebugStructure::AddAMDCheckpoint;
	}

	void DeviceLostDebugStructure::AddNvidiaCheckpoint(DeviceLostDebugStructure* thisPtr, VkCommandBuffer cmdBuf, const char* name, GFX_vk_checkpoint_type type) {
		VkCheckpointDataNV baseCopy{};
		baseCopy.sType = VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV;
		baseCopy.pNext = nullptr;
		if (thisPtr->checkpoints.size() > 0) {
			baseCopy.pCheckpointMarker = &thisPtr->checkpoints.back();
		}
		else {
			baseCopy.pCheckpointMarker = nullptr;
		}
		baseCopy.stage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
		thisPtr->checkpoints.push_back(baseCopy);

		assert(thisPtr->checkpoints.size() < 50 && "checkpoints is getting oversized, missed a clear?");
		printf("\n\n *** ADDING CHECKPOINT *** \n\n");
		vkCmdSetCheckpointNVX(cmdBuf, &thisPtr->checkpoints.back());
	}
	void DeviceLostDebugStructure::AddAMDCheckpoint(DeviceLostDebugStructure* thisPtr, VkCommandBuffer cmdBuf, const char* name, GFX_vk_checkpoint_type type) {
		//i want it to write when i tell it to :(
		//vkCmdWriteBufferMarkerAMD(cmdBuf, VK_PIPELINE_STAGE_NONE, buffer, offset, marker);

		//vkCmdWriteBufferMarkerAMD(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, buffer, 0, static_cast<uint32_t>(type));
	}

	void DeviceLostDebugStructure::AddCheckpoint(VkCommandBuffer cmdBuf, const char* name, GFX_vk_checkpoint_type type) {
		checkpointPtr(this, cmdBuf, name, type);
	}
	void DeviceLostDebugStructure::ClearCheckpoints() {
		checkpoints.clear();
	}


	bool deviceFaultEnabled = false;
	bool nvidiaCheckpoint = false;
	bool amdCheckpoint = false;
	std::array<VkQueue, 4> queues;

	void Initialize(VkDevice vkDevice, std::array<VkQueue, 4>& queuesParam, bool deviceFaultEnabledParam, bool nvidiaCheckpointEnabled, bool amdCheckpointEnabled) {
		device = vkDevice;
		deviceFaultEnabled = deviceFaultEnabledParam;
		nvidiaCheckpoint = nvidiaCheckpointEnabled;
		amdCheckpoint = amdCheckpointEnabled;

		queues = queuesParam;
		if (deviceFaultEnabled) {
			func = (PFN_vkGetDeviceFaultInfoEXT)vkGetDeviceProcAddr(device, "vkGetDeviceFaultInfoEXT");
		}
		if (nvidiaCheckpointEnabled) {
			vkGetQueueCheckpointDataNVX = (PFN_vkGetQueueCheckpointDataNV)vkGetDeviceProcAddr(device, "vkGetQueueCheckpointDataNV");
		}
	}
	void OnDeviceLost() {
		if (deviceFaultEnabled) {
			printf("device fault extension not supported, device lost debugging not supported\n");


			VkDeviceFaultCountsEXT faultCounts = { VK_STRUCTURE_TYPE_DEVICE_FAULT_COUNTS_EXT };
			printf("immediately before device lost function\n");
			VkResult               vkres = func(device, &faultCounts, nullptr);
			if (vkres != VK_SUCCESS) {
				printf("device lost analysis failed : %d", vkres);
				assert(false && "failed to get device fault info");
			}

			VkDeviceFaultInfoEXT faultInfo = { VK_STRUCTURE_TYPE_DEVICE_FAULT_INFO_EXT };

			faultInfo.pVendorInfos = nullptr;
			if (faultCounts.vendorInfoCount > 0) {
				faultInfo.pVendorInfos = new VkDeviceFaultVendorInfoEXT[faultCounts.vendorInfoCount];
			}

			faultInfo.pVendorInfos = nullptr;
			if (faultCounts.addressInfoCount > 0) {
				faultInfo.pAddressInfos = new VkDeviceFaultAddressInfoEXT[faultCounts.addressInfoCount];
			}

			faultCounts.vendorBinarySize = 0;
			vkres = func(device, &faultCounts, &faultInfo);
			if (vkres != VK_SUCCESS) {
				printf("device lost analysis failed on the second request : %d", vkres);
				assert(false && "failed to get device fault info");
			}
			printf("** Report from VK_EXT_device_fault ** \n");
			printf("Description : %s\n", faultInfo.description);
			printf("Vendor Infos : \n");
			for (uint32_t i = 0; i < faultCounts.vendorInfoCount; i++) {
				const VkDeviceFaultVendorInfoEXT* vendorInfo = &faultInfo.pVendorInfos[i];
				printf("\nInfo[%u]\n", i);
				printf("\tDescription: %s\n", vendorInfo->description);
				printf("\tFault code : %zu\n", (size_t)vendorInfo->vendorFaultCode);
				printf("\tFault Data : %zu\n", (size_t)vendorInfo->vendorFaultData);
			}

			static constexpr const char* addressTypeNames[] = {
				"NONE",
				"READ_INVALID",
				"WRITE_INVALID",
				"EXECUTE_INVALID",
				"INSTRUCTION_POINTER_UNKNOWN",
				"INSTRUCTION_POINTER_INVALID",
				"INSTRUCTION_POINTER_FAULT",
			};
			printf("\nAddress Infos :\n");
			for (uint32_t i = 0; i < faultCounts.addressInfoCount; i++) {
				const VkDeviceFaultAddressInfoEXT* addrInfo = &faultInfo.pAddressInfos[i];
				const VkDeviceAddress lower = (addrInfo->reportedAddress & ~(addrInfo->addressPrecision - 1));
				const VkDeviceAddress upper = (addrInfo->reportedAddress | (addrInfo->addressPrecision - 1));
				printf("\nInfo[%u]\n", i);
				printf("\tType : %s\n", addressTypeNames[addrInfo->addressType]);
				printf("\tReported Address : %zu\n", (size_t)addrInfo->reportedAddress);
				printf("\tLower Address : %zu\n", (size_t)lower);
				printf("\tUpper Address : %zu\n", (size_t)upper);
				printf("\tPrecision : %zu\n", (size_t)addrInfo->addressPrecision);
			}

			if (faultCounts.vendorInfoCount > 0) {
				delete[] faultInfo.pVendorInfos;
			}

			faultInfo.pVendorInfos = nullptr;
			if (faultCounts.addressInfoCount > 0) {
				delete[] faultInfo.pAddressInfos;
			}
		}
		printf("before nvidiaa checkpoint branch\n");
		if (nvidiaCheckpoint) {
			printf("finna get into nvidia checkpoitns\n");
			for (uint8_t i = 0; i < 4; i++) {
				uint32_t checkpointCount;

				vkGetQueueCheckpointDataNVX(queues[i], &checkpointCount, NULL);

				printf("[%u] checkpoint found on queue[%d]\n", checkpointCount, i);
				if (checkpointCount > 0) {
					std::vector<VkCheckpointDataNV> checkpointData(checkpointCount);
					//VkQueue queue, uint32_t* pCheckpointDataCount, VkCheckpointDataNV* pCheckpointData
					vkGetQueueCheckpointDataNVX(queues[i], &checkpointCount, checkpointData.data());
					for (auto const& checkpoint : checkpointData) {
						auto* cpData = reinterpret_cast<GFX_vk_checkpoint_data*>(checkpoint.pCheckpointMarker);
						printf("checkpoint {type:name} - %d:%s\n", cpData->type, cpData->name);
					}
				}
			}
		}
		else if (amdCheckpoint) {

		}

		assert(false && "end of device lost debug");
	}
}