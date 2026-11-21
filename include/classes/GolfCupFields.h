// CGolfCup constructor layout in ingame_tutorial_dialog.cpp; separate from its C++ lifetime scaffolding.
#ifndef CLASSES_GOLFCUPFIELDS_H
#define CLASSES_GOLFCUPFIELDS_H
struct GolfCupFields {
    void* vtbl; // 0x0: GolfCupMatrices installs __vt__8CGolfCup.
    char pad_004[0xC];
    int model; // 0x10: GolfCupMatrices clears; meaning unresolved.
    int captureQueue; // 0x14: GolfCupMatrices clears.
    char captureReady; // 0x18: GolfCupMatrices clears.
    char pad_019[3];
    float positionX; // 0x1C: CGolfCup::CGolfCup initializes the three scalar components alike.
    float positionY;
    float positionZ;
    int shadowMesh; // 0x28: passed to GolfCupMesh_CreateScaledTriangles; role unresolved.
    int captureTexture;
    float matrix0[16]; // 0x30: GolfCupMatrices calls PSMTX44Identity.
    float matrix1[16]; // 0x70: GolfCupMatrices calls PSMTX44Identity.
    int savedMaterialResources; // 0xB0: constructor clears three words of the GolfCupList subobject; roles unresolved.
    int savedMaterialResourceCount;
    int savedMaterialResourceCapacity;
};
#define OFF(f, o) typedef char GolfCupFields_off_##f[((int)&((GolfCupFields*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(model, 0x10);
OFF(captureQueue, 0x14);
OFF(captureReady, 0x18);
OFF(positionX, 0x1C);
OFF(positionY, 0x20);
OFF(positionZ, 0x24);
OFF(shadowMesh, 0x28);
OFF(captureTexture, 0x2C);
OFF(matrix0, 0x30);
OFF(matrix1, 0x70);
OFF(savedMaterialResources, 0xB0);
OFF(savedMaterialResourceCount, 0xB4);
OFF(savedMaterialResourceCapacity, 0xB8);
#undef OFF
#endif
