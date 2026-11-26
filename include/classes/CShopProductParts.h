#ifndef CLASSES_CSHOPPRODUCTPARTS_H
#define CLASSES_CSHOPPRODUCTPARTS_H

#include "game/types.h"

namespace IFF_STRUCT {
struct sPart;
}
struct IffItemCommon;

class CShopProductParts {
public:
    void* vtbl;                // 0x0: original vtable pointer
    void* saveData;            // 0x4
    char pad_008[0x5C];        // 0x8
    unsigned int filterFlags;  // 0x64: selected character, compared with a part's bits 18-25 (name inferred)
    char pad_068[0xC8];        // 0x68
    unsigned int partTypeMask; // 0x130

    int CopyRecord(void* dst, unsigned idx);
    int CopyCommonRecord(IffItemCommon* out, unsigned int index);
    int DeleteAll();

    int RebuildFromPartTable(int filterArgument);
    int AcceptsPart(IFF_STRUCT::sPart* part, int test);

    ~CShopProductParts();
    CShopProductParts(const unsigned int* src, unsigned int mode, void* initArg, void* dispatchArg);
};

#define CSHOPPRODUCTPARTS_OFF(f, o) \
    typedef char CShopProductParts_off_##f[((int)&((CShopProductParts*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTPARTS_OFF(saveData, 0x4);
CSHOPPRODUCTPARTS_OFF(filterFlags, 0x64);
CSHOPPRODUCTPARTS_OFF(partTypeMask, 0x130);
#undef CSHOPPRODUCTPARTS_OFF

#endif // CLASSES_CSHOPPRODUCTPARTS_H
