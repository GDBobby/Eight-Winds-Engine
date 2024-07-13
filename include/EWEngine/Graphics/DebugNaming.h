#pragma once

#include "EWEngine/Graphics/VulkanHeader.h"
#include "Preprocessor.h"

#if DEBUG_NAMING
namespace DebugNaming {
	void Initialize();
	void Deconstruct();

};
#endif

