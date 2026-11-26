// gtl::CRevolutionFilterManager: the Revolution media pipeline manager. Its storage starts with the
// gtl::CFilterManager fields and adds the factory's five filter slots and a cached copy of the 24-word header.
// Methods are in the gtl_revolution_filter_manager_*.cpp, revolution_filter_*.cpp and
// gtl_revolution_filter_factory files. No virtual functions are declared here; the vtable comes from the original data.
#ifndef CLASSES_GTL_CREVOLUTIONFILTERMANAGER_H
#define CLASSES_GTL_CREVOLUTIONFILTERMANAGER_H

// Output descriptor written by gtl::CRevolutionFilterManager::BuildDescriptor (0xA4 bytes).
struct RevolutionFilterOutput {
    unsigned int baseState;
    char reservedState[0x84];
    float showCounterFormatParameter;
    unsigned int initialReadOffset;
    unsigned int sampleBufferSize;
    unsigned int parameterWord6;
    unsigned short parameterWord17;
    unsigned short parameterWord18;
    unsigned int transformWorkspaceCount;
    unsigned int splitPacketHeaderWordCount;
};

namespace gtl {

class CRevolutionFilterManager {
public:
    // Initialize the base manager from the config at input, set the manager vtable and start the THP decoder.
    // Returns this.
    void* Init(void* input);
    // Destroy the base manager and free this object when deleting is positive. Returns this.
    void* Destroy(int deleting);
    // Create the five filter stages in pipeline order and store each one in its slot.
    int CreateFilterPipeline();
    // Release each filter slot, destroying filters whose last reference is removed.
    void ReleaseAllFilters();
    // Create the display counter if the manager does not have one yet.
    int EnsureShowCounter();
    // Report the 24-word input parameter block size in bytes.
    int GetParameterBlockSize(int* size);
    // Cache the 24 input words and copy the supported ones into output.
    int BuildDescriptor(unsigned int* input, RevolutionFilterOutput* output);

    void* vtbl;                        // 0x0
    void* context;                     // 0x4
    char data[0x3C];                   // 0x8, settings block passed to the source filter
    void* field44;                     // 0x44, settings for the VTrans filter
    void* field48;                     // 0x48, context for the STrans filter
    void* showCounter;                 // 0x4C
    void* filters[5];                  // 0x50, source, sampling, split, VTrans, STrans
    char pad_064[0x2C];                // 0x64
    unsigned int cachedParameters[24]; // 0x90, the 24 input words of the header
};

} // namespace gtl

#define GTL_CREVOLUTIONFILTERMANAGER_OFF(f, o) \
    typedef char gtl_CRevolutionFilterManager_off_##f[((int)&((gtl::CRevolutionFilterManager*)0)->f == (o)) ? 1 : -1]
GTL_CREVOLUTIONFILTERMANAGER_OFF(vtbl, 0x0);
GTL_CREVOLUTIONFILTERMANAGER_OFF(showCounter, 0x4C);
GTL_CREVOLUTIONFILTERMANAGER_OFF(filters, 0x50);
GTL_CREVOLUTIONFILTERMANAGER_OFF(cachedParameters, 0x90);
#undef GTL_CREVOLUTIONFILTERMANAGER_OFF

#endif
