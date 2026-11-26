#ifndef GAME_WSTRING_H
#define GAME_WSTRING_H

extern "C" void appFreeUni__FPv(void*);

namespace app {
template <class T> class app_allocator;
}

namespace std {
template <class T> struct char_traits;
template <class T> class allocator;

// MSL's 12-byte small-string representation; short wide text starts at +2.
template <class T, class Traits = char_traits<T>, class Allocator = allocator<T> > class basic_string;

template <> class basic_string<wchar_t, char_traits<wchar_t>, app::app_allocator<wchar_t> > {
public:
    unsigned int flags;
    unsigned int length;
    wchar_t* data;

    basic_string() : flags(0), length(0), data(0) {}
    ~basic_string();
};

// The owner supplies the concrete external definition. Plain ABI callers omit
// the inline body; member/local lifetime users need it visible to match MWCC.
#if !defined(GAME_WSTRING_IMPLEMENTATION) && !defined(GAME_WSTRING_DECLARATIONS_ONLY)
inline basic_string<wchar_t, char_traits<wchar_t>, app::app_allocator<wchar_t> >::~basic_string() {
    if (flags >> 31)
        appFreeUni__FPv(data);
}
#endif
}

typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, app::app_allocator<wchar_t> > WString;

#endif
