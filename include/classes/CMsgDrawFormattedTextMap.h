#ifndef CLASSES_CMSGDRAWFORMATTEDTEXTMAP_H
#define CLASSES_CMSGDRAWFORMATTEDTEXTMAP_H

#include "game/types.h"

// The data layout of CMsgDrawFormattedTextMap has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CMsgDrawFormattedTextMap {
public:
    static int IteratorsNotEqual(int* left, int* right);
    static void GetEndIterator(void** result, char* tree);
    static void* FindInsertionParent(void* tree, int* key, void** parentOut, unsigned char* leftOut,
                                     unsigned char* rightOut);
    static void Find(void** result, void* tree, int* key);
    static void* CopyIterator(void** destination, void** source);
};

#endif // CLASSES_CMSGDRAWFORMATTEDTEXTMAP_H
