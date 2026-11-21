// NoticeEntry: explicit layout view; no inheritance, virtual methods, or RTTI.
// Entry view for notice functions in input_manager_and_notice_queue.cpp; each entry has stride 0x18.
#ifndef CLASSES_NOTICEENTRY_H
#define CLASSES_NOTICEENTRY_H

#include "game/types.h"

class NoticeEntry {
public:
    unsigned int stringFlags; // 0x0: NoticeQueue_ShowItemNotice: high bit selects allocated storage
    unsigned int stringLength; // 0x4: NoticeQueue_UpdateAndDraw: allocated string length
    void* stringData; // 0x8: NoticeStringVector_dtor: allocated string freed
    float remainingDisplayTime; // 0xC: NoticeQueue_ShowItemNotice: set from kFloat_0_Notice
    unsigned int active; // 0x10: NoticeQueue_UpdateAndDraw: last active entry selected for display
    int persistent; // 0x14: NoticeQueue_ShowItemNotice: set to 1
};

#define OFF(f, o) typedef char NoticeEntry_off_##f[((int)&((NoticeEntry*)0)->f == (o)) ? 1 : -1]
OFF(stringFlags, 0x0);
OFF(stringLength, 0x4);
OFF(stringData, 0x8);
OFF(remainingDisplayTime, 0xC);
OFF(active, 0x10);
OFF(persistent, 0x14);
#undef OFF

#endif
