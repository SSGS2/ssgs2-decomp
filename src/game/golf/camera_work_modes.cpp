// Camera mode handling for CCameraWork: installing the fixed camera, preloading
// the player, rule and indexed animation groups, starting a transition, and
// choosing the next or a random mode. One function picks and starts the group
// animation a shot camera plays at its pivot.
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CCameraWork.h"
#define AsCCameraWork(p) ((CCameraWork*)(p))
#include "game/types.h"
#include "game/wvector.h"
#include "classes/CVCGPlayer.h"

extern "C" {

extern CSharedDoc* g_sharedDoc;

enum CameraMode {
    kRandomCameraMode = -1,
    kInstalledPositionMode = 21,
    kFirstExtendedCameraMode = 100,
    kFirstTransitionCameraMode = 200,
    kMatrixTransitionMode = 202
};
enum CameraAnimationGroupKind {
    kPlayerResourceGroup = 0,
    kRuleResourceGroup = 1,
    kPlayerAnimationGroup = 2
};
enum {
    kExcludedPlayerKind = 3,
    kExtendedCameraSelector = 3
};
struct CameraGroupNode {
    CameraGroupNode* left;
    CameraGroupNode* right;
    CameraGroupNode* parentAndColor;
    unsigned int value;
};
struct CameraGroupTree {
    unsigned int count;
    CameraGroupNode* root;
    void* endStorage;
    CameraGroupNode* first;
};
struct GolfRosterEntry {
    char playerData[8];
    int kind;
};
struct InstanceCameraSelection {
    char state[0xA];
    unsigned char selector;
};
struct InstanceCameraRoot {
    void* vtable;
    InstanceCameraSelection* settings;
};
struct AffineCameraFrame {
    float basis[9];
    WVector translation;
};
typedef char CameraFrame_translation_offset[((int)&((AffineCameraFrame*)0)->translation == 0x24) ? 1 : -1];

void* GolfPlayer_GetEntryByIndex(void*, void*);

// Return the golf document currently installed in the shared document.
void* GolfDoc_GetCurrent() {
    return (void*)g_sharedDoc->golfDoc;
}

// Look up a roster entry in the current golf document.
void* GolfPlayer_GetCurrentEntryByIndex(void* value) {
    return GolfPlayer_GetEntryByIndex((void*)g_sharedDoc->golfDoc, value);
}

// Read the first iterator from a camera group tree.
void CameraGroupIterator_GetFirst(int* out, const char* src) {
    *out = (*(const int*)&((const CameraGroupTree*)src)->first);
}

// Return the unsigned group key stored after a tree node's three links.
int* CameraGroupIterator_GetKeyAddress(int** value) {
    return (int*)&((CameraGroupNode*)*value)->value;
}

// Return the current golf document's active roster size.
unsigned char GolfDoc_GetPlayerCount() {
    return AsCGolfDoc((void*)g_sharedDoc->golfDoc)->playerCount;
}

// Construct the tree end iterator from its embedded root-link address.
void CameraGroupIterator_GetEnd(void** out, char* value) {
    *out = &((CameraGroupTree*)value)->root;
}

// Compare camera group iterators for inequality.
int CameraGroupIterator_NotEqual(const int* a, const int* b) {
    return *a != *b;
}

// Return the current rule selector used for rule-specific camera resources.
unsigned char SharedDoc_GetGolfRuleType() {
    return AsCSharedDoc(g_sharedDoc)->golfRuleType;
}

// Mark this camera work object as no longer installed.
void CCameraWork::UninstallCamera() {
    char* value = (char*)this;
    (*(char*)&AsCCameraWork(value)->installed) = 0;
}

void __as__7WVectorFRC7WVector(void*, void*);

// Select the fixed-position mode, copy the position, and mark the camera installed.
void CCameraWork::SetInstalledCamera(void* position) {
    ConfigureMode(kInstalledPositionMode, 0);
    (*(WVector*)&this->position[0]) = *(WVector*)position;
    (*(char*)&installed) = 1;
}

void* GolfDoc_GetCurrent();
unsigned char GolfDoc_GetPlayerCount();
void* GolfPlayer_GetCurrentEntryByIndex(void*);
void* ResourceGroups_FindNamedValue(void*, void*, const char*);
void CameraResourceGroups_LoadPlayerResourceGroup(void*, void*);
void* CameraWorkTable_FindAnimation(void*, void*, const char*);
void CameraResourceGroups_LoadPlayerAnimationGroup(void*, void*);
void* ResourceGroups_FindNamedValueInThirdTable(void*, int, const char*);
void CameraWorkTable_LoadAnimationSet(void*, int);
void CameraGroupIterator_GetFirst(int*, const char*);
int* CameraGroupIterator_GetKeyAddress(int**);
void CameraGroupIterator_GetEnd(void**, char*);
int CameraGroupIterator_NotEqual(const int*, const int*);
void CameraGroupIterator_PostIncrement(void**, void**, int);

#pragma inline_depth(0)
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Preload player and indexed camera groups, excluding inactive roster entries.
void CCameraWork::PreloadPlayerAndIndexedGroups() {
    void* characterKey;
    for (unsigned char i = 0; i < GolfDoc_GetPlayerCount(); ++i) {
        void* entry = GolfPlayer_GetCurrentEntryByIndex((void*)(unsigned int)i);
        if (((GolfRosterEntry*)entry)->kind != kExcludedPlayerKind) {
            void* table = GolfDoc_GetCurrent();
            void* item = ((CGolfDoc*)table)->GetItemSelection(i);
            characterKey = *(void**)item;
            if (!ResourceGroups_FindNamedValue(this, characterKey, 0))
                CameraResourceGroups_LoadPlayerResourceGroup(this, characterKey);
        }
    }
    for (unsigned char i = 0; i < GolfDoc_GetPlayerCount(); ++i) {
        void* entry = GolfPlayer_GetCurrentEntryByIndex((void*)(unsigned int)i);
        if (((GolfRosterEntry*)entry)->kind != kExcludedPlayerKind) {
            void* item = ((CGolfDoc*)GolfDoc_GetCurrent())->GetItemSelection(i);
            characterKey = *(void**)item;
            if (!CameraWorkTable_FindAnimation(this, characterKey, 0))
                CameraResourceGroups_LoadPlayerAnimationGroup(this, characterKey);
        }
    }
    int groupIterator;
    CameraGroupIterator_GetFirst(&groupIterator, (char*)&indexedCameraGroups[0]);
    // Original 0x800D80CC: compute the groupEnd iterator at every test.
    int previousIterator;
    int groupEnd;
    while ((CameraGroupIterator_GetEnd((void**)&groupEnd, (char*)&indexedCameraGroups[0]),
            CameraGroupIterator_NotEqual(&groupIterator, &groupEnd))) {
        int* node = CameraGroupIterator_GetKeyAddress((int**)&groupIterator);
        unsigned char value = *(unsigned char*)node;
        if (!ResourceGroups_FindNamedValueInThirdTable(this, value, 0))
            CameraWorkTable_LoadAnimationSet(this, value);
        CameraGroupIterator_PostIncrement((void**)&previousIterator, (void**)&groupIterator, 0);
    }
    (void)characterKey;
}
#pragma opt_strength_reduction reset
#pragma opt_common_subs on
#pragma inline_depth()

// Post-increment a camera-group iterator through its tagged-parent red-black tree.
void CameraGroupIterator_PostIncrement(void** out, void** link, int unused) {
    *out = *link;
    void* node = ((CameraGroupNode*)*link)->right;
    if (node) {
        void* child;
        // Original 0x800D8288: descend to the leftmost child.
        while ((child = ((CameraGroupNode*)node)->left) != 0) {
            node = child;
        }
        *link = node;
        return;
    }
    void* cur = *link;
    void* parent;
    // Walk up until the current node is its parent's left child.
    while ((cur = *link, parent = (void*)((unsigned int)((CameraGroupNode*)cur)->parentAndColor & ~1u),
            cur != ((CameraGroupNode*)parent)->left)) {
        *link = parent;
    }
    *link = parent;
    return;
}

void* InstanceTable_Get();
void Vec2_Zero(void*);
extern int s_extendedCameraModeWeights[];
extern int s_cameraModeWeightsStorage[];
extern char s_cameraTransform[256];
extern const float kFloat_0_CameraWork;
extern const float kFloat_0_95993108;

void Matrix_CopyAffine(void*, void*);
void Matrix_ExtractUniformScaleNormalizedRotation(void*, void*);
void* Matrix_TransformPoint(void*, void*, void*);
void* GolfBall__Fv();
void* __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Vector_SignedAngleXZ(void*, void*);
void* __ml__FRC7WVectorf(void*, void*, float);
void* __pl__FRC7WVectorRC7WVector(void*, void*, void*);

typedef Vec3f Vec3;
extern const float kFloat_1_CameraWork;
extern const float kFloat_10_cameraWork;
extern const float kFloat_0_5_CameraWork;
extern const float kFloat_30_0;
extern const float kFloat_neg1_CameraWork;
#pragma opt_common_subs off
// Start a matrix transition from the raised ball/target midpoint toward the input frame.
void CCameraWork::SetupMode202Transition(void* initialTransform, void* targetPosition) {
    float initial = kFloat_0_95993108;
    resetFovPending = 0;
    fov = initial;
    (*(char*)&randomModeRequested) = 0;
    (*(char*)&groupAnimationActive) = 0;
    Matrix_CopyAffine((char*)&transform[0], initialTransform);
    ConfigureMode(kMatrixTransitionMode, 1);
    Matrix_ExtractUniformScaleNormalizedRotation((char*)&transform[0], (char*)&rotation[0]);
    Vec3 cameraForward, ballToTarget, transformedForward, forwardAxis, ballToTargetDifference, raisedPosition,
        halfwayPosition, halfwayOffset, heightOffset;
    float zero = kFloat_0_CameraWork;
    forwardAxis.x = zero;
    forwardAxis.y = zero;
    forwardAxis.z = kFloat_1_CameraWork;
    Matrix_TransformPoint(&transformedForward, &forwardAxis, (char*)&rotation[0]);
    __as__7WVectorFRC7WVector(&cameraForward, &transformedForward);
    void* ballPosition = GolfBall__Fv();
    __mi__FRC7WVectorRC7WVector(&ballToTargetDifference, targetPosition, ballPosition);
    __as__7WVectorFRC7WVector(&ballToTarget, &ballToTargetDifference);
    float heading = Vector_SignedAngleXZ(&ballToTarget, &cameraForward);
    this->heading = heading;
    zero = kFloat_0_CameraWork;
    heightOffset.x = zero;
    heightOffset.y = kFloat_10_cameraWork;
    heightOffset.z = zero;
    __ml__FRC7WVectorf(&halfwayOffset, &ballToTarget, kFloat_0_5_CameraWork);
    ballPosition = GolfBall__Fv();
    __pl__FRC7WVectorRC7WVector(&halfwayPosition, ballPosition, &halfwayOffset);
    __pl__FRC7WVectorRC7WVector(&raisedPosition, &halfwayPosition, &heightOffset);
    __as__7WVectorFRC7WVector((char*)&position[0], &raisedPosition);
    __as__7WVectorFRC7WVector((char*)&startPosition[0], (char*)&position[0]);
    __as__7WVectorFRC7WVector((char*)&endPosition[0], &((AffineCameraFrame*)initialTransform)->translation);
    Matrix_CopyAffine((char*)&previousTransform[0], (char*)&transform[0]);
}
#pragma opt_common_subs on

// Select the next positive-weight camera mode and reset its input and curve state.
void CCameraWork::CycleMode(void* preserveHistory) {
    volatile void* vself = this;
    if (!preserveHistory) {
        int old = AsCCameraWork(vself)->actionCount;
        actionCount = old - actionCount;
    }
    unsigned char selector = ((InstanceCameraRoot*)InstanceTable_Get())->settings->selector;
    int nextMode;
    if (selector == kExtendedCameraSelector) {
        nextMode = AsCCameraWork(vself)->mode - kFirstExtendedCameraMode;
        int availableModeCount = AsCCameraWork(vself)->extendedModeCount;
        do {
            nextMode = (nextMode + 1) % availableModeCount;
        } while (s_extendedCameraModeWeights[nextMode] <= 0);
        nextMode += kFirstExtendedCameraMode;
    } else {
        nextMode = AsCCameraWork(vself)->mode;
        int availableModeCount = AsCCameraWork(vself)->modeCount;
        do {
            nextMode = (nextMode + 1) % availableModeCount;
        } while (s_cameraModeWeightsStorage[nextMode] <= 0);
    }
    (*(char*)&inputMoved) = 0;
    inputFovOffset = kFloat_0_CameraWork;
    Vec2_Zero((char*)&inputRotationOffset[0]);
    mode = nextMode;
    pendingAction = -1;
    (*(char*)&unk_164) = 0; // Clear the per-shot camera latch; its trigger is still unresolved.
    (*(char*)&randomModeRequested) = 0;
    fov = kFloat_0_95993108;
    resetFovPending = 0;
    historyIndex = 0;
    ConfigureMode(nextMode, 1);
    ((CCameraWork*)this)->SelectShotCameraMode(1);
    ((CCameraWork*)this)->BuildModeCurve((*(unsigned int*)&action));
}

void* GroupVector_GetCount(void*, void*);
void* GroupVector_GetValues(void*, void*, unsigned int);

// The original compiles this function with these optimizations off.
#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma opt_lifetimes off
#pragma opt_propagation off
// Start the animation group that belongs to a player, a rule or a named group at
// a pivot, and return its length in seconds, or -1 when no group was found.
float CCameraWork::StartGroupAnimationAtPivot(int groupKind, const char* name, void* pivotPosition) {
    CCameraWork* self = this;
    int index = 0;
    float animationSegment[12];
    unsigned int segmentCount;
    self->randomModeRequested = (unsigned char)index;
    self->groupAnimationActive = 1;
    (*(WVector*)&self->pivotPosition[0]) = *(WVector*)pivotPosition;
    self->pivot = (int)&self->pivotPosition[0];
    self->roll = kFloat_0_CameraWork;
    self->animationGroup = (void*)index;
    if (groupKind == kPlayerResourceGroup) {
        index = (unsigned char)((CGolfDoc*)GolfDoc_GetCurrent())->ResolveTurnAfterMatchHoleOut();
        void* table = GolfDoc_GetCurrent();
        void* itemSelection = ((CGolfDoc*)table)->GetItemSelection((unsigned char)index);
        // The first selection word is a character ID carried in a pointer-typed ABI slot.
        self->animationGroup = ResourceGroups_FindNamedValue(self, *(void**)itemSelection, name);
    } else if (groupKind == kPlayerAnimationGroup) {
        index = (unsigned char)((CGolfDoc*)GolfDoc_GetCurrent())->ResolveTurnAfterMatchHoleOut();
        void* table = GolfDoc_GetCurrent();
        void* itemSelection = ((CGolfDoc*)table)->GetItemSelection((unsigned char)index);
        self->animationGroup = CameraWorkTable_FindAnimation(self, *(void**)itemSelection, name);
    } else if (groupKind == kRuleResourceGroup) {
        index = (unsigned char)SharedDoc_GetGolfRuleType();
        self->animationGroup = ResourceGroups_FindNamedValueInThirdTable(self, index, name);
    }
    ((CCameraWork*)self)->ResetTrajectoryPlayback();
    float durationSeconds = kFloat_0_CameraWork;
    if (self->animationGroup) {
        GroupVector_GetCount(self->animationGroup, &segmentCount);
        self->groupValueCount = (int)segmentCount;
        for (index = 0; index < self->groupValueCount; ++index) {
            GroupVector_GetValues(self->animationGroup, &animationSegment, (unsigned int)index);
            durationSeconds += animationSegment[0];
        }
        self->animationSegmentIndex = 0;
        durationSeconds /= kFloat_30_0;
        ((CVCGPlayer*)(self->animationGroup))->StartGroupPlayback(0, 0);
        self->playingAnimationGroup = self->animationGroup;
    } else {
        self->playingAnimationGroup = 0;
    }
    if (self->playingAnimationGroup) {
        self->duration = durationSeconds;
        self->animationTime = kFloat_0_CameraWork;
        return durationSeconds;
    } else {
        float noDuration = kFloat_neg1_CameraWork;
        self->duration = noDuration;
        return noDuration;
    }
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs on

int rand();
// Reset camera playback; optionally choose a random mode using configured weights.
void CCameraWork::ResetMode(int mode) {
    inputFovOffset = kFloat_0_CameraWork;
    (*(char*)&inputMoved) = 0;
    Vec2_Zero((char*)&inputRotationOffset[0]);
    pendingAction = -1;
    (*(char*)&groupAnimationActive) = 0;
    Matrix_CopyAffine((char*)&transform[0], s_cameraTransform);
    Matrix_CopyAffine((char*)&previousTransform[0], s_cameraTransform);
    fov = kFloat_0_95993108;
    resetFovPending = 0;
    (*(char*)&randomModeRequested) = (mode == kRandomCameraMode);
    int randomValue = rand();
    volatile void* vself = this;
    int old = AsCCameraWork(vself)->actionCount;
    this->randomValue = randomValue;
    historyIndex = -1;
    actionCount = old - actionCount;
    (*(void**)&pivot) = GolfBall__Fv();
    playingAnimationGroup = 0;
    if (mode == kRandomCameraMode) {
        void* state = InstanceTable_Get();
        unsigned char selector = ((InstanceCameraRoot*)state)->settings->selector;
        if (selector == kExtendedCameraSelector) {
            int modeWeight;
            int remainingWeight = this->randomValue % extendedModeWeightTotal;
            int modeCount = extendedModeCount;
            for (int i = 0; i < modeCount; ++i) {
                modeWeight = s_extendedCameraModeWeights[i];
                modeWeight = modeWeight > 0 ? modeWeight : 0;
                remainingWeight = remainingWeight - modeWeight;
                if (remainingWeight <= 0) {
                    mode = i + kFirstExtendedCameraMode;
                    break;
                }
            }
        } else if (mode == kRandomCameraMode) {
            int modeWeight;
            int remainingWeight = this->randomValue % modeWeightTotal;
            int modeCount = this->modeCount;
            for (int i = 0; i < modeCount; ++i) {
                modeWeight = s_cameraModeWeightsStorage[i];

                modeWeight = (modeWeight > 0) ? modeWeight : 0;
                remainingWeight = remainingWeight - modeWeight;
                if (remainingWeight <= 0) {
                    mode = i;
                    break;
                }
            }
        }
    }
    this->mode = mode;
    ConfigureMode(mode, 1);
    animationGroup = 0;
    if (mode < kFirstTransitionCameraMode) {
        ((CCameraWork*)this)->SelectShotCameraMode(0);
        ((CCameraWork*)this)->BuildModeCurve((*(unsigned int*)&action));
    }
}

} // extern "C"
