#ifndef CLASSES_CCOURSESELECT_H
#define CLASSES_CCOURSESELECT_H

#include "game/types.h"

class CCourseSelect {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* unk_004;                      // 0x4
    char pad_008[0x10];                 // 0x8
    unsigned char selectedCourseIndex;  // 0x18
    signed char holeOrder;              // 0x19
    unsigned char weatherMode;          // 0x1a
    unsigned char holeCount;            // 0x1b
    void* holeNumbers;                  // 0x1c
    void* holeNumberCount;              // 0x20
    void* holeNumberCapacity;           // 0x24
    void* holeCourseIds;                // 0x28
    void* holeCourseCount;              // 0x2c
    void* holeCourseCapacity;           // 0x30
    unsigned int selectionMode;         // 0x34
    void* layout;                       // 0x38
    void* scriptObject;                 // 0x3c
    int bpEntryId;                      // 0x40
    void* courseTextures;               // 0x44
    void* auxiliaryTexture;             // 0x48
    void* coursePage;                   // 0x4c
    int selectionDelayFrames;           // 0x50
    unsigned char selectionComplete;    // 0x54
    char pad_055[0x3];                  // 0x55
    void* sharedFile;                   // 0x58
    void* fixedFont;                    // 0x5c
    void* text;                         // 0x60
    void* messageRenderer;              // 0x64
    void* textTable;                    // 0x68
    void* holeNoSelect;                 // 0x6c
    unsigned char holeSelectorActive;   // 0x70
    char pad_071[0x3];                  // 0x71
    void* miniGameHole;                 // 0x74
    void* texturePack;                  // 0x78
    unsigned char weatherOptionEnabled; // 0x7c
    unsigned char exitRequested;        // 0x7d

    CCourseSelect();
    void* Destroy(int flag);

    void SetMode(int mode);
    int SetWeatherSelectionEnabled(unsigned char value);
    void UpdateLayoutAndTexture();
    void Draw();
    int UpdateVisiblePageGroups();
    int UpdateCourseText();
    int DrawCourseCards();
    int SelectCourseByElementId(int elementId);
    int HandleElement44Activation();
    void StartMenuFadeOut();
    int AreResourcesLoaded();
    int IsButtonAvailable(int elementId);
    int UpdateCourseUnlockStates();

    int UpdateHoleSelector();
    int GetSelectionResult(char* out);
    int ResetOtherButtonsInGroup(int elementId);
    int SetCoursePage(int pageIndex);
    int InitializeButtonStates();
    int UpdateSelection();
    int HandleActivatedElements();
    int RefreshOrderOptionButtons();
};

#define CCOURSESELECT_OFF(f, o) typedef char CCourseSelect_off_##f[((int)&((CCourseSelect*)0)->f == (o)) ? 1 : -1]
CCOURSESELECT_OFF(unk_004, 0x4);
CCOURSESELECT_OFF(selectedCourseIndex, 0x18);
CCOURSESELECT_OFF(holeOrder, 0x19);
CCOURSESELECT_OFF(weatherMode, 0x1a);
CCOURSESELECT_OFF(holeCount, 0x1b);
CCOURSESELECT_OFF(holeNumbers, 0x1c);
CCOURSESELECT_OFF(holeNumberCount, 0x20);
CCOURSESELECT_OFF(holeNumberCapacity, 0x24);
CCOURSESELECT_OFF(holeCourseIds, 0x28);
CCOURSESELECT_OFF(holeCourseCount, 0x2c);
CCOURSESELECT_OFF(holeCourseCapacity, 0x30);
CCOURSESELECT_OFF(selectionMode, 0x34);
CCOURSESELECT_OFF(layout, 0x38);
CCOURSESELECT_OFF(scriptObject, 0x3c);
CCOURSESELECT_OFF(bpEntryId, 0x40);
CCOURSESELECT_OFF(courseTextures, 0x44);
CCOURSESELECT_OFF(auxiliaryTexture, 0x48);
CCOURSESELECT_OFF(coursePage, 0x4c);
CCOURSESELECT_OFF(selectionDelayFrames, 0x50);
CCOURSESELECT_OFF(selectionComplete, 0x54);
CCOURSESELECT_OFF(sharedFile, 0x58);
CCOURSESELECT_OFF(fixedFont, 0x5c);
CCOURSESELECT_OFF(text, 0x60);
CCOURSESELECT_OFF(messageRenderer, 0x64);
CCOURSESELECT_OFF(textTable, 0x68);
CCOURSESELECT_OFF(holeNoSelect, 0x6c);
CCOURSESELECT_OFF(holeSelectorActive, 0x70);
CCOURSESELECT_OFF(miniGameHole, 0x74);
CCOURSESELECT_OFF(texturePack, 0x78);
CCOURSESELECT_OFF(weatherOptionEnabled, 0x7c);
CCOURSESELECT_OFF(exitRequested, 0x7d);
#undef CCOURSESELECT_OFF

#endif // CLASSES_CCOURSESELECT_H
