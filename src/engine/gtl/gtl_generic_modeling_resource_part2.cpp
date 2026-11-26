// Replaces one graphics handle in a model resource. The old handle loses one reference and is destroyed at zero;
// the new handle gains one. Type 3 selects the textures table and type 5 the palettes table; type 4 is ignored.
#include "classes/GtlIGenericModelingResource.h"

extern "C" {

void CubeResourceStorage_dtor(GtlRefHeader* p, int flags);
}

// Required live self. Returns 1 for every selector, including unsupported ones.
int gtl::IGenericModelingResource::SetResource(int type, int index, GtlRefHeader* res) {
    switch (type) {
    case 3: {
        GtlRefHeader* old = ((GtlRefHeader**)textures.data)[index];
        if (old != 0) {
            old->refCount--;
            if (old->refCount == 0)
                CubeResourceStorage_dtor(old, 1);
        }
        ((GtlRefHeader**)textures.data)[index] = 0;
        if (res != 0) {
            ((GtlRefHeader**)textures.data)[index] = res;
            res->refCount++;
        }
        break;
    }
    case 4:
        break;
    case 5: {
        GtlRefHeader* old = ((GtlRefHeader**)palettes.data)[index];
        if (old != 0) {
            old->refCount--;
            if (old->refCount == 0)
                CubeResourceStorage_dtor(old, 1);
        }
        ((GtlRefHeader**)palettes.data)[index] = 0;
        if (res != 0) {
            ((GtlRefHeader**)palettes.data)[index] = res;
            res->refCount++;
        }
        break;
    }
    }
    return 1;
}
