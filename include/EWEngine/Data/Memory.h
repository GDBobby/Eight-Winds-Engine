#pragma once

#include <new>
#include <utility>

#ifndef new
#define new static_assert(false, "new_ewe");
#endif

//#ifndef delete
//#define delete static_assert(false, "ewe_free");
//#endif

void* ewe_alloc_internal(size_t element_size, size_t element_count, const char* file, int line, const char* sourceFunction);

void ewe_free_internal(void* ptr);

// try to allocate size bytes
#ifndef ewe_alloc
//user is in charge of construction
#define ewe_alloc(size, count) ewe_alloc_internal(size, count, __FILE__, __LINE__, __FUNCTION__)
#endif

#ifndef ewe_free
//user is in charge of deconstruction
#define ewe_free(ptr) ewe_free_internal(ptr)
#endif

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