#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "Preprocessor.h"

#if DEBUG_NAMING
namespace EWE{
	namespace DebugNaming {
		void Initialize(VkDevice device, bool extension_enabled);
		void Deconstruct();
		void SetObjectName(VkDevice device, void* object, VkDebugReportObjectTypeEXT objectType, const char *name);

	} //namespace DebugNaming
} //namespace EWE
#endif

