// GtlRecordDeque: layout-only view owned by gtl_texture_projection_input_device.cpp (GtlRecordDeque_PrependRange, GtlRecordDeque_AllocateStorage, GtlRecordDeque_CalculateGrowthCapacity).
// No virtual methods, constructors, or generated RTTI.
#ifndef CLASSES_GTLRECORDDEQUE_H
#define CLASSES_GTLRECORDDEQUE_H

class GtlRecordDeque {
public:
    void* data; // 0x0: GtlRecordDeque_AllocateStorage allocates count * 0x18 bytes
    unsigned int size; // 0x4: GtlRecordDeque_PrependRange increments after moving one record
    unsigned int capacity; // 0x8: GtlRecordDeque_AllocateStorage stores allocation count; GtlRecordDeque_CalculateGrowthCapacity grows it
    void* allocator; // 0xc: GtlRecordDeque_PrependRange passes to record copy GtlRecordDeque_CopyConstructElement
    int firstIndex; // 0x10: GtlRecordDeque_PrependRange decrements for each prepended record
};

#define OFF(f, o) typedef char GtlRecordDeque_off_##f[((int)&((GtlRecordDeque*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x0);
OFF(size, 0x4);
OFF(capacity, 0x8);
OFF(allocator, 0xc);
OFF(firstIndex, 0x10);
#undef OFF

#endif
