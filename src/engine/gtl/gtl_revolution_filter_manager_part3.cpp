// Translation unit 0x803F97FC-0x803F985C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

namespace gtl {
struct CRevolutionFilterManager {};
}

extern "C" {

void gtl_CFilterManager_dtor(void*, int);
void gtl_Free(void*);

// Destroy the base manager and free the wrapper with gtl_Free only for deleting > 0.
// Accepts null. Returns the original address, which is dangling after a deleting call.
gtl::CRevolutionFilterManager* gtl_CRevolutionFilterManager_dtor(
    gtl::CRevolutionFilterManager* self, int deleting)
{
    if (self) {
        gtl_CFilterManager_dtor(self, 0);
        if (deleting > 0) {
            if (self) {
                gtl_Free(self);
            }
        }
    }
    return self;
}

} // extern "C"
