// CHoleInfo: the hole-introduction panel in the HUD.
//
// It animates the course, hole/par and player-order labels from the middle of
// the screen into their corner positions, then dims them, and it owns the
// overlays and the nine player textures the panel draws.
//
// The class is not declared with virtual functions: its vtable comes from the
// original data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CHOLEINFO_H
#define CLASSES_CHOLEINFO_H

#include "game/types.h"
#include "classes/CInfoBase.h"
#include "classes/HudTextPanel.h"

class CHoleInfo : public CInfoBase {
public:
    CHoleInfo();
    ~CHoleInfo();

    void UpdateIntroAnimation(float amount);
    void StartFlash();
    void ResetIntroAndSelectPlayer();
    void ShowCornerLabels();
    void DrawCourseAndPlayerPanels();

    // Load the panel's font through the HUD text panel vtable.
    void SetFont(const char* path) {
        ((HudTextPanelVTable*)this)->loadFont(path, 0, 0);
    }

    float courseStartX;           // 0x18
    float courseStartY;           // 0x1C
    float courseEndX;             // 0x20
    float courseEndY;             // 0x24
    float courseX;                // 0x28
    float courseY;                // 0x2C
    float courseScale;            // 0x30
    float holeStartX;             // 0x34  first half of the course rectangle
    float holeStartY;             // 0x38
    float holeEndX;               // 0x3C
    float holeEndY;               // 0x40
    float holeX;                  // 0x44
    float holeY;                  // 0x48
    float holeScale;              // 0x4C
    float playerStartX;           // 0x50  first half of the player rectangle
    float playerStartY;           // 0x54
    float playerEndX;             // 0x58
    float playerEndY;             // 0x5C
    float playerX;                // 0x60
    float playerY;                // 0x64
    float playerScale;            // 0x68
    char pad_06C[0x1C];           // 0x6C
    float courseOpacity;          // 0x88
    float playerOpacity;          // 0x8C
    float animationTime;          // 0x90  seconds in fade/flash, fraction while moving
    int flashCount;               // 0x94
    int animationState;           // 0x98
    void* courseName;             // 0x9C
    void* alternateCourseName;    // 0xA0
    void* holeAndPar;             // 0xA4
    void* turnOrder;              // 0xA8
    void* optionalOverlayAC;      // 0xAC
    void* optionalSlotB0;         // 0xB0
    void* completion;             // 0xB4
    void* scoreEvent;             // 0xB8
    char pad_0BC[0x1C];           // 0xBC
    void* playerTextures;         // 0xD8  array of nine textures, 0x28 bytes each
    unsigned char selectedPlayer; // 0xDC
    char pad_0DD[3];              // 0xDD
};

#define CHOLEINFO_OFF(f, o) typedef char CHoleInfo_off_##f[((int)&((CHoleInfo*)0)->f == (o)) ? 1 : -1]
CHOLEINFO_OFF(courseStartX, 0x18);
CHOLEINFO_OFF(courseX, 0x28);
CHOLEINFO_OFF(holeStartX, 0x34);
CHOLEINFO_OFF(playerStartX, 0x50);
CHOLEINFO_OFF(playerScale, 0x68);
CHOLEINFO_OFF(courseOpacity, 0x88);
CHOLEINFO_OFF(animationState, 0x98);
CHOLEINFO_OFF(courseName, 0x9C);
CHOLEINFO_OFF(completion, 0xB4);
CHOLEINFO_OFF(playerTextures, 0xD8);
CHOLEINFO_OFF(selectedPlayer, 0xDC);
#undef CHOLEINFO_OFF
typedef char CHoleInfo_size[(sizeof(CHoleInfo) == 0xE0) ? 1 : -1];

#endif // CLASSES_CHOLEINFO_H
