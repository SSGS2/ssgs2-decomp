#ifndef CLASSES_CPLAYERSTATUS_H
#define CLASSES_CPLAYERSTATUS_H

#include "game/types.h"

class CPlayerStatus {
public:
    void* vtbl;              // 0x0: original vtable pointer
    void* playerRecord;      // 0x4
    void* layout;            // 0x8
    char pad_00C[0x40];      // 0xc
    void* equipmentTextures; // 0x4c
    void*
        caddieTextures; // 0x50: CPlayerStatus_UpdateEquipmentTextures (0x802E0E10), player record +0x2F caddie selection
    void* clubSetTextures;        // 0x54: same function, player record +0x2D selection
    void* ballTextures;           // 0x58: same function, player record +0x2E selection
    int selectedEquipmentTexture; // 0x5c
    void* selectedCaddieTexture;  // 0x60: CPlayerStatus_UpdateEquipmentTextures; index -1 selects fallback entry 8
    void* selectedClubSetTexture; // 0x64: CPlayerStatus_UpdateEquipmentTextures, 0x28-byte texture entry
    void* selectedBallTexture;    // 0x68: CPlayerStatus_UpdateEquipmentTextures, 0x28-byte texture entry
    unsigned char power;          // 0x6c
    unsigned char control;        // 0x6d
    unsigned char spin;           // 0x6e
    char pad_06F[0x29];           // 0x6f
    int scriptObject;             // 0x98
    int scriptEntryId;            // 0x9c
    void* numberDraw;             // 0xa0
    void* clubHouseMessage;       // 0xa4

    int Update();
    int Draw();
    int SetEnabled(int enabled);
    int UpdateLayout();
    int PollResourcesLoaded();
    int UpdateEquipmentTextures();
    static int InitEquipmentTextures(void* status);
    static int RefreshEquipmentDisplay(void* status, int nameMode);
    static int InitEquipmentDisplay(void* status, int nameMode);

    void* Init(void* playerRecord);
    ~CPlayerStatus();
};

#define CPLAYERSTATUS_OFF(f, o) typedef char CPlayerStatus_off_##f[((int)&((CPlayerStatus*)0)->f == (o)) ? 1 : -1]
CPLAYERSTATUS_OFF(playerRecord, 0x4);
CPLAYERSTATUS_OFF(layout, 0x8);
CPLAYERSTATUS_OFF(equipmentTextures, 0x4c);
CPLAYERSTATUS_OFF(caddieTextures, 0x50);
CPLAYERSTATUS_OFF(clubSetTextures, 0x54);
CPLAYERSTATUS_OFF(ballTextures, 0x58);
CPLAYERSTATUS_OFF(selectedCaddieTexture, 0x60);
CPLAYERSTATUS_OFF(selectedClubSetTexture, 0x64);
CPLAYERSTATUS_OFF(selectedBallTexture, 0x68);
CPLAYERSTATUS_OFF(selectedEquipmentTexture, 0x5c);
CPLAYERSTATUS_OFF(power, 0x6c);
CPLAYERSTATUS_OFF(control, 0x6d);
CPLAYERSTATUS_OFF(spin, 0x6e);
CPLAYERSTATUS_OFF(scriptObject, 0x98);
CPLAYERSTATUS_OFF(scriptEntryId, 0x9c);
CPLAYERSTATUS_OFF(numberDraw, 0xa0);
CPLAYERSTATUS_OFF(clubHouseMessage, 0xa4);
#undef CPLAYERSTATUS_OFF

#endif // CLASSES_CPLAYERSTATUS_H
