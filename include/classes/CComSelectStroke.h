// CComSelectStroke: the computer stroke-play selection screen.
//
// It owns the course and rank textures shown beside the rows of the loaded
// stroke-level table, the shared stroke-level and message files, and the text
// renderer used for the row labels.
//
// The class has a vtable, taken from the original data, so `vtbl` is spelled out
// as a member and nothing is declared virtual here.
#ifndef CLASSES_CCOMSELECTSTROKE_H
#define CLASSES_CCOMSELECTSTROKE_H

#include "classes/MWStringStorage.h"
#include "game/types.h"

struct RoundSettings;
class CComSelectStroke {
public:
    // Row textures.
    void* GetSelectedCourseTexture(int row);
    void* GetSelectedRankTexture(int row);

    // UI positions of the course and rank markers.
    void* GetCourseMarkerPosition(int marker, void* position);
    void* GetRankMarkerPosition(int marker, void* position);

    // Row text.
    void FormatStrokeRowCourseText(int row, MWStringStorage* text);
    void FormatStrokeRowRoundText(int row, MWStringStorage* text);

    // Vtable slots, named for what they do.
    void ExpandDescriptionText(int description, MWStringStorage* text);
    void CreateAuxItemDialog(void** dialog);

    void* vtbl;                // 0x00: original vtable pointer
    char pad_004[0x14];        // 0x04
    int bpEntryId;             // 0x18
    char pad_01C[0x4];         // 0x1c
    int pageIndex;             // 0x20: page of selection rows
    char pad_024[0x30];        // 0x24
    void* courseTextures[14];  // 0x54: course map textures, one per course
    void* rankTextures[8];     // 0x8c: rank textures; rank eight has none
    void* strokeLevelFile;     // 0xac
    void* messageResourceFile; // 0xb0
    void* messageDraw;         // 0xb4

    int AreResourcesLoaded();
    void InitMessageResourceAndMenu();
    int FindUnlockRow();
    int BuildRoundSettings(RoundSettings* out);

    CComSelectStroke();
    ~CComSelectStroke();
};

#define CCOMSELECTSTROKE_OFF(f, o) \
    typedef char CComSelectStroke_off_##f[((int)&((CComSelectStroke*)0)->f == (o)) ? 1 : -1]
CCOMSELECTSTROKE_OFF(bpEntryId, 0x18);
CCOMSELECTSTROKE_OFF(pageIndex, 0x20);
CCOMSELECTSTROKE_OFF(courseTextures, 0x54);
CCOMSELECTSTROKE_OFF(rankTextures, 0x8c);
CCOMSELECTSTROKE_OFF(strokeLevelFile, 0xac);
CCOMSELECTSTROKE_OFF(messageResourceFile, 0xb0);
CCOMSELECTSTROKE_OFF(messageDraw, 0xb4);
#undef CCOMSELECTSTROKE_OFF
typedef char CComSelectStroke_size[(sizeof(CComSelectStroke) == 0xB8) ? 1 : -1];

#endif // CLASSES_CCOMSELECTSTROKE_H
