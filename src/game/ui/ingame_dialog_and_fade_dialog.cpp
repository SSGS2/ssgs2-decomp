#include <decomp/static_init.h>
#include "MSL/list.h"
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::list<void*, app::app_allocator<void*> > MSLPointerList;
#include "classes/CInGameFadeDialog.h"
#include "classes/CMsgDraw.h"
#include "classes/CInGameDialog.h"
// Translation unit 0x802562A0-0x802579FC (24 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "game/string.h"
// Construct a copied vector element in caller-provided storage.
inline void* operator new(size_t, void* p) { return p; }

extern "C" {

extern float kFloat_6_2831855;
extern float s_inGameDialogHelperFullTurnAngle;
extern float s_fadeDialogHelperFullTurnAngle;
extern const float kDialogOpacityByteScale;

// Imports from other units. Parameter types are kept as void* where the callee's real type is not declared in a
// header; the groups are: memory (appFree/new/delete), strings (String_*, WString_*), dialog and message drawing
// (CInGameDialog_*, CBpCtrl_*, CMsgDraw_*, Text_*), then dialog-base destructor and data symbols.
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, char*);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void String_ReserveCapacity(void*, unsigned int);
extern void __dla__FPv(void*);
extern void WString_ReplaceRange(void*, int, int, const void*, const void*, char*);
extern void PuppetChildList_EraseMatchingValue(void*, void*, void*, int);
extern void CInGameDialog_Ctor(void*);
extern void* __nw__FUl(unsigned int);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void*);
extern void CBpCtrl_CreateEntry(void*, void*, void*, int);
extern void BpCtrl_SetOpacity(int, int);
extern void CBpCtrl_DeleteEntryById(void*, int);
extern void CMsgDraw_ResetTextState(void*);
extern void CMsgDraw_ClearFormattingAndRevealAll(void*);
extern void Text_SetContent(void*, const char*);
extern void CMsgDraw_GetTextExtent(void*, float*);
extern void CMsgDraw_SetColor(void*, unsigned int);
extern void gtl_Vec2_Copy(float*, const char*);
extern void Text_SetPosition(void*, float*);
extern void CMsgDraw_UpdateReveal(void*);
extern void Text_DrawGlyphs(void*);
extern void __dt__15SmallDialogBaseFv(void*, int);
extern int __cntlzw(unsigned int);
extern char __vt__17CInGameFadeDialog[];
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_InGameFadeDialog[];
extern signed char s_bpCtrlInitialized;
extern float kFloat_0_5_TextureDraw;

// All registry lists share links at +0/+4 and a payload at +8. Their embedded
// sentinels contain links only; never read object on the sentinel.
struct DialogRegistryNode {
    DialogRegistryNode* next;
    DialogRegistryNode* prev;
    void* object; // Borrowed on traversal; registry retains one reference per entry.
};
typedef DialogRegistryNode fn_802567C4_Node;
struct fn_802567C4_Base { int refs; };
// ABI virtual slots: v0 deletes; v9 advances by seconds; v15 tests completion.
// Unused slots retain their placeholders so the Wii vtable layout stays explicit.
struct fn_802567C4_Object : fn_802567C4_Base {
    virtual void v0(int);
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9(float);
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual int v15();
};
struct fn_802567C4_Self { char pad[4]; unsigned int count; fn_802567C4_Node* sentinel; fn_802567C4_Node* first; };
// Update dialogs, release completed objects, and erase their empty entries.
void CommonDialogRegistry_UpdateAndReleaseCompleted(fn_802567C4_Self* self, float dt);

struct fn_80256D50_Base { int pad; };
// ABI slots used here: v5 starts/registers dialog behavior; v6 stops it before teardown.
struct fn_80256D50_Object : fn_80256D50_Base {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    char fields[0x44]; int handle; int f50; int f54; int f58; int f5c; int f60; int f64; int f68; int f6c; int f70; int f74;
};
} // extern C
struct SmallDialogBase {
    char storage[0x50];
    // Construct the base dialog and leave its BP entry unregistered (-1).
    SmallDialogBase() {
        CInGameDialog_Ctor(this);
        ((CInGameDialog*)this)->callbackVtable = __vt__17CInGameFadeDialog;
        *(int*)&((CInGameFadeDialog*)this)->bpEntryId = -1; // original +0x4C
    }
    ~SmallDialogBase();
};
struct FadeDialogList {
    int a, b, c;
    // Initialize an empty visibility-override vector (data, count, capacity).
    FadeDialogList() : a(0), b(0), c(0) {}
    ~FadeDialogList();
};
struct FadeDialogObject : SmallDialogBase {
    NString first;
    FadeDialogList list;
    NString second;
    int tail;
    // Register this dialog's BP entry after base and string members exist.
    FadeDialogObject(void* self) {
        fn_80256D50_Object* obj = (fn_80256D50_Object*)this;
        tail = 0;
        obj->v5();
        if (((unsigned int&)Sing_805DC5F0()) == 0) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_InGameFadeDialog);
                s_bpCtrlInitialized = 1;
            }
            ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
        }
        CBpCtrl_CreateEntry((void*)((unsigned int&)Sing_805DC5F0()), self, &obj->handle, 1);
    }
};
extern "C" {
// Allocate a fade dialog and return it through the required output slot.
int CInGameFadeDialog_Create(void* self, void** out);

struct fn_802562A0_Arg {
    char pad[0x40];
    union { unsigned int flags; unsigned char smallSize; };
    unsigned int largeSize;
    void* largeData;
};

#pragma opt_common_subs reset
// Replace or clear UTF-16 text using the inline or heap string storage.
void InGameDialog_SetWideText(fn_802562A0_Arg* self, const wchar_t* str);
#pragma opt_common_subs reset

// Initialize this dialog helper's full-turn angle cache.
void InitInGameDialogHelperFullTurnAngle();

// Initialize the fade-dialog helper's full-turn angle cache.
void InitFadeDialogHelperFullTurnAngle();

struct fn_80257858_Elem { unsigned int flags; char pad[4]; void* object; char pad2[4]; };
struct fn_80257858_Self {
    char pad0[4]; void* vtable; char pad0b[0x44]; int handle;
    unsigned int flagsA; char padA[4]; void* ptrA;
    fn_80257858_Elem* array; unsigned int arrayCount;
    char padB[4];
    unsigned int flagsB; char padC[4]; void* ptrB;
};
// Stop the script, destroy visibility overrides and strings, and release the base dialog.
void* CInGameFadeDialog_dtor(fn_80257858_Self* self, int shouldDelete);

// DrawAligned borrows a two-float HUD anchor for this call only.
// Text payloads are UTF-16 even though the imported setter accepts const char*.
struct DialogHudAnchor { float x, y; };
enum DialogAxisAlignment { kDialogAlignStart = 0, kDialogAlignCenter = 1, kDialogAlignEnd = 2 };
enum DialogRegistryStatus { kDialogRegistryIdle = 0, kDialogRegistryNodesCleared = 1 };
struct DialogTextOwner { char prefix[0xC]; void* message; };
struct fn_8025643C_Self { char pad[0x3c]; void* list; union { unsigned int flags; unsigned char small; }; unsigned int size; void* data; char pad3[0x48]; int field94; };
#pragma opt_common_subs off
// Measure and align a message, update reveal state, and draw its glyphs.
void DialogText_DrawAligned(fn_8025643C_Self* self, const DialogHudAnchor* hudAnchor, unsigned int color, int alignX, int alignY);
#pragma opt_common_subs reset

// Same states as CInGameDialog's transition loop in in_game_dialog_and_snapshot_copy.cpp: opening/closing
// integrate elapsed seconds, open may auto-close, inactive is ready for release.
enum DialogTransitionState { kDialogReset = 1, kDialogInactive = 2,
                             kDialogOpening = 3, kDialogOpen = 4, kDialogClosing = 5 };
struct fn_80256610_Arg {
    char pad[0x18];
    unsigned int value;
};

typedef bool DialogBoolResult;
// Test whether the dialog is inactive and may be released by the registry.
DialogBoolResult CInGameDialog_vfunc_15(fn_80256610_Arg* arg);

struct fn_80256624_Arg {
    char pad[0x18];
    unsigned int value;
};

// Test whether the dialog is opening, open, or closing (including both fades).
DialogBoolResult CInGameDialog_vfunc_3(fn_80256624_Arg* arg);

// Release heap string storage and optionally delete the string object.
void* NString_dtor(void* self, int shouldDelete);

typedef DialogRegistryNode fn_802568D0_Node;

struct fn_802568D0_ObjectVtable {
    char pad[0x18];
    void (*destroy)(void*);
};

// +0x34/+0x38 are independent close blockers; their underlying reasons remain unproved.
struct fn_802568D0_Object {
    char pad0[4];
    fn_802568D0_ObjectVtable* vtable;
    char pad[0x2c];
    unsigned int closeBlock34;
    unsigned int blockRegistryClose;
};

struct fn_802568D0_List {
    char pad[8];
    fn_802568D0_Node* sentinel;
    fn_802568D0_Node* first;
};

struct Q4dB { char pad[4]; };
class Q4d : public Q4dB { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18();
};
// Request close only when both close-blocking fields are zero.
void CommonDialogRegistry_RequestEligibleClose(fn_802568D0_List* list);

struct fn_8025770C_Arg {
    char pad[0x68];
    union { unsigned int flags; unsigned char smallSize; };
    unsigned int largeSize;
};

// Replace the fade dialog's text-group name.
void InGameFadeDialog_SetTextGroupName(fn_8025770C_Arg* self, const char* str);

// The VisOverride_* helpers below are a hand-expanded std::vector<{name, item}> push_back (growth: x1.6 below
// 0x05555555 elements, x1.5 below 0x0AAAAAAA, else clamp to 0x0FFFFFFF elements); VisOverride_Allocator is an empty
// stand-in for the original allocator object. They are kept as written because the compiler output depends on them.
struct VisOverride_Allocator { char value; };
struct VisOverride_Text {
    union { unsigned int flags; unsigned char small; };
    unsigned int size;
    void* data;
    // Release only heap-backed name storage; inline bytes belong to the element.
    ~VisOverride_Text() {
        if (flags >> 31) appFreeUni__FPv(data);
    }
};
// Visibility override: owned narrow group name (12 bytes) + borrowed item value
// (4 bytes). Copying duplicates heap text; destroying an element never deletes item.
// Address-suffixed types describe this specialization's compiler/unwind ABI only.
struct VisOverride_Elem {
    VisOverride_Text text;
    void* item;
    // Initialize an empty group name; the caller assigns item before insertion.
    VisOverride_Elem(unsigned int zero) { text.flags = zero; text.size = zero; text.data = (void*)zero; }
    // Duplicate the owned name and copy the borrowed item value.
    VisOverride_Elem(const VisOverride_Elem& other) {
        unsigned int flags = other.text.flags;
        try {
            if ((flags >> 31) == 0) {
                unsigned int size = other.text.size;
                text.flags = flags;
                text.size = size;
                text.data = other.text.data;
            } else {
                text.flags = 0; text.size = 0; text.data = 0;
                String_ReserveCapacity(&text, other.text.size);
                VisOverride_Allocator allocator;
                VisOverride_Allocator marker = allocator;
                const char* begin = (const char*)other.text.data;
                String_ReplaceRange(&text, 0, 0, begin, begin + other.text.size, (char*)&marker);
            }
            item = other.item;
        } catch (...) { throw; }
    }
    // Let the text member release its owned storage.
    ~VisOverride_Elem() {}
};
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char kFadeDialogOverrideLengthErrorMessage[];
extern void __dt__Q23std12length_errorFv();
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern "C++" {
#include "game/exception.h"
namespace std { struct logic_error : exception {
    // Install the current exception interface in its leading vtable word.
    void SetTable(void* table) { *(void**)this = table; }
    char* message;
    void* shared;
    logic_error(const char* source);
    ~logic_error();
}; }
}
struct VisOverride_MessageGuard {
    void* data;
    VisOverride_Allocator* allocator;
    // Release message bytes if their shared control block was not constructed.
    ~VisOverride_MessageGuard() { if (data) __dla__FPv(data); }
};
struct VisOverride_Shared {
    void* vtable;
    int strong;
    int weak;
    void* data;
    // Attach message bytes to an initially unique shared control block.
    VisOverride_Shared(void* p) {
        strong = 1; weak = 1; vtable = s_sharedCharDeleterVtable; data = p;
    }
};
// Copy exception text, guarding the allocation until shared ownership is installed.
inline std::logic_error::logic_error(const char* source) {
        SetTable(__vt__Q23std11logic_error);
        VisOverride_Allocator messageAllocator;
        messageAllocator.value = 0;
        unsigned int n = strlen(source) + 1;
        char* allocated = (char*)__nwa__FUl((n * 2) - n);
        message = allocated;
        {
            VisOverride_MessageGuard guard;
            guard.data = allocated;
            guard.allocator = &messageAllocator;
            shared = new VisOverride_Shared(allocated);
            guard.data = 0;
        }
        strcpy(message, source);
}
}
namespace std {
struct length_error : logic_error {
    // Finish the derived exception interface after constructing the base message.
    length_error(const char* source) : logic_error(source) { SetTable(__vt__Q23std12length_error); }
    ~length_error();
};
}
extern "C" {
// Throw the visibility-vector length error using the original allocator lifetime.
inline void VisOverride_LengthError() {
    VisOverride_Allocator allocator = VisOverride_Allocator();
    const char* message = kFadeDialogOverrideLengthErrorMessage;
    throw std::length_error(message);
}
struct VisOverride_Buffer {
    VisOverride_Elem* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* originalCapacity;
    unsigned int offset;
    // Track a contiguous constructed range beginning at offset during growth.
    VisOverride_Buffer(unsigned int* p) : data(0), size(0), capacity(0), originalCapacity(p), offset(0) {}
    // Destroy remaining live elements, then free the temporary allocation.
    ~VisOverride_Buffer() {
        if (data != 0) {
            unsigned int n = size;
            VisOverride_Elem* end = data + n;
            VisOverride_Allocator allocator = VisOverride_Allocator();
            size -= n;
            while (n != 0) { --end; end->~VisOverride_Elem(); --n; }
            appFreeUni__FPv(data);
        }
    }
};
// Dialog view reaching only the visibility-override vector (data/size/capacity of 16-byte elements) at +0x5C.
struct VisOverrideDialogView { char pad[0x5c]; VisOverride_Elem* data; unsigned int size; unsigned int capacity; };
// Publish the new vector, destroy old names, and leave old storage with fresh.
inline void VisOverride_Exchange(VisOverrideDialogView* self, VisOverride_Buffer& fresh) {
    unsigned int oldOffset = fresh.offset;
    VisOverride_Elem* oldData = self->data;
    VisOverride_Elem* newData = fresh.data;
    VisOverride_Elem* begin = oldData + oldOffset;
    unsigned int oldSize = self->size;
    unsigned int newSize = fresh.size;
    unsigned int oldCapacity = self->capacity;
    unsigned int newCapacity = fresh.capacity;
    VisOverride_Elem* end = begin + oldSize;
    fresh.data = oldData;
    VisOverride_Allocator destroyAllocator = VisOverride_Allocator();
    self->capacity = newCapacity;
    fresh.capacity = oldCapacity;
    self->data = newData;
    self->size = newSize;
    fresh.size = oldSize;
    while (end > begin) { --end; end->~VisOverride_Elem(); }
    fresh.size = 0;
}
// Choose the larger growth increment without changing reference-valued ABI.
inline const unsigned int& VisOverride_Max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
#pragma use_lmw_stmw on
// Append a named group override, growing the owned entry vector when needed.
// text must be nonnull/terminated and is copied; item is stored without retaining.
void InGameFadeDialog_AppendGroupVisibilityOverride(VisOverrideDialogView* self, const char* text, void* item);
#pragma use_lmw_stmw reset

struct fn_80256414_Arg {
    char pad[0x24];
    float value;
};

// Convert the fade opacity scalar to its byte-scale integer value.
int Dialog_GetOpacityByte(const fn_80256414_Arg* arg);

typedef DialogRegistryNode fn_80256654_Node;

struct fn_80256654_List {
    unsigned int pad;
    unsigned int count;
    fn_80256654_Node* sentinel;
    fn_80256654_Node* first;
};

// Unlink and free all list nodes; referenced objects are handled separately.
int CommonDialogRegistry_ClearNodes(fn_80256654_List* self);

typedef DialogRegistryNode fn_80256C70_Node;

struct fn_80256C70_Self {
    unsigned int pad;
    unsigned int count;
    fn_80256C70_Node* sentinel;
    fn_80256C70_Node* first;
};

struct fn_80256C70_Vtable {
    void* f0;
    void* f1;
    void (*destroy)(void*, int);
};

struct fn_80256C70_ObjPre { unsigned int refcount; };
struct fn_80256C70_Obj : fn_80256C70_ObjPre {
    virtual void destroy(int);
};

// Release referenced objects, then destroy their list nodes.
void RefObjectList_ClearAndRelease(fn_80256C70_Self* self);

// Release referenced objects and list nodes, then clear the registry status word.
void RefObjectList_ReleaseEntries(fn_80256C70_Self* self);

extern void* appMallocDummy__FUl(unsigned int);

typedef DialogRegistryNode fn_80256948_Node;

struct fn_80256948_Self {
    unsigned int pad;
    unsigned int count;
    fn_80256948_Node* sentinel;
    fn_80256948_Node* first;
};

struct fn_80256948_Guard {
    fn_80256948_Self* self;
    fn_80256948_Node* node;
    // Release an unlinked node if registry insertion fails before transfer.
    ~fn_80256948_Guard() { if (node) appFreeUni__FPv(node); }
};
struct fn_80256948_Pointer {
    fn_80256C70_Obj* object;
    // Construct the node's dialog pointer after the caller has retained it.
    fn_80256948_Pointer(fn_80256C70_Obj* obj) {
        try { object = obj; } catch (...) { throw; }
    }
};

#pragma opt_common_subs off
// Retain and insert a dialog only when it is not already registered.
int CommonDialogRegistry_AddUnique(fn_80256948_Self* self, fn_80256C70_Obj* object);
#pragma opt_common_subs reset

struct fn_80256B18_List {
    unsigned int pad;
    unsigned int count;
    fn_80256654_Node* first;
    fn_80256654_Node* sentinel;
};

#pragma opt_common_subs off
// Compare iterator slots while preserving the volatile node reload convention.
inline bool fn_80256B18_Same(fn_80256654_Node* const& a, volatile fn_80256654_Node* const& b) {
    return a == b;
}
// Unlink/free nodes up to the embedded sentinel; payload pointers are not released.
inline void fn_80256B18_Clear(fn_80256B18_List* self, fn_80256654_Node* sentinel, volatile fn_80256654_Node* node) {
    if (!fn_80256B18_Same(sentinel, node)) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        while (!fn_80256B18_Same(sentinel, node)) {
            fn_80256654_Node* old = (fn_80256654_Node*)node;
            node = node->prev;
            appFreeUni__FPv(old);
            --self->count;
        }
    }
}
// Initialize an empty reference-object list and its circular sentinel.
fn_80256B18_List* RefObjectList_Init(fn_80256B18_List* self);
#pragma opt_common_subs reset

struct fn_80256BB4_List {
    unsigned int count;
    fn_80256654_Node* sentinel;
    fn_80256654_Node* first;
};

// Destroy list nodes and optionally delete their container.
void* dtor_80256BB4(MSLPointerList* self, int shouldDelete);

struct fn_802563C8_Arg {
    char pad[0x40];
    union { unsigned int flag; unsigned char byte; };
    unsigned int count;
    unsigned int ptr;
};

// Return borrowed UTF-16 dialog text, or null for zero length (not an empty string).
// arg is the dialog base, not the string member; storage starts at arg+0x40.
// The result is invalidated by text replacement or dialog destruction. Existing
// callers use this pointer as a presence test; its legacy name hides the dialog base.
void* InGameDialog_GetNonemptyWideText(fn_802563C8_Arg* arg);

struct dtor_80257794_Elem {
    unsigned int flags;
    char pad[4];
    void* object;
    char pad2[4];
};

extern "C++" {
namespace std {
// The fade dialog's override strings: heap text is released when the vector drops an element.
template <> struct __vec_destroys_elements<dtor_80257794_Elem> {
    enum { value = 1 };
    // Release one override string only when its heap-ownership bit is set.
    static void destroy(dtor_80257794_Elem* p) {
        if (p != 0) {
            if (p != 0) {
                if ((p->flags >> 31) != 0) {
                    appFreeUni__FPv(p->object);
                }
            }
        }
    }
};
}
typedef std::vector<dtor_80257794_Elem, app::app_allocator<dtor_80257794_Elem> > FadeDialogStringVector;
}
// Destroy override strings in reverse order and free their vector storage.
void* __dt__14FadeDialogListFv(FadeDialogStringVector* self, int shouldDelete);

typedef DialogRegistryNode fn_80256A50_Node;

struct fn_80256A50_Base { unsigned int refcount; };
struct fn_80256A50_Obj : fn_80256A50_Base {
    virtual void v0(int);
    virtual void v1();
    virtual void v2(int);
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual void v8();
    virtual void v9();
    virtual void v10();
    virtual void v11();
    virtual void v12();
    virtual void v13();
    virtual void v14();
    virtual void v15();
};

struct fn_80256A50_Self {
    char pad[4];
    unsigned int count;
    char pad2[4];
    fn_80256A50_Node* first;
};

// Release and erase the matching dialog; return whether it was found.
int CommonDialogRegistry_Remove(fn_80256A50_Self* self, void* target);

struct fn_80256E84_Self {
    char pad[0x50];
    union { unsigned int flags; unsigned char smallSize; };
    unsigned int largeSize;
    char pad2[4];
    void* array;
    unsigned int count;
};

struct fn_80256E84_Elem {
    unsigned int flags;
    char pad[4];
    void* object;
    char pad2[4];
};

// Replace the script name and destroy the saved group-visibility arguments.
void GolfTipsDialog_SetScriptNameAndClearArguments(fn_80256E84_Self* self, const char* str);

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622F0", _ctors_u802562A0_0, InitInGameDialogHelperFullTurnAngle);
STATIC_INIT_ENTRY(".CRT$XCU804622F0", _ctors_u802562A0_1, InitFadeDialogHelperFullTurnAngle);
#pragma pop

// Game entry points in original address order.
extern "C" {

// Replace or clear the dialog's owned UTF-16 text.
void InGameDialog_SetWideText(fn_802562A0_Arg* self, const wchar_t* str) {
    struct Allocator { char value; };
    struct {
        Allocator clearSource; char pad0[3];
        Allocator clearCopy; char pad1[3];
        Allocator assignCopy; char pad2[3];
        Allocator assignSource;
    } allocators;
    if (str != 0) {
        unsigned int replacedTextLength;
        if ((self->flags >> 31) == 0)
            replacedTextLength = self->smallSize & 0x7f;
        else
            replacedTextLength = self->largeSize;
        allocators.assignCopy = allocators.assignSource;
        unsigned int length = wcslen(str);
        WString_ReplaceRange(&self->flags, 0, replacedTextLength, str, str + length, &allocators.assignCopy.value);
    } else {
        const wchar_t* end;
        unsigned int flag = self->flags >> 31;
        const wchar_t* finish;
        int length;
        const wchar_t* begin;
        if (flag == 0) {
            length = self->smallSize & 0x7f;
            begin = ((const wchar_t*)&self->flags + 1);
        } else {
            begin = (const wchar_t*)self->largeData;
            length = self->largeSize;
        }
        unsigned int cntl = __cntlzw(flag);
        end = begin + length;
        flag = cntl >> 5;
        begin = flag ? ((const wchar_t*)&self->flags + 1) : (const wchar_t*)self->largeData;
        length = end - begin;
        finish = flag ? ((const wchar_t*)&self->flags + 1) : (const wchar_t*)self->largeData;
        int offset = begin - finish;
        allocators.clearCopy = allocators.clearSource;
        WString_ReplaceRange(&self->flags, offset, length, 0, 0, &allocators.clearCopy.value);
    }
}

// Borrow nonempty UTF-16 text until replacement or destruction.
void* InGameDialog_GetNonemptyWideText(fn_802563C8_Arg* arg) {
    unsigned int smallFlag = (unsigned int)__cntlzw(arg->flag >> 31) >> 5;
    unsigned int count;
    if (smallFlag)
        count = arg->byte & 0x7f;
    else
        count = arg->count;
    if (count == 0)
        return 0;
    if (smallFlag)
        return (wchar_t*)&arg->flag + 1;
    return (void*)arg->ptr;
}

// Convert the stored fade scalar to a byte opacity.
int Dialog_GetOpacityByte(const fn_80256414_Arg* arg) {
    return (unsigned char)(kDialogOpacityByteScale * arg->value);
}

#pragma push
#pragma opt_common_subs off
// Draw a tagged UTF-16 payload at an aligned HUD anchor.
void DialogText_DrawAligned(fn_8025643C_Self* self, const DialogHudAnchor* hudAnchor, unsigned int color, int alignX, int alignY) {
    void* wideTextPayload;
    void* messageRenderer;
    float positionAndExtent[4];
    unsigned int inlineStorageLeadingZeros;
    unsigned int usesInlineText;
    unsigned int textLength;
    // Phase 1: decode the tagged UTF-16 payload; empty text suppresses drawing.
    inlineStorageLeadingZeros = __cntlzw(self->flags >> 31);
    usesInlineText = inlineStorageLeadingZeros >> 5;
    textLength = usesInlineText ? (self->small & 0x7f) : self->size;
    if (textLength == 0) wideTextPayload = 0;
    else if (usesInlineText) wideTextPayload = (void*)((unsigned short*)&self->flags + 1);
    else wideTextPayload = self->data;
    if (wideTextPayload != 0 && self->list != 0 && ((DialogTextOwner*)self->list)->message != 0) {
        messageRenderer = ((DialogTextOwner*)self->list)->message;
        if (messageRenderer != 0) {
            CMsgDraw_ResetTextState(messageRenderer); CMsgDraw_ClearFormattingAndRevealAll(messageRenderer);
            if (wideTextPayload != 0) Text_SetContent(messageRenderer, (const char*)wideTextPayload);
        }
        // Phase 2: measure before aligning the caller's two-float HUD anchor.
        CMsgDraw_GetTextExtent(messageRenderer, positionAndExtent + 2);
        CMsgDraw_SetColor(messageRenderer, color | 0xff);
        (*(unsigned int*)&((CMsgDraw*)messageRenderer)->colorMultiplier) = color | 0xffffff00u;
        gtl_Vec2_Copy(positionAndExtent, (const char*)hudAnchor);
        switch (alignX) {
            case kDialogAlignStart: break;
            case kDialogAlignCenter: positionAndExtent[0] = positionAndExtent[0] - positionAndExtent[2] * kFloat_0_5_TextureDraw; break;
            case kDialogAlignEnd: positionAndExtent[0] = positionAndExtent[0] - positionAndExtent[2]; break;
        }
        switch (alignY) {
            case kDialogAlignStart: break;
            case kDialogAlignCenter: positionAndExtent[1] = positionAndExtent[1] - positionAndExtent[3] * kFloat_0_5_TextureDraw; break;
            case kDialogAlignEnd: positionAndExtent[1] = positionAndExtent[1] - positionAndExtent[3]; break;
        }
        // Phase 3: apply position and native reveal update, then restore full-text draw state.
        Text_SetPosition(messageRenderer, positionAndExtent); CMsgDraw_UpdateReveal(messageRenderer);
        if (messageRenderer != 0) {
            CMsgDraw_ResetTextState(messageRenderer); CMsgDraw_ClearFormattingAndRevealAll(messageRenderer);
            if (wideTextPayload != 0) Text_SetContent(messageRenderer, (const char*)wideTextPayload);
        }
        ((CMsgDraw*)messageRenderer)->drawSuppressed = 0; Text_DrawGlyphs(messageRenderer);
        (void)usesInlineText;
    }
}
#pragma pop

// Test whether the dialog reached its releasable inactive state.
DialogBoolResult CInGameDialog_vfunc_15(fn_80256610_Arg* arg) {
    return arg->value == kDialogInactive;
}

// Test whether the dialog is opening, open or closing.
DialogBoolResult CInGameDialog_vfunc_3(fn_80256624_Arg* arg) {
    return arg->value - kDialogOpening <= kDialogClosing - kDialogOpening;
}

// Initialize the dialog helper's cached full-turn angle.
void InitInGameDialogHelperFullTurnAngle() {
    s_inGameDialogHelperFullTurnAngle = kFloat_6_2831855;
}

// Free registry nodes and mark the node-cleared status.
int CommonDialogRegistry_ClearNodes(fn_80256654_List* self) {
    fn_80256654_Node* sentinel = (fn_80256654_Node*)((char*)self + 8);
    fn_80256654_Node* node = self->first;
    if (node != sentinel) {
        fn_80256654_Node* first = sentinel->next;
        node->next->prev = first->prev;
        first->prev->next = node->next;
        while (node != sentinel) {
            fn_80256654_Node* old = node;
            node = node->prev;
            appFreeUni__FPv(old);
            --self->count;
        }
    }
    self->pad = kDialogRegistryNodesCleared;
    return 1;
}

// Release retained payloads, free nodes and reset registry status.
void RefObjectList_ReleaseEntries(fn_80256C70_Self* self) {
    {
        fn_80256C70_Node* sentinel = (fn_80256C70_Node*)((char*)self + 8);
        fn_80256C70_Node* node = self->first;
        while (node != sentinel) {
            fn_80256C70_Obj* obj = (fn_80256C70_Obj*)node->object;
            if (obj != 0) {
                unsigned int rc = obj->refcount - 1;
                obj->refcount = rc;
                if (rc == 0) {
                    if (obj != 0) {
                        obj->destroy(1);
                    }
                }
            }
            node = node->prev;
        }
    }
    {
        fn_80256C70_Node* sentinel = (fn_80256C70_Node*)(void*)&self->sentinel;
        register fn_80256C70_Node* node = self->first;
        if (node != sentinel) {
            fn_80256C70_Node* tail = sentinel->next;
            node->next->prev = tail->prev;
            tail->prev->next = node->next;
            while (node != sentinel) {
                fn_80256C70_Node* old = node;
                node = node->prev;
                appFreeUni__FPv(old);
                --self->count;
            }
        }
    }
    self->pad = kDialogRegistryIdle;
}

// Advance dialogs and erase completed registry entries.
void CommonDialogRegistry_UpdateAndReleaseCompleted(fn_802567C4_Self* self, float dt) {
    // Phase 1: update each retained dialog, then release completed payloads.
    if (self->count != 0) {
        fn_802567C4_Node* sentinel = (fn_802567C4_Node*)&self->sentinel;
        fn_802567C4_Node* node = self->first;
        while (node != sentinel) {
            if (node->object != 0) {
                ((fn_802567C4_Object*)node->object)->v9(dt);
                if (((fn_802567C4_Object*)node->object)->v15()) {
                    fn_802567C4_Object* obj = (fn_802567C4_Object*)node->object;
                    int refs = --obj->refs;
                    if (refs == 0 && obj != 0)
                        obj->v0(1);
                    node->object = 0;
                }
            }
            node = node->prev;
        }
        // Phase 2: erase nodes whose payload was nulled above. This foreign erase
        // ABI expects two iterator/predicate frames with self-pointers at +0xC/+4.
        // a.f2 is opaque iterator scratch copied into b.f0; no initialized value
        // is established here. Do not reinterpret these frames as dialog storage.
        struct fn_802567C4_TailA { void* f0; void* f1; void* f2; void* f3; };
        struct fn_802567C4_TailB { void* f0; void* f1; };
        struct fn_802567C4_Tail { fn_802567C4_TailA a; fn_802567C4_TailB b; };
        fn_802567C4_Tail t;
        t.a.f3 = (void*)&t.a;
        t.b.f1 = (void*)&t.a;
        t.a.f0 = 0;
        t.b.f0 = t.a.f2;
        PuppetChildList_EraseMatchingValue(&self->count, &t.b, &t.a, 0);
    }
}

// Request close only when both stored blockers are clear.
void CommonDialogRegistry_RequestEligibleClose(fn_802568D0_List* list) {
    fn_802568D0_Node* sentinel = list->first;
    fn_802568D0_Node* node = sentinel;
    while (node != (fn_802568D0_Node*)&list->sentinel) {
        fn_802568D0_Object* object = (fn_802568D0_Object*)node->object;
        if (object != 0) {
            if (object->closeBlock34 == 0) { // First independent close blocker.
                    // The second blocker must also be clear before requesting close.
                if (object->blockRegistryClose == 0) {
                    ((Q4d*)object)->v18();
                }
            }
        }
        node = node->prev;
    }
}

#pragma push
#pragma opt_common_subs off
// Retain and register a dialog once, returning whether insertion occurred.
int CommonDialogRegistry_AddUnique(fn_80256948_Self* self, fn_80256C70_Obj* object) {
    if (object == 0)
        return 0;
    fn_80256948_Node* sentinel = (fn_80256948_Node*)&self->sentinel;
    fn_80256948_Node* node = self->first;
    while (node != sentinel) {
        if (node->object == object)
            return 0;
        node = node->prev;
    }
    ++object->refcount;

    volatile char allocator = 0;
    fn_80256948_Node* actual = (fn_80256948_Node*)&self->sentinel;
    fn_80256948_Node* added = (fn_80256948_Node*)appMallocDummy__FUl(12);
    fn_80256948_Node* volatile allocation = added;
    fn_80256948_Node* volatile result = added;
    fn_80256948_Guard guard;
    guard.self = (fn_80256948_Self*)((char*)self + 8);
    guard.node = added;
    new (&added->object) fn_80256948_Pointer(object);
    guard.node = 0;

    actual->next->prev = added;
    added->next = actual->next;
    actual->next = added;
    added->prev = actual;
    ++self->count;
    return 1;
}
#pragma pop

// Release and unlink the matching dialog, returning whether it existed.
int CommonDialogRegistry_Remove(fn_80256A50_Self* self, void* target) {
    if (target == 0)
        return 0;
    fn_80256A50_Node* sentinel = (fn_80256A50_Node*)((char*)self + 8);
    fn_80256A50_Node* node = self->first;
    while (node != sentinel) {
        if (node->object == target) {
                    fn_80256A50_Obj* obj = (fn_80256A50_Obj*)node->object;
            if (--obj->refcount == 0) {
                if (obj != 0) {
                    obj->v0(1);
                }
            }
            node->next->prev = node->prev;
            node->prev->next = node->next;
            appFreeUni__FPv(node);
            unsigned int c = self->count;
            --c;
            self->count = c;
            return 1;
        }
        node = node->prev;
    }
    return 0;
}

#pragma push
#pragma opt_common_subs off
// Initialize an empty registry with its embedded circular sentinel.
fn_80256B18_List* RefObjectList_Init(fn_80256B18_List* self) {
    fn_80256654_Node* sentinel = (fn_80256654_Node*)((int)self + 8);
    fn_80256654_Node* node = (fn_80256654_Node*)((unsigned int)self + 8u);
    self->sentinel = node;
    self->pad = kDialogRegistryIdle;
    self->count = 0;
    self->first = node;
    fn_80256B18_Clear(self, sentinel, node);
    return self;
}
#pragma pop

// Destroy list-node storage and optionally delete the list container.
void* dtor_80256BB4(MSLPointerList* self, int shouldDelete) {
    return ::std::__list_deleting_dtor(self, shouldDelete);
}

// Release each dialog reference before freeing its registry node.
void RefObjectList_ClearAndRelease(fn_80256C70_Self* self) {
    {
        fn_80256C70_Node* sentinel = (fn_80256C70_Node*)((char*)self + 8);
        fn_80256C70_Node* node = self->first;
        while (node != sentinel) {
            fn_80256C70_Obj* obj = (fn_80256C70_Obj*)node->object;
            if (obj != 0) {
                unsigned int rc = obj->refcount - 1;
                obj->refcount = rc;
                if (rc == 0) {
                    if (obj != 0) {
                        obj->destroy(1);
                    }
                }
            }
            node = node->prev;
        }
    }
    {
        fn_80256C70_Node* sentinel = (fn_80256C70_Node*)(void*)&self->sentinel;
        register fn_80256C70_Node* node = self->first;
        if (node != sentinel) {
            fn_80256C70_Node* tail = sentinel->next;
            node->next->prev = tail->prev;
            tail->prev->next = node->next;
            while (node != sentinel) {
                fn_80256C70_Node* old = node;
                node = node->prev;
                appFreeUni__FPv(old);
                --self->count;
            }
        }
    }
}

// Initialize the fade-dialog helper's cached full-turn angle.
void InitFadeDialogHelperFullTurnAngle() {
    s_fadeDialogHelperFullTurnAngle = kFloat_6_2831855;
}

// Allocate a fade dialog into the caller's required output slot.
int CInGameFadeDialog_Create(void* self, void** out) {
    if (out == 0) return 0;
    FadeDialogObject* obj = new FadeDialogObject(self);
    *out = obj;
    return obj != 0;
}

// Replace the script name and discard owned visibility-argument text.
void GolfTipsDialog_SetScriptNameAndClearArguments(fn_80256E84_Self* self, const char* str) {
    unsigned int size;
    if ((self->flags >> 31) == 0)
        size = self->smallSize & 0x7f;
    else
        size = self->largeSize;
    const char* s = str;
    volatile char a;
    char b = a;
    unsigned int len = strlen(s);
    String_ReplaceRange(&self->flags, 0, size, s, s + len, &b);
    unsigned int count = self->count;
    self->count = self->count - count;
    fn_80256E84_Elem* p = (fn_80256E84_Elem*)self->array + count;
    unsigned int i = count;
    while (i != 0) {
        --p;
        if (p != 0) {
            if (p != 0) {
                if ((p->flags >> 31) != 0) {
                    appFreeUni__FPv(p->object);
                }
            }
        }
        --i;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Copy a group name and append its borrowed visibility target.
void InGameFadeDialog_AppendGroupVisibilityOverride(VisOverrideDialogView* self, const char* text, void* item) {
    // Phase 1: build an owned name/value temporary; allocator markers carry no data.
    unsigned int inlineSize = 0;
    VisOverride_Elem name(inlineSize);
    if ((name.text.flags >> 31) == 0) inlineSize = name.text.small & 0x7f;
    VisOverride_Allocator allocator;
    VisOverride_Allocator marker = allocator;
    unsigned int length = strlen(text);
    String_ReplaceRange(&name.text, 0, inlineSize, text, text + length, (char*)&marker);
    name.item = item;
    VisOverride_Allocator insertAllocator = VisOverride_Allocator();
    if (self->size < self->capacity) {
        // Spare slot: copy-construct once, then publish the new element count.
        VisOverride_Elem* e = self->data + self->size;
        new (e) VisOverride_Elem(name);
        ++self->size;
    } else {
        // Phase 2: bound the 16-byte element count and allocate a larger buffer.
        VisOverride_Allocator growAllocator = VisOverride_Allocator();
        VisOverride_Allocator limitAllocator = VisOverride_Allocator();
        if (0x0fffffff - self->capacity < 1u) VisOverride_LengthError();
        unsigned int needed = self->size + 1 - self->capacity;
        unsigned int growthHalf;
        unsigned int growthThreeFifths;
        VisOverride_Buffer fresh(&self->capacity);
        unsigned int newCap = self->capacity;
        if (needed > 0x0fffffff - newCap) VisOverride_LengthError();
        if (newCap < 0x05555555u) {
            growthThreeFifths = ((newCap + 1) * 3) / 5;
            newCap += VisOverride_Max(growthThreeFifths, needed);
        } else if (newCap < 0x0aaaaaaau) {
            growthHalf = (newCap + 1) >> 1;
            newCap += VisOverride_Max(growthHalf, needed);
        } else newCap = 0x0fffffff;
        VisOverride_Allocator allocationAllocator = VisOverride_Allocator();
        if (newCap > 0x0fffffffu) VisOverride_LengthError();
        VisOverride_Elem* allocation = (VisOverride_Elem*)appMallocDummy__FUl(newCap * 16);
        unsigned int oldCount = self->size;
        fresh.data = allocation;
        fresh.capacity = newCap;
        fresh.offset = oldCount;
        // Phase 3: construct the appended entry first, then copy existing entries
        // backward into the prefix. offset/size track the constructed live range.
        VisOverride_Elem* e = fresh.size + (allocation + oldCount);
        VisOverride_Allocator copyAllocator = VisOverride_Allocator();
        unsigned int count = 1;
        while (count != 0) {
            new (e) VisOverride_Elem(name);
            --count;
            ++e;
            ++fresh.size;
        }
        VisOverride_Elem* begin = self->data;
        VisOverride_Elem* end = begin + self->size;
        VisOverride_Elem* dest = fresh.data + fresh.offset;
        VisOverride_Allocator moveAllocator = VisOverride_Allocator();
        while (end > begin) {
            --dest;
            --end;
            new (dest) VisOverride_Elem(*end);
            --fresh.offset;
            ++fresh.size;
        }
        // Phase 4: publish only after copying; fresh's destructor frees old storage.
        VisOverride_Exchange(self, fresh);
    }
}
#pragma pop

// Release an owned heap string and optionally delete its wrapper.
void* NString_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        if (self != 0) {
            if (*(unsigned int*)self >> 31)
                appFreeUni__FPv(((NString*)self)->data);
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Replace the fade dialog's owned narrow text-group name.
void InGameFadeDialog_SetTextGroupName(fn_8025770C_Arg* self, const char* str) {
    unsigned int size;
    if ((self->flags >> 31) == 0)
        size = self->smallSize & 0x7f;
    else
        size = self->largeSize;
    volatile struct { char value; char padding[3]; } source;
    char a = source.value;
    char local = a;
    unsigned int len = strlen(str);
    String_ReplaceRange(&self->flags, 0, size, str, str + len, (char*)&local);
}

// Destroy the visibility-override vector and optionally its container.
void* __dt__14FadeDialogListFv(FadeDialogStringVector* self, int shouldDelete) {
    return ::std::__vector_deleting_dtor(self, shouldDelete);
}

// Stop dialog behavior and release scripts, overrides and owned strings.
void* CInGameFadeDialog_dtor(fn_80257858_Self* self, int shouldDelete) {
    // Phase 1: stop dialog behavior and unregister its BP entry.
    if (self != 0) {
        self->vtable = __vt__17CInGameFadeDialog;
        ((fn_80256D50_Object*)self)->v6();
        BpCtrl_SetOpacity(self->handle, 0x80);
        if (self->handle != -1) {
            if (((unsigned int&)Sing_805DC5F0()) == 0) {
                if (!s_bpCtrlInitialized) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_InGameFadeDialog);
                    s_bpCtrlInitialized = 1;
                }
                ((unsigned int&)Sing_805DC5F0()) = (unsigned int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((unsigned int&)Sing_805DC5F0()), self->handle);
            self->handle = -1;
        }
        // Phase 2: release the trailing group string, override names, then leading string.
        if (&self->flagsB != 0) {
            if ((self->flagsB >> 31) != 0) appFreeUni__FPv(self->ptrB);
        }
        if (&self->array != 0) {
            if (&self->array != 0) {
                if (&self->array != 0) {
                    if (&self->array != 0) {
                        if (self->array != 0) {
                            fn_80257858_Elem* p;
                            unsigned int i;
                            i = self->arrayCount;
                            p = self->array + i;
                            self->arrayCount = self->arrayCount - i;
                            while (i != 0) {
                                --p;
                                if (p != 0) {
                                    if (p != 0) {
                                        if ((p->flags >> 31) != 0) appFreeUni__FPv(p->object);
                                    }
                                }
                                --i;
                            }
                            appFreeUni__FPv(self->array);
                        }
                    }
                }
            }
        }
        if (&self->flagsA != 0) {
            if ((self->flagsA >> 31) != 0) appFreeUni__FPv(self->ptrA);
        }
        // Phase 3: destroy the base and honor the deleting-destructor flag.
        __dt__15SmallDialogBaseFv(self, 0);
        if (shouldDelete > 0) __dl__FPv(self);
    }
    return self;
}
} // extern "C"
