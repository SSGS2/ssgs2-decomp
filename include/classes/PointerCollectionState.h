// PointerCollectionState: layout view; shop_product_ring_and_equip_lists.cpp: CShopProductEquipItem_RebuildFilteredItemList and CShopProductRing_RebuildFilteredAuxParts append elements and grow the pointer buffer
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_POINTERCOLLECTIONSTATE_H
#define CLASSES_POINTERCOLLECTIONSTATE_H

struct PointerCollectionState {
    char pad_000[0x8];
    void** data; // 0x8
    unsigned size; // 0xC
    unsigned capacity; // 0x10
};

#define OFF(f, o) typedef char PointerCollectionState_off_##f[((int)&((PointerCollectionState*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x8);
OFF(size, 0xC);
OFF(capacity, 0x10);
#undef OFF
#define AsPointerCollectionState(p) ((PointerCollectionState*)(p))

#endif
