#ifndef GAME_VECTOR_H
#define GAME_VECTOR_H

#include "game/wstring.h"

inline void* operator new(unsigned long, void* storage) {
    return storage;
}
inline void* operator new[](unsigned long, void* storage) {
    return storage;
}

namespace std {
template <class T, class Allocator> class vector;
template <> class vector<int, app::app_allocator<int> > {
public:
    int* items;
    volatile unsigned int size;
    unsigned int capacity;

#if defined(GAME_VECTOR_AGGREGATE)
#elif defined(GAME_VECTOR_INLINE_CONSTRUCTOR)
    vector() : items(0), size(0), capacity(0) {}
#else
    vector();
#endif
    ~vector();
    unsigned int grow(unsigned int count);
    void allocate(unsigned int count);
    void append(unsigned int count);
    void resize(unsigned int count);
    int& operator[](int index);
};

#if defined(GAME_VECTOR_INLINE) && !defined(GAME_VECTOR_IMPLEMENTATION)
inline vector<int, app::app_allocator<int> >::~vector() {
    if (items) {
        size = *(unsigned int*)&size - *(int*)&size;
        appFreeUni__FPv(items);
    }
}
#endif
}
typedef std::vector<int, app::app_allocator<int> > IntVector;
// MWCC's explicit constructor expression (storage->__ct()) starts these member
// lifetimes without placement new[]'s allocation cookie or a scalar null check.
struct IntVectorArray {
    IntVector values[4];
};

struct RenderContext {
    char prefix[0x20A0];
    IntVector vectors[4];
    char players[0x294];
    ~RenderContext();
};

#endif
