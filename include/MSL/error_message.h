#ifndef MSL_ERROR_MESSAGE_H
#define MSL_ERROR_MESSAGE_H

namespace msl_message_runtime {
extern "C" unsigned long strlen(const char*);
extern "C" char* strcpy(char*, const char*);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void __dla__FPv(void*);
extern "C" char s_sharedCharDeleterVtable[];
}

namespace std {
namespace detail {
struct __message_buffer_guard {
    char* pointer;
    char* allocator;
    __message_buffer_guard(char* p, char* a) : pointer(p), allocator(a) {}
    void dispose() { if (pointer) msl_message_runtime::__dla__FPv(pointer); }
};
struct __message_control {
    void* table;
    unsigned strong, weak;
    char* pointer;
    __message_control(char* p) {
        strong = 1;
        weak = 1;
        table = msl_message_runtime::s_sharedCharDeleterVtable;
        pointer = p;
    }
};
}
// Shared diagnostic storage used by MSL logic_error. Guard and Control preserve
// the unit's existing ABI cleanup symbols during incremental header conversion.
template <class Guard, class Control> struct __error_message {
    char* text;
    void* control;
    inline void assign(const char* message) {
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        unsigned n = msl_message_runtime::strlen(message) + 1;
        text = (char*)msl_message_runtime::__nwa__FUl(2 * n - n);
        {
            char* p = text;
            Guard allocation(p, &allocator.value);
            control = new Control(p);
            allocation.pointer = 0;
        }
        msl_message_runtime::strcpy(text, message);
    }
};
}

#endif
