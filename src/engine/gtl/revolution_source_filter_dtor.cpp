// Translation unit 0x803F9C24-0x803F9C84 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

namespace gtl {
class CRevolutionSourceFilter {};
}

extern "C" {

void* gtl_CSourceFilter_dtor(void* self, int deleteFlag);
void gtl_Free(void* self);

// Destroy the source-filter base, then release storage only when deleteFlag > 0.
// Null self is accepted; base cleanup receives 0 so it does not free the storage.
// Return the original address, which must not be dereferenced after a deleting call.
gtl::CRevolutionSourceFilter* gtl_CRevolutionSourceFilter_dtor(
    gtl::CRevolutionSourceFilter* self, int deleteFlag) {
    if (self) {
        gtl_CSourceFilter_dtor(self, 0);
        if (deleteFlag > 0) {
            if (self) {
                gtl_Free(self);
            }
        }
    }
    return self;
}

} // extern "C"
