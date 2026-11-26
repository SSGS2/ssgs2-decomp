// gtl::CSamplingFilter: a filter that queues input samples from an upstream pool and hands them to a
// subclass that turns them into one output sample. The queue is a GtlPagedQueue803F embedded at 0x1C.
// Methods are in gtl_media_sample_and_sampling_filter.cpp, gtl_sampling_filter_queue_reset.cpp and
// gtl_sampling_filter_queues.cpp. No virtual functions are declared here; the vtable comes from the
// original data.
#ifndef CLASSES_GTL_CSAMPLINGFILTER_H
#define CLASSES_GTL_CSAMPLINGFILTER_H

#include "classes/GtlPagedQueue803F.h"
#include "classes/gtl_IFilter.h"

namespace gtl {

class CSamplingFilter : public IFilter {
public:
    // Setup and teardown.
    // Initialize the base filter, an empty queue and the reusable output sample for the given format.
    void* Init(void* format);
    // Release the output sample, queued input, queue pages and base filter; free this when deleteFlag is positive.
    void* Destroy(int deleteFlag);

    // State changes.
    int Process();
    int RequestStart();
    int StopSampling();
    // Release every queued input sample and trim the queue back to its reusable pages.
    int ResetQueue();
    // Reset, then start the subclass through its virtual interface.
    int ResetAndStart();
    int vfunc_8(void* unused, int value);

    // Output sample and upstream pool.
    // Return the output sample with an added reference when an output pointer is supplied.
    int AcquireSharedRef(void* unused, unsigned int* out);
    // Take the upstream pool pointer from a media sample's data word.
    int SetSourceFromSample(void* sample);

    // Input queue.
    // Queue every sample the upstream pool has ready; return whether they cover the requested size.
    static int CollectAndCheckInput(CSamplingFilter* self);
    // Remove the first queued sample and its consumed byte count.
    int DequeueInputSample(void* out, void* outConsumedBytes);
    // Put a sample back at the front of the queue with its consumed byte count.
    int PrependInputSample(unsigned int* input, unsigned int consumedBytes);
    // Pass the recycled sample back to the upstream pool.
    void* RecycleReturnedSample();

    void* sample;                 // 0x10: reusable output sample
    void* samplePool;             // 0x14: upstream pool
    unsigned char started;        // 0x18
    char pad_019[0x3];            // 0x19
    GtlPagedQueue803F inputQueue; // 0x1C: {sample, consumed bytes} entries
};

} // namespace gtl

#define GTL_CSAMPLINGFILTER_OFF(f, o) \
    typedef char gtl_CSamplingFilter_off_##f[((int)&((gtl::CSamplingFilter*)0)->f == (o)) ? 1 : -1]
GTL_CSAMPLINGFILTER_OFF(sample, 0x10);
GTL_CSAMPLINGFILTER_OFF(samplePool, 0x14);
GTL_CSAMPLINGFILTER_OFF(started, 0x18);
GTL_CSAMPLINGFILTER_OFF(inputQueue, 0x1C);
#undef GTL_CSAMPLINGFILTER_OFF
typedef char gtl_CSamplingFilter_size[(sizeof(gtl::CSamplingFilter) == 0x34) ? 1 : -1];

#endif
