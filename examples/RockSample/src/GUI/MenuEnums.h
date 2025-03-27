#pragma once
#include "EWEngine/GUI/MenuEnums.h"

enum MenuStates : uint16_t {
	menu_ShaderGen = EWE::menu_engine_size,

	menu_app_size,
};

enum MenuTextureEnum : uint16_t {
	//this is the drawing order
	MT_shaderBack = EWE::MT_engine_size,

	MT_app_size,
};

