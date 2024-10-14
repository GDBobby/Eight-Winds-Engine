#version 460
static const float M_2_PI = 6.283185307179586476925286766559;
static const float M_PI = 3.1415926535897932384626433832795;

struct RockData {
    uint32_t trackID;
    uint32_t trackPosition;
    float tilt;
    float RPS; // revolutions per second
    vec3 scale;
};
struct TrackData {
    uint32_t rockCount;
    float RPS; // revolutions per second
    float tilt;
    float radius;
};

struct Transform {
    vec3 translation;
    vec3 scale;
    vec3 rotation;

    mat4 CalculateTransform() {
        const float cz = cos(rotation.z);
        const float sz = sin(rotation.z);
        const float cx = cos(rotation.x);
        const float sx = sin(rotation.x);
        const float cy = cos(rotation.y);
        const float sy = sin(rotation.y);

        return {
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
            { translation.x, translation.y, translation.z, 1.0f } 
        };
    }
};

Buffer<RockData> rockInput;
Buffer<TrackData> trackInput;
RWBuffer<mat4> rockOutput;

layout(push_constant) uniform Push {
    float secondsSinceBeginning;
} push;

[shader("compute")]
[numthreads(32, 32, 1)]
void computeMain(uint3 id: SV_DispatchThreadID) {
    const uint rockIndex = id.x * 32 + id.y;
    const RockData rock = rockInput[rockIndex];
    const TrackData track = trackInput[rock.trackID];

    const float offsetInTrack = float(rock.trackPosition) / float(track.rockCount) * M_2_PI;
    const float trackRotation = push.secondsSinceBeginning * track.RPS * M_2_PI;

    const float firstDimensionPosition = trackRotation + offsetInTrack;

    const float cFDP = cos(firstDimensionPosition);
    const float sFDP = sin(firstDimensionPosition);

    const float cTilt = cos(track.tilt);
    const float sTilt = sin(track.tilt);

    Transform transform;
    transform.translation = normalize(vec3(
                                sFDP * sTilt,
                                cFDP * cTilt,
                                cFDP * sTilt    
                            )) * track.radius;

    transform.scale = rock.scale;

    const float rockRotation = rock.tilt + rock.RPS * push.secondsSinceBeginning;

    transform.rotation.x = cos(rockRotation);
    transform.rotation.y = sin(rockRotation);
    transform.rotation.z = cos(rockRotation);

    rockOutput[rockIndex] = transform.CalculateTransform();
}