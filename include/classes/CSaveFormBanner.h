// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSAVEFORMBANNER_H
#define CLASSES_CSAVEFORMBANNER_H

#include "game/types.h"

class CSaveFormBanner {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    unsigned int recordSize;                 // 0x4: 4 uses in SaveForm_ValidateRecord, SaveForm_CompareRecordBytes
    unsigned int recordCount;                 // 0x8: 1 use in SaveForm_ValidateRecord
    unsigned char hasMd5;                // 0xc: 2 uses in SaveForm_ValidateRecord
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSAVEFORMBANNER_OFF(f, o) typedef char CSaveFormBanner_off_##f[((int)&((CSaveFormBanner*)0)->f == (o)) ? 1 : -1]
CSAVEFORMBANNER_OFF(recordSize, 0x4);
CSAVEFORMBANNER_OFF(recordCount, 0x8);
CSAVEFORMBANNER_OFF(hasMd5, 0xc);
#undef CSAVEFORMBANNER_OFF

#endif  // CLASSES_CSAVEFORMBANNER_H
