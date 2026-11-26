// Media pipeline manager lifecycle: construction, file open/close, filter stage setup,
// start/suspend/reset transitions and format copy. Also the show counter and filter helpers.
#include "classes/GtlMediaSample.h"
#include "classes/GtlMediaPipeline.h"
#include "classes/GtlShowCounter.h"
#include "classes/gtl_IFilter.h"

// Local word views are layout adapters shared by several C ABI entry points.
// All getters/setters borrow their object; returned data pointers stay owner-held.
// Optional-deleting destructors free object storage only when flag > 0.

extern "C" {

// Manager states differ from filter states: only ready/running/reset/suspended
// allow format reads. Transition requests reject closed/loading and the requested
// target state; other values, including error states, still attempt the command.
// Start/suspend/reset are sent to all five filters before notifying the counter.
enum ManagerState {
    kManagerClosed = 0,
    kManagerLoading = 1,
    kManagerReady = 2,
    kManagerRunning = 3,
    kManagerReset = 4,
    kManagerSuspended = 5
};
enum ManagerNotification {
    kOpenFailed = 6,
    kOpenWhileActive = 11
};
enum MediaVirtualSlot {
    kDestroySlot = 2,
    kCounterStartSlot = 3,
    kCounterSuspendSlot = 4,
    kCounterResetSlot = 5,
    kReportStateSlot = 0x5C / 4,
    kAfterOpenSlot = 0x6C / 4
};
struct SharedMediaSample {
    int references;
    void** vtable;
};
struct MediaDispatchPrefix {
    int references;
};
struct MediaDispatch : MediaDispatchPrefix {
    virtual void destroy(int);
    virtual void start();
    virtual void suspend();
    virtual void reset();
};
struct ManagerDispatch {
    virtual void slot00();
    virtual void slot01();
    virtual void slot02();
    virtual void slot03();
    virtual void slot04();
    virtual void slot05();
    virtual void slot06();
    virtual void slot07();
    virtual void slot08();
    virtual void slot09();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual int reportState(int);
};
struct FilterStagesView {
    char prefix[0x50];
    void* stages[5];
};

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
extern void SamplePool_RecycleAllSamples(void* object);
extern int DVDClose(void* object);
extern int DVDOpen(void* a, void* b);
extern int DVDReadAsyncPrio(void* fileInfo, void* address, unsigned int length, unsigned int offset, void* callback,
                            int priority);
extern int DVDGetCommandBlockStatus(void* object);
extern int strlen(const char* s);
extern void* strcpy(char* dst, const char* src);

struct CounterHolder {
    void* value;
    unsigned int bufferSize;
    void* fileInfo;
    unsigned char state;
    unsigned char available;
};

extern float kMediaPipelineInitialPlaybackValue;

// Free the DVD read buffer and optionally its owning read context.
void* gtl_MediaDvdRead_Dtor(void** object, int flag);

// Initialize a DVD read context and allocate its requested buffer.
void* gtl_MediaDvdRead_Ctor(CounterHolder* object, unsigned int bufferSize, void* fileInfo);

// Submit a DVD read once, then poll for completion on later updates.
// Returns 0 on submission failure, 1 after submission even while still pending.
// available becomes sticky when command status is zero; no completion callback.
int gtl_MediaDvdRead_Update(void* object);

// Return the read buffer only after the asynchronous read is available.
int gtl_MediaDvdRead_GetData(CounterResult* object, unsigned int* value);

} // extern "C"

// Show counter member definitions. The counter is borrowed from its owner.
int gtl::IShowCounter::AdvanceCount() {
    if (state != 0)
        return 1;
    previousCount = count;
    count++;
    return 1;
}

int gtl::IShowCounter::GetState(unsigned int* value) {
    *value = state;
    return 1;
}

int gtl::IShowCounter::SetState(unsigned int value) {
    state = value;
    return 1;
}

int gtl::IShowCounter::SetCount(unsigned int value) {
    count = value;
    return 1;
}

int gtl::IShowCounter::SetPreviousCount(unsigned int value) {
    previousCount = value;
    return 1;
}

#pragma push
#pragma opt_propagation off
int gtl::IShowCounter::ResetCounts() {
    count = 0;
    previousCount = 0;
    return 1;
}
#pragma pop

// The original tests the pointer twice and is compiled without propagation; both match it.
#pragma push
#pragma opt_propagation off
void* gtl::IShowCounter::Destroy(int flag) {
    if (this != 0 && flag > 0 && this != 0)
        gtl_Free(this);
    return this;
}
#pragma pop

extern "C" {

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

} // extern "C"

// IFilter members, in original address order.
void gtl::IFilter::Init(unsigned int filterType) {
    referenceCount = 1;
    vtable = &__vt__Q23gtl7IFilter;
    type = filterType;
    state = 0;
}

#pragma push
#pragma opt_propagation off
void* gtl::IFilter::Destroy(int deleting) {
    if (this != 0 && deleting > 0 && this != 0)
        gtl_Free(this);
    return this;
}
#pragma pop

#pragma push
#pragma dont_inline on
int gtl::IFilter::GetType(unsigned int* value) {
    *value = type;
    return 1;
}
#pragma pop

int gtl::IFilter::GetState(int* value) {
    *value = state;
    return 1;
}

void gtl::IFilter::SetState(unsigned int value) {
    state = value;
}

// CFilterManager members, in original address order.

#pragma push
#pragma opt_propagation off
void gtl::CFilterManager::Init(const CFilterManagerConfig* config) {
    float f0 = kMediaPipelineInitialPlaybackValue;
    vtbl = __vt__Q23gtl14CFilterManager;
    format = 0;
    output0 = 0;
    output1 = 0;
    showCounter = 0;
    showCounterParameter = f0;
    sampleSize = config->sampleSize;
    sampleCount = config->sampleCount;
    samplePool = 0;
    poolSample = 0;
    loader = 0;
    pendingName = 0;
    startPending = 0;
    outputBuffer = 0;
    state = 0;
    stage0 = 0;
    stage1 = 0;
    stage2 = 0;
    stage3 = 0;
    stage4 = 0;
}
#pragma pop

void* gtl::CFilterManager::Destroy(int flag) {
    if (this != 0) {
        void* first = output0;
        *(void**)this = __vt__Q23gtl14CFilterManager;
        if (first != 0) {
            int count = --((SharedMediaSample*)first)->references;
            if (count == 0 && first != 0) {
                ((MediaDispatch*)first)->destroy(1);
            }
            output0 = 0;
        }
        void* second = output1;
        if (second != 0) {
            int count = --((SharedMediaSample*)second)->references;
            if (count == 0 && second != 0) {
                ((MediaDispatch*)second)->destroy(1);
            }
            output1 = 0;
        }
        void* item = pendingName;
        if (item != 0) {
            gtl_Free(item);
            pendingName = 0;
        }
        if (flag > 0 && this != 0)
            gtl_Free(this);
    }
    return this;
}

int gtl::CFilterManager::OpenFile(const char* path, int flag) {
    if (state != 0) {
        void** vt = (void**)vtbl;
        ((int (*)(CFilterManager*, int))vt[kReportStateSlot])(this, kOpenWhileActive);
        return 0;
    }
    // Phase 1: replace owned path storage; DVDOpen failure leaves that copy owned.
    if (pendingName != 0) {
        gtl_Free(pendingName);
        pendingName = 0;
    }
    pendingName = (char*)gtl_Alloc((unsigned int)(strlen(path) + 1));
    strcpy(pendingName, path);
    if (!DVDOpen((void*)path, inputFile)) {
        void** vt = (void**)vtbl;
        ((int (*)(CFilterManager*, int))vt[kReportStateSlot])(this, kOpenFailed);
        return 0;
    }
    // Create one owned output-sample reference (null allocation is retained).
    {
        void* p = gtl_Alloc((unsigned int)0x14);
        if (p != 0) {
            ((gtl::CMediaSample*)p)->Init(0, 0, 0);
        }
        output0 = p;
    }
    // Create one owned output-sample reference (null allocation is retained).
    {
        void* p = gtl_Alloc((unsigned int)0x14);
        if (p != 0) {
            ((gtl::CMediaSample*)p)->Init(0, 0, 0);
        }
        output1 = p;
    }
    {
        void** vt = (void**)vtbl;
        ((void (*)(CFilterManager*))vt[kAfterOpenSlot])(this);
    }
    // Phase 3: after-open virtual hook has run; retain startup mode and report loading.
    startPending = (unsigned char)flag;
    {
        void** vt = (void**)vtbl;
        return ((int (*)(CFilterManager*, int))vt[kReportStateSlot])(this, kManagerLoading);
    }
}

int gtl::CFilterManager::CloseFile() {
    if (!DVDClose(inputFile))
        return 0;
    ReleaseResources();
    return ((ManagerDispatch*)this)->reportState(0);
}

int gtl::CFilterManager::SetFilter(int index, void* filter, int replace) {
    unsigned int storedIndex;
    ((gtl::IFilter*)filter)->GetType(&storedIndex);
    if ((int)storedIndex != index)
        return 0;
    if (replace) {
        void** slot = &((FilterStagesView*)this)->stages[index];
        if (*slot != 0) {
            if (*slot != 0)
                ((MediaDispatch*)*slot)->destroy(1);
            *slot = 0;
        }
    }
    ((FilterStagesView*)this)->stages[index] = filter;
    return 1;
}

// Transition filters to state 3 and notify the show counter before reporting the state.
int gtl::CFilterManager::StartRunning() {
    int managerState = state;
    if (managerState == kManagerLoading || managerState == kManagerClosed || managerState == kManagerRunning)
        return 0;
    if (!TransitionFilters(kManagerRunning))
        return 0;
    ((MediaDispatch*)showCounter)->start();
    return ((ManagerDispatch*)this)->reportState(kManagerRunning);
}

// Transition to state 4, recycle samples, and reset the show counter.
int gtl::CFilterManager::ResetPipeline() {
    int managerState = state;
    if (managerState == kManagerLoading || managerState == kManagerClosed || managerState == kManagerReset)
        return 0;
    if (!TransitionFilters(kManagerReset))
        return 0;
    SamplePool_RecycleAllSamples(samplePool);
    ((MediaDispatch*)showCounter)->reset();
    return ((ManagerDispatch*)this)->reportState(kManagerReset);
}

// Transition filters to state 5 and notify the show counter before reporting the state.
int gtl::CFilterManager::Suspend() {
    int managerState = state;
    if (managerState == kManagerLoading || managerState == kManagerClosed || managerState == kManagerSuspended)
        return 0;
    if (!TransitionFilters(kManagerSuspended))
        return 0;
    ((MediaDispatch*)showCounter)->suspend();
    return ((ManagerDispatch*)this)->reportState(kManagerSuspended);
}

int gtl::CFilterManager::SetShowCounterParameter(float value) {
    showCounterParameter = value;
    return 1;
}

int gtl::CFilterManager::vfunc_12(int value) {
    return value != 0;
}

int gtl::CFilterManager::vfunc_13() {
    return 1;
}

int gtl::CFilterManager::vfunc_14() {
    return 1;
}

int gtl::CFilterManager::vfunc_15() {
    return 1;
}

int gtl::CFilterManager::CopyFormat(void* output) {
    const unsigned char* src;
    unsigned char* dst;
    int count;
    if (output != 0) {
        int managerState = state;
        if (managerState != kManagerReady && managerState != kManagerRunning && managerState != kManagerReset &&
            managerState != kManagerSuspended)
            return 0;
        dst = (unsigned char*)output;
        src = (const unsigned char*)format;
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
