// The club upgrade dialog: the panel that shows a club's current and next
// level, its stat columns and its price, and asks the player to confirm or
// cancel the upgrade.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CCLUBLVUPDIALOG_H
#define CLASSES_CCLUBLVUPDIALOG_H

#include "game/types.h"

class CClubLvUpDialog {
public:
    CClubLvUpDialog(void* playerRecord);
    ~CClubLvUpDialog();

    // The panel's own state.
    int UpdateLayout();
    int InitLayout();
    int Update();
    int UpdateSelectionDelay();
    int Draw();
    int Close();

    // Choosing a club and showing its upgrade.
    int SetClubIndex(int index);
    int SetClubIcon(int icon);
    int Show();
    int RefreshStatsAndMessage(int value);
    int UpdateUpgradeValues();
    int ProcessInput();

    void* vtbl;               // 0x00
    void* layout;             // 0x04
    int bpObject;             // 0x08  the script object that draws the panel
    int bpEntryId;            // 0x0C
    int clubIcon;             // 0x10
    void* playerData;         // 0x14  the save record the club levels come from
    void* clubHouseMessage;   // 0x18
    void* textTable;          // 0x1C
    int selectionDelayFrames; // 0x20  click feedback countdown, -1 when idle
    int selectedElement;      // 0x24
    int result;               // 0x28  -1 until the player chooses
    int clubIndex;            // 0x2C
};

#define CCLUBLVUPDIALOG_OFF(f, o) typedef char CClubLvUpDialog_off_##f[((int)&((CClubLvUpDialog*)0)->f == (o)) ? 1 : -1]
CCLUBLVUPDIALOG_OFF(layout, 0x4);
CCLUBLVUPDIALOG_OFF(bpObject, 0x8);
CCLUBLVUPDIALOG_OFF(bpEntryId, 0xc);
CCLUBLVUPDIALOG_OFF(clubIcon, 0x10);
CCLUBLVUPDIALOG_OFF(playerData, 0x14);
CCLUBLVUPDIALOG_OFF(clubHouseMessage, 0x18);
CCLUBLVUPDIALOG_OFF(textTable, 0x1c);
CCLUBLVUPDIALOG_OFF(selectionDelayFrames, 0x20);
CCLUBLVUPDIALOG_OFF(selectedElement, 0x24);
CCLUBLVUPDIALOG_OFF(result, 0x28);
CCLUBLVUPDIALOG_OFF(clubIndex, 0x2c);
#undef CCLUBLVUPDIALOG_OFF
typedef char CClubLvUpDialog_size[(sizeof(CClubLvUpDialog) == 0x30) ? 1 : -1];

#endif // CLASSES_CCLUBLVUPDIALOG_H
