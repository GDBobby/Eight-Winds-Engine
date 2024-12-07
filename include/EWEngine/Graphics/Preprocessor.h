#pragma once

 //on linux, _DEBUG needs to be defined explicitly
 //_DEBUG is already defined in Visual Studio while in DEBUG mode
 //this changes it so that on windows, _DEBUG needs to be defined explicitly

#define CALL_TRACING true
#if CALL_TRACING
#include <source_location>
#endif

#define EWE_DEBUG true

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

#define IMAGE_DEBUGGING (true && EWE_DEBUG)

#ifndef DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION
#define DESCRIPTOR_IMAGE_IMPLICIT_SYNCHRONIZATION true
#endif

#define MIPMAP_ENABLED true