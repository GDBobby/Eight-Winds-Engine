cmake_minimum_required(VERSION 3.11.0)

project(RockSample)

# message(STATUS "Project source dir : ${PROJECT_SOURCE_DIR}")
# message(STATUS "CMAKE source dir : ${CMAKE_SOURCE_DIR}")
# message(STATUS "CMake current list dir : ${CMAKE_CURRENT_LIST_DIR}")

# include(${CMAKE_SOURCE_DIR}/EightWindsEngineConfig.cmake REQUIRED)
# message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES src/*.cpp)

file(GLOB_RECURSE HEADER_FILES src/*.h src/*.hpp)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
file(GLOB_RECURSE SHADERS 
    ${SHADER_DIR}/*.vert 
    ${SHADER_DIR}/*.frag
    ${SHADER_DIR}/*.comp
);

foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPV_OUTPUT ${SHADER}.spv)
if(UNIX AND NOT APPLE)
    add_custom_command(
        OUTPUT ${SPV_OUTPUT}
        COMMAND ${CMAKE_SOURCE_DIR}../shader_builder.sh ${SHADER}
        DEPENDS ${SHADER} ${CMAKE_SOURCE_DIR}/shader_builder.sh
        COMMENT "Compiling shader ${SHADER_NAME} to ${SPV_OUTPUT}"
        VERBATIM
    )
elseif(WIN32)
    message(ERROR "need to add windows support for the automatic shader generation step")
endif()

    list(APPEND SPIRV_FILES ${SPV_OUTPUT})
endforeach()
add_custom_target(build_shaders ALL DEPENDS ${SPIRV_FILES})


add_executable(${PROJECT_NAME} ${SOURCES} ${HEADER_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_SOURCE_DIR}/include
)
target_link_directories(${PROJECT_NAME} PUBLIC
	$<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/build/Debug>
	$<$<CONFIG:Release>:${CMAKE_SOURCE_DIR}/build/Release>
)

target_link_libraries(${PROJECT_NAME} PUBLIC EightWindsEngine)

# Create source groups to maintain file structure in Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${HEADER_FILES})

# Set the working directory for the executable
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
    VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    DEBUG_POSTFIX "-debug"  # Appends "-d" in Debug mode
    RELEASE_POSTFIX ""   # No postfix in Release mode
)
