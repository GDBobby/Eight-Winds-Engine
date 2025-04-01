#pragma once
#include "EWEngine/Graphics/Texture/Image_Manager.h"


namespace EWE {
	class Material_Image {
	private:
		enum MaterialImgType : MaterialFlags {
			MT_bump,
			MT_albedo,
			MT_normal,
			MT_rough,
			MT_metal,
			MT_ao,

			MT_SIZE,
		};

	public:
		static MaterialInfo CreateMaterialImage(std::string texPath, bool mipmapping, bool global);
	protected:

	};
}