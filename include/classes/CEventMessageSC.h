// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CEVENTMESSAGESC_H
#define CLASSES_CEVENTMESSAGESC_H

#include "game/types.h"

class CEventMessageSC {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int unk_4;                          // 0x4: 4 uses in CEventMessageSC_MSG_REQ_UpdateMessage, CEventMessageSC_MSG_WIN_FinishWindowScript (CONFLICT sizes [1, 4])
    int scriptCompletionLatched;                          // 0x8: 4 uses in CEventMessageSC_MSG_REQ_UpdateMessage
    int messageDrawPrepared;                          // 0xc: 2 uses in CEventMessageSC_MSG_REQ_UpdateMessage
    int inputDisabled;                         // 0x10: 2 uses in CEventMessageSC_MSG_REQ_UpdateMessage
    unsigned int messageStorage;                // 0x14: 2 uses in CEventMessageSC_MSG_REQ_UpdateMessage, CEventMessageSC_MSG_REQ_dtor
    char pad_018[0x4];                  // 0x18
    void* messageHeapData;                       // 0x1c: 2 uses in CEventMessageSC_MSG_REQ_UpdateMessage, CEventMessageSC_MSG_REQ_dtor
    unsigned char advanceInputState;               // 0x20: 5 uses in CEventMessageSC_MSG_REQ_UpdateMessage
    unsigned char advanceInputRequired;               // 0x21: 3 uses in CEventMessageSC_MSG_REQ_UpdateMessage

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x16
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CEVENTMESSAGESC_H
