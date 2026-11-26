// A localized message box: a text renderer, the message group it reads from, and the
// shared message file that holds the text.
#ifndef CLASSES_CCLUBHOUSEMSG_H
#define CLASSES_CCLUBHOUSEMSG_H

class CClubHouseMsg {
public:
    ~CClubHouseMsg();
    CClubHouseMsg();

    int SetTextFromGroup(void* messageKey, void* sourceGroup, int resizeToGroup);
    int UpdateReveal();
    int DrawText();
    int ResetTextState();
    int SetDrawEnabled(int value);
    int IsResourceLoaded();
    int CreateTextFromResource();

    void* vtbl;              // 0x0: original vtable pointer
    void* sharedFile;        // 0x4
    void* group;             // 0x8
    void* layoutManager;     // 0xc
    void* textObject;        // 0x10
    unsigned int textFlags;  // 0x14
    unsigned int textLength; // 0x18
    void* textData;          // 0x1c
};

#define CCLUBHOUSEMSG_OFF(f, o) typedef char CClubHouseMsg_off_##f[((int)&((CClubHouseMsg*)0)->f == (o)) ? 1 : -1]
CCLUBHOUSEMSG_OFF(vtbl, 0x0);
CCLUBHOUSEMSG_OFF(sharedFile, 0x4);
CCLUBHOUSEMSG_OFF(group, 0x8);
CCLUBHOUSEMSG_OFF(layoutManager, 0xc);
CCLUBHOUSEMSG_OFF(textObject, 0x10);
CCLUBHOUSEMSG_OFF(textFlags, 0x14);
CCLUBHOUSEMSG_OFF(textLength, 0x18);
CCLUBHOUSEMSG_OFF(textData, 0x1c);
#undef CCLUBHOUSEMSG_OFF
typedef char CClubHouseMsg_size[(sizeof(CClubHouseMsg) == 0x20) ? 1 : -1];

#endif
