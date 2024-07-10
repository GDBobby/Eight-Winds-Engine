#include "vulkan/vulkan.h"

namespace EWE{
	namespace Queue {
		enum Enum : uint32_t {
			graphics,
			present,
			compute,
			transfer,
			_count,
		};
	} //namespace Queue
} //namespace EWE