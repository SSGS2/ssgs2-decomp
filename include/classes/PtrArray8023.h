// Pointer-array layout shared by the containers implemented in shop_product_lists_part1.cpp.
// ShopSetItemPointerArray_Initialize/ShopPartsPointerArray_Initialize initialize it; CShopProductAvatar_RebuildFromCharacterTable appends and grows it.
#ifndef CLASSES_PTRARRAY8023_H
#define CLASSES_PTRARRAY8023_H
class PtrArray8023 {
public:
    void* vtable;                 // +0x00: constructor-selected original vtable
    int parameter;                // +0x04: constructor argument; meaning unknown
    void* data;                   // +0x08: pointer storage, freed by destructors
    int count;                    // +0x0C: PtrArray_GetCount; incremented on append
    unsigned int capacity;        // +0x10: append allocation limit
};
#define PTRARRAY8023_OFF(f, o) typedef char PtrArray8023_off_##f[((int)&((PtrArray8023*)0)->f == (o)) ? 1 : -1]
PTRARRAY8023_OFF(vtable, 0x0);
PTRARRAY8023_OFF(parameter, 0x4);
PTRARRAY8023_OFF(data, 0x8);
PTRARRAY8023_OFF(count, 0xC);
PTRARRAY8023_OFF(capacity, 0x10);
#undef PTRARRAY8023_OFF
#define AsPtrArray8023(p) ((PtrArray8023*)(p))
#endif
