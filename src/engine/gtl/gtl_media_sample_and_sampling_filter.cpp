#include "classes/GtlOwnedBuffer803F5428.h"
#include "classes/GtlSamplingFilter.h"
#include "classes/GtlMediaSample.h"
#include "classes/GtlMediaFormat.h"
#define AsGtlMediaSample(p) ((GtlMediaSample*)(p))
#define AsGtlMediaFormat(p) ((GtlMediaFormat*)(p))
// Translation unit 0x803F5428-0x803F5C5C (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
#include "sdk_protos.h"
void* (*g_gtlRuntimeConfig[4])(unsigned int);
void gtl_IFilter_SetState(void*, unsigned int);
void gtl_IFilter_GetState(void*, int*);
int gtl_CSamplingFilter_CollectAndCheckInput(void*);
void* gtl_IFilter_dtor_Base(void*, int);
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
struct SamplingFilterPrefix { int refs; };
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
void* gtl_IFilter_Ctor(void*, int);
void* gtl_CMediaSample_Ctor(void*, void*, unsigned int, void*);
extern void* __vt__Q23gtl15CSamplingFilter[4];
enum FilterState {
    kFilterIdle = 0, kFilterStartRequested = 1, kFilterWaitingForInput = 2,
    kFilterStopped = 3, kFilterSampleComplete = 4, kFilterStartFailed = 6, kFilterSampleFailed = 7
};
enum { kSampleComplete = 4, kAllocatorHook = 0, kFreeHook = 1,
       kQueueBlockShift = 7, kQueueEntriesPerBlock = 1 << kQueueBlockShift,
       kQueueEntryWords = 2, kQueueWordsPerBlock = kQueueEntriesPerBlock * kQueueEntryWords };
void PagedSampleQueue_dtor(void*, int);
void __dl__FPv(void*);
void* gtl_Free(void*);

// Copy the format descriptor, including its fixed 128-byte name and opaque codec parameters.
int gtl_CMediaSample_CopyFormat(void* self, void* out);

// Release owned storage; repeated null guards preserve the original destructor-chain instructions.
void* GtlOwnedBuffer_dtor(void* self, int deleteFlag);

// Build an empty sampling queue and allocate its reusable output sample from the supplied format.
void* gtl_CSamplingFilter_Ctor(void* self, void* format);

// Release owned buffer storage, preserving the volatile count read, then optionally delete the wrapper.
void* dtor_803F5644(void* self, int deleteFlag);

// Destroy the queue wrapper; repeated null guards preserve the original destructor-chain instructions.
void* dtor_803F5744(void* self, int deleteFlag);

// Return the sample buffer address when an output pointer is supplied.
int gtl_CMediaSample_GetData(void* self, int* out);

// Attach a non-null format descriptor without taking ownership.
int gtl_CMediaSample_SetFormat(void* self, void* format);

// Replace the sample buffer address and byte count.
int gtl_CMediaSample_SetBuffer(void* self, int dataAddress, int byteCount);

// Return the current sample byte count.
int gtl_CMediaSample_GetSize(void* self, int* out);

// Allocate through the configured runtime allocation callback.
void* gtl_Alloc(unsigned int size);

// Free a non-null allocation through the configured runtime callback.
void* gtl_Free(void* p);

// Clear borrowed sample pointers and optionally release the sample object.
void* gtl_CMediaSample_dtor(void* self, int deleteFlag);

// Request that sample processing start.
int gtl_CSamplingFilter_RequestStart(void* self);

// Stop sample processing.
int gtl_CSamplingFilter_StopSampling(void* self);

// Wait for input, start the subclass once, then process one sample and publish its state.
int gtl_CSamplingFilter_Process(void* self);

// Release the reusable sample, queued references, queue blocks, and base filter.
void* gtl_CSamplingFilter_dtor(void* self, int deleteFlag);
} // extern "C"


// Function definitions follow the original address order.


#pragma push
#pragma dont_inline on
extern "C" {
void* gtl_CMediaSample_dtor(void* self, int deleteFlag) {
    if (self != 0) {
        AsGtlMediaSample(self)->format = 0;
        AsGtlMediaSample(self)->data = 0;
        AsGtlMediaSample(self)->size = 0;
        if (deleteFlag > 0) {
            if (self != 0) ((void*(*)(void*))gtl_Free)(self);
        }
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
int gtl_CMediaSample_CopyFormat(void* self, void* out) {
    volatile unsigned char* sourceCursor;
    volatile unsigned char* destinationCursor;
    int pairsRemaining;
    GtlMediaFormat* sourceFormat = (GtlMediaFormat*)AsGtlMediaSample(self)->format;
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
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
int gtl_CMediaSample_GetData(void* self, int* out) {
    if (out == 0) return 0;
    *out = AsGtlMediaSample(self)->data;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
int gtl_CMediaSample_SetBuffer(void* self, int dataAddress, int byteCount) {
    AsGtlMediaSample(self)->data = dataAddress;
    AsGtlMediaSample(self)->size = byteCount;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
int gtl_CMediaSample_SetFormat(void* self, void* format) {
    if (format == 0) return 0;
    AsGtlMediaSample(self)->format = (int)format;
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
int gtl_CMediaSample_GetSize(void* self, int* out) {
    *out = AsGtlMediaSample(self)->size;
    return 1;
}
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
    if (p == 0) return p;
    return ((void*(*)(void*))g_gtlRuntimeConfig[kFreeHook])(p);
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
void* gtl_CSamplingFilter_Ctor(void* self, void* format) {
    gtl_IFilter_Ctor(self, kFilterStartRequested);
    // 0x803F55B4-0x803F55C4: lis/addi __vt__Q23gtl15CSamplingFilter (0x804DAA20, no addend), stw r4,0x4(r28).
    // Ours stored vt+0x10, so vtable[10]/[11] in gtl_CSamplingFilter_Process (+0x28/+0x2C) read past the 0x34-byte table.
    AsGtlSamplingFilter(self)->vtable = __vt__Q23gtl15CSamplingFilter;
    AsGtlSamplingFilter(self)->inputSamplePool = 0;
    AsGtlSamplingFilter(self)->started = 0;
    AsGtlSamplingFilter(self)->queueStart = 0;
    AsGtlSamplingFilter(self)->queueSize = 0;
    AsGtlSamplingFilter(self)->queueMap = 0;
    AsGtlSamplingFilter(self)->queueMapStart = 0;
    AsGtlSamplingFilter(self)->queueMapSize = 0;
    AsGtlSamplingFilter(self)->queueMapCapacity = 0;
    unsigned int bufferBytes = AsGtlMediaFormat(format)->sampleBufferSize;
    void* sampleData = gtl_Alloc(bufferBytes);
    Memory_Clear(sampleData, AsGtlMediaFormat(format)->sampleBufferSize);
    void* mediaSample = gtl_Alloc(sizeof(GtlMediaSample));
    if (mediaSample != 0) gtl_CMediaSample_Ctor(mediaSample, sampleData, AsGtlMediaFormat(format)->sampleBufferSize, format);
    AsGtlSamplingFilter(self)->sample = (int)mediaSample;
    return self;
}
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
        if (deleteFlag > 0) __dl__FPv(self);
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
        if (deleteFlag > 0) __dl__FPv(self);
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
        if (deleteFlag > 0) __dl__FPv(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
void* gtl_CSamplingFilter_dtor(void* self, int deleteFlag) {
    if (self != 0) {
        AsGtlSamplingFilter(self)->vtable = __vt__Q23gtl15CSamplingFilter;
        int sampleDataAddress = 0;
        gtl_CMediaSample_GetData((void*)AsGtlSamplingFilter(self)->sample, &sampleDataAddress);
        if (sampleDataAddress != 0) {
            gtl_Free((void*)sampleDataAddress);
            sampleDataAddress = 0;
        }
        RefCountBase* sample = (RefCountBase*)AsGtlSamplingFilter(self)->sample;
        if (sample != 0 && --sample->refs == 0) {
            sample->destroy(1);
        }
        AsGtlSamplingFilter(self)->sample = 0;
        AsGtlSamplingFilter(self)->inputSamplePool = 0;

        // Queue at 0x1C: start, size, block map, map start, map size, map capacity.
        // Blocks hold 0x80 entries of 8 bytes; each entry starts with a counted object.
        struct Queue { unsigned int start, size; unsigned int** map; unsigned int mapStart, mapSize, mapCap; };
        Queue* queue = (Queue*)&AsGtlSamplingFilter(self)->queueStart; // 0x803F57A4: embedded queue.
        unsigned int** block;
        unsigned int* entry;
        {
            int blockIndex = (int)(queue->start >> kQueueBlockShift) + (int)queue->mapStart;
            if (blockIndex < 0) blockIndex += queue->mapCap;
            else if (blockIndex >= (int)queue->mapCap) blockIndex -= queue->mapCap;
            block = queue->map + blockIndex;
            entry = (queue->mapSize ? *block : 0) + ((queue->start & (kQueueEntriesPerBlock - 1)) << 1);
        }
        for (;;) {
            unsigned int endIndex = queue->start + queue->size;
            int blockIndex = (int)(endIndex >> kQueueBlockShift) + (int)queue->mapStart;
            if (blockIndex < 0) blockIndex += queue->mapCap;
            else if (blockIndex >= (int)queue->mapCap) blockIndex -= queue->mapCap;
            unsigned int* end = (queue->mapSize ? queue->map[blockIndex] : 0) + ((endIndex & (kQueueEntriesPerBlock - 1)) << 1);
            if (entry == end) break;
            RefCountBase* object = *(RefCountBase**)entry;
            if (object != 0 && --object->refs == 0) object->destroy(1);
            entry += kQueueEntryWords;
            if (entry == *block + kQueueWordsPerBlock) {
                ++block;
                if (block == queue->map + queue->mapCap) block = queue->map;
                entry = *block;
            }
        }
        queue->size = 0;

        while (AsGtlSamplingFilter(self)->queueMapSize > 2) {
            unsigned int* entries = (unsigned int*)AsGtlSamplingFilter(self)->queueMap;
            unsigned int mapPosition = AsGtlSamplingFilter(self)->queueMapStart;
            gtl_Free((void*)entries[mapPosition]);
            mapPosition++;
            if (mapPosition == AsGtlSamplingFilter(self)->queueMapCapacity) mapPosition = 0;
            AsGtlSamplingFilter(self)->queueMapStart = mapPosition;
            --AsGtlSamplingFilter(self)->queueMapSize;
        }
        if (AsGtlSamplingFilter(self)->queueMapSize != 0)
            queue->start = kQueueEntriesPerBlock / 2;
        PagedSampleQueue_dtor(&AsGtlSamplingFilter(self)->queueStart, 0);
        gtl_IFilter_dtor_Base(self, 0);
        if (deleteFlag > 0 && self != 0) gtl_Free(self);
    }
    return self;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
int gtl_CSamplingFilter_Process(void* self) {
    int state;
    gtl_IFilter_GetState(self, &state);
    if (state == kFilterStopped || state == kFilterIdle) return 1;
    if (AsGtlSamplingFilter(self)->inputSamplePool == 0) return 0;
    if (!gtl_CSamplingFilter_CollectAndCheckInput(self)) {
        gtl_IFilter_SetState(self, kFilterWaitingForInput);
        return 1;
    }
    if (AsGtlSamplingFilter(self)->started == 0) {
        if (!((SamplingFilterCalls*)self)->start(0, AsGtlSamplingFilter(self)->sample)) {
            gtl_IFilter_SetState(self, kFilterStartFailed);
            return 0;
        }
        AsGtlSamplingFilter(self)->started = 1;
    }
    if (((SamplingFilterCalls*)self)->process(AsGtlSamplingFilter(self)->sample) != kSampleComplete) {
        gtl_IFilter_SetState(self, kFilterSampleFailed);
        return 0;
    }
    gtl_IFilter_SetState(self, kFilterSampleComplete);
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
int gtl_CSamplingFilter_RequestStart(void* self) {
    gtl_IFilter_SetState(self, kFilterStartRequested);
    return 1;
}
}
#pragma pop

#pragma push
#pragma dont_inline on
extern "C" {
int gtl_CSamplingFilter_StopSampling(void* self) {
    gtl_IFilter_SetState(self, kFilterStopped);
    return 1;
}
}
#pragma pop
