// CFxRegistryTree: an ordered map of effects keyed by their address.
//
// The tree owns its nodes. `first` caches the node with the smallest key, and
// the address of `root` doubles as the end sentinel of an in-order walk. Both
// the effect box and a sequence keep their live sprays and sequences in these
// trees.
#ifndef CLASSES_CFXREGISTRYTREE_H
#define CLASSES_CFXREGISTRYTREE_H

#include "game/types.h"

// One node. The low bit of parentTag is the red-black colour.
struct FxRegistryNode {
    FxRegistryNode* left;      // 0x00
    FxRegistryNode* right;     // 0x04
    FxRegistryNode* parentTag; // 0x08
    unsigned int key;          // 0x0C
    void* value;               // 0x10
};

class CFxRegistryTree {
public:
    // Find the link where a node with *key belongs. predecessorOut is the
    // greatest key not above the query, or null; descendLeftOut says which way
    // the last node was passed; noPredecessorOut stays 1 only when every visited
    // key was above the query.
    FxRegistryNode** FindInsertionPosition(unsigned int* key, FxRegistryNode** predecessorOut,
                                           unsigned char* descendLeftOut, unsigned char* noPredecessorOut);

    unsigned int count;    // 0x00
    FxRegistryNode* root;  // 0x04
    FxRegistryNode* first; // 0x08
};

#define CFXREGISTRYTREE_OFF(field, offset) \
    typedef char CFxRegistryTree_offset_##field[((int)&((CFxRegistryTree*)0)->field == (offset)) ? 1 : -1]
CFXREGISTRYTREE_OFF(count, 0x0);
CFXREGISTRYTREE_OFF(root, 0x4);
CFXREGISTRYTREE_OFF(first, 0x8);
#undef CFXREGISTRYTREE_OFF
typedef char CFxRegistryTree_size[(sizeof(CFxRegistryTree) == 0xC) ? 1 : -1];
typedef char FxRegistryNode_size[(sizeof(FxRegistryNode) == 0x14) ? 1 : -1];

#endif // CLASSES_CFXREGISTRYTREE_H
