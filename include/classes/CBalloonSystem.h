// CBalloonSystem: the party mode's balloon manager. It owns the spawned
// balloons, mirrors them into the unhit list, and rebuilds those lists at the
// start of a round.
#ifndef CLASSES_CBALLOONSYSTEM_H
#define CLASSES_CBALLOONSYSTEM_H

#include "game/types.h"

struct UpdateObject;
struct Event;

class CBalloonSystem {
public:
    // Construct the system in place; returns the object.
    CBalloonSystem* Init();

    // Release the balloon queues and the native base. Named rather than a
    // destructor because the generated destructor orders the setup differently.
    CBalloonSystem* Destroy(int deleting);

    // Copy the spawned balloons into the unhit list.
    static void OnInit();

    // Construct and queue one balloon of the given spawn kind; returns the
    // number of spawned balloons after the insertion.
    static int Spawn(void* spawnPosition, int type);

    static void OnProcess(UpdateObject* object);
    static void OnDestroy();
    void HandleMsg(const Event* event);
    static void* GetRTTI();
    static void OnDisplay();

    static void OnPreLoadInit();
    static void OnLoad();

    // Release the shared Stamp Hammer queue and clear the selected balloon.
    static void DestroySharedQueue();

    // Free every queued balloon and list node, then release the shared hammer queue.
    static void DestroyAllQueuedBalloons();

    void* vtbl;                        // 0x00
    char pad_004[0x4];                 // 0x04
    int unk_8;                         // 0x08
    char pad_00C[0xC];                 // 0x0C
    int lastHitSlotIndex;              // 0x18  first word of the cleared slot state
    char pad_01C[0xC];                 // 0x1C
    unsigned int balloonsHitThisFrame; // 0x28
};

#define CBALLOONSYSTEM_OFF(f, o) typedef char CBalloonSystem_off_##f[((int)&((CBalloonSystem*)0)->f == (o)) ? 1 : -1]
CBALLOONSYSTEM_OFF(vtbl, 0x0);
CBALLOONSYSTEM_OFF(unk_8, 0x8);
CBALLOONSYSTEM_OFF(lastHitSlotIndex, 0x18);
CBALLOONSYSTEM_OFF(balloonsHitThisFrame, 0x28);
#undef CBALLOONSYSTEM_OFF
typedef char CBalloonSystem_size[(sizeof(CBalloonSystem) == 0x2C) ? 1 : -1];

#endif // CLASSES_CBALLOONSYSTEM_H
