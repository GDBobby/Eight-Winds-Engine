include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.23)

project(EightWindsEngine VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(is_project_root OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(is_project_root ON)
endif()

option(EWE_EXAMPLES "Build examples" ${is_project_root})
option(EWE_PROJECTS "Build projects" ${is_project_root})

Message(STATUS "Cmake Generator : ${CMAKE_GENERATOR}")

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE SOURCES_C ${PROJECT_SOURCE_DIR}/src/*.c) #literally just for miniaudio.c, im tempted to move it somewhere else but idk

file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE HEADER_FILES_PP ${PROJECT_SOURCE_DIR}/include/*.hpp)


include(FetchContent)


FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG d79afa8
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw"
  BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw-build"
)

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_VULKAN_STATIC ON)

FetchContent_MakeAvailable(glfw)


FetchContent_Declare(
	rapidjson
	GIT_REPOSITORY https://github.com/Tencent/rapidjson
	GIT_TAG 6089180
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/rapidjson"
)
SET(RAPIDJSON_BUILD_DOC OFF)
SET(RAPIDJSON_BUILD_EXAMPLES OFF)
SET(RAPIDJSON_BUILD_TESTS OFF)
SET(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF)
SET(RAPIDJSON_BUILD_CXX11 OFF)
SET(RAPIDJSON_BUILD_CXX17 ON)

FetchContent_MakeAvailable(rapidjson)


FetchContent_Declare(
	stb
	GIT_REPOSITORY https://github.com/nothings/stb
	GIT_TAG f4a71b1
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/include/stb"
)
FetchContent_MakeAvailable(stb)


FetchContent_Declare(
	tinyobj
	GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
	GIT_TAG 2945a96
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/include/tinyobjloader"
)
FetchContent_MakeAvailable(tinyobj)

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${SOURCES_C} ${HEADER_FILES} ${HEADER_FILES_PP})

# 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
if (DEFINED VULKAN_SDK_PATH)
  set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
  set(Vulkan_LIB "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
  set(Vulkan_FOUND "True")
else()
  message(STATUS "Vulkan path undefined in .env")
  find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS "Using vulkan lib at: ${Vulkan_LIB}")
endif()

message(STATUS "including directories")
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	
	${Vulkan_INCLUDE_DIR}
	${rapidjson_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/external/include
	
)
message(STATUS "linking directories")
target_link_directories(${PROJECT_NAME} PUBLIC
	${Vulkan_LIB}
)

message(STATUS "linking libs")


target_link_libraries(${PROJECT_NAME} 
	glfw
	vulkan-1
	#vulkan

	$<$<CONFIG:Debug>:
	 SPVRemapperd
	 GenericCodeGend
	 glslangd
	 glslang-default-resource-limitsd
	 MachineIndependentd
	 OGLCompilerd
	 OSDependentd
	 SPIRVd
	 SPIRV-Toolsd
	 SPIRV-Tools-optd
	>
	$<$<CONFIG:Release>:
	 SPVRemapper
	 GenericCodeGen
	 glslang
	 glslang-default-resource-limits
	 MachineIndependent
	 OGLCompiler
	 OSDependent
	 SPIRV
	 SPIRV-Tools
	 SPIRV-Tools-opt
	>
	
)

if(EWE_EXAMPLES)
	message(STATUS "building examples\n\n")
	add_subdirectory(examples)
endif()

if(EWE_PROJECTS)
	message(STATUS "building projects\n\n")
	add_subdirectory(Projects)
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    DEBUG_POSTFIX "-d"  # Appends "-d" in Debug mode
    RELEASE_POSTFIX ""   # No postfix in Release mode
)

# Create source groups to maintain file structure in Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${SOURCES_C} ${HEADER_FILES} ${HEADER_FILES_PP})
