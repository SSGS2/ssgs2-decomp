#ifndef CLASSES_CFX_H
#define CLASSES_CFX_H

#include "game/types.h"

struct Vec2_8010;

class CFx {
public:
    void* vtbl;                  // 0x0: original vtable pointer
    void* secondaryVtable;       // 0x4
    char pad_008[0xc];           // 0x8
    float pendingDeltaTime;      // 0x14
    char effectBox[0x380];       // 0x18: effect box that owns the sequences and sprays
    void* unk_398;               // 0x398
    char pad_39C[0x8];           // 0x39c
    void* unk_3A4;               // 0x3a4
    char pad_3A8[0x4];           // 0x3a8
    unsigned int controlCount;   // 0x3ac
    unsigned int controlRoot;    // 0x3b0
    void* controlFirst;          // 0x3b4
    char updateEnabled;          // 0x3b8
    char paused;                 // 0x3b9
    char attachedEffectsEnabled; // 0x3ba

    void SetPendingDeltaTime(float deltaSeconds);
    void UpdateEffects();
    void UpdateAndDraw();
    void ClearControlsAndResetBox();
    void* CreateSequence(const char* scriptName, int playbackMode);
    int HasSequence(unsigned int key);
    void RemoveSequence(unsigned int key, int mode);
    void RemoveSpray(unsigned int sprayId);
    void ClearEffects(int clearMode);
    void* CreateAttachedEffect(Vec2_8010* sourcePosition, const char* scriptName, unsigned char attachmentFlags,
                               int resourceMode, int cacheMode);

    void SetPaused(unsigned char value);
    void SetUpdateEnabled(unsigned char value);
    void SetAttachedEffectCreationEnabled(unsigned char value);
    void SetWindVector(void* windVector);
    int RemoveControl(int key);

    int ResetAndCreateBoosterWarp(void* value);
    int CreatePowerShot2(void* value);
    int CreateTomahawk(void* value, unsigned char flag);
    int CreateSpike(void* value, unsigned char flag);
    int CreateCobra(void* value);
    int CreatePowerSpin(void* value);
    int MakeShotImpact(void* value);
    int MakeRunDust(void* ballPosition, void* ballVelocity);

    // Destructor entry of the secondary interface vtable: adjust the interface
    // pointer back to the object before running the destructor.
    void* DtorThunk4(int forwarded1);
    ~CFx();

    static CFx* GetInstance();
};

#define CFX_OFF(f, o) typedef char CFx_off_##f[((int)&((CFx*)0)->f == (o)) ? 1 : -1]
CFX_OFF(secondaryVtable, 0x4);
CFX_OFF(pendingDeltaTime, 0x14);
CFX_OFF(unk_398, 0x398);
CFX_OFF(unk_3A4, 0x3a4);
CFX_OFF(controlCount, 0x3ac);
CFX_OFF(controlRoot, 0x3b0);
CFX_OFF(controlFirst, 0x3b4);
CFX_OFF(updateEnabled, 0x3b8);
CFX_OFF(paused, 0x3b9);
CFX_OFF(attachedEffectsEnabled, 0x3ba);
#undef CFX_OFF

#endif // CLASSES_CFX_H
