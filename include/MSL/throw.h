#ifndef MSL_THROW_H
#define MSL_THROW_H

namespace msl_exception_runtime {
extern "C" void __throw(const char*, void*, void*) __attribute__((noreturn));
}

namespace std {
// The runtime copies the exception; its local storage must not acquire a second
// destructor cleanup. CodeWarrior's explicit constructor call preserves that ABI.
template <class Exception>
inline void __throw_exception(const char* typeName, void* destructor) {
    union { double alignment; unsigned char bytes[sizeof(Exception)]; } storage;
    Exception* error = (Exception*)storage.bytes;
    error->__ct();
    msl_exception_runtime::__throw(typeName, error, destructor);
}
}

#endif
