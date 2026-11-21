// WindStateView: layout view owned by wind_regions_and_vectors.cpp.
// Evidence: CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94; CWind_GetWindVectorAtPoint copies direction vectors. No virtual declarations or runtime objects.
#ifndef CLASSES_TRAJECTORYBOUNDS80213768_H
#define CLASSES_TRAJECTORYBOUNDS80213768_H

// CWind_GetWindVectorAtPoint follows adjacentRegion at a 0x3C-byte stride.
struct WindRegionLinkView {
    char pad_00[0x38];
    int adjacentRegion;
};
#define REGION_OFF(f, o) typedef char WindRegion80213768_off_##f[((int)&((WindRegionLinkView*)0)->f == (o)) ? 1 : -1]
REGION_OFF(adjacentRegion, 0x38);
#undef REGION_OFF
typedef char WindRegion80213768_size[(sizeof(WindRegionLinkView) == 0x3C) ? 1 : -1];

class WindStateView {
public:
    char pad_0[0x1C];
    float sampledWindVector[3]; // 0x1C; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    float windStrength; // 0x28; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    char pad_2C[0x4];
    float rotationZ; // 0x30; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    char pad_34[0x24];
    float viewTranslation[3]; // 0x58; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    char pad_64[0x1C];
    float rotationY; // 0x80; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    char pad_84[0x8];
    unsigned char windEnabled; // 0x8C; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    char pad_8D[0x3];
    void* boxes; // 0x90; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    int boxCount; // 0x94; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    unsigned int boxCapacity; // 0x98; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    float direction[3]; // 0x9C; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    float overrideDirection[3]; // 0xA8; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
    char pad_B4[0x8];
    unsigned char useOverrideDirection; // 0xBC; CWind_ContainsPointInRegion iterates 0x3c-byte boxes from +0x90 through count +0x94
};

#define OFF(f, o) typedef char TrajectoryBounds80213768_off_##f[((int)&((WindStateView*)0)->f == (o)) ? 1 : -1]
OFF(sampledWindVector, 0x1C);
OFF(windStrength, 0x28);
OFF(rotationZ, 0x30);
OFF(viewTranslation, 0x58);
OFF(rotationY, 0x80);
OFF(windEnabled, 0x8C);
OFF(boxes, 0x90);
OFF(boxCount, 0x94);
OFF(boxCapacity, 0x98);
OFF(direction, 0x9C);
OFF(overrideDirection, 0xA8);
OFF(useOverrideDirection, 0xBC);
#undef OFF

#endif
