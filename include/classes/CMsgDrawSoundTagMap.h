#ifndef CLASSES_CMSGDRAWSOUNDTAGMAP_H
#define CLASSES_CMSGDRAWSOUNDTAGMAP_H

#include "game/types.h"

struct SoundTagMapNode;
struct SoundTagMapValue;

// The formatted-text map from glyph index to sound tag. The object is the tree header; its nodes are defined in
// text_format_trees_and_msg_resources.cpp.
class CMsgDrawSoundTagMap {
public:
    // Insert a node below where, copying the value's inline bytes or deep-copying its heap string.
    void* InsertNode(SoundTagMapNode* where, int left, int add, SoundTagMapValue* value);

    int count;       // 0x00
    void* root;      // 0x04
    int treeState;   // 0x08
    void* firstNode; // 0x0C
};

#define CMSGDRAWSOUNDTAGMAP_OFFSET(field, offset) \
    typedef char CMsgDrawSoundTagMap_offset_##field[((int)&((CMsgDrawSoundTagMap*)0)->field == (offset)) ? 1 : -1]
CMSGDRAWSOUNDTAGMAP_OFFSET(count, 0x00);
CMSGDRAWSOUNDTAGMAP_OFFSET(firstNode, 0x0C);
typedef char CMsgDrawSoundTagMap_size[(sizeof(CMsgDrawSoundTagMap) == 0x10) ? 1 : -1];

#endif // CLASSES_CMSGDRAWSOUNDTAGMAP_H
