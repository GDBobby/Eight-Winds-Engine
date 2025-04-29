#pragma once

#include <EWGraphics/Vulkan/Pipeline.h>
#include "EWEngine/Systems/Rendering/Pipelines/PipeEnum.h"

namespace EWE {
	namespace Pipe {
		enum Enum {
			Terrain = Pipe::ENGINE_MAX_COUNT,
			SimpleTerrain,
			Perlin,
			GenGrass,
		};
	}
}