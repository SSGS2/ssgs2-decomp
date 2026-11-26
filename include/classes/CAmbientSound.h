#ifndef CLASSES_CAMBIENTSOUND_H
#define CLASSES_CAMBIENTSOUND_H

#include "game/types.h"

struct ActorMessage;
struct AmbientBoxAxes;

// Plays the repeating sounds of the course's ambient source lists. The lists
// themselves belong to the course data; this actor only starts and mixes them.
class CAmbientSound {
public:
    void OnInit();
    void OnProcess(float delta);
    void HandleMsg(const ActorMessage* message);
    unsigned char* GetRTTI();

    // Attenuation of one box source at the listener, 1 inside the box and 0 at
    // fadeDistance or further outside it.
    static float GetVolume(AmbientBoxAxes* box, float fadeDistance);

    // Factory used by the actor registry: allocates and constructs one ambient-sound actor.
    static void* Create();

    // Registers the ambient-sound type and its factory with the actor system.
    static void RegisterType();

    ~CAmbientSound();

    void* vtbl; // 0x0: original vtable pointer
};

#endif // CLASSES_CAMBIENTSOUND_H
