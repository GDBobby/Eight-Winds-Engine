#include "EWEngine/Data/TransformInclude.h"

#include <cstdio>

glm::mat4 TransformComponent::mat4() {
    const float cz = glm::cos(rotation.z);
    const float sz = glm::sin(rotation.z);
    const float cx = glm::cos(rotation.x);
    const float sx = glm::sin(rotation.x);
    const float cy = glm::cos(rotation.y);
    const float sy = glm::sin(rotation.y);



    modelMatrix = {
        
        {
            scale.x * (cy * cz + sy * sx * sz),
            scale.x * (cx * sz),
            scale.x * (cy * sx * sz - cz * sy),
            0.0f,
        },
        {
            scale.y * (cz * sy * sx - cy * sz),
            scale.y * (cx * cz),
            scale.y * (cy * cz * sx + sy * sz),
            0.0f,
        },
        {
            scale.z * (cx * sy),
            scale.z * (-sx),
            scale.z * (cy * cx),
            0.0f,
        },
        {translation.x, translation.y, translation.z, 1.0f} };

    return modelMatrix;
}
void TransformComponent::mat4(float* buffer) const {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);

    buffer[0] = scale.x * (c1 * c3 + s1 * s2 * s3);
    buffer[1] = scale.x * (c2 * s3);
    buffer[2] = scale.x * (c1 * s2 * s3 - c3 * s1);
    buffer[3] = 0.f;

    buffer[4] = scale.y * (c3 * s1 * s2 - c1 * s3);
    buffer[5] = scale.y * (c2 * c3);
    buffer[6] = scale.y * (c1 * c3 * s2 + s1 * s3);
    buffer[7] = 0.f;

    buffer[8] = scale.z * (c2 * s1);
    buffer[9] = scale.z * (-s2);
    buffer[10] = scale.z * (c1 * c2);
    buffer[11] = 0.0f;

    buffer[12] = translation.x;
    buffer[13] = translation.y;
    buffer[14] = translation.z;
    buffer[15] = 1.0f;


}

glm::mat3 TransformComponent::normalMatrix() {
    //always call this AFTER modelMatrix
    //fining all instances that use this funciton with this comment

    invScaleSquared[0] = 1.f / (scale.x * scale.x);
    invScaleSquared[1] = 1.f / (scale.y * scale.y);
    invScaleSquared[2] = 1.f / (scale.z * scale.z);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            normMat[i][j] = modelMatrix[i][j] * invScaleSquared[i];
        }
    }
    return normMat;
}


bool TransformComponent::similar(TransformComponent& second) {
    bool ret = true;
    float difference = translation.x - second.translation.x;
    if ((difference > 0.0001f) || (difference < -0.0001f)) {
        printf("first : second ~ %.5f : %.5f \n", translation.x, second.translation.x);
        printf("trans x diff %.5f ", difference);
        ret = false;
        //return false;
    }
    difference = translation.y - second.translation.y;
    if ((difference > 0.0001f) || (difference < -0.0001f)) {
        printf("first : second ~ %.5f : %.5f \n", translation.y, second.translation.y);
        printf("trans y diff %.5f ", difference);
        ret = false;
        //return false;
    }
    difference = translation.z - second.translation.z;
    if ((difference > 0.0001f) || (difference < -0.0001f)) {
        printf("first : second ~ %.5f : %.5f \n", translation.z, second.translation.z);
        printf("trans z diff %.5f ", difference);
        ret = false;
    }
    difference = rotation.x - second.rotation.x;
    if ((difference > 0.0001f) || (difference < -0.0001f)) {
        printf("first : second ~ %.5f : %.5f \n", rotation.x, second.rotation.x);
        printf("rot x diff %.5f ", difference);
        ret = false;
    }
    difference = rotation.y - second.rotation.y;
    if ((difference > 0.0001f) || (difference < -0.0001f)) {
        printf("first : second ~ %.5f : %.5f \n", rotation.y, second.rotation.y);
        printf("rot y diff %.5f ", difference);
        ret = false;
    }
    difference = rotation.z - second.rotation.z;
    if ((difference > 0.0001f) || (difference < -0.0001f)) {
        printf("first : second ~ %.5f : %.5f \n", rotation.z, second.rotation.z);
        printf("rot z diff %.5f ", difference);
        ret = false;
    }

    return ret;
}




	Matrix3ForGLSL Transform2D::Matrix() const {
		const float cosine = glm::cos(rotation);
		const float sine = glm::sin(rotation);
		Matrix3ForGLSL ret{};
		ret.columns[0].x = scale.x * cosine;
		ret.columns[0].y = scale.x * sine;

		ret.columns[1].x = -scale.y * sine;
		ret.columns[1].y = scale.y * cosine;

		ret.columns[2].x = translation.x;
		ret.columns[2].y = translation.y;
		ret.columns[2].z = 1.f;

		return ret;
	}

	Matrix3ForGLSL Transform2D::Matrix(glm::vec2 const& meterScaling) const {
		const float cosine = glm::cos(rotation);
		const float sine = glm::sin(rotation);

		Matrix3ForGLSL mat3{};
		mat3.columns[0] = glm::vec4(scale.x * cosine, -scale.y * sine, translation.x * meterScaling.x, 0.f);
		mat3.columns[1] = glm::vec4(scale.x * sine, scale.y * cosine, -translation.y * meterScaling.y, 0.f);
		mat3.columns[2] = glm::vec4(0.f, 0.f, 1.f, 0.f);

		return mat3;
	}
	Matrix3ForGLSL Transform2D::Matrix(const glm::mat3 conversionMatrix, const glm::vec2 tilesOnScreen) const {

		const float cosine = glm::cos(rotation);
		const float sine = glm::sin(rotation);

		const glm::vec3 tempTrans = conversionMatrix * glm::vec3(translation, 1.f);
		glm::vec2 renderTrans{ tempTrans.x, -tempTrans.y };

		glm::vec2 renderScale{ scale.x / tilesOnScreen.x, scale.y / tilesOnScreen.y };

		Matrix3ForGLSL ret{};
		ret.columns[0].x = renderScale.x * cosine;
		ret.columns[0].y = renderScale.x * sine;

		ret.columns[1].x = -renderScale.y * sine;
		ret.columns[1].y = renderScale.y * cosine;

		ret.columns[2].x = renderTrans.x;
		ret.columns[2].y = renderTrans.y;
		ret.columns[2].z = 1.f;

		return ret;
	}