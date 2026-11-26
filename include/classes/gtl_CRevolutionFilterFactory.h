// gtl::CRevolutionFilterFactory: creates the filter stages of the Revolution media pipeline.
// Each Create method stores the new filter through its output pointer and returns 1, or 0 for null arguments.
// The methods are defined in the gtl_revolution_filter_factory_*.cpp and revolution_*.cpp files.
// No virtual functions are declared here; the vtable comes from the original data.
#ifndef CLASSES_GTL_CREVOLUTIONFILTERFACTORY_H
#define CLASSES_GTL_CREVOLUTIONFILTERFACTORY_H

struct RevolutionFilter;

namespace gtl {

class CRevolutionFilter;

class CRevolutionFilterFactory {
public:
    // Install the factory vtable in this object.
    CRevolutionFilterFactory();
    // Free this object on the gtl heap when deleting is positive; returns this.
    void* Destroy(int deleting);

    // Create the source filter from the {pointer, word} block in creationArgs.
    int CreateSourceFilter(void** outFilter, void* creationArgs);
    // Create the sampling filter with the given settings.
    int CreateSampling(CRevolutionFilter** output, void* settings);
    // Create the split filter that reads from filter.
    int CreateSplitFilter(void** out, void* filter);
    // Create the VTrans filter from source.
    int CreateVTransFilter(CRevolutionFilter** output, void* source);
    // Create the STrans filter with the constructor context.
    int CreateSTransFilter(RevolutionFilter** output, void* constructorContext);

    void* vtbl; // 0x0
};

} // namespace gtl

typedef char gtl_CRevolutionFilterFactory_off_vtbl[((int)&((gtl::CRevolutionFilterFactory*)0)->vtbl == 0x0) ? 1 : -1];

#endif
