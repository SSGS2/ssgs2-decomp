#ifndef CLASSES_FADESTATE_H
#define CLASSES_FADESTATE_H

#include "game/types.h"

class FadeState {
public:
    void* resource;  // 0x0: FadeState_StartFadeIn/StartFadeOut; FadeState_RequestFramebufferCapture/8023E088/8023E0D0
    float alpha;     // 0x4: FadeState_StartFadeIn/StartFadeOut; FadeState_RequestFramebufferCapture/8023E088/8023E0D0
    float alphaStep; // 0x8: FadeState_StartFadeIn/StartFadeOut; FadeState_RequestFramebufferCapture/8023E088/8023E0D0
    int state;       // 0xc: FadeState_StartFadeIn/StartFadeOut; FadeState_RequestFramebufferCapture/8023E088/8023E0D0
    unsigned char
        captureRequested; // 0x10: FadeState_StartFadeIn/StartFadeOut; FadeState_RequestFramebufferCapture/8023E088/8023E0D0
    unsigned char
        alternateDrawPhase; // 0x11: FadeState_StartFadeIn/StartFadeOut; FadeState_RequestFramebufferCapture/8023E088/8023E0D0
    unsigned char
        alternateDrawEnabled; // 0x12: FadeState_StartFadeIn/StartFadeOut; FadeState_RequestFramebufferCapture/8023E088/8023E0D0
    unsigned char
        releaseResourceOnCompletion; // 0x13: FadeState_StartFadeIn/StartFadeOut; FadeState_RequestFramebufferCapture/8023E088/8023E0D0
};

#define FADESTATE_OFF(f, o) typedef char FadeState_off_##f[((int)&((FadeState*)0)->f == (o)) ? 1 : -1]
FADESTATE_OFF(alpha, 0x4);
FADESTATE_OFF(alphaStep, 0x8);
FADESTATE_OFF(state, 0xc);
FADESTATE_OFF(captureRequested, 0x10);
FADESTATE_OFF(alternateDrawPhase, 0x11);
FADESTATE_OFF(alternateDrawEnabled, 0x12);
FADESTATE_OFF(releaseResourceOnCompletion, 0x13);
FADESTATE_OFF(resource, 0x0);
#undef FADESTATE_OFF

#endif // CLASSES_FADESTATE_H
