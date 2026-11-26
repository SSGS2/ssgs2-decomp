// gtl::CRevolutionSamplingFilter: the Revolution sampling filter. It is the sampling filter base (gtl::CSamplingFilter,
// 0x34 bytes) followed by the byte counts of the packet being assembled. Methods are in
// gtl_revolution_sampling_filter_part1.cpp, revolution_sampling_filter_fill.cpp,
// gtl_revolution_sampling_filter_part2.cpp and gtl_revolution_sampling_filter_part3.cpp.
// No virtual functions are declared here; the vtable comes from the original data.
#ifndef CLASSES_GTL_CREVOLUTIONSAMPLINGFILTER_H
#define CLASSES_GTL_CREVOLUTIONSAMPLINGFILTER_H

#include "classes/gtl_CSamplingFilter.h"

namespace gtl {

class CRevolutionSamplingFilter : public CSamplingFilter {
public:
    // Construct the base queue and seed both byte counts from the owner's +0x94 word. Returns this.
    CRevolutionSamplingFilter* Init(void* owner);
    // Restore the seed request size, clear the completed count and reset the base queue. Returns 1.
    int Reset();
    // Assemble one output sample from the queue. Returns 4 when complete, 5 when the queue runs empty.
    int FillOutputSample(void* output);
    // Write the next requested byte count to outRequestedBytes. Returns 1.
    int GetOutputBytes(unsigned int* outRequestedBytes);
    // Destroy the base and free this object when deleteFlag is positive. Returns this.
    void* Destroy(int deleteFlag);

    unsigned int initialRequestBytes; // 0x34: reset seed, from the owner's +0x94 word
    unsigned int requestedBytes;      // 0x38: bytes to assemble for the next output sample
    unsigned int completedBytes;      // 0x3C: size of the last fully assembled output sample
};

} // namespace gtl

#define GTL_CREVOLUTIONSAMPLINGFILTER_OFF(f, o) \
    typedef char gtl_CRevolutionSamplingFilter_off_##f[((int)&((gtl::CRevolutionSamplingFilter*)0)->f == (o)) ? 1 : -1]
GTL_CREVOLUTIONSAMPLINGFILTER_OFF(initialRequestBytes, 0x34);
GTL_CREVOLUTIONSAMPLINGFILTER_OFF(requestedBytes, 0x38);
GTL_CREVOLUTIONSAMPLINGFILTER_OFF(completedBytes, 0x3C);
#undef GTL_CREVOLUTIONSAMPLINGFILTER_OFF

#endif
