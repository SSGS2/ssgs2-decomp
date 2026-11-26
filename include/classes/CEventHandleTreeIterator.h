#ifndef CLASSES_CEVENTHANDLETREEITERATOR_H
#define CLASSES_CEVENTHANDLETREEITERATOR_H

#include "game/types.h"

// One node of a keyed tree owned by CEventHandle. The low bit of the parent
// pointer holds the node's colour; the key and the borrowed value follow the
// link header.
struct EventHandleTreeNode {
    EventHandleTreeNode* left;   // 0x0
    EventHandleTreeNode* right;  // 0x4
    unsigned int parentAndColor; // 0x8
    unsigned int key;            // 0xC
    void* value;                 // 0x10
};

// A cursor into one of CEventHandle's keyed trees. It holds the node the
// iteration is at; advancing walks the tree in key order.
class CEventHandleTreeIterator {
public:
    // The key/value record of the node the iterator points at.
    void* GetValue();

    // Copy the current node to destination, then advance in key order.
    static void PostIncrement(CEventHandleTreeIterator* destination, CEventHandleTreeIterator* iterator, void* unused);

    EventHandleTreeNode* node; // 0x0
};

typedef char CEventHandleTreeIterator_size[(sizeof(CEventHandleTreeIterator) == 4) ? 1 : -1];

#endif // CLASSES_CEVENTHANDLETREEITERATOR_H
