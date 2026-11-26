// CFxEffectTree: the ordered tree of effect controls, keyed by serial number.
//
// The header is three words: the node count, the root node, and the cached
// in-order first node. The address of the root slot doubles as the end sentinel.
#ifndef CLASSES_CFXEFFECTTREE_H
#define CLASSES_CFXEFFECTTREE_H

#include "game/types.h"

// A node of the control tree. Its record starts with the child links, the
// packed parent pointer, and the signed key, and continues with its control.
struct FxControlTreeNode;

class CFxEffectTree {
public:
    // Find the node to insert a key under and which of its sides to use. The
    // returned node is the root slot itself when the tree is empty.
    FxControlTreeNode* FindInsertionPosition(int* key, FxControlTreeNode** parent, unsigned char* insertLeft,
                                             unsigned char* beforeFirst);

    unsigned int count;       // 0x00
    FxControlTreeNode* root;  // 0x04
    FxControlTreeNode* first; // 0x08  in-order first node
};

#define CFXEFFECTTREE_OFF(f, o) typedef char CFxEffectTree_off_##f[((int)&((CFxEffectTree*)0)->f == (o)) ? 1 : -1]
CFXEFFECTTREE_OFF(root, 0x4);
CFXEFFECTTREE_OFF(first, 0x8);
#undef CFXEFFECTTREE_OFF
typedef char CFxEffectTree_size[(sizeof(CFxEffectTree) == 0xC) ? 1 : -1];

#endif // CLASSES_CFXEFFECTTREE_H
