// Revolution filter factory, part 2: builds the manager's five-slot filter pipeline.
//
// CreateFilterPipeline fills the slots in order: source, sampling, split, and the two transform
// filters. Each stage is created directly into its slot.

#include "classes/gtl_CRevolutionFilterFactory.h"
#include "classes/gtl_CRevolutionFilterManager.h"

// Populate the manager's five filter slots using their respective borrowed settings.
// Returns 1 unconditionally: individual factory status values are ignored, so this
// does not prove every slot is non-null. Existing slots are not released here.
int gtl::CRevolutionFilterManager::CreateFilterPipeline() {
    // Source construction receives a two-word argument block; the other methods
    // receive the context or the settings pointer at manager +0x44/+0x48 directly.
    gtl::CRevolutionFilterFactory factory;
    struct Parameters {
        void* context;
        void* data;
    } parameters;
    parameters.context = context;
    parameters.data = data;
    // Build each stage in pipeline order and publish directly into the manager.
    factory.CreateSourceFilter(&filters[0], &parameters);
    factory.CreateSampling((gtl::CRevolutionFilter**)&filters[1], context);
    factory.CreateSplitFilter(&filters[2], context);
    factory.CreateVTransFilter((gtl::CRevolutionFilter**)&filters[3], field44);
    factory.CreateSTransFilter((RevolutionFilter**)&filters[4], field48);
    return 1;
}
