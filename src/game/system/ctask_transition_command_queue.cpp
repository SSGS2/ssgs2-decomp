#pragma ipa file
#pragma auto_inline off
#pragma use_lmw_stmw on
#include "classes/CTask.h"
#include "classes/MWStringStorage.h"
#include "classes/SoundManagerState.h"
#include "classes/NameRegistry801C.h"
#define AsCTask(p) ((CTask*)(p))
// Translation unit 0x801FFA9C-0x80200A14 (23 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
// A command page contains 23 records; its pointer ring tracks allocated pages.
enum TaskCommandConstants {
    kRecordsPerPage = 23, kCommandRecordBytes = 0x2C,
    kRetiredPageThreshold = 2 * kRecordsPerPage, kEmptyFrontOffset = 11,
    kFadeIncreasingAlpha = 1, kFadeTransitionReady = 2, kResumeFadeFrames = 10,
    kRingStateOffset = 8, kTaskCommandQueueOffset = 0x24,
    kNameRegistryOffset = 0x10, kActorThreadOffset = 0x50,
    kLengthErrorTypeOffset = 6, kLoadingFinishNotifiedByte = 4,
    kSecondaryLoadingStatusByte = 5, kCommandPayloadOffset = 0x0C,
    kInlineNameLengthMask = 0x7F
};
// Command records are 0x2C bytes: a 12-byte MW string key, then an opaque
// 32-byte handler payload. Key bit 31 selects heap storage at +8; otherwise
// the bytes start at +1. Dispatch borrows both views until DiscardFront frees
// the key. The eight payload words have no established gameplay definitions.
// The page ring reserves a trailing slot/page; frontOffset counts records,
// not bytes. pageRing.firstPage/capacity address the separate pointer ring.
struct TaskCommandPages {
    void** pages;
    unsigned int firstPage;
    unsigned int pageCount;
    unsigned int pageCapacity;
};
struct TaskCommandQueue {
    unsigned int frontOffset;
    unsigned int recordCount;
    TaskCommandPages pageRing;
};
struct TaskCommandOwner {
    char taskPrefix[kTaskCommandQueueOffset];
    TaskCommandQueue commands;
};
struct TaskNameNode {
    void* value;
    const char* name;
    char listLinks[0xC];
    TaskNameNode* bucketNext;
};

struct Poly {
    virtual void m0() = 0;
    virtual void m1() = 0;
    virtual void m2() = 0;
    virtual void m3() = 0;
    virtual void m4() = 0;
    virtual void m5() = 0;
    virtual void m6() = 0;
    virtual void m7() = 0;
    virtual void m8() = 0;
    virtual void m9() = 0;
    virtual void m10() = 0;
    virtual void m11() = 0;
    virtual void m12() = 0;
    virtual void drawLoading() = 0;
    virtual void m14() = 0;
    virtual void m15() = 0;
    virtual void m16() = 0;
    virtual void m17() = 0;
    virtual void m18() = 0;
    virtual void drawTask() = 0;
    virtual void m20() = 0;
    virtual void m21() = 0;
};

extern "C" {

// Default task hook: perform no work.
void CTask_OnCreateActorsDefault() {
}

// Return the page-pointer ring embedded in a task command deque.
void* TaskCommandDeque_GetPageRing(void* p) {
    return &((TaskCommandQueue*)p)->pageRing;
}

// Return the task's current child actor.
void* Object_GetChild(void* p) {
    return AsCTask(p)->child;
}

extern "C" char s_taskTypeInfo;
extern "C" unsigned char s_actorLoadingContext[8];
extern "C" unsigned char s_taskTransitionsDeferred;
extern "C" void* g_soundManager;
extern "C" const float kFloat_1_TaskTransition;
extern "C" const unsigned int kClearRecordPageCapacity;
extern "C" signed char s_fadeOverlayInitialized;
extern "C" void* g_soundManager;
extern "C" char s_fadeOverlay[];
extern "C" char s_fadeOverlayDtorRegistration_801FFA9C[];

extern "C" void CThread_CancelIfRunning(void*);
extern "C" void CTask_ReplaceActiveTaskByName(void*, void*, void*);
extern "C" void ActorFactoryInterface_InitVtable(void*);
extern "C" void CTask_RequestTransition(void*, void*, void*);
extern "C" void dtor_8029EA18(void*, int);
extern "C" void __dl__FPv(void*);
extern "C" char __vt__8CTaskDoc[];
extern "C" void appFreeUni__FPv(void*);
extern "C" void String_ReplaceRange(void*, int, int, const char*, const char*, unsigned char*);
extern "C" unsigned int strlen(const char*);
extern "C" void TaskCommandDeque_AppendRecord(void*, void*, void*);
extern "C" int TaskCommandDeque_GetFreeRecordCount(void*);
extern "C" unsigned int TaskCommandDeque_GetFrontOffset(void*);
extern "C" int TaskCommandDeque_GetRecordCapacity(void*);
extern "C" void* PointerRing_GetHeadSlot(void*);
extern "C" void* RingBuffer32_PopFrontIndex(void*);
extern "C" void* RingBuffer32_GetBackWord(void*);
extern "C" void RingBuffer32_AppendWordThunk(void*, void*);
extern "C" void* RingBuffer32_BuildEndWordIterator(void*, void*);
extern "C" void* RingIterator32_SubtractSignedOffset(void*, void*, int);
extern "C" void* RingBuffer32_GetAllocation(void*);
extern "C" void TaskCommandRecord_CopyConstruct(void*, void*, void*);
extern "C" unsigned int TaskCommandDeque_GetMaxSize(void*);
extern "C" unsigned int TaskCommandRecord_AllocateArray(void*, int);
extern "C" void* TaskCommandDeque_InitializeEmpty(void*, void*);
extern "C" unsigned int RingBuffer32_GetUsableCapacity(void*);
extern "C" unsigned int RingBuffer32_GetCountWord(void*);
extern "C" void* UnsignedWord_SelectMaxPointer(void*, void*);
extern "C" void RingBuffer32_ReallocateStorage(void*, void*);
extern "C" int RingBuffer_IsEmpty(void*);
extern "C" void* RingBuffer32_BuildBeginByteIterator(void*, void*);
extern "C" void RingBuffer32_InsertRangeFromWordIterators(void*, void*, void*, void*, int);
extern "C" void RingBuffer32_SwapHeaderWords(void*, void*);
extern "C" void RingBuffer32_ResetCount(void*);
extern "C" void TaskCommandDeque_BuildEndRecordIterator(void*, void*);
extern "C" void* RingStorage_GetTrailingPayload(void*);
extern "C" void RingBuffer_ThrowLengthError(const char*);
extern "C" const char taskScoreExceptionStrings[];
extern "C" unsigned char s_taskRingBufferMinimumCapacity;
extern "C" void TaskCommandDeque_ClearRecordNames(void*, unsigned char*);
extern "C" void FadeOverlay_Init(void*);
extern "C" void ResourceState_Dtor();
extern "C" void FadeState_RequestFramebufferCapture(void*);
extern "C" void FadeState_StartFadeIn(void*, int);
extern "C" int FadeState_GetState(void*);
extern "C" void FadeState_StartFadeOut(void*, int);
extern "C" void OSLockMutex(void*);
extern "C" void OSUnlockMutex(void*);
extern "C" int strcmp(const char*, const char*);
extern "C" void TaskCommandDeque_DiscardFront(void*, unsigned int, const void*);
extern "C" void __register_global_object(void*, void (*)(), void*);

struct Dev {
    virtual void v0() = 0;
    virtual void v1() = 0;
    virtual void v2() = 0;
    virtual void v3() = 0;
    virtual void v4() = 0;
    virtual void v5() = 0;
    virtual void v6() = 0;
    virtual void v7() = 0;
    virtual void v8() = 0;
    virtual void finishLoading() = 0;
    virtual void v12() = 0;
    virtual void v13() = 0;
    virtual void process(float) = 0;
    virtual void v15() = 0;
    virtual void v16() = 0;
    virtual void v17() = 0;
    virtual void v18() = 0;
    virtual int isReady() = 0;
    virtual void processWhileLoading(float) = 0;
};

struct Handler {
    virtual void h0() = 0;
    virtual void h1() = 0;
    virtual void h2() = 0;
    virtual void h3() = 0;
    virtual void h4() = 0;
    virtual void h5() = 0;
    virtual void h6() = 0;
    virtual void h7() = 0;
    virtual void h8() = 0;
    virtual void handleCommand(void*) = 0;
};

struct InstanceLock_u801FFA9C {
    void* mutex;
    // Hold the registry mutex for the lifetime of this lookup scope.
    InstanceLock_u801FFA9C(void* m) {
        mutex = m;
        OSLockMutex(m);
    }
    // Release the registry mutex on normal return and exception unwinding.
    ~InstanceLock_u801FFA9C() {
        OSUnlockMutex(mutex);
    }
};

#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Find a borrowed named handler slot under the registry's recursive mutex.
static inline void* FindSlot(void* device, unsigned char* key) {
    InstanceLock_u801FFA9C lock(((NameRegistry801C*)((char*)device + kNameRegistryOffset))->lock);
    if (key != 0 && ((NameRegistry801C*)((char*)device + kNameRegistryOffset))->buckets != 0) {
        unsigned int hash;
        {
            void* mutex2 = ((NameRegistry801C*)((char*)device + kNameRegistryOffset))->lock;
            OSLockMutex(mutex2);
            unsigned int len = strlen((const char*)key);
            unsigned int idx = (len - 1) & (unsigned int)((int)(-len & ~len) >> 31);
            unsigned int mask = (((key[0] & 0x2a) | (key[idx] & 0x15)) & 0x3f) | (len << 6);
            hash = mask & ((NameRegistry801C*)((char*)device + kNameRegistryOffset))->hashMask;
            OSUnlockMutex(mutex2);
        }
        void** bucket = (void**)((NameRegistry801C*)((char*)device + kNameRegistryOffset))->buckets;
        void* node = bucket[hash];
        while (node != 0) {
            if (strcmp(((TaskNameNode*)node)->name, (const char*)key) == 0) {
                return *(void**)node;
            }
            node = ((TaskNameNode*)node)->bucketNext;
        }
    }
    return 0;
}
// Gate actor processing through loading and fades, then deliver queued named commands before updating.
// child must exist; deltaSeconds is passed unchanged to the actor's update hook.
// Loading byte 4 latches finishLoading once and consumes that update. The fade
// gate suppresses loading updates in state 1, starts a 10-frame fade from state 2,
// and permits processWhileLoading in the remaining states.
void CTask_UpdateCurrentTaskWithFadeGates(void* task, float deltaSeconds) {
    Dev* actor = (Dev*)AsCTask(task)->child;
    if (actor->isReady() == 0) {
        // Phase 1: obtain the overlay, then decide whether loading may advance.
        // Repeated lazy access is intentional: do not cache across external calls.
        if (((unsigned int&)Sing_805DC608()) == 0) {
            if (s_fadeOverlayInitialized == 0) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_801FFA9C);
                s_fadeOverlayInitialized = 1;
            }
            ((unsigned int&)Sing_805DC608()) = (unsigned int)s_fadeOverlay;
        }
        if (FadeState_GetState((void*)((unsigned int&)Sing_805DC608())) != kFadeIncreasingAlpha) {
            if (((unsigned int&)Sing_805DC608()) == 0) {
                if (s_fadeOverlayInitialized == 0) {
                    FadeOverlay_Init(s_fadeOverlay);
                    __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_801FFA9C);
                    s_fadeOverlayInitialized = 1;
                }
                ((unsigned int&)Sing_805DC608()) = (unsigned int)s_fadeOverlay;
            }
            if (FadeState_GetState((void*)((unsigned int&)Sing_805DC608())) == kFadeTransitionReady) {
                if (((unsigned int&)Sing_805DC608()) == 0) {
                    if (s_fadeOverlayInitialized == 0) {
                        FadeOverlay_Init(s_fadeOverlay);
                        __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_801FFA9C);
                        s_fadeOverlayInitialized = 1;
                    }
                    ((unsigned int&)Sing_805DC608()) = (unsigned int)s_fadeOverlay;
                }
                FadeState_StartFadeOut((void*)((unsigned int&)Sing_805DC608()), kResumeFadeFrames);
            }
            actor = (Dev*)AsCTask(task)->child;
            actor->processWhileLoading(deltaSeconds);
        }
    } else {
        // Phase 2: publish readiness once; command dispatch starts next update.
        unsigned char* loadingState = (unsigned char*)&s_actorLoadingContext;
        if (loadingState[kLoadingFinishNotifiedByte] == 0) {
            actor = (Dev*)AsCTask(task)->child;
            actor->finishLoading();
            loadingState[kLoadingFinishNotifiedByte] = 1;
            return;
        }
        // Phase 3: map logical front to page/record, dispatch a borrowed payload,
        // then destroy the consumed key even if no handler matched.
        if (((TaskCommandOwner*)task)->commands.recordCount != 0) while (((TaskCommandOwner*)task)->commands.recordCount != 0) {
            unsigned int firstPage = ((TaskCommandOwner*)task)->commands.pageRing.firstPage;
            unsigned int frontOffset = ((TaskCommandOwner*)task)->commands.frontOffset;
            unsigned int pageCapacity = ((TaskCommandOwner*)task)->commands.pageRing.pageCapacity;
            unsigned char** pages = (unsigned char**)((TaskCommandOwner*)task)->commands.pageRing.pages;
            unsigned int pageOffset = frontOffset / kRecordsPerPage;
            unsigned int pageIndex = (firstPage + pageOffset) % pageCapacity;
            unsigned int recordIndex = frontOffset - pageOffset * kRecordsPerPage;
            unsigned char* record = pages[pageIndex] + recordIndex * kCommandRecordBytes;
            unsigned char* key;
            key = ((*(unsigned int*)record >> 31) == 0) ? record + 1 : *(unsigned char**)(record + 8);
            void* slot = FindSlot((Dev*)AsCTask(task)->child, key);
            void* handler = slot != 0 ? *(void**)slot : 0;
            if (handler != 0) {
                ((Handler*)handler)->handleCommand(record + kCommandPayloadOffset);
            }
            unsigned char zero = 0;
            TaskCommandDeque_DiscardFront(&((TaskCommandOwner*)task)->commands, 1, &zero);
        }
        // Phase 4: refresh child after handlers, which may have changed task state.
        actor = (Dev*)AsCTask(task)->child;
        actor->process(deltaSeconds);
    }
}
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset

// Release heap-backed command-name storage and optionally delete the record.
void* StringStorage_dtor(void* p, int flag) {
    if (p != 0) {
        if (p != 0 && (*(unsigned int*)p >> 31) != 0) appFreeUni__FPv(AsMWStringStorage(p)->data);
        if (flag > 0) __dl__FPv(p);
    }
    return p;
}

// Temporary request storage. Only [name, name + 0x2C) becomes a queued record.
// Payload words 0..4 and 7 are caller-supplied ABI words (pointer-shaped here,
// often integers at call sites); words 5/6 are not initialized by this routine.
// Do not assign universal meanings: title/shop/party/save handlers interpret them.
// Example: shop callers pass word1=1 and all other supplied words=0.
// The queue copies words, not pointees; only the name receives a deep copy.
struct Entry801FFFF8 {
    unsigned char requestFlag;
    char pad1[3];
    unsigned char nameAllocatorTag;
    char pad5[7];
    unsigned int name[3];
    void* payloadWord0;
    void* payloadWord1;
    void* payloadWord2;
    void* payloadWord3;
    void* payloadWord4;
    int uninitializedPayloadWords5And6[2];
    void* payloadWord7;
};
// Build a named request, append it to the task command deque, then release the temporary name.
void* TaskCommandQueue_AppendNamedRequest(void* task, void* payloadWord0, const char* handlerName, void* payloadWord1, void* payloadWord2, void* payloadWord3, void* payloadWord4, void* payloadWord7) {
    Entry801FFFF8 entry;
    entry.name[0] = 0;
    entry.name[1] = 0;
    entry.name[2] = 0;
    unsigned char inlineNameLength = ((unsigned char*)entry.name)[0] & kInlineNameLengthMask;
    entry.payloadWord1 = payloadWord1;
    entry.payloadWord0 = payloadWord0;
    entry.payloadWord2 = payloadWord2;
    entry.payloadWord3 = payloadWord3;
    entry.payloadWord4 = payloadWord4;
    entry.payloadWord7 = payloadWord7;
    String_ReplaceRange(entry.name, 0, inlineNameLength, handlerName, handlerName + strlen(handlerName), &entry.nameAllocatorTag);
    struct StringCleanup {
        unsigned int* words;
        // Destroy only the temporary request's heap-backed key after append copies it.
        ~StringCleanup() {
            if (words[0] >> 31) appFreeUni__FPv((void*)words[2]);
        }
    } cleanup = {entry.name};
    entry.requestFlag = 0;
    TaskCommandDeque_AppendRecord(&((TaskCommandOwner*)task)->commands, entry.name, &entry);
    return 0; // Status is always null; ownership of the temporary never escapes.
}

// Destroy queued command names, free each allocated page and release the page-pointer ring.
void* TaskCommandDeque_Dtor(void* p, int flag) {
    if (p != 0) {
        unsigned char clear = 0;
        TaskCommandDeque_ClearRecordNames(p, &clear);
        unsigned int count = ((TaskCommandQueue*)p)->pageRing.pageCount;
        if (count != 0) {
            unsigned int cap = ((TaskCommandQueue*)p)->pageRing.pageCapacity;
            unsigned int rem;
            if (cap != 0) {
                unsigned int sum = ((TaskCommandQueue*)p)->pageRing.firstPage + count;
                unsigned int q = sum / cap;
                rem = sum - q * cap;
            } else {
                rem = 0;
            }
            unsigned int begin = ((TaskCommandQueue*)p)->pageRing.firstPage;
            void** base = ((TaskCommandQueue*)p)->pageRing.pages;
            void** end = base + rem;
            void** wrap = base + cap;
            void** it = base + begin;
            while (end != it) {
                if (*it != 0) appFreeUni__FPv(*it);
                ++it;
                if (wrap == it) it = base;
            }
        }
        if (p != 0) {
            if (p != 0) {
                if (p != 0) {
                    if (p != 0) {
                        if (p != 0) {
                            if (p != 0) {
                                void* data = ((TaskCommandQueue*)p)->pageRing.pages;
                                if (data != 0) {
                                    ((TaskCommandQueue*)p)->pageRing.pageCount = 0;
                                    appFreeUni__FPv(data);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (flag > 0) __dl__FPv(p);
    }
    return p;
}

// Replace the active task now, or save its transition arguments when transitions are deferred.
void CTask_RequestTransition(void* task, void* taskName, void* transitionData) {
    if (s_taskTransitionsDeferred != 0) {
        if (AsCTask(task)->transitionPending != 0) return;
        unsigned char pending = 1;
        AsCTask(task)->transitionArg0 = taskName;
        float duration = kFloat_1_TaskTransition;
        AsCTask(task)->transitionArg1 = transitionData;
        AsCTask(task)->transitionPending = pending;
        AsSoundManagerState(g_soundManager)->transitionDuration = duration;
        return;
    }
    CTask_ReplaceActiveTaskByName(task, taskName, transitionData);
}

// Save transition arguments and optionally start the loading display and fade-in sequence.
void CTask_RequestTransitionWithFade(void* task, void* taskName, void* transitionData, int startFade) {
    unsigned char pending = 1;
    if (AsCTask(task)->transitionPending != 0) return;
    if (startFade != 0) {
        Poly* child = (Poly*)AsCTask(task)->child;
        child->drawLoading();
        if (((unsigned int&)Sing_805DC608()) == 0) {
            if (s_fadeOverlayInitialized == 0) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_801FFA9C);
                s_fadeOverlayInitialized = 1;
            }
            ((unsigned int&)Sing_805DC608()) = (unsigned int)s_fadeOverlay;
        }
        FadeState_RequestFramebufferCapture((void*)((unsigned int&)Sing_805DC608()));
        if (((unsigned int&)Sing_805DC608()) == 0) {
            if (s_fadeOverlayInitialized == 0) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_801FFA9C);
                s_fadeOverlayInitialized = 1;
            }
            ((unsigned int&)Sing_805DC608()) = (unsigned int)s_fadeOverlay;
        }
        FadeState_StartFadeIn((void*)((unsigned int&)Sing_805DC608()), 0);
    }
    AsCTask(task)->transitionArg0 = taskName;
    float duration = kFloat_1_TaskTransition;
    AsCTask(task)->transitionArg1 = transitionData;
    AsCTask(task)->transitionPending = pending;
    AsSoundManagerState(g_soundManager)->transitionDuration = duration;
}

// Draw the task before loading starts, or draw its loading view when no transition is pending.
void CTask_DrawCurrentTaskOrLoading(void* p) {
    if (s_actorLoadingContext[kLoadingFinishNotifiedByte] == 0) {
        Poly* q = (Poly*)AsCTask(p)->child;
        q->drawTask();
        return;
    }
    if (AsCTask(p)->transitionPending != 0) return;
    Poly* q = (Poly*)AsCTask(p)->child;
    q->drawLoading();
}

// Destroy the string-tree base and optionally free the derived task document.
void* CTaskDoc_dtor(void* p, int flag) {
    if (p != 0) {
        dtor_8029EA18(p, 0);
        if (flag > 0) __dl__FPv(p);
    }
    return p;
}

// Return the task type-information descriptor.
void* CTask_GetRTTI() {
    return &s_taskTypeInfo;
}

// Return the secondary actor-loading status byte.
unsigned char ActorLoading_GetSecondaryStatus() {
    return s_actorLoadingContext[kSecondaryLoadingStatusByte];
}

// Cancel the child actor's embedded worker thread when a child exists.
void CTask_CancelChildThread(void* p) {
    void* q = AsCTask(p)->child;
    if (q != 0) {
        CThread_CancelIfRunning((char*)q + kActorThreadOffset);
    }
}

// Apply the previously saved task transition when one is pending.
void CTask_ProcessPendingTransition(void* p) {
    if (AsCTask(p)->transitionPending != 0) {
        CTask_ReplaceActiveTaskByName(p, AsCTask(p)->transitionArg0, AsCTask(p)->transitionArg1);
    }
}

// Return the command deque's embedded page-pointer ring.
void* TaskCommandDeque_GetPageRingAlias(void* p) {
    return &((TaskCommandQueue*)p)->pageRing;
}

// Return the command deque's front offset within its page sequence.
unsigned int TaskCommandDeque_GetFrontOffset(void* p) {
    return *(unsigned int*)p;
}

// Return record capacity supplied by all pages except the reserved page.
int TaskCommandDeque_GetRecordCapacity(void* p) {
    unsigned int n = ((TaskCommandQueue*)p)->pageRing.pageCount;
    if (n == 0) return 0;
    return (n - 1) * kRecordsPerPage;
}

// Return the number of free records after the front offset and active records.
int TaskCommandDeque_GetFreeRecordCount(void* p) {
    unsigned int n = ((TaskCommandQueue*)p)->pageRing.pageCount;
    int sum = *(int*)p + *(int*)((char*)p + 4);
    int value = n == 0 ? 0 : (n - 1) * kRecordsPerPage;
    return value - sum;
}

struct ClearRecord {
    unsigned int flags;
    unsigned int pad;
    void* data;
    char rest[0x20];
};

struct ClearIterator {
    ClearRecord** current;
    ClearRecord** base;
    ClearRecord** begin;
    ClearRecord** end;
    // Build a page-pointer iterator; begin retains the logical ring origin.
    ClearIterator(unsigned int* obj) {
        ClearRecord** b = (ClearRecord**)obj[2];
        ClearRecord** e = b + obj[5];
        ClearRecord** c = b + obj[3];
        base = b;
        end = e;
        current = c;
        begin = c;
    }
};

// Advance by page slots, wrapping at most once within the pointer allocation.
inline void clearAdvance(ClearIterator& iter, int amount) {
    int span = iter.end - iter.base;
    int offset = iter.current - iter.base;
    int index = amount + offset;
    if (index < 0) index += span;
    else if (index >= span) index -= span;
    ((volatile ClearIterator*)&iter)->current = ((volatile ClearIterator*)&iter)->base + index;
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Destroy and remove a prefix of commands, retiring old pages and recentering an empty deque.
void TaskCommandDeque_DiscardFront(void* queue, unsigned int amount, const void*) {
    // Requires amount <= recordCount. Example: frontOffset=25 means record 2
    // of logical page 1 (25 / 23, 25 % 23), then wrap firstPage + 1 in the
    // page-pointer ring. Names own heap buffers; payload pointers are borrowed.
    // Phase 1: locate the first record and destroy the requested prefix page by page.
    if (amount == 0) return;
    TaskCommandQueue* commands;
    ClearRecord** pageSlot;
    ClearRecord** pageSlotsBegin;
    ClearRecord** pageSlotsEnd;
    ClearRecord* firstPage;
    unsigned int recordWithinPage;
    unsigned int firstPageRemaining;
    unsigned int recordsThisPage;
    ClearRecord* record;
    const unsigned int* countSource;
    unsigned int recordIndex;
    unsigned int nextFirstPage;
    commands = (TaskCommandQueue*)queue;
    ClearIterator iter((unsigned int*)commands);
    clearAdvance(iter, commands->frontOffset / kRecordsPerPage);
    pageSlot = iter.current;
    pageSlotsBegin = iter.base;
    pageSlotsEnd = iter.end;
    recordWithinPage = commands->frontOffset % kRecordsPerPage;
    firstPage = *pageSlot;
    firstPageRemaining = kRecordsPerPage - recordWithinPage;
    record = firstPage + recordWithinPage;
    countSource = amount < firstPageRemaining ? &amount : &firstPageRemaining;
    recordsThisPage = *countSource;
    for (;;) {
        for (recordIndex = 0; recordIndex < recordsThisPage; ++recordIndex, ++record) {
            if (record != 0) {
                if (record != 0 && (record->flags >> 31) != 0) appFreeUni__FPv(record->data);
            }
        }
        commands->recordCount -= recordsThisPage;
        commands->frontOffset += recordsThisPage;
        amount -= recordsThisPage;
        if (amount == 0) break;
        if (++pageSlot == pageSlotsEnd) pageSlot = pageSlotsBegin;
        record = *pageSlot;
        countSource = &kClearRecordPageCapacity;
        if (amount < kRecordsPerPage) countSource = &amount;
        recordsThisPage = *countSource;
    }
    // Phase 2: retire a front page only after two full pages have been passed;
    // retain one spare page before the front and translate frontOffset by 23.
    for (pageSlotsEnd = 0; commands->frontOffset >= kRetiredPageThreshold;) {
        appFreeUni__FPv((void*)((unsigned int*)commands->pageRing.pages)[commands->pageRing.firstPage]);
        nextFirstPage = commands->pageRing.firstPage + 1;
        commands->pageRing.firstPage = nextFirstPage;
        if (nextFirstPage == commands->pageRing.pageCapacity) commands->pageRing.firstPage = (unsigned int)pageSlotsEnd;
        --commands->pageRing.pageCount;
        commands->frontOffset -= kRecordsPerPage;
    }
    // Phase 3: an empty deque restarts at the midpoint (11) of its retained page.
    if (commands->recordCount == 0) commands->frontOffset = kEmptyFrontOffset;
}
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

#pragma opt_common_subs off
#pragma opt_propagation off
// Destroy all command names across wrapped pages while reducing the record count to zero.
void TaskCommandDeque_ClearRecordNames(void* p, unsigned char*) {
    TaskCommandQueue* obj = (TaskCommandQueue*)p;
    if (obj->recordCount == 0) return;
    ClearIterator iter((unsigned int*)obj);
    clearAdvance(iter, obj->frontOffset / kRecordsPerPage);
    ClearRecord** slot = iter.current;
    ClearRecord** base = iter.base;
    ClearRecord** limit = iter.end;
    ClearRecord* first = *slot;
    unsigned int rem = obj->frontOffset % kRecordsPerPage;
    ClearRecord* record = first + rem;
    unsigned int count = kRecordsPerPage - rem;
    const unsigned int* n = count < obj->recordCount ? &count : &obj->recordCount;
    ClearRecord* stop = record + *n;
    for (;;) {
        obj->recordCount -= stop - record;
        while (record < stop) {
            if (record != 0) {
                if (record != 0 && (record->flags >> 31) != 0)
                    appFreeUni__FPv(record->data);
            }
            ++record;
        }
        if (obj->recordCount == 0) break;
        if (++slot == limit) slot = base;
        record = *slot;
        n = obj->recordCount > kRecordsPerPage ? &kClearRecordPageCapacity : &obj->recordCount;
        stop = record + *n;
    }
}

#pragma opt_propagation reset
#pragma opt_common_subs reset

// Construct the document's string-tree base and install its vtable.
void* CTaskDoc_Ctor(void* p) {
    ActorFactoryInterface_InitVtable(p);
    *(void**)p = (void*)__vt__8CTaskDoc;
    return p;
}

// Append a command, reusing a front page or growing the page-pointer ring when necessary.
void TaskCommandDeque_AppendRecord(void* queue, void* sourceRecord, void*) {
    // sourceRecord is a borrowed 44-byte record; successful append owns a deep-copied key.
    // Phase 1: ensure a writable record slot, recycling a retired front page first.
    // taskScoreExceptionStrings retains a historical pool spelling. Definitions in
    // title_task_and_ring_buffer.cpp copy a 0x2C command record, allocate 23-record pages and build
    // deque iterators; no score/player/hole semantics exist in these helpers.
    // Iterator arrays are four-word ring cursors, not record payloads.
    unsigned char grownQueueStorage[0x18];
    unsigned char freeSlotIterator[0x18];
    unsigned int recycledPageIterator[4], recycledRingEnd[4], sparePageIterator[4], spareRingEnd[4], grownPageIterator[4], grownRingEnd[4];
    unsigned int insertionBegin[4], oldPagesBegin[4], oldPagesEnd[4], oldRingEnd[4];
    void* pageRing;
    unsigned int pageAddressOrCapacityLimit, allocatedPage;
    void* grownPageRing;
    void* recordAllocator;
    recordAllocator = TaskCommandDeque_GetPageRingAlias(queue);
    if (TaskCommandDeque_GetFreeRecordCount(queue) == 0) {
        pageRing = TaskCommandDeque_GetPageRing(queue);
        if (TaskCommandDeque_GetFrontOffset(queue) >= kRecordsPerPage) {
            // Here the reused word is a page ADDRESS; the other branch uses capacity.
            pageAddressOrCapacityLimit = *(unsigned int*)PointerRing_GetHeadSlot(pageRing);
            RingBuffer32_PopFrontIndex(pageRing);
            *(unsigned int*)RingBuffer32_GetBackWord(pageRing) = pageAddressOrCapacityLimit;
            unsigned int zero = 0;
            RingBuffer32_AppendWordThunk(pageRing, &zero);
            *(unsigned int*)queue -= kRecordsPerPage;
            RingBuffer32_BuildEndWordIterator(recycledRingEnd, pageRing);
            RingIterator32_SubtractSignedOffset(recycledPageIterator, recycledRingEnd, 2);
            TaskCommandRecord_CopyConstruct(recordAllocator, *(void**)RingBuffer32_GetAllocation(recycledPageIterator), sourceRecord);
        } else {
            pageAddressOrCapacityLimit = TaskCommandDeque_GetMaxSize(queue) - kRecordsPerPage;
            if (TaskCommandDeque_GetRecordCapacity(queue) > pageAddressOrCapacityLimit) RingBuffer_ThrowLengthError(taskScoreExceptionStrings + kLengthErrorTypeOffset);
            pageAddressOrCapacityLimit = RingBuffer32_GetUsableCapacity(pageRing);
            if (RingBuffer32_GetCountWord(pageRing) < pageAddressOrCapacityLimit) {
                allocatedPage = TaskCommandRecord_AllocateArray(recordAllocator, kRecordsPerPage);
                *(unsigned int*)RingBuffer32_GetBackWord(pageRing) = allocatedPage;
                unsigned int zero = 0;
                RingBuffer32_AppendWordThunk(pageRing, &zero);
                RingBuffer32_BuildEndWordIterator(spareRingEnd, pageRing);
                RingIterator32_SubtractSignedOffset(sparePageIterator, spareRingEnd, 2);
                TaskCommandRecord_CopyConstruct(recordAllocator, *(void**)RingBuffer32_GetAllocation(sparePageIterator), sourceRecord);
            } else {
                // Phase 2: stage a larger page ring; transfer existing page pointers
                // without copying their records, then swap and destroy old storage.
                TaskCommandDeque_InitializeEmpty(grownQueueStorage, recordAllocator);
                grownPageRing = TaskCommandDeque_GetPageRing(grownQueueStorage);
                unsigned int size = RingBuffer32_GetUsableCapacity(pageRing) * 2;
                unsigned int requestedCapacity[1] = {size};
                RingBuffer32_ReallocateStorage(grownPageRing, *(void**)UnsignedWord_SelectMaxPointer(requestedCapacity, &s_taskRingBufferMinimumCapacity));
                unsigned int zero = 0;
                RingBuffer32_AppendWordThunk(grownPageRing, &zero);
                allocatedPage = TaskCommandRecord_AllocateArray(recordAllocator, kRecordsPerPage);
                *(unsigned int*)RingBuffer32_GetBackWord(grownPageRing) = allocatedPage;
                unsigned int zero2 = 0;
                RingBuffer32_AppendWordThunk(grownPageRing, &zero2);
                RingBuffer32_BuildEndWordIterator(grownRingEnd, grownPageRing);
                RingIterator32_SubtractSignedOffset(grownPageIterator, grownRingEnd, 2);
                TaskCommandRecord_CopyConstruct(recordAllocator, *(void**)RingBuffer32_GetAllocation(grownPageIterator), sourceRecord);
                if (RingBuffer_IsEmpty(pageRing) == 0) {
                    RingBuffer32_BuildEndWordIterator(oldRingEnd, pageRing);
                    RingIterator32_SubtractSignedOffset(oldPagesEnd, oldRingEnd, 1);
                    RingBuffer32_BuildBeginByteIterator(oldPagesBegin, pageRing);
                    RingBuffer32_BuildBeginByteIterator(insertionBegin, grownPageRing);
                    RingBuffer32_InsertRangeFromWordIterators(grownPageRing, insertionBegin, oldPagesBegin, oldPagesEnd, 0);
                }
                RingBuffer32_SwapHeaderWords(pageRing, grownPageRing);
                RingBuffer32_ResetCount(grownPageRing);
                TaskCommandDeque_Dtor(grownQueueStorage, -1);
            }
        }
    } else {
        TaskCommandDeque_BuildEndRecordIterator(freeSlotIterator, queue);
        TaskCommandRecord_CopyConstruct(recordAllocator, RingStorage_GetTrailingPayload(freeSlotIterator), sourceRecord);
    }
    // Publish the new record only after its key and payload have been constructed.
    ++((TaskCommandQueue*)queue)->recordCount;
}

} // extern "C"
