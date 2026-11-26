#ifndef CLASSES_CSTRING_H
#define CLASSES_CSTRING_H

#include "game/types.h"

// The data layout of CString has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CString {
public:
    static char* FindCharForwarder(const char* text, int c);
    static char* FindLastCharForwarder(const char* text, int c);

    static char* FindSubstringForwarder(const char* s, const char* needle);
};

#endif // CLASSES_CSTRING_H
