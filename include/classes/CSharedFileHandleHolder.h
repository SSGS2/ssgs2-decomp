#ifndef CLASSES_CSHAREDFILEHANDLEHOLDER_H
#define CLASSES_CSHAREDFILEHANDLEHOLDER_H

#include "game/types.h"

// A one-word holder for a cached shared-file handle.
class CSharedFileHandleHolder {
public:
    void* handle; // 0x0

    CSharedFileHandleHolder(void* pathString);
    ~CSharedFileHandleHolder();
};

typedef char CSharedFileHandleHolder_size[(sizeof(CSharedFileHandleHolder) == 0x4) ? 1 : -1];

#endif // CLASSES_CSHAREDFILEHANDLEHOLDER_H
