#include <decomp/static_init.h>
#include "classes/GtlDvdFileStream.h"
// Translation unit 0x80412070-0x80412370 (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/DvdStreamFields.h"
extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_80412070_AudioEffectList;
extern char __vt__Q43gtl4nw4r2ut13DvdFileStream[];
void __dl__FPv(void*);
int DVDFastOpen(void*, void*);
void gtl_DvdFilePosition_SeekClamped(void*, int, int);

enum { kDefaultDvdPriority = 2, kSeekFromStart = 0, kCloseVirtualSlot = 4 };

struct DvdFileStreamState {
    void* vtable;
    unsigned char isOpen;
    unsigned char pad05[3];
    void* readResult;
    void* readCallback;
    void* readUserData;
    unsigned int length;
    unsigned int position;
    void* cancelCallback;
    void* cancelUserData;
    unsigned char cancelPending;
    unsigned char pad25[3];
    unsigned char fileInfo[0x3C];
    void* callbackOwner;
    int priority;
    unsigned char busy;
    unsigned char closeOnDestroy;
    unsigned char ownsFile;
};
// fileInfo is the embedded SDK block at +0x28; its adjacent callbackOwner at +0x64
// lets SDK completions recover self. Callbacks/user data are borrowed, never freed here.
// length/position are bytes. closeOnDestroy controls closing; deleteFlag controls storage.

typedef char DvdFileStreamState_priority_offset[((int)&((DvdFileStreamState*)0)->priority == 0x68) ? 1 : -1];
typedef char DvdFileStreamState_ownership_offset[((int)&((DvdFileStreamState*)0)->ownsFile == 0x6E) ? 1 : -1];

// Open a DVD entry, initialize its callbacks, and reset the clamped read position.
// entryNumber carries an SDK integer entry index in the existing pointer-shaped ABI.
// Returns self even if DVDFastOpen fails; inspect isOpen for success. Successful open is owned.
void* gtl_nw4r_ut_DvdFileStream_Ctor(DvdFileStreamState* self, void* entryNumber) {
    // Phase 1: publish a closed, idle stream with no pending callbacks.
    self->busy = 0;
    self->vtable = __vt__Q43gtl4nw4r2ut13DvdFileStream;
    self->length = 0;
    self->position = 0;
    self->closeOnDestroy = 0;
    self->ownsFile = 0;
    self->isOpen = 0;
    self->priority = kDefaultDvdPriority;
    self->readCallback = 0;
    self->readUserData = 0;
    self->readResult = 0;
    self->cancelCallback = 0;
    self->cancelPending = 0;
    self->cancelUserData = 0;
    self->callbackOwner = self;
    // Phase 2: acquire the SDK file and enable destructor closing only on success.
    if (DVDFastOpen(entryNumber, self->fileInfo)) {
        self->length = *(unsigned int*)&AsDvdStreamFields(self)->fileLength;
        gtl_DvdFilePosition_SeekClamped(&self->length, 0, kSeekFromStart);
        self->closeOnDestroy = 1;
        self->ownsFile = 1;
        self->isOpen = 1;
    }
    return self;
}

// The DVD SDK returns the embedded fileInfo pointer to completion callbacks.
struct DvdCallbackFileInfo {
    char reserved[0x3C];
    GtlDvdFileStream* owner;
};
struct IntrusiveLink {
    IntrusiveLink* next;
    IntrusiveLink* prev;
};
// Circular intrusive lists store {count, sentinel.next, sentinel.prev} in the header.
// Links belong to their enclosing objects. Detaching clears both link words and
// decrements count without freeing payloads. Callers provide valid linked nodes and
// arrange synchronization (voice/task callers commonly disable interrupts).

// Detach one node unless self-linked, and return its saved successor without changing *position.
// position points to a caller's node iterator, not to the list sentinel or a writable next iterator.
void* IntrusiveList_UnlinkNodeAndDecrementCount(int* count, IntrusiveLink** position) {
    IntrusiveLink* link = *position;
    IntrusiveLink* savedNext = link->next;
    // This loop removes at most one node: the first iteration advances to savedNext.
    while (link != savedNext) {
        IntrusiveLink* next = link->next;
        IntrusiveLink* prev = link->prev;
        next->prev = prev;
        prev->next = next;
        *count = *count - 1;
        link->next = 0;
        link->prev = 0;
        link = next;
    }
    return savedNext;
}

struct IntrusiveListState { int count; IntrusiveLink sentinel; };

// Detach every intrusive node without freeing the objects that contain them.
// deleteFlag > 0 frees only the list header. Detached link storage remains caller-owned.
void* gtl_IntrusiveList_Dtor(void* self, int deleteFlag) {
    if (self != 0) {
        IntrusiveLink* head = &((IntrusiveListState*)self)->sentinel;
        IntrusiveLink* link = head->next;
        while (link != head) {
            IntrusiveLink* next = link->next;
            IntrusiveLink* prev = link->prev;
            next->prev = prev;
            prev->next = next;
            ((IntrusiveListState*)self)->count = ((IntrusiveListState*)self)->count - 1;
            link->next = 0;
            link->prev = 0;
            link = next;
        }
        if (deleteFlag > 0) __dl__FPv(self);
    }
    return self;
}

// Close an owned stream through its virtual close method, then optionally free it.
// closeOnDestroy, rather than ownsFile, is the condition actually tested here.
void* gtl_nw4r_ut_DvdFileStream_dtor(void* self, int deleteFlag) {
    if (self != 0) {
        void** vtable;
        unsigned char closeOnDestroy = ((DvdFileStreamState*)self)->closeOnDestroy;
        *(void**)self = __vt__Q43gtl4nw4r2ut13DvdFileStream;
        if (closeOnDestroy != 0) {
            typedef void (*CloseMethod)(void*);
            vtable = (void**) *(void**)self;
            CloseMethod close = (CloseMethod)vtable[kCloseVirtualSlot];
            close(self);
        }
        if (deleteFlag > 0) __dl__FPv(self);
    }
    return self;
}

// Insert a detached node before the iterator and increment the list count.
// Returns link; does not update *position. The list borrows the enclosing object's storage.
void* gtl_IntrusiveList_InsertBefore(int* count, IntrusiveLink** position, IntrusiveLink* link) {
    IntrusiveLink* next = *position;
    IntrusiveLink* prev = next->prev;
    link->next = next;
    link->prev = prev;
    next->prev = link;
    prev->next = link;
    *count = *count + 1;
    return link;
}

// Detach one node, decrement the count, and return the next iterator.
// Unlike the guarded iterator helper above, this always decrements, even for a self-linked node.
void* IntrusiveList_UnlinkNode(int* count, IntrusiveLink* link) {
    IntrusiveLink* next = link->next;
    IntrusiveLink* prev = link->prev;
    next->prev = prev;
    prev->next = next;
    *count = *count - 1;
    link->next = 0;
    link->prev = 0;
    return next;
}

// Detach all audio effect nodes without destroying their containing effects.
// count addresses the full list header; the second ABI parameter is unused.
// Audio-controller callers pass only the header, so never read the second argument.
void gtl_AudioEffectList_UnlinkAll(int* count, IntrusiveLink* unused) {
    IntrusiveLink* head = &((IntrusiveListState*)count)->sentinel;
    IntrusiveLink* link = head->next;
    while (link != head) {
        IntrusiveLink* next = link->next;
        IntrusiveLink* prev = link->prev;
        next->prev = prev;
        prev->next = next;
        *count = *count - 1;
        link->next = 0;
        link->prev = 0;
        link = next;
    }
}

// Initialize this audio/list unit's two-pi constant at startup.
void GtlAudioEffectList_InitializeMathConstants() {
    s_twoPi_80412070_AudioEffectList = kFloat_6_2831855;
}

// Clear cancellation-pending before forwarding (result, stream, cancelUserData).
// fileInfo is borrowed SDK storage; result carries the signed SDK result in the existing ABI.
void gtl_DvdFileStream_OnCancelComplete(void* result, void* fileInfo) {
    unsigned char* state = (unsigned char*)((DvdCallbackFileInfo*)fileInfo)->owner;
    ((GtlDvdFileStream*)state)->cancelPending = 0;
    typedef void (*Callback)(void*, void*, void*);
    Callback callback = (Callback)AsDvdStreamFields(state)->cancelCallback;
    if (callback != 0) {
        callback(result, state, AsDvdStreamFields(state)->cancelUserData);
    }
}

// Save the read result and clear busy state before notifying the caller.
// Delivers (result, stream, readUserData); the user callback observes busy == 0 and the saved result.
void gtl_DvdFileStream_OnReadComplete(void* result, void* fileInfo) {
    unsigned char* state = (unsigned char*)((DvdCallbackFileInfo*)fileInfo)->owner;
    ((GtlDvdFileStream*)state)->busy = 0;
    AsDvdStreamFields(state)->readResult = result;
    typedef void (*Callback)(void*, void*, void*);
    Callback callback = (Callback)AsDvdStreamFields(state)->readCallback;
    if (callback != 0) {
        callback(result, state, AsDvdStreamFields(state)->readUserData);
    }
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462710", _ctors_u80412070_0, GtlAudioEffectList_InitializeMathConstants);
#pragma pop
