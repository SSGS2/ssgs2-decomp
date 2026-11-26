// Media pipeline manager update loop: loading, stage transitions, output access and teardown.
// Also the global media registry that holds the pipelines and their helpers.
#include "classes/gtl_IFilter.h"
#include <decomp/local_class.h>
#include "classes/GtlShowCounter.h"
#include "classes/GtlMediaSample.h"
#include "classes/GtlMediaFormat.h"
#define AsGtlMediaFormat(p) ((GtlMediaFormat*)(p))
#include "classes/GtlMediaPipeline.h"
#include "classes/gtl_CRevolutionFilterManager.h"
#define AsGtlMediaPipeline(p) ((gtl::CFilterManager*)(p))
#include "classes/GtlMediaRegistryNode.h"
#define AsGtlMediaRegistryNode(p) ((GtlMediaRegistryNode*)(p))
#include "classes/GtlMediaRegistry.h"
#define AsGtlMediaRegistry(p) ((GtlMediaRegistry*)(p))

#include "sdk_protos.h"

extern "C" {

// Filter states used by the pipeline's source, transform, split and output stages.
enum PipelineFilterState {
    kFilterReset = 0,
    kFilterRunning = 1,
    kFilterWaitingForInput = 2,
    kFilterStopped = 3,
    kFilterOutputReady = 4
};
// Manager states and transition requests are separate domains from filter result states.
enum PipelineManagerState {
    kPipelineInactive0 = 0,
    kPipelineLoading = 1,
    kPipelineReady = 2,
    kPipelineInactive4 = 4
};
enum FilterTransitionCommand {
    kFilterCommandStart = 3,
    kFilterCommandSuspend = 5,
    kFilterCommandReset = 4
};
enum PipelineError {
    kSourceFailure = 7,
    kTransformOrSplitFailure = 8,
    kPrimaryOutputFailure = 9,
    kSecondaryOutputFailure = 10,
    kLoaderSetupFailure = 11
};
enum {
    kPipelineStageCount = 5,
    kMediaFormatNameBytes = 0x80
};

struct RegistryValueReference {
    void* allocatorState;
    void** value;
};

void gtl_Free(void*);
void* gtl_MediaRegistry_GetInstance(void*);
void gtl_MediaRegistry_RemoveByReference(void*, void*);
void* gtl_Alloc(unsigned int);
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

// Compare the format discriminator, name, timing and sample-layout fields.
int gtl_MediaFormat_Equals(const void* left, const void* right);

// Append a pipeline only if the registry does not already contain it.
int gtl_MediaRegistry_Add(void* registry, void* pipeline);

// Global registry storage: {count, sentinelPrevious, sentinelNext}.
// The sentinel address is &sentinelPrevious, and an empty list links both directions to it.
// Nodes own list storage only; their pipeline values are borrowed (see Add/Remove).
// The physical prefix uses the same field roles as GtlMediaRegistry.
struct RegNode {
    int count;
    void* sentinelPrevious;
    void* sentinelNext;
};
RegNode s_gtlMediaRegistry;
char s_mediaRegistryDtorRegistration[12];
signed char s_mediaRegistryInitialized;

// Free a non-null registry-node allocation.
void dtor_803F47D8(void* allocation);

// Store an integer through the supplied output/storage pointer.
void GtlIntStorage_SetValue(void* storage, int value);

// Free a non-null pipeline helper allocation.
void dtor_803F4E60(void* allocation);

// Free a non-null pipeline helper allocation.
void dtor_803F5310(void* allocation);

extern char __vt__Q23gtl12CMediaSample[];

#pragma auto_inline off
// Adapt a pipeline pointer to the registry's by-reference removal interface.
void gtl_MediaRegistry_Remove(void* registry, void* pipeline);
#pragma auto_inline on

// Lazily construct the global registry with an empty circular-list sentinel.
void* gtl_MediaRegistry_GetInstance(void* unused);

// Unlink and free registry nodes, then optionally free the registry object.
void gtl_MediaRegistry_Dtor(void* registry, int deleting);

#pragma auto_inline off
// Allocate a format and sample pool, then construct the sample wrapper and DVD loader.
int gtl_MediaPipeline_CreateSamplePoolAndDvdLoader(void* self);
#pragma auto_inline on

// Create the platform filter manager, or report availability without allocating in probe mode.
int gtl_FilterManagerFactory_Create(void* factory, void* outManager, int probeOnly);

// Remove and free every registry node holding the referenced pipeline pointer.
void gtl_MediaRegistry_RemoveByReference(void* registry, void* valueReference);

// Update registered pipelines in list order; return zero at the first failure.
int gtl_MediaRegistry_UpdateAll(void* registry);

} // extern "C"

// Function definitions follow the original address order.

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Return an added primary-sample reference only while the primary filter has usable output.
int gtl::CFilterManager::GetPrimaryOutput(void* output) {
    int zero = 0;
    void* out = output;
    // The output is cleared before the checks, so callers must pass writable storage.
    *(int*)out = zero;
    if (out == 0 || output0 == 0 || stage3 == 0) {
        return 0;
    }
    int filterState;
    ((gtl::IFilter*)stage3)->GetState(&filterState);
    if (filterState != kFilterOutputReady && filterState != kFilterStopped) {
        *(int*)out = zero;
        return 0;
    }
    void* ref = output0;
    *(void**)out = ref;
    ++*(int*)ref;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Return an added secondary-sample reference only while the secondary filter is output-ready.
int gtl::CFilterManager::GetSecondaryOutput(void* output) {
    int zero = 0;
    void* out = output;
    // The output is cleared before the checks, so callers must pass writable storage.
    *(int*)out = zero;
    if (out == 0 || output1 == 0 || stage4 == 0) {
        return 0;
    }
    int filterState;
    ((gtl::IFilter*)stage4)->GetState(&filterState);
    if (filterState != kFilterOutputReady) {
        *(int*)out = zero;
        return 0;
    }
    void* ref = output1;
    *(void**)out = ref;
    ++*(int*)ref;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Replace the primary sample buffer, freeing old data only when internally owned.
void gtl::CFilterManager::SetOutputBuffer(int buffer) {
    int byteCount;
    ((gtl::CMediaSample*)output0)->GetSize((int*)&byteCount);
    if (outputBuffer == 0) {
        int oldData;
        ((gtl::CMediaSample*)output0)->GetData((int*)&oldData);
        if (oldData != 0) {
            gtl_Free((void*)oldData);
            oldData = 0;
        }
    }
    outputBuffer = buffer;
    ((gtl::CMediaSample*)output0)->SetBuffer(buffer, byteCount);
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Update each pipeline stage, transfer temporary sample references, and report stage failures.
int gtl::CFilterManager::Update() {
    struct Obj {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual int v1() DECOMP_LOCAL_VIRTUAL;
        virtual void v2() DECOMP_LOCAL_VIRTUAL;
        virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL;
        virtual void v5() DECOMP_LOCAL_VIRTUAL;
        virtual void ReadRestartControlSlot6(int*) DECOMP_LOCAL_VIRTUAL;
        virtual void v7() DECOMP_LOCAL_VIRTUAL;
        virtual int HandleEndOfInputSlot8() DECOMP_LOCAL_VIRTUAL;
        virtual void v9() DECOMP_LOCAL_VIRTUAL;
        virtual void v10() DECOMP_LOCAL_VIRTUAL;
        virtual void v11() DECOMP_LOCAL_VIRTUAL;
        virtual void v12() DECOMP_LOCAL_VIRTUAL;
        virtual void v13() DECOMP_LOCAL_VIRTUAL;
        virtual void v14() DECOMP_LOCAL_VIRTUAL;
        virtual void v15() DECOMP_LOCAL_VIRTUAL;
        virtual void v16() DECOMP_LOCAL_VIRTUAL;
        virtual void v17() DECOMP_LOCAL_VIRTUAL;
        virtual void v18() DECOMP_LOCAL_VIRTUAL;
        virtual void v19() DECOMP_LOCAL_VIRTUAL;
        virtual void v20() DECOMP_LOCAL_VIRTUAL;
        virtual void ReportStateOrErrorSlot21(int) DECOMP_LOCAL_VIRTUAL;
        virtual void v22(int) DECOMP_LOCAL_VIRTUAL;
        virtual void v23(int) DECOMP_LOCAL_VIRTUAL;
        virtual int v24() DECOMP_LOCAL_VIRTUAL;
        virtual void v25() DECOMP_LOCAL_VIRTUAL;
        virtual void v26() DECOMP_LOCAL_VIRTUAL;
        virtual void v27() DECOMP_LOCAL_VIRTUAL;
        virtual int IsEndOfInputSlot28() DECOMP_LOCAL_VIRTUAL;
        virtual void v29(int) DECOMP_LOCAL_VIRTUAL;
        virtual void v30() DECOMP_LOCAL_VIRTUAL;
        virtual void v31() DECOMP_LOCAL_VIRTUAL;
    };
    struct Iface {
        char pad[4];
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual int UpdateSlot1() DECOMP_LOCAL_VIRTUAL;
        virtual void v2() DECOMP_LOCAL_VIRTUAL;
        virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL;
        virtual void SourceCompletionSlot5() DECOMP_LOCAL_VIRTUAL;
        virtual void SubmitInputSlot6(int) DECOMP_LOCAL_VIRTUAL;
        virtual void AcquireOutputSlot7(int, void**) DECOMP_LOCAL_VIRTUAL;
        virtual void v8() DECOMP_LOCAL_VIRTUAL;
        virtual void v9() DECOMP_LOCAL_VIRTUAL;
        virtual void v10() DECOMP_LOCAL_VIRTUAL;
    };
    struct Ref {
        int count;
        virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
    }; // count, then the class table (destroy is its first entry)
    struct State {
        int value;
    };
    // Each union preserves one original address-taken stack slot while naming its
    // two successive domains. Count members are dead before state/control writes.
    int transformState;
    union {
        int showCount;
        int sourceState;
    } sourceQuery;
    union {
        int previousShowCount;
        int restartControl;
    } controlQuery;
    int nextTransformState;
    volatile int frame[8];
    // Phase 1: skip inactive managers, progress loading, then require all five stages.
    int managerState = state;
    if (managerState == kPipelineInactive4 || managerState == kPipelineInactive0)
        return 1;
    if (!UpdateLoading())
        return 0;
    if (!stage0 || !stage1 || !stage2 || !stage3 || !stage4)
        return 1;
    if (((Obj*)this)->IsEndOfInputSlot28())
        return ((Obj*)this)->HandleEndOfInputSlot8();
    else {
        void* p = showCounter;
        if (p) {
            struct Other {
                char pad[4];
                virtual void f0() DECOMP_LOCAL_VIRTUAL;
                virtual void f1() DECOMP_LOCAL_VIRTUAL;
                virtual void f2() DECOMP_LOCAL_VIRTUAL;
                virtual void f3() DECOMP_LOCAL_VIRTUAL;
                virtual void f4() DECOMP_LOCAL_VIRTUAL;
                virtual void f5(float, float) DECOMP_LOCAL_VIRTUAL;
            };
            ((Other*)p)->f5(AsGtlMediaFormat(format)->showCounterFormatParameter, showCounterParameter);
        }
    }
    // Phase 2: synchronize transitions with the show counter when transform output
    // is stopped/ready. Equal counts suspend; a changed count starts the filters.
    ((gtl::IFilter*)stage1)->GetState(&transformState);
    if (transformState == kFilterStopped || transformState == kFilterOutputReady) {
        ((gtl::IShowCounter*)showCounter)->GetCount(&sourceQuery.showCount);
        ((gtl::IShowCounter*)showCounter)->GetPreviousCount(&controlQuery.previousShowCount);
        TransitionFilters(sourceQuery.showCount == controlQuery.previousShowCount ? kFilterCommandSuspend
                                                                                  : kFilterCommandStart);
    }
    // Phase 3: update source, acquire port 0 and submit to transform. End-of-source
    // processing then overwrites the former show-count slots with state/control.
    void* sourceFilter = stage0;
    void* transformFilter = stage1;
    if (!((Iface*)sourceFilter)->UpdateSlot1()) {
        ((Obj*)this)->ReportStateOrErrorSlot21(kSourceFailure);
        return 0;
    }
    {
        void* value = 0;
        ((Iface*)sourceFilter)->AcquireOutputSlot7(0, &value);
        ((Iface*)transformFilter)->SubmitInputSlot6((int)value);
        if (value) {
            Ref* r = (Ref*)value;
            --r->count;
            if (r->count == 0) {
                if (value)
                    r->destroy(1);
            }
            value = 0;
        }
    }
    ((gtl::IFilter*)sourceFilter)->GetState(&sourceQuery.sourceState);
    if (sourceQuery.sourceState == kFilterOutputReady) {
        ((Obj*)this)->ReadRestartControlSlot6(&controlQuery.restartControl);
        if (controlQuery.restartControl == 1)
            ((Iface*)sourceFilter)->SourceCompletionSlot5();
    }
    // Phase 4: transform update; forward port 0 only when output-ready.
    if (!((Iface*)transformFilter)->UpdateSlot1()) {
        ((Obj*)this)->ReportStateOrErrorSlot21(kTransformOrSplitFailure);
        return 0;
    }
    ((gtl::IFilter*)transformFilter)->GetState(&nextTransformState);
    if (nextTransformState == kFilterOutputReady) {
        void* value = 0;
        ((Iface*)transformFilter)->AcquireOutputSlot7(0, &value);
        ((Iface*)stage2)->SubmitInputSlot6((int)value);
        if (value) {
            Ref* r = (Ref*)value;
            --r->count;
            if (r->count == 0) {
                if (value)
                    r->destroy(1);
            }
            value = 0;
        }
    }
    // (7), transform/split (8), primary (9), and secondary (10) failure.
    if (!ForwardSplitOutputs(stage2, stage3, stage4)) {
        ((Obj*)this)->ReportStateOrErrorSlot21(kTransformOrSplitFailure);
        return 0;
    }
    if (!((Iface*)stage3)->UpdateSlot1()) {
        ((Obj*)this)->ReportStateOrErrorSlot21(kPrimaryOutputFailure);
        return 0;
    }
    if (!((Iface*)stage4)->UpdateSlot1()) {
        ((Obj*)this)->ReportStateOrErrorSlot21(kSecondaryOutputFailure);
        return 0;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Unregister the pipeline before invoking its deleting destructor.
void gtl::CFilterManager::UnregisterAndDelete() {
    void* registry = gtl_MediaRegistry_GetInstance(this);
    gtl_MediaRegistry_Remove(registry, this);
    if (this != 0) {
        struct Base {
            virtual void v2(int) DECOMP_LOCAL_VIRTUAL;
        };
        ((Base*)this)->v2(1);
    }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Release the show-counter reference and clear the pipeline alias.
void gtl::CFilterManager::ReleaseShowCounter() {
    void* counter = showCounter;
    if (counter != 0) {
        if (--(*(int*)counter) == 0 && counter != 0) {
            struct Prefix {
                char pad[4];
            };
            struct Ref : Prefix {
                virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
            };
            ((Ref*)counter)->destroy(1);
        }
        showCounter = 0;
    }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Store the pipeline manager state and return success.
int gtl::CFilterManager::SetState(int newState) {
    state = newState;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Test for source completion while the transform waits for more input.
int gtl::CFilterManager::vfunc_28() {
    int sourceState;
    int transformState;
    ((gtl::IFilter*)stage0)->GetState(&sourceState);
    ((gtl::IFilter*)stage1)->GetState(&transformState);
    return sourceState == kFilterOutputReady && transformState == kFilterWaitingForInput;
}
#pragma pop

extern "C" {

#pragma push
#pragma dont_inline on
#pragma auto_inline off
// Allocate format/pool storage and construct the sample wrapper and DVD loader.
int gtl_MediaPipeline_CreateSamplePoolAndDvdLoader(void* self) {
    if (AsGtlMediaPipeline(self)->format != 0) {
        gtl_Free(AsGtlMediaPipeline(self)->format);
        AsGtlMediaPipeline(self)->format = 0;
    }
    AsGtlMediaPipeline(self)->format = gtl_Alloc(sizeof(GtlMediaFormat));
    strncpy(AsGtlMediaFormat(AsGtlMediaPipeline(self)->format)->name, AsGtlMediaPipeline(self)->pendingName,
            kMediaFormatNameBytes);
    (*(void**)&AsGtlMediaFormat(AsGtlMediaPipeline(self)->format)->sourceByteLength) =
        AsGtlMediaPipeline(self)->sourceByteLengthWord;
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
    if (poolSample != 0)
        ((gtl::CMediaSample*)poolSample)
            ->Init((int)AsGtlMediaPipeline(self)->samplePool, 0, (int)AsGtlMediaPipeline(self)->format);
    AsGtlMediaPipeline(self)->poolSample = poolSample;
    if (AsGtlMediaPipeline(self)->loader != 0)
        gtl_MediaDvdRead_Dtor(AsGtlMediaPipeline(self)->loader, 1);
    struct Base {
        virtual void f0() DECOMP_LOCAL_VIRTUAL;
        virtual void f1() DECOMP_LOCAL_VIRTUAL;
        virtual void f2() DECOMP_LOCAL_VIRTUAL;
        virtual void f3() DECOMP_LOCAL_VIRTUAL;
        virtual void f4() DECOMP_LOCAL_VIRTUAL;
        virtual void f5() DECOMP_LOCAL_VIRTUAL;
        virtual void f6() DECOMP_LOCAL_VIRTUAL;
        virtual void f7() DECOMP_LOCAL_VIRTUAL;
        virtual void f8() DECOMP_LOCAL_VIRTUAL;
        virtual void f9() DECOMP_LOCAL_VIRTUAL;
        virtual void f10() DECOMP_LOCAL_VIRTUAL;
        virtual void f11() DECOMP_LOCAL_VIRTUAL;
        virtual void f12() DECOMP_LOCAL_VIRTUAL;
        virtual void f13() DECOMP_LOCAL_VIRTUAL;
        virtual void f14() DECOMP_LOCAL_VIRTUAL;
        virtual void f15() DECOMP_LOCAL_VIRTUAL;
        virtual void f16() DECOMP_LOCAL_VIRTUAL;
        virtual void f17() DECOMP_LOCAL_VIRTUAL;
        virtual void f18() DECOMP_LOCAL_VIRTUAL;
        virtual void f19() DECOMP_LOCAL_VIRTUAL;
        virtual void f20() DECOMP_LOCAL_VIRTUAL;
        virtual void f21() DECOMP_LOCAL_VIRTUAL;
        virtual void f22(void**, void*) DECOMP_LOCAL_VIRTUAL;
    };
    void* result;
    ((Base*)self)->f22(&result, AsGtlMediaPipeline(self)->format);
    void* loader = gtl_Alloc(0x10);
    if (loader != 0)
        gtl_MediaDvdRead_Ctor(loader, result, AsGtlMediaPipeline(self)->inputFile);
    AsGtlMediaPipeline(self)->loader = loader;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Free a non-null registry-node allocation.
void dtor_803F47D8(void* allocation) {
    if (allocation != 0) {
        gtl_Free(allocation);
    }
}
#pragma pop

} // extern "C"

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Poll DVD loading, configure the filters, and finish any deferred start.
int gtl::CFilterManager::UpdateLoading() {
    // Phase 1: create the loader and pool for a loading pipeline that has no format yet.
    struct Obj {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual void v2() DECOMP_LOCAL_VIRTUAL;
        virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL;
        virtual void v5() DECOMP_LOCAL_VIRTUAL;
        virtual void v6() DECOMP_LOCAL_VIRTUAL;
        virtual void v7() DECOMP_LOCAL_VIRTUAL;
        virtual void v8() DECOMP_LOCAL_VIRTUAL;
        virtual void v9() DECOMP_LOCAL_VIRTUAL;
        virtual void v10() DECOMP_LOCAL_VIRTUAL;
        virtual void v11() DECOMP_LOCAL_VIRTUAL;
        virtual void v12() DECOMP_LOCAL_VIRTUAL;
        virtual void v13() DECOMP_LOCAL_VIRTUAL;
        virtual void v14() DECOMP_LOCAL_VIRTUAL;
        virtual void v15() DECOMP_LOCAL_VIRTUAL;
        virtual void v16() DECOMP_LOCAL_VIRTUAL;
        virtual void v17() DECOMP_LOCAL_VIRTUAL;
        virtual void v18() DECOMP_LOCAL_VIRTUAL;
        virtual void v19() DECOMP_LOCAL_VIRTUAL;
        virtual void v20() DECOMP_LOCAL_VIRTUAL;
        virtual void ReportStateOrErrorSlot21(int) DECOMP_LOCAL_VIRTUAL;
        virtual void v22() DECOMP_LOCAL_VIRTUAL;
        virtual void PostLoadHookSlot23() DECOMP_LOCAL_VIRTUAL;
        virtual void v24() DECOMP_LOCAL_VIRTUAL;
        virtual void v25() DECOMP_LOCAL_VIRTUAL;
        virtual void v26() DECOMP_LOCAL_VIRTUAL;
        virtual void ConfigureLoadedHeaderSlot27(int, int) DECOMP_LOCAL_VIRTUAL;
        virtual void v28() DECOMP_LOCAL_VIRTUAL;
        virtual void v29() DECOMP_LOCAL_VIRTUAL;
    };
    struct Holder {
        char pad[4];
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual void v2() DECOMP_LOCAL_VIRTUAL;
        virtual void v3() DECOMP_LOCAL_VIRTUAL;
        virtual void v4() DECOMP_LOCAL_VIRTUAL;
        virtual void v5() DECOMP_LOCAL_VIRTUAL;
        virtual void SubmitInputSlot6(int) DECOMP_LOCAL_VIRTUAL;
        virtual void v7() DECOMP_LOCAL_VIRTUAL;
        virtual void v8() DECOMP_LOCAL_VIRTUAL;
    };
    struct Handle {
        char pad[4];
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual int UpdateSlot1() DECOMP_LOCAL_VIRTUAL;
        virtual void StartSlot2() DECOMP_LOCAL_VIRTUAL;
    };
    int loadedHeader;
    if ((*(int*)&format) == 0 && state == kPipelineLoading) {
        DVDGetCommandBlockStatus(inputFile);
        if (!gtl_MediaPipeline_CreateSamplePoolAndDvdLoader(this)) {
            ((Obj*)this)->ReportStateOrErrorSlot21(kLoaderSetupFailure);
            return 0;
        }
    }
    // Phase 2: poll the DVD loader; configure filters only after its header arrives.
    if (loader) {
        int ok;
        if (!gtl_MediaDvdRead_Update(loader))
            ok = 0;
        else {
            if (gtl_MediaDvdRead_GetData(loader, &loadedHeader) == 1) {
                ((Obj*)this)->ConfigureLoadedHeaderSlot27(loadedHeader, (*(int*)&format));
                ((gtl::CMediaSample*)output0)->SetFormat(format);
                ((gtl::CMediaSample*)output1)->SetFormat(format);
                void* dvdLoader = loader;
                if (dvdLoader) {
                    gtl_MediaDvdRead_Dtor(dvdLoader, 1);
                    loader = 0;
                }
                ((Obj*)this)->PostLoadHookSlot23();
                ((Holder*)stage0)->SubmitInputSlot6((*(int*)&poolSample));
            }
            ok = 1;
        }
        if (!ok) {
            ((Obj*)this)->ReportStateOrErrorSlot21(kSourceFailure);
            return 0;
        }
        // Loading is complete when all stages and the show counter exist and the loader is gone.
        int valid = stage0 && stage1 && stage2 && stage3 && stage4 && !loader && showCounter;
        if (valid) {
            if (startPending)
                ((Handle*)stage0)->StartSlot2();
            else
                ((Obj*)this)->ReportStateOrErrorSlot21(kPipelineReady);
        }
    }
    // Phase 3: drive the deferred source start until it reaches waiting-for-input.
    if (startPending && stage0) {
        if (!((Handle*)stage0)->UpdateSlot1()) {
            ((Obj*)this)->ReportStateOrErrorSlot21(kSourceFailure);
            return 0;
        }
        int filterState;
        ((gtl::IFilter*)stage0)->GetState(&filterState);
        if (filterState == kFilterWaitingForInput) {
            ((Obj*)this)->ReportStateOrErrorSlot21(kPipelineReady);
            startPending = 0;
        }
    }
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Issue a supported transition command to all five filters in order.
int gtl::CFilterManager::TransitionFilters(int requestedState) {
    struct Table {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual int v2() DECOMP_LOCAL_VIRTUAL;
        virtual int v3() DECOMP_LOCAL_VIRTUAL;
        virtual int v4() DECOMP_LOCAL_VIRTUAL;
    };
    struct Holder {
        char pad[4];
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual int StartSlot2() DECOMP_LOCAL_VIRTUAL;
        virtual int SuspendSlot3() DECOMP_LOCAL_VIRTUAL;
        virtual int ResetSlot4() DECOMP_LOCAL_VIRTUAL;
    };
    void* stageCursor = this;
    int stageIndex = 0;
    int result;
    while (stageIndex < kPipelineStageCount) {
        void* filter = ((gtl::CFilterManager*)stageCursor)->stage0;
        if (filter == 0)
            return 0;
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
        if (result == 0)
            return 0;
        ++stageIndex;
        stageCursor = (char*)stageCursor + 4;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Return the pipeline source-restart control word through caller storage.
int gtl::CFilterManager::GetSourceRestartControl(int* outControlValue) {
    *outControlValue = sourceRestartControl;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
int gtl::CFilterManager::ForwardSplitOutputs(void* splitFilter, void* primarySink, void* secondarySink) {
    struct Iface {
        char pad[4];
        virtual void f0() DECOMP_LOCAL_VIRTUAL;
        virtual int UpdateSlot1() DECOMP_LOCAL_VIRTUAL;
        virtual void f2() DECOMP_LOCAL_VIRTUAL;
        virtual void f3() DECOMP_LOCAL_VIRTUAL;
        virtual void f4() DECOMP_LOCAL_VIRTUAL;
        virtual void f5() DECOMP_LOCAL_VIRTUAL;
        virtual void f6() DECOMP_LOCAL_VIRTUAL;
        virtual void AcquireOutputSlot7(int, void**) DECOMP_LOCAL_VIRTUAL;
    };
    struct Sink {
        char pad[4];
        virtual void f0() DECOMP_LOCAL_VIRTUAL;
        virtual void f1() DECOMP_LOCAL_VIRTUAL;
        virtual void f2() DECOMP_LOCAL_VIRTUAL;
        virtual void f3() DECOMP_LOCAL_VIRTUAL;
        virtual void f4() DECOMP_LOCAL_VIRTUAL;
        virtual void f5() DECOMP_LOCAL_VIRTUAL;
        virtual void SubmitInputSlot6(void*) DECOMP_LOCAL_VIRTUAL;
    };
    struct RefBase {
        int count;
    };
    struct Ref : RefBase {
        virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
    };
    int splitState;
    if (!((Iface*)splitFilter)->UpdateSlot1())
        return 0;
    ((gtl::IFilter*)splitFilter)->GetState(&splitState);
    if (splitState == kFilterOutputReady) {
        void* outputSample = 0;
        ((Iface*)splitFilter)->AcquireOutputSlot7(0, (void**)&outputSample);
        ((Sink*)primarySink)->SubmitInputSlot6(outputSample);
        if (outputSample != 0 && --((Ref*)outputSample)->count == 0)
            ((Ref*)outputSample)->destroy(1);
        outputSample = 0;
        ((Iface*)splitFilter)->AcquireOutputSlot7(1, (void**)&outputSample);
        ((Sink*)secondarySink)->SubmitInputSlot6(outputSample);
        if (outputSample != 0 && --((Ref*)outputSample)->count == 0)
            ((Ref*)outputSample)->destroy(1);
    }
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Run virtual cleanup hooks, then release samples, pool, format, loader and outputs.
void gtl::CFilterManager::ReleaseResources() {
    struct V {
        virtual void f0() DECOMP_LOCAL_VIRTUAL;
        virtual void f1() DECOMP_LOCAL_VIRTUAL;
        virtual void f2() DECOMP_LOCAL_VIRTUAL;
        virtual void f3() DECOMP_LOCAL_VIRTUAL;
        virtual void f4() DECOMP_LOCAL_VIRTUAL;
        virtual void f5() DECOMP_LOCAL_VIRTUAL;
        virtual void f6() DECOMP_LOCAL_VIRTUAL;
        virtual void f7() DECOMP_LOCAL_VIRTUAL;
        virtual void f8() DECOMP_LOCAL_VIRTUAL;
        virtual void f9() DECOMP_LOCAL_VIRTUAL;
        virtual void f10() DECOMP_LOCAL_VIRTUAL;
        virtual void f11() DECOMP_LOCAL_VIRTUAL;
        virtual void f12() DECOMP_LOCAL_VIRTUAL;
        virtual void f13() DECOMP_LOCAL_VIRTUAL;
        virtual void f14() DECOMP_LOCAL_VIRTUAL;
        virtual void f15() DECOMP_LOCAL_VIRTUAL;
        virtual void f16() DECOMP_LOCAL_VIRTUAL;
        virtual void f17() DECOMP_LOCAL_VIRTUAL;
        virtual void f18() DECOMP_LOCAL_VIRTUAL;
        virtual void f19() DECOMP_LOCAL_VIRTUAL;
        virtual void f20() DECOMP_LOCAL_VIRTUAL;
        virtual void f21() DECOMP_LOCAL_VIRTUAL;
        virtual void f22() DECOMP_LOCAL_VIRTUAL;
        virtual void f23() DECOMP_LOCAL_VIRTUAL;
        virtual void f24() DECOMP_LOCAL_VIRTUAL;
        virtual void f25() DECOMP_LOCAL_VIRTUAL;
        virtual void f26() DECOMP_LOCAL_VIRTUAL;
        virtual void f27() DECOMP_LOCAL_VIRTUAL;
        virtual void f28() DECOMP_LOCAL_VIRTUAL;
    };
    struct RefBase {
        int count;
    };
    struct Ref : RefBase {
        virtual void destroy(int) DECOMP_LOCAL_VIRTUAL;
    };
    int zero = 0;
    // Phase 1: run the cleanup hooks (slots 24 and 26) while every resource is still live.
    ((V*)this)->f24();
    ((V*)this)->f26();
    // Phase 2: drop sample references, destroy pool/loader, then clear output refs.
    void* resource = poolSample;
    if (resource != 0 && --((Ref*)resource)->count == 0)
        ((Ref*)resource)->destroy(1);
    poolSample = (void*)zero;
    resource = samplePool;
    if (resource != 0) {
        SamplePool_Dtor(resource, 1);
        samplePool = (void*)zero;
    }
    resource = format;
    if (resource != 0) {
        gtl_Free(resource);
        format = (void*)zero;
    }
    resource = loader;
    if (resource != 0) {
        gtl_MediaDvdRead_Dtor(resource, 1);
        loader = (void*)zero;
    }
    resource = output0;
    if (resource != 0 && --((Ref*)resource)->count == 0)
        ((Ref*)resource)->destroy(1);
    output0 = (void*)zero;
    resource = output1;
    if (resource != 0 && --((Ref*)resource)->count == 0)
        ((Ref*)resource)->destroy(1);
    output1 = (void*)zero;
}
#pragma pop

extern "C" {

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Free a non-null pipeline helper allocation.
void dtor_803F4E60(void* allocation) {
    if (allocation != 0) {
        gtl_Free(allocation);
    }
}
#pragma pop

} // extern "C"

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Copy the pipeline timing-control scalar to caller storage.
int gtl::CFilterManager::GetShowCounterParameter(float* outTimingValue) {
    *outTimingValue = showCounterParameter;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Store the pipeline control word without interpreting caller policy.
int gtl::CFilterManager::SetSourceRestartControl(int controlValue) {
    sourceRestartControl = controlValue;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Copy the current pipeline manager state to caller storage.
int gtl::CFilterManager::GetState(int* outState) {
    *outState = state;
    return 1;
}
#pragma pop

extern "C" {

#pragma push
#pragma dont_inline on
#pragma auto_inline on
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
        if (deleting > 0)
            gtl_Free(registry);
    }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
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
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Append a borrowed pipeline pointer only when it is absent from the registry.
int gtl_MediaRegistry_Add(void* registry, void* pipeline) {
    void* sentinel = &AsGtlMediaRegistry(registry)->sentinelPrevious;
    void* cursor = AsGtlMediaRegistry(registry)->sentinelNext;
    while (cursor != sentinel && AsGtlMediaRegistryNode(cursor)->value != pipeline) {
        cursor = AsGtlMediaRegistryNode(cursor)->next;
    }
    if (cursor != sentinel)
        return 0;
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
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Update registered pipelines in list order, stopping on the first failure.
int gtl_MediaRegistry_UpdateAll(void* registry) {
    void* cursor;
    void* sentinel;
    sentinel = &AsGtlMediaRegistry(registry)->sentinelPrevious;
    cursor = AsGtlMediaRegistry(registry)->sentinelNext;
    while (cursor != sentinel) {
        if (!AsGtlMediaPipeline(AsGtlMediaRegistryNode(cursor)->value)->Update())
            return 0;
        cursor = AsGtlMediaRegistryNode(cursor)->next;
    }
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline off
// Adapt a pipeline pointer to the by-reference registry removal ABI.
void gtl_MediaRegistry_Remove(void* registry, void* pipeline) {
    struct Frame {
        void* slot;
        char pad[4];
        void* t;
        void** pslot;
        struct {
            void* first;
            void** second;
        } r;
    } f;
    f.slot = pipeline;
    f.pslot = &f.slot;
    f.r.first = f.t;
    f.r.second = f.pslot;
    gtl_MediaRegistry_RemoveByReference(registry, &f.r);
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
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
            while (scan != sentinel && AsGtlMediaRegistryNode(scan)->value == key)
                scan = AsGtlMediaRegistryNode(scan)->next;
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
            if (cur == sentinel)
                break;
        } else {
            cur = AsGtlMediaRegistryNode(cur)->next;
        }
    }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Store an integer through the borrowed output pointer.
void GtlIntStorage_SetValue(void* storage, int value) {
    *(int*)storage = value;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline on
// Probe availability or allocate the platform filter manager.
int gtl_FilterManagerFactory_Create(void* factory, void* outManager, int probeOnly) {
    void* manager;
    if (outManager == 0 || *(void**)factory == 0)
        return 0;
    if (probeOnly != 0)
        return 1;
    manager = gtl_Alloc(0xf0);
    if (manager != 0) {
        ((gtl::CRevolutionFilterManager*)manager)->Init(*(void**)factory);
    }
    *(void**)outManager = manager;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Free a non-null pipeline helper allocation.
void dtor_803F5310(void* allocation) {
    if (allocation != 0) {
        gtl_Free(allocation);
    }
}
#pragma pop

#pragma push
#pragma dont_inline on
#pragma auto_inline reset
// Compare format discriminator, name, timing and sample-layout fields.
int gtl_MediaFormat_Equals(const void* left, const void* right) {
    const int* leftWords = (const int*)left;
    const int* rightWords = (const int*)right;
    int leftKind = *leftWords;
    int rightKind = *rightWords;
    // Original 0x803F3F84 unit: preserve the shared mismatch return.
    do {
        if (leftKind != rightKind)
            break;
        if (AsGtlMediaFormat(left)->sourceByteLength != AsGtlMediaFormat(right)->sourceByteLength)
            break;
        if (strcmp(AsGtlMediaFormat(left)->name, AsGtlMediaFormat(right)->name) != 0)
            break;
        if (AsGtlMediaFormat(left)->showCounterFormatParameter != AsGtlMediaFormat(right)->showCounterFormatParameter)
            break; // rate passed to the show counter
        if (AsGtlMediaFormat(left)->initialReadOffset != AsGtlMediaFormat(right)->initialReadOffset)
            break;
        if (AsGtlMediaFormat(left)->sampleBufferSize != AsGtlMediaFormat(right)->sampleBufferSize)
            break;
        // Compare both 16-bit sample-format attributes without guessing their units or dimensions.
        if (AsGtlMediaFormat(left)->unk_98 != AsGtlMediaFormat(right)->unk_98)
            break;
        if (AsGtlMediaFormat(left)->unk_9A != AsGtlMediaFormat(right)->unk_9A)
            break;
        if (AsGtlMediaFormat(left)->transformWorkspaceCount != AsGtlMediaFormat(right)->transformWorkspaceCount)
            break;
        if (AsGtlMediaFormat(left)->splitPacketHeaderWordCount != AsGtlMediaFormat(right)->splitPacketHeaderWordCount)
            break;
        return 1;
    } while (0);
    return 0;
}
#pragma pop

} // extern "C"

#pragma push
#pragma dont_inline on
#pragma auto_inline off
// Set up a one-reference media sample around a borrowed buffer and format word.
void gtl::CMediaSample::Init(int data, int byteCount, int format) {
    references = 1;
    vtbl = (void*)__vt__Q23gtl12CMediaSample;
    this->format = format;
    this->data = data;
    size = byteCount;
}
#pragma pop
