// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_INTSTRINGTREENODE8013_H
#define CLASSES_INTSTRINGTREENODE8013_H

#include "game/types.h"

class IntStringTreeNode8013 {
public:
    void* left;                         // 0x0: IntStringTree_DestroySubtree / IntStringTree_DestroyValue: traverse children, then free stringBuffer when stringFlags high bit is set
    void* right;                        // 0x4: IntStringTree_DestroySubtree / IntStringTree_DestroyValue: traverse children, then free stringBuffer when stringFlags high bit is set
    void* parent;                       // 0x8: IntStringTree_DestroySubtree / IntStringTree_DestroyValue: traverse children, then free stringBuffer when stringFlags high bit is set
    int key;                            // 0xc: IntStringTree_DestroySubtree / IntStringTree_DestroyValue: traverse children, then free stringBuffer when stringFlags high bit is set
    unsigned int stringFlags;           // 0x10: IntStringTree_DestroySubtree / IntStringTree_DestroyValue: traverse children, then free stringBuffer when stringFlags high bit is set
    unsigned int stringLength;          // 0x14: IntStringTree_DestroySubtree / IntStringTree_DestroyValue: traverse children, then free stringBuffer when stringFlags high bit is set
    void* stringBuffer;                 // 0x18: IntStringTree_DestroySubtree / IntStringTree_DestroyValue: traverse children, then free stringBuffer when stringFlags high bit is set
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define INTSTRINGTREENODE8013_OFF(f, o) typedef char IntStringTreeNode8013_off_##f[((int)&((IntStringTreeNode8013*)0)->f == (o)) ? 1 : -1]
INTSTRINGTREENODE8013_OFF(right, 0x4);
INTSTRINGTREENODE8013_OFF(parent, 0x8);
INTSTRINGTREENODE8013_OFF(key, 0xc);
INTSTRINGTREENODE8013_OFF(stringFlags, 0x10);
INTSTRINGTREENODE8013_OFF(stringLength, 0x14);
INTSTRINGTREENODE8013_OFF(stringBuffer, 0x18);
INTSTRINGTREENODE8013_OFF(left, 0x0);
#undef INTSTRINGTREENODE8013_OFF

#endif  // CLASSES_INTSTRINGTREENODE8013_H
