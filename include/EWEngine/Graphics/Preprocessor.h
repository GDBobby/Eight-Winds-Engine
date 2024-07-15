#pragma once

#ifndef _DEBUG
 //on linux, _DEBUG needs to be defined explicitly
 //_DEBUG is already defined in Visual Studio while in DEBUG mode
    #define _DEBUG true
#endif

#define DEBUGGING_DEVICE_LOST false
#define USING_NVIDIA_AFTERMATH true && DEBUGGING_DEVICE_LOST

#define GPU_LOGGING true
#define DECONSTRUCTION_DEBUG true

#define DEBUGGING_PIPELINES false
#define DEBUGGING_DYNAMIC_PIPE false

#ifdef _DEBUG
#define DEBUG_NAMING true
#else
#define DEBUG_NAMING false
#endif

#define RENDER_DEBUG false