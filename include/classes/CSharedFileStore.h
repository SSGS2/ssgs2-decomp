#ifndef CLASSES_CSHAREDFILESTORE_H
#define CLASSES_CSHAREDFILESTORE_H

#include "game/types.h"

// One entry of the shared-file registry: a filename and a metadata word.
// The filename uses the string layout of the game: a flags word whose bit 31
// marks heap text, then the length and the text pointer.
class CSharedFileStore {
public:
    CSharedFileStore(const CSharedFileStore& src);
    ~CSharedFileStore();

    void* vtbl;              // 0x00
    unsigned int nameFlags;  // 0x04  bit 31: text is on the heap
    unsigned int nameLength; // 0x08
    char* nameText;          // 0x0C
    unsigned int metadata;   // 0x10
};

typedef char CSharedFileStore_size[(sizeof(CSharedFileStore) == 0x14) ? 1 : -1];

#endif // CLASSES_CSHAREDFILESTORE_H
