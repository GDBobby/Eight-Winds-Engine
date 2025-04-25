#pragma once

#include <fstream>
#include <cstdint>

#include <LAB/Vector.h>
#include <LAB/Matrix.h>


namespace EWE {
    namespace Reading {
        void GLM3FromFile(std::ifstream& inFile, lab::vec3& vec);
        void GLM3FromFileSwapEndian(std::ifstream& inFile, lab::vec3& vec);
        void GLM2FromFile(std::ifstream& inFile, lab::vec2& vec);
        void GLM2FromFileSwapEndian(std::ifstream& inFile, lab::vec2& vec);

        void IntFromFile(std::ifstream& inFile, int* value);
        void IntFromFileSwapEndian(std::ifstream& inFile, int* value);
        void UIntFromFile(std::ifstream& inFile, uint32_t* value);
        void UIntFromFileSwapEndian(std::ifstream& inFile, uint32_t* value);

        void UInt64FromFile(std::ifstream& inFile, uint64_t* value);
        void UInt64FromFileSwapEndian(std::ifstream& inFile, uint64_t* value);

        void FloatFromFile(std::ifstream& inFile, float* value);
        void FloatFromFileSwapEndian(std::ifstream& inFile, float* value);

        void GLMMat4FromFile(std::ifstream& inFile, lab::mat4* mat);
        void GLMMat4FromFileSwapEndian(std::ifstream& inFile, lab::mat4* mat);

        
        void swapBasicEndian(void* value, size_t size);
        void swapGLMVec3Endian(lab::vec3& vec);
        void swapGLMVec2Endian(lab::vec2& vec);
        void swapGLMMat4Endian(lab::mat4& mat);
    }
}