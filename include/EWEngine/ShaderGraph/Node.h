#pragma once

namespace EWE {
	//
	struct Edge;

	struct Node {
		Edge* inEdge{nullptr};
		Edge* outEdge{nullptr};

		void execute() {}
	};
}

