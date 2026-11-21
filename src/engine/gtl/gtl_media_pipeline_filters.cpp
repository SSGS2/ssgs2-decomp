#include "classes/GtlMediaPipeline.h"
#define AsGtlMediaPipeline(p) ((GtlMediaPipeline*)(p))
// Translation unit 0x803F364C-0x803F3F84 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Local word views are layout adapters shared by several C ABI entry points.
// Semantic typedefs below distinguish filter type/state from show-counter fields;
// identical offsets do not imply the same owning class or enum domain.
// All getters/setters borrow their object; returned data pointers stay owner-held.
// Optional-deleting destructors free object storage only when flag > 0.
struct TargetObject {
    char padding00[0xC];
    unsigned int value;
};

struct TargetObjectWith08 {
    char padding00[0x8];
    unsigned int value;
};

struct TargetObjectWith0C {
    char padding00[0xC];
    unsigned int value;
};

struct TargetObjectWith10 {
    char padding00[0x10];
    unsigned int value;
};

struct TargetObjectWith68 {
    char padding00[0x68];
    float value;
};

struct TargetObjectWith08Out {
    char padding00[0x8];
    unsigned int value;
};

typedef TargetObjectWith08 FilterTypeView;
typedef TargetObjectWith08 ShowCounterActivityView;
typedef TargetObjectWith0C FilterStateView;
typedef TargetObjectWith0C ShowCounterCountView;
typedef TargetObject FilterStateWriteView;
typedef TargetObjectWith10 ShowCounterPreviousCountView;
typedef TargetObjectWith68 ManagerPlaybackValueView;

// Manager states differ from filter states: only ready/running/reset/suspended
// allow format reads. Transition requests reject closed/loading and the requested
// target state; other values, including error states, still attempt the command.
// Start/suspend/reset are sent to all five filters before notifying the counter.
enum ManagerState { kManagerClosed=0, kManagerLoading=1, kManagerReady=2,
    kManagerRunning=3, kManagerReset=4, kManagerSuspended=5 };
enum ManagerNotification { kOpenFailed=6, kOpenWhileActive=11 };
enum MediaVirtualSlot { kDestroySlot=2, kCounterStartSlot=3, kCounterSuspendSlot=4,
    kCounterResetSlot=5, kReportStateSlot=0x5C/4, kAfterOpenSlot=0x6C/4 };
struct ShowCounter {
    char padding00[0x8];
    unsigned int active;
    unsigned int count;
    unsigned int previous;
};

struct CounterState {
    char padding00[0xC];
    unsigned int count;
    unsigned int previous;
};

struct Filter {
    unsigned int referenceCount;
    void* vtable;
    unsigned int type;
    unsigned int state;
};
struct SharedMediaSample { int references; void** vtable; };
struct MediaDispatchPrefix { int references; };
struct MediaDispatch : MediaDispatchPrefix {
    virtual void destroy(int);
    virtual void start();
    virtual void suspend();
    virtual void reset();
};
struct ManagerDispatch {
    virtual void slot00(); virtual void slot01(); virtual void slot02();
    virtual void slot03(); virtual void slot04(); virtual void slot05();
    virtual void slot06(); virtual void slot07(); virtual void slot08();
    virtual void slot09(); virtual void slot10(); virtual void slot11();
    virtual void slot12(); virtual void slot13(); virtual void slot14();
    virtual void slot15(); virtual void slot16(); virtual void slot17();
    virtual void slot18(); virtual void slot19(); virtual void slot20();
    virtual int reportState(int);
};
struct FilterStagesView { char prefix[0x50]; void* stages[5]; };

struct CounterResult {
    void* buffer;
    unsigned int bufferSize;
    void* fileInfo;
    unsigned char submitted;
    unsigned char available;
};

extern char __vt__Q23gtl7IFilter[];
extern char __vt__Q23gtl14CFilterManager[];
void gtl_Free(void* object);
extern void* gtl_Alloc(unsigned int size);
extern int gtl_CFilterManager_TransitionFilters(void* object, int state);
extern int DVDClose(void* object);
extern int DVDOpen(void* a, void* b);
extern void gtl_CMediaSample_Ctor(void* object, int a, int b, int c);
extern void gtl_CFilterManager_ReleaseResources(void* object);
extern void SamplePool_RecycleAllSamples(void* object);
extern int DVDReadAsyncPrio(void* fileInfo, void* address, unsigned int length, unsigned int offset, void* callback, int priority);
extern int DVDGetCommandBlockStatus(void* object);
extern int strlen(const char* s);
extern void* strcpy(char* dst, const char* src);
extern int gtl_IFilter_GetType(FilterTypeView* object, unsigned int* value);

struct CounterHolder {
    void* value;
    unsigned int bufferSize;
    void* fileInfo;
    unsigned char state;
    unsigned char available;
};

struct FilterManager {
    void* vtable;
    unsigned int format;
    char padding08[0x3C];
    void* outputSample0;
    void* outputSample1;
    void* showCounter;
    void* stage0;
    void* stage1;
    void* stage2;
    void* stage3;
    void* stage4;
    void* state;
    float playbackValue;
    void* reserved6C;
    void* sampleSize;
    void* sampleCount;
    void* samplePool;
    void* poolSample;
    void* loader;
    void* pendingName;
    unsigned char startPending;
    char padding89[3];
    void* outputBuffer;
};

// Constructor input is two numeric configuration words (sample size/count),
// stored in pointer-shaped local view fields solely to preserve existing layout.
// They are values, not allocated pointers; never dereference/free these two slots.
struct Pair { unsigned int first; unsigned int second; };

struct CallbackObject {
    char padding00[4];
    void** vtable;
};

struct CallbackBase {
    void** vtable;
};

struct CallbackManager {
    void** vtable;
    char padding04[0x48];
    CallbackObject* callback;
    char padding50[0x14];
    int state;
};

struct FilterSlot {
    char padding00[0x50];
    void* filter;
};

extern float kMediaPipelineInitialPlaybackValue;

// Free the DVD read buffer and optionally its owning read context.
void* gtl_MediaDvdRead_Dtor(void** object, int flag);

// Transition filters to state 3 and notify the show counter before reporting the state.
int gtl_CFilterManager_StartRunning(CallbackManager* object);

// Transition filters to state 5 and notify the show counter before reporting the state.
int gtl_CFilterManager_Suspend(CallbackManager* object);

// Initialize a DVD read context and allocate its requested buffer.
void* gtl_MediaDvdRead_Ctor(CounterHolder* object, unsigned int bufferSize, void* fileInfo);

// Close the DVD file, release pipeline resources, and report state 0.
int gtl_CFilterManager_CloseFile(void* object);

#pragma opt_propagation off
// Initialize pipeline resources and the configured sample size and count.
void gtl_CFilterManager_Ctor(FilterManager* object, const struct Pair* input);

// Return the read buffer only after the asynchronous read is available.
int gtl_MediaDvdRead_GetData(CounterResult* object, unsigned int* value);

// Optionally free the show-counter interface storage.
void* gtl_IShowCounter_dtor(void* object, int flag);

// Optionally free this media interface storage.
void* gtl_IFilter_dtor_Base(void* object, int flag);

// Clear the current and previous show counts.
int gtl_IShowCounter_ResetCounts(CounterState* object);

#pragma opt_propagation reset
// Submit a DVD read once, then poll for completion on later updates.
// Returns 0 on submission failure, 1 after submission even while still pending.
// available becomes sticky when command status is zero; no completion callback.
int gtl_MediaDvdRead_Update(void* object);

// Transition to state 4, recycle samples, and reset the show counter.
int gtl_CFilterManager_ResetPipeline(CallbackManager* object);

// Copy the active media format only while the pipeline has a usable format.
// output must have at least 0xA4 bytes and adequate word alignment. Returns 1
// only after a full copy; null output or unusable state returns 0 without writes.
// Layout matches GtlMediaFormat: +0 discriminator, +4 source bytes, +8..0x87
// name[128], +0x88 show-counter format parameter (units unconfirmed), +0x8C read
// offset in bytes, +0x90 sample buffer bytes, +0x94 opaque word, +0x98/+0x9A
// opaque halfwords, +0x9C workspace count, +0xA0 packet-header word count.
// The byte-pair loop copies only name[128]; it does not perform string conversion.
int gtl_CFilterManager_CopyFormat(void* object, void* output);

// Validate the filter type and optionally destroy the previous filter in that stage.
// index must be 0..4 and filter must be non-null. Type mismatch returns 0.
// replace destroys the old stage; otherwise overwriting does not release it.
// No reference increment occurs: caller transfers a handle suitable for stage teardown.
int gtl_CFilterManager_SetFilter(void* object, int index, void* filter, int replace);

// Release both reference-counted output samples and the pending file name.
void* gtl_CFilterManager_dtor(void* object, int flag);

// Initialize a filter with its type value and initial state.
void gtl_IFilter_Ctor(Filter* object, unsigned int filterType);

// Advance the show count only while the counter is inactive.
int gtl_IShowCounter_AdvanceCount(ShowCounter* object);

// Return the show counter activity word (distinct from manager/filter states).
int gtl_IShowCounter_GetState(ShowCounterActivityView* object, unsigned int* value);

// Replace the previous show-count snapshot used by pipeline progress checks.
int gtl_IShowCounter_SetPreviousCount(ShowCounterPreviousCountView* object, unsigned int value);

// Store the pipeline playback value.
int gtl_CFilterManager_SetShowCounterParameter(ManagerPlaybackValueView* object, float value);

// Return whether the supplied status is nonzero.
int gtl_CFilterManager_vfunc_12(void* object, int value);

// Return the filter stage/type discriminator through a required output pointer.
int gtl_IFilter_GetType(FilterTypeView* object, unsigned int* value);

// Return the filter result state through a required output pointer.
int gtl_IFilter_GetState(FilterStateView* object, unsigned int* value);

// Store a filter result state without initiating a manager transition.
void gtl_IFilter_SetState(FilterStateWriteView* object, unsigned int value);

// Default successful pipeline hook.
int gtl_CFilterManager_vfunc_13();

// Default successful pipeline hook.
int gtl_CFilterManager_vfunc_14();

// Default successful pipeline hook.
int gtl_CFilterManager_vfunc_15();

// Store the activity word controlling whether the show count advances.
int gtl_IShowCounter_SetState(ShowCounterActivityView* object, unsigned int value);

// Replace the current show count without changing its previous snapshot.
int gtl_IShowCounter_SetCount(ShowCounterCountView* object, unsigned int value);

// Copy and open the DVD path, allocate output samples, then report loading state.
// Requires closed state and a borrowed NUL-terminated path; copies the name.
// flag is retained as a byte for later startup handling. Return is the virtual
// state-notification result, not proof that asynchronous loading has completed.
int gtl_CFilterManager_OpenFile(FilterManager* object, const char* path, int flag);

} // extern "C"

// Game entry points in original address order.
extern "C" {

int gtl_IShowCounter_AdvanceCount(ShowCounter* object) {
    if (object->active != 0)
        return 1;
    object->previous = object->count;
    object->count++;
    return 1;
}

int gtl_IShowCounter_GetState(ShowCounterActivityView* object, unsigned int* value) {
    *value = object->value;
    return 1;
}

int gtl_IShowCounter_SetState(ShowCounterActivityView* object, unsigned int value) {
    object->value = value;
    return 1;
}

int gtl_IShowCounter_SetCount(ShowCounterCountView* object, unsigned int value) {
    object->value = value;
    return 1;
}

int gtl_IShowCounter_SetPreviousCount(ShowCounterPreviousCountView* object, unsigned int value) {
    object->value = value;
    return 1;
}

#pragma push
#pragma opt_propagation off
int gtl_IShowCounter_ResetCounts(CounterState* object) {
    object->count = 0;
    object->previous = 0;
    return 1;
}
#pragma pop

#pragma push
#pragma opt_propagation off
void* gtl_IShowCounter_dtor(void* object, int flag) {
    if (object != 0 && flag > 0 && object != 0)
        gtl_Free(object);
    return object;
}
#pragma pop

void* gtl_MediaDvdRead_Ctor(CounterHolder* object, unsigned int bufferSize, void* fileInfo) {
    object->bufferSize = bufferSize;
    object->fileInfo = fileInfo;
    object->state = 0;
    object->available = 0;
    object->value = gtl_Alloc(bufferSize);
    return object;
}

void* gtl_MediaDvdRead_Dtor(void** object, int flag) {
    if (object != 0) {
        if (*object != 0) {
            gtl_Free(*object);
            *object = 0;
        }
        if (flag > 0 && object != 0)
            gtl_Free(object);
    }
    return object;
}

int gtl_MediaDvdRead_Update(void* object) {
    CounterResult* read = (CounterResult*)object;
    if (read->submitted == 0) {
        if (!DVDReadAsyncPrio(read->fileInfo, read->buffer, read->bufferSize, 0, 0, 2))
            return 0;
        read->submitted = 1;
    }
    int flag = 0;
    if (DVDGetCommandBlockStatus(read->fileInfo) == 0)
        flag = 1;
    if (flag != 0)
        read->available = 1;
    return 1;
}

#pragma push
#pragma opt_propagation off
int gtl_MediaDvdRead_GetData(CounterResult* object, unsigned int* value) {
    if (object->available == 0) {
        *value = 0;
        return 0;
    }
    *value = (unsigned int)object->buffer;
    return 1;
}
#pragma pop

void gtl_IFilter_Ctor(Filter* object, unsigned int filterType) {
    object->referenceCount = 1;
    object->vtable = &__vt__Q23gtl7IFilter;
    object->type = filterType;
    object->state = 0;
}

#pragma push
#pragma opt_propagation off
void* gtl_IFilter_dtor_Base(void* object, int flag) {
    if (object != 0 && flag > 0 && object != 0)
        gtl_Free(object);
    return object;
}
#pragma pop

#pragma push
#pragma dont_inline on
int gtl_IFilter_GetType(FilterTypeView* object, unsigned int* value) {
    *value = object->value;
    return 1;
}
#pragma pop

int gtl_IFilter_GetState(FilterStateView* object, unsigned int* value) {
    *value = object->value;
    return 1;
}

void gtl_IFilter_SetState(FilterStateWriteView* object, unsigned int value) {
    object->value = value;
}

#pragma push
#pragma opt_propagation off
void gtl_CFilterManager_Ctor(FilterManager* object, const struct Pair* input) {
    float f0 = kMediaPipelineInitialPlaybackValue;
    object->vtable = __vt__Q23gtl14CFilterManager;
    object->format = 0;
    object->outputSample0 = 0;
    object->outputSample1 = 0;
    object->showCounter = 0;
    object->playbackValue = f0;
    object->sampleSize = (void*)input->first;
    object->sampleCount = (void*)input->second;
    object->samplePool = 0;
    object->poolSample = 0;
    object->loader = 0;
    object->pendingName = 0;
    object->startPending = 0;
    object->outputBuffer = 0;
    object->state = 0;
    object->stage0 = 0;
    object->stage1 = 0;
    object->stage2 = 0;
    object->stage3 = 0;
    object->stage4 = 0;
}
#pragma pop

void* gtl_CFilterManager_dtor(void* object, int flag) {
    unsigned char* p = (unsigned char*)object;
    if (object != 0) {
      void* first = AsGtlMediaPipeline(p)->output0;
      *(void**)object = __vt__Q23gtl14CFilterManager;
      if (first != 0) {
        int count = --((SharedMediaSample*)first)->references;
        if (count == 0 && first != 0) {
            ((MediaDispatch*)first)->destroy(1);
        }
        AsGtlMediaPipeline(p)->output0 = 0;
      }
      void* second = AsGtlMediaPipeline(p)->output1;
      if (second != 0) {
        int count = --((SharedMediaSample*)second)->references;
        if (count == 0 && second != 0) {
            ((MediaDispatch*)second)->destroy(1);
        }
        AsGtlMediaPipeline(p)->output1 = 0;
      }
      void* item = AsGtlMediaPipeline(p)->pendingName;
      if (item != 0) {
        gtl_Free(item);
        AsGtlMediaPipeline(p)->pendingName = 0;
      }
      if (flag > 0 && object != 0)
          gtl_Free(object);
    }
    return object;
}

int gtl_CFilterManager_OpenFile(FilterManager* object, const char* path, int flag) {
    if (object->state != 0) {
        void** vt = (void**)object->vtable;
        ((int (*)(FilterManager*, int))vt[kReportStateSlot])(object, kOpenWhileActive);
        return 0;
    }
    // Phase 1: replace owned path storage; DVDOpen failure leaves that copy owned.
    if (object->pendingName != 0) {
        gtl_Free(object->pendingName);
        object->pendingName = 0;
    }
    object->pendingName = gtl_Alloc((unsigned int)(strlen(path) + 1));
    strcpy((char*)object->pendingName, path);
    if (!DVDOpen((void*)path, AsGtlMediaPipeline(object)->inputFile)) {
        void** vt = (void**)object->vtable;
        ((int (*)(FilterManager*, int))vt[kReportStateSlot])(object, kOpenFailed);
        return 0;
    }
    // Create one owned output-sample reference (null allocation is retained).
    {
        void* p = gtl_Alloc((unsigned int)0x14);
        if (p != 0) {
            gtl_CMediaSample_Ctor(p, 0, 0, 0);
        }
        object->outputSample0 = p;
    }
    // Create one owned output-sample reference (null allocation is retained).
    {
        void* p = gtl_Alloc((unsigned int)0x14);
        if (p != 0) {
            gtl_CMediaSample_Ctor(p, 0, 0, 0);
        }
        object->outputSample1 = p;
    }
    {
        void** vt = (void**)object->vtable;
        ((void (*)(FilterManager*))vt[kAfterOpenSlot])(object);
    }
    // Phase 3: after-open virtual hook has run; retain startup mode and report loading.
    object->startPending = (unsigned char)flag;
    {
        void** vt = (void**)object->vtable;
        return ((int (*)(FilterManager*, int))vt[kReportStateSlot])(object, kManagerLoading);
    }
}

int gtl_CFilterManager_CloseFile(void* object) {
    if (!DVDClose(AsGtlMediaPipeline(object)->inputFile))
        return 0;
    gtl_CFilterManager_ReleaseResources(object);
    return ((ManagerDispatch*)object)->reportState(0);
}

int gtl_CFilterManager_SetFilter(void* object, int index, void* filter, int replace) {
    unsigned int storedIndex;
    gtl_IFilter_GetType((FilterTypeView*)filter, &storedIndex);
    if ((int)storedIndex != index)
        return 0;
    if (replace) {
        void** slot = &((FilterStagesView*)object)->stages[index];
        if (*slot != 0) {
            if (*slot != 0)
                ((MediaDispatch*)*slot)->destroy(1);
            *slot = 0;
        }
    }
    ((FilterStagesView*)object)->stages[index] = filter;
    return 1;
}

int gtl_CFilterManager_StartRunning(CallbackManager* object) {
    int state = object->state;
    if (state == kManagerLoading || state == kManagerClosed || state == kManagerRunning)
        return 0;
    if (!gtl_CFilterManager_TransitionFilters(object, kManagerRunning))
        return 0;
    {
        CallbackObject* callback = object->callback;
        ((MediaDispatch*)callback)->start();
    }
    return ((ManagerDispatch*)object)->reportState(kManagerRunning);
}

int gtl_CFilterManager_ResetPipeline(CallbackManager* object) {
    int state = AsGtlMediaPipeline(object)->state;
    if (state == kManagerLoading || state == kManagerClosed || state == kManagerReset)
        return 0;
    if (!gtl_CFilterManager_TransitionFilters(object, kManagerReset))
        return 0;
    SamplePool_RecycleAllSamples(AsGtlMediaPipeline(object)->samplePool);
    CallbackObject* callback = (*(CallbackObject**)&AsGtlMediaPipeline(object)->showCounter);
    ((MediaDispatch*)callback)->reset();
    return ((ManagerDispatch*)object)->reportState(kManagerReset);
}

int gtl_CFilterManager_Suspend(CallbackManager* object) {
    int state = object->state;
    if (state == kManagerLoading || state == kManagerClosed || state == kManagerSuspended)
        return 0;
    if (!gtl_CFilterManager_TransitionFilters(object, kManagerSuspended))
        return 0;
    {
        CallbackObject* callback = object->callback;
        ((MediaDispatch*)callback)->suspend();
    }
    return ((ManagerDispatch*)object)->reportState(kManagerSuspended);
}

int gtl_CFilterManager_SetShowCounterParameter(ManagerPlaybackValueView* object, float value) {
    object->value = value;
    return 1;
}

int gtl_CFilterManager_vfunc_12(void* object, int value) {
    return value != 0;
}

int gtl_CFilterManager_vfunc_13() {
    return 1;
}

int gtl_CFilterManager_vfunc_14() {
    return 1;
}

int gtl_CFilterManager_vfunc_15() {
    return 1;
}

int gtl_CFilterManager_CopyFormat(void* object, void* output) {
    const unsigned char* src;
    unsigned char* dst;
    int count;
    if (output != 0) {
        int state = AsGtlMediaPipeline(object)->state;
        if (state != kManagerReady && state != kManagerRunning && state != kManagerReset && state != kManagerSuspended)
            return 0;
        dst = (unsigned char*)output;
        src = (const unsigned char*)AsGtlMediaPipeline(object)->format;
        *(unsigned int*)dst = *(unsigned int*)src;
        *(unsigned int*)(dst + 4) = *(unsigned int*)(src + 4);
        const unsigned char* srcCursor = src + 7;
        unsigned char* dstCursor = dst + 7;
        count = 0x40;
        do {
            unsigned char first = srcCursor[1];
            unsigned char second = *(srcCursor += 2);
            dstCursor[1] = first;
            *(dstCursor += 2) = second;
        } while (--count != 0);
        *(float*)(dst + 0x88) = *(float*)(src + 0x88);
        *(unsigned int*)(dst + 0x8C) = *(unsigned int*)(src + 0x8C);
        *(unsigned int*)(dst + 0x90) = *(unsigned int*)(src + 0x90);
        *(unsigned int*)(dst + 0x94) = *(unsigned int*)(src + 0x94);
        *(unsigned short*)(dst + 0x98) = *(unsigned short*)(src + 0x98);
        *(unsigned short*)(dst + 0x9A) = *(unsigned short*)(src + 0x9A);
        *(unsigned int*)(dst + 0x9C) = *(unsigned int*)(src + 0x9C);
        *(unsigned int*)(dst + 0xA0) = *(unsigned int*)(src + 0xA0);
        return 1;
    }
    return 0;
}
} // extern "C"
