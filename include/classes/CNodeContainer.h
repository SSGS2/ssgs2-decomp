// CNodeContainer: a named node in a course tree.
//
// A node owns its name (an MSL small string), one word of metadata and a vector
// of 12-byte entries. It is the element type of the course's node vector.
//
// The class has no virtual functions on purpose: its vtable is still taken from
// the original game data, so `vtbl` is spelled out as the first member.
#ifndef CLASSES_CNODECONTAINER_H
#define CLASSES_CNODECONTAINER_H

#include "game/types.h"

// The 12-byte MSL narrow string that stores a node's name.
struct NodeName {
    unsigned int flags; // 0x0  bit 31 selects heap storage
    unsigned int length;
    char* data;
};
typedef char NodeName_size[(sizeof(NodeName) == 0xC) ? 1 : -1];

class CNodeContainer {
public:
    CNodeContainer(const NodeName& name);
    ~CNodeContainer();

    static void CopyConstructAt(void* object, void* destination, void* value);
    static void Append(void* object, void* value);

    void* vtbl;                 // 0x00
    unsigned int nameFlags;     // 0x04  bit 31 selects heap storage
    unsigned int nameLength;    // 0x08
    char* nameData;             // 0x0C  heap bytes when nameFlags bit 31 is set
    unsigned int metadataWord;  // 0x10
    void* entries;              // 0x14  data pointer of the entry vector
    unsigned int entryCount;    // 0x18
    unsigned int entryCapacity; // 0x1C
};

#define CNODECONTAINER_OFF(f, o) typedef char CNodeContainer_off_##f[((int)&((CNodeContainer*)0)->f == (o)) ? 1 : -1]
CNODECONTAINER_OFF(nameFlags, 0x4);
CNODECONTAINER_OFF(nameData, 0xc);
CNODECONTAINER_OFF(metadataWord, 0x10);
CNODECONTAINER_OFF(entries, 0x14);
CNODECONTAINER_OFF(entryCount, 0x18);
CNODECONTAINER_OFF(entryCapacity, 0x1c);
#undef CNODECONTAINER_OFF
typedef char CNodeContainer_size[(sizeof(CNodeContainer) == 0x20) ? 1 : -1];

#endif // CLASSES_CNODECONTAINER_H
