// Layout views used by the input-device methods in gtl_texture_projection_input_device.cpp.
// These are generic input records; their fields do not imply a Wii-specific device.
#ifndef CLASSES_GTLINPUTDEVICERECORDS_H
#define CLASSES_GTLINPUTDEVICERECORDS_H

struct GtlInputDescriptorItem {
    unsigned int unk_0;
    unsigned int key; // +4: ResolveItemIndex compares against a type-1 query
    unsigned int typeMask; // +8: low byte intersects a type-2 query's mask
    unsigned int unk_C;
};

struct GtlInputDescriptor {
    char pad_000[0xC];
    unsigned int stateSizeBytes; // +C: ReadState validates size before clearing the caller's buffer
    unsigned int itemCount; // +10: ResolveItemIndex loop bound
    unsigned int* items; // +14: ResolveItemIndex walks four words per descriptor item
};

struct GtlInputItemQuery {
    char pad_000[8];
    unsigned int keyOrMaskIndex; // +8: type 1 = key; type 2 = mask[7:0], match ordinal[23:8]
    int type; // +C: ResolveItemIndex selects key (1) or masked ordinal (2) lookup
};

#define ITEM_OFF(f, o) typedef char GtlInputDescriptorItem_off_##f[((int)&((GtlInputDescriptorItem*)0)->f == (o)) ? 1 : -1]
ITEM_OFF(unk_0, 0x0);
ITEM_OFF(key, 0x4);
ITEM_OFF(typeMask, 0x8);
ITEM_OFF(unk_C, 0xC);
#undef ITEM_OFF
#define OFF(f, o) typedef char GtlInputDescriptor_off_##f[((int)&((GtlInputDescriptor*)0)->f == (o)) ? 1 : -1]
OFF(stateSizeBytes, 0xC);
OFF(itemCount, 0x10);
OFF(items, 0x14);
#undef OFF
#define QUERY_OFF(f, o) typedef char GtlInputItemQuery_off_##f[((int)&((GtlInputItemQuery*)0)->f == (o)) ? 1 : -1]
QUERY_OFF(keyOrMaskIndex, 0x8);
QUERY_OFF(type, 0xC);
#undef QUERY_OFF
typedef char GtlInputDescriptorItem_size[(sizeof(GtlInputDescriptorItem) == 0x10) ? 1 : -1];

#endif
