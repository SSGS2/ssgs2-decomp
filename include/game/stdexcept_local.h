#ifndef GAME_STDEXCEPT_LOCAL_H
#define GAME_STDEXCEPT_LOCAL_H


// Local replacement for the exc2 change to exception.h: only the units that include this header get it.
// Include before sdk_protos.h. Final path: include/game/stdexcept_local.h
extern "C" {
unsigned long strlen(const char*) __attribute__((nothrow));
char* strcpy(char*, const char*) __attribute__((nothrow));
void __dla__FPv(void*) __attribute__((nothrow));
}

#include "game/exception.h"

// std::logic_error and its two children as MSL builds them. The message is copied into a new char[] that a
// reference-counted control block (a shared_ptr_deleter<char, default_delete<char[]> >) owns; a unique_ptr-like
// guard holds the buffer until the control block exists.
// The constructors are inline, so every throw site contains the whole message construction. The children throw
// from inside their constructor: the thrown object then never gets a cleanup action of its own, which is what
// the game's exception tables show. `typeName` is the "std::out_of_range" type-name string the unit's copy of
// the runtime type info uses (each unit pools its own).
extern "C" {
void __throw(const char* typeName, void* object, void* destroy) __attribute__((noreturn, nothrow));
}
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12out_of_range[];  // vtable of the shared_ptr_deleter control block

namespace std {
namespace detail {
struct default_delete_chars {  // default_delete<char[]>
    char unused;
    default_delete_chars() : unused(0) {}
    void operator()(char* p) const { delete[] p; }
};
// unique_ptr<char, default_delete<char[]>&>: owns the buffer until release()
struct char_array_guard {
    char* ptr;
    default_delete_chars& deleter;
    char_array_guard(char* p, default_delete_chars& d) : ptr(p), deleter(d) {}
    ~char_array_guard() { if (ptr) deleter(ptr); }
    void release() { ptr = 0; }
};
struct message_control {  // shared_ptr_deleter<char, default_delete<char[]> >
    const void* table;
    int useCount;
    int weakCount;
    char* ptr;
    message_control(char* p) : useCount(1), weakCount(1) { table = s_sharedCharDeleterVtable; ptr = p; }
};
}

class logic_error : public exception {
public:
    logic_error(const char* text) {
        detail::default_delete_chars deleter;
        unsigned int length = strlen(text) + 1;
        message = static_cast<char*>(operator new[]((length << 1) - length));
        {
            char* buffer = message;
            detail::char_array_guard guard(buffer, deleter);
            control = new detail::message_control(buffer);
            guard.release();
        }
        strcpy(message, text);
    }
    virtual ~logic_error();
private:
    char* message;
    detail::message_control* control;
};

extern "C" void __dt__Q23std12out_of_rangeFv(void*);
extern "C" void __dt__Q23std12length_errorFv(void*);

class out_of_range : public logic_error {
public:
    out_of_range(const char* text) : logic_error(text) {}
    out_of_range(const char* text, const char* typeName) : logic_error(text) {
        __throw(typeName, this, (void*)__dt__Q23std12out_of_rangeFv);
    }
    virtual ~out_of_range();
};

class length_error : public logic_error {
public:
    length_error(const char* text) : logic_error(text) {}
    length_error(const char* text, const char* typeName) : logic_error(text) {
        __throw(typeName, this, (void*)__dt__Q23std12length_errorFv);
    }
    virtual ~length_error();
};
}

#endif
