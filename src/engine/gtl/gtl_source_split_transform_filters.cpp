// Audio filter nodes: a source filter that reads samples, a split filter with several outputs,
// and a transform filter.

#pragma push
#include "classes/GtlMediaSample.h"
#include "classes/GtlPagedQueue803F.h"
#include "classes/gtl_CSourceFilter.h"
#include "classes/gtl_CSplitFilter.h"
#include "classes/gtl_IFilter.h"
#include "classes/gtl_CTransformFilter.h"

extern "C" {

// Shared filter state/result codes; Process may succeed while waiting for input.
enum FilterState {
    kFilterReset = 0,
    kFilterRunning = 1,
    kFilterWaitingForInput = 2,
    kFilterStopped = 3,
    kFilterOutputReady = 4,
    kFilterFormatsRejected = 6,
    kFilterProcessFailed = 7
};
enum {
    kSourceReadBufferBytes = 0x40000,
    kSourceReadAlignment = 4,
    kStoredValueUnitShift = 7
};

struct CallbackObject {
    char baseStorage[4];
    virtual void v0();
    virtual void v1();
    virtual void resetState();
    virtual void v3();
    virtual void resetInput();
};

struct RefObjectBase {
    unsigned int refs;
};

struct TransformFilterVirtuals {
    char baseStorage[4];
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual int formatsMatch(void*, void*);
    virtual int allocateOutputBuffer(void*, void*);
    virtual int transformSample(void*, void*);
};

struct FilterVTable8808 {
    char baseStorage[4];
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual int formatsMatch(void*, void*);
    virtual int splitSample(void*, void*, void*);
};

struct RefObject : RefObjectBase {
    virtual void destroy(int deleting);
};

void* gtl_Alloc(unsigned int);

extern void gtl_Free(void*);
extern int gtl_MediaFormat_Equals(void*, void*);
extern int SamplePool_IsEmpty(void*);
extern int SamplePool_PeekAvailableSample(void*, void**);
extern void SamplePool_GetDimensions(void*, void*);
extern void SamplePool_CommitSample(void*, unsigned int);
extern int SamplePool_PeekAvailableSample(void*, void**);
extern void* gtl_Alloc(unsigned int);
extern char __vt__Q23gtl16CTransformFilter[];
extern char __vt__Q23gtl13CSourceFilter[];
extern char __vt__Q23gtl12CSplitFilter[];

struct SourceData {
    unsigned int formatKind;
    unsigned int byteLength;
    char formatPayload[0x84];
    unsigned int firstReadOffset;
};
} // extern "C"

#pragma pop

// Definitions follow the original text-address order.
extern "C" {} // extern "C"

#pragma push
// Clear queue counters and initialize an empty page-pointer vector.
void* GtlPagedQueue803F::Init(void*) {
    unsigned int* fields = (unsigned int*)this;
    fields[0] = 0;
    fields[1] = 0;
    fields[2] = 0;
    fields[3] = 0;
    fields[4] = 0;
    fields[5] = 0;
    struct RangeVector {
        unsigned int* begin;
        unsigned int* end;
        unsigned int* capacity;
    } empty = {0, 0, 0};
    RangeVector* destination = (RangeVector*)((char*)this + 8);
    unsigned int count = (unsigned int)(empty.end - empty.begin);
    if (count != 0) {
        unsigned int* storage = (unsigned int*)gtl_Alloc((count + 1) * 4);
        destination->begin = storage;
        destination->end = storage + count + 1;
        destination->capacity = destination->end;
    }
    return this;
}
#pragma pop

#pragma push
// Return zero for a stored zero, otherwise scale the stored value minus one by 128.
unsigned int GtlPagedQueue803F::GetStorageEntryCapacity() {
    return pages.size == 0 ? 0 : (pages.size - 1) << kStoredValueUnitShift;
}
#pragma pop

extern "C" {} // extern "C"

// Views of the object's second vtable (at offset 4). A 4-byte base comes before the vtable pointer.
struct Base4View {
    unsigned int unk;
};
struct CallbackAt4 : Base4View {
    virtual void v0();
    virtual void v1();
    virtual void resetState();
    virtual void v3();
    virtual void resetInput();
};
struct SourceReaderView : Base4View {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
    virtual int validateSource(void*, int);
    virtual int readSample(void*, void*, unsigned int*);
    virtual int getBytesRead(unsigned int*);
};

#pragma push
void* gtl::CSourceFilter::Init(const SourceData* sourceData, void* callbackObject) {
    ((gtl::IFilter*)this)->Init(0);
    vtable = __vt__Q23gtl13CSourceFilter;
    readOffset = 0;
    outputSample = 0;
    samplePool = 0;
    readPending = 0;
    callback = callbackObject;
    source = (void*)sourceData;
    sourceValidated = 0;
    readOffset = sourceData->firstReadOffset;
    return this;
}
#pragma pop

#pragma push
void* gtl::CSourceFilter::Destroy(int deleting) {
    if (this != 0) {
        ((gtl::IFilter*)this)->Destroy(0);
        if (deleting > 0 && this != 0)
            gtl_Free(this);
    }
    return this;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::Process() {
    int state;
    ((gtl::IFilter*)this)->GetState(&state);
    if (state == kFilterStopped || state == kFilterOutputReady)
        return 1;
    if (sourceValidated == 0) {
        if (!((SourceReaderView*)this)->validateSource(outputSample, 0)) {
            ((gtl::IFilter*)this)->SetState(kFilterFormatsRejected);
            return 0;
        }
        sourceValidated = 1;
    }
    if (((SourceData*)source)->byteLength <= readOffset) {
        ((gtl::IFilter*)this)->SetState(kFilterOutputReady);
        return 1;
    }
    if (samplePool == 0)
        return 0;
    if (SamplePool_IsEmpty(samplePool)) {
        ((gtl::IFilter*)this)->SetState(kFilterWaitingForInput);
        return 1;
    }
    void* object;
    *(unsigned int*)&object = 0;
    if (!SamplePool_PeekAvailableSample(samplePool, &object))
        return 1;
    unsigned int args[2];
    args[0] = kSourceReadBufferBytes;
    args[1] = kSourceReadAlignment;
    SamplePool_GetDimensions(samplePool, args);
    switch (((SourceReaderView*)this)->readSample(callback, object, args)) {
    case kFilterOutputReady: {
        unsigned int amount;
        ((SourceReaderView*)this)->getBytesRead(&amount);
        SamplePool_CommitSample(samplePool, amount);
        readOffset += amount;
        ((gtl::IFilter*)this)->SetState(kFilterRunning);
        break;
    }
    case kFilterRunning:
        break;
    default:
        ((gtl::IFilter*)this)->SetState(kFilterProcessFailed);
        int refs = --*(int*)object;
        if (refs == 0 && object != 0)
            ((RefObject*)object)->destroy(1);
        return 0;
    }
    int refs = --*(int*)object;
    if (refs == 0 && object != 0)
        ((RefObject*)object)->destroy(1);
    return 1;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::StartProcessing() {
    int value;
    ((gtl::IFilter*)this)->GetState(&value);
    if (value != kFilterOutputReady)
        ((gtl::IFilter*)this)->SetState(kFilterRunning);
    return 1;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::ResumeUnlessOutputReady() {
    int value;
    ((gtl::IFilter*)this)->GetState(&value);
    if (value != kFilterOutputReady)
        ((gtl::IFilter*)this)->SetState(kFilterRunning);
    return 1;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::ResetReadState() {
    readPending = 0;
    readOffset = ((SourceData*)source)->firstReadOffset;
    ((gtl::IFilter*)this)->SetState(kFilterReset);
    return 1;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::ResetAndStart() {
    ((CallbackAt4*)this)->resetInput();
    ((CallbackAt4*)this)->resetState();
    return 1;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::GetOutputSample(void* unused, unsigned int** out) {
    if (out == 0)
        return 0;
    unsigned int* value = (*(unsigned int**)&outputSample);
    *out = value;
    (*value)++;
    return 1;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::StoreInputSample(void* input) {
    unsigned int dataAddress;
    outputSample = input;
    ((gtl::CMediaSample*)input)->GetData((int*)&dataAddress);
    samplePool = (void*)dataAddress;
    return 1;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::vfunc_8(int value) {
    return value != 0;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::GetReadOffset(unsigned int* out) {
    if (out == 0)
        return 0;
    *out = readOffset;
    return 1;
}
#pragma pop

#pragma push
int gtl::CSourceFilter::IsReadPending() {
    return readPending;
}
#pragma pop

#pragma push
void gtl::CSourceFilter::MarkReadPending() {
    readPending = 1;
}
#pragma pop

#pragma push
void gtl::CSourceFilter::ClearReadPending() {
    readPending = 0;
}
#pragma pop

// Create a two-output filter and allocate both sample wrappers.
#pragma push
gtl::CSplitFilter* gtl::CSplitFilter::Init(void* input, void* arg2) {
    ((gtl::IFilter*)this)->Init(2);
    vtable = (void*)__vt__Q23gtl12CSplitFilter;
    inputSample = 0;
    formatsChecked = 0;
    void* first = gtl_Alloc(0x14);
    if (first != 0)
        ((gtl::CMediaSample*)first)->Init(0, 0, (int)input);
    primaryOutput = first;
    void* second = gtl_Alloc(0x14);
    if (second != 0)
        ((gtl::CMediaSample*)second)->Init(0, 0, (int)input);
    secondaryOutput = second;
    return this;
}
#pragma pop

#pragma push
void* gtl::CSplitFilter::Destroy(int deleting) {
    if (this != 0) {
        RefObject* input = (RefObject*)inputSample;
        vtable = (void*)__vt__Q23gtl12CSplitFilter;
        if (input != 0) {
            unsigned int refs = *(unsigned int*)input - 1;
            *(unsigned int*)input = refs;
            if (refs == 0)
                input->destroy(1);
            inputSample = 0;
        }
        RefObject* value = (RefObject*)primaryOutput;
        if (value != 0) {
            value->refs--;
            if (value->refs == 0)
                value->destroy(1);
            primaryOutput = 0;
        }
        value = (RefObject*)secondaryOutput;
        if (value != 0) {
            value->refs--;
            if (value->refs == 0)
                value->destroy(1);
            secondaryOutput = 0;
        }
        ((gtl::IFilter*)this)->Destroy(0);
        if (deleting > 0 && this != 0)
            gtl_Free(this);
    }
    return this;
}
#pragma pop

#pragma push
int gtl::CSplitFilter::Process() {
    int state;
    ((gtl::IFilter*)this)->GetState(&state);
    if (state == kFilterStopped || state == kFilterReset)
        return 1;
    void* input = inputSample;
    if (input == 0) {
        ((gtl::IFilter*)this)->SetState(kFilterWaitingForInput);
        return 1;
    }
    if (formatsChecked == 0) {
        void* first = primaryOutput;
        if (!((FilterVTable8808*)this)->formatsMatch(input, first) ||
            !((FilterVTable8808*)this)->formatsMatch(input, secondaryOutput)) {
            ((gtl::IFilter*)this)->SetState(kFilterFormatsRejected);
            return 0;
        }
        formatsChecked = 1;
    }
    int result = ((FilterVTable8808*)this)->splitSample(inputSample, primaryOutput, secondaryOutput);
    if (result != kFilterOutputReady) {
        ((gtl::IFilter*)this)->SetState(kFilterProcessFailed);
        return 0;
    }
    ((gtl::IFilter*)this)->SetState(kFilterOutputReady);
    return 1;
}
#pragma pop

#pragma push
int gtl::CSplitFilter::SetRunning() {
    ((gtl::IFilter*)this)->SetState(kFilterRunning);
    return 1;
}
#pragma pop

#pragma push
int gtl::CSplitFilter::Stop() {
    ((gtl::IFilter*)this)->SetState(kFilterStopped);
    return 1;
}
#pragma pop

#pragma push
int gtl::CSplitFilter::Reset() {
    void* value = inputSample;
    if (value != 0) {
        unsigned int count = *(unsigned int*)value - 1;
        *(unsigned int*)value = count;
        if (count == 0) {
            if (value != 0)
                ((RefObject*)value)->destroy(1);
        }
        inputSample = 0;
    }
    ((gtl::IFilter*)this)->SetState(kFilterReset);
    return 1;
}
#pragma pop

#pragma push
int gtl::CSplitFilter::ResetAndStart(CallbackObject* self) {
    self->resetInput();
    self->resetState();
    return 1;
}
#pragma pop

#pragma push
int gtl::CSplitFilter::GetOutputSample(int which, unsigned int** out) {
    if (out == 0)
        return 0;
    if (which == 0) {
        unsigned int* value = (*(unsigned int**)&primaryOutput);
        *out = value;
        (*value)++;
    } else if (which == 1) {
        unsigned int* value = (*(unsigned int**)&secondaryOutput);
        *out = value;
        (*value)++;
    }
    return 1;
}
#pragma pop

#pragma push
int gtl::CSplitFilter::SetInputSample(RefObject* value) {
    RefObject* old;
    old = (*(RefObject**)&inputSample);
    if (old != 0) {
        old->refs--;
        if (old->refs == 0)
            old->destroy(1);
        (*(RefObject**)&inputSample) = 0;
    }
    (*(RefObject**)&inputSample) = value;
    value->refs++;
    return 1;
}
#pragma pop

#pragma push
int gtl::CSplitFilter::FormatsMatch(void* left, void* right) {
    if (left == 0 || right == 0)
        return 0;
    // gtl::CMediaSample::CopyFormat fills a 0xA4-byte description.
    char leftData[0xa4];
    char rightData[0xa4];
    ((gtl::CMediaSample*)left)->CopyFormat(leftData);
    ((gtl::CMediaSample*)right)->CopyFormat(rightData);
    return gtl_MediaFormat_Equals(leftData, rightData);
}
#pragma pop

// Initialize the transform interface, empty input, output sample, and format-check latch.
#pragma push
gtl::CTransformFilter* gtl::CTransformFilter::Init(void* arg1, void* outputSampleArg) {
    ((gtl::IFilter*)this)->Init((unsigned int)arg1);
    inputSample = 0;
    vtable = __vt__Q23gtl16CTransformFilter;
    outputSample = (unsigned int*)outputSampleArg;
    formatsChecked = 0;
    return this;
}
#pragma pop

// Release the transform input reference, destroy the base filter, and optionally free the object.
#pragma push
void* gtl::CTransformFilter::Destroy(int deleting) {
    if (this != 0) {
        void* input = inputSample;
        vtable = __vt__Q23gtl16CTransformFilter;
        if (input != 0) {
            unsigned int refs = *(unsigned int*)input - 1;
            *(unsigned int*)input = refs;
            if (refs == 0)
                ((RefObject*)input)->destroy(1);
            inputSample = 0;
        }
        ((gtl::IFilter*)this)->Destroy(0);
        if (deleting > 0 && this != 0)
            gtl_Free(this);
    }
    return this;
}
#pragma pop

// Prepare an output buffer, validate formats once, and transform the pending input sample.
#pragma push
int gtl::CTransformFilter::Process() {
    int state;
    ((gtl::IFilter*)this)->GetState(&state);
    if (state == kFilterStopped || state == kFilterReset)
        return 1;
    unsigned int ready;
    ((gtl::CMediaSample*)outputSample)->GetData((int*)&ready);
    if (!ready) {
        char description[0xa4]; // gtl::CMediaSample::CopyFormat fills 0xA4 bytes
        ((gtl::CMediaSample*)outputSample)->CopyFormat(description);
        unsigned int second;
        ((TransformFilterVirtuals*)this)->allocateOutputBuffer(&ready, &second);
        ((gtl::CMediaSample*)outputSample)->SetBuffer(ready, second);
    }
    if (inputSample == 0) {
        ((gtl::IFilter*)this)->SetState(kFilterWaitingForInput);
        return 1;
    }
    if (formatsChecked == 0) {
        if (!((TransformFilterVirtuals*)this)->formatsMatch(inputSample, outputSample)) {
            ((gtl::IFilter*)this)->SetState(kFilterFormatsRejected);
            return 0;
        }
        formatsChecked = 1;
    }
    unsigned int pending;
    ((gtl::CMediaSample*)outputSample)->GetSize((int*)&pending);
    if (!pending)
        return 1;
    int result = ((TransformFilterVirtuals*)this)->transformSample(inputSample, outputSample);
    if (result == kFilterProcessFailed) {
        ((gtl::IFilter*)this)->SetState(kFilterProcessFailed);
        return 0;
    }
    ((gtl::IFilter*)this)->SetState(kFilterOutputReady);
    return 1;
}
#pragma pop

// Expose output after a stop, otherwise set the transform filter running.
#pragma push
int gtl::CTransformFilter::ResumeOrExposeOutput() {
    int value;
    ((gtl::IFilter*)this)->GetState(&value);
    if (value == kFilterStopped)
        ((gtl::IFilter*)this)->SetState(kFilterOutputReady);
    else
        ((gtl::IFilter*)this)->SetState(kFilterRunning);
    return 1;
}
#pragma pop
