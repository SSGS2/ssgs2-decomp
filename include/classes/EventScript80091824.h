// Event-script state used by the handlers in event_conte_script_commands.cpp.
// Partial memory layout only; no constructors, inheritance, or virtual methods.
#ifndef CLASSES_EVENTSCRIPT80091824_H
#define CLASSES_EVENTSCRIPT80091824_H

class EventScript80091824 {
public:
    unsigned char conditionCombineMode;
    unsigned char counter; // CEventConteProc_ProcessIfCommand: nesting depth, limited to 15
    volatile unsigned short flags;
    volatile unsigned short extraFlags;
    unsigned char unk006[0x40e];
    volatile int lengthBase; // CEventConteProc_ProcessGotoCommand: base for the encoded jump target
    volatile int cursor; // CEventConteProc_SkipCommand8 et al.: advances by command size
    volatile int count; // CEventConteProc_ProcessCameraSetCommand: saturating source-update counter
    void* activeSource; // CEventConteProc_ProcessCameraSetCommand: new CEventCameraSC_CAM_SET, update, delete/transfer
    char pad_424[0x10];
    void** queuedItems; // CEventConteProc_ProcessCameraSetCommand: array receiving new CEventConteProc nodes
    unsigned int queuedCount; // incremented on append, restored after buffer growth
    unsigned int queuedCapacity; // compared to count and used for buffer growth
};

#define EVENTSCRIPT80091824_OFF(f, o) typedef char EventScript80091824_off_##f[((int)&((EventScript80091824*)0)->f == (o)) ? 1 : -1]
EVENTSCRIPT80091824_OFF(conditionCombineMode, 0x0);
EVENTSCRIPT80091824_OFF(counter, 0x1);
EVENTSCRIPT80091824_OFF(flags, 0x2);
EVENTSCRIPT80091824_OFF(extraFlags, 0x4);
EVENTSCRIPT80091824_OFF(unk006, 0x6);
EVENTSCRIPT80091824_OFF(lengthBase, 0x414);
EVENTSCRIPT80091824_OFF(cursor, 0x418);
EVENTSCRIPT80091824_OFF(count, 0x41c);
EVENTSCRIPT80091824_OFF(activeSource, 0x420);
EVENTSCRIPT80091824_OFF(queuedItems, 0x434);
EVENTSCRIPT80091824_OFF(queuedCount, 0x438);
EVENTSCRIPT80091824_OFF(queuedCapacity, 0x43c);
#undef EVENTSCRIPT80091824_OFF

#endif
