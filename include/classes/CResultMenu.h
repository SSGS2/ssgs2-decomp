// CResultMenu: methods live in result_menu_and_fight_result.cpp.
// Non-virtual layout view; original function addresses are cited per field.
#ifndef CLASSES_CRESULTMENU_H
#define CLASSES_CRESULTMENU_H

namespace class_layout {
class CResultMenu {
public:
    char pad_000[0x18];
    int resultMode; // 0x18: CResultMenu_UpdateTaggedResultText literal load/store
    void* layout; // 0x1C: CResultMenu_PollLoadComplete literal load/store
    char pad_020[0x4];
    void* resource; // 0x24: CResultMenu_UpdateTaggedResultText literal load/store
    char pad_028[0x4];
    void* progressBpObject; // 0x2C: CResultMenu_UpdateProgressAndTotals literal load/store
    int resultTexture; // 0x30: CResultMenu_UpdateProgressAndTotals literal load/store
    void* uiTextTable; // 0x34: CResultMenu_UpdateProgressAndTotals literal load/store
    char pad_038[0x4];
    void* taggedText; // 0x3C: CResultMenu_UpdateTaggedResultText literal load/store
    void* text; // 0x40: CResultMenu_UpdateTaggedResultText literal load/store
    char pad_044[0x2C];
    float currentProgress; // 0x70: CResultMenu_UpdateProgressAndTotals literal load/store
    float initialSegmentProgress; // 0x74: CResultMenu_UpdateProgressAndTotals literal load/store
    int segmentLength; // 0x78: CResultMenu_UpdateProgressAndTotals literal load/store
    int segmentIndex; // 0x7C: CResultMenu_UpdateProgressAndTotals literal load/store
    char pad_080[0x8];
    float displayElapsedTime; // 0x88: CResultMenu_UpdateProgressAndTotals literal load/store
    int progressComplete; // 0x8C: CResultMenu_UpdateProgressAndTotals literal load/store
    int totalsDisplayed; // 0x90: CResultMenu_UpdateProgressAndTotals literal load/store
    char pad_094[0x4];
    float savedStreamVolume; // 0x98: CResultMenu_UpdateProgressAndTotals literal load/store
    char pad_09C[0x8];
    unsigned int score; // 0xA4: CResultMenu_SetResultData literal load/store
    unsigned int earnedPang; // 0xA8: CResultMenu_SetResultData literal load/store
    unsigned int earnedBonusPang; // 0xAC: CResultMenu_SetResultData literal load/store
    unsigned int doublePangEnabled; // 0xB0: CResultMenu_SetResultData literal load/store
    unsigned int remainingProgress; // 0xB4: CResultMenu_SetResultData literal load/store
    unsigned int normalTotalProgress; // 0xB8: CResultMenu_SetResultData literal load/store
    unsigned int roundHoleCount; // 0xBC: CResultMenu_SetResultData literal load/store
    unsigned int currentHoleIndex; // 0xC0: CResultMenu_SetResultData literal load/store
    unsigned int resultMessageIndex; // 0xC4: CResultMenu_SetResultData literal load/store
    unsigned short specialTotalProgress; // 0xC8: CResultMenu_SetResultData literal load/store
    char pad_0CA[0x2];
    int specialMode; // 0xCC: CResultMenu_UpdateTaggedResultText literal load/store
};

#define CRESULTMENU_OFF(f, o) typedef char CResultMenu_off_##f[((int)&((CResultMenu*)0)->f == (o)) ? 1 : -1]
CRESULTMENU_OFF(resultMode, 0x18);
CRESULTMENU_OFF(layout, 0x1C);
CRESULTMENU_OFF(resource, 0x24);
CRESULTMENU_OFF(progressBpObject, 0x2C);
CRESULTMENU_OFF(resultTexture, 0x30);
CRESULTMENU_OFF(uiTextTable, 0x34);
CRESULTMENU_OFF(taggedText, 0x3C);
CRESULTMENU_OFF(text, 0x40);
CRESULTMENU_OFF(currentProgress, 0x70);
CRESULTMENU_OFF(initialSegmentProgress, 0x74);
CRESULTMENU_OFF(segmentLength, 0x78);
CRESULTMENU_OFF(segmentIndex, 0x7C);
CRESULTMENU_OFF(displayElapsedTime, 0x88);
CRESULTMENU_OFF(progressComplete, 0x8C);
CRESULTMENU_OFF(totalsDisplayed, 0x90);
CRESULTMENU_OFF(savedStreamVolume, 0x98);
CRESULTMENU_OFF(score, 0xA4);
CRESULTMENU_OFF(earnedPang, 0xA8);
CRESULTMENU_OFF(earnedBonusPang, 0xAC);
CRESULTMENU_OFF(doublePangEnabled, 0xB0);
CRESULTMENU_OFF(remainingProgress, 0xB4);
CRESULTMENU_OFF(normalTotalProgress, 0xB8);
CRESULTMENU_OFF(roundHoleCount, 0xBC);
CRESULTMENU_OFF(currentHoleIndex, 0xC0);
CRESULTMENU_OFF(resultMessageIndex, 0xC4);
CRESULTMENU_OFF(specialTotalProgress, 0xC8);
CRESULTMENU_OFF(specialMode, 0xCC);
#undef CRESULTMENU_OFF

} // namespace class_layout
#endif // CLASSES_CRESULTMENU_H
