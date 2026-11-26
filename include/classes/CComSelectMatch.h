#ifndef CLASSES_CCOMSELECTMATCH_H
#define CLASSES_CCOMSELECTMATCH_H

#include "game/types.h"

struct EncodedWideStringFields;

// Computer-match selection screen. The layout is the common selection-base storage
// followed by the match table, the eight match layouts, the three result-category
// layouts, the shared files and the message renderer.
class CComSelectMatch {
public:
    CComSelectMatch();
    ~CComSelectMatch();

    void ExpandMatchDescriptionText(int descriptionIndex, void* description);
    void CreateBallItemDialog(void** out);
    void DescribeMatchSelection(void* matchIndex, void* description);

    void RebuildMessageDraw();
    int AreResourcesLoaded();
    int GetOpponentPortraitLayout(int arg);
    void GetRowGroupPosition(int index, int arg);
    void GetCategoryGroupPosition(int index, int arg);
    void GetCourseDescription(int arg, EncodedWideStringFields* encoded);
    int GetResultCategoryLayout(int matchSlot);
    static int GetFirstIncompletePage();
    int BuildRoundSettings(void* roundSettings);

    void* vtbl;             // 0x0: original vtable pointer
    char pad_004[0x14];     // 0x4
    void* bpEntryId;        // 0x18
    int selectedMatchIndex; // 0x1c
    void* matchPageIndex;   // 0x20
    char pad_024[0x30];     // 0x24
    union {
        struct {
            void* firstMatchLayout;
            char pad_058[0x1c];
        };
        void* matchLayouts[8]; // 0x54
    };
    union {
        struct {
            void* firstCategoryLayout;
            char pad_078[0x8];
        };
        void* categoryLayouts[3]; // 0x74
    };
    void* matchTableFile;      // 0x80
    void* messageResourceFile; // 0x84
    void* messageDraw;         // 0x88
};

#define CCOMSELECTMATCH_OFF(f, o) typedef char CComSelectMatch_off_##f[((int)&((CComSelectMatch*)0)->f == (o)) ? 1 : -1]
CCOMSELECTMATCH_OFF(bpEntryId, 0x18);
CCOMSELECTMATCH_OFF(selectedMatchIndex, 0x1c);
CCOMSELECTMATCH_OFF(matchPageIndex, 0x20);
CCOMSELECTMATCH_OFF(firstMatchLayout, 0x54);
CCOMSELECTMATCH_OFF(firstCategoryLayout, 0x74);
CCOMSELECTMATCH_OFF(matchTableFile, 0x80);
CCOMSELECTMATCH_OFF(messageResourceFile, 0x84);
CCOMSELECTMATCH_OFF(messageDraw, 0x88);
CCOMSELECTMATCH_OFF(matchLayouts, 0x54);
CCOMSELECTMATCH_OFF(categoryLayouts, 0x74);
#undef CCOMSELECTMATCH_OFF

#endif // CLASSES_CCOMSELECTMATCH_H
