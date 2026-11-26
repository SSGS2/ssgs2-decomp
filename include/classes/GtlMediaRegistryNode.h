// GtlMediaRegistryNode: layout view; methods in gtl_media_pipeline_filter_manager.cpp (gtl_MediaRegistry_Add, gtl_MediaRegistry_RemoveByReference).
// Inferred descriptive type name; no vtable, RTTI, constructors or base classes emitted.
#ifndef CLASSES_GTLMEDIAREGISTRYNODE_H
#define CLASSES_GTLMEDIAREGISTRYNODE_H

class GtlMediaRegistryNode {
public:
    void* previous; // 0x0
    void* next;     // 0x4
    void* value;    // 0x8
};

#define OFF(f, o) typedef char GtlMediaRegistryNode_off_##f[((int)&((GtlMediaRegistryNode*)0)->f == (o)) ? 1 : -1]
OFF(previous, 0x0);
OFF(next, 0x4);
OFF(value, 0x8);
#undef OFF

#endif
