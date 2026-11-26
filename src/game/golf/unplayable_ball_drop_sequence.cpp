// The unplayable-ball drop sequence: the controller's state machine that picks a
// drop point, shows the drop camera and then places the ball. The unit also holds
// the float tween and flash-debug helpers it shares, the shared model-preview
// render parameters, and the exhibition camera-preset getters.
#include "classes/GtlIGenericEnvironmentResource.h"
#include <decomp/static_init.h>
#include "classes/ExhibitionModelPositionRecord.h"
#include "classes/FloatTween.h"
#include "classes/CameraTransform800D.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "game/golfball.h"
#include "game/wvector.h"
#include "classes/CComUnplayable.h"
#include "classes/CExhibition.h"
#include "classes/CFlashMemoryDebug.h"
#include "classes/CQuadTree.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CSharedFilePtr.h"
#include "classes/CRival.h"

enum DropSequenceState {
    kChooseDropPosition = 0,
    kWaitForDropCamera = 1,
    kDropSequenceIdle = 2
};
enum TweenMode {
    kKeepTweenMode = -1,
    kTweenLinear = 0,
    kTweenSineOut = 1,
    kTweenCosineIn = 2
};
enum {
    kDropCameraHoldMs = 3000,
    kShowDropCameraRequest = 0x2E,
    kPlaceBallRequest = 0x11,
    kGenericLightRecordBytes = 0x88,
    kUnplayableTipIndex = 6,
    kModelRenderParameterBytes = 0x68,
    kDefaultModelRenderMode = 3
};
struct DropActorNames {
    char screen[7], golfRule[9];
};
extern "C" {

extern float kFloat_6_2831855;
extern float s_initializedTwoPi_8029B6B0;
extern float s_initializedTwoPi_8029B81C;
extern float s_initializedTwoPi_8029B8FC;
extern float s_initializedTwoPi_8029BBEC;
extern float s_initializedTwoPi_8029BBF8;
extern float s_initializedTwoPi_8029BDF4;
extern float kFloatTweenZero;
extern float kFloatTweenUnitBlend;
extern float kFloat_1_5707964_tweenDartsAudio;
extern float kFloat_1_5_UnplayableTip;
extern float kFloat_0_5_UnplayableTip;
extern float kFloat_0_0_Exhibition;
extern char __vt__14CComUnplayable[];
extern char __vt__17CFlashMemoryDebug[];
extern char s_characterEnvironmentFilename[];
extern char s_unplayableDropActorNames[];
extern char s_cameraTransform[];
extern void* g_taskManager;
extern CSharedDoc* g_sharedDoc;
extern char s_flashMemoryDebugInitialized;
extern char s_sharedModelRenderParamsInitialized;
extern char s_flashMemoryDebugDtorRegistration[];
extern CFlashMemoryDebug s_flashMemoryDebug;
extern char s_sharedModelRenderParamsDtorRegistration[];
extern char s_sharedModelRenderParams[];
extern void* Table_GetEntry(void*, int);
extern void* GolfBall__Fv(void);
extern void* __dl__FPv(void*);
extern void GolfTips_ShowIndexedMessage(int, int, int, float, float);
extern void Memory_Clear(void*, int);
extern void __register_global_object(void*, void*, void*);
extern void* ModelRenderParams_DeletingDtor(void*, int);
extern void* __dt__17CFlashMemoryDebugFv(void*, int);
extern float Math_SinApprox(float);
extern float Math_CosApprox(float);
extern void* SharedFile_Create(const char*, int);
extern void* CSharedFilePtr_GetLoadedBuffer(void*);
extern void* __nwa__FUl(int);
extern void gtl_IGenericEnvironmentResource_CopyLights(void*, void*, int);
extern void gtl_GenericLight_GetDescription(void*, void*);
extern void __dla__FPv(void*);
extern void ComUnplayable_FindDropCandidate(void*, void*);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void* CQuadTree_GetInstance(void);
extern void* Object_GetChild(void*);
extern void* TaskActorTable_FindByName(void*, void*);
extern unsigned int OS_GetTimeMilliseconds(void);

// Initialize this linked unit's cached full-turn angle.
void InitializeTwoPi_8029B6B0(void) {
    s_initializedTwoPi_8029B6B0 = kFloat_6_2831855;
}

// Initialize this linked unit's cached full-turn angle.
void InitializeTwoPi_8029B81C(void) {
    s_initializedTwoPi_8029B81C = kFloat_6_2831855;
}

// Initialize this linked unit's cached full-turn angle.
void InitializeTwoPi_8029B8FC(void) {
    s_initializedTwoPi_8029B8FC = kFloat_6_2831855;
}

// Mark the unplayable-ball drop controller idle.
void CComUnplayable::ResetToIdle() {
    state = kDropSequenceIdle;
}

// Initialize this linked unit's cached full-turn angle.
void InitializeTwoPi_8029BBEC(void) {
    s_initializedTwoPi_8029BBEC = kFloat_6_2831855;
}

// Initialize this linked unit's cached full-turn angle.
void InitializeTwoPi_8029BBF8(void) {
    s_initializedTwoPi_8029BBF8 = kFloat_6_2831855;
}

// Attach the rival and drop-position thinker; begin with no pending drop.
CComUnplayable::CComUnplayable(int rival, int dropThinker) {
    state = kDropSequenceIdle;
    *(void**)this = (void*)__vt__14CComUnplayable;
    shotPositionThinker = dropThinker;
    this->rival = rival;
}

// Report whether an unplayable-ball drop sequence is active.
int CComUnplayable::IsDropSequenceActive() {
    int* x = (int*)this;
    return ((CComUnplayable*)x)->state != kDropSequenceIdle;
}

// Initialize this linked unit's cached full-turn angle.
void InitializeTwoPi_8029BDF4(void) {
    s_initializedTwoPi_8029BDF4 = kFloat_6_2831855;
}

// Begin an idle drop sequence and display the unplayable-ball tip.
// Static member taking the controller as a restrict-qualified pointer: that is what
// lets the original move the constant loads past the state store.
int CComUnplayable::BeginDropSequence(CComUnplayable* __restrict self) {
    if (self->state != kDropSequenceIdle)
        return 0;
    self->state = kChooseDropPosition;
    GolfTips_ShowIndexedMessage(kUnplayableTipIndex, 0, 0, kFloat_1_5_UnplayableTip, kFloat_0_5_UnplayableTip);
    return 1;
}

// Lazily initialize and register shared model-preview render parameters.
void* ModelRenderParams_GetShared(void) {
    if (!*(signed char*)&s_sharedModelRenderParamsInitialized) {
        Memory_Clear(s_sharedModelRenderParams, kModelRenderParameterBytes);
        *(int*)s_sharedModelRenderParams = kDefaultModelRenderMode;
        __register_global_object(s_sharedModelRenderParams, (void*)ModelRenderParams_DeletingDtor,
                                 s_sharedModelRenderParamsDtorRegistration);
        s_sharedModelRenderParamsInitialized = 1;
    }
    return s_sharedModelRenderParams;
}

// Lazily initialize the flash-memory debug singleton and register its destructor.
CFlashMemoryDebug* CFlashMemoryDebug::GetInstance() {
    if (!*(signed char*)&s_flashMemoryDebugInitialized) {
        s_flashMemoryDebug.vtbl = __vt__17CFlashMemoryDebug;
        for (int i = 0; i < (int)sizeof(s_flashMemoryDebug.pad_004); ++i)
            s_flashMemoryDebug.pad_004[i] = 0;
        s_flashMemoryDebug.synchronous = 1;
        s_flashMemoryDebug.pad_00C = 0;
        s_flashMemoryDebug.pad_010 = 0;
        s_flashMemoryDebug.pad_014 = 0;
        __register_global_object(&s_flashMemoryDebug, (void*)__dt__17CFlashMemoryDebugFv,
                                 s_flashMemoryDebugDtorRegistration);
        s_flashMemoryDebugInitialized = 1;
    }
    return &s_flashMemoryDebug;
}

// Initialize an inactive tween at the supplied value.
void FloatTween_Init(void* tween, float initialValue) {
    float zero = kFloatTweenZero;
    ((FloatTween*)tween)->mode = 0;
    ((FloatTween*)tween)->value = initialValue;
    ((FloatTween*)tween)->initial = zero;
    ((FloatTween*)tween)->target = zero;
    ((FloatTween*)tween)->duration = zero;
    ((FloatTween*)tween)->elapsed = zero;
    ((FloatTween*)tween)->active = 0;
}

// Advance the selected interpolation curve and clamp to the target at the end.
// deltaTime, elapsed and duration use the SAME caller-selected time unit; this
// helper does not impose milliseconds or seconds. Return 0 if already inactive,
// 1 for any active update, including the update that finishes the tween.
int FloatTween_Update(FloatTween* tween, float deltaTime) {
    if (tween->active == 0)
        return 0;
    tween->elapsed += deltaTime;
    if (tween->elapsed >= tween->duration) {
        tween->active = 0;
        tween->value = tween->target;
        return 1;
    }
    float progress = tween->elapsed / tween->duration;
    float blend = kFloatTweenZero;
    switch (tween->mode) {
    case kTweenLinear:
        blend = progress;
        break;
    case kTweenSineOut:
        blend = Math_SinApprox(progress * kFloat_1_5707964_tweenDartsAudio);
        break;
    case kTweenCosineIn:
        blend = kFloatTweenUnitBlend - Math_CosApprox(progress * kFloat_1_5707964_tweenDartsAudio);
        break;
    }
    tween->value = blend * (tween->target - tween->initial) + tween->initial;
    return 1;
}

struct ModelRenderParameterWords {
    int words[kModelRenderParameterBytes / sizeof(int)];
};
// Copy the complete 0x68-byte model rendering parameter block.
void ModelRenderParams_Copy(void* dst, void* src) {
    *(ModelRenderParameterWords*)dst = *(ModelRenderParameterWords*)src;
}

struct SmallVec {
    int refs;
    void* vtable;
};
struct VTable {
    virtual void slot0(void);
    virtual void slot1(void);
    virtual void slot2(void);
    virtual void slot3(void);
    virtual void slot4(void);
    virtual void slot5(void);
    virtual void slot6(void);
    virtual void slot7(void);
    virtual void slot8(void);
    virtual void apply(void* req);
};
// Load the character environment, copy its first light description, and release temporary resources.
// lightDescription is caller-owned output. Return 0 only if SharedFile_Create
// fails, 1 after copying. A valid loaded environment with AT LEAST ONE light and
// successful intermediate allocations is required; none of those are checked.
// CopyLights materializes lightCount records at 0x88-byte stride, but the single
// GetDescription call consumes only record zero. No light array escapes this call.
int CharacterEnvironment_CopyFirstLightDescription(void* lightDescription) {
    // Acquire the borrowed file buffer, construct the environment, then drop the file handle.
    void* sharedFile = SharedFile_Create(s_characterEnvironmentFilename, 1);
    if (sharedFile == 0)
        return 0;
    void* fileBuffer = CSharedFilePtr_GetLoadedBuffer(sharedFile);
    void* environment = 0;
    void* fileSize = (void*)CSharedFilePtr::GetSize(sharedFile);
    gtl::IGenericEnvironmentResource::CreateFromMemory(fileBuffer, (int)fileSize, &environment);
    if (sharedFile)
        CSharedFileHandle::Destroy(sharedFile, 1);
    int lightCount;
    ((gtl::IGenericEnvironmentResource*)environment)->GetLightCount(&lightCount);
    // Materialize all records to satisfy CopyLights; publish only the first description.
    void* lights = __nwa__FUl(lightCount * kGenericLightRecordBytes);
    gtl_IGenericEnvironmentResource_CopyLights(environment, lights, lightCount);
    gtl_GenericLight_GetDescription(lightDescription, lights);
    if (lights)
        __dla__FPv(lights);
    // Release temporary records and the environment's final reference if exhausted.
    if (environment != 0) {
        int remainingReferences = *(int*)environment - 1;
        *(int*)environment = remainingReferences;
        if (remainingReferences == 0) {
            void** vtable = *(void***)((char*)environment + 4);
            ((void (*)(void*, int))vtable[2])(environment, 1);
        }
    }
    return 1;
}

// Synchronous actor-message payload (borrowed for apply): +0 kind is zero here;
// +4 value is the request opcode; +8 arg is null for camera display, or points
// to the controller-owned WVector drop position for ball placement. +C..+1C
// are zero in both requests.
struct Request {
    int kind;
    int value;
    void* arg;
    int fields[5];
};

#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
// Choose a legal drop point, show it for three seconds, then place the ball and resume rival thinking.
int CComUnplayable::UpdateDropSequence() {
    int* state = &this->state;
    switch (*state) {
    case kChooseDropPosition: {
        // Select a world-space candidate, adjust the camera position to ground clearance,
        // then tell the screen actor to display it. The position stays in dropPosition
        // for the placement request below.
        ComUnplayable_FindDropCandidate((*(void**)&shotPositionThinker), dropPosition);
        *(WVector*)(AsCameraTransform800D(s_cameraTransform)->position) = *(WVector*)(dropPosition);
        void* quadTreeOrTaskActors = CQuadTree_GetInstance();
        char adjustedCameraPosition[16];
        CQuadTree::AdjustPointToRegionAndGroundClearance(adjustedCameraPosition, quadTreeOrTaskActors,
                                                         AsCameraTransform800D(s_cameraTransform)->position);
        *(WVector*)(AsCameraTransform800D(s_cameraTransform)->position) = *(WVector*)adjustedCameraPosition;
        quadTreeOrTaskActors = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        Request request;
        request.kind = 0;
        request.value = kShowDropCameraRequest;
        request.arg = 0;
        request.fields[0] = 0;
        request.fields[1] = 0;
        request.fields[2] = 0;
        request.fields[3] = 0;
        request.fields[4] = 0;
        void* mgr = TaskActorTable_FindByName(quadTreeOrTaskActors, s_unplayableDropActorNames);
        if (mgr)
            ((VTable*)mgr)->apply(&request);
        startTimeMs = OS_GetTimeMilliseconds();
        *state = kWaitForDropCamera;
        break;
    }
    case kWaitForDropCamera: {
        unsigned int now = OS_GetTimeMilliseconds();
        if (now - startTimeMs >= kDropCameraHoldMs) {
            // Unsigned subtraction preserves the millisecond timer's wraparound policy.
            // Clear the document request before sending the position to the rule actor.
            CSharedDoc* sharedDocument = (CSharedDoc*)g_sharedDoc;
            ((CGolfDoc*)sharedDocument->golfDoc)->dropRequest = 0;
            void* actorTable = g_taskManager ? Object_GetChild(g_taskManager) : 0;
            Request request;
            request.kind = 0;
            request.value = kPlaceBallRequest;
            request.arg = dropPosition;
            request.fields[0] = 0;
            request.fields[1] = 0;
            request.fields[2] = 0;
            request.fields[3] = 0;
            request.fields[4] = 0;
            void* manager = TaskActorTable_FindByName(actorTable, s_unplayableDropActorNames +
                                                                      (int)&((DropActorNames*)0)->golfRule);
            if (manager)
                ((VTable*)manager)->apply(&request);
            ((CRival*)((*(void**)&rival)))->ResetShotThinkingState();
            *state = kDropSequenceIdle;
        }
        break;
    }
    case kDropSequenceIdle:
        return 0;
    }
    return 1;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset

// Model-position preset queries. Each looks up a record by index and returns one
// stored value, or zero when the record is absent. Limit1 and Limit2 are the two
// stored endpoints; they are not sorted or clamped here.
// Look up a model-position record and return its camera distance, or zero when absent.
float CExhibition::model_pos_GetCameraDistance(void* __restrict p, int index) {
    void* q = Table_GetEntry(*(void**)p, index);
    if (q != 0) {
        return ((ExhibitionModelPositionRecord*)q)->cameraDistance;
    }
    return kFloat_0_0_Exhibition;
}

// Return the model-position record's first camera distance limit, or zero when absent.
float CExhibition::model_pos_GetCameraDistanceLimit1(void* __restrict p, int index) {
    void* q = Table_GetEntry(*(void**)p, index);
    if (q != 0) {
        return ((ExhibitionModelPositionRecord*)q)->cameraDistanceLimit1;
    }
    return kFloat_0_0_Exhibition;
}

// Return the model-position record's second camera distance limit, or zero when absent.
float CExhibition::model_pos_GetCameraDistanceLimit2(void* __restrict p, int index) {
    void* q = Table_GetEntry(*(void**)p, index);
    if (q != 0) {
        return ((ExhibitionModelPositionRecord*)q)->cameraDistanceLimit2;
    }
    return kFloat_0_0_Exhibition;
}

// Return the model-position record's viewport center, or zero when absent.
float CExhibition::model_pos_GetViewportCenterX(void* __restrict p, int index) {
    void* value = *(void**)p;
    void* q = Table_GetEntry(value, index);
    if (q != 0) {
        return ((ExhibitionModelPositionRecord*)q)->viewportCenterX;
    }
    return kFloat_0_0_Exhibition;
}

// Return the model-position record's viewport bottom, or zero when absent.
float CExhibition::model_pos_GetViewportBottom(void* __restrict p, int index) {
    void* arg = *(void* volatile*)p;
    void* q = Table_GetEntry(arg, index);
    if (q != 0) {
        return ((ExhibitionModelPositionRecord*)q)->viewportBottom;
    }
    return kFloat_0_0_Exhibition;
}

// Return the first viewport-bottom limit, or zero when the record is absent.
float ExhibitionModelPosition_GetViewportBottomLimit1(void* __restrict p, int id) {
    void* arg = *(void* volatile*)p;
    void* q = Table_GetEntry(arg, id);
    if (q != 0) {
        return ((ExhibitionModelPositionRecord*)q)->viewportBottomLimit1;
    }
    return kFloat_0_0_Exhibition;
}

// Return the second viewport-bottom limit, or zero when the record is absent.
float ExhibitionModelPosition_GetViewportBottomLimit2(void* __restrict p, int id) {
    void* arg = *(void* volatile*)p;
    void* q = Table_GetEntry(arg, id);
    if (q != 0) {
        return ((ExhibitionModelPositionRecord*)q)->viewportBottomLimit2;
    }
    return kFloat_0_0_Exhibition;
}

// Return the model-position record's X rotation, or zero when absent.
float CExhibition::model_pos_GetRotationX(void* __restrict p, int index) {
    void* q = Table_GetEntry(*(void**)p, index);
    if (q != 0) {
        return ((ExhibitionModelPositionRecord*)q)->rotationX;
    }
    return kFloat_0_0_Exhibition;
}

// The registered deleting destructor frees the singleton's storage; the class
// itself owns nothing.
CFlashMemoryDebug::~CFlashMemoryDebug() {}

// The drop controller owns nothing; the deleting destructor only frees the object.
CComUnplayable::~CComUnplayable() {}

// Optionally free model rendering parameter storage.
void* ModelRenderParams_DeletingDtor(void* p, int deletingFlag) {
    if (p != 0 && deletingFlag > 0) {
        __dl__FPv(p);
    }
    return p;
}

// Test whether the ball's ground-type byte lies outside the three accepted values.
int GolfBall_IsGroundTypeOutsideZeroToTwo(void) {
    unsigned char x = ((CGolfBall*)GolfBall__Fv())->m_groundType;
    switch (x) {
    case 0:
    case 1:
    case 2:
        return 0;
    default:
        return 1;
    }
}

// Start interpolation from the current value, or snap immediately for a zero duration or unchanged target.
// mode -1 keeps the previous curve; other unsupported modes blend as zero until
// completion. Negative durations also snap. duration uses Update's deltaTime unit.
void FloatTween_Start(FloatTween* tween, int mode, float target, float duration) {
    if (mode != kKeepTweenMode)
        tween->mode = mode;
    float zero = kFloatTweenZero;
    float initial = tween->value;
    tween->target = target;
    tween->initial = initial;
    tween->duration = duration;
    tween->elapsed = zero;
    if (duration > zero && initial != tween->target) {
        tween->active = 1;
        return;
    }
    tween->value = target;
    tween->active = 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046246C", _ctors_u8029B6B0_0, InitializeTwoPi_8029B6B0);
STATIC_INIT_ENTRY(".CRT$XCU8046246C", _ctors_u8029B6B0_1, InitializeTwoPi_8029B81C);
STATIC_INIT_ENTRY(".CRT$XCU8046246C", _ctors_u8029B6B0_2, InitializeTwoPi_8029B8FC);
STATIC_INIT_ENTRY(".CRT$XCU8046246C", _ctors_u8029B6B0_3, InitializeTwoPi_8029BBEC);
STATIC_INIT_ENTRY(".CRT$XCU8046246C", _ctors_u8029B6B0_4, InitializeTwoPi_8029BBF8);
STATIC_INIT_ENTRY(".CRT$XCU8046246C", _ctors_u8029B6B0_5, InitializeTwoPi_8029BDF4);
#pragma pop
