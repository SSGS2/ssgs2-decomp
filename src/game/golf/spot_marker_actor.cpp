// The shot marker actor: creation, loading its sprites and fonts, per-frame
// animation, and teardown. Drawing and messages are in
// spot_marker_draw_and_messages.cpp.
#include "classes/CClubRangeInfo.h"
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#include "classes/CSpot.h"
#include "game/wvector.h"
#include "classes/CQuadTree.h"

extern "C" {

void __ct__18NativeBase80201D18Fv(void*);
void __construct_array(void*, void (*)(void*), void (*)(void*), int, int);
void IActor_AddSkipFlags(void*, int);
void ResourcePreloadQueue_AddRequest(void*, int, int, float, void*, float, const char*, int, int, int);
void __dla__FPv(void*);
extern "C" void __dt__6CBpTexFv(void*);
extern "C" void __ct__6CBpTexFv(void*);
void Vec3_Zero(void*);
float Random_RangeFloat(int, float, float);
float Math_SinApprox(float);
float Math_CosApprox(float);
void* InstanceTable_Get(void);
float GolfParameter_ResolveFloat(void*, int);
void __ml__FfRC7WVector(float*, const float*, float);
void Matrix_SetAxisRotation(void*, float, int);
int Pointer_IsActionActive(void*, void*, int);
int Pointer_GetRisingActionBits(void*, void*, int);
void* CQuadTree_GetInstance(void);
void* GolfBall__Fv(void);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, void*, int);
void Object_DeleteVirtual(void*, void*);
void ResourceManager_ReleaseId(void*, void*);
void OverlayPlayer_Destroy(void*, int);
void ResourceHandle_Dtor(void*, int);
void DebugText_Init(void*);
void DebugText_LoadGlyphAtlas(void*, const char*, float, float, int, int, const char*);
void* WOverlay_Create(void*, const char*, int);
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern int g_resourceManager;
extern CSharedDoc* g_sharedDoc;
extern int g_inputPointer;
extern void* g_chatMsg;
// Float constants, still linked from the original data.
extern const float kFloat_0_spot, kFloat_1_spot, kSpotDistanceGlyphOffsetX, kSpotGlyphOffsetY, kSpotLabelOffset,
    kFloat_2_Marker, kSpotEndpointSize;
extern const float kFloat_NegPi_Spot, kFloat_Pi_Spot, kSpotWorldUnitsPerYard, kSpotRangeJitterFraction,
    kFloat_TwoPi_Spot, kFloat_0_5_Marker, kSpotComparisonEpsilon, kFloat_neg1_U800EFEAC;
// Reference values the blends are compared with: all three are 1, 1 and 0.
extern float g_spotPositionBlendReference, g_spotArrowPositionBlendReference, g_spotHeightReference;

// The strings the actor loads. The field sizes give the original byte offsets.
struct SpotIndicatorStrings {
    char taskName[9];
    char distanceFont[15];
    char distanceGlyphs[19];
    char playerNumberFont[13];
    char playerNumberGlyphs[7];
    char validTargetOverlay[21];
    char invalidTargetOverlay[21];
    char arrowOverlay[14];
    char playerMarkerOverlay[16];
    char indicatorBitmap[15];
    char fallingBitmap[13];
    char slopeBitmap[11];
    char targetHoleBitmap[22];
    char scoreTargetBitmap[18];
    char leftControlAction[9];  // "LCONTROL"
    char rightControlAction[9]; // "RCONTROL"
    char hideAction[5];         // "HIDE"
    char distanceFormat[6];     // "%.0fy"
    char padding[5];
};
typedef char SpotStringPoolSize[(sizeof(SpotIndicatorStrings) == 0xF8) ? 1 : -1];
typedef char SpotHideStringOffset[((int)&((SpotIndicatorStrings*)0)->hideAction == 0xE8) ? 1 : -1];
extern const SpotIndicatorStrings spotIndicatorStrings;
// The bitmap names of the nine marker textures.
struct TextureNameList {
    const char* names[CSpot::kTextureCount];
};
extern const TextureNameList s_spotPlayerMarkerBitmapNames;
extern void* g_sceneManager;
extern char __vt__5CSpot[];

} // extern "C"

#define kZero kFloat_0_spot
#define kOne kFloat_1_spot
#define kPi kFloat_Pi_Spot
#define kTwoPi kFloat_TwoPi_Spot

static const unsigned int kFloatSignBit = 0x80000000U;

enum {
    kPreloadResourceKind = 5,
    kActorSkipFlags = 0x1400,
    kTutorialScoreTargetStageLimit = 6,
    kRefreshTargetFlag = 1,
    kPuttingClubKind = 3,
    kTaskVtblOffset = 0x3C // offset of the second interface's vtable inside __vt__5CSpot
};

// Fields of other classes that this file reads. They belong in those classes'
// headers once those exist.
struct ChatMessageState {
    char pad_00[0x10];
    int messageValue; // 0x10
};
struct SelectedClubState {
    char pad_00[0xA];
    unsigned char kind; // 0x0A
};
struct ShotMarkerParameters {
    char pad_00[4];
    SelectedClubState* selectedClub; // 0x04
    char pad_08[0x44];
    unsigned int flags; // 0x4C
    char pad_50[0x20];
    float markerHeightOffset; // 0x70
};

// The nine marker textures are driven through their own vtables.
struct TextureBindingVtbl {
    virtual void v0();
    virtual void bindTexture(void*);
};
struct TextureBindingReadiness {
    virtual void nh0();
    virtual void nh1();
    virtual void nh2();
    virtual void nh3();
    virtual int isReady();
};
// A vector copied as three raw words, as the original does here.
struct VectorWords {
    unsigned int x, y, z;
};
struct OwnedSpotObject {
    virtual void destroy(int);
};

// Storage for objects the actor allocates, built by the original constructors.
struct DebugTextStorage {
    char storage[0x124];
    DebugTextStorage() {
        DebugText_Init(this);
    }
};

static inline CGolfDoc* GolfDoc() {
    return (CGolfDoc*)g_sharedDoc->golfDoc;
}

static inline WVector& AsVector(float* v) {
    return *(WVector*)v;
}

// Construct the actor with no resources loaded.
CSpot::CSpot() {
    __ct__18NativeBase80201D18Fv(this);
    vtbl = __vt__5CSpot;
    taskVtbl = __vt__5CSpot + kTaskVtblOffset;
    distanceText = 0;
    playerNumberText = 0;
    clubRangeInfo = 0;
    indicatorBitmap = 0;
    fallingBitmap = 0;
    slopeBitmap = 0;
    targetHolePositionBitmap = 0;
    arrowOverlay = 0;
    playerMarkerOverlay = 0;
    auxiliaryMarker = 0;
    __construct_array(textureBindings, __ct__6CBpTexFv, __dt__6CBpTexFv, sizeof(SpotTextureBinding), kTextureCount);
    scoreTargetBitmap = 0;
    markerVertices = 0;
    messageValueAtLoad = 0;
    IActor_AddSkipFlags(this, kActorSkipFlags);
    markerScreenLink = &markerScreen;
    unk_08 = 1;
    ballScreenLink = &ballScreen;
    fourthScreenLink = fourthScreenRecord;
    thirdScreenLink = thirdScreenRecord;
    screenRecordCount = 0;
    validTargetOverlay = 0;
    invalidTargetOverlay = 0;
}

// Show the other players' markers and queue the second interface for preloading.
void CSpot::OnPreLoadInit() {
    playerMarkersVisible = 1;
    void* taskInterface = this;
    if (this != 0) {
        taskInterface = &taskVtbl;
    }
    ResourcePreloadQueue_AddRequest(g_sceneManager, kPreloadResourceKind, 0, kOne, taskInterface, kZero,
                                    spotIndicatorStrings.taskName, 0, 0, 0);
}

// Delete an overlay through the resource manager and clear its slot.
// Each global and slot test stays separate: the release calls may change the manager.
inline void DeleteOverlay(void*& overlay) {
    if (g_resourceManager && overlay) {
        Object_DeleteVirtual((void*)g_resourceManager, overlay);
        overlay = 0;
    }
}
// Release a bitmap through the resource manager and clear its slot.
inline void ReleaseBitmap(void*& bitmap) {
    if (g_resourceManager && bitmap) {
        ResourceManager_ReleaseId((void*)g_resourceManager, bitmap);
        bitmap = 0;
    }
}

// Release the overlays, bitmaps, labels, club range info and vertex array.
void CSpot::OnDestroy() {
    DeleteOverlay(arrowOverlay);
    DeleteOverlay(validTargetOverlay);
    DeleteOverlay(invalidTargetOverlay);
    ReleaseBitmap(indicatorBitmap);
    ReleaseBitmap(fallingBitmap);
    ReleaseBitmap(slopeBitmap);
    DeleteOverlay(playerMarkerOverlay);
    ReleaseBitmap(scoreTargetBitmap);
    ReleaseBitmap(targetHolePositionBitmap);
    if (auxiliaryMarker) {
        OverlayPlayer_Destroy(auxiliaryMarker, 1);
        auxiliaryMarker = 0;
    }
    if (playerNumberText) {
        ResourceHandle_Dtor(playerNumberText, 1);
        playerNumberText = 0;
    }
    if (distanceText) {
        ResourceHandle_Dtor(distanceText, 1);
        distanceText = 0;
    }
    if (clubRangeInfo) {
        if (clubRangeInfo) {
            void* object = clubRangeInfo;
            ((OwnedSpotObject*)object)->destroy(1);
        }
        clubRangeInfo = 0;
    }
    if (markerVertices) {
        if (markerVertices) {
            __dla__FPv((char*)markerVertices - 0x10);
        }
        markerVertices = 0;
    }
}

// Destroy the club range info; the deleting flag frees the object.
CClubRangeInfo::~CClubRangeInfo() {}

// Create the glyph atlases, overlays, texture bindings and bitmaps.
void CSpot::OnLoad() {
    void* clubRange = new CClubRangeInfo();
    clubRangeInfo = clubRange;
    void* distanceLabel = new DebugTextStorage();
    distanceText = distanceLabel;
    DebugText_LoadGlyphAtlas(distanceLabel, (spotIndicatorStrings.distanceFont), kSpotDistanceGlyphOffsetX,
                             kSpotGlyphOffsetY, 0x10, 0x10, spotIndicatorStrings.distanceGlyphs);
    void* playerLabel = new DebugTextStorage();
    playerNumberText = playerLabel;
    DebugText_LoadGlyphAtlas(playerLabel, spotIndicatorStrings.playerNumberFont, kZero, kSpotGlyphOffsetY, 0x20, 0x10,
                             spotIndicatorStrings.playerNumberGlyphs);
    validTargetOverlay = WOverlay_Create((void*)g_resourceManager, spotIndicatorStrings.validTargetOverlay, 0);
    invalidTargetOverlay = WOverlay_Create((void*)g_resourceManager, spotIndicatorStrings.invalidTargetOverlay, 0);
    arrowOverlay = WOverlay_Create((void*)g_resourceManager, spotIndicatorStrings.arrowOverlay, 0);
    playerMarkerOverlay = WOverlay_Create((void*)g_resourceManager, spotIndicatorStrings.playerMarkerOverlay, 0);
    TextureNameList textureNames = s_spotPlayerMarkerBitmapNames;
    SpotTextureBinding* binding = textureBindings;
    for (int i = 0; i < kTextureCount; ++i, ++binding) {
        ((TextureBindingVtbl*)binding)->bindTexture((void*)textureNames.names[i]);
    }
    indicatorBitmap =
        ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.indicatorBitmap, 0, 0, 0);
    fallingBitmap = ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.fallingBitmap, 0, 0, 0);
    slopeBitmap = ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.slopeBitmap, 0, 0, 0);
    targetHolePositionBitmap =
        ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.targetHoleBitmap, 0, 0, 0);
    if (GolfDoc()->tutorialStage < kTutorialScoreTargetStageLimit) {
        scoreTargetBitmap =
            ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.scoreTargetBitmap, 0, 0, 0);
    }
    messageValueAtLoad = ((ChatMessageState*)g_chatMsg)->messageValue;
}

// Reset the marker's animation state and pick a random horizontal offset,
// scaled by a fraction of the shot range.
void CSpot::OnInit() {
    groundHitIndex = 0;
    heading = kZero;
    rotationPhase = kZero;
    Vec3_Zero(position);
    Vec3_Zero(targetPosition);
    Vec3_Zero(previousPosition);
    positionBlend = kOne;
    enabled = 1;
    arrowWidth = kFloat_2_Marker;
    endpointSize = kSpotEndpointSize;
    labelWidthOffset = kSpotLabelOffset;
    labelHeightOffset = kSpotLabelOffset;
    targetHeightOffset = kZero;
    targetDirty = 1;
    pulsePhase = kZero;
    markerHeading = kZero;
    arrowBlend = kZero;
    arrowBlendDirection = kOne;
    Vec3_Zero(&unk_50);
    // The product is built in a named local, the way the original does; the
    // operator form puts its temporary in a different stack slot.
    float scaled[3];
    float angle = Random_RangeFloat(-1, kFloat_NegPi_Spot, kPi);
    float sine = Math_SinApprox(angle);
    float direction[3];
    direction[0] = Math_CosApprox(angle);
    direction[1] = kZero;
    direction[2] = sine;
    float shotRange = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
    float jitterRange = kSpotWorldUnitsPerYard * shotRange;
    jitterRange = kSpotRangeJitterFraction * jitterRange;
    float scale = Random_RangeFloat(-1, kZero, jitterRange);
    __ml__FfRC7WVector(scaled, direction, scale);
    initialRandomOffset = AsVector(scaled);
}

// Whether two values are within the comparison tolerance of each other. The
// absolute difference is taken by testing the sign bit.
static inline bool IsNear(float reference, float value) {
    float difference = value - reference;
    float magnitude = (*(unsigned int*)&difference & kFloatSignBit) ? -difference : difference;
    return magnitude < kSpotComparisonEpsilon;
}

// Wait for the textures, handle the HIDE action, and animate the marker: advance
// its phases, find the ground point under the aim, and sweep the arrow.
void CSpot::OnProcess(float deltaSeconds) {
    float animationStep = deltaSeconds;
    SpotTextureBinding* binding = textureBindings;
    int allTexturesReady = 1;
    for (int i = 0; i < kTextureCount; ++i) {
        int bindingReady = ((TextureBindingReadiness*)binding)->isReady();
        allTexturesReady = allTexturesReady & bindingReady;
        ++binding;
    }
    // Nothing happens until every texture is ready.
    if (allTexturesReady) {
        // Reset the placement, turn the tutorial marker, and check the input.
        targetHeightOffset = kZero;
        AsVector(position) = AsVector(targetPosition);
        {
            void* parameters = InstanceTable_Get();
            ((ShotMarkerParameters*)parameters)->markerHeightOffset = kZero;
        }
        {
            unsigned char tutorialMarkerVisible = GolfDoc()->tutorialMarkerVisible;
            if (tutorialMarkerVisible) {
                markerHeading = kPi * animationStep + markerHeading;
                if (markerHeading > kTwoPi) {
                    markerHeading = markerHeading - kTwoPi;
                }
            }
        }
        {
            void* parameters = InstanceTable_Get();
            if (((ShotMarkerParameters*)parameters)->flags & kRefreshTargetFlag) {
                targetDirty = 1;
            }
        }
        {
            // HIDE toggles the other players' markers only when neither control action is held.
            int held = Pointer_IsActionActive((void*)g_inputPointer, (void*)spotIndicatorStrings.leftControlAction, 1);
            if (!held) {
                held = Pointer_IsActionActive((void*)g_inputPointer, (void*)spotIndicatorStrings.rightControlAction, 1);
                if (!held) {
                    held =
                        Pointer_GetRisingActionBits((void*)g_inputPointer, (void*)spotIndicatorStrings.hideAction, 1);
                    if (held) {
                        playerMarkersVisible = !playerMarkersVisible;
                    }
                }
            }
        }
        // Advance the phases, then refresh the ground point and animate the arrow.
        if (enabled) {
            {
                rotationPhase = kPi * animationStep + rotationPhase;
                if (rotationPhase > kTwoPi) {
                    rotationPhase = rotationPhase - kTwoPi;
                }
            }
            {
                pulsePhase = kFloat_0_5_Marker * animationStep + pulsePhase;
                if (pulsePhase > kOne) {
                    pulsePhase = kZero;
                }
            }
            // Place the target at the ball plus the aim range turned by the heading, on the ground.
            if (targetDirty) {
                bool nearTarget = IsNear(g_spotPositionBlendReference, positionBlend);
                if (nearTarget) {
                    void* parameters = InstanceTable_Get();
                    if (((ShotMarkerParameters*)parameters)->flags & kRefreshTargetFlag) {
                        pulsePhase = kZero;
                    }
                    targetDirty = 0;
                    groundHitIndex = 0;
                    float headingMatrix[12];
                    VectorWords groundQueryPosition;
                    WVector groundPosition;
                    Matrix_SetAxisRotation(headingMatrix, heading, 1);
                    void* rangeParameters = InstanceTable_Get();
                    float rangeYards = GolfParameter_ResolveFloat(rangeParameters, 0);
                    float rangeWorldUnits = kSpotWorldUnitsPerYard * rangeYards;
                    WVector rangeOffset = rangeWorldUnits * *(WVector*)(headingMatrix + 6);
                    WVector* ball = (WVector*)GolfBall__Fv();
                    WVector ballPlusRange = *ball + rangeOffset;
                    WVector targetCopy;
                    targetCopy = ballPlusRange;
                    // The ground query takes the vector as three raw words.
                    groundQueryPosition = *(VectorWords*)&targetCopy;
                    void* quadTree = CQuadTree_GetInstance();
                    GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPosition, quadTree, &groundQueryPosition, 1,
                                                            &groundHitIndex, 1);
                    AsVector(targetPosition) = groundPosition;
                    {
                        bool nearGround = IsNear(g_spotHeightReference, targetHeightOffset);
                        if (nearGround) {
                            // Snap the marker to the target when the height offset is within tolerance.
                            AsVector(position) = AsVector(targetPosition);
                        }
                    }
                }
            }

            {
                void* parameters = InstanceTable_Get();
                // Sweep the arrow, unless the club is a putter or the marker is still blending.
                SelectedClubState* club = ((ShotMarkerParameters*)parameters)->selectedClub;
                unsigned char clubKind = club->kind;
                if (clubKind != kPuttingClubKind && IsNear(g_spotArrowPositionBlendReference, positionBlend)) {
                    // The blend moves back and forth between 0 and 1 at kFloat_2_Marker per second.
                    float rate = kFloat_2_Marker * arrowBlendDirection;
                    float blend = rate * animationStep + arrowBlend;
                    arrowBlend = blend;
                    if (blend > kOne) {
                        arrowBlend = kOne;
                        arrowBlendDirection = kFloat_neg1_U800EFEAC;
                    } else if (blend < kZero) {
                        arrowBlend = kZero;
                        arrowBlendDirection = kOne;
                    }
                    // The arrow's position sweeps between the lower and upper lie scales with weight
                    // blend squared. It only places the arrow; the ball flight is not affected.
                    float lowerLieScale = ((CQuadTree*)CQuadTree_GetInstance())->GetMinPowerOnGround();
                    float lowerWeight = kOne - arrowBlend * arrowBlend;
                    float weightedLowerGround = lowerWeight * lowerLieScale;
                    float upperLieScale = ((CQuadTree*)CQuadTree_GetInstance())->GetMaxPowerOnGround();
                    float blendSquared = arrowBlend * arrowBlend;
                    arrowProgress = blendSquared * upperLieScale + weightedLowerGround;
                } else {
                    arrowProgress = kOne;
                }
            }
        }
    }
}
