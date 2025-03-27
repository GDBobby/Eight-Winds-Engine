#version 460
static const float M_2_PI = 6.283185307179586476925286766559;
static const float M_PI = 3.1415926535897932384626433832795;
static const uint MAX_TRACK_COUNT = 32;

struct RockData {
    float scale;
    float azimuthalRotation;
    float radius;
    float speed;

    // https://en.wikipedia.org/wiki/Spherical_coordinate_system
};

struct Transform {
    vec4 translation;
    // vec3 scale;
    float scale;
    vec3 forwardDir;

    mat4 CalculateTransform() {
        const vec3 upWorld = { 0.0, 1.0, 0.0 };
        const vec3 right = normalize(cross(upWorld, forwardDir));
        const vec3 upDir = normalize(cross(forwardDir, right));



        return {
            { //scale.x
                scale * right.x,
                scale * right.y,
                scale * right.z,
                0.0f,
            },
            { //scale.y
                scale * upDir.x,
                scale * upDir.y,
                scale * upDir.z,
                0.0f,
            },
            { //scale.z
                scale * forwardDir.x,
                scale * forwardDir.y,
                scale * forwardDir.z,
                0.0f,
            },
            translation 
        };
    }
};

layout(set = 0, binding = 1) uniform RockBuffer {
    RockData rockInput[1024];
};
layout(set = 0, binding = 2) buffer OutputBuffer {
    (layout row_major) mat4 rockOutput[];
};

layout(set = 0, binding = 0) cbuffer GlobalUbo {
    float4x4 projView;
    float4 cameraPos;
};

layout(push_constant) uniform Push {
    float secondsSinceBeginning;
} push;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 id: SV_DispatchThreadID) {
    const uint rockIndex = id.x * 32 + id.y;
    const RockData rock = rockInput[rockIndex];

    const float zenithAngle = push.secondsSinceBeginning * rock.speed * M_2_PI;

    const float cZenith = cos(zenithAngle);
    const float sZenith = sin(zenithAngle);

    const float cAzimuthal = cos(rock.azimuthalRotation);
    const float sAzimuthal = sin(rock.azimuthalRotation);

    Transform transform;
    transform.translation = vec4(
                                sZenith * cAzimuthal * rock.radius,
                                sZenith * sAzimuthal * rock.radius,
                                cZenith * rock.radius,
                                1.0 //this goes into the mat4 later as the final 1.0
                            );

    transform.scale = rock.scale;

    transform.forwardDir = normalize(cameraPos.xyz - transform.translation.xyz);

    rockOutput[rockIndex] = transpose(transform.CalculateTransform());
}