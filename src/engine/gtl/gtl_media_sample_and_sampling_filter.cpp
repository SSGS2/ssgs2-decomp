// Media samples and sampling filters: a sample holds a buffer, its byte count and a format.
// The sampling filter queues input samples and has its subclass process them one at a time.
// It also owns a reusable output sample and the start and stop requests.

#include "classes/gtl_IFilter.h"
#include "classes/GtlOwnedBuffer803F5428.h"
#include "classes/gtl_CSamplingFilter.h"
#include "classes/GtlMediaSample.h"
#include "classes/GtlMediaFormat.h"
#define AsGtlMediaFormat(p) ((GtlMediaFormat*)(p))

extern "C" {
#include "sdk_protos.h"
void* (*g_gtlRuntimeConfig[4])(unsigned int);
void* gtl_Alloc(unsigned int);
void* gtl_Free(void*);

struct RefCountBase {
    int refs;
    virtual void destroy(int);
};
struct SampleOwner {
    char pad[4];
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
};
struct SamplingFilterPrefix {
    int refs;
};
struct SamplingFilterCalls : SamplingFilterPrefix {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual int start(int, int);
    virtual int process(int);
};
extern void* __vt__Q23gtl15CSamplingFilter[4];
enum FilterState {
    kFilterIdle = 0,
    kFilterStartRequested = 1,
    kFilterWaitingForInput = 2,
    kFilterStopped = 3,
    kFilterSampleComplete = 4,
    kFilterStartFailed = 6,
    kFilterSampleFailed = 7
};
enum {
    kSampleComplete = 4,
    kAllocatorHook = 0,
    kFreeHook = 1,
    kQueueBlockShift = 7,
    kQueueEntriesPerBlock = 1 << kQueueBlockShift,
    kQueueEntryWords = 2,
    kQueueWordsPerBlock = kQueueEntriesPerBlock * kQueueEntryWords
};
void PagedSampleQueue_dtor(void*, int);
void __dl__FPv(void*);
void* gtl_Free(void*);

// Release owned storage; repeated null guards preserve the original destructor-chain instructions.
void* GtlOwnedBuffer_dtor(void* self, int deleteFlag);

// Release owned buffer storage, preserving the volatile count read, then optionally delete the wrapper.
void* dtor_803F5644(void* self, int deleteFlag);

// Destroy the queue wrapper; repeated null guards preserve the original destructor-chain instructions.
void* dtor_803F5744(void* self, int deleteFlag);

// Allocate through the configured runtime allocation callback.
void* gtl_Alloc(unsigned int size);

// Free a non-null allocation through the configured runtime callback.
void* gtl_Free(void* p);

} // extern "C"

// Function definitions follow the original address order.

#pragma push
#pragma dont_inline on
// Clear the borrowed sample pointers and optionally release the sample object.
void* gtl::CMediaSample::Destroy(int deleteFlag) {
    if (this != 0) {
        format = 0;
        data = 0;
        size = 0;
        if (deleteFlag > 0) {
            if (this != 0)
                ((void* (*)(void*))gtl_Free)(this);
        }
    }
    return this;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Copy the format descriptor, including its fixed 128-byte name and opaque codec parameters.
int gtl::CMediaSample::CopyFormat(void* out) {
    volatile unsigned char* sourceCursor;
    volatile unsigned char* destinationCursor;
    int pairsRemaining;
    GtlMediaFormat* sourceFormat = (GtlMediaFormat*)format;
    AsGtlMediaFormat(out)->unk_0 = sourceFormat->unk_0;
    AsGtlMediaFormat(out)->sourceByteLength = sourceFormat->sourceByteLength;
    sourceCursor = (volatile unsigned char*)sourceFormat->name - 1;
    destinationCursor = (volatile unsigned char*)AsGtlMediaFormat(out)->name - 1;
    pairsRemaining = sizeof(sourceFormat->name) / 2;
    do {
        unsigned char firstByte = sourceCursor[1];
        unsigned char secondByte = *(sourceCursor += 2);
        destinationCursor[1] = firstByte;
        *(destinationCursor += 2) = secondByte;
    } while (--pairsRemaining != 0);
    // These codec fields are copied verbatim; interpreting them is the format provider's responsibility.
    AsGtlMediaFormat(out)->showCounterFormatParameter = sourceFormat->showCounterFormatParameter;
    AsGtlMediaFormat(out)->initialReadOffset = sourceFormat->initialReadOffset;
    AsGtlMediaFormat(out)->sampleBufferSize = sourceFormat->sampleBufferSize;
    // +0x94 is a copied opaque format word, not unused padding.
    *(int*)AsGtlMediaFormat(out)->unk_94 = *(int*)sourceFormat->unk_94;
    AsGtlMediaFormat(out)->unk_98 = sourceFormat->unk_98;
    AsGtlMediaFormat(out)->unk_9A = sourceFormat->unk_9A;
    AsGtlMediaFormat(out)->transformWorkspaceCount = sourceFormat->transformWorkspaceCount;
    AsGtlMediaFormat(out)->splitPacketHeaderWordCount = sourceFormat->splitPacketHeaderWordCount;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Return the sample buffer address when an output pointer is supplied.
int gtl::CMediaSample::GetData(int* out) {
    if (out == 0)
        return 0;
    *out = data;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Replace the sample buffer address and byte count.
int gtl::CMediaSample::SetBuffer(int dataAddress, int byteCount) {
    data = dataAddress;
    size = byteCount;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Attach a non-null format descriptor without taking ownership.
int gtl::CMediaSample::SetFormat(void* newFormat) {
    if (newFormat == 0)
        return 0;
    format = (int)newFormat;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Return the current sample byte count.
int gtl::CMediaSample::GetSize(int* out) {
    *out = size;
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
void* gtl_Alloc(unsigned int size) {
    return g_gtlRuntimeConfig[kAllocatorHook](size);
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
void* gtl_Free(void* p) {
    if (p == 0)
        return p;
    return ((void* (*)(void*))g_gtlRuntimeConfig[kFreeHook])(p);
}
}
#pragma pop

#pragma push
#pragma dont_inline on
// Build an empty sampling queue and allocate its reusable output sample from the supplied format.
void* gtl::CSamplingFilter::Init(void* format) {
    IFilter::Init(kFilterStartRequested);
    // The vtable is the table's start; the process and start slots read the full 0x34-byte table.
    vtable = __vt__Q23gtl15CSamplingFilter;
    samplePool = 0;
    started = 0;
    inputQueue.firstIndex = 0;
    inputQueue.size = 0;
    inputQueue.pages.dataAddress = 0;
    inputQueue.pages.firstIndex = 0;
    inputQueue.pages.size = 0;
    inputQueue.pages.capacity = 0;
    unsigned int bufferBytes = AsGtlMediaFormat(format)->sampleBufferSize;
    void* sampleData = gtl_Alloc(bufferBytes);
    Memory_Clear(sampleData, AsGtlMediaFormat(format)->sampleBufferSize);
    void* mediaSample = gtl_Alloc(sizeof(gtl::CMediaSample));
    if (mediaSample != 0)
        ((gtl::CMediaSample*)mediaSample)
            ->Init((int)sampleData, AsGtlMediaFormat(format)->sampleBufferSize, (int)format);
    sample = mediaSample;
    return this;
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
void* dtor_803F5644(void* self, int deleteFlag) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    void* data = AsGtlOwnedBuffer803F5428(self)->data;
                    if (data != 0) {
                        int previousCount = (*(volatile int*)&AsGtlOwnedBuffer803F5428(self)->count);
                        AsGtlOwnedBuffer803F5428(self)->count = previousCount - previousCount;
                        gtl_Free(data);
                    }
                }
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
void* GtlOwnedBuffer_dtor(void* self, int deleteFlag) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        if (self != 0) {
                            void* data = AsGtlOwnedBuffer803F5428(self)->data;
                            if (data != 0) {
                                int previousCount = AsGtlOwnedBuffer803F5428(self)->count;
                                AsGtlOwnedBuffer803F5428(self)->count = previousCount - previousCount;
                                gtl_Free(data);
                            }
                        }
                    }
                }
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
void* dtor_803F5744(void* self, int deleteFlag) {
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                PagedSampleQueue_dtor(self, 0);
            }
        }
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
// Release the reusable sample, queued references, queue blocks, and base filter.
void* gtl::CSamplingFilter::Destroy(int deleteFlag) {
    if (this != 0) {
        vtable = __vt__Q23gtl15CSamplingFilter;
        int sampleDataAddress = 0;
        ((gtl::CMediaSample*)sample)->GetData((int*)&sampleDataAddress);
        if (sampleDataAddress != 0) {
            gtl_Free((void*)sampleDataAddress);
            sampleDataAddress = 0;
        }
        RefCountBase* outputSample = (RefCountBase*)sample;
        if (outputSample != 0 && --outputSample->refs == 0) {
            outputSample->destroy(1);
        }
        sample = 0;
        samplePool = 0;

        // Blocks hold 0x80 entries of 8 bytes; each entry starts with a counted object.
        GtlPagedQueue803F* queue = &inputQueue;
        unsigned int** block;
        unsigned int* entry;
        {
            int blockIndex = (int)(queue->firstIndex >> kQueueBlockShift) + (int)queue->pages.firstIndex;
            if (blockIndex < 0)
                blockIndex += queue->pages.capacity;
            else if (blockIndex >= (int)queue->pages.capacity)
                blockIndex -= queue->pages.capacity;
            block = (unsigned int**)queue->pages.dataAddress + blockIndex;
            entry = (queue->pages.size ? *block : 0) + ((queue->firstIndex & (kQueueEntriesPerBlock - 1)) << 1);
        }
        for (;;) {
            unsigned int endIndex = queue->firstIndex + queue->size;
            int blockIndex = (int)(endIndex >> kQueueBlockShift) + (int)queue->pages.firstIndex;
            if (blockIndex < 0)
                blockIndex += queue->pages.capacity;
            else if (blockIndex >= (int)queue->pages.capacity)
                blockIndex -= queue->pages.capacity;
            unsigned int* end = (queue->pages.size ? ((unsigned int**)queue->pages.dataAddress)[blockIndex] : 0) +
                                ((endIndex & (kQueueEntriesPerBlock - 1)) << 1);
            if (entry == end)
                break;
            RefCountBase* object = *(RefCountBase**)entry;
            if (object != 0 && --object->refs == 0)
                object->destroy(1);
            entry += kQueueEntryWords;
            if (entry == *block + kQueueWordsPerBlock) {
                ++block;
                if (block == (unsigned int**)queue->pages.dataAddress + queue->pages.capacity)
                    block = (unsigned int**)queue->pages.dataAddress;
                entry = *block;
            }
        }
        queue->size = 0;

        while (inputQueue.pages.size > 2) {
            unsigned int* entries = (unsigned int*)inputQueue.pages.dataAddress;
            unsigned int mapPosition = inputQueue.pages.firstIndex;
            gtl_Free((void*)entries[mapPosition]);
            mapPosition++;
            if (mapPosition == inputQueue.pages.capacity)
                mapPosition = 0;
            inputQueue.pages.firstIndex = mapPosition;
            --inputQueue.pages.size;
        }
        if (inputQueue.pages.size != 0)
            queue->firstIndex = kQueueEntriesPerBlock / 2;
        PagedSampleQueue_dtor(&inputQueue, 0);
        IFilter::Destroy(0);
        if (deleteFlag > 0 && this != 0)
            gtl_Free(this);
    }
    return this;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Wait for input, start the subclass once, then process one sample and publish its state.
int gtl::CSamplingFilter::Process() {
    int state;
    GetState(&state);
    if (state == kFilterStopped || state == kFilterIdle)
        return 1;
    if (samplePool == 0)
        return 0;
    if (!CollectAndCheckInput(this)) {
        SetState(kFilterWaitingForInput);
        return 1;
    }
    if (started == 0) {
        if (!((SamplingFilterCalls*)this)->start(0, (int)sample)) {
            SetState(kFilterStartFailed);
            return 0;
        }
        started = 1;
    }
    if (((SamplingFilterCalls*)this)->process((int)sample) != kSampleComplete) {
        SetState(kFilterSampleFailed);
        return 0;
    }
    SetState(kFilterSampleComplete);
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Request that sample processing start.
int gtl::CSamplingFilter::RequestStart() {
    SetState(kFilterStartRequested);
    return 1;
}
#pragma pop

#pragma push
#pragma dont_inline on
// Stop sample processing.
int gtl::CSamplingFilter::StopSampling() {
    SetState(kFilterStopped);
    return 1;
}
#pragma pop
