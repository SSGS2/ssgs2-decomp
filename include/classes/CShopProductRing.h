#ifndef CLASSES_CSHOPPRODUCTRING_H
#define CLASSES_CSHOPPRODUCTRING_H

#include "game/types.h"

struct ShopRecordCountView;
struct ShopCommonRecordBuffer;
struct RingProductRecords;
struct RingProductRecordsForClear;
class CShopProductRing {
public:
    void* vtbl;     // 0x0: original vtable pointer
    void* saveData; // 0x4

    int IsOwned(void* arg1);

    static int GetCount(ShopRecordCountView* arg);
    int CopyCommonRecord(ShopCommonRecordBuffer* out);
    static int CopyRecord(RingProductRecords* arg, void* dst, unsigned int index);
    static int DeleteAll(RingProductRecordsForClear* arg);
    int RebuildFilteredAuxParts(void* filterContext);

    ~CShopProductRing();
    CShopProductRing(void* value, void* arg2);
};

#define CSHOPPRODUCTRING_OFF(f, o) \
    typedef char CShopProductRing_off_##f[((int)&((CShopProductRing*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTRING_OFF(saveData, 0x4);
#undef CSHOPPRODUCTRING_OFF

#endif // CLASSES_CSHOPPRODUCTRING_H
