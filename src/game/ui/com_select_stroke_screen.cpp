typedef unsigned long new_size_t;
enum { kFullRoundHoles = 18, kNoRankTexture = 8, kAuxPartHelpInventoryIndex = 6, kItemDialogMode = 2 };

// Selection contracts (all indices are unchecked):
//   GetSelectedRankTexture/GetSelectedCourseTexture/FormatStrokeRowCourseText (slots 19/21/23):
//     visible selection -> lookup(selectionData,index) -> stroke row/save offset.
//   FormatStrokeRowRoundText (slot 24) uses a stroke-table row directly, without lookup.
//   GetCourseMarkerPosition/GetRankMarkerPosition (slots 20/22): index 0..3 selects
//     a named BP group; value receives its position.
//   v25: index selects the description tag; null destination means no operation.
// The four course/rank marker IDs are string pointers in computerStroke*MarkerIds,
// not numeric element IDs. Their literal group names and highlight pairings are unresolved here.
// Display modes: backNine == 0 selects front, nonzero selects back; 18 holes suppresses that prefix.
// Rank 8 means no texture; other values must index rankTextures[0..7]. Dialog mode 2 is passed
// to CPopItemDialog_Ctor; its broader mode semantics are not established by this unit.

// Byte offsets into the existing string pool; retain its storage and encoding.
enum {
    kSelectBackNineTag = 0x3A,
    kSelectionSeparator = 0x72,
    kOneHoleTag = 0x78,
    kThreeHolesTag = 0xA6,
    kSixHolesTag = 0xD4,
    kNineHolesTag = 0x102,
    kEighteenHolesTag = 0x130,
};

#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/singletons.h"
#include "classes/CComSelectStroke.h"
#include <decomp/cw_bitfields.h>

#define AsCComSelectStroke(p) ((CComSelectStroke*)(p))
extern "C" {
void Text_ExpandTaggedText(void*, void*, void*, int);
extern void* computerStrokeDescriptionTags[];
void* __nw__FUl(unsigned long);
void __dl__FPv(void*) __attribute__((nothrow));
void* ItemInventory_GetAuxPartIdAt(void*, int);
void CPopItemDialog_Ctor(void*, void*, int);
extern CSharedDoc* g_sharedDoc;
struct Owner { char pad[0xb4]; void* messageDraw; };
// Expand the indexed selection-description tag into the caller's text.
// value borrows the destination text object; index must select an existing tag.
void CComSelectStroke_vfunc_25(Owner* self, int index, void* value) {
    if (value) Text_ExpandTaggedText(self->messageDraw, value, computerStrokeDescriptionTags[index], 0);
}
}
// Allocate the help dialog through the game's scalar allocator.
inline void* operator new(new_size_t size) { return __nw__FUl(size); }
void operator delete(void*) throw();
struct Obj {
    char data[0x30];
    // Construct the dialog using the selected auxiliary item and the supplied mode.
    Obj(void* p, int n) { CPopItemDialog_Ctor(this, p, n); }
};
// Create the auxiliary-item help dialog for the computer stroke selection screen.
// out is required and receives the newly allocated dialog; ownership passes to the caller.
// Inventory slot 6 is used directly, not derived from the current visible selection.
extern "C" void CComSelectStroke_vfunc_26(void*, Obj** out) {
    *out = new Obj(ItemInventory_GetAuxPartIdAt(&g_sharedDoc->itemInventory, kAuxPartHelpInventoryIndex), kItemDialogMode);
}

extern "C" {
struct Target;
struct VTable { char pad[0x34]; int (*lookup)(Target*, void*, int); };
// ABI-only view: retain virtual slot order and borrowed resource arrays.
struct Target {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10();
    virtual int lookup(void*, int);
    char pad4[0x1c];
    void* selectionData;
    char pad24[0x30];
    void* courseTextures[14];
    void* rankTextures[8];
    void* strokeLevelFile;
};
void* SaveData_AcquireInitializedInstance();
char* SaveData_GetCurrentRecord(void*);
// Resolve the selected save-record rank to its texture; rank 8 has no texture.
// Returned texture is borrowed; self/selection/save data must be valid and rank must be 0..8.
void* CComSelectStroke_GetSelectedRankTexture(Target* self, int index) {
    int selectionOffset = self->lookup(self->selectionData, index);
    signed char rankIndex = (selectionOffset + (signed char*)SaveData_GetCurrentRecord(SaveData_AcquireInitializedInstance()))[0x218];
    if (rankIndex == kNoRankTexture) return 0;
    return self->rankTextures[rankIndex];
}
}

extern "C" {
void* CSharedFilePtr_GetLoadedBuffer(void*);
// Binary stroke-level row: bytes +0x0B/+0x0C/+0x0D are course index, front/back selector,
// and hole count. These are signed bytes; valid data supplies a course index 0..13.
struct Item { char precedingColumns[0xb]; signed char courseIndex; signed char backNine; signed char holeCount; };
Item* Table_GetEntry(void*, int);
extern char __vt__15CComStrokeLevel[];
}
struct LookupGuard80325610 {
    void* volatile ptr;
    void* tag;
    // Build a non-owning stroke-level table view over the shared file's loaded bytes.
    LookupGuard80325610(void* p) : ptr(0) { if (p) ptr=p; tag=__vt__15CComStrokeLevel; }
    // The view owns no bytes; shared-file lifetime is managed by the screen.
    ~LookupGuard80325610() {}
};
// Resolve the selected stroke-level row to its course-map texture.
// Returns a borrowed texture; the shared stroke file must already be loaded.
extern "C" void* CComSelectStroke_GetSelectedCourseTexture(Target* self, int index) {
    LookupGuard80325610 local(CSharedFilePtr_GetLoadedBuffer(self->strokeLevelFile));
    return self->courseTextures[Table_GetEntry(local.ptr, self->lookup(self->selectionData,index))->courseIndex];
}
// Partial wide-string ABI: flag selects inline/heap storage. The length expression below
// reads inline low seven bits or the word at +4. Do not infer a full string layout from ptr.
struct Str { CW_BITS3( unsigned flag : 1, unsigned len : 7, unsigned pad : 24) char* ptr; };
struct Dummy { char c; };
extern void* computerStrokeCourseNameKeys[];
extern "C" {
void* MessageTable_FindTextByName(void*, void*);
int wcslen(void*);
void WString_ReplaceRange(Str*, int, int, void*, void*, Dummy*);
}
// Replace the label with the course name from the selected stroke-level row.
// str is optional and borrowed; text lookup must return a valid NUL-terminated UTF-16 string.
extern "C" void CComSelectStroke_FormatStrokeRowCourseText(Target* self, int index, Str* str) {
    if (str) { Dummy d;
        LookupGuard80325610 local(CSharedFilePtr_GetLoadedBuffer(self->strokeLevelFile));
        char* courseName = (char*)MessageTable_FindTextByName(AsCComSelectStroke(self)->messageDraw, computerStrokeCourseNameKeys[Table_GetEntry(local.ptr, self->lookup(self->selectionData, index))->courseIndex]);
        unsigned int oldLength;
        if (str->flag == 0) oldLength = *(unsigned char*)str & 0x7f; else oldLength = *(int*)((char*)str + 4);

        // Preserve this allocator-tag byte/stack layout; its value is not a text property.
        d.c = ((char*)&local)[-4];
        int courseNameLength = wcslen(courseName);
        WString_ReplaceRange(str, 0, oldLength, courseName, (short*)courseName + courseNameLength, &d);
    }
}

extern char computerStrokeSelectionTags[];
#pragma push
#pragma use_lmw_stmw on
#pragma switch_tables on

// Build a front/back and hole-count label; full rounds omit the front/back prefix.
// str is optional and borrowed; index directly selects a loaded stroke-table row.
// Only 1/3/6/9/18 have count tags. Other counts leave any existing prefix/text as produced.
extern "C" void CComSelectStroke_FormatStrokeRowRoundText(Target* self, int index, Str* str) {
    if (str) {
        LookupGuard80325610 local(CSharedFilePtr_GetLoadedBuffer(self->strokeLevelFile));
        // First expand front/back and append the separator, except for a full round.
        if (Table_GetEntry(local.ptr, index)->holeCount != kFullRoundHoles) {
            if (Table_GetEntry(local.ptr, index)->backNine == 0) Text_ExpandTaggedText(AsCComSelectStroke(self)->messageDraw, str, computerStrokeSelectionTags, 0);
            else Text_ExpandTaggedText(AsCComSelectStroke(self)->messageDraw, str, computerStrokeSelectionTags + kSelectBackNineTag, 0);
            char* separator = computerStrokeSelectionTags + kSelectionSeparator;
            unsigned int prefixLength;
            if (str->flag == 0) prefixLength = *(unsigned char*)str & 0x7f; else prefixLength = *(int*)((char*)str + 4);
            Dummy d;

            d.c = ((char*)&local)[-4];
            int separatorLength = wcslen(separator);
            WString_ReplaceRange(str, prefixLength, 0, separator, (short*)separator + separatorLength, &d);
        }
        // Expand the count tag into the same destination using the text system's tag semantics.
        switch (Table_GetEntry(local.ptr, index)->holeCount) {
        case 1: Text_ExpandTaggedText(AsCComSelectStroke(self)->messageDraw, str, computerStrokeSelectionTags + kOneHoleTag, 0); break;
        case 3: Text_ExpandTaggedText(AsCComSelectStroke(self)->messageDraw, str, computerStrokeSelectionTags + kThreeHolesTag, 0); break;
        case 6: Text_ExpandTaggedText(AsCComSelectStroke(self)->messageDraw, str, computerStrokeSelectionTags + kSixHolesTag, 0); break;
        case 9: Text_ExpandTaggedText(AsCComSelectStroke(self)->messageDraw, str, computerStrokeSelectionTags + kNineHolesTag, 0); break;
        case kFullRoundHoles: Text_ExpandTaggedText(AsCComSelectStroke(self)->messageDraw, str, computerStrokeSelectionTags + kEighteenHolesTag, 0); break;
        case 0: case 2: case 4: case 5: case 7: case 8: case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: return; default: break;
        }
    }
}

extern "C" {
#pragma pop
struct FourWords { int a[4]; };
extern FourWords computerStrokeCourseMarkerIds;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_ComputerStrokeMarkers[]; // Runtime destructor-registration record for the BP controller singleton.
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void __register_global_object(void*, void (*)(void*), void*);
void* IdMap_Lookup(void*, void*);
void* BpObject_GetOrCreateNamedGroup(void*, int);
void* UIGroup_GetPosition(void*, void*);
// Return the UI position for the indexed course selection marker.
// value is borrowed output storage; coordinates and return status are those of UIGroup_GetPosition.
void* CComSelectStroke_GetCourseMarkerPosition(Target* self, int index, void* value) {
    bool missing = !((void* volatile&)Sing_805DC5F0());
    FourWords words = computerStrokeCourseMarkerIds;
    // Lazily publish the BP controller, registering its destructor only on first construction.
    if (missing) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_ComputerStrokeMarkers);
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = s_bpCtrl;
    }
    return UIGroup_GetPosition(BpObject_GetOrCreateNamedGroup(IdMap_Lookup(((void* volatile&)Sing_805DC5F0()), AsCComSelectStroke(self)->bpEntryId), words.a[index]), value);
}
extern FourWords computerStrokeRankMarkerIds;
// Return the UI position for the indexed rank selection marker.
// value is borrowed output storage; coordinates and return status are those of UIGroup_GetPosition.
void* CComSelectStroke_GetRankMarkerPosition(Target* self, int index, void* value) {
    bool missing = !((void* volatile&)Sing_805DC5F0());
    FourWords words = computerStrokeRankMarkerIds;
    // Same shared BP-controller lifetime as the course-marker accessor above.
    if (missing) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_ComputerStrokeMarkers);
            s_bpCtrlInitialized = 1;
        }
        ((void* volatile&)Sing_805DC5F0()) = s_bpCtrl;
    }
    return UIGroup_GetPosition(BpObject_GetOrCreateNamedGroup(IdMap_Lookup(((void* volatile&)Sing_805DC5F0()), AsCComSelectStroke(self)->bpEntryId), words.a[index]), value);
}
}

