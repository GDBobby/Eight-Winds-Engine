#include "data/TransformInclude.h"

glm::mat4 TransformComponent::mat4() {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);



    modelMatrix = {
        {
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
        },
        {translation.x, translation.y, translation.z, 1.0f} };

    return modelMatrix;
}

glm::mat3 TransformComponent::normalMatrix() {
    //always call this AFTER modelMatrix
    //fining all instances that use this funciton wit hthis comment

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