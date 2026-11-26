#ifndef CLASSES_CPARTICLEEFFECT_H
#define CLASSES_CPARTICLEEFFECT_H

#include "game/types.h"

struct EffectMessage;
class CParticleEffect {
public:
    void* vtbl; // 0x0: original vtable pointer
    char pad_004[0x14];
    void* taskMainVtable;        // 0x18: CParticleEffect_Create 0x801A8C64, secondary vtable
    void* activeHead;            // 0x1c: OnProcess 0x801A94B4, circular active list head
    void* activeCursor;          // 0x20: OnProcess 0x801A94C0/0x801A94EC advances through activeHead
    void* allocationHead;        // 0x24
    void* freeHead;              // 0x28: OnDestroy 0x801A90D8 returns active nodes to this list
    int nodeAllocationBatchSize; // 0x2c: CParticleEffect_Create initializes to 0x10
    int slotMask;                // 0x30: Create 0x801A8CB4-0x801A8CBC computes slotCount - 1
    int slotCount;               // 0x34: Create 0x801A8CC4-0x801A8CD4 clears slotCount pointers
    void* slotBuckets;           // 0x38
    char allocationMutex[0x18];
    void* effectManager; // 0x54: OnPreLoadInit constructs via ParticleBitmapRegistry_Ctor; OnProcess updates it
    char pad_058[0x10];
    float emissionDurationOverride;    // 0x68
    void* secondListHead;              // 0x6c: Create initializes second list
    void* secondListCursor;            // 0x70: Create clears second list cursor
    void* secondAllocationHead;        // 0x74
    void* secondFreeHead;              // 0x78: Create clears second free list
    int secondNodeAllocationBatchSize; // 0x7c: Create initializes to 0x10
    int secondSlotMask;                // 0x80: Create 0x801A8D48-0x801A8D50 computes secondSlotCount - 1
    int secondSlotCount;               // 0x84: Create 0x801A8D58-0x801A8D68 clears second slot array
    void* secondSlotBuckets;           // 0x88

    char secondAllocationMutex[0x18];
    void* lightColumn; // 0xa4: OnPreLoadInit 0x801A917C/0x801A9180 constructs with CLightColumn_Ctor

    ~CParticleEffect();
    void HandleMsg(EffectMessage* request);
    static void* GetRTTI();
    void DrawThunk18();

    void OnProcess(float dt);
    static void Draw(char* arg);
    void OnInit();
    void OnDestroy();
    void OnPreLoadInit();
};

#define CPARTICLEEFFECT_OFF(f, o) typedef char CParticleEffect_off_##f[((int)&((CParticleEffect*)0)->f == (o)) ? 1 : -1]
CPARTICLEEFFECT_OFF(allocationHead, 0x24);
CPARTICLEEFFECT_OFF(slotBuckets, 0x38);
CPARTICLEEFFECT_OFF(emissionDurationOverride, 0x68);
CPARTICLEEFFECT_OFF(secondAllocationHead, 0x74);
CPARTICLEEFFECT_OFF(secondSlotBuckets, 0x88);
CPARTICLEEFFECT_OFF(taskMainVtable, 0x18);
CPARTICLEEFFECT_OFF(activeHead, 0x1c);
CPARTICLEEFFECT_OFF(activeCursor, 0x20);
CPARTICLEEFFECT_OFF(freeHead, 0x28);
CPARTICLEEFFECT_OFF(nodeAllocationBatchSize, 0x2c);
CPARTICLEEFFECT_OFF(slotMask, 0x30);
CPARTICLEEFFECT_OFF(slotCount, 0x34);
CPARTICLEEFFECT_OFF(effectManager, 0x54);
CPARTICLEEFFECT_OFF(secondListHead, 0x6c);
CPARTICLEEFFECT_OFF(secondListCursor, 0x70);
CPARTICLEEFFECT_OFF(secondFreeHead, 0x78);
CPARTICLEEFFECT_OFF(secondNodeAllocationBatchSize, 0x7c);
CPARTICLEEFFECT_OFF(secondSlotMask, 0x80);
CPARTICLEEFFECT_OFF(secondSlotCount, 0x84);
CPARTICLEEFFECT_OFF(lightColumn, 0xa4);
#undef CPARTICLEEFFECT_OFF

#endif // CLASSES_CPARTICLEEFFECT_H
