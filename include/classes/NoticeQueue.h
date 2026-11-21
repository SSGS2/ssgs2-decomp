// NoticeQueue: explicit layout view; no inheritance, virtual methods, or RTTI.
// Layout view for NoticeQueue_UpdateAndDraw/80186214/801865C8/8018666C; class name is descriptive.
#ifndef CLASSES_NOTICEQUEUE_H
#define CLASSES_NOTICEQUEUE_H

#include "game/types.h"

class NoticeQueue {
public:
    char pad_000[0x10];
    void* textTemplates; // 0x10: NoticeQueue_ShowItemNotice: Text_ExpandTaggedText receiver
    void* display; // 0x14: NoticeQueue_UpdateAndDraw: text display updated and drawn
    unsigned int active; // 0x18: NoticeQueue_ResetAndActivate sets 1; NoticeQueue_UpdateAndDraw tests and clears
    void* entries; // 0x1C: NoticeQueue_Clear: entries with stride 0x18
    unsigned int entryCount; // 0x20: NoticeQueue_Clear: determines end of entries
};

#define OFF(f, o) typedef char NoticeQueue_off_##f[((int)&((NoticeQueue*)0)->f == (o)) ? 1 : -1]
OFF(textTemplates, 0x10);
OFF(display, 0x14);
OFF(active, 0x18);
OFF(entries, 0x1C);
OFF(entryCount, 0x20);
#undef OFF

#endif
