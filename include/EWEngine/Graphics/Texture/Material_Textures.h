#pragma once
#include "EWEngine/Graphics/Texture/Texture_Manager.h"


namespace EWE {
	class Material_Texture {
	private:
		enum MaterialTexType : MaterialFlags {
			MT_bump,
			MT_albedo,
			MT_normal,
			MT_rough,
			MT_metal,
			MT_ao,
		};

	public:
		static MaterialTextureInfo createMaterialTexture(std::string texPath, bool global);
	protected:

	};
}