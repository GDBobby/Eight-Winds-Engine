#pragma once

#include "vulkan/vulkan.h"
#include "Preprocessor.h"

#if DEBUG_NAMING
namespace EWE{
	namespace DebugNaming {
		void QueueBegin(VkQueue queue, float red, float green, float blue, const char* name);
		void QueueEnd(VkQueue queue);

		void Initialize(bool extension_enabled);
		void Deconstruct();
		void SetObjectName(void* object, VkObjectType objectType, const char* name);

	} //namespace DebugNaming
} //namespace EWE
#endif

