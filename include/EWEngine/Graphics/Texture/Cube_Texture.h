#pragma once

#include <EWEngine/Graphics/Texture/Texture_Manager.h>

namespace EWE {
	class Cube_Texture {
	private:

	protected:
		static void createCubeImage(ImageInfo& cubeTexture, std::vector<PixelPeek>& pixelPeek);

		static void createCubeImageView(ImageInfo& cubeTexture);
		static void createCubeSampler(ImageInfo& cubeTexture);

	public:
		static TextureDesc createCubeTexture(std::string texPath, std::string extension = ".png");
	};
}

