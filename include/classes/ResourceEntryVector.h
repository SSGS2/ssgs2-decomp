// ResourceEntryVector: non-virtual layout view; methods/accesses in resource_manager_cache_and_loading.cpp.
// No inheritance or virtual declarations: the original vtables remain external symbols.
#ifndef CLASSES_RESOURCEENTRYVECTOR_H
#define CLASSES_RESOURCEENTRYVECTOR_H

class ResourceEntryVector {
public:
    void* vtable; // 0x0: ResourceEntryVector_FindByName/ResourceCache_RetainOrAppend start entries at +4
    void* entries; // 0x4: ResourceEntryVector_FindByName scans 16-byte entries
    unsigned count; // 0x8: ResourceCache_RetainOrAppend increments after insertion
    unsigned capacity; // 0xC: ResourceCache_RetainOrAppend checks/grows allocation
};

#define OFF(f, o) typedef char ResourceEntryVector_off_##f[((int)&((ResourceEntryVector*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(entries, 0x4);
OFF(count, 0x8);
OFF(capacity, 0xC);
#undef OFF

#endif
