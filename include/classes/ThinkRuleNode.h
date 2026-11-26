// ThinkRuleNode: layout-only view owned by pad_swing_ctrl_and_com_think.cpp (ComConditionTree_dtor, ThinkActionTree_dtor).
// No virtual methods, constructors, or generated RTTI.
#ifndef CLASSES_THINKRULENODE_H
#define CLASSES_THINKRULENODE_H

class ThinkRuleNode {
public:
    void* left;  // 0x0: ComConditionTree_dtor/ThinkActionTree_dtor recurse through first child
    void* right; // 0x4: ComConditionTree_dtor/ThinkActionTree_dtor recurse through second child
    char pad_008[0x8];
    unsigned int stringFlags; // 0x10: destructors test bit 31 before freeing heapString
    char pad_014[0x4];
    void* heapString; // 0x18: destructors pass to appFreeUni when stringFlags bit 31 is set
};

#define OFF(f, o) typedef char ThinkRuleNode_off_##f[((int)&((ThinkRuleNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(stringFlags, 0x10);
OFF(heapString, 0x18);
#undef OFF

#endif
