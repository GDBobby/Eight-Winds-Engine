#include "EWEngine/Data/ReadEWEFromFile.h"

namespace EWE {
    namespace Reading {


        void GLM3FromFile(std::ifstream& inFile, glm::vec3& vec) {
            inFile.read((char*)&vec, sizeof(float) * 3);
        }
        void GLM3FromFileSwapEndian(std::ifstream& inFile, glm::vec3& vec) {
            printf(" ~~~~~~~~~~~~~~~~~~~~~~ need to debug this and ensure it works ~~~~~~~~~~~~~~~~~~~~~~~~ \n");

            uint32_t buffer[3];
            inFile.read((char*)buffer, sizeof(float) * 3);
            for (int i = 0; i < 3; i++) {
                buffer[i] = ((buffer[i] & 0xFF) << 24) | (((buffer[i] >> 8) & 0xFF) << 16) | (((buffer[i] >> 16) & 0xFF) << 8) | ((buffer[i] >> 24) & 0xFF);
            }
            memcpy(&vec, &buffer, sizeof(float) * 3);
        }
        void GLM2FromFile(std::ifstream& inFile, glm::vec2& vec) {
            inFile.read((char*)&vec, sizeof(float) * 2);
        }
        void GLM2FromFileSwapEndian(std::ifstream& inFile, glm::vec2& vec) {

            uint32_t buffer[2];
            inFile.read((char*)buffer, sizeof(float) * 2);
            for (int i = 0; i < 2; i++) {
                buffer[i] = ((buffer[i] & 0xFF) << 24) | (((buffer[i] >> 8) & 0xFF) << 16) | (((buffer[i] >> 16) & 0xFF) << 8) | ((buffer[i] >> 24) & 0xFF);
            }

            memcpy(&vec, &buffer, sizeof(float) * 2);
        }
        void GLMMat4FromFile(std::ifstream& inFile, glm::mat4* mat) {
            inFile.read((char*)mat, sizeof(glm::mat4));
        }
        void GLMMat4FromFileSwapEndian(std::ifstream& inFile, glm::mat4* mat) {
            uint32_t buffer[16];
            inFile.read((char*)buffer, sizeof(glm::mat4));
            for (int i = 0; i < 16; i++) {
                buffer[i] = ((buffer[i] & 0xFF) << 24) | (((buffer[i] >> 8) & 0xFF) << 16) | (((buffer[i] >> 16) & 0xFF) << 8) | ((buffer[i] >> 24) & 0xFF);
            }
            memcpy(mat, buffer, sizeof(glm::mat4));
        }

        void IntFromFile(std::ifstream& inFile, int* value) {
            inFile.read(reinterpret_cast<char*>(value), sizeof(int));
        }
        void UIntFromFile(std::ifstream& inFile, uint32_t* value) {
            inFile.read(reinterpret_cast<char*>(value), sizeof(uint32_t));
        }

        void UInt64FromFile(std::ifstream& inFile, uint64_t* value) {
            inFile.read(reinterpret_cast<char*>(value), sizeof(uint64_t));
            for (uint8_t i = 0; i < sizeof(uint64_t); i++) {
                printf("\t %d : %d \n", i, *(((char*)value) + i));
            }
        }
        void FloatFromFile(std::ifstream& inFile, float* value) {
            inFile.read(reinterpret_cast<char*>(value), sizeof(float));
        }
        void IntFromFileSwapEndian(std::ifstream& inFile, int* value) {
            uint32_t outValue;
            inFile.read((char*)&outValue, sizeof(int));
            outValue = ((outValue & 0xFF) << 24) | (((outValue >> 8) & 0xFF) << 16) | (((outValue >> 16) & 0xFF) << 8) | ((outValue >> 24) & 0xFF);
            memcpy(value, &outValue, sizeof(int));
        }
        void UIntFromFileSwapEndian(std::ifstream& inFile, uint32_t* value) {
            inFile.read((char*)value, sizeof(int));
            *value = ((*value & 0xFF) << 24) | (((*value >> 8) & 0xFF) << 16) | (((*value >> 16) & 0xFF) << 8) | ((*value >> 24) & 0xFF);
        }
        void UInt64FromFileSwapEndian(std::ifstream& inFile, uint64_t* value) {
            inFile.read((char*)value, sizeof(uint64_t));
            *value = ((*value & 0xFF) << 56) | (((*value >> 8) & 0xFF) << 48) |
                (((*value >> 16) & 0xFF) << 40) | (((*value >> 24) & 0xFF) << 32) |
                (((*value >> 32) & 0xFF) << 24) | (((*value >> 40) & 0xFF) << 16) |
                (((*value >> 48) & 0xFF) << 8) | ((*value >> 56) & 0xFF);
        }

        void FloatFromFileSwapEndian(std::ifstream& inFile, float* value) {
            uint32_t outValue;
            inFile.read((char*)&outValue, sizeof(int));
            outValue = ((outValue & 0xFF) << 24) | (((outValue >> 8) & 0xFF) << 16) | (((outValue >> 16) & 0xFF) << 8) | ((outValue >> 24) & 0xFF);
            memcpy(value, &outValue, sizeof(float));
        }
        void swapBasicEndian(void* value, size_t size) {

            printf(" ~~~~~~~~~~~~~~~~ DEBUG TO ENSURE THIS WORKS CORRECTLY ~~~~~~~~~~~~~~~ \n");
            char* rein = reinterpret_cast<char*>(value);

            size_t byte = 0;
            while (byte < (size - byte)) {
                std::swap(rein[byte], rein[size - byte - 1]);
                byte++;
            }
        }
        void swapGLMVec3Endian(glm::vec3& vec) {
            swapBasicEndian(&vec.x, sizeof(float));
            swapBasicEndian(&vec.y, sizeof(float));
            swapBasicEndian(&vec.z, sizeof(float));
        }
        void swapGLMVec2Endian(glm::vec2& vec) {
            swapBasicEndian(&vec.x, sizeof(float));
            swapBasicEndian(&vec.y, sizeof(float));
        }
        void swapGLMMat4Endian(glm::mat4& mat) {
            for (int x = 0; x < 4; x++) {
                for (int y = 0; y < 4; y++) {
                    swapBasicEndian(&mat[x][y], sizeof(float));
                }
            }
        }
    }
}