// Computer stroke-play selection screen: the course and rank textures shown
// beside the rows of a loaded stroke-level table, the row text built from
// tagged message strings, the UI positions of the course and rank markers, and
// the auxiliary-item help dialog.
#include "classes/CComSelectStroke.h"

#include "classes/CSharedDoc.h"
#include "classes/MWStringStorage.h"
#include "game/singletons.h"
#include "classes/CBpCtrl.h"

enum {
    kFullRoundHoles = 18,
    kNoRankTexture = 8,
    kAuxPartHelpInventoryIndex = 6,
    kItemDialogMode = 2
};

// Offsets into computerStrokeSelectionTags, whose strings are UTF-16 tags in the
// pooled message text.
enum {
    kSelectFrontTag = 0x00,     // "\<D=MSG_COMMON_SELECT_FRONT>"
    kSelectBackNineTag = 0x3A,  // "\<D=MSG_COMMON_SELECT_BACK>"
    kSelectionSeparator = 0x72, // " "
    kOneHoleTag = 0x78,         // "\<D=MSG_COMMON_HOLE_1>"
    kThreeHolesTag = 0xA6,      // "\<D=MSG_COMMON_HOLE_3>"
    kSixHolesTag = 0xD4,        // "\<D=MSG_COMMON_HOLE_6>"
    kNineHolesTag = 0x102,      // "\<D=MSG_COMMON_HOLE_9>"
    kEighteenHolesTag = 0x130   // "\<D=MSG_COMMON_HOLE_18>"
};

// Four UI groups marked by a pooled string name.
struct MarkerGroupNames {
    const char* names[4];
};

// Columns of a stroke-level row. A course index is 0..13 in valid data.
struct StrokeLevelRow {
    char precedingColumns[0xB];
    signed char courseIndex; // 0x0B
    signed char backNine;    // 0x0C
    signed char holeCount;   // 0x0D
};

extern "C" {
void Text_ExpandTaggedText(void*, MWStringStorage*, void*, int);
extern void* computerStrokeDescriptionTags[];
extern char computerStrokeSelectionTags[];
extern void* computerStrokeCourseNameKeys[];

void* __nw__FUl(unsigned long);
void __dl__FPv(void*) __attribute__((nothrow));
void* ItemInventory_GetAuxPartIdAt(void*, int);
void __ct__14CPopItemDialogFPvPv(void*, void*, int);
extern CSharedDoc* g_sharedDoc;

void* SaveData_AcquireInitializedInstance();
char* SaveData_GetCurrentRecord(void*);

void* CSharedFilePtr_GetLoadedBuffer(void*);
StrokeLevelRow* Table_GetEntry(void*, int);
extern char __vt__15CComStrokeLevel[];

void* MessageTable_FindTextByName(void*, void*);
int wcslen(const wchar_t*);
void WString_ReplaceRange(MWStringStorage*, int, int, const wchar_t*, const wchar_t*, void*);

extern MarkerGroupNames computerStrokeCourseMarkerIds;
extern MarkerGroupNames computerStrokeRankMarkerIds;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_ComputerStrokeMarkers[];
void CBpCtrl_dtor(void*);
void __register_global_object(void*, void (*)(void*), void*);
void* IdMap_Lookup(void*, int);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void* UIGroup_GetPosition(void*, void*);
}

// The selection base's row lookup (vtable slot 0x34): the entry index for a page
// and a row. It is the only virtual slot this screen calls.
struct SelectionPage {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual int GetEntryIndex(int page, int row);
};

// A stroke-level table laid over the bytes of the shared file. It owns no
// storage; the screen owns the file for as long as the view exists.
struct StrokeLevelTable {
    void* data;   // 0x0
    void* vtable; // 0x4  the table object's vtable tag
    StrokeLevelTable(void* buffer) : data(0) {
        if (buffer)
            data = buffer;
        vtable = __vt__15CComStrokeLevel;
    }
    ~StrokeLevelTable() {}
};

// The help dialog, allocated and constructed here; the caller owns it.
struct AuxItemDialog {
    char storage[0x30];
    AuxItemDialog(void* item, int mode) {
        __ct__14CPopItemDialogFPvPv(this, item, mode);
    }
};

// Allocate through the game's scalar allocator.
inline void* operator new(unsigned long size) {
    return __nw__FUl(size);
}
void operator delete(void*) throw();

// Resolve a row's course index to its course-map texture.
void* CComSelectStroke::GetSelectedCourseTexture(int row) {
    StrokeLevelTable table(CSharedFilePtr_GetLoadedBuffer(strokeLevelFile));
    StrokeLevelRow* entry = Table_GetEntry(table.data, ((SelectionPage*)this)->GetEntryIndex(pageIndex, row));
    return courseTextures[entry->courseIndex];
}

// Return the UI position of a course marker group.
void* CComSelectStroke::GetCourseMarkerPosition(int marker, void* position) {
    void* controller = Sing_805DC5F0();
    MarkerGroupNames names = computerStrokeCourseMarkerIds;
    // Build the shared BP controller on first use, registering its destructor once.
    if (!controller) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_ComputerStrokeMarkers);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    return UIGroup_GetPosition(
        BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId), names.names[marker]), position);
}

// Resolve a row's saved rank to its texture; rank eight has no texture.
void* CComSelectStroke::GetSelectedRankTexture(int row) {
    int entry = ((SelectionPage*)this)->GetEntryIndex(pageIndex, row);
    signed char rankIndex =
        (entry + (signed char*)SaveData_GetCurrentRecord(SaveData_AcquireInitializedInstance()))[0x218];
    if (rankIndex == kNoRankTexture)
        return 0;
    return rankTextures[rankIndex];
}

// Return the UI position of a rank marker group.
void* CComSelectStroke::GetRankMarkerPosition(int marker, void* position) {
    void* controller = Sing_805DC5F0();
    MarkerGroupNames names = computerStrokeRankMarkerIds;
    // Same shared BP controller as the course markers.
    if (!controller) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_ComputerStrokeMarkers);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    return UIGroup_GetPosition(
        BpObject_GetOrCreateNamedGroup(IdMap_Lookup(Sing_805DC5F0(), bpEntryId), names.names[marker]), position);
}

// Replace the row label with the course name from the selected stroke-level row.
void CComSelectStroke::FormatStrokeRowCourseText(int row, MWStringStorage* text) {
    if (text) {
        StrokeLevelTable table(CSharedFilePtr_GetLoadedBuffer(strokeLevelFile));
        StrokeLevelRow* entry = Table_GetEntry(table.data, ((SelectionPage*)this)->GetEntryIndex(pageIndex, row));
        const wchar_t* courseName =
            (const wchar_t*)MessageTable_FindTextByName(messageDraw, computerStrokeCourseNameKeys[entry->courseIndex]);
        unsigned int oldLength;
        if ((text->flags >> 31) == 0)
            oldLength = *(unsigned char*)text & 0x7f;
        else
            oldLength = text->length;

        // The text system takes a one-byte allocator tag, read here from the byte
        // just below the table view.
        char allocatorTag = ((char*)&table)[-4];
        int courseNameLength = wcslen(courseName);
        WString_ReplaceRange(text, 0, oldLength, courseName, courseName + courseNameLength, &allocatorTag);
    }
}

// Build a front/back and hole-count label; a full round omits the front/back
// prefix. Only 1, 3, 6, 9 and 18 holes have count tags. The switch is a jump
// table and the prologue saves registers with lmw/stmw.
#pragma push
#pragma use_lmw_stmw on
#pragma switch_tables on
void CComSelectStroke::FormatStrokeRowRoundText(int row, MWStringStorage* text) {
    if (text) {
        StrokeLevelTable table(CSharedFilePtr_GetLoadedBuffer(strokeLevelFile));
        // First expand front/back and append the separator, except for a full round.
        if (Table_GetEntry(table.data, row)->holeCount != kFullRoundHoles) {
            if (Table_GetEntry(table.data, row)->backNine == 0)
                Text_ExpandTaggedText(messageDraw, text, computerStrokeSelectionTags + kSelectFrontTag, 0);
            else
                Text_ExpandTaggedText(messageDraw, text, computerStrokeSelectionTags + kSelectBackNineTag, 0);
            const char* separator = computerStrokeSelectionTags + kSelectionSeparator;
            unsigned int prefixLength;
            if ((text->flags >> 31) == 0)
                prefixLength = *(unsigned char*)text & 0x7f;
            else
                prefixLength = text->length;
            char allocatorTag = ((char*)&table)[-4];
            int separatorLength = wcslen((const wchar_t*)separator);
            WString_ReplaceRange(text, prefixLength, 0, (const wchar_t*)separator,
                                 (const wchar_t*)separator + separatorLength, &allocatorTag);
        }
        // Expand the count tag into the same destination using the text system's tag semantics.
        switch (Table_GetEntry(table.data, row)->holeCount) {
        case 1:
            Text_ExpandTaggedText(messageDraw, text, computerStrokeSelectionTags + kOneHoleTag, 0);
            break;
        case 3:
            Text_ExpandTaggedText(messageDraw, text, computerStrokeSelectionTags + kThreeHolesTag, 0);
            break;
        case 6:
            Text_ExpandTaggedText(messageDraw, text, computerStrokeSelectionTags + kSixHolesTag, 0);
            break;
        case 9:
            Text_ExpandTaggedText(messageDraw, text, computerStrokeSelectionTags + kNineHolesTag, 0);
            break;
        case kFullRoundHoles:
            Text_ExpandTaggedText(messageDraw, text, computerStrokeSelectionTags + kEighteenHolesTag, 0);
            break;
        case 0:
        case 2:
        case 4:
        case 5:
        case 7:
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
            return;
        default:
            break;
        }
    }
}
#pragma pop

// Expand the indexed description tag into the caller's text.
void CComSelectStroke::ExpandDescriptionText(int description, MWStringStorage* text) {
    if (text)
        Text_ExpandTaggedText(messageDraw, text, computerStrokeDescriptionTags[description], 0);
}

// Create the help dialog for the selected auxiliary item. Inventory slot six is
// used directly; ownership passes to the caller.
void CComSelectStroke::CreateAuxItemDialog(void** dialog) {
    *dialog = new AuxItemDialog(ItemInventory_GetAuxPartIdAt(g_sharedDoc->itemInventory, kAuxPartHelpInventoryIndex),
                                kItemDialogMode);
}
