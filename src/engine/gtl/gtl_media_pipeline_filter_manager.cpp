#include <decomp/local_class.h>
#include "classes/GtlMediaSample.h"
#define AsGtlMediaSample(p) ((GtlMediaSample*)(p))
#include "classes/GtlMediaFormat.h"
#define AsGtlMediaFormat(p) ((GtlMediaFormat*)(p))
#include "classes/GtlMediaPipeline.h"
#define AsGtlMediaPipeline(p) ((GtlMediaPipeline*)(p))
#include "classes/GtlMediaRegistryNode.h"
#define AsGtlMediaRegistryNode(p) ((GtlMediaRegistryNode*)(p))
#include "classes/GtlMediaRegistry.h"
#define AsGtlMediaRegistry(p) ((GtlMediaRegistry*)(p))

#include "sdk_protos.h"
// Translation unit 0x803F3F84-0x803F5428 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Filter states used by the pipeline's source, transform, split and output stages.
enum PipelineFilterState {
    kFilterReset = 0, kFilterRunning = 1, kFilterWaitingForInput = 2,
    kFilterStopped = 3, kFilterOutputReady = 4
};
// Manager states and transition requests are separate domains from filter result states.
enum PipelineManagerState {
    kPipelineInactive0 = 0, kPipelineLoading = 1, kPipelineReady = 2, kPipelineInactive4 = 4
};
enum FilterTransitionCommand {
    kFilterCommandStart = 3, kFilterCommandSuspend = 5, kFilterCommandReset = 4
};
enum PipelineError {
    kSourceFailure = 7, kTransformOrSplitFailure = 8,
    kPrimaryOutputFailure = 9, kSecondaryOutputFailure = 10, kLoaderSetupFailure = 11
};
enum { kPipelineStageCount = 5, kMediaFormatNameBytes = 0x80 };

struct RegistryValueReference { void* allocatorState; void** value; };

void gtl_Free(void*);
void* gtl_MediaRegistry_GetInstance(void*);
void gtl_MediaRegistry_RemoveByReference(void*, void*);
int gtl_IFilter_GetState(void*, int*);
int gtl_IShowCounter_GetCount(void*, int*);
int gtl_IShowCounter_GetPreviousCount(void*, int*);
int gtl_CFilterManager_UpdateLoading(void*);
int gtl_CFilterManager_ForwardSplitOutputs(void*, void*, void*, void*);
int gtl_CFilterManager_Update(void*);
void gtl_CMediaSample_GetSize(void*, int*);
int gtl_CMediaSample_GetData(void*, int*);
int gtl_CMediaSample_SetBuffer(void*, int, int);
void* gtl_Alloc(unsigned int);
void gtl_CRevolutionFilterManager_Ctor(void*, void*);
void* __register_global_object(void*, void*, void*);
void gtl_MediaRegistry_Dtor(void*, int);
int strcmp(const char*, const char*);
void SamplePool_Dtor(void*, int);
void gtl_MediaDvdRead_Dtor(void*, int);
void SamplePool_Ctor(void*, void*, void*);
void gtl_MediaDvdRead_Ctor(void*, void*, void*);
int DVDGetCommandBlockStatus(void*);
int gtl_MediaDvdRead_Update(void*);
int gtl_MediaDvdRead_GetData(void*, int*);
void gtl_CMediaSample_SetFormat(void*, void*);

// Issue a supported command to all five filters; stop at the first missing or failed stage.
int gtl_CFilterManager_TransitionFilters(void* self, int requestedState);

// Compare the format discriminator, name, timing and sample-layout fields.
int gtl_MediaFormat_Equals(const void* left, const void* right);

// Append a pipeline only if the registry does not already contain it.
int gtl_MediaRegistry_Add(void* registry, void* pipeline);

// Global registry storage: {count, sentinelPrevious, sentinelNext}.
// The sentinel address is &sentinelPrevious, and an empty list links both directions to it.
// Nodes own list storage only; their pipeline values are borrowed (see Add/Remove).
// The physical prefix uses the same field roles as GtlMediaRegistry.
struct RegNode { int count; void* sentinelPrevious; void* sentinelNext; };
RegNode s_gtlMediaRegistry;
char s_mediaRegistryDtorRegistration[12];
signed char s_mediaRegistryInitialized;

// Free a non-null registry-node allocation.
void dtor_803F47D8(void* allocation);

// Store an integer through the supplied output/storage pointer.
void GtlIntStorage_SetValue(void* storage, int value);

// Store the pipeline state and report success.
int gtl_CFilterManager_SetState(void* self, int state);

// Store the pipeline's integer control value; its policy is defined by the caller.
int gtl_CFilterManager_SetSourceRestartControl(void* self, int controlValue);

// Copy the pipeline's integer control value to the caller.
int gtl_CFilterManager_GetSourceRestartControl(void* self, int* outControlValue);

// Copy the pipeline's floating-point timing control to the caller.
int gtl_CFilterManager_GetShowCounterParameter(void* self, float* outTimingValue);

// Copy the current pipeline state to the caller.
int gtl_CFilterManager_GetState(void* self, int* outState);

// Free a non-null pipeline helper allocation.
void dtor_803F4E60(void* allocation);

// Free a non-null pipeline helper allocation.
void dtor_803F5310(void* allocation);

extern char __vt__Q23gtl12CMediaSample[];

#pragma auto_inline off
// Create a one-reference sample around a buffer, byte count, and format reference.
void gtl_CMediaSample_Ctor(void* self, int data, int byteCount, int format);
#pragma auto_inline on

#pragma auto_inline off
// Adapt a pipeline pointer to the registry's by-reference removal interface.
void gtl_MediaRegistry_Remove(void* registry, void* pipeline);
#pragma auto_inline on

// Unregister the pipeline, then invoke its deleting destructor.
void gtl_CFilterManager_Destroy(void* self);

// Report end-of-input when the source has output-ready state and the transform is waiting for input.
int gtl_CFilterManager_vfunc_28(void* self);

// Advance loading and each filter stage, transfer referenced samples, and stop on a stage error.
// Pipeline order: stage0 source -> stage1 transform -> stage2 split -> stage3/4
// primary/secondary sinks. Return 1 includes idle/loading/incomplete work; return
// 0 reports a failed operation after dispatching the corresponding error state.
// Local dispatch views preserve slot positions, not recovered SDK declarations:
// v5 = source completion/restart operation (exact semantics unresolved).
// Obj.v6 = read restart control; v21 = report state/error; v28 = end-of-input
// predicate; v8 = end-of-input handler (precise completion behavior unresolved).
// Each acquired sample is submitted, then the temporary reference is released;
// the receiver must keep its own reference if it retains the sample.
int gtl_CFilterManager_Update(void* self);

// Update registered pipelines in list order; return zero at the first failure.
int gtl_MediaRegistry_UpdateAll(void* registry);

// Release the show counter's reference and clear the pipeline's pointer.
void gtl_CFilterManager_ReleaseShowCounter(void* self);

// Lazily construct the global registry with an empty circular-list sentinel.
void* gtl_MediaRegistry_GetInstance(void* unused);

// Replace the primary buffer, freeing its old data only when no external buffer was set.
void gtl_CFilterManager_SetOutputBuffer(void* self, int buffer);

// Unlink and free registry nodes, then optionally free the registry object.
void gtl_MediaRegistry_Dtor(void* registry, int deleting);

// Release pipeline-owned samples, pool, format, DVD loader, and output references.
void gtl_CFilterManager_ReleaseResources(void* self);

#pragma auto_inline off
// Allocate a format and sample pool, then construct the sample wrapper and DVD loader.
int gtl_MediaPipeline_CreateSamplePoolAndDvdLoader(void* self);
#pragma auto_inline on

// Create the platform filter manager, or report availability without allocating in probe mode.
int gtl_FilterManagerFactory_Create(void* factory, void* outManager, int probeOnly);

// Return an added reference to the primary sample when its filter has usable output.
int gtl_CFilterManager_GetPrimaryOutput(void* manager, void* output);

// Return an added reference to the secondary sample only when its filter is output-ready.
int gtl_CFilterManager_GetSecondaryOutput(void* manager, void* output);

// Remove and free every registry node holding the referenced pipeline pointer.
void gtl_MediaRegistry_RemoveByReference(void* registry, void* valueReference);

// Update the split filter and forward output ports 0/1 to primary/secondary sinks.
// Borrow all three stages; return 0 only when split update fails, otherwise 1
// f6 submits input; release each acquired reference immediately after submission.
int gtl_CFilterManager_ForwardSplitOutputs(void* unused, void* splitFilter, void* primarySink, void* secondarySink);

// Poll DVD loading, configure the filters once data arrives, and finish a pending start.
int gtl_CFilterManager_UpdateLoading(void* self);

} // extern "C"


// Function definitions follow the original address order.


#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Return an added primary-sample reference only while the primary filter has usable output.
int gtl_CFilterManager_GetPrimaryOutput(void* manager, void* output) {
    int zero = 0;
    void* out = output;
    void* self = manager;
    // The original clears the output before its guard; callers must pass writable output storage.
    *(int*)out = zero;
    if (out == 0 || AsGtlMediaPipeline(self)->output0 == 0 || AsGtlMediaPipeline(self)->stage3 == 0) {
        return 0;
    }
    // The state is asked of the object at +0x5C (the original passes it straight on in r3), not of self.
    int filterState;
    gtl_IFilter_GetState(AsGtlMediaPipeline(self)->stage3, &filterState);
    if (filterState != kFilterOutputReady && filterState != kFilterStopped) {
        *(int*)out = zero;
        return 0;
    }
    void* ref = AsGtlMediaPipeline(self)->output0;
    *(void**)out = ref;
    ++*(int*)ref;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Return an added secondary-sample reference only while the secondary filter is output-ready.
int gtl_CFilterManager_GetSecondaryOutput(void* manager, void* output) {
    int zero = 0;
    void* out = output;
    void* self = manager;
    // The original clears the output before its guard; callers must pass writable output storage.
    *(int*)out = zero;
    if (out == 0 || AsGtlMediaPipeline(self)->output1 == 0 || AsGtlMediaPipeline(self)->stage4 == 0) {
        return 0;
    }
    int filterState;
    // The state is asked of the object at +0x60, not of self: the original loads it with
    // lwz r3,0x60(r3) at +0x3C and passes that r3 straight to bl gtl_IFilter_GetState at +0x54.
    gtl_IFilter_GetState(AsGtlMediaPipeline(self)->stage4, &filterState);
    if (filterState != kFilterOutputReady) {
        *(int*)out = zero;
        return 0;
    }
    void* ref = AsGtlMediaPipeline(self)->output1;
    *(void**)out = ref;
    ++*(int*)ref;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Replace the primary sample buffer, freeing old data only when internally owned.
void gtl_CFilterManager_SetOutputBuffer(void* self, int buffer) {
    int byteCount;
    gtl_CMediaSample_GetSize(AsGtlMediaPipeline(self)->output0, &byteCount);
    if (AsGtlMediaPipeline(self)->outputBuffer == 0) {
        int oldData;
        gtl_CMediaSample_GetData(AsGtlMediaPipeline(self)->output0, &oldData);
        if (oldData != 0) {
            gtl_Free((void*)oldData);
            oldData = 0;
        }
    }
    AsGtlMediaPipeline(self)->outputBuffer = buffer;
    gtl_CMediaSample_SetBuffer(AsGtlMediaPipeline(self)->output0, buffer, byteCount);
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Update each pipeline stage, transfer temporary sample references, and report stage failures.
int gtl_CFilterManager_Update(void* self) {
    struct Obj {
        virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual int v1() DECOMP_LOCAL_VIRTUAL; virtual void v2() DECOMP_LOCAL_VIRTUAL; virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL; virtual void v5() DECOMP_LOCAL_VIRTUAL; virtual void ReadRestartControlSlot6(int*) DECOMP_LOCAL_VIRTUAL; virtual void v7() DECOMP_LOCAL_VIRTUAL;
        virtual int HandleEndOfInputSlot8() DECOMP_LOCAL_VIRTUAL; virtual void v9() DECOMP_LOCAL_VIRTUAL; virtual void v10() DECOMP_LOCAL_VIRTUAL; virtual void v11() DECOMP_LOCAL_VIRTUAL;
        virtual void v12() DECOMP_LOCAL_VIRTUAL; virtual void v13() DECOMP_LOCAL_VIRTUAL; virtual void v14() DECOMP_LOCAL_VIRTUAL; virtual void v15() DECOMP_LOCAL_VIRTUAL;
        virtual void v16() DECOMP_LOCAL_VIRTUAL; virtual void v17() DECOMP_LOCAL_VIRTUAL; virtual void v18() DECOMP_LOCAL_VIRTUAL; virtual void v19() DECOMP_LOCAL_VIRTUAL;
        virtual void v20() DECOMP_LOCAL_VIRTUAL; virtual void ReportStateOrErrorSlot21(int) DECOMP_LOCAL_VIRTUAL; virtual void v22(int) DECOMP_LOCAL_VIRTUAL; virtual void v23(int) DECOMP_LOCAL_VIRTUAL;
        virtual int v24() DECOMP_LOCAL_VIRTUAL; virtual void v25() DECOMP_LOCAL_VIRTUAL; virtual void v26() DECOMP_LOCAL_VIRTUAL; virtual void v27() DECOMP_LOCAL_VIRTUAL;
        virtual int IsEndOfInputSlot28() DECOMP_LOCAL_VIRTUAL; virtual void v29(int) DECOMP_LOCAL_VIRTUAL; virtual void v30() DECOMP_LOCAL_VIRTUAL; virtual void v31() DECOMP_LOCAL_VIRTUAL;
    };
    struct Iface {
        char pad[4];
        virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual int UpdateSlot1() DECOMP_LOCAL_VIRTUAL; virtual void v2() DECOMP_LOCAL_VIRTUAL; virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL; virtual void SourceCompletionSlot5() DECOMP_LOCAL_VIRTUAL; virtual void SubmitInputSlot6(int) DECOMP_LOCAL_VIRTUAL; virtual void AcquireOutputSlot7(int, void**) DECOMP_LOCAL_VIRTUAL;
        virtual void v8() DECOMP_LOCAL_VIRTUAL; virtual void v9() DECOMP_LOCAL_VIRTUAL; virtual void v10() DECOMP_LOCAL_VIRTUAL;
    };
    struct Ref { int count; 
    virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
    }; // count, then the class table (destroy is its first entry)
    struct State { int value; };
    // Each union preserves one original address-taken stack slot while naming its
    // two successive domains. Count members are dead before state/control writes.
    int transformState;
    union { int showCount; int sourceState; } sourceQuery;
    union { int previousShowCount; int restartControl; } controlQuery;
    int nextTransformState;
    volatile int frame[8];
    // Phase 1: skip inactive managers, progress loading, then require all five stages.
    int state = AsGtlMediaPipeline(self)->state;
    if (state == kPipelineInactive4 || state == kPipelineInactive0) return 1;
    if (!gtl_CFilterManager_UpdateLoading(self)) return 0;
    if (!AsGtlMediaPipeline(self)->stage0 || !AsGtlMediaPipeline(self)->stage1 ||
        !AsGtlMediaPipeline(self)->stage2 || !AsGtlMediaPipeline(self)->stage3 || !AsGtlMediaPipeline(self)->stage4) return 1;
    if (((Obj*)self)->IsEndOfInputSlot28()) return ((Obj*)self)->HandleEndOfInputSlot8();
    else {
        void* p = AsGtlMediaPipeline(self)->showCounter;
        if (p) { struct Other { char pad[4]; 
        virtual void f0() DECOMP_LOCAL_VIRTUAL; virtual void f1() DECOMP_LOCAL_VIRTUAL; virtual void f2() DECOMP_LOCAL_VIRTUAL; virtual void f3() DECOMP_LOCAL_VIRTUAL; virtual void f4() DECOMP_LOCAL_VIRTUAL; virtual void f5(float, float) DECOMP_LOCAL_VIRTUAL;
        }; ((Other*)p)->f5(AsGtlMediaFormat(AsGtlMediaPipeline(self)->format)->showCounterFormatParameter, AsGtlMediaPipeline(self)->showCounterParameter); }
    }
    // Phase 2: synchronize transitions with the show counter when transform output
    // is stopped/ready. Equal counts suspend; a changed count starts the filters.
    gtl_IFilter_GetState(AsGtlMediaPipeline(self)->stage1, &transformState);
    if (transformState == kFilterStopped || transformState == kFilterOutputReady) {
        gtl_IShowCounter_GetCount(AsGtlMediaPipeline(self)->showCounter, &sourceQuery.showCount);
        gtl_IShowCounter_GetPreviousCount(AsGtlMediaPipeline(self)->showCounter, &controlQuery.previousShowCount);
        gtl_CFilterManager_TransitionFilters(self, sourceQuery.showCount == controlQuery.previousShowCount ? kFilterCommandSuspend : kFilterCommandStart);
    }
    // Phase 3: update source, acquire port 0 and submit to transform. End-of-source
    // processing then overwrites the former show-count slots with state/control.
    void* sourceFilter = AsGtlMediaPipeline(self)->stage0;
    void* transformFilter = AsGtlMediaPipeline(self)->stage1;
    if (!((Iface*)sourceFilter)->UpdateSlot1()) { ((Obj*)self)->ReportStateOrErrorSlot21(kSourceFailure); return 0; }
    {
        void* value = 0;
        ((Iface*)sourceFilter)->AcquireOutputSlot7(0, &value);
        ((Iface*)transformFilter)->SubmitInputSlot6((int)value);
        if (value) {
            Ref* r = (Ref*)value;
            --r->count;
            if (r->count == 0) {
                if (value) r->destroy(1);
            }
            value = 0;
        }
    }
    gtl_IFilter_GetState(sourceFilter, &sourceQuery.sourceState);
    if (sourceQuery.sourceState == kFilterOutputReady) {
        ((Obj*)self)->ReadRestartControlSlot6(&controlQuery.restartControl);
        if (controlQuery.restartControl == 1) ((Iface*)sourceFilter)->SourceCompletionSlot5();
    }
    // Phase 4: transform update; forward port 0 only when output-ready.
    if (!((Iface*)transformFilter)->UpdateSlot1()) { ((Obj*)self)->ReportStateOrErrorSlot21(kTransformOrSplitFailure); return 0; }
    gtl_IFilter_GetState(transformFilter, &nextTransformState);
    if (nextTransformState == kFilterOutputReady) {
        void* value = 0;
        ((Iface*)transformFilter)->AcquireOutputSlot7(0, &value);
        ((Iface*)AsGtlMediaPipeline(self)->stage2)->SubmitInputSlot6((int)value);
        if (value) {
            Ref* r = (Ref*)value;
            --r->count;
            if (r->count == 0) {
                if (value) r->destroy(1);
            }
            value = 0;
        }
    }
    // (7), transform/split (8), primary (9), and secondary (10) failure.
    if (!gtl_CFilterManager_ForwardSplitOutputs(self, AsGtlMediaPipeline(self)->stage2, AsGtlMediaPipeline(self)->stage3, AsGtlMediaPipeline(self)->stage4)) { ((Obj*)self)->ReportStateOrErrorSlot21(kTransformOrSplitFailure); return 0; }
    if (!((Iface*)AsGtlMediaPipeline(self)->stage3)->UpdateSlot1()) { ((Obj*)self)->ReportStateOrErrorSlot21(kPrimaryOutputFailure); return 0; }
    if (!((Iface*)AsGtlMediaPipeline(self)->stage4)->UpdateSlot1()) { ((Obj*)self)->ReportStateOrErrorSlot21(kSecondaryOutputFailure); return 0; }
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Unregister the pipeline before invoking its deleting destructor.
void gtl_CFilterManager_Destroy(void* self) {
    void* registry = gtl_MediaRegistry_GetInstance(self);
    gtl_MediaRegistry_Remove(registry, self);
    if (self != 0) {
        struct Base { 
        virtual void v2(int) DECOMP_LOCAL_VIRTUAL;
        };
        ((Base*)self)->v2(1);
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Release the show-counter reference and clear the pipeline alias.
void gtl_CFilterManager_ReleaseShowCounter(void* self) {
    void* showCounter = AsGtlMediaPipeline(self)->showCounter;
    if (showCounter != 0) {
        if (--(*(int*)showCounter) == 0 && showCounter != 0) {
            struct Prefix { char pad[4]; };
            struct Ref : Prefix { 
            virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((Ref*)showCounter)->destroy(1);
        }
        AsGtlMediaPipeline(self)->showCounter = 0;
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Store the pipeline manager state and return success.
int gtl_CFilterManager_SetState(void* self, int state) {
    AsGtlMediaPipeline(self)->state = state;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Test for source completion while the transform waits for more input.
int gtl_CFilterManager_vfunc_28(void* self) {
    int sourceState;
    int transformState;
    gtl_IFilter_GetState(AsGtlMediaPipeline(self)->stage0, &sourceState);
    gtl_IFilter_GetState(AsGtlMediaPipeline(self)->stage1, &transformState);
    return sourceState == kFilterOutputReady && transformState == kFilterWaitingForInput;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline off
extern "C" {
// Allocate format/pool storage and construct the sample wrapper and DVD loader.
int gtl_MediaPipeline_CreateSamplePoolAndDvdLoader(void* self) {
    if (AsGtlMediaPipeline(self)->format != 0) {
        gtl_Free(AsGtlMediaPipeline(self)->format);
        AsGtlMediaPipeline(self)->format = 0;
    }
    AsGtlMediaPipeline(self)->format = gtl_Alloc(sizeof(GtlMediaFormat));
    strncpy(AsGtlMediaFormat(AsGtlMediaPipeline(self)->format)->name, AsGtlMediaPipeline(self)->pendingName, kMediaFormatNameBytes);
    (*(void**)&AsGtlMediaFormat(AsGtlMediaPipeline(self)->format)->sourceByteLength) = AsGtlMediaPipeline(self)->sourceByteLengthWord;
    if ((*(void**)&AsGtlMediaPipeline(self)->pendingName) != 0) {
        gtl_Free((*(void**)&AsGtlMediaPipeline(self)->pendingName));
        (*(void**)&AsGtlMediaPipeline(self)->pendingName) = 0;
    }
    void* samplePool = gtl_Alloc(0x54);
    if (samplePool != 0) {
        try {
            SamplePool_Ctor(samplePool, &AsGtlMediaPipeline(self)->sampleSize, AsGtlMediaPipeline(self)->format);
        } catch (...) {
            gtl_Free(samplePool);
            throw;
        }
    }
    AsGtlMediaPipeline(self)->samplePool = samplePool;
    void* poolSample = gtl_Alloc(0x14);
    if (poolSample != 0) gtl_CMediaSample_Ctor(poolSample, (int)AsGtlMediaPipeline(self)->samplePool, 0, (int)AsGtlMediaPipeline(self)->format);
    AsGtlMediaPipeline(self)->poolSample = poolSample;
    if (AsGtlMediaPipeline(self)->loader != 0) gtl_MediaDvdRead_Dtor(AsGtlMediaPipeline(self)->loader, 1);
    struct Base { 
    virtual void f0() DECOMP_LOCAL_VIRTUAL; virtual void f1() DECOMP_LOCAL_VIRTUAL; virtual void f2() DECOMP_LOCAL_VIRTUAL; virtual void f3() DECOMP_LOCAL_VIRTUAL; virtual void f4() DECOMP_LOCAL_VIRTUAL; virtual void f5() DECOMP_LOCAL_VIRTUAL; virtual void f6() DECOMP_LOCAL_VIRTUAL; virtual void f7() DECOMP_LOCAL_VIRTUAL; virtual void f8() DECOMP_LOCAL_VIRTUAL; virtual void f9() DECOMP_LOCAL_VIRTUAL; virtual void f10() DECOMP_LOCAL_VIRTUAL; virtual void f11() DECOMP_LOCAL_VIRTUAL; virtual void f12() DECOMP_LOCAL_VIRTUAL; virtual void f13() DECOMP_LOCAL_VIRTUAL; virtual void f14() DECOMP_LOCAL_VIRTUAL; virtual void f15() DECOMP_LOCAL_VIRTUAL; virtual void f16() DECOMP_LOCAL_VIRTUAL; virtual void f17() DECOMP_LOCAL_VIRTUAL; virtual void f18() DECOMP_LOCAL_VIRTUAL; virtual void f19() DECOMP_LOCAL_VIRTUAL; virtual void f20() DECOMP_LOCAL_VIRTUAL; virtual void f21() DECOMP_LOCAL_VIRTUAL; virtual void f22(void**, void*) DECOMP_LOCAL_VIRTUAL;
    };
    void* result;
    ((Base*)self)->f22(&result, AsGtlMediaPipeline(self)->format);
    void* loader = gtl_Alloc(0x10);
    if (loader != 0) gtl_MediaDvdRead_Ctor(loader, result, AsGtlMediaPipeline(self)->inputFile);
    AsGtlMediaPipeline(self)->loader = loader;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Free a non-null registry-node allocation.
void dtor_803F47D8(void* allocation) {
    if (allocation != 0) {
        gtl_Free(allocation);
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Poll DVD loading, configure the filters, and finish any deferred start.
int gtl_CFilterManager_UpdateLoading(void* self) {
    // Phase 1: create the loader/pool for a loading pipeline without a format.
    struct Obj {
        virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual void v1() DECOMP_LOCAL_VIRTUAL; virtual void v2() DECOMP_LOCAL_VIRTUAL; virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL; virtual void v5() DECOMP_LOCAL_VIRTUAL; virtual void v6() DECOMP_LOCAL_VIRTUAL; virtual void v7() DECOMP_LOCAL_VIRTUAL;
        virtual void v8() DECOMP_LOCAL_VIRTUAL; virtual void v9() DECOMP_LOCAL_VIRTUAL; virtual void v10() DECOMP_LOCAL_VIRTUAL; virtual void v11() DECOMP_LOCAL_VIRTUAL;
        virtual void v12() DECOMP_LOCAL_VIRTUAL; virtual void v13() DECOMP_LOCAL_VIRTUAL; virtual void v14() DECOMP_LOCAL_VIRTUAL; virtual void v15() DECOMP_LOCAL_VIRTUAL;
        virtual void v16() DECOMP_LOCAL_VIRTUAL; virtual void v17() DECOMP_LOCAL_VIRTUAL; virtual void v18() DECOMP_LOCAL_VIRTUAL; virtual void v19() DECOMP_LOCAL_VIRTUAL;
        virtual void v20() DECOMP_LOCAL_VIRTUAL; virtual void ReportStateOrErrorSlot21(int) DECOMP_LOCAL_VIRTUAL; virtual void v22() DECOMP_LOCAL_VIRTUAL; virtual void PostLoadHookSlot23() DECOMP_LOCAL_VIRTUAL;
        virtual void v24() DECOMP_LOCAL_VIRTUAL; virtual void v25() DECOMP_LOCAL_VIRTUAL; virtual void v26() DECOMP_LOCAL_VIRTUAL; virtual void ConfigureLoadedHeaderSlot27(int, int) DECOMP_LOCAL_VIRTUAL;
        virtual void v28() DECOMP_LOCAL_VIRTUAL; virtual void v29() DECOMP_LOCAL_VIRTUAL;
    };
    struct Holder { char pad[4]; 
    virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual void v1() DECOMP_LOCAL_VIRTUAL; virtual void v2() DECOMP_LOCAL_VIRTUAL; virtual void v3() DECOMP_LOCAL_VIRTUAL; virtual void v4() DECOMP_LOCAL_VIRTUAL; virtual void v5() DECOMP_LOCAL_VIRTUAL; virtual void SubmitInputSlot6(int) DECOMP_LOCAL_VIRTUAL; virtual void v7() DECOMP_LOCAL_VIRTUAL; virtual void v8() DECOMP_LOCAL_VIRTUAL;
    };
    struct Handle { char pad[4]; 
    virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual int UpdateSlot1() DECOMP_LOCAL_VIRTUAL; virtual void StartSlot2() DECOMP_LOCAL_VIRTUAL;
    };
    int loadedHeader;
    if ((*(int*)&AsGtlMediaPipeline(self)->format) == 0 && AsGtlMediaPipeline(self)->state == kPipelineLoading) {
        DVDGetCommandBlockStatus(AsGtlMediaPipeline(self)->inputFile);
        if (!gtl_MediaPipeline_CreateSamplePoolAndDvdLoader(self)) {
            ((Obj*)self)->ReportStateOrErrorSlot21(kLoaderSetupFailure);
            return 0;
        }
    }
    // Phase 2: poll the DVD loader; configure filters only after its header arrives.
    if (AsGtlMediaPipeline(self)->loader) {
        int ok;
        if (!gtl_MediaDvdRead_Update(AsGtlMediaPipeline(self)->loader)) ok = 0;
        else {
            if (gtl_MediaDvdRead_GetData(AsGtlMediaPipeline(self)->loader, &loadedHeader) == 1) {
                ((Obj*)self)->ConfigureLoadedHeaderSlot27(loadedHeader, (*(int*)&AsGtlMediaPipeline(self)->format));
                gtl_CMediaSample_SetFormat(AsGtlMediaPipeline(self)->output0, AsGtlMediaPipeline(self)->format);
                gtl_CMediaSample_SetFormat(AsGtlMediaPipeline(self)->output1, AsGtlMediaPipeline(self)->format);
                void* loader = AsGtlMediaPipeline(self)->loader;
                if (loader) { gtl_MediaDvdRead_Dtor(loader, 1); AsGtlMediaPipeline(self)->loader = 0; }
                ((Obj*)self)->PostLoadHookSlot23();
                ((Holder*)AsGtlMediaPipeline(self)->stage0)->SubmitInputSlot6((*(int*)&AsGtlMediaPipeline(self)->poolSample));
            }
            ok = 1;
        }
        if (!ok) { ((Obj*)self)->ReportStateOrErrorSlot21(kSourceFailure); return 0; }
        // Every part loaded, the loader gone (0x80 cleared) and 0x4C set.
        int valid = AsGtlMediaPipeline(self)->stage0 && AsGtlMediaPipeline(self)->stage1 &&
            AsGtlMediaPipeline(self)->stage2 && AsGtlMediaPipeline(self)->stage3 &&
            AsGtlMediaPipeline(self)->stage4 && !AsGtlMediaPipeline(self)->loader && AsGtlMediaPipeline(self)->showCounter;
        if (valid) {
            if (AsGtlMediaPipeline(self)->startPending) ((Handle*)AsGtlMediaPipeline(self)->stage0)->StartSlot2();
            else ((Obj*)self)->ReportStateOrErrorSlot21(kPipelineReady);
        }
    }
    // Phase 3: drive the deferred source start until it reaches waiting-for-input.
    if (AsGtlMediaPipeline(self)->startPending && AsGtlMediaPipeline(self)->stage0) {
        if (!((Handle*)AsGtlMediaPipeline(self)->stage0)->UpdateSlot1()) { ((Obj*)self)->ReportStateOrErrorSlot21(kSourceFailure); return 0; }
        int state;
        gtl_IFilter_GetState(AsGtlMediaPipeline(self)->stage0, &state);
        if (state == kFilterWaitingForInput) { ((Obj*)self)->ReportStateOrErrorSlot21(kPipelineReady); AsGtlMediaPipeline(self)->startPending = 0; }
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Issue a supported transition command to all five filters in order.
int gtl_CFilterManager_TransitionFilters(void* self, int requestedState) {
    struct Table { 
    virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual void v1() DECOMP_LOCAL_VIRTUAL; virtual int v2() DECOMP_LOCAL_VIRTUAL; virtual int v3() DECOMP_LOCAL_VIRTUAL; virtual int v4() DECOMP_LOCAL_VIRTUAL;
    };
    struct Holder { char pad[4]; 
    virtual void v0() DECOMP_LOCAL_VIRTUAL; virtual void v1() DECOMP_LOCAL_VIRTUAL; virtual int StartSlot2() DECOMP_LOCAL_VIRTUAL; virtual int SuspendSlot3() DECOMP_LOCAL_VIRTUAL; virtual int ResetSlot4() DECOMP_LOCAL_VIRTUAL;
    };
    void* stageCursor = self;
    int stageIndex = 0;
    int result;
    while (stageIndex < kPipelineStageCount) {
        void* filter = AsGtlMediaPipeline(stageCursor)->stage0;
        if (filter == 0) return 0;
        // Callers pass only the three transition codes handled below.
        switch (requestedState) {
        case kFilterCommandStart:
            result = ((Holder*)filter)->StartSlot2();
            break;
        case kFilterCommandSuspend:
            result = ((Holder*)filter)->SuspendSlot3();
            break;
        case kFilterCommandReset:
            result = ((Holder*)filter)->ResetSlot4();
            break;
        }
        if (result == 0) return 0;
        ++stageIndex;
        stageCursor = (char*)stageCursor + 4;
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Return the pipeline source-restart control word through caller storage.
int gtl_CFilterManager_GetSourceRestartControl(void* self, int* outControlValue) {
    *outControlValue = AsGtlMediaPipeline(self)->sourceRestartControl;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
int gtl_CFilterManager_ForwardSplitOutputs(void* unused, void* splitFilter, void* primarySink, void* secondarySink) {
    struct Iface {
        char pad[4];
        virtual void f0() DECOMP_LOCAL_VIRTUAL; virtual int UpdateSlot1() DECOMP_LOCAL_VIRTUAL; virtual void f2() DECOMP_LOCAL_VIRTUAL;
        virtual void f3() DECOMP_LOCAL_VIRTUAL; virtual void f4() DECOMP_LOCAL_VIRTUAL; virtual void f5() DECOMP_LOCAL_VIRTUAL; virtual void f6() DECOMP_LOCAL_VIRTUAL;
        virtual void AcquireOutputSlot7(int, void**) DECOMP_LOCAL_VIRTUAL;
    };
    struct Sink {
        char pad[4];
        virtual void f0() DECOMP_LOCAL_VIRTUAL; virtual void f1() DECOMP_LOCAL_VIRTUAL; virtual void f2() DECOMP_LOCAL_VIRTUAL; virtual void f3() DECOMP_LOCAL_VIRTUAL;
        virtual void f4() DECOMP_LOCAL_VIRTUAL; virtual void f5() DECOMP_LOCAL_VIRTUAL; virtual void SubmitInputSlot6(void*) DECOMP_LOCAL_VIRTUAL;
    };
    struct RefBase { int count; };
    struct Ref : RefBase { 
    virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
    };
    int splitState;
    if (!((Iface*)splitFilter)->UpdateSlot1()) return 0;
    gtl_IFilter_GetState(splitFilter, &splitState);
    if (splitState == kFilterOutputReady) {
        void* outputSample = 0;
        ((Iface*)splitFilter)->AcquireOutputSlot7(0, (void**)&outputSample);
        ((Sink*)primarySink)->SubmitInputSlot6(outputSample);
        if (outputSample != 0 && --((Ref*)outputSample)->count == 0) ((Ref*)outputSample)->destroy(1);
        outputSample = 0;
        ((Iface*)splitFilter)->AcquireOutputSlot7(1, (void**)&outputSample);
        ((Sink*)secondarySink)->SubmitInputSlot6(outputSample);
        if (outputSample != 0 && --((Ref*)outputSample)->count == 0) ((Ref*)outputSample)->destroy(1);
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Run virtual cleanup hooks, then release samples, pool, format, loader and outputs.
void gtl_CFilterManager_ReleaseResources(void* self) {
    struct V { 
    virtual void f0() DECOMP_LOCAL_VIRTUAL; virtual void f1() DECOMP_LOCAL_VIRTUAL; virtual void f2() DECOMP_LOCAL_VIRTUAL; virtual void f3() DECOMP_LOCAL_VIRTUAL; virtual void f4() DECOMP_LOCAL_VIRTUAL; virtual void f5() DECOMP_LOCAL_VIRTUAL; virtual void f6() DECOMP_LOCAL_VIRTUAL; virtual void f7() DECOMP_LOCAL_VIRTUAL; virtual void f8() DECOMP_LOCAL_VIRTUAL; virtual void f9() DECOMP_LOCAL_VIRTUAL; virtual void f10() DECOMP_LOCAL_VIRTUAL; virtual void f11() DECOMP_LOCAL_VIRTUAL; virtual void f12() DECOMP_LOCAL_VIRTUAL; virtual void f13() DECOMP_LOCAL_VIRTUAL; virtual void f14() DECOMP_LOCAL_VIRTUAL; virtual void f15() DECOMP_LOCAL_VIRTUAL; virtual void f16() DECOMP_LOCAL_VIRTUAL; virtual void f17() DECOMP_LOCAL_VIRTUAL; virtual void f18() DECOMP_LOCAL_VIRTUAL; virtual void f19() DECOMP_LOCAL_VIRTUAL; virtual void f20() DECOMP_LOCAL_VIRTUAL; virtual void f21() DECOMP_LOCAL_VIRTUAL; virtual void f22() DECOMP_LOCAL_VIRTUAL; virtual void f23() DECOMP_LOCAL_VIRTUAL; virtual void f24() DECOMP_LOCAL_VIRTUAL; virtual void f25() DECOMP_LOCAL_VIRTUAL; virtual void f26() DECOMP_LOCAL_VIRTUAL; virtual void f27() DECOMP_LOCAL_VIRTUAL; virtual void f28() DECOMP_LOCAL_VIRTUAL;
    };
    struct RefBase { int count; };
    struct Ref : RefBase { 
    virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
    };
    int zero = 0;
    // Phase 1: invoke virtual cleanup slots 24 and 26 while every resource is live.
    // Their exact policies are unresolved in this unit; they precede sample/pool
    // retirement and must retain this order (not interchangeable with ref releases).
    ((V*)self)->f24();
    ((V*)self)->f26();
    // Phase 2: drop sample references, destroy pool/loader, then clear output refs.
    void* resource = AsGtlMediaPipeline(self)->poolSample;
    if (resource != 0 && --((Ref*)resource)->count == 0) ((Ref*)resource)->destroy(1);
    AsGtlMediaPipeline(self)->poolSample = (void*)zero;
    resource = AsGtlMediaPipeline(self)->samplePool;
    if (resource != 0) { SamplePool_Dtor(resource, 1); AsGtlMediaPipeline(self)->samplePool = (void*)zero; }
    resource = AsGtlMediaPipeline(self)->format;
    if (resource != 0) { gtl_Free(resource); AsGtlMediaPipeline(self)->format = (void*)zero; }
    resource = AsGtlMediaPipeline(self)->loader;
    if (resource != 0) { gtl_MediaDvdRead_Dtor(resource, 1); AsGtlMediaPipeline(self)->loader = (void*)zero; }
    resource = AsGtlMediaPipeline(self)->output0;
    if (resource != 0 && --((Ref*)resource)->count == 0) ((Ref*)resource)->destroy(1);
    AsGtlMediaPipeline(self)->output0 = (void*)zero;
    resource = AsGtlMediaPipeline(self)->output1;
    if (resource != 0 && --((Ref*)resource)->count == 0) ((Ref*)resource)->destroy(1);
    AsGtlMediaPipeline(self)->output1 = (void*)zero;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Free a non-null pipeline helper allocation.
void dtor_803F4E60(void* allocation) {
    if (allocation != 0) {
        gtl_Free(allocation);
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Copy the pipeline timing-control scalar to caller storage.
int gtl_CFilterManager_GetShowCounterParameter(void* self, float* outTimingValue) {
    *outTimingValue = AsGtlMediaPipeline(self)->showCounterParameter;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Store the pipeline control word without interpreting caller policy.
int gtl_CFilterManager_SetSourceRestartControl(void* self, int controlValue) {
    AsGtlMediaPipeline(self)->sourceRestartControl = controlValue;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Copy the current pipeline manager state to caller storage.
int gtl_CFilterManager_GetState(void* self, int* outState) {
    *outState = AsGtlMediaPipeline(self)->state;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Drain the circular registry list and optionally release its header allocation.
void gtl_MediaRegistry_Dtor(void* registry, int deleting) {
    if (registry != 0) {
        void* first = &AsGtlMediaRegistry(registry)->sentinelPrevious;
        void* last = AsGtlMediaRegistry(registry)->sentinelNext;
        if (last != first) {
            void* before = AsGtlMediaRegistryNode(first)->previous;
            void* lastNode = AsGtlMediaRegistryNode(last)->previous;
            AsGtlMediaRegistryNode(lastNode)->next = AsGtlMediaRegistryNode(before)->next;
            *(*(void***)&AsGtlMediaRegistryNode(before)->next) = lastNode;
            while (last != first) {
                void* node = last;
                last = AsGtlMediaRegistryNode(last)->next;
                gtl_Free(node);
                --AsGtlMediaRegistry(registry)->count;
            }
        }
        // Second pass is the expanded member/base container cleanup. The first
        // pass already drained the circular list; retain its original sentinel check.
        void* end2 = &AsGtlMediaRegistry(registry)->sentinelPrevious;
        void* cur2 = AsGtlMediaRegistry(registry)->sentinelNext;
        if (cur2 != end2) {
            void* node2 = *(void**)end2;
            void* tail2 = AsGtlMediaRegistryNode(cur2)->previous;
            AsGtlMediaRegistryNode(tail2)->next = AsGtlMediaRegistryNode(node2)->next;
            *(*(void***)&AsGtlMediaRegistryNode(node2)->next) = tail2;
            while (cur2 != end2) {
                void* doomed2 = cur2;
                cur2 = AsGtlMediaRegistryNode(cur2)->next;
                gtl_Free(doomed2);
                --AsGtlMediaRegistry(registry)->count;
            }
        }
        if (deleting > 0) gtl_Free(registry);
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Lazily construct the registry with both sentinel links pointing to itself.
void* gtl_MediaRegistry_GetInstance(void* unused) {
    if (!s_mediaRegistryInitialized) {
        AsGtlMediaRegistry(&s_gtlMediaRegistry)->count = 0;
        void* sentinel = (char*)&s_gtlMediaRegistry + 4;
        AsGtlMediaRegistry(&s_gtlMediaRegistry)->sentinelNext = sentinel;
        AsGtlMediaRegistry(&s_gtlMediaRegistry)->sentinelPrevious = sentinel;
        __register_global_object(&s_gtlMediaRegistry, (void*)gtl_MediaRegistry_Dtor, s_mediaRegistryDtorRegistration);
        s_mediaRegistryInitialized = 1;
    }
    return &s_gtlMediaRegistry;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Append a borrowed pipeline pointer only when it is absent from the registry.
int gtl_MediaRegistry_Add(void* registry, void* pipeline) {
    void* sentinel = &AsGtlMediaRegistry(registry)->sentinelPrevious;
    void* cursor = AsGtlMediaRegistry(registry)->sentinelNext;
    while (cursor != sentinel && AsGtlMediaRegistryNode(cursor)->value != pipeline) {
        cursor = AsGtlMediaRegistryNode(cursor)->next;
    }
    if (cursor != sentinel) return 0;
    void* node = gtl_Alloc(12);
    AsGtlMediaRegistryNode(node)->value = pipeline;
    void* previous = *(void**)sentinel;
    AsGtlMediaRegistryNode(previous)->next = node;
    AsGtlMediaRegistryNode(node)->previous = previous;
    *(void**)sentinel = node;
    AsGtlMediaRegistryNode(node)->next = sentinel;
    ++AsGtlMediaRegistry(registry)->count;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Update registered pipelines in list order, stopping on the first failure.
int gtl_MediaRegistry_UpdateAll(void* registry) {
    void* cursor;
    void* sentinel;
    sentinel = &AsGtlMediaRegistry(registry)->sentinelPrevious;
    cursor = AsGtlMediaRegistry(registry)->sentinelNext;
    while (cursor != sentinel) {
        if (!gtl_CFilterManager_Update(AsGtlMediaRegistryNode(cursor)->value)) return 0;
        cursor = AsGtlMediaRegistryNode(cursor)->next;
    }
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline off
extern "C" {
// Adapt a pipeline pointer to the by-reference registry removal ABI.
void gtl_MediaRegistry_Remove(void* registry, void* pipeline) {
    struct Frame {
        void* slot;
        char pad[4];
        void* t;
        void** pslot;
        struct { void* first; void** second; } r;
    } f;
    f.slot = pipeline;
    f.pslot = &f.slot;
    f.r.first = f.t;
    f.r.second = f.pslot;
    gtl_MediaRegistry_RemoveByReference(registry, &f.r);
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Unlink and free all list nodes holding the referenced pipeline pointer.
void gtl_MediaRegistry_RemoveByReference(void* registry, void* valueReference) {
    void* self = registry;
    void* arg = valueReference;
    void* sentinel = &AsGtlMediaRegistry(self)->sentinelPrevious;
    void* cur = AsGtlMediaRegistry(self)->sentinelNext;
    while (cur != sentinel) {
        void* key = *((RegistryValueReference*)arg)->value;
        if (AsGtlMediaRegistryNode(cur)->value == key) {
            void* scan = AsGtlMediaRegistryNode(cur)->next;
            while (scan != sentinel && AsGtlMediaRegistryNode(scan)->value == key) scan = AsGtlMediaRegistryNode(scan)->next;
            if (cur != scan) {
                void* first = AsGtlMediaRegistryNode(scan)->previous;
                void* node = AsGtlMediaRegistryNode(cur)->previous;
                AsGtlMediaRegistryNode(node)->next = AsGtlMediaRegistryNode(first)->next;
                *(*(void***)&AsGtlMediaRegistryNode(first)->next) = node;
                while (cur != scan) {
                    void* removed = cur;
                    cur = AsGtlMediaRegistryNode(cur)->next;
                    gtl_Free(removed);
                    --AsGtlMediaRegistry(self)->count;
                }
            }
            cur = scan;
            if (cur == sentinel) break;
        } else {
            cur = AsGtlMediaRegistryNode(cur)->next;
        }
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Store an integer through the borrowed output pointer.
void GtlIntStorage_SetValue(void* storage, int value) {
    *(int*)storage = value;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
extern "C" {
// Probe availability or allocate the platform filter manager.
int gtl_FilterManagerFactory_Create(void* factory, void* outManager, int probeOnly) {
    void* manager;
    if (outManager == 0 || *(void**)factory == 0) return 0;
    if (probeOnly != 0) return 1;
    manager = gtl_Alloc(0xf0);
    if (manager != 0) {
        gtl_CRevolutionFilterManager_Ctor(manager, *(void**)factory);
    }
    *(void**)outManager = manager;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Free a non-null pipeline helper allocation.
void dtor_803F5310(void* allocation) {
    if (allocation != 0) {
        gtl_Free(allocation);
    }
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
extern "C" {
// Compare format discriminator, name, timing and sample-layout fields.
int gtl_MediaFormat_Equals(const void* left, const void* right) {
    const int* leftWords = (const int*)left;
    const int* rightWords = (const int*)right;
    int leftKind = *leftWords;
    int rightKind = *rightWords;
    // Original 0x803F3F84 unit: preserve the shared mismatch return.
    do {
        if (leftKind != rightKind) break;
        if (AsGtlMediaFormat(left)->sourceByteLength != AsGtlMediaFormat(right)->sourceByteLength) break;
        if (strcmp(AsGtlMediaFormat(left)->name, AsGtlMediaFormat(right)->name) != 0) break;
        if (AsGtlMediaFormat(left)->showCounterFormatParameter != AsGtlMediaFormat(right)->showCounterFormatParameter) break; // rate passed to the show counter
        if (AsGtlMediaFormat(left)->initialReadOffset != AsGtlMediaFormat(right)->initialReadOffset) break;
        if (AsGtlMediaFormat(left)->sampleBufferSize != AsGtlMediaFormat(right)->sampleBufferSize) break;
        // Compare both 16-bit sample-format attributes without guessing their units or dimensions.
        if (AsGtlMediaFormat(left)->unk_98 != AsGtlMediaFormat(right)->unk_98) break;
        if (AsGtlMediaFormat(left)->unk_9A != AsGtlMediaFormat(right)->unk_9A) break;
        if (AsGtlMediaFormat(left)->transformWorkspaceCount != AsGtlMediaFormat(right)->transformWorkspaceCount) break;
        if (AsGtlMediaFormat(left)->splitPacketHeaderWordCount != AsGtlMediaFormat(right)->splitPacketHeaderWordCount) break;
        return 1;
    } while (0);
    return 0;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline off
extern "C" {
// Construct a one-reference media sample around a borrowed buffer and format word.
void gtl_CMediaSample_Ctor(void* self, int data, int byteCount, int format) {
    AsGtlMediaSample(self)->references = 1;
    AsGtlMediaSample(self)->vtbl = (void*)__vt__Q23gtl12CMediaSample;
    AsGtlMediaSample(self)->format = format;
    AsGtlMediaSample(self)->data = data;
    AsGtlMediaSample(self)->size = byteCount;
}
}
#pragma pop
