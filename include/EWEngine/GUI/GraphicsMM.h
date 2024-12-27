#pragma once
#include "MenuModule.h"

namespace EWE {
	class GraphicsMM : public MenuModule {
	public:
		GraphicsMM();

		void ProcessClick(double xpos, double ypos) final;

	};
}