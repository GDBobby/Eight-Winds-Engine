#pragma once

#include <EWEngine/Graphics/Texture/Texture.h>

namespace EWE {
	namespace UI_Texture {
		static void CreateUIImage(ImageInfo& uiImageInfo, std::vector<PixelPeek> const& pixelPeek);

		static void CreateUIImageView(ImageInfo& uiImageInfo, uint8_t layerCount);
		static void CreateUISampler(ImageInfo& uiImageInfo);
	}//namespace UI_Texture
} //namespace EWE

