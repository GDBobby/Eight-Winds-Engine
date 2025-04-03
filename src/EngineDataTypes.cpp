#include "EWEngine/Data/EngineDataTypes.h"

namespace EWE {

	Matrix3ForGLSL::Matrix3ForGLSL(glm::mat3 const& inMat) {
		columns[0].x = inMat[0].x;
		columns[0].y = inMat[0].y;
		columns[0].z = inMat[0].z;
		columns[1].x = inMat[1].x;
		columns[1].y = inMat[1].y;
		columns[1].z = inMat[1].z;
		columns[2].x = inMat[2].x;
		columns[2].y = inMat[2].y;
		columns[2].z = inMat[2].z;
	}


}