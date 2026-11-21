#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CCameraWork.h"
#define AsCCameraWork(p) ((CCameraWork*)(p))
#include "game/types.h"
#include "game/wvector.h"
// Translation unit 0x800D80CC-0x800D89AC (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

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
enum { kExcludedPlayerKind = 3, kExtendedCameraSelector = 3 };
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
struct GolfRosterEntry { char playerData[8]; int kind; };
struct InstanceCameraSelection { char state[0xA]; unsigned char selector; };
struct InstanceCameraRoot { void* vtable; InstanceCameraSelection* settings; };
struct AffineCameraFrame { float basis[9]; WVector translation; };
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
void CCameraWork_UninstallCamera(char* value) {
    (*(char*)&AsCCameraWork(value)->installed) = 0;
}

void CCameraWork_ConfigureMode(void*, int, int);
void __as__7WVectorFRC7WVector(void*, void*);

// Select the fixed-position mode, copy the position, and mark the camera installed.
void CCameraWork_SetInstalledCamera(void* self, void* position) {
    CCameraWork_ConfigureMode(self, kInstalledPositionMode, 0);
    (*(WVector*)&AsCCameraWork(self)->position[0]) = *(WVector*)position;
    (*(char*)&AsCCameraWork(self)->installed) = 1;
}

void* GolfDoc_GetCurrent();
unsigned char GolfDoc_GetPlayerCount();
void* GolfPlayer_GetCurrentEntryByIndex(void*);
void* CGolfDoc_GetItemSelection(void*, unsigned int);
int ResourceGroups_FindNamedValue(void*, void*, int);
void CameraResourceGroups_LoadPlayerResourceGroup(void*, void*);
int CameraWorkTable_FindAnimation(void*, void*, int);
void CameraResourceGroups_LoadPlayerAnimationGroup(void*, void*);
int ResourceGroups_FindNamedValueInThirdTable(void*, int, int);
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
void CCameraWork_PreloadPlayerAndIndexedGroups(void* self) {
    void* characterKey;
    for (unsigned char i = 0; i < GolfDoc_GetPlayerCount(); ++i) {
        void* entry = GolfPlayer_GetCurrentEntryByIndex((void*)(unsigned int)i);
        if (((GolfRosterEntry*)entry)->kind != kExcludedPlayerKind) {
            void* table = GolfDoc_GetCurrent();
            void* item = CGolfDoc_GetItemSelection(table, i);
            characterKey = *(void**)item;
            if (!ResourceGroups_FindNamedValue(self, characterKey, 0)) CameraResourceGroups_LoadPlayerResourceGroup(self, characterKey);
        }
    }
    for (unsigned char i = 0; i < GolfDoc_GetPlayerCount(); ++i) {
        void* entry = GolfPlayer_GetCurrentEntryByIndex((void*)(unsigned int)i);
        if (((GolfRosterEntry*)entry)->kind != kExcludedPlayerKind) {
            void* item = CGolfDoc_GetItemSelection(GolfDoc_GetCurrent(), i);
            characterKey = *(void**)item;
            if (!CameraWorkTable_FindAnimation(self, characterKey, 0)) CameraResourceGroups_LoadPlayerAnimationGroup(self, characterKey);
        }
    }
    int groupIterator;
    CameraGroupIterator_GetFirst(&groupIterator, (char*)&AsCCameraWork(self)->indexedCameraGroups[0]);
    // Original 0x800D80CC: compute the groupEnd iterator at every test.
    int previousIterator;
    int groupEnd;
    while ((CameraGroupIterator_GetEnd((void**)&groupEnd, (char*)&AsCCameraWork(self)->indexedCameraGroups[0]),
            CameraGroupIterator_NotEqual(&groupIterator, &groupEnd))) {
        int* node = CameraGroupIterator_GetKeyAddress((int**)&groupIterator);
        unsigned char value = *(unsigned char*)node;
        if (!ResourceGroups_FindNamedValueInThirdTable(self, value, 0)) CameraWorkTable_LoadAnimationSet(self, value);
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
    while ((cur = *link,
            parent = (void*)((unsigned int)((CameraGroupNode*)cur)->parentAndColor & ~1u),
            cur != ((CameraGroupNode*)parent)->left)) {
        *link = parent;
    }
    *link = parent;
    return;
}

void* InstanceTable_Get();
void Vec2_Zero(void*);
void CCameraWork_ConfigureMode(void*, int, int);
void CCameraWork_SelectShotCameraMode(void*, int);
void CCameraWork_BuildModeCurve(void*, void*);
extern int s_extendedCameraModeWeights[];
extern int s_cameraModeWeightsStorage[];
extern char s_cameraTransform[256];
extern const float kFloat_0_CameraWork;
extern const float kFloat_0_95993108;

void Matrix_CopyAffine(void*, void*);
void CCameraWork_SetupMode202Transition(void*, void*, void*);
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
void CCameraWork_SetupMode202Transition(void* self, void* initialTransform, void* targetPosition) {
    float initial = kFloat_0_95993108;
    AsCCameraWork(self)->resetFovPending = 0;
    AsCCameraWork(self)->fov = initial;
    (*(char*)&AsCCameraWork(self)->randomModeRequested) = 0;
    (*(char*)&AsCCameraWork(self)->groupAnimationActive) = 0;
    Matrix_CopyAffine((char*)&AsCCameraWork(self)->transform[0], initialTransform);
    CCameraWork_ConfigureMode(self, kMatrixTransitionMode, 1);
    Matrix_ExtractUniformScaleNormalizedRotation((char*)&AsCCameraWork(self)->transform[0], (char*)&AsCCameraWork(self)->rotation[0]);
    Vec3 cameraForward, ballToTarget, transformedForward, forwardAxis, ballToTargetDifference, raisedPosition, halfwayPosition, halfwayOffset, heightOffset;
    float zero = kFloat_0_CameraWork;
    forwardAxis.x = zero;
    forwardAxis.y = zero;
    forwardAxis.z = kFloat_1_CameraWork;
    Matrix_TransformPoint(&transformedForward, &forwardAxis, (char*)&AsCCameraWork(self)->rotation[0]);
    __as__7WVectorFRC7WVector(&cameraForward, &transformedForward);
    void* ballPosition = GolfBall__Fv();
    __mi__FRC7WVectorRC7WVector(&ballToTargetDifference, targetPosition, ballPosition);
    __as__7WVectorFRC7WVector(&ballToTarget, &ballToTargetDifference);
    float heading = Vector_SignedAngleXZ(&ballToTarget, &cameraForward);
    AsCCameraWork(self)->heading = heading;
    zero = kFloat_0_CameraWork;
    heightOffset.x = zero;
    heightOffset.y = kFloat_10_cameraWork;
    heightOffset.z = zero;
    __ml__FRC7WVectorf(&halfwayOffset, &ballToTarget, kFloat_0_5_CameraWork);
    ballPosition = GolfBall__Fv();
    __pl__FRC7WVectorRC7WVector(&halfwayPosition, ballPosition, &halfwayOffset);
    __pl__FRC7WVectorRC7WVector(&raisedPosition, &halfwayPosition, &heightOffset);
    __as__7WVectorFRC7WVector((char*)&AsCCameraWork(self)->position[0], &raisedPosition);
    __as__7WVectorFRC7WVector((char*)&AsCCameraWork(self)->startPosition[0], (char*)&AsCCameraWork(self)->position[0]);
    __as__7WVectorFRC7WVector((char*)&AsCCameraWork(self)->endPosition[0], &((AffineCameraFrame*)initialTransform)->translation);
    Matrix_CopyAffine((char*)&AsCCameraWork(self)->previousTransform[0], (char*)&AsCCameraWork(self)->transform[0]);
}
#pragma opt_common_subs on

// Select the next positive-weight camera mode and reset its input and curve state.
void CCameraWork_CycleMode(void* self, void* preserveHistory) {
    volatile void* vself = self;
    if (!preserveHistory) {
        int old = AsCCameraWork(vself)->actionCount;
        AsCCameraWork(self)->actionCount = old - AsCCameraWork(self)->actionCount;
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
    (*(char*)&AsCCameraWork(self)->inputMoved) = 0;
    AsCCameraWork(self)->inputFovOffset = kFloat_0_CameraWork;
    Vec2_Zero((char*)&AsCCameraWork(self)->inputRotationOffset[0]);
    AsCCameraWork(self)->mode = nextMode;
    AsCCameraWork(self)->pendingAction = -1;
    (*(char*)&AsCCameraWork(self)->unk_164) = 0; // Clear the per-shot camera latch; its trigger is still unresolved.
    (*(char*)&AsCCameraWork(self)->randomModeRequested) = 0;
    AsCCameraWork(self)->fov = kFloat_0_95993108;
    AsCCameraWork(self)->resetFovPending = 0;
    AsCCameraWork(self)->historyIndex = 0;
    CCameraWork_ConfigureMode(self, nextMode, 1);
    CCameraWork_SelectShotCameraMode(self, 1);
    CCameraWork_BuildModeCurve(self, (*(void**)&AsCCameraWork(self)->action));
}

unsigned int CGolfDoc_ResolveTurnAfterMatchHoleOut(void*);
void* GroupVector_GetCount(void*, void*);
void* GroupVector_GetValues(void*, void*, unsigned int);
void CVCGPlayer_StartGroupPlayback(void*, int, int);
void CCameraWork_ResetTrajectoryPlayback(void*);

#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma opt_lifetimes off
#pragma opt_propagation off
// Choose a player/rule animation group at a pivot and return its duration, or -1.
float CCameraWork_StartGroupAnimationAtPivot(void* camera, int groupKind, int variant, void* pivotPosition, void* unused) {
    int playerOrRuleIndex;
    void* self;
    int selectedVariant;
    int selectedGroupKind;
    float animationSegment[12];
    unsigned int segmentCount;
    playerOrRuleIndex = 0;
    self = camera;
    selectedVariant = variant;
    selectedGroupKind = groupKind;
    (*(char*)&AsCCameraWork(self)->randomModeRequested) = (char)playerOrRuleIndex;
    (*(char*)&AsCCameraWork(self)->groupAnimationActive) = 1;
    (*(WVector*)&AsCCameraWork(self)->pivotPosition[0]) = *(WVector*)pivotPosition;
    (*(void**)&AsCCameraWork(self)->pivot) = (char*)&AsCCameraWork(self)->pivotPosition[0];
    AsCCameraWork(self)->roll = kFloat_0_CameraWork;
    AsCCameraWork(self)->animationGroup = playerOrRuleIndex;
    if (selectedGroupKind == kPlayerResourceGroup) {
        playerOrRuleIndex = (unsigned char)CGolfDoc_ResolveTurnAfterMatchHoleOut(GolfDoc_GetCurrent());
        void* table = GolfDoc_GetCurrent();
        void* itemSelection = CGolfDoc_GetItemSelection(table, (unsigned char)playerOrRuleIndex);
        // The first selection word is a character ID carried in a pointer-typed ABI slot.
        (*(void**)&AsCCameraWork(self)->animationGroup) = (void*)ResourceGroups_FindNamedValue(self, *(void**)itemSelection, selectedVariant);
    } else if (selectedGroupKind == kPlayerAnimationGroup) {
        playerOrRuleIndex = (unsigned char)CGolfDoc_ResolveTurnAfterMatchHoleOut(GolfDoc_GetCurrent());
        void* table = GolfDoc_GetCurrent();
        void* itemSelection = CGolfDoc_GetItemSelection(table, (unsigned char)playerOrRuleIndex);
        (*(void**)&AsCCameraWork(self)->animationGroup) = (void*)CameraWorkTable_FindAnimation(self, *(void**)itemSelection, selectedVariant);
    } else if (selectedGroupKind == kRuleResourceGroup) {
        int t = SharedDoc_GetGolfRuleType();
        playerOrRuleIndex = (unsigned char)t;
        (*(void**)&AsCCameraWork(self)->animationGroup) = (void*)ResourceGroups_FindNamedValueInThirdTable(self, playerOrRuleIndex, selectedVariant);
    }
    CCameraWork_ResetTrajectoryPlayback(self);
    float durationSeconds = kFloat_0_CameraWork;
    if ((*(void**)&AsCCameraWork(self)->animationGroup)) {
        GroupVector_GetCount((*(void**)&AsCCameraWork(self)->animationGroup), &segmentCount);
        (*(unsigned int*)&AsCCameraWork(self)->groupValueCount) = segmentCount;
        for (playerOrRuleIndex = 0; playerOrRuleIndex < AsCCameraWork(self)->groupValueCount; ++playerOrRuleIndex) {
            GroupVector_GetValues((*(void**)&AsCCameraWork(self)->animationGroup), &animationSegment, (unsigned int)playerOrRuleIndex);
            durationSeconds += animationSegment[0];
        }
        AsCCameraWork(self)->animationSegmentIndex = 0;
        durationSeconds /= kFloat_30_0;
        CVCGPlayer_StartGroupPlayback((*(void**)&AsCCameraWork(self)->animationGroup), 0, 0);
        (*(void**)&AsCCameraWork(self)->playingAnimationGroup) = (*(void**)&AsCCameraWork(self)->animationGroup);
    } else {
        (*(void**)&AsCCameraWork(self)->playingAnimationGroup) = 0;
    }
    if ((*(void**)&AsCCameraWork(self)->playingAnimationGroup)) {
        AsCCameraWork(self)->duration = durationSeconds;
        AsCCameraWork(self)->animationTime = kFloat_0_CameraWork;
        return durationSeconds;
    } else {
        float f1 = kFloat_neg1_CameraWork;
        AsCCameraWork(self)->duration = f1;
        return f1;
    }
    (void)unused;
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs on

int rand();
// Reset camera playback; optionally choose a random mode using configured weights.
void CCameraWork_ResetMode(void* self, int mode) {
    AsCCameraWork(self)->inputFovOffset = kFloat_0_CameraWork;
    (*(char*)&AsCCameraWork(self)->inputMoved) = 0;
    Vec2_Zero((char*)&AsCCameraWork(self)->inputRotationOffset[0]);
    AsCCameraWork(self)->pendingAction = -1;
    (*(char*)&AsCCameraWork(self)->groupAnimationActive) = 0;
    Matrix_CopyAffine((char*)&AsCCameraWork(self)->transform[0], s_cameraTransform);
    Matrix_CopyAffine((char*)&AsCCameraWork(self)->previousTransform[0], s_cameraTransform);
    AsCCameraWork(self)->fov = kFloat_0_95993108;
    AsCCameraWork(self)->resetFovPending = 0;
    (*(char*)&AsCCameraWork(self)->randomModeRequested) = (mode == kRandomCameraMode);
    int randomValue = rand();
    volatile void* vself = self;
    int old = AsCCameraWork(vself)->actionCount;
    AsCCameraWork(self)->randomValue = randomValue;
    AsCCameraWork(self)->historyIndex = -1;
    AsCCameraWork(self)->actionCount = old - AsCCameraWork(self)->actionCount;
    (*(void**)&AsCCameraWork(self)->pivot) = GolfBall__Fv();
    AsCCameraWork(self)->playingAnimationGroup = 0;
    if (mode == kRandomCameraMode) {
        void* state = InstanceTable_Get();
        unsigned char selector = ((InstanceCameraRoot*)state)->settings->selector;
        if (selector == kExtendedCameraSelector) {
            int modeWeight;
            int remainingWeight = AsCCameraWork(self)->randomValue % AsCCameraWork(self)->extendedModeWeightTotal;
            int modeCount = AsCCameraWork(self)->extendedModeCount;
            for (int i = 0; i < modeCount; ++i) {
                modeWeight = s_extendedCameraModeWeights[i];
                modeWeight = modeWeight > 0 ? modeWeight : 0;
                remainingWeight = remainingWeight - modeWeight;
                if (remainingWeight <= 0) { mode = i + kFirstExtendedCameraMode; break; }
            }
        } else if (mode == kRandomCameraMode) {
            int modeWeight;
            int remainingWeight = AsCCameraWork(self)->randomValue % AsCCameraWork(self)->modeWeightTotal;
            int modeCount = AsCCameraWork(self)->modeCount;
            for (int i = 0; i < modeCount; ++i) {
                modeWeight = s_cameraModeWeightsStorage[i];

                modeWeight = (modeWeight > 0) ? modeWeight : 0;
                remainingWeight = remainingWeight - modeWeight;
                if (remainingWeight <= 0) { mode = i; break; }
            }
        }
    }
    AsCCameraWork(self)->mode = mode;
    CCameraWork_ConfigureMode(self, mode, 1);
    AsCCameraWork(self)->animationGroup = 0;
    if (mode < kFirstTransitionCameraMode) { CCameraWork_SelectShotCameraMode(self, 0); CCameraWork_BuildModeCurve(self, (*(void**)&AsCCameraWork(self)->action)); }
}

} // extern "C"
