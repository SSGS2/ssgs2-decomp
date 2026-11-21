// GtlMediaRegistry: layout view; methods in gtl_media_pipeline_filter_manager.cpp (gtl_MediaRegistry_Add, gtl_MediaRegistry_Dtor).
// Inferred descriptive type name; no vtable, RTTI, constructors or base classes emitted.
#ifndef CLASSES_GTLMEDIAREGISTRY_H
#define CLASSES_GTLMEDIAREGISTRY_H

class GtlMediaRegistry {
public:
    int count; // 0x0
    void* sentinelPrevious; // 0x4
    void* sentinelNext; // 0x8
};

#define OFF(f, o) typedef char GtlMediaRegistry_off_##f[((int)&((GtlMediaRegistry*)0)->f == (o)) ? 1 : -1]
OFF(count, 0x0);
OFF(sentinelPrevious, 0x4);
OFF(sentinelNext, 0x8);
#undef OFF

#endif
