#pragma once

#include <EWEngine/Graphics/Pipeline.h>

namespace EWE {
	namespace Pipe {
		enum Enum {
			Terrain = Pipe::ENGINE_MAX_COUNT,
			TerrainWM,
			Perlin,
			GenGrass,
		};
	}
}