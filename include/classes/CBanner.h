#ifndef CLASSES_CBANNER_H
#define CLASSES_CBANNER_H

#include "game/types.h"

struct BannerNode;

// The save banner: its shared file handles and its localized text, kept in a
// tree keyed by string. Some member functions still reach the object's data
// through file-local layouts or raw offsets.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
class CBanner {
public:
    ~CBanner();

    int ReloadFileHandles();

    static void* Create();

    void* vtbl;                        // 0x00
    void** fileHandles;                // 0x04
    unsigned int fileHandleCount;      // 0x08
    unsigned int fileHandleCapacity;   // 0x0C
    int initializationAttemptFinished; // 0x10
    unsigned int localizedTextCount;   // 0x14
    BannerNode* localizedTextRoot;     // 0x18  its address is the tree's end marker
    BannerNode* localizedTextFirst;    // 0x1C
    unsigned char loadRequested;       // 0x20
};

#define CBANNER_OFFSET(field, offset) \
    typedef char CBanner_offset_##field[((int)&((CBanner*)0)->field == (offset)) ? 1 : -1]
CBANNER_OFFSET(fileHandles, 0x04);
CBANNER_OFFSET(initializationAttemptFinished, 0x10);
CBANNER_OFFSET(localizedTextRoot, 0x18);
CBANNER_OFFSET(loadRequested, 0x20);
#undef CBANNER_OFFSET
typedef char CBanner_size[(sizeof(CBanner) == 0x24) ? 1 : -1];

#endif // CLASSES_CBANNER_H
