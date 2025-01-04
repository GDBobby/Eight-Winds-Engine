#pragma once

 //on linux, _DEBUG needs to be defined explicitly
 //_DEBUG is already defined in Visual Studio while in DEBUG mode
 //so im just replacing _DEBUG with EWE_DEBUG, supports release with debug too


#ifndef EWE_DEBUG_PREDEF
//EWE_DEBUG includes validation layers, this will make a release build not shippable
#define EWE_DEBUG true
#else
#define EWE_DEBUG true
 //EWE_DEBUG_PREDEF
#endif

#define CALL_TRACING (true && EWE_DEBUG)
#if CALL_TRACING
#include <source_location>
#endif

#define DEBUGGING_DEVICE_LOST false
#define USING_NVIDIA_AFTERMATH (true && DEBUGGING_DEVICE_LOST)

#define GPU_LOGGING true
#define DECONSTRUCTION_DEBUG (true && EWE_DEBUG)

#define DEBUGGING_PIPELINES (false && EWE_DEBUG)
#define DEBUGGING_MATERIAL_PIPE (false && EWE_DEBUG)

#define DEBUG_NAMING (true && EWE_DEBUG)

#define RENDER_DEBUG false

#define USING_VMA false
#define DEBUGGING_MEMORY_WITH_VMA (USING_VMA && false)

#define SEMAPHORE_TRACKING (true && DEBUG_NAMING)

#define COMMAND_BUFFER_TRACING (true && EWE_DEBUG)
#define DEBUGGING_FENCES (false && EWE_DEBUG)

#define ONE_SUBMISSION_THREAD_PER_QUEUE false

#define IMAGE_DEBUGGING (true && EWE_DEBUG)

#ifndef DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION
#define DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION true
#endif

#define MIPMAP_ENABLED true

#if EWE_DEBUG
    #define EWE_UNREACHABLE assert(false)
#else
    #ifdef _MSC_VER
        #define EWE_UNREACHABLE __assume(false)
    #elif defined(__GNUC__) || defined(__clang__)
        #define EWE_UNREACHABLE __builtin_unreachable()
    #else
        #define EWE_UNREACHABLE throw std::runtime_error("unreachable code")
    #endif
#endif