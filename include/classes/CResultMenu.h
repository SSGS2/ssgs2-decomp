// CResultMenu: the stroke and match result screens. The methods live in
// result_menu_and_fight_result.cpp.
//
// The class has no virtual functions on purpose: its vtable is still taken from
// the original game data, and the constructor stores it in the base's storage.
#ifndef CLASSES_CRESULTMENU_H
#define CLASSES_CRESULTMENU_H

extern "C" void MPBase1_Ctor(void*);

// The menu-process base. Its first word is where the vtable pointer is stored.
struct CWarningLogoBase {
    void* vtbl; // 0x00
    char pad_04[4];
    int active; // 0x08
    char pad_0C[0xC];
    CWarningLogoBase() {
        MPBase1_Ctor(this);
    }
    ~CWarningLogoBase();
};

struct ResultMenuData;

class CResultMenu : public CWarningLogoBase {
public:
    CResultMenu(void* arg);

    // Release the textures, text, numeric entries, BP entries and layout. A
    // shouldDelete > 0 also deletes the object, so the returned pointer may then be dangling.
    void* Destroy(int shouldDelete);
    // Update the result layout while resources are loading.
    void UpdateLayout();
    // Report whether the layout, BP resources and optional texture are ready.
    int PollLoadComplete();
    // Draw the result background, texture preview, numeric text and progress.
    void Draw();
    // Start the result scripts for this mode and placing.
    void SelectStateScript();
    // Build the stroke/match message and set its position, size and colour.
    void UpdateTaggedResultText();
    // Reveal the totals and advance the rank progress. Returns 1 once the totals are shown.
    int UpdateProgressAndTotals(float deltaSeconds);
    // Choose the input player, register the numeric entries and pick the rank interval.
    void InitRankProgress();
    // Per-frame update. Returns 1 when the screen should be dismissed.
    int UpdateUntilDismissed(float dt);
    // Copy the result payload into the menu and load the placing texture if one applies.
    void SetResultData(ResultMenuData* config);

    int resultMode;         // 0x18
    void* layout;           // 0x1C
    int mainBpEntryId;      // 0x20
    void* mainBpObject;     // 0x24
    int progressBpEntryId;  // 0x28
    void* progressBpObject; // 0x2C
    void* resultTexture;    // 0x30
    void* uiTextTable;      // 0x34
    void* textFactory;      // 0x38
    void* taggedText;       // 0x3C
    // `text` is followed by the eleven rank-interval end thresholds; the words view addresses them together.
    union {
        struct {
            void* text;                // 0x40
            int segmentThresholds[11]; // 0x44
        };
        int textAndThresholdWords[12];
    };
    float currentProgress;               // 0x70
    float initialSegmentProgress;        // 0x74
    int segmentLength;                   // 0x78
    int segmentIndex;                    // 0x7C
    int initialized;                     // 0x80
    int headerDisplayed;                 // 0x84
    float displayElapsedTime;            // 0x88
    int progressComplete;                // 0x8C
    int totalsDisplayed;                 // 0x90
    int completionFrames;                // 0x94
    float savedStreamVolume;             // 0x98
    int placing;                         // 0x9C
    int textureIndex;                    // 0xA0
    unsigned int score;                  // 0xA4
    unsigned int earnedPang;             // 0xA8
    unsigned int earnedBonusPang;        // 0xAC
    unsigned int doublePangEnabled;      // 0xB0
    unsigned int remainingProgress;      // 0xB4
    unsigned int normalTotalProgress;    // 0xB8
    unsigned int roundHoleCount;         // 0xBC
    unsigned int currentHoleIndex;       // 0xC0
    unsigned int resultMessageIndex;     // 0xC4
    unsigned short specialTotalProgress; // 0xC8
    char pad_0CA[0x2];
    int specialMode; // 0xCC
};

#define CRESULTMENU_OFF(f, o) typedef char CResultMenu_off_##f[((int)&((CResultMenu*)0)->f == (o)) ? 1 : -1]
CRESULTMENU_OFF(resultMode, 0x18);
CRESULTMENU_OFF(layout, 0x1C);
CRESULTMENU_OFF(mainBpObject, 0x24);
CRESULTMENU_OFF(progressBpObject, 0x2C);
CRESULTMENU_OFF(resultTexture, 0x30);
CRESULTMENU_OFF(text, 0x40);
CRESULTMENU_OFF(currentProgress, 0x70);
CRESULTMENU_OFF(displayElapsedTime, 0x88);
CRESULTMENU_OFF(savedStreamVolume, 0x98);
CRESULTMENU_OFF(score, 0xA4);
CRESULTMENU_OFF(remainingProgress, 0xB4);
CRESULTMENU_OFF(specialTotalProgress, 0xC8);
CRESULTMENU_OFF(specialMode, 0xCC);
#undef CRESULTMENU_OFF

#endif // CLASSES_CRESULTMENU_H
