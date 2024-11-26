#pragma once
#include "EWEngine/Graphics/VulkanHeader.h"

namespace EWE {
	namespace Cube_Texture {
		ImageID CreateCubeImage(std::string texPath, Queue::Enum queue, std::string extension = ".png");
	};
}

