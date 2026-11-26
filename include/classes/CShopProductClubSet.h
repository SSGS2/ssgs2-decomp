#ifndef CLASSES_CSHOPPRODUCTCLUBSET_H
#define CLASSES_CSHOPPRODUCTCLUBSET_H

#include "game/types.h"
#include "classes/CShopBaseD.h"

// Club-set product list: a vector of owned copies of club-set IFF records.
// The vector's storage and record copies are released by DeleteAll, which keeps the
// pointer capacity. The vtable is the CShopBaseD vtable.
class CShopProductClubSet : public CShopBaseD {
public:
    CShopProductClubSet(void* a, void* b);

    int DeleteAll();
    void* GetCount();
    int CopyRecord(void* destination, unsigned int productIndex);
    int CopyCommonHeader(unsigned char* dest, unsigned int index);
    int CollectAcceptedRecords(int filter);
    int IsOwnedOrEquipped(void* productRecord, int testEquipped);
    void* Destroy(int shouldDelete);

    void* context;         // 0x04
    void** begin;          // 0x08 pointer array of owned records
    unsigned int size;     // 0x0C
    unsigned int capacity; // 0x10
};

#define CSHOPPRODUCTCLUBSET_OFF(f, o) \
    typedef char CShopProductClubSet_off_##f[((int)&((CShopProductClubSet*)0)->f == (o)) ? 1 : -1]
CSHOPPRODUCTCLUBSET_OFF(begin, 0x8);
CSHOPPRODUCTCLUBSET_OFF(size, 0xc);
CSHOPPRODUCTCLUBSET_OFF(capacity, 0x10);
#undef CSHOPPRODUCTCLUBSET_OFF

#endif // CLASSES_CSHOPPRODUCTCLUBSET_H
