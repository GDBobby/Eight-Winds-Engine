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

Message(STATUS "Cmake Generator : ${CMAKE_GENERATOR}")

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE SOURCES_C ${PROJECT_SOURCE_DIR}/src/*.c) #literally just for miniaudio.c, im tempted to move it somewhere else but idk

file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h)
file(GLOB_RECURSE HEADER_FILES_PP ${PROJECT_SOURCE_DIR}/include/*.hpp)

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

# 2. Set GLFW_PATH in .env.cmake to target specific glfw
if (DEFINED GLFW_PATH)
  message(STATUS "Using GLFW path specified in .env")
  set(GLFW_INCLUDE_DIR "${GLFW_PATH}/include")
  if (MSVC)
    set(GLFW_LIB "${GLFW_PATH}/lib-vc2022") # 2.1 Update lib-vc2019 to use same version as your visual studio
  elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    message(STATUS "USING MINGW")
    set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64") # 2.1 make sure matches glfw mingw subdirectory
  endif()
else()
  message(STATUS "GLFW path undefined in .env")
  find_package(glfw3 3.3 REQUIRED)
  set(GLFW_LIB glfw)
  message(STATUS "Found GLFW")
endif()
if (NOT GLFW_LIB)
	message(FATAL_ERROR "Could not find glfw library!")
else()
	message(STATUS "Using glfw lib at: ${GLFW_LIB}")
endif()

# 3. Set BOOST_SDK_PATH in .env.cmake to target specific boost version
if (DEFINED BOOST_SDK_PATH)
  set(BOOST_INCLUDE_DIR "${BOOST_SDK_PATH}") # 1.1 Make sure this include path is correct
  set(BOOST_LIB "${BOOST_SDK_PATH}/stage/lib") # 1.2 Make sure lib path is correct
  set(BOOST_FOUND "True")
else()
  message(STATUS "boost path undefined in .env")
  find_package(Boost REQUIRED) # throws error if could not find boost
  message(STATUS "Found Boost: $ENV{BOOST_SDK}")
  find_package(Boost COMPONENTS serialization REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE Boost::serialization)
endif()
if (NOT BOOST_FOUND)
	message(FATAL_ERROR "Could not find Boost library!")
else()
	message(STATUS "Using Boost lib at: ${BOOST_LIB}")
endif()

# 4. Set RAPIDJSON_PATH in .env.cmake to target specific rapidjson version
if (DEFINED RAPIDJSON_PATH)
  set(RAPIDJSON_INCLUDE_DIR "${RAPIDJSON_PATH}/include") # Make sure this include path is correct
  set(RAPIDJSON_FOUND "True")
else()
  message(STATUS "did not define rapidjson path")
  find_package(RapidJSON REQUIRED) # throws error if could not find RAPIDJSON
  message(STATUS "Found RAPIDJSON: $ENV{RAPIDJSON_SDK}")
endif()
if (NOT RAPIDJSON_FOUND)
	message(FATAL_ERROR "Could not find RAPIDJSON library!")
else()
	message(STATUS "Using RAPIDJSON lib at: ${RAPIDJSON_INCLUDE_DIR}")
endif()

# If TINYOBJ_PATH not specified in .env.cmake, try fetching from git repo
if (NOT TINYOBJ_PATH)
  message(STATUS "TINYOBJ_PATH not specified in .env.cmake, using external/tinyobjloader")
  set(TINYOBJ_PATH external/include)
endif()

# If STB_PATH not specified in .env.cmake, try fetching from git repo
if (NOT STB_PATH)
  message(STATUS "STB_PATH not specified in .env.cmake, using external/stb")
  set(STB_PATH external/include)
endif()

message(STATUS "including directories")
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	${Vulkan_INCLUDE_DIR}
	${BOOST_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${RAPIDJSON_INCLUDE_DIR}
    	${TINYOBJ_PATH}
	${STB_PATH}
	${TINYOBJLOADER_INCLUDE_DIR}
)
message(STATUS "linking directories")
target_link_directories(${PROJECT_NAME} PUBLIC
	${GLFW_LIB}
	${Vulkan_LIB}
	${BOOST_LIB}
)

message(STATUS "linking libs")
target_link_libraries(${PROJECT_NAME} 
	glfw3 
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


message(STATUS "cmake current binary dir : ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config")

# Export the target properties to be used by other projects
export(TARGETS ${PROJECT_NAME}
    FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
)

if(EWE_EXAMPLES)
	message(STATUS "building examples\n\n")
	add_subdirectory(examples)
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    DEBUG_POSTFIX "-d"  # Appends "-d" in Debug mode
    RELEASE_POSTFIX ""   # No postfix in Release mode
)

# Create source groups to maintain file structure in Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${SOURCES_C} ${HEADER_FILES} ${HEADER_FILES_PP})
