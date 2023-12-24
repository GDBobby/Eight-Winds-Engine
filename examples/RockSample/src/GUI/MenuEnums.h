#pragma once
#include "EWEngine/gui/MenuEnums.h"

enum MenuClickReturn : uint16_t {
	MCR_swapToShaderGen = EWE::MCR_engine_size,
	MCR_swapToMainMenu,
	MCR_app_size,
};
enum MenuStates : uint16_t {
	menu_ShaderGen = EWE::menu_engine_size,

	menu_app_size,
};

enum MenuTextureEnum : uint16_t {
	//this is the drawing order
	MT_shaderBack = EWE::MT_engine_size,

	MT_app_size,
};

