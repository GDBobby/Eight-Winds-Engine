#pragma once


#include "EWGraphics/Data/EngineDataTypes.h"

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
}//namespace EWE