#pragma once

#include <EWEngine/Graphics/Texture/Texture.h>

namespace EWE {
	namespace UI_Texture {
		void CreateUIImage(ImageInfo& uiImageInfo, std::vector<PixelPeek> const& pixelPeek);

		void CreateUIImageView(ImageInfo& uiImageInfo);
		void CreateUISampler(ImageInfo& uiImageInfo);
	}//namespace UI_Texture
} //namespace EWE

