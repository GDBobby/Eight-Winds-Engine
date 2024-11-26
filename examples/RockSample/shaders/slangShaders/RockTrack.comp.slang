#version 460
static const float M_2_PI = 6.283185307179586476925286766559;
static const float M_PI = 3.1415926535897932384626433832795;
static const uint MAX_TRACK_COUNT = 32;

struct RockData {
    vec3 scale;
    //x is 1 azimuthal rotation, y is radius, z is speed. the object rotates around the zenith 
    vec3 translationSpherical; // https://en.wikipedia.org/wiki/Spherical_coordinate_system
};

struct Transform {
    vec4 translation;
    vec3 scale;
    vec3 forwardDir;

    mat4 CalculateTransform() {
        const float3 upWorld = { 0.0, 1.0, 0.0 };
        const float3 right = normalize(cross(upWorld, forwardDir));
        const float3 upDir = cross(forwardDir, right);



        return {
            {
                scale.x * right.x,
                scale.x * right.y,
                scale.x * right.z,
                0.0f,
            },
            {
                scale.y * upDir.x,
                scale.y * upDir.y,
                scale.y * upDir.z,
                0.0f,
            },
            {
                scale.z * forwardDir.x,
                scale.z * forwardDir.y,
                scale.z * forwardDir.z,
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

    const float zenithAngle = push.secondsSinceBeginning * rock.translationSpherical.z * M_2_PI;

    const float cZenith = cos(zenithAngle);
    const float sZenith = sin(zenithAngle);

    const float cAzimuthal = cos(rock.translationSpherical.x);
    const float sAzimuthal = sin(rock.translationSpherical.x);

    Transform transform;
    transform.translation = vec4(
                                sZenith * cAzimuthal * rock.translationSpherical.y,
                                sZenith * sAzimuthal * rock.translationSpherical.y,
                                cZenith * rock.translationSpherical.y,
                                1.0 //this goes into the mat4 later as the final 1.0
                            );

    transform.scale = rock.scale;

    transform.forwardDir = cameraPos.xyz - transform.translation.xyz;



    rockOutput[rockIndex] = transpose(transform.CalculateTransform());
}