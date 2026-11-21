#ifndef GAME_STRING_H
#define GAME_STRING_H

#include "game/wstring.h"

#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
extern "C" void MWString_ConstructCString(void*, const char*);
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
namespace nstring_detail {
extern "C" unsigned int strlen(const char*);
extern "C" void String_ReserveCapacity(void*, unsigned int);
extern "C" void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
}
#endif

namespace std {
// MSL's narrow, 12-byte small-string specialization.
template <>
class basic_string<char, char_traits<char>, app::app_allocator<char> > {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    basic_string() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    basic_string(const char* text) { MWString_ConstructCString(this, text); }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    basic_string(const char* text) {
        basic_string* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    ~basic_string();
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

#if defined(GAME_NSTRING_INLINE) && !defined(GAME_NSTRING_IMPLEMENTATION)
inline basic_string<char, char_traits<char>, app::app_allocator<char> >::~basic_string() {
    if (flags >> 31) appFreeUni__FPv(data);
}
#endif
}

typedef std::basic_string<char, std::char_traits<char>, app::app_allocator<char> > NString;

#endif
