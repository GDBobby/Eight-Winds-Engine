#pragma once

#include "EWEngine/Data/EngineDataTypes.h"


namespace EWE{
	static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;

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