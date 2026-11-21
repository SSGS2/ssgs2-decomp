#include <decomp/static_init.h>
#include <decomp/ct_member.h>
typedef unsigned long new_size_t;
// Soft keyboard name entry and Shop2 task lifecycle (separate receiver types).
// Mod lookup: page/grid data -> KeyboardPageEntry and AppendEntry; clickable
// regions -> SoftKeyboard_SetRectangle; character capacity -> AllocateInputBuffers;
// UTF-16 output -> TextInput_CopyWideText. Input/hit consumers are in wgeneric3d_render_queue_and_soft_keyboard.cpp.
// Pages own their vector allocation but BORROW UTF-16 page text; the keyboard
// destructor frees the vector and its own editable text, never page character tables.
// String/exception pool: layoutControlAndExceptionStrings (shared with input).
#include "classes/MWStringStorage.h"
// Field names follow the keyboard processing view in wgeneric3d_render_queue_and_soft_keyboard.cpp.
struct SoftKeyboardView {
    void* vtable; float pointerX, pointerY;
    MWStringStorage text;
    void* keyCoordinates; void* characterPages; void* characterModifiers;
    unsigned int hoverState, hoverX, hoverY, reserved30;
    int cursorIndex; int pageIndex; int textLength;
    // Four 16-byte records of signed s32 UI-pixel {left,top,right,bottom},
    // copied here as raw words. wgeneric3d_render_queue_and_soft_keyboard.cpp's hit() adds offsetX/Y then tests
    // left < pointerX < right and top < pointerY < bottom (edges excluded).
    // Slots 0/1/2 are grid/accept/delete; slot 3 has no hit consumer there.
    // Coordinates are relative to keyboard offset; no scale conversion here.
    unsigned int rectangles[4][4];
    void* pages; unsigned int storedPageCount, pageCapacity;
    void* modifierTables; unsigned int modifierCount, modifierCapacity;
    void* owner; int maxCharacters; unsigned int pageCount;
};
static inline SoftKeyboardView* AsSoftKeyboard(void* object) { return (SoftKeyboardView*)object; }
// First three words of CShop2Task: each is set in CShop2Task_Ctor to an address inside __vt__10CShop2Task
// (the main task vtable, then the +0x10 and +0x1C offsets for the actor interfaces).
struct ShopTaskInterfacePointers { void* task; void* actor; void* secondaryActor; };
struct ShopActorStringPool { char empty[1], className[11], actorName[6]; };
enum { kTaskMainActorOffset = 0x18, kTaskSecondaryActorOffset = 8,
       kActorVtableOffset = 0x10, kSecondaryActorVtableOffset = 0x1C,
       kPageStorageOffset = (int)&((SoftKeyboardView*)0)->pages,
       kModifierStorageOffset = (int)&((SoftKeyboardView*)0)->modifierTables,
       kKeyboardTextOffset = (int)&((SoftKeyboardView*)0)->text,
       kInlineStringLengthMask = 0x7F };

typedef char keyboard_text_at_0C[((int)&((SoftKeyboardView*)0)->text == 0x0C) ? 1 : -1];
typedef char keyboard_rectangles_at_40[((int)&((SoftKeyboardView*)0)->rectangles == 0x40) ? 1 : -1];
typedef char keyboard_pages_at_80[((int)&((SoftKeyboardView*)0)->pages == 0x80) ? 1 : -1];
typedef char keyboard_modifiers_at_8C[((int)&((SoftKeyboardView*)0)->modifierTables == 0x8C) ? 1 : -1];
typedef char keyboard_max_at_9C[((int)&((SoftKeyboardView*)0)->maxCharacters == 0x9C) ? 1 : -1];
typedef char keyboard_page_count_at_A0[((int)&((SoftKeyboardView*)0)->pageCount == 0xA0) ? 1 : -1];

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char layoutControlAndExceptionStrings[];
}
}

// Translation unit 0x80080FB4-0x8008199C (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern char s_taskMainTypeInfo;
extern char s_shop2MainTypeInfo;
extern char s_shop2TaskRttiDescriptor;
extern char __vt__10CShop2Task[];
extern char shop2MainActorStrings[];
extern char cSoftKeyboardVtable[];
extern int g_defaultActorSkipMask;
extern int g_actorSkipMask;
extern float kFloat_6_2831855;
extern float s_twoPi_TaskMain;
extern float s_twoPi_Shop2Task;

void destroy__9CTaskMainFi(void*, int);

void destroy__10CShop2MainFi(void*, int);
void* CShop2Task_dtor(void*, int);
void* CTask_Ctor(void*);
void wcscpy(void*, void*);
unsigned int strlen(const char*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
void* CTask_SetMainActor(void*, void*);
void appFreeUni__FPv(void*);
void __dla__FPv(void*);
void __dl__FPv(void*);
void CTask_dtor(void*, int);
void* __nwa__FUl(unsigned int);
void WString_ReplaceRepeatedChar(void*, unsigned int, unsigned int, unsigned int, unsigned int);
void* appMallocDummy__FUl(unsigned int);

// Default task-main lifecycle hooks intentionally have no side effects.
void CTaskMain_OnReload() {}
void CTaskMain_vfunc_16() {}
void CTaskMain_HandleMsg() {}
void CTaskMain_OnDisplay() {}
void CTaskMain_OnProcess() {}
void CTaskMain_OnDestroy() {}
void CTaskMain_OnLoad() {}
void CTaskMain_OnPreLoadInit() {}
void* CTaskMain_GetRTTI() { return &s_taskMainTypeInfo; }
void* CShop2Main_GetRTTI() { return &s_shop2MainTypeInfo; }
void CTaskMain_DestroyThunk18(void* self, int flag) { destroy__9CTaskMainFi(static_cast<char*>(self) - kTaskMainActorOffset, flag); }
void CShop2Main_DestroyThunk18(void* self, int flag) { destroy__10CShop2MainFi(static_cast<char*>(self) - kTaskMainActorOffset, flag); }
void IActor_OnInit() {}
// Initialize the task-main rotation-period constant during static startup.
void TaskMain_InitRuntimeFullTurn() { s_twoPi_TaskMain = kFloat_6_2831855; }

void CTask_StartActorLoading(void*);
void CTask_ProcessActors(void*, float);
void CTask_DisplayActorsByPriority(void*);

// Forward actor loading to the base task.
void CShop2Task_StartActorLoading(void* self) { CTask_StartActorLoading(self); }
// Advance base-task actors by the supplied time step.
void CShop2Task_ProcessActors(void* self, float timeStep) { CTask_ProcessActors(self, timeStep); }
// Draw the task's actors in base-task priority order.
void CShop2Task_DisplayActorsByPriority(void* self) { CTask_DisplayActorsByPriority(self); }
// Create the Shop2 main actor with an empty initialization string and make it current.
void CShop2Task_CreateActors(void* self) {
    char* base = shop2MainActorStrings;
    struct Local { volatile char pad[4]; unsigned int temp[3]; } local = {};
    unsigned int len = strlen(base);
    String_ReserveCapacity(local.temp, len);
    char ch = local.pad[0];
    String_ReplaceRange(local.temp, 0, 0, base, base + len, &ch);
    void* result = TaskActorTable_CreateNamedActor(self, base + (int)&((ShopActorStringPool*)0)->className, base + (int)&((ShopActorStringPool*)0)->actorName, 0, local.temp);
    CTask_SetMainActor(self, result);
    if (local.temp[0] >> 31)
        appFreeUni__FPv(reinterpret_cast<void*>(local.temp[2]));
}
// Release character metadata, page/modifier vectors and heap text, then optionally delete.
void* SoftKeyboard_dtor(void* self, int shouldDelete) {
    // Repeated non-null expressions preserve the original compiler's inlining shape.
    // Plain C: free the three per-character arrays, the modifier and page vectors, then heap text; delete self if requested.
    if (self != 0) {
        AsSoftKeyboard(self)->vtable = cSoftKeyboardVtable;
        void* p = AsSoftKeyboard(self)->keyCoordinates;
        if (p) { __dla__FPv(p); AsSoftKeyboard(self)->keyCoordinates = 0; }
        p = AsSoftKeyboard(self)->characterPages;
        if (p) { __dla__FPv(p); AsSoftKeyboard(self)->characterPages = 0; }
        p = AsSoftKeyboard(self)->characterModifiers;
        if (p) { __dla__FPv(p); AsSoftKeyboard(self)->characterModifiers = 0; }
        if ((int)self + kModifierStorageOffset && (int)self + kModifierStorageOffset && (int)self + kModifierStorageOffset && (int)self + kModifierStorageOffset) {
            p = AsSoftKeyboard(self)->modifierTables;
            if (p) {
                volatile int* modifierCount = (volatile int*)&AsSoftKeyboard(self)->modifierCount;
                int oldCount = *modifierCount; *modifierCount = oldCount - oldCount;
                appFreeUni__FPv(p);
            }
        }
        if ((int)self + kPageStorageOffset && (int)self + kPageStorageOffset && (int)self + kPageStorageOffset && (int)self + kPageStorageOffset) {
            p = AsSoftKeyboard(self)->pages;
            if (p) {
                volatile int* pageCount = (volatile int*)&AsSoftKeyboard(self)->storedPageCount;
                int oldCount = *pageCount; *pageCount = oldCount - oldCount;
                appFreeUni__FPv(p);
            }
        }
        if ((int)self + kKeyboardTextOffset && (int)self + kKeyboardTextOffset && (int)self + kKeyboardTextOffset && AsSoftKeyboard(self)->text.flags >> 31)
            appFreeUni__FPv(AsSoftKeyboard(self)->text.data);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}
void* CShop2Task_GetRTTI() { return &s_shop2TaskRttiDescriptor; }
// Initialize the shop-task rotation-period constant during static startup.
void Shop2Task_InitRuntimeFullTurn() { s_twoPi_Shop2Task = kFloat_6_2831855; }
void CShop2Task_vfunc_1_Thunk8(void* self, int deleting) { CShop2Task_dtor(static_cast<char*>(self) - kTaskSecondaryActorOffset, deleting); }

// Initialize empty keyboard text and page/modifier vectors for the supplied owner.
void* SoftKeyboard_Ctor(void* self, int arg) {
    void* vtable = cSoftKeyboardVtable;
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->text.flags) = 0;
    AsSoftKeyboard(self)->vtable = vtable;
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->text.length) = 0;
    AsSoftKeyboard(self)->text.data = 0;
    AsSoftKeyboard(self)->pages = 0;
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->storedPageCount) = 0;
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->pageCapacity) = 0;
    AsSoftKeyboard(self)->modifierTables = 0;
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->modifierCount) = 0;
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->modifierCapacity) = 0;
    AsSoftKeyboard(self)->owner = reinterpret_cast<void*>(arg);
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->maxCharacters) = 0;
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->pageCount) = 0;
    AsSoftKeyboard(self)->cursorIndex = 0;
    *reinterpret_cast<void**>(&AsSoftKeyboard(self)->reserved30) = 0;
    AsSoftKeyboard(self)->textLength = 0;
    AsSoftKeyboard(self)->pageIndex = 0;
    AsSoftKeyboard(self)->keyCoordinates = 0;
    AsSoftKeyboard(self)->characterPages = 0;
    return self;
}

// Copy four rectangle words while preserving the source's volatile load order.
// Required self/source; index 0..3 selects grid/accept/delete/reserved region.
// source supplies four signed-pixel {left,top,right,bottom} words; return status 1.
// Partial overlap observes the explicit interleaved loads/stores below, not memmove.
// No validation/clamping/ownership change; consumer boundary tests exclude edges.
int SoftKeyboard_SetRectangle(void* self, int index, void* source) {
    int offset = index << 4;
    volatile unsigned int* src = reinterpret_cast<volatile unsigned int*>(source);
    unsigned int first = src[0];
    // Keep the indexed base calculation before the rectangle member offset.
    SoftKeyboardView* keyboard = (SoftKeyboardView*)(static_cast<char*>(self) + offset);
    unsigned int second = src[1];
    keyboard->rectangles[0][0] = first;
    int result = 1;
    unsigned int third = src[2];
    keyboard->rectangles[0][1] = second;
    unsigned int fourth = src[3];
    keyboard->rectangles[0][2] = third;
    keyboard->rectangles[0][3] = fourth;
    return result;
}

#pragma exceptions on
#pragma cpp_exceptions on
} // extern C
// Construct in caller-provided page-vector storage; do not allocate or take ownership.
inline void* operator new(new_size_t, void* p) { return p; }
// A failed placement construction leaves storage owned by the vector growth guard.
inline void operator delete(void*, void*) {}
extern "C" {
struct KeyboardPageEntry {
    // 0x1C-byte native record: signed-pixel rectangle (+0..C), borrowed UTF-16
    // text pointer (+10), grid columns/rows (+14/+18). Copied as unsigned words
    // to preserve all bits; no pointer widening or text lifetime change occurs.
    unsigned int x0,y0,x1,y1,textAddress,columns,rows;
    // Copy one page's geometry, borrowed character-table pointer and grid dimensions.
    KeyboardPageEntry(const KeyboardPageEntry& x) {
        unsigned int top=x.y0,left=x.x0; x0=left; y0=top;
        unsigned int bottom=x.y1,right=x.x1; x1=right; y1=bottom;
        unsigned int columnCount=x.columns,textBits=x.textAddress; textAddress=textBits; columns=columnCount;
        rows=x.rows;
    }
};
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[], layoutControlAndExceptionStrings[];
char* strcpy(char*, const char*);
void* __nw__FUl(unsigned int);
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct ArrayNewGuard {
    void* p; char* flag;
    // Own message bytes until their shared control block has been constructed.
    ArrayNewGuard(void* q, char* f):p(q),flag(f) {}
    // Construction failure releases untransferred new[] storage; p=0 commits it.
    ~ArrayNewGuard() { if(p) __dla__FPv(p); }
};
struct AllocatorTag {
    volatile char x; char pad[3];
    // Preserve the allocator's volatile initialization marker; it owns no storage.
    AllocatorTag() { x=0; }
};
struct SharedCharBuffer {
    void* v; int a,b; void* p;
    // Retain the new message through one strong/one weak shared-control reference.
    SharedCharBuffer(void* q) { a=1; b=1; v=s_sharedCharDeleterVtable; p=q; }
};
struct LogicErrorBase_u80080FB4 {
    void* v; char* text; SharedCharBuffer* ref;
    // Copy required NUL-terminated message and transfer its new[] storage to ref.
    // Guard frees bytes if allocating the control block throws; no borrowed text escapes.
    LogicErrorBase_u80080FB4(const char* s) {
        v=__vt__Q23std11logic_error;
        AllocatorTag flag;
        unsigned int n=strlen(s)+1;
        char* allocated=(char*)__nwa__FUl((n<<1)-n);
        text=allocated;
        {
        ArrayNewGuard guard(allocated,(char*)&flag.x);
        ref=new SharedCharBuffer(allocated);
        guard.p=0;
        }
        strcpy(text,s);
    }
    // Run the original base-exception cleanup with its ABI destructor flag.
    ~LogicErrorBase_u80080FB4() { __dt__Q23std9exceptionFv(this,-1); }
};
} // extern C
// The next structs re-implement the inlined std::length_error (a logic_error with a shared copy of its
// message text) exactly as the original compiler emitted it. AllocatorTag is a one-byte volatile marker
// object; ThrowPageVectorLengthError builds the error and calls __throw.
namespace std {
struct length_error: LogicErrorBase_u80080FB4 {
    // Build the shared-message base and install the derived exception vtable.
    length_error(const char* s):LogicErrorBase_u80080FB4(s) { v=__vt__Q23std12length_error; }
    ~length_error(); // Runtime-supplied unwinding cleanup; no definition in this unit.
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {
// Construct and throw the original std::length_error for a page-vector overflow.
inline void ThrowPageVectorLengthError() {
    AllocatorTag allocator;
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(layoutControlAndExceptionStrings);
        exception_runtime::__throw(exception_runtime::layoutControlAndExceptionStrings + 0x14, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
enum KeyboardPageGrowth {
    kPageBytes = sizeof(KeyboardPageEntry), kMaxPages = 0x09249249,
    kThreeFifthsGrowthLimit = 0x030C30C3, kHalfGrowthLimit = 0x06186186
};

// Own the replacement allocation until its elements are committed to the keyboard.
struct PageVectorGrowth {
    KeyboardPageEntry* data; unsigned int count; unsigned int cap; unsigned int* owner; unsigned int remaining;
    // Start with no allocation; owner points at the keyboard capacity word.
    PageVectorGrowth(unsigned int* o):data(0),count(0),cap(0),owner(o),remaining(0) {}
    // Before commit free replacement storage; after swap free the old allocation.
    // Page elements only borrow text, so no text destructor is required.
    ~PageVectorGrowth() {
        if(data) { AllocatorTag allocator; count=count-count; appFreeUni__FPv(data); }
    }
};
// Choose growth slack or the required additional pages without copying the operands.
inline const unsigned int& MaxUnsigned(const unsigned int& a,const unsigned int& b) { return a<b?b:a; }

#pragma push
#pragma use_lmw_stmw on
// Append required value (one 28-byte KeyboardPageEntry); return its zero-based
// index and update both storedPageCount and pageCount. self is a live keyboard.
// The record/text are borrowed: value may name an existing page, its text must
// outlive keyboard use. Grid consumers require nonzero columns/rows and valid
// rectangles; this storage operation does not validate page contents.
// Capacity bound is floor(0xFFFFFFFF/28). Overflow throws length_error; allocation
// exceptions propagate. No publication before allocation and new-page copy;
// the guard frees replacement storage on unwinding once it has been installed.
// Existing page copies cannot throw (seven scalar words). Marker objects and
// reverse copy preserve exception metadata and the original memory access order.
unsigned int CSoftKeyboardNameEntry_AppendEntry(void* self, void* value) {
    AllocatorTag allocator;
    unsigned int count = AsSoftKeyboard(self)->storedPageCount;
    unsigned int capacity = AsSoftKeyboard(self)->pageCapacity;
    if (count < capacity) {
        // Fast append: reserve the live slot, then copy the page. No allocation.
        unsigned int next = count + 1;
        AsSoftKeyboard(self)->storedPageCount = next;
        unsigned int* dst = reinterpret_cast<unsigned int*>((char*)AsSoftKeyboard(self)->pages + (next-1) * kPageBytes);
        unsigned int* src=(unsigned int*)value;
        unsigned int top=src[1], left=src[0], right=src[2], bottom=src[3];
        dst[0]=left; unsigned int text=src[4]; dst[1]=top;
        unsigned int columns=src[5]; dst[2]=right; unsigned int rows=src[6];
        dst[3]=bottom; dst[4]=text; dst[5]=columns; dst[6]=rows;
    } else {
        // Growth 1/4: validate byte-count limit and select bounded geometric slack.
        AllocatorTag copyAllocator;
        AllocatorTag checkAllocator;
        if (kMaxPages-capacity < 1) ThrowPageVectorLengthError();
        unsigned int requested = AsSoftKeyboard(self)->storedPageCount + 1 - AsSoftKeyboard(self)->pageCapacity;
        PageVectorGrowth growth(&AsSoftKeyboard(self)->pageCapacity);
        unsigned int old = AsSoftKeyboard(self)->pageCapacity;
        unsigned int maxSize = kMaxPages;
        unsigned int newCap;
        if(requested > maxSize-old) ThrowPageVectorLengthError();
        if (old < kThreeFifthsGrowthLimit) {
            unsigned int candidate = (old + 1) * 3 / 5;
            newCap = old + MaxUnsigned(candidate,requested);
        } else if (old < kHalfGrowthLimit) {
            unsigned int candidate = (old+1) / 2;
            newCap = old + MaxUnsigned(candidate,requested);
        } else newCap = maxSize;
        AllocatorTag allocationAllocator;
        if(newCap>maxSize) ThrowPageVectorLengthError();
        // Growth 2/4: allocate and copy the NEW page first, keeping aliased value
        // valid until its read is complete. Only then move the old page records.
        unsigned int oldCount = AsSoftKeyboard(self)->storedPageCount;
        unsigned int oldCapacity = AsSoftKeyboard(self)->pageCapacity;
        unsigned int* data = reinterpret_cast<unsigned int*>(appMallocDummy__FUl(newCap * kPageBytes));
        oldCount = AsSoftKeyboard(self)->storedPageCount;
        unsigned int* at = (unsigned int*)((char*)data + oldCount * kPageBytes);
        unsigned int* target=(unsigned int*)((char*)at+growth.count*kPageBytes);
        unsigned int nextCount=growth.count+1;
        unsigned int* src=(unsigned int*)value;
        unsigned int top=src[1], left=src[0], right=src[2], bottom=src[3];
        target[0]=left; unsigned int text=src[4]; target[1]=top;
        unsigned int columns=src[5]; target[2]=right; unsigned int rows=src[6];
        target[3]=bottom; target[4]=text; target[5]=columns; target[6]=rows;
        unsigned int* oldData = reinterpret_cast<unsigned int*>(AsSoftKeyboard(self)->pages);
        growth.data=(KeyboardPageEntry*)data; growth.cap=newCap; growth.remaining=oldCount; growth.count=nextCount;
        unsigned int* from = (unsigned int*)((char*)oldData + AsSoftKeyboard(self)->storedPageCount * kPageBytes);
        unsigned int* to = at;
        AllocatorTag rangeAllocator;
        AllocatorTag constructionAllocator;
        // Growth 3/4: copy [0,oldCount) backwards; growth.count counts completed
        // records and remaining is the untransferred prefix length.
        while (from > oldData) {
            from -= 7; to -= 7;
            new(to) KeyboardPageEntry(*(KeyboardPageEntry*)from);
            --growth.remaining; ++growth.count;
        }
        // Growth 4/4: publish replacement pointer/capacity/count, leaving oldData
        // in the guard for release. pageCount is synchronized below both paths.
        AllocatorTag clearAllocator;
        AsSoftKeyboard(self)->storedPageCount = 0;
        KeyboardPageEntry* p=growth.data; unsigned int n=growth.count;
        growth.data=(KeyboardPageEntry*)oldData;
        unsigned int c0=AsSoftKeyboard(self)->pageCapacity;
        AsSoftKeyboard(self)->pageCapacity = growth.cap;
        growth.cap=c0;
        AsSoftKeyboard(self)->pages = p;
        AsSoftKeyboard(self)->storedPageCount = n;
        growth.count=0;
    }
    unsigned int result=AsSoftKeyboard(self)->storedPageCount;
    AsSoftKeyboard(self)->pageCount = result;
    return result-1;
}
#pragma pop

// Copy inline or heap-backed UTF-16 text to the caller's output buffer.
int TextInput_CopyWideText(void* self, void* outputText) {
    unsigned int flags = AsSoftKeyboard(self)->text.flags;
    void* ptr;
    if (flags >> 31)
        ptr = AsSoftKeyboard(self)->text.data;
    else
        ptr = (unsigned short*)&AsSoftKeyboard(self)->text.flags + 1;
    wcscpy(outputText, ptr);
    return 1;
}

// Initialize the task interfaces and clear both actor skip masks.
void* CShop2Task_Ctor(void* self) {
    CTask_Ctor(self);
    ((ShopTaskInterfacePointers*)self)->task = __vt__10CShop2Task;
    ((ShopTaskInterfacePointers*)self)->actor = __vt__10CShop2Task + kActorVtableOffset;
    ((ShopTaskInterfacePointers*)self)->secondaryActor = __vt__10CShop2Task + kSecondaryActorVtableOffset;
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
    return self;
}

// Select the active keyboard page.
// value carries an integer index in pointer bits; caller supplies [0,pageCount).
// No validation; return status 1 and keep all page/text storage owned as before.
int SoftKeyboard_SetPageIndex(void* self, void* value) {
    AsSoftKeyboard(self)->pageIndex = reinterpret_cast<int>(value);
    return 1;
}

// Destroy base task state and optionally free the derived task.
void* CShop2Task_dtor(void* self, int shouldDelete) {
    if (self != 0) {
        CTask_dtor(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Allocate per-character position/page/modifier arrays and resize the text with zero fill.
// Required constructed self, nonnegative count with count*8 fitting allocation size.
// Caller must free previous arrays before reuse: this replaces their pointers.
// Publishes each allocation in sequence; exceptions can leave partial new buffers.
// count is UTF-16 code units, not Unicode scalar values. String STORAGE length
// follows count; the separate logical textLength/cursor are not assigned here.
void SoftKeyboard_AllocateInputBuffers(void* self, int count) {
    char* base = static_cast<char*>(self);
    int* maxCharacters = &AsSoftKeyboard(self)->maxCharacters;
    *maxCharacters = count;
    int saved = count;
    void* alloc1 = __nwa__FUl(saved << 3);
    AsSoftKeyboard(self)->keyCoordinates = alloc1;
    void* alloc2 = __nwa__FUl(saved << 2);
    AsSoftKeyboard(self)->characterPages = alloc2;
    void* alloc3 = __nwa__FUl(saved);
    AsSoftKeyboard(self)->characterModifiers = alloc3;
    unsigned int flags = AsSoftKeyboard(self)->text.flags;
    unsigned int textLength = !(flags >> 31)
        ? (*reinterpret_cast<unsigned char*>(base + kKeyboardTextOffset) & kInlineStringLengthMask)
        : AsSoftKeyboard(self)->text.length;
    if ((unsigned int)count > textLength) {
        unsigned int old = textLength;
        void* text = base + kKeyboardTextOffset;
        WString_ReplaceRepeatedChar(text, old, 0, count - old, 0);
    } else {
        void* text = base + kKeyboardTextOffset;
        unsigned int diff = textLength - count;
        WString_ReplaceRepeatedChar(text, count, diff, 0, 0);
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046202C", _ctors_u80080FB4_0, TaskMain_InitRuntimeFullTurn);
STATIC_INIT_ENTRY(".CRT$XCU8046202C", _ctors_u80080FB4_1, Shop2Task_InitRuntimeFullTurn);
#pragma pop
