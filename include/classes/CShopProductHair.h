#ifndef CLASSES_CSHOPPRODUCTHAIR_H
#define CLASSES_CSHOPPRODUCTHAIR_H

#include "game/types.h"

struct IffItemCommon;

class CShopProductHair {
public:
    void* vtbl;     // 0x0: original vtable pointer
    void* saveData; // 0x4
    char pad_008[0x5C];
    unsigned int
        characterTypeId; // 0x64: CShopProductHair_CheckUnlockAndOwnership compares low 26 bits to product's character index

    int CheckUnlockAndOwnership(void* hairStyleRecord, int testOwned);

    int BuildProductList(int filterArgument);
    int CopyRecord(void* outRecord, unsigned int productIndex);
    int CopyCommonRecord(IffItemCommon* out, unsigned int index);

    ~CShopProductHair();
    CShopProductHair(void* source, void* baseArg, void* initArg);
};

#define CSHOPPRODUCTHAIR_OFF(f, o) \
    typedef char CShopProductHair_off_##f[((int)&((CShopProductHair*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTHAIR_OFF(saveData, 0x4);
CSHOPPRODUCTHAIR_OFF(characterTypeId, 0x64);
#undef CSHOPPRODUCTHAIR_OFF

#endif // CLASSES_CSHOPPRODUCTHAIR_H
