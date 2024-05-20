#pragma once

#include <new>
#include <utility>
#include <cassert>
#include <cstdint>

#define USING_MALLOC false


void* ewe_alloc_internal(std::size_t element_size, std::size_t element_count, const char* file, int line, const char* sourceFunction);
void ewe_free_internal(void* ptr);

#ifdef _DEBUG
void ewe_alloc_mem_track(void* ptr, const char* file, int line, const char* sourceFunction);
void ewe_free_mem_track(void* ptr);
#endif


#ifndef ewe_call_trace
#define ewe_call_trace __FILE__, __LINE__, __FUNCTION__
#endif
//i think function will give a call stack
#ifndef ewe_alloc
//user is in charge of construction
    #define ewe_alloc(size, count) ewe_alloc_internal(size, count, ewe_call_trace)
#endif

#ifndef ewe_free
//user is in charge of deconstruction
#define ewe_free(ptr) ewe_free_internal(ptr)
#endif

template<typename T, typename... Args>
static T* ConstructSingular(const char* file, int line, const char* sourceFunction, Args&&... args) {
#if USING_MALLOC
    void* memory = ewe_alloc_internal(sizeof(T), 1, file, line, sourceFunction);
    assert(memory && "memory is nullptr");
    return new (memory) T(std::forward<Args>(args)...);
#else
    T* ret = new T(std::forward<Args>(args)...);
#ifdef _DEBUG
    ewe_alloc_mem_track(reinterpret_cast<void*>(ret), file, line, sourceFunction);
#endif
    return ret;
#endif
    //return memory;
}

/*
template<typename T, typename... Args>
static T** ConstructMultiple(uint32_t count, Args&&... args) {
    void* memory = ewe_alloc(sizeof(T), count);
    if (memory == nullptr) {
		return nullptr;
	}
    uint64_t memLoc = reinterpret_cast<uint64_t>(memory);
    T* ret = new (memory) T(std::forward<Args>(args)...);
    for (uint32_t i = 0; i < count; i++) {
        memcpy(reinterpret_cast<void*>(memLoc + i * sizeof(T)), , sizeof(T));
    }
}
*/

/*
* doesnt compile, probably not possible
template<typename T, typename... Args> 
static T** constructVector(std::vector<Args> args_param) {
    const uint64_t memory_size = sizeof(T);
    void* memory = std::malloc(memory_size * args_param.size());
    if (memory == nullptr) {
        return nullptr;
    }
    T** ret = &memory;
    uint32_t object_iter = 0;
    for (auto const& params : args_param)
    {
        ret[object_iter] = new(T[object_iter]) T(std::forward<Args>(args_params[objectIter])...);
        object_iter++;
    }
}
*/

/*
_THROW1(_STD bad_alloc) {
    void* p;
    while ((p = malloc(size)) == 0)
        if (_callnewh(size) == 0)
        {       // report no memory
            static const std::bad_alloc nomem;
            _RAISE(nomem);
        }

    return (p);
}
*/