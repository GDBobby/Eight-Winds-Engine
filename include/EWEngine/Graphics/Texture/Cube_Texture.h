#pragma once

#include <EWEngine/Graphics/Texture/Texture_Manager.h>

namespace EWE {
	class Cube_Texture {
	private:

	protected:
		static void createCubeImage(ImageInfo& cubeTexture, EWEDevice& device, std::vector<PixelPeek>& pixelPeek);

		static void createCubeImageView(ImageInfo& cubeTexture, EWEDevice& device);
		static void createCubeSampler(ImageInfo& cubeTexture, EWEDevice& device);

	public:
		static TextureDesc createCubeTexture(EWEDevice& device, std::string texPath);
	};
}

