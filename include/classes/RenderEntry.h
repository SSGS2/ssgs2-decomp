// Renderer command layout from wgeneric3d_render_queue_and_soft_keyboard.cpp; WGeneric3D_ReplaySortedEntriesTwoPass replays flagsOrTextureSlot..primitiveType and resourceOverride state.
#ifndef CLASSES_RENDERENTRY_H
#define CLASSES_RENDERENTRY_H
struct RenderEntry {
        unsigned char state;
        char pad[3];
        float sortKey;
        int flagsOrTextureSlot;
        int vertexCount;
        int vertexFormatOrPrimitiveFlags;
        int vertexData;
        int primitiveType;
        int resourceOverride;
};
#define OFF(f, o) typedef char RenderEntry_off_##f[((int)&((RenderEntry*)0)->f == (o)) ? 1 : -1]
OFF(state, 0x0);
OFF(sortKey, 0x4);
OFF(flagsOrTextureSlot, 0x8);
OFF(vertexCount, 0xC);
OFF(vertexFormatOrPrimitiveFlags, 0x10);
OFF(vertexData, 0x14);
OFF(primitiveType, 0x18);
OFF(resourceOverride, 0x1C);
#undef OFF
typedef char RenderEntry_size[(sizeof(RenderEntry) == 0x20) ? 1 : -1];
#define AsRenderEntry(p) ((RenderEntry*)(p))
#endif
