#include <decomp/static_init.h>
// Animation ranges expose their first and last frame at separate ABI words.
struct RoundAnimationRangeView { float firstFrame; int eventData; float lastFrame; };
// Rotation updates are per call: speed decays once and angle -= speed * 0.5.
// No delta-time argument is applied here; animation range endpoints are frame values.
// Animation IDs 0..6 select the dolphin, 7..10 the capsule; the animation names below
// define the exact ranges. Unknown IDs still latch playing state without selecting a range.
enum RoundDolConstants {
    kMotionVectorBytes = 12, kMaxPadMotionSamples = 32, kResampledMotionSamples = 10,
    kRotateAction = 0x0F, kFirstCapsuleAnimation = 7,
    kRoundAnimationStopped = 0, kRoundAnimationPlaying = 1,
    kAnimNoMotion = 0, kAnimEntrance = 1, kAnimRotateLow = 2,
    kAnimRotateMiddle = 3, kAnimRotateHigh = 4, kAnimRotateSuper = 5,
    kAnimRotateOver = 6, kAnimCapsuleLow = 7, kAnimCapsuleMiddle = 8,
    kAnimCapsuleHigh = 9, kAnimCapsuleSuper = 10
};

#include "classes/CRoundDol3D.h"
#define AsCRoundDol3D(p) ((CRoundDol3D*)(p))

// Translation unit 0x803156A4-0x80316848 (28 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "game/wvector.h"
#include "classes/CRoundDolMain.h"
#include "classes/CRoundDolTask.h"

#define AsCRoundDolTask(p) ((CRoundDolTask*)(p))

#define AsCRoundDolMain(p) ((CRoundDolMain*)(p))
namespace std { struct nothrow_t {}; }
extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u803156A4 {
    const unsigned int* words;
    bool active;
    // Arm cleanup for a completed inline/heap string until ownership is transferred.
    explicit StringUnwindCleanup_u803156A4(const void* string)
        : words((const unsigned int*)string), active(true) {}
    // Disarm cleanup after explicit ownership transfer or manual destruction.
    void release() { active = false; }
    // Free heap text on exit unless explicit release disarmed this cleanup.
    ~StringUnwindCleanup_u803156A4() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};


// Opaque legacy ABI parameter; each method uses its actual class layout below.
typedef void RoundDolReceiver;

extern unsigned int CVCGPlayer_StartGroupPlayback(unsigned int, unsigned int, int);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void ParseConditionedQuotedToken(unsigned int, unsigned int, unsigned int, unsigned int,
                        unsigned int);
extern void AnimationEventParser_ProcessFrameRange(void*, void*, float, float, void*, int, int); /* (self, manager, bounds, x f1, y f2, r6, r7) */
extern void CTask_RequestTransitionWithFade(void*, void*, void*, int);
extern void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
extern void SoundManager_SetNamedVolumeScale(void*, const char*, float);
extern void FxAttachment_ParseBoneReferences(RoundDolReceiver*, unsigned int, unsigned int,
                        unsigned int, unsigned int, unsigned int, unsigned int);
extern void FxBox_CreateAndAttachEffectByExtension(RoundDolReceiver*, void*, unsigned int, void*);
extern void ShadowModel_SetNamedProjectionEnabled(unsigned int, const char*, unsigned int);
extern void __ct__18NativeBase80201D18Fv(void*);
extern void DolfiniSession_dtor(unsigned int, unsigned int);
extern unsigned int CRoundDolMain_UpdateStateMachine(unsigned int);
extern void SaveProcess_SetTransitionTargetName(void*);
extern void SaveProcess_SetFollowUpTaskName(void*);
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void __dl__FPv(void*);
extern void Matrix_SetAxisRotation(void*, float, int);
extern void Model_SetMatrixRecursive(void*, void*, void*);
extern void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
extern char __vt__13CRoundDolMain[];
extern void* g_taskManager;
extern char roundEndTaskTransitionStrings[];
extern void CRoundDolMain_DrawStateUI(unsigned int);
extern unsigned int g_soundManager;
extern unsigned int g_shadowManager;
extern const char roundDol3DStrings[];

// Packed external string pool; member extents come from the global catalog.
struct RoundDolStringPool {
    char roundSound[20];
    char dolphinModel[13];
    char standModel[19];
    char backgroundModel[11];
    char capsuleModel[21];
    char cameraMotion[19];
    char backgroundLight[17];
    char turnSound[17];
    char dolphinShadow[8];
    char ceilingLight[10];
    char capsuleShadow[5];
    char background[3];
    char dolphinBackBone[17];
    char capsuleBone[14];
    char entrance[21];
    char noMotion[22];
    char rotateLow[23];
    char rotateMiddle[26];
    char rotateHigh[24];
    char rotateSuper[25];
    char rotateOver[24];
    char capsuleLow[31];
    char capsuleMiddle[34];
    char capsuleHigh[32];
    char capsuleSuper[33];
    char padding[3];
};
#define RoundDolNames ((RoundDolStringPool*)roundDol3DStrings)

extern const float kFloat_0_RoundScene;
extern const float kFloat_6_2831855;
extern float s_roundDol3DTwoPi;
extern float kRoundDolEffectScreenCenterX;
extern const float kRoundDolEffectScreenCenterY;
extern const float kFloat_0_5_RoundSound;
extern const float kFloat_200_RoundSound;
extern const float kFloat_1_805E13D8;
extern const float kFloat_10_RoundScene;extern float kFloat_neg1024_RoundDol3D;extern float kFloat_0_1_RoundScene;
extern const float kRoundDolMotionAxisMaximum;extern float kRoundDolMotionAxisMinimum;extern float kRoundDolMotionMagnitudeScale;extern float kRoundDolMotionAccelerationThreshold;
extern const float kRoundDolMotionAccelerationDivisor;extern float kFloat_2_4434605_RoundDol3D;extern float kRoundDolActiveRotationDecay;extern float kRoundDolRotationSoundBase;
extern const float kRoundDolRotationVolumeSlope;extern float kRoundDolSpeedLevel7Threshold;extern float kRoundDolSpeedLevel6Threshold;extern float kRoundDolSpeedLevel5Threshold;
extern const float kRoundDolSpeedLevel4Threshold;extern float kRoundDolSpeedLevel3Threshold;extern float kRoundDolSpeedLevel2Threshold;
extern const float kRoundDolIdleRotationDecay;
extern unsigned int g_view;
extern unsigned int g_project;
extern unsigned int g_defaultActorSkipMask;
extern unsigned int g_actorSkipMask;
extern char __vt__13CRoundDolTask[];
extern int Parse_NonemptyQuotedTokenUnder64Bytes(unsigned int, unsigned int, void*);
extern void Sound_PlayByName(unsigned int, void*, int, int, int, int, float, float, float);
extern void ShadowManager_RenderShadowTextures(unsigned int, unsigned int);
extern unsigned int CProjectG_GetExtensionProperty(unsigned int);
extern void Engine_ClearFramebuffer(int, int, int, unsigned int, float, int);
extern void ShadowManager_DrawProjectedModel(unsigned int, unsigned int);
extern void CTask_Ctor(void*);
extern void* WPuppet_FindAnimationRangeByName(void*, void*);
extern void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
extern void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
extern void* CTask_SetMainActor(void*, void*);
extern void appFreeUni__FPv(void*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
extern void* SaveData_GetCurrentRecord(unsigned int);
extern void SaveData_Ctor(void*);
extern void* SaveRecord_CopyCtor(void*, void*);
extern void* __nw__FUl(unsigned long);
extern void* DolfiniSession_Ctor(void*, void*);
extern void DolfiniSession_LoadResources(void*);
extern char s_saveData[];
extern signed char s_saveDataInitialized;
extern void DolfiniSession_dtor(unsigned int, unsigned int);
extern unsigned int strlen(const char*);
extern void Engine_SetCachedRenderState(int, int);
extern void WPuppet_SetRendererOption(void*, int);
extern void CFxBox_DrawSprays(void*, void*);
extern void Vector_CopyComponents(void*, void*);
extern void Vec3_Lerp(void*, void*, void*, float);
extern double __fabs(double);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_roundDolInputManagerDtorRegistration[];
extern void InputManager_Init(void*);
extern void InputManager_Dtor(void*);
extern void __register_global_object(void*, void*, void*);
extern int Input_IsActionButtonDown(void*, int, int);
extern void* InputManager_GetSelectedPad(void*);
extern void gtl_CInputExDeviceBase_GetDeviceResult(void*, void*);
extern void gtl_CScreenPointer_GetSample(void*, int, void*);
extern float Vec3_Length(void*);
extern void SoundManager_SetNamedEffectPitch(void*, const char*, float);
extern int CRoundDol3D_ResampleMotion(RoundDolReceiver*, void*, void*, int);

// Return the current discrete speed selection used by the dolphin scene.
unsigned int CRoundDol3D_GetSpeedLevel(RoundDolReceiver* self) {
    return AsCRoundDol3D(self)->speedLevel;
}

// Return the current dolphin scene state without advancing it.
unsigned int CRoundDol3D_GetState(RoundDolReceiver* self) {
    return AsCRoundDol3D(self)->state;
}

// Enable or disable the scene's camera-target override.
void CRoundDol3D_SetCameraTargetOverrideEnabled(RoundDolReceiver* self, unsigned int value) {
    AsCRoundDol3D(self)->cameraTargetOverrideEnabled = value;
}

// Mute the Gurudol rotation sound by its named sound channel.
void Sound_SetGurudolRoundVolume() {
    SoundManager_SetNamedVolumeScale((void*)g_soundManager, roundDol3DStrings, kFloat_0_RoundScene);
}

// Reset motion/animation state, select the entrance endpoint and rebuild the dolphin transform.
void CRoundDol3D_ResetRotation(RoundDolReceiver* self) {
    char buf[0x30];
    float f1 = kFloat_0_RoundScene;
    // This reset-only word remains distinct from the speed, animation and state fields.
    AsCRoundDol3D(self)->unk_B4 = 0;
    AsCRoundDol3D(self)->rotationAngle = f1;
    AsCRoundDol3D(self)->rotationSpeed = f1;
    AsCRoundDol3D(self)->speedLevel = 0;
    AsCRoundDol3D(self)->animation = 0;
    AsCRoundDol3D(self)->animationTime = f1;
    AsCRoundDol3D(self)->animationEnd = f1;
    AsCRoundDol3D(self)->state = kRoundAnimationStopped;
    ((void (*)(void*, float, int))Matrix_SetAxisRotation)(buf, f1, 0);
    f1 = ((RoundAnimationRangeView*)WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->dolphinModel, (void*)(RoundDolNames->entrance)))->lastFrame;
    AsCRoundDol3D(self)->animationTime = f1;
    WPuppet_SetAnimationTimeFromRanges(AsCRoundDol3D(self)->dolphinModel, 0, f1, 0, 0);
    Model_SetMatrixRecursive(AsCRoundDol3D(self)->dolphinModel, AsCRoundDol3D(self)->dolphinModel, buf);
    WPuppet_UpdateBoneWorldMatrices(AsCRoundDol3D(self)->dolphinModel, 0, 0, kFloat_1_805E13D8);
}

// Start the requested camera-motion group with the player's default playback options.
unsigned int CRoundDol3D_StartGroupPlayback(RoundDolReceiver* self, unsigned int value) {
    return CVCGPlayer_StartGroupPlayback((*(unsigned int*)&AsCRoundDol3D(self)->cameraMotionPlayer), value, 0);
}

// Copy the camera target position vector from another round scene.
void CRoundDol3D_vfunc_24(char* self, char* other) {
    AsCRoundDol3D(self)->cameraTarget = AsCRoundDol3D(other)->cameraTarget;
}

// Forward a conditioned quoted token to the event parser with default options.
void CRoundDol3D_ParseConditionedQuotedTokenDefault(unsigned int a, unsigned int b, unsigned int c, unsigned int d) {
    ParseConditionedQuotedToken(a, b, d, 0, 0);
}

// Enable or disable projected shadows for the capsule model.
void CRoundDol3D_SetCapsuleShadowEnabled(unsigned int a, unsigned int value) {
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager,RoundDolNames->capsuleShadow, value != 0);
}

// Enable or disable projected shadows for the dolphin model.
void CRoundDol3D_SetDolphinShadowEnabled(unsigned int a, unsigned int value) {
    ShadowModel_SetNamedProjectionEnabled(g_shadowManager,RoundDolNames->dolphinShadow, value != 0);
}

// Seed the effect attachment position and create the requested effect in this scene's effect box.
void CRoundDol3D_vfunc_25(RoundDolReceiver* self, void* output, unsigned int value) {
    void* data = AsCRoundDol3D(self)->effectBox;
    float y = kRoundDolEffectScreenCenterY;
    float x = kRoundDolEffectScreenCenterX;
    ((float*)output)[0] = x;
    ((float*)output)[1] = y;
    FxBox_CreateAndAttachEffectByExtension(self, output, value, data);
}

// Parse attachment bone references using the active model and animation range.
void CRoundDol3D_ParseBoneReferences(RoundDolReceiver* self, unsigned int b, unsigned int c) {
    FxAttachment_ParseBoneReferences(self, b, c, AsCRoundDol3D(self)->activeModel, AsCRoundDol3D(self)->animation, 1, 0);
}

// Initialize the round-scene module's cached full-turn angle.
void RoundDol3D_InitTwoPiCache() {
    s_roundDol3DTwoPi = kFloat_6_2831855;
}

// Construct the task base, clear the session and install the main/interface vtables.
RoundDolReceiver* CRoundDolMain_Ctor(RoundDolReceiver* self) {
    __ct__18NativeBase80201D18Fv(self);
    (*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession) = 0;
    (*(unsigned int*)&AsCRoundDolMain(self)->vtbl) = (unsigned int)__vt__13CRoundDolMain;
    AsCRoundDolMain(self)->taskMainVtable = (unsigned int)(__vt__13CRoundDolMain + 0x3C);
    return self;
}

// Draw the active Dolfini session's state UI.
void CRoundDolMain_OnDisplay(RoundDolReceiver* self) {
    if ((*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession) != 0) {
        CRoundDolMain_DrawStateUI((*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession));
    }
}

// Destroy and clear the owned Dolfini session.
void CRoundDolMain_OnDestroy(RoundDolReceiver* self) {
    if ((*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession) != 0) {
        DolfiniSession_dtor((*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession), 1);
        (*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession) = 0;
    }
}

// Parse a sound name from the event and play it with the scene's default sound parameters.
void CRoundDol3D_vfunc_2(unsigned int a, unsigned int b) {
    char buf[0x48];
    if (Parse_NonemptyQuotedTokenUnder64Bytes(a, b, buf) != 0) {
        Sound_PlayByName(g_soundManager, buf, 0, 0, 0, 0, kFloat_0_5_RoundSound, kFloat_200_RoundSound, kFloat_1_805E13D8);
    }
}

// Draw capsule/dolphin/stand models and effects, then restore the camera option.
void CRoundDol3D_DrawModelsAndEffects(RoundDolReceiver* self) {
    Engine_SetCachedRenderState(2, 1);
    Engine_SetCachedRenderState(6, 1);
    Engine_SetCachedRenderState(3, 3);
    Engine_SetCachedRenderState(5, 1);
    if ((*(int*)&AsCRoundDol3D(self)->animationIndex) >= kFirstCapsuleAnimation) {
        void* obj = AsCRoundDol3D(self)->capsuleModel;
        WPuppet_SetRendererOption(obj, 2);
        obj = AsCRoundDol3D(self)->capsuleModel;
        void* arg = (void*)g_view;
        void (*fp)(void*, void*, int, int, int, float, int) =
            *(void (**)(void*, void*, int, int, int, float, int))((char*)*(void**)obj + 0x18); /* 0x80315BC4, 0x80315BF0, 0x80315C1C: lwz r12,0x18(r12) loads the slot; ours called the table address */
        fp(obj, arg, 1, 1, 0, kFloat_1_805E13D8, 0);
    }
    {
        void* obj = AsCRoundDol3D(self)->dolphinModel;
        void* arg = (void*)g_view;
        void (*fp)(void*, void*, int, int, int, float, int) =
            *(void (**)(void*, void*, int, int, int, float, int))((char*)*(void**)obj + 0x18); /* 0x80315BC4, 0x80315BF0, 0x80315C1C: lwz r12,0x18(r12) loads the slot; ours called the table address */
        fp(obj, arg, 1, 1, 0, kFloat_1_805E13D8, 0);
    }
    {
        void* obj = AsCRoundDol3D(self)->dolphinStandModel;
        void* arg = (void*)g_view;
        void (*fp)(void*, void*, int, int, int, float, int) =
            *(void (**)(void*, void*, int, int, int, float, int))((char*)*(void**)obj + 0x18); /* 0x80315BC4, 0x80315BF0, 0x80315C1C: lwz r12,0x18(r12) loads the slot; ours called the table address */
        fp(obj, arg, 1, 1, 0, kFloat_1_805E13D8, 0);
    }
    CFxBox_DrawSprays(AsCRoundDol3D(self)->effectBox, (void*)g_view);
    {
        void* obj = (void*)g_view;
        void (*fp)(void*, int) = *(void (**)(void*, int))((char*)*(void**)obj + 0x10); /* 0x80315C40 lwz r12,0x10(r12) */
        fp(obj, 4);
    }
}
// Render shadow textures, clear with the project color, and draw projected shadows.
void CRoundDolMain_RenderShadowPass() {    ShadowManager_RenderShadowTextures(g_shadowManager, g_view);
    unsigned int r = CProjectG_GetExtensionProperty(g_project);
    Engine_ClearFramebuffer(0, 0, 7, r, kFloat_1_805E13D8, 0);
    ShadowManager_DrawProjectedModel(g_shadowManager, g_view);
}

// Construct the round task interfaces and clear actor skipping masks.
void* CRoundDolTask_Ctor(void* self) {
    CTask_Ctor(self);
    (*(unsigned int*)&AsCRoundDolTask(self)->vtbl) = (unsigned int)__vt__13CRoundDolTask;
    AsCRoundDolTask(self)->secondaryVtable = (unsigned int)__vt__13CRoundDolTask + 0x10;
    AsCRoundDolTask(self)->tertiaryVtable = (unsigned int)__vt__13CRoundDolTask + 0x1C;
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
    return self;
}

// Reduce angular speed to zero, advance the angle and rebuild the dolphin pose.
void CRoundDol3D_DecelerateRotation(RoundDolReceiver* self) {
    float f1 = kRoundDolIdleRotationDecay;
    float f0 = kFloat_0_RoundScene;
    char buf[0x30];
    float f2 = AsCRoundDol3D(self)->rotationSpeed;
    f1 = f2 - f1;
    AsCRoundDol3D(self)->rotationSpeed = f1;
    if (f0 >= f1) {
        AsCRoundDol3D(self)->rotationSpeed = f0;
    }
    f1 = AsCRoundDol3D(self)->rotationAngle;
    f0 = kFloat_neg1024_RoundDol3D;
    if (f0 >= f1) {
        f0 = kFloat_0_RoundScene;
        AsCRoundDol3D(self)->rotationAngle = f0;
    }
    f2 = AsCRoundDol3D(self)->rotationSpeed;
    f1 = kFloat_0_5_RoundSound;
    f0 = AsCRoundDol3D(self)->rotationAngle;
    f1 = f0 - (f2 * f1);
    AsCRoundDol3D(self)->rotationAngle = f1;
    ((void (*)(void*, float, int))Matrix_SetAxisRotation)(buf, f1, 0);
    Model_SetMatrixRecursive(AsCRoundDol3D(self)->dolphinModel, AsCRoundDol3D(self)->dolphinModel, buf);
    WPuppet_UpdateBoneWorldMatrices(AsCRoundDol3D(self)->dolphinModel, 0, 0, kFloat_1_805E13D8);
}

// Destroy the owned session and task base, then optionally free the main object.
void* CRoundDolMain_dtor(void* self, int flag) {
    if (self != 0) {
        void* session = AsCRoundDolMain(self)->dolfiniSession;
        (*(unsigned int*)&AsCRoundDolMain(self)->vtbl) = (unsigned int)__vt__13CRoundDolMain;
        AsCRoundDolMain(self)->taskMainVtable = (unsigned int)__vt__13CRoundDolMain + 0x3C;
        if (session != 0) {
            DolfiniSession_dtor((unsigned int)session, 1);
            AsCRoundDolMain(self)->dolfiniSession = 0;
        }
        if (self != 0) {
            __dt__18NativeBase80201D18Fv(self, 0);
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Advance the session state machine and request the follow-up task when it finishes.
void CRoundDolMain_OnProcess(RoundDolReceiver* self) {
    unsigned int p = (*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession);
    if (p == 0) return;
    if (CRoundDolMain_UpdateStateMachine(p) == 0) return;
    char* base = roundEndTaskTransitionStrings;
    SaveProcess_SetTransitionTargetName(base);
    base = roundEndTaskTransitionStrings;
    SaveProcess_SetFollowUpTaskName(base + 0x0B);
    CTask_RequestTransitionWithFade(g_taskManager, roundEndTaskTransitionStrings + 0x16, roundEndTaskTransitionStrings + 0x27, 0);
    TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, roundEndTaskTransitionStrings + 0x28, 7, 0, 0, 0, 0);
}

// Build the named round actors and install the main actor.
void CRoundDolTask_CreateActors(RoundDolReceiver* self) {
    char* base = roundEndTaskTransitionStrings;
    char* p = base + 0x27;
    unsigned int len = strlen(p);
    unsigned int obj[3] = {0, 0, 0};
    char tmp = 0;
    String_ReserveCapacity(obj, len);
    tmp = (char)0;
    String_ReplaceRange(obj, 0, 0, p, p + len, &tmp);
    StringUnwindCleanup_u803156A4 stringCleanup0(obj);
    void* r = TaskActorTable_CreateNamedActor(self, base + 0x38, base + 0x46, 0, obj);
    CTask_SetMainActor(self, r);
    if (obj[0] & 0x80000000u) {
        { stringCleanup0.release(); appFreeUni__FPv((void*)obj[2]); }
    }
}

struct Fn8030E168Obj {
    char pad[0x89c];
    // Construct an owned Dolfini session from the supplied save-record copy.
    Fn8030E168Obj(void* arg) { DolfiniSession_Ctor(this, arg); } };
// Create a session from a copy of the current save record and load its resources.
void CRoundDolMain_OnLoad(RoundDolReceiver* self) {
    char bufA[0x828];
    char bufB[0x828];
    if ((*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession) != 0) {
        DolfiniSession_dtor((*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession), 1);
        (*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession) = 0;
    }
    if (((unsigned int&)Sing_805DC5E0()) == 0) {
        if (s_saveDataInitialized == 0) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5E0()) = (unsigned int)s_saveData;
    }
    // Copy the save record into constructor input; the newly allocated session is owned by self.
    unsigned int p = ((unsigned int&)Sing_805DC5E0());
    void* r = SaveData_GetCurrentRecord(p);
    SaveRecord_CopyCtor(bufA, r);
    void* obj = new Fn8030E168Obj(SaveRecord_CopyCtor(bufB, bufA));
    (*(unsigned int*)&AsCRoundDolMain(self)->dolfiniSession) = (unsigned int)obj;
    DolfiniSession_LoadResources(obj);
}

// Resample to ten vectors using the prior endpoint, then cache the first input vector.
// outputSamples has room for ten vectors. inputSamples is writable scratch: the previous
// endpoint is written at inputCount (or slot 31 for >=32 inputs), so reserve 32 slots.
// Zero inputs holds lastMotionSample and returns 0; otherwise cache input[0] and return 1.
// Negative counts are outside the contract; raw motion coordinate units are unresolved.
int CRoundDol3D_ResampleMotion(RoundDolReceiver* self, void* outputSamples, void* inputSamples, int inputCount) {
    void* scene = self;
    void* outputCursor = outputSamples;
    void* inputCursor = inputSamples;
    int sampleIndex = inputCount;
    if (inputCount == 0) {
        sampleIndex = 0;
        float outputCountForHold = kFloat_10_RoundScene;
        while ((float)sampleIndex < outputCountForHold) {
            Vector_CopyComponents(outputCursor, AsCRoundDol3D(scene)->previousFirstMotionSample);
            outputCursor = (char*)outputCursor + kMotionVectorBytes;
            sampleIndex++;
        }
        return 0;
    }
    int inputIntervalCount;
    if (inputCount < kMaxPadMotionSamples) {
        Vector_CopyComponents((char*)inputCursor + (inputCount * kMotionVectorBytes), AsCRoundDol3D(scene)->previousFirstMotionSample);
        inputIntervalCount = inputCount;
    } else {
        Vector_CopyComponents((char*)inputCursor + (kMaxPadMotionSamples - 1) * kMotionVectorBytes, AsCRoundDol3D(scene)->previousFirstMotionSample);
        inputIntervalCount = kMaxPadMotionSamples - 1;
    }
    sampleIndex = 0;
    float outputCount = kFloat_10_RoundScene;
    float outputsPerInput = kFloat_10_RoundScene / (float)inputIntervalCount;
    while ((float)sampleIndex < outputCount) {
        int inputIndex = (int)((float)sampleIndex / outputsPerInput);
        Vec3_Lerp(outputCursor, (char*)inputCursor + (inputIndex * kMotionVectorBytes), (char*)inputCursor + ((inputIndex + 1) * kMotionVectorBytes),
                    ((float)sampleIndex - (float)inputIndex * outputsPerInput) / outputsPerInput);
        outputCursor = (char*)outputCursor + kMotionVectorBytes;
        sampleIndex++;
    }
    Vector_CopyComponents(AsCRoundDol3D(scene)->previousFirstMotionSample, inputCursor);
    return 1;
}

// Clamp and accumulate pad motion, update rotation speed/sound, then classify its speed level.
// Return 0 when the rotate action is released; return 1 while held, including no selected pad.
// The reverse 32..1 accumulation below intentionally includes the count word as float bits;
// do not change its window to match the ten resampled vectors in a readability patch.
int CRoundDol3D_UpdateRotationFromPadMotion(RoundDolReceiver* self) {
    char rawSamples[0x180];
    char padSample[kMotionVectorBytes];
    if (((unsigned int&)Sing_805DC5C8()) == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_roundDolInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5C8()) = (unsigned int)s_gameInputManager;
    }
    if (Input_IsActionButtonDown((void*)((unsigned int&)Sing_805DC5C8()), kRotateAction, -1) == 0) {
        AsCRoundDol3D(self)->unk_B4 = 0;
        return 0;
    }
    if (((unsigned int&)Sing_805DC5C8()) == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_roundDolInputManagerDtorRegistration);
            s_inputManagerInitialized = 1;
        }
        ((unsigned int&)Sing_805DC5C8()) = (unsigned int)s_gameInputManager;
    }
    void* selectedPad = InputManager_GetSelectedPad((void*)((unsigned int&)Sing_805DC5C8()));
    if (selectedPad == 0) {
        return 1;
    }
    // Collect up to 32 raw pad vectors, then resample to the ten-vector output window.
    unsigned int* sampleCount = &AsCRoundDol3D(self)->motionSampleCount;
    gtl_CInputExDeviceBase_GetDeviceResult(selectedPad, sampleCount);
    char* rawSampleCursor = rawSamples;
    int rawSampleIndex = 0;
    while (rawSampleIndex < (int)(*sampleCount)) {
        if (rawSampleIndex >= kMaxPadMotionSamples) {
            (*sampleCount) = kMaxPadMotionSamples;
            break;
        }
        gtl_CScreenPointer_GetSample(selectedPad, rawSampleIndex, padSample);
        Vector_CopyComponents(rawSampleCursor, padSample);
        rawSampleCursor += kMotionVectorBytes;
        rawSampleIndex++;
    }
    CRoundDol3D_ResampleMotion(self, AsCRoundDol3D(self)->motionSamples, rawSamples, (*sampleCount));
    (*sampleCount) = kResampledMotionSamples;
    // Clamp the ten resampled vectors to the original +/-3.4 axis limits.
    float f2 = kRoundDolMotionAxisMaximum;
    float f1 = kRoundDolMotionAxisMinimum;
    char* clampedSampleCursor = AsCRoundDol3D(self)->motionSamples;
    for (int i = 0; i < (*sampleCount); i++) {
        float v0 = ((WVector*)clampedSampleCursor)->x;
        if (f2 < v0) ((WVector*)clampedSampleCursor)->x = f2;
        else if (v0 < f1) ((WVector*)clampedSampleCursor)->x = f1;
        float v1 = ((WVector*)clampedSampleCursor)->y;
        if (f2 < v1) ((WVector*)clampedSampleCursor)->y = f2;
        else if (v1 < f1) ((WVector*)clampedSampleCursor)->y = f1;
        float v2 = ((WVector*)clampedSampleCursor)->z;
        if (f2 < v2) ((WVector*)clampedSampleCursor)->z = f2;
        else if (v2 < f1) ((WVector*)clampedSampleCursor)->z = f1;
        clampedSampleCursor += kMotionVectorBytes;
    }
    // Preserve the original reverse window: vector slots 32 through 1.
    // Slot 32 starts at motionSampleCount; changing this to 31..0 changes behavior.
    float totalMotionMagnitude = kFloat_0_RoundScene;
    for (int i = 0; i < kMaxPadMotionSamples; i++) {
        totalMotionMagnitude += Vec3_Length(AsCRoundDol3D(self)->motionSamples + (kMaxPadMotionSamples - i) * kMotionVectorBytes);
    }
    // Convert accumulated magnitude into acceleration, cap the speed, then apply active decay.
    float f0 = kRoundDolMotionAccelerationThreshold;
    f1 = kRoundDolMotionMagnitudeScale;
    f1 = totalMotionMagnitude * f1;
    f1 = (float)__fabs((double)f1);
    if (f1 > f0) {
        float motionAcceleration = kRoundDolMotionAccelerationDivisor;
        float acceleratedSpeed = AsCRoundDol3D(self)->rotationSpeed;
        motionAcceleration = f1 / motionAcceleration;
        acceleratedSpeed = acceleratedSpeed + motionAcceleration;
        AsCRoundDol3D(self)->rotationSpeed = acceleratedSpeed;
    }
    float currentSpeed = AsCRoundDol3D(self)->rotationSpeed;
    float maximumSpeed = kFloat_2_4434605_RoundDol3D;
    if (maximumSpeed < currentSpeed) {
        AsCRoundDol3D(self)->rotationSpeed = maximumSpeed;
    }
    float speedBeforeDecay = AsCRoundDol3D(self)->rotationSpeed;
    float decayedSpeed = kRoundDolActiveRotationDecay;
    float zeroSpeed = kFloat_0_RoundScene;
    decayedSpeed = speedBeforeDecay - decayedSpeed;
    AsCRoundDol3D(self)->rotationSpeed = decayedSpeed;
    if (decayedSpeed <= zeroSpeed) {
        AsCRoundDol3D(self)->rotationSpeed = zeroSpeed;
    }
    // Map speed to named-sound volume/pitch; values at or below the threshold mute it.
    float soundSpeed = AsCRoundDol3D(self)->rotationSpeed;
    float soundStartSpeed = kFloat_0_1_RoundScene;
    if (soundSpeed > soundStartSpeed) {
        float soundMaximumSpeed = kFloat_2_4434605_RoundDol3D;
        float volumeSlope = kRoundDolRotationVolumeSlope;
        float normalizedSpeed = soundSpeed / soundMaximumSpeed;
        float soundBase = kRoundDolRotationSoundBase;
        float basePitch = kFloat_0_5_RoundSound;
        SoundManager_SetNamedVolumeScale((void*)g_soundManager, roundDol3DStrings, volumeSlope * normalizedSpeed + soundBase);
        SoundManager_SetNamedEffectPitch((void*)g_soundManager, roundDol3DStrings, soundBase * normalizedSpeed + basePitch);
    } else {
        SoundManager_SetNamedVolumeScale((void*)g_soundManager, roundDol3DStrings, kFloat_0_RoundScene);
    }
    // Advance the unwrapped angle and select speed level 1..7 with strict threshold tests.
    float oldAngle = AsCRoundDol3D(self)->rotationAngle;
    float angleResetThreshold = kFloat_neg1024_RoundDol3D;
    if (angleResetThreshold >= oldAngle) {
        AsCRoundDol3D(self)->rotationAngle = kFloat_0_RoundScene;
    }
    float rotationSpeed = AsCRoundDol3D(self)->rotationSpeed;
    float angleScale = kFloat_0_5_RoundSound;
    float newAngle = AsCRoundDol3D(self)->rotationAngle;
    float topSpeedThreshold = kRoundDolSpeedLevel7Threshold;
    newAngle = newAngle - rotationSpeed * angleScale;
    AsCRoundDol3D(self)->rotationAngle = newAngle;
    if (rotationSpeed > topSpeedThreshold) {
        AsCRoundDol3D(self)->speedLevel = 7;
    } else if (rotationSpeed > kRoundDolSpeedLevel6Threshold) {
        AsCRoundDol3D(self)->speedLevel = 6;
    } else if (rotationSpeed > kRoundDolSpeedLevel5Threshold) {
        AsCRoundDol3D(self)->speedLevel = 5;
    } else if (rotationSpeed > kRoundDolSpeedLevel4Threshold) {
        AsCRoundDol3D(self)->speedLevel = 4;
    } else if (rotationSpeed > kRoundDolSpeedLevel3Threshold) {
        AsCRoundDol3D(self)->speedLevel = 3;
    } else if (rotationSpeed > kRoundDolSpeedLevel2Threshold) {
        AsCRoundDol3D(self)->speedLevel = 2;
    } else {
        AsCRoundDol3D(self)->speedLevel = 1;
    }
    return 1;
}
// Select the dolphin or capsule animation range and dispatch its starting-frame events.
void CRoundDol3D_SetAnimationByIndex(RoundDolReceiver* self, unsigned int animationIndex) {
    AsCRoundDol3D(self)->animationIndex = animationIndex;
    AsCRoundDol3D(self)->state = kRoundAnimationPlaying;
    // Dolphin cases borrow a named range, store its frame endpoints, and dispatch first-frame events.
    switch (animationIndex) {
    case kAnimNoMotion: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->dolphinModel, (void*)(RoundDolNames->noMotion));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->dolphinModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimEntrance: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->dolphinModel, (void*)(RoundDolNames->entrance));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->dolphinModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimRotateLow: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->dolphinModel, (void*)(RoundDolNames->rotateLow));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->dolphinModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimRotateMiddle: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->dolphinModel, (void*)(RoundDolNames->rotateMiddle));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->dolphinModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimRotateHigh: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->dolphinModel, (void*)(RoundDolNames->rotateHigh));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->dolphinModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimRotateSuper: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->dolphinModel, (void*)(RoundDolNames->rotateSuper));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->dolphinModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimRotateOver: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->dolphinModel, (void*)(RoundDolNames->rotateOver));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->dolphinModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    // Capsule cases perform the same setup against the capsule model; playback advances elsewhere.
    case kAnimCapsuleLow: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->capsuleModel, (void*)(RoundDolNames->capsuleLow));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->capsuleModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimCapsuleMiddle: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->capsuleModel, (void*)(RoundDolNames->capsuleMiddle));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->capsuleModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimCapsuleHigh: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->capsuleModel, (void*)(RoundDolNames->capsuleHigh));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->capsuleModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    case kAnimCapsuleSuper: {
        void* range = WPuppet_FindAnimationRangeByName(AsCRoundDol3D(self)->capsuleModel, (void*)(RoundDolNames->capsuleSuper));
        AsCRoundDol3D(self)->animation = (unsigned int)range;
        void* model = AsCRoundDol3D(self)->capsuleModel;
        float firstFrame = ((RoundAnimationRangeView*)range)->firstFrame;
        AsCRoundDol3D(self)->animationTime = firstFrame;
        float lastFrame = ((RoundAnimationRangeView*)range)->lastFrame;
        AsCRoundDol3D(self)->animationEnd = lastFrame;
        (*(void**)&AsCRoundDol3D(self)->activeModel) = model;
        AnimationEventParser_ProcessFrameRange(self, model ? model : (void*)0, firstFrame, firstFrame, range, -1, 1);
        break;
    }
    default:
        break;
    }
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625C4", _ctors_u803156A4_0, RoundDol3D_InitTwoPiCache);
#pragma pop
