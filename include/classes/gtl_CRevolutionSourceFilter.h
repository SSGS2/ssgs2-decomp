// gtl::CRevolutionSourceFilter: the DVD-backed source filter. It extends CSourceFilter with the
// byte count of the last DVD read it submitted.
#ifndef CLASSES_GTL_CREVOLUTIONSOURCEFILTER_H
#define CLASSES_GTL_CREVOLUTIONSOURCEFILTER_H

#include "classes/gtl_CSourceFilter.h"

struct DVDFileInfo;

namespace gtl {

class CRevolutionSourceFilter : public CSourceFilter {
public:
    void* Init(void* source, unsigned int callback);
    int GetSubmittedReadBytes(unsigned int* outSubmittedBytes);
    int SubmitOrPollDvdRead(DVDFileInfo* file, void* sample, unsigned int* requestedBytes);
    CRevolutionSourceFilter* Destroy(int deleteFlag);

    unsigned int submittedReadBytes; // 0x2C
};

} // namespace gtl

#define GTL_CREVOLUTIONSOURCEFILTER_OFF(f, o) \
    typedef char gtl_CRevolutionSourceFilter_off_##f[((int)&((gtl::CRevolutionSourceFilter*)0)->f == (o)) ? 1 : -1]
GTL_CREVOLUTIONSOURCEFILTER_OFF(submittedReadBytes, 0x2C);
#undef GTL_CREVOLUTIONSOURCEFILTER_OFF

#endif // CLASSES_GTL_CREVOLUTIONSOURCEFILTER_H
