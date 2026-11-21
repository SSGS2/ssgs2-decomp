// ThinkRuleTree: layout-only view owned by pad_swing_ctrl_and_com_think.cpp (ComConditionTree_Init, ComActionTree_Init and destructors).
// No virtual methods, constructors, or generated RTTI.
#ifndef CLASSES_THINKRULETREE_H
#define CLASSES_THINKRULETREE_H

class ThinkRuleTree {
public:
    void* first; // 0x0: ComConditionTree_Init clears first link
    char* root; // 0x4: ComConditionTree_dtor/ThinkActionTree_dtor start recursive cleanup here
    void* current; // 0x8: ComConditionTree_Init sets to address of root
};

#define OFF(f, o) typedef char ThinkRuleTree_off_##f[((int)&((ThinkRuleTree*)0)->f == (o)) ? 1 : -1]
OFF(first, 0x0);
OFF(root, 0x4);
OFF(current, 0x8);
#undef OFF

#endif
