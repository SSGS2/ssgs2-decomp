// gtl::CRevolutionSplitFilter: the Revolution splitter. It is the base splitter (gtl::CSplitFilter, 0x20 bytes)
// with no fields of its own. Its vtable is installed by gtl_CRevolutionSplitFilter_Ctor
// (revolution_split_filter.cpp). No virtual functions are declared here.
#ifndef CLASSES_GTL_CREVOLUTIONSPLITFILTER_H
#define CLASSES_GTL_CREVOLUTIONSPLITFILTER_H

#include "classes/gtl_CSplitFilter.h"

namespace gtl {

class CRevolutionSplitFilter : public CSplitFilter {
public:
    // Construct the base splitter and install the Revolution vtable.
    CRevolutionSplitFilter(void* outputFormat, void* baseArgument);
    // Share the packet's two payload regions with the output samples without copying. Returns 4, the
    // base Process method's ready result. The input must be valid and its storage must outlive the outputs.
    int SplitInput(void* inputSample, void* primaryOutput, void* secondaryOutput);
    // Destroy the base splitter, then free this object when deleting is positive. Returns this.
    void* Destroy(int deleting);
};

} // namespace gtl

#endif // CLASSES_GTL_CREVOLUTIONSPLITFILTER_H
