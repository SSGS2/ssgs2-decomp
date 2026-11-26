// The DVD file stream (gtl::nw4r::ut::DvdFileStream): setup and teardown, and the completion
// handlers for async reads and cancels. Also the intrusive list helpers and the audio effect
// list constants that share this unit.

#include <decomp/static_init.h>
#include "classes/GtlDvdFileStream.h"

#include "classes/DvdStreamFields.h"
extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_80412070_AudioEffectList;
extern char __vt__Q43gtl4nw4r2ut13DvdFileStream[];
void __dl__FPv(void*);
int DVDFastOpen(void*, void*);
void gtl_DvdFilePosition_SeekClamped(void*, int, int);

enum {
    kDefaultDvdPriority = 2,
    kSeekFromStart = 0,
    kCloseVirtualSlot = 4
};

} // extern "C"

// Initialize the stream: open a DVD entry, set up its callbacks, and reset the clamped read position.
// entryNumber carries an SDK integer entry index in the existing pointer-shaped ABI.
// Returns this even if DVDFastOpen fails; inspect isOpen for success. Successful open is owned.
void* gtl::nw4r::ut::DvdFileStream::Init(void* entryNumber) {
    // Phase 1: publish a closed, idle stream with no pending callbacks.
    busy = 0;
    vtable = __vt__Q43gtl4nw4r2ut13DvdFileStream;
    length = 0;
    position = 0;
    closeOnDestroy = 0;
    ownsFile = 0;
    isOpen = 0;
    priority = kDefaultDvdPriority;
    readCallback = 0;
    readUserData = 0;
    readResult = 0;
    cancelCallback = 0;
    cancelPending = 0;
    cancelUserData = 0;
    callbackOwner = this;
    // Phase 2: acquire the SDK file and enable destructor closing only on success.
    if (DVDFastOpen(entryNumber, fileInfo)) {
        length = *(unsigned int*)&AsDvdStreamFields(this)->fileLength;
        gtl_DvdFilePosition_SeekClamped(&length, 0, kSeekFromStart);
        closeOnDestroy = 1;
        ownsFile = 1;
        isOpen = 1;
    }
    return this;
}

extern "C" {

// The DVD SDK returns the embedded fileInfo pointer to completion callbacks.
struct DvdCallbackFileInfo {
    char reserved[0x3C];
    gtl::nw4r::ut::DvdFileStream* owner;
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

struct IntrusiveListState {
    int count;
    IntrusiveLink sentinel;
};

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
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

// Close an owned stream through its virtual close method, then optionally free it.
// closeOnDestroy, rather than ownsFile, is the condition actually tested here.
void* gtl::nw4r::ut::DvdFileStream::Destroy(int deleteFlag) {
    if (this != 0) {
        void** vtbl;
        unsigned char closeOnDestroy = this->closeOnDestroy;
        *(void**)this = __vt__Q43gtl4nw4r2ut13DvdFileStream;
        if (closeOnDestroy != 0) {
            typedef void (*CloseMethod)(void*);
            vtbl = (void**)*(void**)this;
            CloseMethod close = (CloseMethod)vtbl[kCloseVirtualSlot];
            close(this);
        }
        if (deleteFlag > 0)
            __dl__FPv(this);
    }
    return this;
}

extern "C" {

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

} // extern "C"

// Clear cancellation-pending before forwarding (result, stream, cancelUserData).
// fileInfo is borrowed SDK storage; result carries the signed SDK result in the existing ABI.
void gtl::nw4r::ut::DvdFileStream::OnCancelComplete(void* result, void* fileInfo) {
    unsigned char* state = (unsigned char*)((DvdCallbackFileInfo*)fileInfo)->owner;
    ((gtl::nw4r::ut::DvdFileStream*)state)->cancelPending = 0;
    typedef void (*Callback)(void*, void*, void*);
    Callback callback = (Callback)AsDvdStreamFields(state)->cancelCallback;
    if (callback != 0) {
        callback(result, state, AsDvdStreamFields(state)->cancelUserData);
    }
}

// Save the read result and clear busy state before notifying the caller.
// Delivers (result, stream, readUserData); the user callback observes busy == 0 and the saved result.
void gtl::nw4r::ut::DvdFileStream::OnReadComplete(void* result, void* fileInfo) {
    unsigned char* state = (unsigned char*)((DvdCallbackFileInfo*)fileInfo)->owner;
    ((gtl::nw4r::ut::DvdFileStream*)state)->busy = 0;
    AsDvdStreamFields(state)->readResult = result;
    typedef void (*Callback)(void*, void*, void*);
    Callback callback = (Callback)AsDvdStreamFields(state)->readCallback;
    if (callback != 0) {
        callback(result, state, AsDvdStreamFields(state)->readUserData);
    }
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462710", _ctors_u80412070_0, GtlAudioEffectList_InitializeMathConstants);
#pragma pop
