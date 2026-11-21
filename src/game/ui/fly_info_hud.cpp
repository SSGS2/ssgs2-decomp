#include "classes/CFlyInfoFields.h"
#include <decomp/record_sizes.h>
#include "classes/CSharedDoc.h"
// Borrow the shared document's typed layout; no allocation or ownership change.
template <typename T> inline CSharedDoc* AsCSharedDoc(T p) { return (CSharedDoc*)p; }
#include "classes/WView.h"
// Borrow the current view's typed layout.
template <typename T> inline WView* AsWView(T p) { return (WView*)p; }
#include "classes/CGolfDoc.h"
// Borrow the golf document's typed layout.
template <typename T> inline CGolfDoc* AsCGolfDoc(T p) { return (CGolfDoc*)p; }
#include "game/types.h"
#include "game/wvector.h"
#include "sdk_protos.h"
// Translation unit 0x800F47F4-0x800F5500 (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct ClubRangeDisplay {
    char reserved_00[0x14];
    void* textBuffer;
    Vec3f rangePoint;
};
struct FlyLayoutObject { char pad[0x10]; void* vtable; };
// Owned members: textBuffer is new[] storage; text overlays/layouts are constructed
// objects; fonts are resource-manager handles. dartsResultLayout is the mode-1
// result layout; modeTwoResultLayout also shows points, and
// modeTwoShotDetailLayout holds mode-2 per-shot detail rows.
// These views overlap CFlyInfoFields and must retain the same offsets.
struct FlyHudResources {
    void* vtable;
    void* fontResource;
    char pad08[8];
    void* textMode; // Integer mode word in this ABI overlay; CFlyInfoFields uses int.
    void* textBuffer;
    float elapsedTime;
    void* groundText;
    void* pinText;
    FlyLayoutObject* dartsResultLayout;
    FlyLayoutObject* modeTwoResultLayout;
    FlyLayoutObject* modeTwoShotDetailLayout;
    void* labelFontResource;
};

extern void* g_view;
extern void* g_resourceManager;
extern char __vt__8CFlyInfo[], cInfoBaseVtable[];
extern char golfHudFontStrings[];

// Packed string-pool view; sizes include terminators and intervening bytes.
struct FlyHudStrings {
    char prefix[0x85];
    char largeFont[0xD]; // +0x85
    char reserved_92[0x185];
    char partyPointsFont[0x10]; // +0x217
    char reserved_227[0x71];
    char rangeWholeYards[0x6]; // +0x298
    char groundAtlas[0xC]; // +0x29E
    char fairway[0x8]; // +0x2AA
    char rough[0x6]; // +0x2B2
    char green[0x6]; // +0x2B8
    char bunker[0x7]; // +0x2BE
    char tee[0x4]; // +0x2C5
    char ice[0x4]; // +0x2C9
    char sand[0x5]; // +0x2CD
    char water[0x6]; // +0x2D2
    char partyResultLayout[0x15]; // +0x2D8
    char partyResultAtlas[0x1B]; // +0x2ED
    char groundTextAtlas[0x11]; // +0x308
    char carryLabel[0x7]; // +0x319
    char remainingDistanceLabel[0x9]; // +0x320
    char emptyText[0x1]; // +0x329
};
// Borrow named entries in the fixed packed string pool without copying it.
inline FlyHudStrings& FlyStrings() { return *(FlyHudStrings*)golfHudFontStrings; }

extern float kFlightCarryFadeStartTimes[];
extern float kFlightDetailFadeStartTimes[];
extern const float kFloat_0_hud, kFloat_3_2_hud, kFloat_25_805DE368, kFloat_3_Hud, kFloat_1_805DE314;
extern float kFloat_128_Hud, kFloat_50_HudLayout, kGroundAtlasBunkerY, kFloat_100_hud, kGroundAtlasIceY, kGroundAtlasSandY, kGroundAtlasWaterY;
extern float kFlightCarryLabelWidth, kFloat_18_805DE3C4, kCupHeightLevelTextYOffset, kFlightRemainingDistanceLabelWidth, kFlightRemainingDistanceLabelHeight;
extern const float kFlightFirstUpdateTimeEpsilon, kFlightSecondaryPinFadeStartTime, kFloat_4_FlyInfo, kFloat_255_MessageUI, kFloat_2_805DE318;
extern float kFlightPartyModeOneFadeStart0, kFlightPartyModeOneFadeStart1;
extern void* g_sharedDoc;
extern Vec3f* PreData__9CGolfBallFi(void*, int);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern unsigned char GetGroundType__9CQuadTreeF7WVector(void*, void*);
extern void* CQuadTree_GetInstance();
extern void __ct__9CInfoBaseFv(void*);
extern void* __nwa__FUl(int);
extern void* __nw__FUl(unsigned int);
extern void* OverlayPlayer_Init(void*, const char*);
extern void Register_NormalizedNamedRectangle(void*, const char*, float*, int);
extern void CSimpleSpd_Ctor(void*, const char*, const char*);
extern void Matrix_TransformPoint(void*, void*, void*);
extern void WView_ProjectViewPoint(void*, void*, void*, int);
extern void* GolfBall__Fv();
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float Magnitude__7WVectorCFv(void*);
extern void HudFont_DrawAlignedText(void*, void*, float, float, float, float, int, int);
extern void OverlayPlayer_Destroy(void*, int);
extern void ResourceManager_DeleteResource(void*, void*);
extern void __dla__FPv(void*);
extern void __dl__FPv(void*);
extern void* memset(void*, int, unsigned int);

struct HudAtlasRectangle { float x, y, w, h; };

// Ball physics/playback state returned by GolfBall__Fv (not the rendered CBall).
struct FlightBallState {
    Vec3f position;
    char reserved_0C[0xC];
    int restartFlightDisplay; // Nonzero restarts the distance/fade display.
    char reserved_1C[0x3C];
    unsigned char displayUpdateBlocked;
    char reserved_59[3];
    int flightPhase; // Zero is the phase in which distance text fades in.
    char reserved_60[0x1983C];
    int encodedResult;
    char reserved_198A0[4];
    int resultCheck;
};
struct HolePinRecord { char reserved_00[0xC]; Vec3f pinPosition; char reserved_18[8]; };
enum GroundType { kGroundTee = 0, kGroundFairway = 1, kGroundGreen = 2,
    kGroundBunker = 3, kGroundRough = 4, kGroundIce = 7, kGroundSand = 10, kGroundWater = 12 };
// Party mode names remain numeric until the mode-to-game mapping is verified.
enum { kGolfStateFlightDisplay = 0x80, kFlightDisplayPhase = 0,
       kPartyModeOne = 1, kPartyModeTwo = 2,
       kFlightTextBufferBytes = 0x20, kFlightNumericTextMode = 2,
       kCarryFadeCount = 3, kDetailFadeCount = 5, kModeOneFadeCount = 2 };
typedef CFlyInfoFields FlyDisplay;
typedef char FlightBallState_restart_offset[((int)&((FlightBallState*)0)->restartFlightDisplay == 0x18) ? 1 : -1];
typedef char FlightBallState_phase_offset[((int)&((FlightBallState*)0)->flightPhase == 0x5C) ? 1 : -1];
typedef char FlightBallState_result_offset[((int)&((FlightBallState*)0)->encodedResult == 0x1989C) ? 1 : -1];
typedef char HolePinRecord_size[(sizeof(HolePinRecord) == 0x20) ? 1 : -1];


struct FadeSchedule5 { float v[5]; };
struct FadeSchedule2 { float x, y; };
struct Arr2u { unsigned int v[2]; };

// Clamp one scalar opacity to [0, maximum], retaining the original float comparisons.
inline float ClampOpacity(float value, float maximum) {
    return value < kFloat_0_hud ? kFloat_0_hud : (value > maximum ? maximum : value);
}
// Clamp a scheduled opacity in place; the lower-bound comparison uses double precision.
inline void ClampScheduledOpacity(float& value, double minimum, float maximum) {
    if (value < minimum) value = minimum;
    else if (value > maximum) value = maximum;
}
// Test the stored sign bit, including negative zero, exactly as the original does.
inline float AbsoluteBySignBit(const float& value) {
    if ((*(const unsigned int*)&value & 0x80000000u) != 0) return -value;
    else return value;
}

#pragma opt_common_subs off
// Reset shot distances on flight entry, then update ground labels and staggered HUD fades.
// self and the shared golf/ball documents must exist. deltaSeconds advances elapsedTime
// only in phase 0 and after reset/block checks; reset returns without advancing it.
// Positions are world space, Y is vertical, and horizontal distances divide world
// units by 3.2 to obtain yards. Opacity is stored as float in [0,255].
void CFlyInfo_UpdateFlightFadesAndDistances(FlyDisplay* self, float deltaSeconds) {
    register float fadeMultiplier;
    float maximumOpacity, groundOpacity, pinOpacity, secondaryPinOpacity, value, elapsedSinceFade, elapsedTime, fadeRate;
    double minimumOpacity;
    FadeSchedule5 detailFadeTimes;
    Vec3f carryFadeTimes;
    Vec3f groundProbePosition;
    Vec3f pinPositionOnReset, pinDeltaOnReset, startDeltaOnReset, horizontalDeltaOnReset;
    Vec3f pinPositionAtFirstFrame, pinDeltaAtFirstFrame, startDeltaAtFirstFrame, horizontalDeltaAtFirstFrame;
    volatile FadeSchedule2 modeOneFadeTimes;

    // Gate on the flight-display bit and remember it for rising-edge detection.
    int flightJustStarted = 0;
    CGolfDoc* golfDoc = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc);
    unsigned int golfState = (unsigned int)golfDoc->gameStateFlags;
    if ((golfState & kGolfStateFlightDisplay) == 0) { self->previousGolfState = golfState; return; }
    unsigned int previousState = self->previousGolfState;
    if ((previousState & kGolfStateFlightDisplay) == 0) flightJustStarted = 1;
    self->previousGolfState = golfState;
    FlightBallState* ball = (FlightBallState*)GolfBall__Fv();
    // On entry/restart, clear all fades and snapshot horizontal distances, then return.
    if (ball->restartFlightDisplay != 0 || flightJustStarted != 0) {
        float z = kFloat_0_hud;
        self->elapsedTime = z;
        self->secondaryPinTextOpacity = z;
        self->pinTextOpacity = z;
        self->groundTextOpacity = z;
        self->carryValueOpacity = z;
        self->carryLabelsOpacity = z;
        self->currentShotValueOpacity = z;
        self->shotDetailOpacities[0] = z;
        self->shotDetailOpacities[1] = z;
        self->shotDetailOpacities[2] = z;
        self->shotDetailOpacities[3] = z;
        self->shotDetailOpacities[4] = z;
        self->dartsTotalOpacity = z;
        self->dartsBreakdownOpacity = z;
        Vec3f* startPosition = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
        __mi__FRC7WVectorRC7WVector(&startDeltaOnReset, GolfBall__Fv(), startPosition);
        *(WVector*)&horizontalDeltaOnReset = *(WVector*)&startDeltaOnReset;
        horizontalDeltaOnReset.y = kFloat_0_hud;
        self->distanceFromStart = (*(WVector*)&horizontalDeltaOnReset).Magnitude() / kFloat_3_2_hud;
        golfDoc = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned char holeIndex = AsCGolfDoc(golfDoc)->currentHoleIndex;
        HolePinRecord* pinRecord = (HolePinRecord*)AsCGolfDoc(golfDoc)->holeRecords + holeIndex;
        *(Arr3*)&pinPositionOnReset = *(Arr3*)&pinRecord->pinPosition;
        __mi__FRC7WVectorRC7WVector(&pinDeltaOnReset, GolfBall__Fv(), &pinPositionOnReset);
        *(WVector*)&horizontalDeltaOnReset = *(WVector*)&pinDeltaOnReset;
        horizontalDeltaOnReset.y = kFloat_0_hud;
        self->distanceToPin = (*(WVector*)&horizontalDeltaOnReset).Magnitude() / kFloat_3_2_hud;
        return;
    }
    if (ball->displayUpdateBlocked != 0) return;
    switch (ball->flightPhase) {
    default: break;
    case kFlightDisplayPhase:
        {
            float absoluteElapsedTime;
            absoluteElapsedTime = AbsoluteBySignBit(self->elapsedTime);
            int firstFlightUpdate = absoluteElapsedTime < kFlightFirstUpdateTimeEpsilon;
            // The first unblocked phase-0 update refreshes distances and the ground name.
            if (firstFlightUpdate) {
                Vec3f* startPosition = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
                __mi__FRC7WVectorRC7WVector(&startDeltaAtFirstFrame, GolfBall__Fv(), startPosition);
                *(WVector*)&horizontalDeltaAtFirstFrame = *(WVector*)&startDeltaAtFirstFrame;
                horizontalDeltaAtFirstFrame.y = kFloat_0_hud;
                self->distanceFromStart = (*(WVector*)&horizontalDeltaAtFirstFrame).Magnitude() / kFloat_3_2_hud;
                golfDoc = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc);
                unsigned char holeIndex = AsCGolfDoc(golfDoc)->currentHoleIndex;
                HolePinRecord* pinRecord = (HolePinRecord*)AsCGolfDoc(golfDoc)->holeRecords + holeIndex;
                *(Arr3*)&pinPositionAtFirstFrame = *(Arr3*)&pinRecord->pinPosition;
                __mi__FRC7WVectorRC7WVector(&pinDeltaAtFirstFrame, GolfBall__Fv(), &pinPositionAtFirstFrame);
                *(WVector*)&horizontalDeltaAtFirstFrame = *(WVector*)&pinDeltaAtFirstFrame;
                horizontalDeltaAtFirstFrame.y = kFloat_0_hud;
                self->distanceToPin = (*(WVector*)&horizontalDeltaAtFirstFrame).Magnitude() / kFloat_3_2_hud;

                *(Arr3*)&groundProbePosition = *(Arr3*)&ball->position;
                unsigned char groundType = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &groundProbePosition);
                switch (groundType) {
                case kGroundFairway: strcpy(self->groundName, FlyStrings().fairway); break;
                case kGroundGreen: strcpy(self->groundName, FlyStrings().green); break;
                case kGroundRough: strcpy(self->groundName, FlyStrings().rough); break;
                case kGroundBunker: strcpy(self->groundName, FlyStrings().bunker); break;
                case kGroundTee: strcpy(self->groundName, FlyStrings().tee); break;
                case kGroundIce: strcpy(self->groundName, FlyStrings().ice); break;
                case kGroundSand: strcpy(self->groundName, FlyStrings().sand); break;
                case kGroundWater: strcpy(self->groundName, FlyStrings().water); break;
                default: strcpy(self->groundName, FlyStrings().rough); break;
                }
            }
            if ((ball->encodedResult ^ ~ball->resultCheck) == 1) {
                int partyMode = (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode);
                // With this encoded result, only party modes 1 and 2 continue updating.
                if (partyMode == kPartyModeTwo) {
                    // Both supported modes share the adjustment below.
                } else if (partyMode != kPartyModeOne) break;
                {
                    float absoluteElapsedTime2;
                    absoluteElapsedTime2 = AbsoluteBySignBit(self->elapsedTime);
                    int firstResultUpdate = absoluteElapsedTime2 < kFlightFirstUpdateTimeEpsilon;
                    if (firstResultUpdate) {
                        self->distanceToPin = kFloat_0_hud;
                        strcpy(self->groundName, FlyStrings().emptyText);
                    }
                }
            }
            // Advance the clock once; accumulate time-relative opacity, then clamp.
            // This adds (elapsed - start) * rate each update, rather than assigning
            // an opacity from elapsed time. Preserve that recurrence when editing.
            float nextTime = self->elapsedTime + deltaSeconds;
            fadeMultiplier = kFloat_2_805DE318;
            maximumOpacity = kFloat_255_MessageUI;
            self->elapsedTime = nextTime;
            groundOpacity = ClampOpacity(nextTime * (fadeMultiplier * maximumOpacity) + self->groundTextOpacity, maximumOpacity);
            self->groundTextOpacity = groundOpacity;
            pinOpacity = ClampOpacity((self->elapsedTime - kFloat_1_805DE314) * (fadeMultiplier * maximumOpacity) + self->pinTextOpacity, maximumOpacity);
            self->pinTextOpacity = pinOpacity;
            secondaryPinOpacity = ClampOpacity((self->elapsedTime - kFlightSecondaryPinFadeStartTime) * (fadeMultiplier * maximumOpacity) + self->secondaryPinTextOpacity, maximumOpacity);
            self->secondaryPinTextOpacity = secondaryPinOpacity;
            int partyMode = (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode);
            if (partyMode == kPartyModeTwo) {
                // Mode 2: three adjacent carry/current-shot fades vanish after 4 seconds;
                // the five detail fades continue on their own schedule.
                if (self->elapsedTime < kFloat_4_FlyInfo) {
                    *(Arr3*)&carryFadeTimes = *(Arr3*)kFlightCarryFadeStartTimes;
                    minimumOpacity = kFloat_0_hud;
                    for (int j = 0; j < kCarryFadeCount; j++) {
                        float value;
                        value = (fadeMultiplier * maximumOpacity) * (self->elapsedTime - (&carryFadeTimes.x)[j]) + (&self->carryValueOpacity)[j];
                        ClampScheduledOpacity(value, minimumOpacity, maximumOpacity);
                        (&self->carryValueOpacity)[j] = value;
                    }
                } else {
                    float z = kFloat_0_hud;
                    self->carryValueOpacity = z;
                    self->carryLabelsOpacity = z;
                    self->currentShotValueOpacity = z;
                    self->pinTextOpacity = z;
                }
                detailFadeTimes = *(FadeSchedule5*)kFlightDetailFadeStartTimes;
                minimumOpacity = kFloat_0_hud;
                fadeRate = fadeMultiplier * maximumOpacity;
                for (int i = 0; i < kDetailFadeCount; i++) {
                    elapsedTime = self->elapsedTime; elapsedSinceFade = detailFadeTimes.v[i]; value = self->shotDetailOpacities[i];
                    elapsedSinceFade = elapsedTime - elapsedSinceFade;
                    value = fadeRate * elapsedSinceFade + value;
                    ClampScheduledOpacity(value, minimumOpacity, maximumOpacity);
                    self->shotDetailOpacities[i] = value;
                }
            }
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyModeOne) {
                // Mode 1: adjacent +0x64/+0x68 fades drive darts total and breakdown.
                minimumOpacity = kFloat_0_hud;
                *(unsigned int*)&modeOneFadeTimes.x = *(unsigned int*)&kFlightPartyModeOneFadeStart0;
                *(unsigned int*)&modeOneFadeTimes.y = *(unsigned int*)&kFlightPartyModeOneFadeStart1;
                for (int j = 0; j < kModeOneFadeCount; j++) {
                    float value;
                    value = (fadeMultiplier * maximumOpacity) * (self->elapsedTime - (&modeOneFadeTimes.x)[j]) + (&self->dartsTotalOpacity)[j];
                    ClampScheduledOpacity(value, minimumOpacity, maximumOpacity);
                    (&self->dartsTotalOpacity)[j] = value;
                }
            }
        }
    }
}

#pragma opt_common_subs reset
struct VtF48B4 { void* s0; void* s1; void* s2; void* s3; void* s4; void (*m5)(void*, const char*, void*, int); };
class HudFontSetup { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void setFont(const char*, void*, int);
};
struct FlightTextOverlay {
    char pad[0x50];
    // Initialize an overlay from its atlas resource name.
    FlightTextOverlay(const char* atlasName) { OverlayPlayer_Init(this, atlasName); }
};
struct PartyResultLayout {
    char pad[0x14];
    // Initialize a party layout from the layout and atlas resource names, in that order.
    PartyResultLayout(const char* layoutName, const char* atlasName) { CSimpleSpd_Ctor(this, layoutName, atlasName); }
};

#pragma opt_common_subs off
} // extern C
struct CInfoBase {
    char storage[0x88];
    // Initialize the base HUD and null the derived members needed for cleanup.
    CInfoBase() {
        FlyHudResources* self = (FlyHudResources*)this;
        __ct__9CInfoBaseFv(self);
        self->groundText = 0;
        self->vtable = __vt__8CFlyInfo;
        self->pinText = 0;
        self->dartsResultLayout = 0;
        self->modeTwoResultLayout = 0;
        self->modeTwoShotDetailLayout = 0;
        AsCFlyInfoFields(self)->previousGolfState = 0;
        memset(AsCFlyInfoFields(self)->labelFont, 0, 8);
    }
    ~CInfoBase();
};
struct CFlyInfo : CInfoBase {
    CFlyInfo();
};
// Create owned text storage/overlays and the layouts required by the current party mode.
CFlyInfo::CFlyInfo() { HudAtlasRectangle carryLabelRectangle;
    // Load fonts and create the ground-name atlas; members become owned by this HUD.
    FlyHudResources* self = (FlyHudResources*)this;
    ((HudFontSetup*)self)->setFont(FlyStrings().largeFont, 0, 1);
    ((HudFontSetup*)self)->setFont(FlyStrings().partyPointsFont, AsCFlyInfoFields(self)->labelFont, 1);
    self->textBuffer = __nwa__FUl(kFlightTextBufferBytes);
    self->textMode = (void*)kFlightNumericTextMode;
    self->groundText = new FlightTextOverlay(FlyStrings().groundAtlas);

    // Register atlas rectangles in pixels: x/y origin and width/height, not UVs.
    // Ground ID -> label: 0 tee, 1 fairway, 2 green, 3 bunker, 4 rough,
    // 7 ice, 10 sand, 12 water. Unknown IDs use rough in the updater.
    HudAtlasRectangle groundRectangles[8];
    groundRectangles[7].x=kFloat_0_hud; groundRectangles[7].y=kFloat_0_hud;
    groundRectangles[7].w=kFloat_128_Hud; groundRectangles[7].h=kFloat_25_805DE368;
    Register_NormalizedNamedRectangle(self->groundText, FlyStrings().fairway, &groundRectangles[7].x, 0);
    groundRectangles[6].x=kFloat_0_hud; groundRectangles[6].y=kFloat_25_805DE368;
    groundRectangles[6].w=kFloat_128_Hud; groundRectangles[6].h=kFloat_25_805DE368;
    Register_NormalizedNamedRectangle(self->groundText, FlyStrings().rough, &groundRectangles[6].x, 0);
    groundRectangles[5].x=kFloat_0_hud; groundRectangles[5].y=kFloat_50_HudLayout;
    groundRectangles[5].w=kFloat_128_Hud; groundRectangles[5].h=kFloat_25_805DE368;
    Register_NormalizedNamedRectangle(self->groundText, FlyStrings().green, &groundRectangles[5].x, 0);
    groundRectangles[4].x=kFloat_0_hud; groundRectangles[4].y=kGroundAtlasBunkerY;
    groundRectangles[4].w=kFloat_128_Hud; groundRectangles[4].h=kFloat_25_805DE368;
    Register_NormalizedNamedRectangle(self->groundText, FlyStrings().bunker, &groundRectangles[4].x, 0);
    groundRectangles[3].x=kFloat_0_hud; groundRectangles[3].y=kFloat_100_hud;
    groundRectangles[3].w=kFloat_128_Hud; groundRectangles[3].h=kFloat_25_805DE368;
    Register_NormalizedNamedRectangle(self->groundText, FlyStrings().tee, &groundRectangles[3].x, 0);
    groundRectangles[2].x=kFloat_0_hud; groundRectangles[2].y=kGroundAtlasIceY;
    groundRectangles[2].w=kFloat_128_Hud; groundRectangles[2].h=kFloat_25_805DE368;
    Register_NormalizedNamedRectangle(self->groundText, FlyStrings().ice, &groundRectangles[2].x, 0);
    groundRectangles[1].x=kFloat_0_hud; groundRectangles[1].y=kGroundAtlasSandY;
    groundRectangles[1].w=kFloat_128_Hud; groundRectangles[1].h=kFloat_25_805DE368;
    Register_NormalizedNamedRectangle(self->groundText, FlyStrings().sand, &groundRectangles[1].x, 0);
    groundRectangles[0].x=kFloat_0_hud; groundRectangles[0].y=kGroundAtlasWaterY;
    groundRectangles[0].w=kFloat_128_Hud; groundRectangles[0].h=kFloat_25_805DE368;
    Register_NormalizedNamedRectangle(self->groundText, FlyStrings().water, &groundRectangles[0].x, 0);

    // Choose mode-specific layouts: mode 1 owns one, mode 2 owns two, others none.
    void* sharedDoc = (void*)g_sharedDoc;
    int partyMode = (*(int*)&AsCSharedDoc(sharedDoc)->partyGameMode);
    if (partyMode == kPartyModeOne) {
        void* p = new PartyResultLayout(FlyStrings().partyResultLayout, FlyStrings().partyResultAtlas);
        self->dartsResultLayout = (FlyLayoutObject*)p;
    }
    sharedDoc = (void*)g_sharedDoc;
    partyMode = (*(int*)&AsCSharedDoc(sharedDoc)->partyGameMode);
    if (kPartyModeTwo == partyMode) {
        void* p = new PartyResultLayout(FlyStrings().partyResultLayout, FlyStrings().partyResultAtlas);
        self->modeTwoResultLayout = (FlyLayoutObject*)p;
        p = new PartyResultLayout(FlyStrings().partyResultLayout, FlyStrings().partyResultAtlas);
        self->modeTwoShotDetailLayout = (FlyLayoutObject*)p;
    }

    // Create carry/remaining-distance labels; mode 2 supplies its own remaining label.
    self->pinText = new FlightTextOverlay(FlyStrings().groundTextAtlas);
    
    carryLabelRectangle.x = kFloat_1_805DE314; carryLabelRectangle.y = kFloat_1_805DE314;
    carryLabelRectangle.w = kFlightCarryLabelWidth; carryLabelRectangle.h = kFloat_18_805DE3C4;
    Register_NormalizedNamedRectangle(self->pinText, FlyStrings().carryLabel, &carryLabelRectangle.x, 0);

    if (!self->modeTwoResultLayout) {
        HudAtlasRectangle remainingDistanceRectangle;
        remainingDistanceRectangle.x = kFloat_1_805DE314;
        remainingDistanceRectangle.y = kCupHeightLevelTextYOffset;
        remainingDistanceRectangle.w = kFlightRemainingDistanceLabelWidth;
        remainingDistanceRectangle.h = kFlightRemainingDistanceLabelHeight;
        Register_NormalizedNamedRectangle(self->pinText, FlyStrings().remainingDistanceLabel, &remainingDistanceRectangle.x, 0);
    }

    self->elapsedTime = kFloat_0_hud;
}
extern "C" {

#pragma opt_common_subs reset
// Project the selected club-range point and draw its horizontal distance from the ball in yards.
// rangePoint and ball position are world space. inputTransform produces view space;
// ProjectViewPoint produces screen coordinates. textBuffer must hold the formatted
// whole-yard value; the text anchor is 25 screen units above the projected point.
void CClubRangeInfo_DrawRangeYardText(ClubRangeDisplay* self) {
    struct DrawScratch {
        Vec3f viewPoint;
        Vec3f horizontalDelta;
        struct ScreenPoint { float x, y; char reserved_08[0x28]; } screen;
    } scratch;
    Matrix_TransformPoint(&scratch.viewPoint, &self->rangePoint, (char*)AsWView(g_view)->inputTransform);
    WView_ProjectViewPoint(g_view, (Vec3f*)&scratch.screen, &scratch.viewPoint, 0);
    void* ball = GolfBall__Fv();
    __mi__FRC7WVectorRC7WVector(&scratch.horizontalDelta, &self->rangePoint, ball);
    scratch.horizontalDelta.y = kFloat_0_hud;
    float distanceYards = (*(WVector*)(&scratch.horizontalDelta)).Magnitude();
    distanceYards /= kFloat_3_2_hud;
    sprintf((char*)self->textBuffer, FlyStrings().rangeWholeYards, distanceYards);
    float screenY = scratch.screen.y;
    float screenX = scratch.screen.x;
    screenY = screenY - kFloat_25_805DE368;
    HudFont_DrawAlignedText(self, self->textBuffer, screenX, screenY, kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
}

struct Dt10B { char pad[16]; };
class Dt10 : public Dt10B { public: 
virtual void v08(int);
};
// Release flight HUD overlays, layouts, resources and text storage; optionally delete the object.
// Null is accepted. deleting > 0 frees self; the returned original address may dangle.
void* CFlyInfo_dtor(FlyHudResources* self, int deleting) {
    if (self != 0) {
        self->vtable = __vt__8CFlyInfo;
        // Destroy owned overlay/layout objects before releasing their supporting fonts.
        if (self->groundText != 0) {
            OverlayPlayer_Destroy(self->groundText, 1);
            self->groundText = 0;
        }
        if (self->pinText != 0) {
            OverlayPlayer_Destroy(self->pinText, 1);
            self->pinText = 0;
        }
        if (self->dartsResultLayout != 0) {
            if (self->dartsResultLayout != 0) {
                ((Dt10*)self->dartsResultLayout)->v08(1);
            }
            self->dartsResultLayout = 0;
        }
        if (self->modeTwoShotDetailLayout != 0) {
            if (self->modeTwoShotDetailLayout != 0) {
                ((Dt10*)self->modeTwoShotDetailLayout)->v08(1);
            }
            self->modeTwoShotDetailLayout = 0;
        }
        if (self->modeTwoResultLayout != 0) {
            if (self->modeTwoResultLayout != 0) {
                ((Dt10*)self->modeTwoResultLayout)->v08(1);
            }
            self->modeTwoResultLayout = 0;
        }
        // Release derived font, then base font and new[] text storage.
        if (self->labelFontResource != 0) {
            ResourceManager_DeleteResource(g_resourceManager, self->labelFontResource);
            self->labelFontResource = 0;
        }
        if (self != 0) {
            self->vtable = cInfoBaseVtable;
            if (self->fontResource != 0) {
                ResourceManager_DeleteResource(g_resourceManager, self->fontResource);
                self->fontResource = 0;
            }
            if (self->textBuffer != 0) {
                __dla__FPv(self->textBuffer);
                self->textBuffer = 0;
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"
