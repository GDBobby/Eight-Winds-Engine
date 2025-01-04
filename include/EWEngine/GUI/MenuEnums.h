#pragma once

#include <cstdint>

namespace EWE{
	enum MenuStates : uint16_t {
		menu_main = 0,
		//menu_controls,
		menu_audio_settings,
		menu_graphics_settings,

		menu_level_builder,

		menu_engine_size, //oversize check? idk if i need this
	};

	enum MenuTextureEnum : uint16_t {
		//this is the drawing order
		MT_NineUI, //nineUI
		MT_NineFade,
		MT_Slider,
		MT_BracketButton,
		MT_Bracket,
		MT_Unchecked,

		MT_Button,
		MT_Checked,
		MT_Base,

		MT_engine_size,
	};
}