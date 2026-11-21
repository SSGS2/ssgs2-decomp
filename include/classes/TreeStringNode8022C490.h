// Node layout read by CComThinkCtrl_ClearStringTree/ComConditionMap_Clear in com_think_ctrl_conditions.cpp.
#ifndef CLASSES_TREESTRINGNODE8022C490_H
#define CLASSES_TREESTRINGNODE8022C490_H
#include "game/types.h"
class TreeStringNode8022C490 {
public:
    void* left; // 0x0: recursive subtree cleanup.
    void* right; // 0x4: recursive subtree cleanup.
    unsigned int parentAndColor; // 0x8: tree search/rebalance representation.
    int key; // 0xc: 0x8022D23C compares request key.
    unsigned int stringFlags; // 0x10: high bit gates freeing heapText.
    unsigned int textLength; // 0x14: long-string representation (D7String).
    void* heapText; // 0x18: ComConditionMap_Clear passes to appFreeUni.
    void* value; // 0x1c: CComThinkCtrl_ProcessRequest calls through this slot; D7Value also uses it as a word.
};
#define TREESTRINGNODE8022C490_OFF(f,o) typedef char TreeStringNode8022C490_off_##f[((int)&((TreeStringNode8022C490*)0)->f == (o)) ? 1 : -1]
TREESTRINGNODE8022C490_OFF(left,0x0);
TREESTRINGNODE8022C490_OFF(right,0x4);
TREESTRINGNODE8022C490_OFF(parentAndColor,0x8);
TREESTRINGNODE8022C490_OFF(key,0xc);
TREESTRINGNODE8022C490_OFF(stringFlags,0x10);
TREESTRINGNODE8022C490_OFF(textLength,0x14);
TREESTRINGNODE8022C490_OFF(heapText,0x18);
TREESTRINGNODE8022C490_OFF(value,0x1c);
#undef TREESTRINGNODE8022C490_OFF
#endif
