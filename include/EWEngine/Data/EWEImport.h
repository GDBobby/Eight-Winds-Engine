#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "EWEngine/graphics/model/EWE_model.h"

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <fstream>
#include <vector>
#include <string>

#define EXPECTED_IMPORT_VERSION "2.0.0" //need to do some SHA256 key or some shit

#define bobmat_size 16

class ImportData {
public:
    struct bobmat4 {
        float bmat4[bobmat_size];
        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& bmat4;
        }
    };
    struct boneEData {
        uint32_t boneID{ 69420 };
        bobmat4 boneTransform{};

        boneEData() {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& boneID;
            archive& boneTransform;
        }
    };
    struct meshEData {
        std::string versionTracker = "";
        std::vector<std::pair<std::vector<EWE::bobVertex>, std::vector<uint32_t>>> meshes;

        meshEData(std::vector<EWE::bobVertex>& vertex, std::vector<uint32_t>& index) {
            meshes.push_back(std::make_pair(vertex, index));
        }
        meshEData() {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& meshes;
        }
    };
    struct meshNTEData {
        std::string versionTracker = "";
        std::vector<std::pair<std::vector<EWE::boneVertexNoTangent>, std::vector<uint32_t>>> meshesNT;
        meshNTEData(std::vector<EWE::boneVertexNoTangent>& vertex, std::vector<uint32_t>& index) {
            meshesNT.push_back(std::make_pair(vertex, index));
        }
        meshNTEData() {}

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& meshesNT;
        }
    };
    struct meshSimpleData {
        std::vector<std::pair<std::vector<EWE::bobAVertex>, std::vector<uint32_t>>> meshesSimple;
        meshSimpleData(std::vector<EWE::bobAVertex>& vertex, std::vector<uint32_t>& index) {
            meshesSimple.push_back(std::make_pair(vertex, index));
        }
        meshSimpleData() {}
        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& meshesSimple;
        }
    };
    struct meshNTSimpleData {
        std::vector<std::pair<std::vector<EWE::LeafVertex>, std::vector<uint32_t>>> meshesNTSimple;
        meshNTSimpleData(std::vector<EWE::LeafVertex>& vertex, std::vector<uint32_t>& index) {
            meshesNTSimple.push_back(std::make_pair(vertex, index));
        }
        meshNTSimpleData() {}
        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& meshesNTSimple;
        }
    };
    struct AnimData {
        std::string versionTracker = "";
        std::vector<bobmat4> defaultBoneValues; //T-POSE or something, for when an animation doesn't cover everything

        int32_t handBone = -1;

        //animation
        //1 duration will have 1 set of bones, 1 bone will hold a vector of matrix transforms by length of duration

        std::vector< //animation count
            std::vector< //animation duration
            std::vector< //bone count
            boneEData>>> //{bone id, bone transform}, bone ID will keep track of which bone as i clear useless bones. i could also use a map, might be better
            animations;

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& defaultBoneValues;
            archive& animations;
            archive& handBone;
        }
    };
    struct FullAnimData {
        std::string versionTracker = "";
        int32_t handBone = -1;

        std::vector< //each animation
            std::vector< //animation frame duration
            std::vector< //boneCount
            bobmat4>>> animations;

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& animations;
            archive& handBone;
        }
    };
    struct NameExportData {
        std::string versionTracker = "";
        std::vector<std::string> meshNames;
        std::vector<std::string> meshNTNames;
        std::vector<std::string> meshSimpleNames;
        std::vector<std::string> meshNTSimpleNames;

        template<class Archive>
        void serialize(Archive& archive, const unsigned int version) {
            archive& versionTracker;
            archive& meshNTNames;
            archive& meshNames;
            archive& meshSimpleNames;
            archive& meshNTSimpleNames;
        }
    };

    meshEData meshExport;
    meshNTEData meshNTExport;
    meshSimpleData meshSimpleExport;
    meshNTSimpleData meshNTSimpleExport;
    AnimData animExport;
    NameExportData nameExport;

    template <typename T>
    static void readData(T& data, std::string meshPath);

    //static ImportData loadDataThreaded(std::string importPath);

    static ImportData loadData(std::string importPath);

};