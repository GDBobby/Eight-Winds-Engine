#pragma once
#include <EWEngine/GUI/MenuModule.h>
#include "MenuEnums.h"

namespace EWE {
	class MainMenuMM : public MenuModule {
	public:
		MainMenuMM();

		void ProcessClick(double xpos, double ypos) final;
	};
}