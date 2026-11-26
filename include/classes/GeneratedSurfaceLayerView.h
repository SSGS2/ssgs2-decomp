// GeneratedSurfaceLayerView: partial layout for methods in the address-named translation unit.
// Field comments identify the function providing layout evidence.
#ifndef CLASSES_TEXTURELAYER801ED364_H
#define CLASSES_TEXTURELAYER801ED364_H

class GeneratedSurfaceLayerView {
public:
    float directionZ;         // 0x0: GeneratedSurface_BuildFromPolygon stores a scaled sine/cosine; role unresolved
    float directionX;         // 0x4: GeneratedSurface_BuildFromPolygon stores the paired scaled direction component
    unsigned int packedColor; // 0x8: GeneratedSurface_BuildFromPolygon ORs s_sceneLighting+0x18 with opaque alpha
    unsigned int textureCoordinates; // 0xC: GeneratedSurfaceLayer_Init
    unsigned int bitmapResource;     // 0x10: GeneratedSurfaceLayer_Init
};

#define OFF(f, o) typedef char TextureLayer801ED364_off_##f[((int)&((GeneratedSurfaceLayerView*)0)->f == (o)) ? 1 : -1]
OFF(directionZ, 0x0);
OFF(directionX, 0x4);
OFF(packedColor, 0x8);
OFF(textureCoordinates, 0xC);
OFF(bitmapResource, 0x10);
#undef OFF

#endif
