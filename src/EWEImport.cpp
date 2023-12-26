#include "EWEngine/Data/EWEImport.h"

#include <thread>

#define MODEL_PATH "models\\"

template <typename T>
void ImportData::readData(T& data, std::string meshPath) {
    //printf("starting up mesh thread :%s \n", meshPath.c_str());
    std::ifstream inFile(meshPath, std::ifstream::binary);
    //inFile.open();
    if (!inFile.is_open()) {
        printf("failed to open : %s \n", meshPath.c_str());
        //std throw
    }
    //printf("before formatingg input file in mesh \n");
    boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
    binary_input_archive& data;
    inFile.close();
    //printf("file read successfully \n");
}

/*
ImportData ImportData::loadDataThreaded(std::string importPath) {
    ImportData returnData;
    printf("entering static load data function \n");
    std::thread meshThread[2];
    std::string meshPath = importPath;
    meshPath += "_mesh.ewe";

    bool meshThreadActive[2] = { false, false };

    if (std::filesystem::exists(meshPath)) {
        meshThreadActive[0] = true;
        meshThread[0] = std::thread(&ImportData::readData<meshEData>, std::ref(returnData.meshExport), meshPath);
    }
    else {
        printf("mesh path doesn't exist : %s \n", meshPath.c_str());
    }

    meshPath = importPath + "_meshNT.ewe";
    if (std::filesystem::exists(meshPath)) {
        meshThreadActive[1] = true;
        meshThread[1] = std::thread(&ImportData::readData<meshNTEData>, std::ref(returnData.meshNTExport), meshPath);
    }
    else {
        printf("mesh NT path doesn't exist : %s \n", meshPath.c_str());
    }

    meshPath = importPath + "_Names.ewe";
    if (std::filesystem::exists(meshPath)) {
        std::ifstream inFile(meshPath, std::ifstream::binary);
        if (!inFile.is_open()) {
            printf("failed to open : %s \n", meshPath.c_str());
        }
        printf("before formatingg input file in mesh \n");
        boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
        binary_input_archive& returnData.nameExport;
        inFile.close();
        printf("file read successfully \n");
    }


    for (int i = 0; i < 2; i++) {
        if (meshThreadActive[i]) {
            if (meshThread[i].joinable()) {
                meshThread[i].join();
            }
            meshThreadActive[i] = false;
        }
    }

    meshPath = importPath + "_simpleMesh.ewe";
    if (std::filesystem::exists(meshPath)) {
        meshThreadActive[0] = true;
        meshThread[0] = std::thread(&ImportData::readData<meshSimpleData>, std::ref(returnData.meshSimpleExport), meshPath);
    }

    meshPath = importPath + "_simpleMeshNT.ewe";
    if (std::filesystem::exists(meshPath)) {
        meshThreadActive[1] = true;
        meshThread[1] = std::thread(&ImportData::readData<meshNTSimpleData>, std::ref(returnData.meshNTSimpleExport), meshPath);
    }

    for (int i = 0; i < 2; i++) {
        if (meshThreadActive[i]) {
            if (meshThread[i].joinable()) {
                meshThread[i].join();
            }
            meshThreadActive[i] = false;
        }
    }

    printf("returning from static import load function \n");
    return returnData;
}
*/

ImportData ImportData::loadData(std::string importPath) {
    ImportData returnData;
    //printf("entering static load data function \n");
    std::thread meshThread[2];
    std::string meshPath = MODEL_PATH;
    meshPath += importPath + "_mesh.ewe";

    bool meshThreadActive[2] = { false, false };


    if (std::filesystem::exists(meshPath)) {
        meshThreadActive[0] = true;
        meshThread[0] = std::thread(&ImportData::readData<meshEData>, std::ref(returnData.meshExport), meshPath);
    }
    else {
        //printf("mesh NT path doesn't exist : %s \n", meshPath.c_str());
    }

    meshPath = MODEL_PATH + importPath + "_meshNT.ewe";
    if (std::filesystem::exists(meshPath)) {
        meshThreadActive[1] = true;
        meshThread[1] = std::thread(&ImportData::readData<meshNTEData>, std::ref(returnData.meshNTExport), meshPath);
    }
    else {
        //printf("mesh NT path doesn't exist : %s \n", meshPath.c_str());
    }

    meshPath = MODEL_PATH + importPath + "_Names.ewe";
    if (std::filesystem::exists(meshPath)) {
        std::ifstream inFile(meshPath, std::ifstream::binary);
        if (!inFile.is_open()) {
            printf("failed to open : %s \n", meshPath.c_str());
        }
       // printf("before formatingg input file in mesh \n");
        boost::archive::binary_iarchive binary_input_archive(inFile, boost::archive::no_header);
        binary_input_archive& returnData.nameExport;
        inFile.close();
        //printf("file read successfully \n");
    }


    for (int i = 0; i < 2; i++) {
        if (meshThreadActive[i]) {
            if (meshThread[i].joinable()) {
                printf("waiting on mesh thread : %d \n", i);
                meshThread[i].join();
            }
            meshThreadActive[i] = false;
        }
    }

    meshPath = MODEL_PATH + importPath + "_simpleMesh.ewe";
    if (std::filesystem::exists(meshPath)) {
        meshThreadActive[0] = true;
        meshThread[0] = std::thread(&ImportData::readData<meshSimpleData>, std::ref(returnData.meshSimpleExport), meshPath);
    }

    meshPath = MODEL_PATH + importPath + "_simpleMeshNT.ewe";
    if (std::filesystem::exists(meshPath)) {
        meshThreadActive[1] = true;
        meshThread[1] = std::thread(&ImportData::readData<meshNTSimpleData>, std::ref(returnData.meshNTSimpleExport), meshPath);
    }

    for (int i = 0; i < 2; i++) {
        if (meshThreadActive[i]) {
            if (meshThread[i].joinable()) {
                printf("waiting on simple mesh thread : %d \n", i);
                meshThread[i].join();
            }
            meshThreadActive[i] = false;
        }
    }

    //printf("returning from static import load function \n");
    return returnData;
}