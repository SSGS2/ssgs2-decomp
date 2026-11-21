// CMarker: constructor, resource release, and drawing live in creature_dtor_thunks_and_marker.cpp.
// Layout-only view; no generated vtable or RTTI.
#ifndef CLASSES_CMARKER_H
#define CLASSES_CMARKER_H

class CMarker {
public:
    void* vtbl; // CMarker_Ctor installs __vt__7CMarker.
    float* sourcePositions; // 0x4: CMarker_Draw reads three-float source positions.
    unsigned int pointCount; // 0x8: CMarker_Draw gates and bounds vertex generation.
    void* sourcePositionCapacity;
    unsigned int sourceVertices;
    unsigned int vertexCount;
    unsigned int vertexCapacity;
    void* vertexBuffer; // 0x1C: CMarker_Draw: Resource_GetOffsetPtr, flush, SetVertexStream.
    void* indexBuffer; // 0x20: CMarker_Draw: Engine_SetIndexBuffer.
    void* sprite; // 0x24: CMarker_Draw: W3dSpr_GetBitmap.
};

#define OFF(f, o) typedef char CMarker_off_##f[((int)&((CMarker*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(sourcePositions, 0x4);
OFF(pointCount, 0x8);
OFF(sourcePositionCapacity, 0xC);
OFF(sourceVertices, 0x10);
OFF(vertexCount, 0x14);
OFF(vertexCapacity, 0x18);
OFF(vertexBuffer, 0x1C);
OFF(indexBuffer, 0x20);
OFF(sprite, 0x24);
#undef OFF

inline CMarker* AsCMarker(void* p) { return (CMarker*)p; }

#endif
