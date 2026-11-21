// CFlyInfo layout view: constructor/update in fly_info_hud.cpp, drawing in golf_hud_flight_info_and_pang_rewards.cpp.
// Kept separate from those units' existing local construction/destruction types.
#ifndef CLASSES_CFLYINFOFIELDS_H
#define CLASSES_CFLYINFOFIELDS_H
struct CFlyInfoFields {
    char pad_0[0xC];
    unsigned int color; // CFlyInfo_DrawFlightHud writes ARGB before drawing.
    int textMode; // CFlyInfo_DrawFlightHud changes numeric/text alignment modes.
    void* textBuffer; // +0x14: constructor allocates 0x20 bytes; drawing passes to sprintf.
    float elapsedTime; // +0x18: CFlyInfo_UpdateFlightFadesAndDistances adds dt, resets at shot start.
    void* groundText; // +0x1C: constructor creates text entries for ground types.
    void* pinText; // +0x20: constructor creates the pin/status text entries.
    void* dartsResultLayout; // +0x24: CSimpleSpd constructor; drawing sets element opacity.
    void* modeTwoResultLayout; // +0x28: same, alternate fly-information layout.
    void* modeTwoShotDetailLayout; // +0x2C: same, additional fly-information layout.
    char labelFont[8]; // +0x30: constructor clears eight bytes and passes to text setup.
    float groundTextOpacity;
    float pinTextOpacity;
    float secondaryPinTextOpacity;
    float carryValueOpacity;
    float carryLabelsOpacity;
    float currentShotValueOpacity;
    float shotDetailOpacities[5]; // Staggered fade values advanced by CFlyInfo_UpdateFlightFadesAndDistances.
    float dartsTotalOpacity;
    float dartsBreakdownOpacity;
    char pad_6C[8];
    float distanceFromStart; // +0x74: update takes horizontal magnitude of ball - PreData(ball, 0).
    float distanceToPin; // +0x78: update takes horizontal magnitude of ball - holeRecords[n]+0xC.
    char groundName[8]; // +0x7C: update copies ground-type text; drawing passes to groundText.
    unsigned int previousGolfState; // +0x84: update detects a rising edge of golfDoc +0x358 bit 0x80.
};
#define OFF(f, o) typedef char CFlyInfoFields_off_##f[((int)&((CFlyInfoFields*)0)->f == (o)) ? 1 : -1]
OFF(color, 0xC);
OFF(textMode, 0x10);
OFF(textBuffer, 0x14);
OFF(elapsedTime, 0x18);
OFF(groundText, 0x1C);
OFF(pinText, 0x20);
OFF(dartsResultLayout, 0x24);
OFF(modeTwoResultLayout, 0x28);
OFF(modeTwoShotDetailLayout, 0x2C);
OFF(labelFont, 0x30);
OFF(groundTextOpacity, 0x38);
OFF(pinTextOpacity, 0x3C);
OFF(secondaryPinTextOpacity, 0x40);
OFF(carryValueOpacity, 0x44);
OFF(carryLabelsOpacity, 0x48);
OFF(currentShotValueOpacity, 0x4C);
OFF(shotDetailOpacities, 0x50);
OFF(dartsTotalOpacity, 0x64);
OFF(dartsBreakdownOpacity, 0x68);
OFF(distanceFromStart, 0x74);
OFF(distanceToPin, 0x78);
OFF(groundName, 0x7C);
OFF(previousGolfState, 0x84);
#undef OFF
#define AsCFlyInfoFields(p) ((CFlyInfoFields*)(p))
#endif
