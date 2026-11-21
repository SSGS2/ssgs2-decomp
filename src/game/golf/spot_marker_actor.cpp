#include "classes/CGolfDoc.h" // Layout offsets checked in the header.
#include "classes/CSharedDoc.h"
#include "classes/CSpot.h"

// Borrow the checked marker actor layout without constructing or owning storage.
inline CSpot* AsCSpot(void* object) { return (CSpot*)object; }
// Translation unit 0x800EFEAC-0x800F0998 (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void __ct__18NativeBase80201D18Fv(void*);
void __construct_array(void*, void (*)(void*), void (*)(void*), int, int);
void IActor_AddSkipFlags(void*, int);
void ResourcePreloadQueue_AddRequest(void*, int, int, float, void*, float, const char*, int, int, int);
void __dt__9CInfoBaseFv(void*, int);
void __dl__FPv(void*);
void __dla__FPv(void*);
void CBpTex_dtor(void*);
void CBpTex_Init(void*);
void Vec3_Zero(void*);
float Random_RangeFloat(int, float, float);
float Math_SinApprox(float);
float Math_CosApprox(float);
void* InstanceTable_Get(void);
float GolfParameter_ResolveFloat(void*, int);
void __ml__FfRC7WVector(float*, const float*, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __as__7WVectorFRC7WVector(void*, float*);
void Matrix_SetAxisRotation(void*, float, int);
int Pointer_IsActionActive(void*, void*, int);
int Pointer_GetRisingActionBits(void*, void*, int);
void* CQuadTree_GetInstance(void);
float CQuadTree_GetMinPowerOnGround(void*);
void* GolfBall__Fv(void);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, void*, int);
float CQuadTree_GetMaxPowerOnGround(void*);
void Object_DeleteVirtual(void*, void*);
void ResourceManager_ReleaseId(void*, void*);
void OverlayPlayer_Destroy(void*, int);
void ResourceHandle_Dtor(void*, int);
void* __nw__FUl(unsigned int);
void __ct__14CClubRangeInfoFv(void*);
void DebugText_Init(void*);
void DebugText_LoadGlyphAtlas(void*, const char*, float, float, int, int, const char*);
void* WOverlay_Create(void*, const char*, int);
void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern int g_resourceManager;
extern CSharedDoc* g_sharedDoc;
extern int g_inputPointer;
extern void* g_chatMsg;
// Linked marker constants: glyph offsets 3/-30, endpoint size 12, label offsets 20;
// angles -pi/pi/2*pi, range jitter scale 0.02, and equality tolerance 0.00001.
extern const float kFloat_0_spot, kFloat_1_spot, kSpotDistanceGlyphOffsetX, kSpotGlyphOffsetY, kSpotLabelOffset, kFloat_2_Marker, kSpotEndpointSize;
extern const float kFloat_NegPi_Spot, kFloat_Pi_Spot, kSpotWorldUnitsPerYard, kSpotRangeJitterFraction, kFloat_TwoPi_Spot, kFloat_0_5_Marker, kSpotComparisonEpsilon, kFloat_neg1_U800EFEAC;
// Linked comparison targets: one, one, and zero, respectively.
extern float g_spotPositionBlendReference, g_spotArrowPositionBlendReference, g_spotHeightReference;
// Packed CP932/ASCII pool; field sizes preserve the original byte offsets.
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
    char leftControlAction[9];
    char rightControlAction[9];
    char hideAction[5];
    char distanceFormat[6];
    char padding[5];
};
typedef char SpotStringPoolSize[(sizeof(SpotIndicatorStrings) == 0xF8) ? 1 : -1];
typedef char SpotHideStringOffset[((int)&((SpotIndicatorStrings*)0)->hideAction == 0xE8) ? 1 : -1];
extern const SpotIndicatorStrings spotIndicatorStrings;
extern const char* const s_spotPlayerMarkerBitmapNames[9]; // Player/COM marker bitmap names.
extern void* g_sceneManager;
extern char __vt__5CSpot[];

static const unsigned int kFloatSignBit = 0x80000000U;

enum { kSpotTextureCount = 9, kSpotTextureBindingBytes = 0x28,
       kSpotPreloadResourceKind = 5, kSpotActorSkipFlags = 0x1400,
       kTutorialScoreTargetStageLimit = 6, kRefreshTargetFlag = 1,
       kPuttingClubKind = 3 };
struct SpotTextureBinding { char storage[kSpotTextureBindingBytes]; };
struct SpotScreenRecord { char storage[0x38]; };

// Complete local construction view. Shared named fields also use CSpot's checked layout.
struct SpotState {
    void* vtable;
    int reserved04;
    int displayPriority;
    char reserved0C[0x0C];
    void* baseVtable;
    void* distanceText;
    void* playerNumberText;
    char position[0x0C];
    float previousPositionX;
    char reserved34[0x14];
    float heading;
    float positionBlend;
    char motionOffset[0x0C];
    char randomOffset[0x0C];
    char targetPosition[0x0C];
    float targetHeightOffset;
    int groundHitIndex;
    float rotationPhase;
    unsigned char enabled;
    char reserved81[3];
    float arrowWidth;
    float endpointSize;
    void* clubRangeInfo;
    char markerColor[4];
    float labelWidthOffset;
    float labelHeightOffset;
    void* indicatorBitmap;
    void* fallingBitmap;
    void* slopeBitmap;
    void* targetHoleBitmap;
    float pulsePhase;
    char arrowProgress[4];
    float arrowBlend;
    float arrowBlendDirection;
    void* validTargetOverlay;
    void* invalidTargetOverlay;
    void* arrowOverlay;
    void* playerMarkerOverlay;
    void* auxiliaryMarker;
    SpotTextureBinding textureBindings[kSpotTextureCount];
    float markerHeading;
    void* scoreTargetBitmap;
    void* markerScreenLink;
    void* ballScreenLink;
    void* fourthScreenLink;
    void* thirdScreenLink;
    int screenRecordCount;
    SpotScreenRecord screenRecords[4];
    unsigned char targetDirty;
    char reserved335[7];
    void* markerVertices;
    unsigned char playerMarkersVisible;
    char reserved341[3];
    int messageValueAtLoad;
    char reserved348[0x14];
    unsigned char reserved35C;
    char pad35D[3];
    float reserved360;
    float reserved364;
    float reserved368;
};

struct SpotMessageState { char precedingFields[0x10]; int messageValue; };
struct SelectedClubState { char precedingFields[0xA]; unsigned char kind; };
struct ShotMarkerParameters {
    char precedingFields[4];
    SelectedClubState* selectedClub;
    char precedingFlags[0x44];
    unsigned int flags;
    char precedingHeight[0x20];
    float markerHeightOffset;
};
typedef char SpotStateTextureOffset[((int)&((SpotState*)0)->textureBindings == 0xD0) ? 1 : -1];
typedef char SpotStateScreenOffset[((int)&((SpotState*)0)->screenRecords == 0x254) ? 1 : -1];
typedef char SpotStateDirtyOffset[((int)&((SpotState*)0)->targetDirty == 0x334) ? 1 : -1];

struct ClubRangeInfoStorage {
    char storage[0x24];
    // Construct the marker's owned club-range info object in its original storage.
    ClubRangeInfoStorage() { __ct__14CClubRangeInfoFv(this); }
};

struct DebugTextStorage {
    char storage[0x124];
    // Construct a marker debug label in its original storage.
    DebugTextStorage() { DebugText_Init(this); }
};

struct TextureBindingLoader {
    virtual void v0();
    virtual void bindTexture(void*);
};

// Create glyph atlases, marker overlays, nine texture bindings, and target bitmaps.
void CSpot_OnLoad(SpotState* self) {
    void* clubRange = new ClubRangeInfoStorage();
    AsCSpot(self)->clubRangeInfo = clubRange;
    void* distanceLabel = new DebugTextStorage();
    AsCSpot(self)->distanceText = distanceLabel;
    DebugText_LoadGlyphAtlas(distanceLabel, (spotIndicatorStrings.distanceFont), kSpotDistanceGlyphOffsetX, kSpotGlyphOffsetY, 0x10, 0x10, spotIndicatorStrings.distanceGlyphs);
    void* playerLabel = new DebugTextStorage();
    AsCSpot(self)->playerNumberText = playerLabel;
    DebugText_LoadGlyphAtlas(playerLabel, spotIndicatorStrings.playerNumberFont, kFloat_0_spot, kSpotGlyphOffsetY, 0x20, 0x10, spotIndicatorStrings.playerNumberGlyphs);
    AsCSpot(self)->validTargetOverlay = WOverlay_Create((void*)g_resourceManager, spotIndicatorStrings.validTargetOverlay, 0);
    AsCSpot(self)->invalidTargetOverlay = WOverlay_Create((void*)g_resourceManager, spotIndicatorStrings.invalidTargetOverlay, 0);
    AsCSpot(self)->arrowOverlay = WOverlay_Create((void*)g_resourceManager, spotIndicatorStrings.arrowOverlay, 0);
    AsCSpot(self)->playerMarkerOverlay = WOverlay_Create((void*)g_resourceManager, spotIndicatorStrings.playerMarkerOverlay, 0);
    void* textureNames[kSpotTextureCount];
    { const char* const* source = s_spotPlayerMarkerBitmapNames; for (int k = 0; k < kSpotTextureCount; ++k) textureNames[k] = (void*)source[k]; }
    SpotTextureBinding* binding = self->textureBindings;
    for (int i = 0; i < kSpotTextureCount; ++i, ++binding) {
        ((TextureBindingLoader*)binding)->bindTexture(textureNames[i]);
    }
    AsCSpot(self)->indicatorBitmap = ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.indicatorBitmap, 0, 0, 0);
    AsCSpot(self)->fallingBitmap = ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.fallingBitmap, 0, 0, 0);
    AsCSpot(self)->slopeBitmap = ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.slopeBitmap, 0, 0, 0);
    AsCSpot(self)->targetHolePositionBitmap = ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.targetHoleBitmap, 0, 0, 0);
    if (((CGolfDoc*)g_sharedDoc->golfDoc)->tutorialStage /* 0x98 */ < kTutorialScoreTargetStageLimit)
        AsCSpot(self)->scoreTargetBitmap = ResourceManager_LoadBitmap((void*)g_resourceManager, spotIndicatorStrings.scoreTargetBitmap, 0, 0, 0);
    self->messageValueAtLoad = ((SpotMessageState*)g_chatMsg)->messageValue;
}

// Reset marker animation and seed a random horizontal offset scaled by shot range.
void CSpot_OnInit(SpotState* self) {
    float tmp[3];
    float pos[3];
    self->groundHitIndex = 0;
    self->heading = kFloat_0_spot;
    self->rotationPhase = kFloat_0_spot;
    Vec3_Zero(AsCSpot(self)->position /* 0x24 */);
    Vec3_Zero(AsCSpot(self)->targetPosition /* 0x68 */);
    Vec3_Zero(AsCSpot(self)->previousPosition /* 0x30 */);
    self->positionBlend = kFloat_1_spot;
    self->enabled = 1;
    self->arrowWidth = kFloat_2_Marker;
    self->endpointSize = kSpotEndpointSize;
    self->labelWidthOffset = kSpotLabelOffset;
    self->labelHeightOffset = kSpotLabelOffset;
    self->targetHeightOffset = kFloat_0_spot;
    self->targetDirty = 1;
    self->pulsePhase = kFloat_0_spot;
    AsCSpot(self)->markerHeading = kFloat_0_spot;
    self->arrowBlend = kFloat_0_spot;
    self->arrowBlendDirection = kFloat_1_spot;
    Vec3_Zero(self->motionOffset);
    float angle = Random_RangeFloat(-1, kFloat_NegPi_Spot, kFloat_Pi_Spot);
    float sine = Math_SinApprox(angle);
    pos[0] = Math_CosApprox(angle);
    pos[1] = kFloat_0_spot;
    pos[2] = sine;
    float shotRange = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
    float jitterRange = kSpotWorldUnitsPerYard * shotRange;
    jitterRange = kSpotRangeJitterFraction * jitterRange;
    float scale = Random_RangeFloat(-1, kFloat_0_spot, jitterRange);
    __ml__FfRC7WVector(tmp, pos, scale);
    __as__7WVectorFRC7WVector(self->randomOffset, tmp);
}

struct OwnedSpotObject {
    virtual void destroy(int);
};

struct TextureBindingReadiness {
    virtual void nh0();
    virtual void nh1();
    virtual void nh2();
    virtual void nh3();
    virtual int isReady();
};

// Keep each global/slot test separate: release calls may change manager state.
// Delete an overlay through the resource manager and clear its owning actor slot.
inline void DeleteSpotOverlay(void*& overlay) {
    if (g_resourceManager && overlay) {
        Object_DeleteVirtual((void*)g_resourceManager, overlay);
        overlay = 0;
    }
}
// Release a bitmap resource handle and clear the actor slot when the manager is live.
inline void ReleaseSpotBitmap(void*& bitmap) {
    if (g_resourceManager && bitmap) {
        ResourceManager_ReleaseId((void*)g_resourceManager, bitmap);
        bitmap = 0;
    }
}

// Release marker overlays, bitmap handles, labels, and the optional vertex array.
void CSpot_OnDestroy(SpotState* self) {
    DeleteSpotOverlay(self->arrowOverlay);
    DeleteSpotOverlay(self->validTargetOverlay);
    DeleteSpotOverlay(self->invalidTargetOverlay);
    ReleaseSpotBitmap(self->indicatorBitmap);
    ReleaseSpotBitmap(self->fallingBitmap);
    ReleaseSpotBitmap(self->slopeBitmap);
    DeleteSpotOverlay(self->playerMarkerOverlay);
    ReleaseSpotBitmap(self->scoreTargetBitmap);
    ReleaseSpotBitmap(self->targetHoleBitmap);
    if (self->auxiliaryMarker) { OverlayPlayer_Destroy((void*)self->auxiliaryMarker, 1); self->auxiliaryMarker = 0; }
    if (self->playerNumberText) { ResourceHandle_Dtor((void*)self->playerNumberText, 1); self->playerNumberText = 0; }
    if (self->distanceText) { ResourceHandle_Dtor((void*)self->distanceText, 1); self->distanceText = 0; }
    if (self->clubRangeInfo) {
        if (self->clubRangeInfo) {
            void* object = (void*)self->clubRangeInfo;
            ((OwnedSpotObject*)object)->destroy(1);
        }
        self->clubRangeInfo = 0;
    }
    if (self->markerVertices) {
        if (self->markerVertices)
            __dla__FPv((char*)self->markerVertices - 0x10);
        self->markerVertices = 0;
    }
}

// Show player markers and queue the secondary task interface for resource preloading.
void CSpot_OnPreLoadInit(void* arg) {
    SpotState* p = (SpotState*)arg;
    AsCSpot(p)->playerMarkersVisible = 1;
    void* adjusted = p;
    if (p != 0) adjusted = &p->baseVtable;
    ResourcePreloadQueue_AddRequest(g_sceneManager, kSpotPreloadResourceKind, 0, kFloat_1_spot, adjusted, kFloat_0_spot, spotIndicatorStrings.taskName, 0, 0, 0);
}

// Destroy the club-range info base and optionally free its allocation.
void* CClubRangeInfo_dtor(void* ptr, int shouldDelete) {
    if (ptr) {
        __dt__9CInfoBaseFv(ptr, 0);
        if (shouldDelete > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}

// Construct the actor, texture bindings, and four projected-screen record links.
SpotState* CSpot_Ctor(SpotState* self) {
    __ct__18NativeBase80201D18Fv(self);
    self->vtable = __vt__5CSpot;
    self->baseVtable = __vt__5CSpot + 0x3c;
    self->distanceText = 0;
    self->playerNumberText = 0;
    self->clubRangeInfo = 0;
    self->indicatorBitmap = 0;
    self->fallingBitmap = 0;
    self->slopeBitmap = 0;
    self->targetHoleBitmap = 0;
    self->arrowOverlay = 0;
    self->playerMarkerOverlay = 0;
    self->auxiliaryMarker = 0;
    __construct_array(self->textureBindings, CBpTex_Init, CBpTex_dtor, sizeof(SpotTextureBinding), kSpotTextureCount);
    self->scoreTargetBitmap = 0;
    self->markerVertices = 0;
    self->messageValueAtLoad = 0;
    IActor_AddSkipFlags(self, kSpotActorSkipFlags);
    self->markerScreenLink = &self->screenRecords[0];
    self->displayPriority = 1;
    self->ballScreenLink = &self->screenRecords[1];
    self->fourthScreenLink = &self->screenRecords[3];
    self->thirdScreenLink = &self->screenRecords[2];
    self->screenRecordCount = 0;
    self->validTargetOverlay = 0;
    self->invalidTargetOverlay = 0;
    return self;
}

// Wait for textures, handle the HIDE action, and animate/recompute the shot landing marker.
void CSpot_OnProcess(SpotState* self, float deltaTime) {
    float animationStep = deltaTime;
    int allTexturesReady = 1;
    SpotTextureBinding* binding = self->textureBindings;
    for (int i = 0; i < kSpotTextureCount; ++i) {
        int bindingReady = ((TextureBindingReadiness*)binding)->isReady();
        allTexturesReady = allTexturesReady & bindingReady;
        ++binding;
    }
    // Phase 1: all texture bindings must be ready before input or marker animation.
    if (allTexturesReady) {

        // Phase 2: reset placement, tutorial heading, and input visibility.
        self->targetHeightOffset = kFloat_0_spot;
        __as__7WVectorFRC7WVector(AsCSpot(self)->position /* 0x24 */, AsCSpot(self)->targetPosition /* 0x68 */);
        {
            void* p = InstanceTable_Get();
            ((ShotMarkerParameters*)p)->markerHeightOffset = kFloat_0_spot;
        }
        {
            void* g = (void*)g_sharedDoc;
            void* obj = (void*)((CSharedDoc*)g)->golfDoc;
            unsigned char b = ((CGolfDoc*)obj)->tutorialMarkerVisible /* 0xB8 */;
            if (b) {
                float f1 = AsCSpot(self)->markerHeading;
                f1 = kFloat_Pi_Spot * animationStep + f1;
                if (f1 > kFloat_TwoPi_Spot) f1 = f1 - kFloat_TwoPi_Spot;
                AsCSpot(self)->markerHeading = f1;
            }
        }
        {
            void* p = InstanceTable_Get();
            if (((ShotMarkerParameters*)p)->flags & kRefreshTargetFlag) self->targetDirty = 1;
        }
        {
            const SpotIndicatorStrings* actions = &spotIndicatorStrings;
            // HIDE toggles player markers only when neither control action is held.
            int r = Pointer_IsActionActive((void*)g_inputPointer, (void*)actions->leftControlAction, 1);
            if (!r) {
                r = Pointer_IsActionActive((void*)g_inputPointer, (void*)actions->rightControlAction, 1);
                if (!r) {
                    r = Pointer_GetRisingActionBits((void*)g_inputPointer, (void*)actions->hideAction, 1);
                    if (r) {
                        self->playerMarkersVisible = (self->playerMarkersVisible == 0) ? 1 : 0;
                    }
                }
            }
        }
        // Phase 3: advance marker phases, refresh ground placement, then animate lie feedback.
        if (self->enabled) {

            {
                float f1 = self->rotationPhase;
                f1 = kFloat_Pi_Spot * animationStep + f1;
                self->rotationPhase = f1;
                if (f1 > kFloat_TwoPi_Spot) {
                    self->rotationPhase = f1 - kFloat_TwoPi_Spot;
                }
            }
            {
                float f1 = self->pulsePhase;
                f1 = kFloat_0_5_Marker * animationStep + f1;
                self->pulsePhase = f1;
                if (f1 > kFloat_1_spot) {
                    self->pulsePhase = kFloat_0_spot;
                }
            }
            // Phase 4: resolve ball + heading-rotated range to a world-space ground point.
            if (self->targetDirty) {
                float f1 = self->positionBlend;
                float f0 = g_spotPositionBlendReference;
                f1 = f1 - f0;
                float af1 = f1;
                unsigned int u = *(unsigned int*)&af1;
                if (u & kFloatSignBit) af1 = -af1;
                volatile bool nearTarget = af1 < kSpotComparisonEpsilon;
                if (nearTarget) {
                    void* p = InstanceTable_Get();
                    if (((ShotMarkerParameters*)p)->flags & kRefreshTargetFlag) self->pulsePhase = kFloat_0_spot;
                    self->targetDirty = 0;
                    self->groundHitIndex = 0;
                    float headingMatrix[16];
                    Matrix_SetAxisRotation(headingMatrix, AsCSpot(self)->heading, 1);
                    void* p2 = InstanceTable_Get();
                    float rangeYards = GolfParameter_ResolveFloat(p2, 0);
                    float rangeWorldUnits = kSpotWorldUnitsPerYard * rangeYards;
                    float rangeOffset[3];
                    __ml__FfRC7WVector(rangeOffset, headingMatrix + 6, rangeWorldUnits);
                    void* r = GolfBall__Fv();
                    float ballPlusRange[3];
                    __pl__FRC7WVectorRC7WVector(ballPlusRange, r, rangeOffset);
                    float targetCopy[3];
                    __as__7WVectorFRC7WVector(targetCopy, ballPlusRange);
                    // World-space landing candidate: ball + rotated range; copy for the ground-query ABI.
                    float groundQueryPosition[3];
                    groundQueryPosition[0] = targetCopy[0];
                    groundQueryPosition[1] = targetCopy[1];
                    groundQueryPosition[2] = targetCopy[2];
                    void* m = CQuadTree_GetInstance();
                    float groundPosition[3];
                    GetGroundPoint__9CQuadTreeF7WVectorbPib(groundPosition, m, groundQueryPosition, 1, &AsCSpot(self)->groundHitIndex /* 0x78 */, 1);
                    __as__7WVectorFRC7WVector(AsCSpot(self)->targetPosition /* 0x68 */, groundPosition);
                    {
                        float f1 = self->targetHeightOffset;
                        float f0 = g_spotHeightReference;
                        f1 = f1 - f0;
                        float af = f1;
                        unsigned int uu = *(unsigned int*)&af;
                        if (uu & kFloatSignBit) af = -af;
                        volatile bool nearGround = af < kSpotComparisonEpsilon;
                        if (nearGround) {
                            // Snap to the ground target when the height offset is within tolerance.
                            __as__7WVectorFRC7WVector(AsCSpot(self)->position /* 0x24 */, AsCSpot(self)->targetPosition /* 0x68 */);
                        }
                    }
                }
            }

            {
                void* p = InstanceTable_Get();
                // Phase 5: animate the arrow with the current club and ground lie.
                SelectedClubState* q = ((ShotMarkerParameters*)p)->selectedClub;
                unsigned char clubKind = q->kind;
                if (clubKind != kPuttingClubKind) {
                    float f1 = self->positionBlend;
                    float f0 = g_spotArrowPositionBlendReference;
                    f1 = f1 - f0;
                    float af1 = f1;
                    unsigned int u = *(unsigned int*)&af1;
                    if (u & kFloatSignBit) af1 = -af1;
                    volatile bool nearDirection = af1 < kSpotComparisonEpsilon;
                    if (!nearDirection) {
                        AsCSpot(self)->arrowProgress = kFloat_1_spot;
                    } else {
                        float f2 = kFloat_2_Marker;
                        float blendDirection = self->arrowBlendDirection;
                        float previousBlend = self->arrowBlend;
                        f2 = f2 * blendDirection;
                        float one = kFloat_1_spot;
                        f2 = f2 * animationStep + previousBlend;
                        self->arrowBlend = f2;
                        if (f2 > one) {
                            self->arrowBlend = one;
                            self->arrowBlendDirection = kFloat_neg1_U800EFEAC;
                        } else {
                            float zero = kFloat_0_spot;
                            if (zero > f2) {
                                self->arrowBlend = zero;
                                self->arrowBlendDirection = one;
                            }
                        }
                        void* m = CQuadTree_GetInstance();
                        float lowerLieScale = CQuadTree_GetMinPowerOnGround(m);
                        float blend = self->arrowBlend;
                        float unitWeight = kFloat_1_spot;
                        // Visual sweep between the lower/upper lie scales with weight blend^2.
                        // The triangular blend reverses at 0/1; squaring slows its lower-end sweep.
                        // This sets marker arrow progress, not ball-flight physics; preserve FP order.
                        float lowerWeight = unitWeight - blend * blend; // fnmsubs: 1 - blend^2
                        float weightedLowerGround = lowerWeight * lowerLieScale;
                        void* m2 = CQuadTree_GetInstance();
                        float upperLieScale = CQuadTree_GetMaxPowerOnGround(m2);
                        float blendForUpper = self->arrowBlend;
                        AsCSpot(self)->arrowProgress = blendForUpper * blendForUpper * upperLieScale + weightedLowerGround;
                    }
                } else {
                    AsCSpot(self)->arrowProgress = kFloat_1_spot;
                }
            }

        }
    }
    return;
}

} // extern "C"
