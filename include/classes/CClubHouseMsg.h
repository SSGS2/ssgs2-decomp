// Layout view of CClubHouseMsg; the constructed type and its cleanup remain in item_info_and_clubhouse_msg.cpp.
// Evidence: constructor 0x802D9D18, destructor 0x802D9DCC, helpers 0x802D9F1C-0x802DA1BC.
#ifndef CLASSES_CCLUBHOUSEMSG_H
#define CLASSES_CCLUBHOUSEMSG_H
class CClubHouseMsgFields {
public:
    void* vtbl; // 0x0
    void* sharedFile; // 0x4
    void* group; // 0x8
    void* layoutManager; // 0xc
    void* textObject; // 0x10
    unsigned int textFlags; // 0x14
    unsigned int textLength; // 0x18
    void* textData; // 0x1c
};
#define CCLUBHOUSEMSG_OFF(f, o) typedef char CClubHouseMsg_off_##f[((int)&((CClubHouseMsgFields*)0)->f == (o)) ? 1 : -1]
CCLUBHOUSEMSG_OFF(vtbl, 0x0);
CCLUBHOUSEMSG_OFF(sharedFile, 0x4);
CCLUBHOUSEMSG_OFF(group, 0x8);
CCLUBHOUSEMSG_OFF(layoutManager, 0xc);
CCLUBHOUSEMSG_OFF(textObject, 0x10);
CCLUBHOUSEMSG_OFF(textFlags, 0x14);
CCLUBHOUSEMSG_OFF(textLength, 0x18);
CCLUBHOUSEMSG_OFF(textData, 0x1c);
#undef CCLUBHOUSEMSG_OFF
#endif
