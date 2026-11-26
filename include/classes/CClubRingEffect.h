// CClubRingEffect: the rotating textured ring that the club and caddie effects
// draw. It holds a texture, two triangle-strip vertex buffers and the world and
// rotation matrices; the strips are emitted through the embedded CGeneTex3D
// primitive at 0xD4. The class name is inferred from that role.
#ifndef CLASSES_CCLUBRINGEFFECT_H
#define CLASSES_CCLUBRINGEFFECT_H

#include "game/types.h"
#include "classes/CGeneTex3D.h"

class CClubRingEffect {
public:
    CClubRingEffect(void* name);
    ~CClubRingEffect();

    void CreateRingVertexBuffers();
    void SetTranslation(void* translation);

    void* texture;         // 0x00  resource-manager id
    float angle;           // 0x04  current ring rotation
    unsigned char enabled; // 0x08
    char pad_09[3];
    void* innerVertices;     // 0x0C
    void* outerVertices;     // 0x10
    float worldMatrix[16];   // 0x14
    float innerRotation[16]; // 0x54
    float outerRotation[16]; // 0x94
    CGeneTex3D primitive;    // 0xD4
};

#define CCLUBRINGEFFECT_OFF(f, o) typedef char CClubRingEffect_off_##f[((int)&((CClubRingEffect*)0)->f == (o)) ? 1 : -1]
CCLUBRINGEFFECT_OFF(texture, 0x00);
CCLUBRINGEFFECT_OFF(angle, 0x04);
CCLUBRINGEFFECT_OFF(enabled, 0x08);
CCLUBRINGEFFECT_OFF(innerVertices, 0x0C);
CCLUBRINGEFFECT_OFF(outerVertices, 0x10);
CCLUBRINGEFFECT_OFF(worldMatrix, 0x14);
CCLUBRINGEFFECT_OFF(innerRotation, 0x54);
CCLUBRINGEFFECT_OFF(outerRotation, 0x94);
CCLUBRINGEFFECT_OFF(primitive, 0xD4);
#undef CCLUBRINGEFFECT_OFF
typedef char CClubRingEffect_size[(sizeof(CClubRingEffect) == 0xE0) ? 1 : -1];

#endif // CLASSES_CCLUBRINGEFFECT_H
