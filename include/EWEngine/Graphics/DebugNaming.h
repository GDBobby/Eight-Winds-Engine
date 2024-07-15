#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "Preprocessor.h"

#if DEBUG_NAMING
namespace EWE{
	namespace DebugNaming {
		void QueueBegin(VkQueue queue, float red, float green, float blue, const char* name);
		void QueueEnd(VkQueue queue);

		void Initialize(VkDevice device, bool extension_enabled);
		void Deconstruct();
		void SetObjectName(VkDevice device, void* object, VkObjectType objectType, const char* name);

	} //namespace DebugNaming
} //namespace EWE
#endif

