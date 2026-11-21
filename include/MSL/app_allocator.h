#ifndef MSL_APP_ALLOCATOR_H
#define MSL_APP_ALLOCATOR_H

// app::app_allocator<T>: the game's allocator for standard containers. It holds no state, so containers
// keep it in no space; storage is returned to the game heap with appFreeUni.
extern "C" void appFreeUni__FPv(void*);
namespace msl_allocator_runtime {
extern "C" void* appMallocDummy__FUl(unsigned long);
}

namespace app {
template <class T> class app_allocator {
public:
    template <class U> struct rebind { typedef app_allocator<U> other; };
    T* allocate(unsigned long n) { return (T*)msl_allocator_runtime::appMallocDummy__FUl(n * sizeof(T)); }
    void deallocate(T* p, unsigned long) { appFreeUni__FPv(p); }
};
}

#endif
