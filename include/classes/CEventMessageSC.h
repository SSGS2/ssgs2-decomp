#ifndef CLASSES_CEVENTMESSAGESC_H
#define CLASSES_CEVENTMESSAGESC_H

#include "game/types.h"

class CEventMessageSC {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int unk_4;                          // 0x4
    int scriptCompletionLatched;        // 0x8
    int messageDrawPrepared;            // 0xc
    int inputDisabled;                  // 0x10
    unsigned int messageStorage;        // 0x14
    char pad_018[0x4];                  // 0x18
    void* messageHeapData;              // 0x1c
    unsigned char advanceInputState;    // 0x20
    unsigned char advanceInputRequired; // 0x21

    int MSG_REQ_UpdateMessage();
};

#define CEVENTMESSAGESC_OFF(f, o) typedef char CEventMessageSC_off_##f[((int)&((CEventMessageSC*)0)->f == (o)) ? 1 : -1]
CEVENTMESSAGESC_OFF(unk_4, 0x4);
CEVENTMESSAGESC_OFF(scriptCompletionLatched, 0x8);
CEVENTMESSAGESC_OFF(messageDrawPrepared, 0xc);
CEVENTMESSAGESC_OFF(inputDisabled, 0x10);
CEVENTMESSAGESC_OFF(messageStorage, 0x14);
CEVENTMESSAGESC_OFF(messageHeapData, 0x1c);
CEVENTMESSAGESC_OFF(advanceInputState, 0x20);
CEVENTMESSAGESC_OFF(advanceInputRequired, 0x21);
#undef CEVENTMESSAGESC_OFF

#endif // CLASSES_CEVENTMESSAGESC_H
