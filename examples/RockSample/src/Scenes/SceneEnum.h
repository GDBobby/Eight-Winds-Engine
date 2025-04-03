#pragma once
#pragma once

enum Scene_Enum { //need less than 256 scenes
	scene_mainmenu,
	scene_exitting,
	scene_loading,
	scene_ocean,
	scene_freeCamera, //this is specifically for capturing footage, maybe repurposed later
	scene_shaderGen,
	scene_LevelCreation,
	scene_PBR,

	loop_tester, //for quick testing
};