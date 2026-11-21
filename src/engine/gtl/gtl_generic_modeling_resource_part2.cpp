#include <decomp/cw_bitfields.h>
// Translation unit 0x803DB750-0x803DB888 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct GenericModelingRes {
    CW_BITS2(
        unsigned int refCount : 16,
        unsigned int unk0 : 16)
};

struct gtl_IGenericModelingResource {
    char reserved00[0xC];
    GenericModelingRes** resA;
    char unk10[0x8];
    GenericModelingRes** resB;
};

extern "C" {

void CubeResourceStorage_dtor(GenericModelingRes* p, int flags);
int gtl_IGenericModelingResource_SetResource(gtl_IGenericModelingResource* self, int type, int index, GenericModelingRes* res)
{
    switch (type) {
    case 3: {
        GenericModelingRes* old = self->resA[index];
        if (old != 0) {
            old->refCount--;
            if (old->refCount == 0)
                CubeResourceStorage_dtor(old, 1);
        }
        self->resA[index] = 0;
        if (res != 0) {
            self->resA[index] = res;
            res->refCount++;
        }
        break;
    }
    case 4:
        break;
    case 5: {
        GenericModelingRes* old = self->resB[index];
        if (old != 0) {
            old->refCount--;
            if (old->refCount == 0)
                CubeResourceStorage_dtor(old, 1);
        }
        self->resB[index] = 0;
        if (res != 0) {
            self->resB[index] = res;
            res->refCount++;
        }
        break;
    }
    }
    return 1;
}

} // extern "C"
