include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.23)

project(EightWindsEngine VERSION 1.1.1)

message(STATUS "Project source dir : ${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE source dir : ${CMAKE_SOURCE_DIR}")
message(STATUS "CMake current list dir : ${CMAKE_CURRENT_LIST_DIR}")
message(STATUS "~~~ CURRENT SOURCE DIR : ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "project name : ${PROJECT_NAME}")
message(STATUS "build location : ${CMAKE_CURRENT_BINARY_DIR}")


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(is_project_root OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(is_project_root ON)
else()
	set(is_project_root OFF)
endif()
message(STATUS "is EWEngine project root? ${is_project_root}")

option(EWE_EXAMPLES "Build examples" ${is_project_root})
option(EWE_PROJECTS "Build projects" ${is_project_root})
option(USING_NVIDIA_AFTERMATH, "Nvidia Aftermath" OFF)

Message(STATUS "Cmake Generator : ${CMAKE_GENERATOR}")

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/*.c)

file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.h ${PROJECT_SOURCE_DIR}/include/*.hpp)


include(FetchContent)
if(POLICY CMP0077)
	cmake_policy(SET CMP0077 NEW)
endif()

FetchContent_Declare(
	stb
	GIT_REPOSITORY https://github.com/nothings/stb
	GIT_TAG f4a71b1
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/include/stb"
)
FetchContent_MakeAvailable(stb)


FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG d79afa8
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw"
  BINARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw-build"
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")
set(GLFW_VULKAN_STATIC ON CACHE BOOL "")

FetchContent_MakeAvailable(glfw)


FetchContent_Declare(
	rapidjson
	GIT_REPOSITORY https://github.com/Tencent/rapidjson
	GIT_TAG 6089180
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/rapidjson"
)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_CXX11 OFF CACHE BOOL "")
set(RAPIDJSON_BUILD_CXX17 ON CACHE BOOL "")
set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE BOOL "")

FetchContent_MakeAvailable(rapidjson)

#tinyobj
FetchContent_Declare(
	tinyobj
	GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
	GIT_TAG 2945a96
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/include/tinyobjloader"
)
FetchContent_MakeAvailable(tinyobj)

FetchContent_Declare(
	vma
	GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
	GIT_TAG 98c4f34
	SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/vma"
)
FetchContent_MakeAvailable(vma)

#glslang
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(BUILD_EXTERNAL ON CACHE BOOL "")
set(ENABLE_GLSLANG_BINARIES ON CACHE BOOL "")
set(ENABLE_CTEST OFF CACHE BOOL "")
set(ENABLE_HLSL OFF CACHE BOOL "")
set(GLSLANG_TESTS OFF CACHE BOOL "")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/external/include/notvulkan/glslang")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/external/include/notvulkan/glslang")

set(GLSLANG_LIBS
	OSDependent
	SPIRV
	glslang
)

#potentially use a cmake command, USE_AFTERMATH
#if(USE_AFTERMATH)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/explicit_external/")

if(USING_NVIDIA_AFTERMATH)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8) #64bit, x64
	message(STATUS "including 64 bit Aftermath")
		set(AFTERMATH_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/explicit_external/Aftermath/lib/x64/")
		
		set(AFTERMATH_LIBS
			GFSDK_Aftermath_Lib.x64
		)

	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4) #32bit, x86
		message(FATAL_ERROR "Don't expect x86 to work")
		set(AFTERMATH_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/explicit_external/Aftermath/lib/x86")
		
		set(AFTERMATH_LIBS
			GFSDK_Aftermath_Lib.x86
		)
	endif()
endif()





add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADER_FILES})

# 1. Set VULKAN_SDK_PATH in .env.cmake to target specific vulkan version
if (DEFINED VULKAN_SDK_PATH)
  set(Vulkan_INCLUDE_DIR "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
  set(Vulkan_LIB_DIR "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
  set(Vulkan_FOUND "True")
  if(UNIX)
  	set(Vulkan_LIBRARY libvulkan.so)
  else()
	set(Vulkan_LIBRARY vulkan-1)
  endif()
else()
  message(STATUS "Vulkan path undefined in .env")
  find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
  message(STATUS "Found Vulkan package:  ${Vulkan_INCLUDE_DIRS}") #this line isnt working
endif()
if (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
else()
	message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARY}")
endif()

message(STATUS "including directories")
target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	
	${Vulkan_INCLUDE_DIR}
	${rapidjson_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/external/include
)
#set(EWENGINE_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "Build directory for EWEngine")


message(STATUS "linking directories")

target_link_directories(${PROJECT_NAME} PUBLIC
	${Vulkan_LIB_DIR}
	$<$<CONFIG:Debug>:${AFTERMATH_LIB_DIR}>
)

message(STATUS "linking libs")


target_link_libraries(${PROJECT_NAME} 
	glfw
	${Vulkan_LIBRARY}
	${GLSLANG_LIBS}
	$<$<CONFIG:Debug>:${AFTERMATH_LIBS}>
)

if(EWE_EXAMPLES)
	message(STATUS "building examples\n\n")
	add_subdirectory(examples)
endif()

if(EWE_PROJECTS)
	message(STATUS "building projects\n\n")

	if(!UNIX)
		#currently bugged, linux users will need to manually change this line for now.
		#ill address this eventually
		#add_subdirectory(Projects)
	endif()
endif()

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    DEBUG_POSTFIX "-d"  # Appends "-d" in Debug mode
    RELEASE_POSTFIX ""   # No postfix in Release mode
)

# Create source groups to maintain file structure in Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${HEADER_FILES})
