// Revolution sampling filter, part 2: reporting the byte count of the next output sample.

#include "classes/gtl_CRevolutionSamplingFilter.h"

// Write the next output sample's requested byte count and return 1. Neither pointer is checked
// or retained, and no queue state changes. This is the next target size, not bytes already queued.
int gtl::CRevolutionSamplingFilter::GetOutputBytes(unsigned int* outRequestedBytes) {
    *outRequestedBytes = requestedBytes;
    return 1;
}
