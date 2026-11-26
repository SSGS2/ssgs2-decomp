// GtlAudioControllerState: layout view; methods in gtl_audio_controller_effects.cpp (gtl_AudioController_InitMembers initializes; gtl_AudioController_Update interpolates; gtl_AudioController_Initialize installs callback).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_AUDIOCONTROLLER80404E94_H
#define CLASSES_AUDIOCONTROLLER80404E94_H

struct GtlAudioCallbackList {
    unsigned int count;
    void* first;
    void* last;
};
#define OFF(f, o) typedef char AudioCallbackList80404DCC_off_##f[((int)&((GtlAudioCallbackList*)0)->f == (o)) ? 1 : -1]
OFF(count, 0x0);
OFF(first, 0x4);
OFF(last, 0x8);
#undef OFF

class GtlAudioControllerState {
public:
    unsigned int mode;                // 0x0
    unsigned int outputBuffer;        // 0x4
    unsigned int callbackCount;       // 0x8
    unsigned int callbackFirst;       // 0xC
    unsigned int callbackLast;        // 0x10
    void* previousCallback;           // 0x14
    unsigned char initialized;        // 0x18
    unsigned char unk_19;             // 0x19
    unsigned char driveStatusChanged; // 0x1A
    char pad_01B[0x1];
    float masterStart;                    // 0x1C
    float masterEnd;                      // 0x20
    int masterDuration;                   // 0x24
    int masterFrame;                      // 0x28
    float masterFadeStart;                // 0x2C
    float masterFadeEnd;                  // 0x30
    int masterFadeDuration;               // 0x34
    int masterFadeFrame;                  // 0x38
    float masterScaleStart;               // 0x3C
    float masterScaleEnd;                 // 0x40
    int masterScaleDuration;              // 0x44
    int masterScaleFrame;                 // 0x48
    unsigned int updateCallback;          // 0x4C
    unsigned int stopCountdown;           // 0x50
    float channelFade0Start;              // 0x54
    float channelFade0End;                // 0x58
    int channelFade0Duration;             // 0x5C
    int channelFade0Frame;                // 0x60
    float channelFade1Start;              // 0x64
    float channelFade1End;                // 0x68
    int channelFade1Duration;             // 0x6C
    int channelFade1Frame;                // 0x70
    float channelFade2Start;              // 0x74
    float channelFade2End;                // 0x78
    int channelFade2Duration;             // 0x7C
    int channelFade2Frame;                // 0x80
    float channelScale0Start;             // 0x84
    float channelScale0End;               // 0x88
    int channelScale0Duration;            // 0x8C
    int channelScale0Frame;               // 0x90
    float channelScale1Start;             // 0x94
    float channelScale1End;               // 0x98
    int channelScale1Duration;            // 0x9C
    int channelScale1Frame;               // 0xA0
    float channelScale2Start;             // 0xA4
    float channelScale2End;               // 0xA8
    int channelScale2Duration;            // 0xAC
    int channelScale2Frame;               // 0xB0
    GtlAudioCallbackList channelLists[3]; // 0xB4: gtl_AudioChannelList_InitEmpty initializes each list
    unsigned int savedEffectCallback0;    // 0xD8
    unsigned int savedEffectCallback1;    // 0xDC
    unsigned int savedEffectCallback2;    // 0xE0
    unsigned int savedEffectContext0;     // 0xE4
    unsigned int savedEffectContext1;     // 0xE8
    unsigned int savedEffectContext2;     // 0xEC
    unsigned char silentCallbacks[3];     // 0xF0: gtl_AudioController_AttachEffect primes two silent callbacks
};

#define OFF(f, o) typedef char AudioController80404E94_off_##f[((int)&((GtlAudioControllerState*)0)->f == (o)) ? 1 : -1]
OFF(mode, 0x0);
OFF(outputBuffer, 0x4);
OFF(callbackCount, 0x8);
OFF(callbackFirst, 0xC);
OFF(callbackLast, 0x10);
OFF(previousCallback, 0x14);
OFF(initialized, 0x18);
OFF(unk_19, 0x19);
OFF(driveStatusChanged, 0x1A);
OFF(masterStart, 0x1C);
OFF(masterEnd, 0x20);
OFF(masterDuration, 0x24);
OFF(masterFrame, 0x28);
OFF(masterFadeStart, 0x2C);
OFF(masterFadeEnd, 0x30);
OFF(masterFadeDuration, 0x34);
OFF(masterFadeFrame, 0x38);
OFF(masterScaleStart, 0x3C);
OFF(masterScaleEnd, 0x40);
OFF(masterScaleDuration, 0x44);
OFF(masterScaleFrame, 0x48);
OFF(updateCallback, 0x4C);
OFF(stopCountdown, 0x50);
OFF(channelFade0Start, 0x54);
OFF(channelFade0End, 0x58);
OFF(channelFade0Duration, 0x5C);
OFF(channelFade0Frame, 0x60);
OFF(channelFade1Start, 0x64);
OFF(channelFade1End, 0x68);
OFF(channelFade1Duration, 0x6C);
OFF(channelFade1Frame, 0x70);
OFF(channelFade2Start, 0x74);
OFF(channelFade2End, 0x78);
OFF(channelFade2Duration, 0x7C);
OFF(channelFade2Frame, 0x80);
OFF(channelScale0Start, 0x84);
OFF(channelScale0End, 0x88);
OFF(channelScale0Duration, 0x8C);
OFF(channelScale0Frame, 0x90);
OFF(channelScale1Start, 0x94);
OFF(channelScale1End, 0x98);
OFF(channelScale1Duration, 0x9C);
OFF(channelScale1Frame, 0xA0);
OFF(channelScale2Start, 0xA4);
OFF(channelScale2End, 0xA8);
OFF(channelScale2Duration, 0xAC);
OFF(channelScale2Frame, 0xB0);
OFF(channelLists, 0xB4);
OFF(savedEffectCallback0, 0xD8);
OFF(savedEffectCallback1, 0xDC);
OFF(savedEffectCallback2, 0xE0);
OFF(savedEffectContext0, 0xE4);
OFF(savedEffectContext1, 0xE8);
OFF(savedEffectContext2, 0xEC);
OFF(silentCallbacks, 0xF0);
#undef OFF

#define AsAudioController80404E94(p) ((GtlAudioControllerState*)(p))

#endif
