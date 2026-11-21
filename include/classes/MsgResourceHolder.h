// MsgResourceHolder_Ctor (0x8031FC70) / Dtor (0x8031FCD0), swing_result_hud_and_equipped_stats.cpp.
// Plain layout view; ownership is established by Dtor's deleting virtual call.
#ifndef CLASSES_MSGRESOURCEHOLDER_H
#define CLASSES_MSGRESOURCEHOLDER_H

struct MsgResourceHolder {
    int unk_0;
    void* messageResource; // +0x4: output of WFixedFont_CreateMsgEcbResource; deleted by Dtor.
    void* resourceOwner; // +0x8: font +0x10 passed to WFixedFont_CreateMsgEcbResource.
};

#define OFF(f, o) typedef char MsgResourceHolder_off_##f[((int)&((MsgResourceHolder*)0)->f == (o)) ? 1 : -1]
OFF(unk_0, 0x0);
OFF(messageResource, 0x4);
OFF(resourceOwner, 0x8);
#undef OFF
#define AsMsgResourceHolder(p) ((MsgResourceHolder*)(p))

#endif
