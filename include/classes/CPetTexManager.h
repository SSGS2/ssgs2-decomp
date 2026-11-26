// CPetTexManager: the texture table a puppet builds while its material layers
// are loaded.
//
// The manager is reference counted. Each entry pairs a loaded texture handle with
// the byte a material lookup returns for it and the material index it came from.
// Entries are kept in a circular list of separately allocated nodes; the manager
// owns the nodes and one resource-manager image ID per entry.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CPETTEXMANAGER_H
#define CLASSES_CPETTEXMANAGER_H

#include "game/types.h"

// One texture used by the puppet's material layers.
struct PetTextureEntry {
    int key;           // 0x00  image ID released when the manager is destroyed
    unsigned char val; // 0x04  byte the material lookup returns
    short extra;       // 0x06  stored with the entry, not read here
};

// A list node: two links, then the entry.
struct PetTextureEntryNode {
    PetTextureEntryNode* prev; // 0x00
    PetTextureEntryNode* next; // 0x04
    PetTextureEntry entry;     // 0x08
};

// The embedded list: an element count, then the sentinel's two links. The links
// are spliced by hand in this unit.
struct PetTextureEntryList {
    unsigned long size;        // 0x00
    PetTextureEntryNode* prev; // 0x04
    PetTextureEntryNode* next; // 0x08
};

class CPetTexManager {
public:
    CPetTexManager();
    ~CPetTexManager();

    // Add an entry for a texture; the list takes ownership of it. Returns 1.
    int AppendTextureEntry(int key, unsigned char val, unsigned short extra);

    int referenceCount;          // 0x00
    void* vtbl;                  // 0x04
    PetTextureEntryList entries; // 0x08
};

#define CPETTEXMANAGER_OFFSET(field, offset) \
    typedef char CPetTexManager_offset_##field[((int)&((CPetTexManager*)0)->field == (offset)) ? 1 : -1]
CPETTEXMANAGER_OFFSET(referenceCount, 0x00);
CPETTEXMANAGER_OFFSET(vtbl, 0x04);
CPETTEXMANAGER_OFFSET(entries, 0x08);
#undef CPETTEXMANAGER_OFFSET
typedef char CPetTexManager_size[(sizeof(CPetTexManager) == 0x14) ? 1 : -1];
typedef char PetTextureEntryNode_size[(sizeof(PetTextureEntryNode) == 0x10) ? 1 : -1];

#endif // CLASSES_CPETTEXMANAGER_H
