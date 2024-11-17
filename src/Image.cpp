#include "EWEngine/Graphics/Texture/Image.h"

#include <stb/stb_image.h>

#ifndef TEXTURE_DIR
#define TEXTURE_DIR "textures/"
#endif

#define MIPMAP_ENABLED true


namespace EWE {


    PixelPeek::PixelPeek(std::string const& path)
#if DEBUG_NAMING
        : debugName{ path }
#endif
    {
        pixels = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
#if EWE_DEBUG
        assert(pixels && ((width * height) > 0) && path.c_str());
#endif
    }



}//namespace EWE