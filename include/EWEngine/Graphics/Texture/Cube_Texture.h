#pragma once

#include <EWEngine/Graphics/Texture/Texture_Manager.h>

namespace EWE {
	namespace Cube_Texture {
		TextureDesc CreateCubeImage(std::string texPath, Queue::Enum queue, std::string extension = ".png");
	};
}

