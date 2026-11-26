// RenderPacketFields: layout view owned by methods in retry_menu_and_render_packet.cpp: RenderPacket_Init initializes, RenderPacket_ClearResources walks size entries of 0xC bytes and clears, RenderPacket_Reset resets
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_RENDERPACKETFIELDS_H
#define CLASSES_RENDERPACKETFIELDS_H

struct RenderPacketFields {
    int entries;               // 0x0
    int size;                  // 0x4
    int capacity;              // 0x8
    float matrix[16];          // 0xC: RenderPacket_Init calls PSMTX44Identity
    char auxiliaryState[0x68]; // 0x4C: RenderPacket_Init zeroes this block
    int resetMarker;           // 0xB4
    int loaded;                // 0xB8
};

#define OFF(f, o) typedef char RenderPacketFields_off_##f[((int)&((RenderPacketFields*)0)->f == (o)) ? 1 : -1]
OFF(entries, 0x0);
OFF(size, 0x4);
OFF(capacity, 0x8);
OFF(matrix, 0xC);
OFF(auxiliaryState, 0x4C);
OFF(resetMarker, 0xB4);
OFF(loaded, 0xB8);
#undef OFF
#define AsRenderPacketFields(p) ((RenderPacketFields*)(p))

#endif
