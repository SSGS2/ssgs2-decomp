// Golf HUD panels: flight distances, wind, shot-count, power and remaining-time
// text, the hole-introduction panels, Pang reward settlement and presentation,
// the calipers gauge, and the balloon and drive-distance readouts.
//
// HUD coordinates are viewport pixels, distances are yards, and opacity is
// converted to an 8-bit alpha. The panels own their fonts, overlays and text
// buffers; round and player data is borrowed and never retained.
extern "C" float kFlightDetailFadeStartTimes[];
#define CINFOBASE_NO_DTOR 1
#include "classes/COption.h"
#include "classes/GolfDocSnapshot8013.h"
#include "classes/PartyItemModifiers.h"
#include "classes/GameInputManager.h"
#include "iff/IffStruct.h"
// Byte layout of golfHudFontStrings; names and extents follow the verified string-pool map.
struct GolfHudStringPool {
    char font_hole_gsr[15];              // +0x0
    char text_1234567890ym[19];          // +0xF
    char barFont[14];                    // +0x22
    char text_1234567890ym_2[17];        // +0x30
    char font_wind_gsr[15];              // +0x41
    char text_1234567890ym_3[21];        // +0x50
    char font_club_gsr[15];              // +0x65
    char text_0123456789abcdef[17];      // +0x74
    char largeFont[13];                  // +0x85
    char text_1234567890ym_x_p[21];      // +0x92
    char num_shop_00_gsr[16];            // +0xA7
    char text_1234567890[12];            // +0xB7
    char num_sts_gsr[12];                // +0xC3
    char text_1234567890_2[11];          // +0xCF
    char gall_num_gsr[13];               // +0xDA
    char common_num_0_gsr[17];           // +0xE7
    char score_num11_20_gsr[19];         // +0xF8
    char text_1723456890[14];            // +0x10B
    char custom_num_00_gsr[18];          // +0x119
    char custom_num_01_gsr[18];          // +0x12B
    char custom_num_02_gsr[18];          // +0x13D
    char custom_num_03_gsr[18];          // +0x14F
    char num_stm_00_gsr[15];             // +0x161
    char num_system15_19_gsr[20];        // +0x170
    char text_1234567890_3[13];          // +0x184
    char num_result_19_gsr[18];          // +0x191
    char font_countdown_gsr[19];         // +0x1A3
    char num_status_00_gsr[18];          // +0x1B6
    char text_1234567890_4[13];          // +0x1C8
    char num_status_01_gsr[18];          // +0x1D5
    char text_1234567890_5[15];          // +0x1E7
    char tips_num_gsr[13];               // +0x1F6
    char text_1234567890ym_xp[20];       // +0x203
    char party_point_gsr[16];            // +0x217
    char p[2];                           // +0x227
    char tips_num_pts_gsr[17];           // +0x229
    char text_5_2f[6];                   // +0x23A
    char text_6_1f[6];                   // +0x240
    char text_1f_s[7];                   // +0x246
    char y[2];                           // +0x24D
    char text_0f_s[7];                   // +0x24F
    char top_hole_l_gsr[16];             // +0x256
    char top_hole_m_gsr[16];             // +0x266
    char top_hole_h_gsr[16];             // +0x276
    char text_1fm[6];                    // +0x286
    char text_0_00m[6];                  // +0x28C
    char text_2fm[6];                    // +0x292
    char wholeYardsFormat[6];            // +0x298
    char ground_gsr[12];                 // +0x29E
    char Fairway[8];                     // +0x2AA
    char Rough[6];                       // +0x2B2
    char Green[6];                       // +0x2B8
    char Bunker[7];                      // +0x2BE
    char Tee[4];                         // +0x2C5
    char Ice[4];                         // +0x2C9
    char Sand[5];                        // +0x2CD
    char Water[6];                       // +0x2D2
    char party_result_txt_spd[21];       // +0x2D8
    char score_party_result_txt_gsr[27]; // +0x2ED
    char ground_text_gsr[17];            // +0x308
    char localizedText[7];               // +0x319
    char localizedText_2[9];             // +0x320
    char reserved_329[1];
    char integerFormat[3];                                   // +0x32A
    char decimalPoint[2];                                    // +0x32D
    char fractionalYardsFormat[6];                           // +0x32F
    char threeColumnIntegerFormat[4];                        // +0x335
    char distanceFormat[6];                                  // +0x339
    char DartsSystem[12];                                    // +0x33F
    char slash[2];                                           // +0x34B
    char oneMeterWind[3];                                    // +0x34D
    char unknownWind[2];                                     // +0x350
    char wholeMetersFormat[6];                               // +0x352
    char add_pang_gsr[14];                                   // +0x358
    char pc_eventmark_gsr[18];                               // +0x366
    char percentFormat[5];                                   // +0x378
    char unavailable[2];                                     // +0x37D
    char itemResourceFormat[18];                             // +0x37F
    char fashionResourceFormat[25];                          // +0x391
    char impact_gsr[12];                                     // +0x3AA
    char badImpact[5];                                       // +0x3B6
    char pangyaImpact[8];                                    // +0x3BB
    char powerSpin[11];                                      // +0x3C3
    char powerCurve[12];                                     // +0x3CE
    char Tomahawk[10];                                       // +0x3DA
    char fullPower[6];                                       // +0x3E4
    char superPangya[13];                                    // +0x3EA
    char Cobra[7];                                           // +0x3F7
    char Spike[7];                                           // +0x3FE
    char PowerTomahawk[15];                                  // +0x405
    char PowerCobra[12];                                     // +0x414
    char PowerSpike[12];                                     // +0x420
    char powerPrefix[7];                                     // +0x42C
    char impactPrefix[2];                                    // +0x433
    char tomahawkSuffix[9];                                  // +0x435
    char cobraSuffix[6];                                     // +0x43E
    char spikeSuffix[6];                                     // +0x444
    char start_name_gsr[16];                                 // +0x44A
    char start_name_2_gsr[18];                               // +0x45A
    char PartyCourseName[16];                                // +0x46C
    char start_hole_gsr[16];                                 // +0x47C
    char Hole[5];                                            // +0x48C
    char Par[4];                                             // +0x491
    char start_posi_gsr[16];                                 // +0x495
    char leaderLabel[5];                                     // +0x4A5
    char vs[3];                                              // +0x4AA
    char complete_gsr[14];                                   // +0x4AD
    char score_event_gsr[17];                                // +0x4BB
    char pang_bonus_gsr[16];                                 // +0x4CC
    char pangUpSound[8];                                     // +0x4DC
    char pangDownSound[10];                                  // +0x4E4
    char multiplierFormat[4];                                // +0x4EE
    char indicator_gsr[15];                                  // +0x4F2
    char calipers_gsr[14];                                   // +0x501
    char skins_win01_gsr[16];                                // +0x50F
    char skins_win02_gsr[16];                                // +0x51F
    char skins_win03_gsr[16];                                // +0x52F
    char skins_lose01_gsr[17];                               // +0x53F
    char skins_lose02_gsr[17];                               // +0x550
    char skins_draw01_gsr[17];                               // +0x561
    char skins_draw02_gsr[17];                               // +0x572
    char text_2fy[6];                                        // +0x583
    char text_3f[5];                                         // +0x589
    char drivePlayerResourceFormat[11];                      // +0x58E
    char tree_insert_length_error[26];                       // +0x599
    char std_exception_std_logic_error_std_length_error[55]; // +0x5B3
    char reserved_tail[2];
};
typedef char GolfHudStringPool_size[(sizeof(GolfHudStringPool) == 1516) ? 1 : -1];

typedef char GolfHudStringPool_barFont[(IFF_OFFSET(GolfHudStringPool, barFont) == 0x22) ? 1 : -1];
typedef char GolfHudStringPool_largeFont[(IFF_OFFSET(GolfHudStringPool, largeFont) == 0x85) ? 1 : -1];
typedef char GolfHudStringPool_wholeYardsFormat[(IFF_OFFSET(GolfHudStringPool, wholeYardsFormat) == 0x298) ? 1 : -1];
typedef char GolfHudStringPool_integerFormat[(IFF_OFFSET(GolfHudStringPool, integerFormat) == 0x32A) ? 1 : -1];
typedef char GolfHudStringPool_badImpact[(IFF_OFFSET(GolfHudStringPool, badImpact) == 0x3B6) ? 1 : -1];
typedef char GolfHudStringPool_powerPrefix[(IFF_OFFSET(GolfHudStringPool, powerPrefix) == 0x42C) ? 1 : -1];
typedef char GolfHudStringPool_pangUpSound[(IFF_OFFSET(GolfHudStringPool, pangUpSound) == 0x4DC) ? 1 : -1];
typedef char GolfHudStringPool_calipers_gsr[(IFF_OFFSET(GolfHudStringPool, calipers_gsr) == 0x501) ? 1 : -1];
typedef char GolfHudStringPool_drivePlayerResourceFormat
    [(IFF_OFFSET(GolfHudStringPool, drivePlayerResourceFormat) == 0x58E) ? 1 : -1];
#include "classes/CFlyInfo.h"
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "classes/CBonusInfo.h"
#include "classes/CGolfDoc.h"
#include "classes/CHoleInfo.h"
#include "classes/CPangInfo.h"
#include "classes/CSharedDoc.h"
#include "classes/CWindInfo.h"
#include "classes/CRemainTimeInfo.h"
#include "classes/CShotCountInfo.h"
#include "classes/CPowerInfo.h"
#define AsCBonusInfo(p) ((CBonusInfo*)(p))
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#define AsCHoleInfo(p) ((CHoleInfo*)(p))
#define AsCPangInfo(p) ((CPangInfo*)(p))
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#define AsCWindInfo(p) ((CWindInfo*)(p))
#include "sdk_protos.h"
#include "game/types.h"
#include "game/wvector.h"
#include "game/golfball.h"
#include "game/singletons.h"
#include "classes/CWind.h"
#include "classes/CQuadTree.h"
#include "classes/CFontNumDraw.h"
#include "classes/CSimpleSpd.h"
#include "classes/CAccuracyInfo.h"
#include "classes/CCalipers.h"
#include "classes/CBalloonInfo.h"
#include "classes/CDriveInfo.h"
#include "classes/CTimeOutInfo.h"

struct CourseScoreBonusPang {
    int oneStrokeBonus, threeUnderParBonus, twoUnderParBonus, oneUnderParBonus, parBonus, fallbackScoreMultiplier;
};
struct PangAnimationFields;
struct ShotPangReward;
struct PangScreenPositionFields;

extern "C" {
extern const float kFloat_20_MessageUI;
extern const float kFloat_255_MessageUI;
extern const float kFloat_4_FlyInfo;
extern const float kCupHeightOneDecimalThreshold;
extern const float kFloat_3_Hud;
extern const float kFloat_33_Hud;
extern const float kCupHeightTextXOffset;
extern const float kFloat_0_64_805DE3E8;
extern const float kFloat_35_Hud;
extern const float kCupHeightBelowTextYOffset;
extern const float kFloat_60_Hud;
extern const float kFloat_12_Hud;
extern const float kFloat_7_Hud;
extern float kFloat_100_hud;
extern const float kFloat_8_96_FlyInfo;
extern const float kFloat_12_8_Hud;
extern const float kFloat_6_4_GolfHud;
extern const float kFloat_64_Hud;
extern const float kFloat_72_Hud;
extern const float kFloat_neg100_Hud;
extern const float kFloat_neg37_GolfHud;
extern const float kFloat_neg153_GolfHud;
extern const float kFloat_neg9_FlyInfo;
extern const double kSignedToDoubleBias_PangBonusRendering;
extern const float kFloat_0_5_hud;
extern const float kFloat_0_hud;
extern const float kFloat_2_805DE318;
extern const float kFloat_32_0;
extern const float kFloat_1_805DE314;
extern const float kFloat_46_Hud;
extern const unsigned int s_dartsScoreOpacityElementIds[2];
extern const unsigned int s_dartsScorePositionElementIds[2];
extern const float kFloat_128_Hud;
extern const float kFloat_256_MessageUI;
extern const float kFloat_25_805DE368;
extern const float kGroundAtlasSandY;
extern const float kFloat_70_Hud;
extern const float kFloat_50_HudLayout;
extern const float kFloat_53_Hud;
extern const float kFloat_40_BonusInfo;
extern const float kFloat_18_805DE3C4;
extern const float kFloat_45_Hud;
extern const float kFloat_400_Hud;
extern const float kFloat_15_Hud;
extern const float kFloat_398_Hud;
extern const float kFloat_30_GolfHud;
extern const float kFloat_28_GolfHud;
extern const float kFloat_neg11_Hud;
extern const float kFlightSecondaryPinFadeStartTime;
extern const float kFloat_510_Hud;
extern const float kShotCountFadeStepSeconds;
extern const float kFloat_34_Hud;
extern const float kFloat_88_GolfHud;
extern const float kFloat_3_2_hud;
extern const float kFloat_8_Hud;
extern const float kFloat_502_GolfHud;
extern const float kFloat_1_1_Hud;
extern const float kFloat_328_Hud;
extern const float kHoleIntroDimmedOpacity;
extern const float kFloat_0_25_HudShared;
extern const float kFloat_445_GolfHud;
extern float kFloat_0_6_805DE33C;
extern const float kFloat_5_Hud;
extern const float kDriveHudLabelHeight;
extern float kFloat_85_GolfHud;
extern float kFloat_36_GolfHud;
extern float kFloat_54_Hud;
extern float kCupHudFixedScreenY;
extern const float kFloat_108_hud;
extern float kFloat_126_Hud;
extern float kFloat_144_Hud;
extern const float kFloat_162_Hud;
extern float kFloat_180_Hud;
extern float kFloat_198_GolfHud;
extern float kGroundAtlasBunkerY;
extern float kFlightRemainingDistanceLabelWidth;
extern const float kFloat_neg25_Hud;
extern const float kFloat_48_Hud;
extern const float kFloat_141_GolfHud;
extern const float kFloat_186_Hud;
extern const float kFloat_51_Hud;
extern const float kFloat_0_0625_Hud;
extern const float kFloat_105_GolfHud;
extern const float kFloat_63_GolfHud;
extern const float kFloat_96_GolfHud;
extern const float kFloat_160_GolfHud;
extern const float kFloat_106_GolfHud;
extern const float kFloat_202_GolfHud;
extern const float kFloat_242_HoleInfo;
extern const float kHudScalar0_8125;
extern const float kHudScalar0_6875;
extern const float lbl_805DE49C;
extern const float lbl_805DE4A0;
extern const float lbl_805DE4A4;
extern const float lbl_805DE4A8;
extern const float lbl_805DE4AC;
extern const float lbl_805DE4B0;
extern const float lbl_805DE4B4;
extern const float lbl_805DE4F4;
extern const float lbl_805DE4F8;
extern const float kDriveHudLabelWidth;
extern const float kFloat_164_Hud;
extern const float kFloat_161_GolfHud;
extern const float kFloat_neg112_Hud;
extern float kFloat_0_5625_Hud;
extern float kFloat_0_625_PangBanner;
extern float kFloat_0_75_Hud;
extern float kFloat_0_875_PangBonus;
extern const double kDouble_2_Hud;
extern float kPangCounterPulseRiseSeconds;
extern const float kFloat_0_1_HudShared;
extern float kFloat_200_Hud;
extern const float kFloat_0_34906584_Hud;
extern const float kFloat_192_6_GolfHud;
extern float kFloat_13_440001_GolfHud;
extern float kCalipersNoMeasurementX;
extern const float kDriveHudRowOriginAndLabelMargin;
extern float kNearestPinLeadDistanceThreshold;
extern float kFloat_65_GolfHud;
extern float kDriveHudHighlightAlphaDelta;
extern float kDriveHudVerticalLayoutScale;
extern float kCalipersPutterLabelYOffset;
extern float kCalipersClubLabelYOffset;
extern float kCalipersMaximumGaugeX;
extern float kCalipersGaugeDistanceSpan;
extern float kCalipersIndicatorYOffset;
extern float kCalipersPointerHitLeft;
extern float kCalipersPointerHitRight;
extern float kCalipersMinimumGaugeX;
void HudFont_DrawAlignedText(void*, const void*, float, float, float, float, int, const void*);
void NamedText_DrawAlignedScaled(void*, const void*, int, float, float, float);
void* WOverlay_Create(void*, const void*, int);
void* GolfPlayer_GetEntryByIndex(void*, int);
void* GolfBall__Fv();
void Vec2_Copy(void*, void*);
void ResourceManager_DeleteResource(void*, void*);
void WOverlay_DrawTransformedRect(void*, void*, const void*, float, const void*, int, unsigned int, int);
char* String_FormatTemporary(const char*, ...);
void* __mi__FRC7WVectorRC7WVector(void*, const void*, const void*);
void* __as__7WVectorFRC7WVector(void*, const void*);
float Magnitude__7WVectorCFv(const void*);
void* PreData__9CGolfBallFi(void*, int);
void Object_DeleteVirtual(void*, void*);
int SharedDoc_IsPlayerFlag0Set(int);
int SharedDoc_GetCourseId();
int ItemInventory_FindBallIndex(void*, unsigned int);
void* ItemInventory_GetBallIdAt(void*, int);
void Sound_StopByAlias(void*, const void*);
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
void Sound_PlayByName(void*, const char*, int, int, const char*, int, float, float, float);
void GolfPlayer_ApplyShotPangReward(void*, unsigned char, ShotPangReward*);
void __ct__9CInfoBaseFv(void*);
void* __nwa__FUl(unsigned int);
void* PartyItemSlots_GetInstance();
unsigned int ItemInventory_GetItemIdAt(void*, int);
void OverlayMapTree_FreeSubtree(void*, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __dt__9CInfoBaseFv(void*);
extern char __vt__9CWindInfo[];
extern char __vt__10CBonusInfo[];
extern char golfHudFontStrings[];
extern CourseScoreBonusPang courseScoreBonusPang[];
extern char s_cameraTransform[];
extern void* g_sharedDoc;
extern void* g_resourceManager;
extern void* g_project;
extern void* g_view;
extern void* g_option;
extern void* g_taskManager;
extern float s_flyInfoFontScale;
extern float s_flyInfoWidthScale;
extern unsigned int s_hudWhiteRgbMask;
void* Object_GetChild(void*);
void* TaskActorTable_FindByName(void*, const char*, void*);
extern void* g_bonusInfoCurrentTask;
extern void* s_golfTaskTypeInfo;
void __dla__FPv(void*);
void Sprite_DrawColoredRect(void*, float*, int, int, float);
int WOverlay_GetHeight(void*);
int WOverlay_GetWidth(void*);
void* ItemInventory_FindDataWithHairStyleFallback(void*, unsigned int);
void* HudFontConfigMap_FindInsertionPosition(void*, unsigned int*, unsigned int*, unsigned char*, unsigned char*);
void* HudFontConfigMap_InsertAndRebalance(void*, void*, unsigned char, unsigned char, unsigned int*);
float CSharedDoc_ComputeAuxPartModifier(void*, int, int);
extern char __vt__14CShotCountInfo[];
extern char __vt__10CPowerInfo[];
extern char __vt__15CRemainTimeInfo[];
extern char cInfoBaseVtable[];
extern char __vt__13CAccuracyInfo[];
extern int s_holeIntroPlayerTextureIds[];
extern char __vt__9CHoleInfo[];
void OverlayPlayer_Destroy(void*, int);
void* __destroy_new_array(void*, void*);
extern "C" void __dt__6CBpTexFv(void*);
void* InstanceTable_Get();
void Register_NormalizedNamedRectangle(void*, const char*, float*, int);
void OverlayPlayer_Init(void*, const char*);
void* __nw__FUl(unsigned int);
int ItemInventory_GetArchiveCourseCount(void*);
extern "C" void __ct__6CBpTexFv(void*);
extern void* g_soundManager;
extern char __vt__9CPangInfo[];
void* memset(void*, int, unsigned int);
extern signed char s_saveDataInitialized;
extern char s_saveData[];
int PartyCourseTable_FindCourseHolePosition(unsigned char, unsigned char);
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void UIRender_SetDefaultState();
void Engine_SetFlexibleVertexFormat(unsigned int);
void* GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, int, void*, int);
float GolfParameter_ResolveFloat(void*, int);
void Normalize__7WVectorFv(void*);
void* CQuadTree_GetInstance();
unsigned char GetGroundType__9CQuadTreeF7WVector(void*, void*);
int CourseData_IsPointOutsidePlayableRegion(void*, void*);
double pow(double, double);
extern int s_roundCompletionBonusByCourse[];
float Math_CosApprox(float);
float Vec3_DotProduct(void*, void*);
unsigned long long Vec2_ScaleByFloat(void*, float);
unsigned long long Vec2_Add(void*, void*);
extern float s_ballBonusBannerDuration;
extern float s_holeInOneBannerDuration;
extern const float kFloat_153_6_Hud;
extern const float kFloat_19_2_PangBanner;
extern char __vt__12CBalloonInfo[];
extern char __vt__9CCalipers[];
extern char __vt__10CDriveInfo[];
extern signed char s_inputManagerInitialized;
void InputManager_Init(void*);
void InputManager_Dtor();
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_GolfHud[];
int InputManager_GetSelectedPlayer(void*);
void InputManager_GetPointerPosition(void*, float*, int);
int Input_IsActionPressed(void*, int, int);
extern int s_driveRewardDelayByStrokeCount[3];
}

// HUD state values are independent of the controller's physical button assignments.
enum HudConstants {
    kGolfHudActive = 0x80,
    kOpaqueAlpha = 255,
    kRgbWhite = 0x00FFFFFF,
    kInvalidPlayer = 0xFF,
    kTextBufferBytes = 0x20,
    kMillisecondsPerSecond = 1000,
    kMaxDisplayedSeconds = 99,
    kBonusEntryCount = 19,
    kBonusUnavailable = 0x100,
    kRewardUnscheduled = 1,
    kBonusStateMask = 0xFFF,
    kBonusAccepted = 0x10,
    kBonusRejected = 0x40,
    kSpecialShotMask = 0x70,
    kTomahawkShot = 0x10,
    kCobraShot = 0x20,
    kSpikeShot = 0x40,
    kPowerSpinMask = 3,
    kPowerCurveMask = 0xC,
    kClubPutter = 3,
    kCalipersConfirmAction = 4,
    kSelectedInputPlayer = -1,
    kShotCountFadeSteps = 60,
    kShotCountAlphaStep = 4,
    kMaxBalloonPang = 9999
};
enum HudTextAlignment {
    kTextAlignLeft,
    kTextAlignCenter,
    kTextAlignRight
};
enum TimeoutFadeState {
    kTimeoutFadeIn,
    kTimeoutFadeOut,
    kTimeoutHold,
    kTimeoutHidden
};
enum HoleIntroState {
    kHoleFadeIn,
    kHoleMoveToCorner,
    kHoleDim,
    kHoleIdle,
    kHoleFlash
};

// Hole-intro layout values, linked from the original data.
#define kHoleIntroCourseInset lbl_805DE49C // 116
#define kHoleIntroStartY lbl_805DE4A0      // 97
#define kHoleIntroPoseZ lbl_805DE4A4       // -14
#define kHoleIntroEndX lbl_805DE4A8        // -6
#define kHoleIntroHoleY lbl_805DE4AC       // 127
#define kHoleIntroHoleWidth lbl_805DE4B0   // 136
#define kHoleIntroPlayerY lbl_805DE4B4     // 159
// The reward-entry sprite sheet is 512 pixels tall: one row is 1/512 of it.
#define kBonusUvRowStep lbl_805DE4F4
#define kBonusUvSecondSheetY lbl_805DE4F8 // 0.9375
enum DriveDistanceAnimationState {
    kDriveWait,
    kDriveDelay,
    kDriveCount,
    kDriveSettle,
    kDriveDone
};

// Array indices in the pang reward display; each entry occupies 0x2C bytes.
enum PangBonusIndex {
    kBonusLongDrive,
    kBonusLongDriveMultiplier,
    kBonusApproach,
    kBonusLongPutt,
    kBonusChipIn,
    kBonusPinContact,
    kBonusMatchEarlyWin,
    kBonusSpecialShot,
    kBonusScore,
    kBonusPowerSpin,
    kBonusPowerCurve,
    kBonusObjectHitMultiplier,
    kBonusHoleInOne,
    kBonusRoundCompletion,
    kBonusPangMultiplier,
    kBonusRecovery,
    kBonusBall,
    kBonusBallBounce,
    kBonusPowerShot
};
// Low twelve bits hold reward-entry phase; 0x1000 independently marks a
// vertical queue shift. Timers use seconds, Y coordinates use HUD pixels.
enum RewardEntryAnimationPhase {
    kRewardPulseRise = 2,
    kRewardPulseFall = 4,
    kRewardReady = 8,
    kRewardTransfer = 0x10,
    kRewardTransferFinished = 0x20,
    kRewardRejectFade = 0x40,
    kRewardHidden = 0x80,
    kRewardEntrySettled = 0x2000,
    kRewardEntryShifting = 0x1000
};
enum PangDisplayFlags {
    kPangRewardsCalculated = 1,
    kPangResetPending = 2,
    kPangEntriesSettled = 4,
    kPangTotalsSettled = 8,
    kPangDisplayComplete = 0x10
};

// These round-state bits belong to CGolfDoc, separate from HUD/entry flags.
// Full producer semantics are unresolved; both frame and draw paths use them.
enum PangRoundAdmission {
    kPangRoundAdmissionMask = 0x81880,
    kPangRoundRestrictedFlag = 0x1000
};

// Fly and wind information.

struct PlayerPangTotalsFields {
    char reserved_00[0x4C];
    int holePang[18];
    char reserved_94[0x20];
    unsigned int shotRewardFlags;
    char reserved_B8[0x14];
    int pangTotal;
    int bonusPangTotal;
};

// Only the club descriptor and power mode are needed by these HUD consumers.
struct HudClubDescriptor {
    char reserved_00[0xA];
    unsigned char clubType;
};
struct HudGolfParameters {
    char reserved_00[4];
    HudClubDescriptor* club;
    char reserved_08[0x3C];
    int powerMode;
    unsigned char chipInBonusSelector; // +0x48: value four enables the fourfold non-special chip-in branch.
};
typedef char HudClubDescriptor_type_offset[(IFF_OFFSET(HudClubDescriptor, clubType) == 0xA) ? 1 : -1];
typedef char HudGolfParameters_mode_offset[(IFF_OFFSET(HudGolfParameters, powerMode) == 0x44) ? 1 : -1];

// Clamp one flight-HUD value to the supplied display range.
static inline float FlyClamp(float value, float low, float high) {
    return value < low ? low : value > high ? high : value;
}

// Party variants reuse the snapshot's trailing result storage with different scalar formats.
struct HudFlightResultFields {
    char reserved_00[0x170];
    float carryDistance;
    int dartsScore;
    union {
        float distances[19];
        int dartsTargetScore;
    } result;
};
struct HudPartyItemSlots {
    char reserved_00[0x10];
    PartyItemModifiers effects;
};
struct HudProjectEventFields {
    char reserved_00[0x28];
    unsigned char eventMarkEnabled;
};
struct HudImpactFields {
    char reserved_00[0x2D8];
    unsigned char superPangya;
};
struct HudHoleNumberFields {
    char reserved_00[0x3B7];
    unsigned char displayedHoleNumber;
};
struct HudCourseInventoryFields {
    char reserved_00[0xA90];
    void* courseEntries;
};
struct HudPartyCourseOrderFields {
    char reserved_00[0x83D];
    unsigned char courseIds[18];
};

// The first round-result word is the signed score relative to par used by the versus HUD.
static inline int HudPlayer_ScoreToPar(const void* player) {
    return (int)((const GolfDocSnapshot8013*)player)->roundResultWords[0];
}

#define HUD_FIELD(type, field, offset) \
    typedef char type##_##field##_offset[(IFF_OFFSET(type, field) == offset) ? 1 : -1]
HUD_FIELD(HudFlightResultFields, carryDistance, 0x170);
HUD_FIELD(HudFlightResultFields, dartsScore, 0x174);
HUD_FIELD(HudFlightResultFields, result, 0x178);
HUD_FIELD(HudPartyItemSlots, effects, 0x10);
HUD_FIELD(HudProjectEventFields, eventMarkEnabled, 0x28);
HUD_FIELD(HudImpactFields, superPangya, 0x2D8);
HUD_FIELD(HudHoleNumberFields, displayedHoleNumber, 0x3B7);
HUD_FIELD(HudCourseInventoryFields, courseEntries, 0xA90);
HUD_FIELD(HudPartyCourseOrderFields, courseIds, 0x83D);
#undef HUD_FIELD

struct WindTextFields {
    char pad0[0xC];
    int color;
    char pad10[4];
    void* text;
};
struct WindInventoryDocFields {
    char pad0[0xA04];
    char subA04[0xB90 - 0xA04];
    char* playerB90;
};

struct D0 {
    inline ~D0();
};
struct D1 : D0 {
    inline
        // Forward destruction to the bonus-icon map tree cleanup in D0.
        ~D1() {}
};
struct D2 : D1 {
    inline
        // Forward destruction through the next bonus-icon map cleanup layer.
        ~D2() {}
};
struct D3 : D2 {
    inline
        // Forward destruction through the next bonus-icon map cleanup layer.
        ~D3() {}
};
struct D4 : D3 {
    inline
        // Forward destruction through the outer bonus-icon map cleanup layer.
        ~D4() {}
};

struct Leaf {
    void* p0;
    void* p1;
};

template <int N> struct FreeTreeNode {
    FreeTreeNode<N - 1>* l;
    FreeTreeNode<N - 1>* r;
};

// Release both remaining subtrees through the common tree helper, then free this node.
static inline void FreeTree(D0* self, FreeTreeNode<0>* n) {
    if (n->l != 0)
        OverlayMapTree_FreeSubtree(self, n->l);
    if (n->r != 0)
        OverlayMapTree_FreeSubtree(self, n->r);
    appFreeUni__FPv(n);
}

template <int N>
// Expand the next tree-cleanup level in the original left/right/node order.
static inline void FreeTree(D0* self, FreeTreeNode<N>* n) {
    if (n->l != 0)
        FreeTree(self, n->l);
    if (n->r != 0)
        FreeTree(self, n->r);
    appFreeUni__FPv(n);
}

// Free the bonus-icon tree rooted in this map's header.
inline D0::~D0() {
    FreeTreeNode<3>* n = *(FreeTreeNode<3>**)((char*)this + 4);
    if (n != 0) {
        if (n->l != 0)
            FreeTree(this, n->l);
        if (n->r != 0)
            FreeTree(this, n->r);
        appFreeUni__FPv(n);
    }
}

extern "C" D4* dtor_800F66EC(D4* self, int flags);

#pragma opt_lifetimes off
#pragma use_lmw_stmw on
// Flight drawing borrows HUD, layout and player storage. Coordinates are screen pixels;
// distance values are yards and opacity values span 0..255. No helper transfers ownership.
// Scratch references keep the original caller declaration order for the compiler's
// register allocator. These temporary outputs are not additional persistent HUD state.
enum FlightHudElementId {
    kDartsDistancePosition = 14,
    kModeTwoDistancePosition = 17,
    kModeTwoCarryValuePosition = 18,
    kModeTwoCarryUnitPosition = 24,
    kModeTwoCurrentShotValuePosition = 19,
    kModeTwoCurrentShotUnitPosition = 25,
    kDartsPinLabelElement = 13,
    kModeTwoPinLabelElement = 3,
    kModeTwoCarryValueElement = 4,
    kModeTwoTotalDistancePosition = 23
};
enum FlightHudDisplayMode {
    kFlightDartsMode = 1,
    kFlightModeTwo = 2
};
enum FlightHudColor {
    kRemainingDistanceRgb = 0x00FF3434,
    kOpacityByteMask = 0xFF
};

// Draw horizontal distance from the shot origin, with integer, decimal point and hundredths aligned separately.
static inline void DrawFlightTravelDistance(CFlyInfo*& textStyle, CFlyInfo*& flyInfoBytes, void*& hudObject,
                                            float& travelTextX, float& travelTextY) {
    float distanceYards = flyInfoBytes->distanceFromStart;
    int wholeYards = (int)distanceYards;
    textStyle->color = -1;
    textStyle->textMode = kTextAlignRight;
    float fractionalYards = distanceYards - (float)wholeYards;
    int hundredthsYards = (int)(kFloat_100_hud * fractionalYards);
    sprintf((char*)((char*)flyInfoBytes->textBuffer),
            (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)), wholeYards);
    HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, travelTextX - kFloat_8_96_FlyInfo, travelTextY,
                            kFloat_4_FlyInfo, kFloat_0_64_805DE3E8, 0, 0);
    textStyle->textMode = kTextAlignCenter;
    HudFont_DrawAlignedText(hudObject, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, decimalPoint)), travelTextX,
                            travelTextY, kFloat_4_FlyInfo, kFloat_0_64_805DE3E8, 0, 0);
    textStyle->textMode = kTextAlignLeft;
    sprintf((char*)((char*)flyInfoBytes->textBuffer),
            (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, fractionalYardsFormat)), hundredthsYards);
    HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, kFloat_8_96_FlyInfo + travelTextX, travelTextY,
                            kFloat_4_FlyInfo, kFloat_0_64_805DE3E8, 0, 0);
}

// Draw Driving Contest carry and current-shot values with the two SPD anchors.
// Required initialized HUD/style and a writable two-float sprite-position scratch;
// scaledGlyphWidth/baselineCorrection are pixels. All other scratch stays local.
// Mutates text style, reusable textBuffer and spritePositionPx, and emits draws.
// Borrowed round data is reloaded before each result lookup; no result is cached
// across draws and no reference/ownership is returned to the caller.
static inline void DrawFlightCarrySummary(CFlyInfo* const& textStyle, CFlyInfo* const& flyInfoBytes,
                                          void* const& hudObject, const float& scaledGlyphWidth,
                                          const float& fontBaselineCorrection, float* spritePositionPx) {
    unsigned int carryAlpha;
    void* carryLayout;
    float carryValueX, carryValueY, carryValueScale;
    void* sharedDocument;
    void* golfDocument;
    void* playerResult;
    float carryDistance, carryUnitX, carryUnitY, carryUnitScale;
    float currentShotX, currentShotY, currentShotScale;
    int strokeIndex;
    float currentShotInsetPx;
    unsigned int currentShotAlpha;
    float currentShotDistance;
    carryAlpha = Runtime_ConvertDoubleToUnsigned(flyInfoBytes->carryValueOpacity);
    textStyle->color = (carryAlpha << 24) | s_hudWhiteRgbMask;
    carryLayout = flyInfoBytes->modeTwoResultLayout;
    ((CSimpleSpd*)carryLayout)->GetSpritePosition2D(kModeTwoCarryValuePosition, spritePositionPx);

    carryValueY = spritePositionPx[1] - fontBaselineCorrection;
    carryValueX = spritePositionPx[0] + scaledGlyphWidth;
    carryValueScale = s_flyInfoFontScale;
    sharedDocument = (void*)g_sharedDoc;
    golfDocument = ((void*)AsCSharedDoc(sharedDocument)->golfDoc);
    playerResult = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
    carryDistance = ((HudFlightResultFields*)playerResult)->carryDistance;
    textStyle->textMode = kTextAlignRight;
    sprintf((char*)((char*)flyInfoBytes->textBuffer),
            (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, threeColumnIntegerFormat)), (int)carryDistance);
    HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, carryValueX, carryValueY, kFloat_4_FlyInfo,
                            carryValueScale, 0, 0);

    {

        ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))
            ->GetSpritePosition2D(kModeTwoCarryUnitPosition, spritePositionPx);
        carryUnitY = spritePositionPx[1] - fontBaselineCorrection;
        carryUnitX = spritePositionPx[0] + scaledGlyphWidth - kFloat_12_8_Hud;
        carryUnitScale = s_flyInfoFontScale;
        textStyle->textMode = kTextAlignLeft;
        sprintf((char*)((char*)flyInfoBytes->textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, p)));
        HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, carryUnitX, carryUnitY, kFloat_4_FlyInfo,
                                carryUnitScale, 0, flyInfoBytes->labelFont);
    }
    {

        golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        playerResult = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
        strokeIndex = ((GolfDocSnapshot8013*)playerResult)
                          ->holeStrokeCounts[AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex];
        currentShotInsetPx = kFloat_6_4_GolfHud;
        currentShotAlpha = Runtime_ConvertDoubleToUnsigned(flyInfoBytes->currentShotValueOpacity);
        textStyle->color = (currentShotAlpha << 24) | s_hudWhiteRgbMask;
        {
            ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))
                ->GetSpritePosition2D(kModeTwoCurrentShotValuePosition, spritePositionPx);
            currentShotY = spritePositionPx[1] - fontBaselineCorrection;
            currentShotX = currentShotInsetPx + (spritePositionPx[0] + scaledGlyphWidth);
            currentShotScale = s_flyInfoFontScale;
            golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
            playerResult = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
            currentShotDistance = ((HudFlightResultFields*)playerResult)->result.distances[strokeIndex];
            textStyle->textMode = kTextAlignRight;
            sprintf((char*)((char*)flyInfoBytes->textBuffer),
                    (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, distanceFormat)), currentShotDistance);
            HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, currentShotX, currentShotY, kFloat_4_FlyInfo,
                                    currentShotScale, 0, 0);
        }
        {
            ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))
                ->GetSpritePosition2D(kModeTwoCurrentShotUnitPosition, spritePositionPx);
            currentShotY = spritePositionPx[1] - fontBaselineCorrection;
            currentShotX = currentShotInsetPx + ((spritePositionPx[0] + scaledGlyphWidth) - kFloat_12_8_Hud);
            currentShotScale = s_flyInfoFontScale;
            textStyle->textMode = kTextAlignLeft;
            sprintf((char*)((char*)flyInfoBytes->textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, p)));
            HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, currentShotX, currentShotY, kFloat_4_FlyInfo,
                                    currentShotScale, 0, flyInfoBytes + 0x30);
        }
    }
}

// Draw ground/pin labels and update result-layout opacity after the ball-outcome gate.
static inline void DrawFlightGroundAndPinLabels(CFlyInfo*& textStyle, CFlyInfo*& flyInfoBytes, char*& hudLookupTables,
                                                void*& groundText, unsigned int& groundAlpha, float& viewportHalfFactor,
                                                float& groundInsetX, float& groundInsetY, float& groundTextY,
                                                float& groundTextX, float& pinLabelOffsetX, float& pinLabelOffsetY,
                                                float& viewportCenterX, float& viewportCenterY, int& dartsPinAlpha,
                                                int& modeTwoPinAlpha, void*& pinText, unsigned int& pinAlpha,
                                                int& carryLayerAlpha, int*& carryLabelElementIds,
                                                unsigned int& carryLabelIndex) {
    groundText = textStyle->groundText;
    if (groundText) {
        groundAlpha = Runtime_ConvertDoubleToUnsigned(flyInfoBytes->groundTextOpacity);
        viewportHalfFactor = kFloat_0_5_hud;
        groundInsetX = kFloat_64_Hud;
        groundInsetY = kFloat_72_Hud;
        ((CFlyInfo*)groundText)->color = (groundAlpha << 24) | 0x00ffffff;
        groundTextY = viewportHalfFactor * AsWView(g_view)->viewportHeight - groundInsetY;
        groundTextX = viewportHalfFactor * AsWView(g_view)->viewportWidth - groundInsetX;
        NamedText_DrawAlignedScaled(flyInfoBytes->groundText, flyInfoBytes->groundName, 0, groundTextX, groundTextY,
                                    kFloat_1_805DE314);
    }

    viewportCenterX = kFloat_0_5_hud * AsWView(g_view)->viewportWidth;
    viewportCenterY = kFloat_0_5_hud * AsWView(g_view)->viewportHeight;
    if (flyInfoBytes->dartsResultLayout) {
        dartsPinAlpha = (int)flyInfoBytes->pinTextOpacity;
        dartsPinAlpha &= kOpacityByteMask;
        ((CSimpleSpd*)(flyInfoBytes->dartsResultLayout))->SetOpacity(dartsPinAlpha);
        ((CSimpleSpd*)(flyInfoBytes->dartsResultLayout))->ApplyElementOpacity(kDartsPinLabelElement);
    } else if (flyInfoBytes->modeTwoResultLayout) {
        modeTwoPinAlpha = (int)flyInfoBytes->pinTextOpacity;
        modeTwoPinAlpha &= kOpacityByteMask;
        ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))->SetOpacity(modeTwoPinAlpha);
        ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))->ApplyElementOpacity(kModeTwoPinLabelElement);
    } else if (pinText = textStyle->pinText) {
        pinLabelOffsetX = kFloat_neg100_Hud;
        pinLabelOffsetY = kFloat_neg37_GolfHud;
        pinAlpha = Runtime_ConvertDoubleToUnsigned(flyInfoBytes->pinTextOpacity);
        ((CFlyInfo*)pinText)->color = (pinAlpha << 24) | 0x00ffffff;
        NamedText_DrawAlignedScaled(
            flyInfoBytes->pinText, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, localizedText)), 0,
            viewportCenterX + pinLabelOffsetX, viewportCenterY + pinLabelOffsetY, kFloat_1_805DE314);
    }
    if (!flyInfoBytes->modeTwoResultLayout && !flyInfoBytes->dartsResultLayout) {
        pinLabelOffsetX = kFloat_neg153_GolfHud;
        pinLabelOffsetY = kFloat_neg9_FlyInfo;
        *(unsigned int*)((char*)flyInfoBytes->pinText + 0xc) =
            (Runtime_ConvertDoubleToUnsigned(flyInfoBytes->secondaryPinTextOpacity) << 24) | 0xffffff;
        NamedText_DrawAlignedScaled(
            flyInfoBytes->pinText, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, localizedText_2)), 0,
            viewportCenterX + pinLabelOffsetX, viewportCenterY + pinLabelOffsetY, kFloat_1_805DE314);
    }
    if (flyInfoBytes->modeTwoResultLayout) {
        carryLayerAlpha = (int)flyInfoBytes->carryValueOpacity;
        carryLayerAlpha &= kOpacityByteMask;
        ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))->SetOpacity(carryLayerAlpha);
        ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))->ApplyElementOpacity(kModeTwoCarryValueElement);
        carryLayerAlpha = (int)flyInfoBytes->carryLabelsOpacity;
        carryLayerAlpha &= kOpacityByteMask;
        ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))->SetOpacity(carryLayerAlpha);
        carryLabelElementIds = (int*)(hudLookupTables + 0x188);
        for (carryLabelIndex = 0; carryLabelIndex < 3; carryLabelIndex++, carryLabelElementIds++)
            ((CSimpleSpd*)(flyInfoBytes->modeTwoResultLayout))->ApplyElementOpacity(*carryLabelElementIds);
    }
}

// Draw per-stroke mode-two distances followed by the total, each with its staggered opacity.
static inline void DrawFlightShotDetails(
    CFlyInfo*& textStyle, CFlyInfo*& flyInfoBytes, void*& hudObject, char*& hudLookupTables, float& scaledGlyphWidth,
    float& fontBaselineCorrection, float* spritePositionPx, int& strokeCount, int& strokeIndex, void*& golfDocument,
    void*& playerResult, char*& opacityCursorBytes, int*& strokeOpacityElementIds, const char*& strokeDistanceFormat,
    int*& strokePositionElementIds, int& distanceOffsetBytes, int& strokeLayerAlpha, unsigned int& strokeTextAlpha,
    float& strokeTextX, float& strokeTextY, float& strokeTextScale, float& strokeDistance, int& opacityOffsetBytes,
    int& totalLayerAlpha, int*& totalOpacityElementIds, unsigned int& totalLabelIndex, float& totalTextOpacity,
    unsigned int& totalTextAlpha, float& totalTextX, float& totalTextY, float& totalTextScale, float& totalDistance) {
    if (flyInfoBytes->modeTwoShotDetailLayout) {

        textStyle->textMode = kTextAlignRight;
        golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        playerResult = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
        strokeIndex = 0;
        strokeCount =
            *(unsigned char*)((char*)playerResult +
                              AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex + 0x28);
        if (strokeCount > 0) {
            opacityCursorBytes = (char*)flyInfoBytes;
            strokeOpacityElementIds = (int*)(hudLookupTables + 0x194);
            strokeDistanceFormat = (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, text_5_2f));
            strokePositionElementIds = (int*)(hudLookupTables + 0x1a0);
            distanceOffsetBytes = 0;
            for (; strokeIndex < strokeCount; strokeIndex++) {
                strokeLayerAlpha = (int)*(float*)(opacityCursorBytes + 0x50);
                strokeLayerAlpha &= kOpacityByteMask;
                ((CSimpleSpd*)(flyInfoBytes->modeTwoShotDetailLayout))->SetOpacity(strokeLayerAlpha);
                ((CSimpleSpd*)(flyInfoBytes->modeTwoShotDetailLayout))->ApplyElementOpacity(*strokeOpacityElementIds);
                strokeTextAlpha = Runtime_ConvertDoubleToUnsigned(*(float*)(opacityCursorBytes + 0x50));
                textStyle->color = s_hudWhiteRgbMask | (strokeTextAlpha << 24);
                ((CSimpleSpd*)(flyInfoBytes->modeTwoShotDetailLayout))
                    ->GetSpritePosition2D(*strokePositionElementIds, spritePositionPx);

                strokeTextY = spritePositionPx[1] - fontBaselineCorrection;
                strokeTextX = spritePositionPx[0] + scaledGlyphWidth;
                strokeTextScale = s_flyInfoFontScale;
                golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
                playerResult = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
                // Keep the distance byte cursor: changing its stride expression changes register allocation.
                strokeDistance = *(float*)((char*)&((HudFlightResultFields*)playerResult)->result.distances[1] +
                                           distanceOffsetBytes);
                textStyle->textMode = kTextAlignRight;
                sprintf((char*)((char*)flyInfoBytes->textBuffer), strokeDistanceFormat, strokeDistance);
                HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, strokeTextX, strokeTextY, kFloat_4_FlyInfo,
                                        strokeTextScale, 0, 0);
                opacityCursorBytes += 4;
                ++strokeOpacityElementIds;
                ++strokePositionElementIds;
                distanceOffsetBytes += 4;
            }
        }
        opacityOffsetBytes = strokeCount * 4;
        totalLayerAlpha = (int)*(float*)((char*)flyInfoBytes + 0x50 + opacityOffsetBytes);
        totalLayerAlpha &= kOpacityByteMask;
        ((CSimpleSpd*)(flyInfoBytes->modeTwoShotDetailLayout))->SetOpacity(totalLayerAlpha);
        totalOpacityElementIds = (int*)(hudLookupTables + 0x1ac);
        for (totalLabelIndex = 0; totalLabelIndex < 3; ++totalLabelIndex, ++totalOpacityElementIds)
            ((CSimpleSpd*)(flyInfoBytes->modeTwoShotDetailLayout))->ApplyElementOpacity(*totalOpacityElementIds);
        totalTextOpacity = *(float*)((char*)flyInfoBytes + 0x54 + opacityOffsetBytes);
        totalTextAlpha = Runtime_ConvertDoubleToUnsigned(totalTextOpacity);
        textStyle->color = s_hudWhiteRgbMask | (totalTextAlpha << 24);
        ((CSimpleSpd*)(flyInfoBytes->modeTwoShotDetailLayout))
            ->GetSpritePosition2D(kModeTwoTotalDistancePosition, spritePositionPx);

        totalTextY = spritePositionPx[1] - fontBaselineCorrection;
        totalTextX = spritePositionPx[0] + scaledGlyphWidth;
        totalTextScale = s_flyInfoFontScale;
        golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        playerResult = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
        totalDistance = ((HudFlightResultFields*)playerResult)->result.distances[0];
        textStyle->textMode = kTextAlignRight;
        sprintf((char*)((char*)flyInfoBytes->textBuffer),
                (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, distanceFormat)), totalDistance);
        HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, totalTextX, totalTextY, kFloat_4_FlyInfo,
                                totalTextScale, 0, 0);
    }
}

// Orchestrate flight-HUD geometry, distance values, result labels and mode-specific details.
void CFlyInfo::DrawFlightHud() {
    // Phases: gate on active HUD, interpolate distance positions, draw distance
    // and pin labels, apply per-layer opacity, then draw carry/ground result details.
    void* hudObject = (void*)this;
    CFlyInfo* textStyle = this;
    CFlyInfo* flyInfoBytes = this;
    char* hudLookupTables = (char*)courseScoreBonusPang;
    float hudBaselineY, travelTextX, travelTextY, glyphWidthPx, glyphGapPx, scaledGlyphWidth, fontBaselineCorrection;
    void* sharedDocument = (void*)g_sharedDoc;
    void* golfDocument = ((void*)AsCSharedDoc(sharedDocument)->golfDoc);
    if (((unsigned int)AsCGolfDoc(golfDocument)->gameStateFlags & kGolfHudActive) == 0)
        return;
    void* ball = GolfBall__Fv();
    glyphWidthPx = kFloat_20_MessageUI;
    glyphGapPx = kFloat_4_FlyInfo;
    scaledGlyphWidth = s_flyInfoFontScale * (s_flyInfoWidthScale * glyphWidthPx);
    hudBaselineY = AsCOption(g_option)->hudBaselineY;
    float distanceTargetInsetPx =
        kFloat_0_64_805DE3E8 * (kCupHeightOneDecimalThreshold + kFloat_3_Hud * (glyphWidthPx + glyphGapPx));
    float spritePositionPx[2], distanceStartPx[2], distanceTargetPx[2];
    char dartsTotalText[8];
    float travelStartPx[2], travelFallbackTargetPx[2], pinStartPx[2], pinTargetPx[2];
    travelStartPx[0] = kFloat_0_5_hud * AsWView(g_view)->viewportWidth - glyphWidthPx;
    travelStartPx[1] = hudBaselineY - kFloat_33_Hud;
    fontBaselineCorrection = kCupHeightTextXOffset * (s_flyInfoFontScale - kFloat_0_64_805DE3E8);
    Vec2_Copy(distanceStartPx, travelStartPx);
    void* distanceResultLayout = flyInfoBytes->dartsResultLayout;
    if (distanceResultLayout) {
        ((CSimpleSpd*)distanceResultLayout)->GetSpritePosition2D(kDartsDistancePosition, spritePositionPx);
        float targetX = spritePositionPx[0] + scaledGlyphWidth;
        distanceTargetPx[1] = spritePositionPx[1];
        distanceTargetPx[0] = targetX - distanceTargetInsetPx;
    } else if ((distanceResultLayout = flyInfoBytes->modeTwoResultLayout) != 0) {
        ((CSimpleSpd*)distanceResultLayout)->GetSpritePosition2D(kModeTwoDistancePosition, spritePositionPx);
        float targetX = spritePositionPx[0] + scaledGlyphWidth;
        distanceTargetPx[1] = spritePositionPx[1];
        distanceTargetPx[0] = targetX - distanceTargetInsetPx;
    } else {
        float targetY = kFloat_0_5_hud * AsWView(g_view)->viewportHeight - kCupHeightBelowTextYOffset;
        float targetX = kFloat_0_5_hud * AsWView(g_view)->viewportWidth + kFloat_35_Hud;
        travelFallbackTargetPx[1] = targetY;
        travelFallbackTargetPx[0] = targetX;
        Vec2_Copy(distanceTargetPx, travelFallbackTargetPx);
    }
    float rawDistanceBlend = kFloat_2_805DE318 * (flyInfoBytes->elapsedTime - kFloat_0_5_hud);
    float distanceBlend = rawDistanceBlend < kFloat_0_hud        ? kFloat_0_hud
                          : rawDistanceBlend > kFloat_1_805DE314 ? kFloat_1_805DE314
                                                                 : rawDistanceBlend;
    float transitionDX = distanceBlend * (distanceTargetPx[0] - distanceStartPx[0]);
    float transitionDY = distanceBlend * (kFloat_60_Hud + (distanceTargetPx[1] - distanceStartPx[1]));
    travelTextX = distanceStartPx[0] + transitionDX;
    travelTextY = (distanceStartPx[1] + transitionDY) - kFloat_60_Hud;
    if (((int)AsCSharedDoc(g_sharedDoc)->partyGameMode) != kFlightModeTwo ||
        flyInfoBytes->elapsedTime < kFloat_4_FlyInfo) {
        DrawFlightTravelDistance(textStyle, flyInfoBytes, hudObject, travelTextX, travelTextY);
    }
    if (flyInfoBytes->modeTwoResultLayout == 0) {
        unsigned int remainingDistanceAlpha = 255;
        pinStartPx[0] = kFloat_0_5_hud * AsWView(g_view)->viewportWidth - kFloat_20_MessageUI;
        pinStartPx[1] = hudBaselineY - kFloat_12_Hud;
        Vec2_Copy(distanceStartPx, pinStartPx);
        float pinTargetY = kFloat_0_5_hud * AsWView(g_view)->viewportHeight - kFloat_7_Hud;
        float pinTargetX = kFloat_0_5_hud * AsWView(g_view)->viewportWidth + kFloat_35_Hud;
        pinTargetPx[1] = pinTargetY;
        pinTargetPx[0] = pinTargetX;
        Vec2_Copy(distanceTargetPx, pinTargetPx);
        if (flyInfoBytes->dartsResultLayout) {
            remainingDistanceAlpha =
                Runtime_ConvertDoubleToUnsigned(kFloat_255_MessageUI * (kFloat_1_805DE314 - distanceBlend));
        }
        float rawPinBlend = kFloat_2_805DE318 * (flyInfoBytes->elapsedTime - kFloat_1_805DE314);
        float pinBlend = FlyClamp(rawPinBlend, kFloat_0_hud, kFloat_1_805DE314);
        textStyle->color = (remainingDistanceAlpha << 24) | kRemainingDistanceRgb;
        float transitionDX = pinBlend * (distanceTargetPx[0] - distanceStartPx[0]);
        float transitionDY = pinBlend * (kFloat_60_Hud + (distanceTargetPx[1] - distanceStartPx[1]));
        float numberTextX = distanceStartPx[0] + transitionDX;
        float numberTextY = (distanceStartPx[1] + transitionDY) - kFloat_60_Hud;
        int hideRemainingHundredths = ((CGolfBall*)ball)->m_state == 0;
        if (hideRemainingHundredths) {
            int holedStateBits =
                ~(*(int*)&((CGolfBall*)ball)->m_holeInCrypt[0] ^ *(int*)&((CGolfBall*)ball)->m_holeInCrypt[8]);
            hideRemainingHundredths = holedStateBits == 1;
        }
        float distanceYards = flyInfoBytes->distanceToPin;
        int wholeYards = (int)distanceYards;
        textStyle->textMode = kTextAlignRight;
        float fractionalYards = distanceYards - (float)wholeYards;
        int hundredthsYards = (int)(kFloat_100_hud * fractionalYards);
        sprintf((char*)((char*)flyInfoBytes->textBuffer),
                (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)), wholeYards);
        HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, numberTextX - kFloat_8_96_FlyInfo, numberTextY,
                                kFloat_4_FlyInfo, kFloat_0_64_805DE3E8, 0, 0);
        textStyle->textMode = kTextAlignCenter;
        HudFont_DrawAlignedText(hudObject, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, decimalPoint)),
                                numberTextX, numberTextY, kFloat_4_FlyInfo, kFloat_0_64_805DE3E8, 0, 0);
        textStyle->textMode = kTextAlignLeft;
        if (hideRemainingHundredths)
            hundredthsYards = 0;
        sprintf((char*)((char*)flyInfoBytes->textBuffer),
                (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, fractionalYardsFormat)), hundredthsYards);
        HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, kFloat_8_96_FlyInfo + numberTextX, numberTextY,
                                kFloat_4_FlyInfo, kFloat_0_64_805DE3E8, 0, 0);
    } else {
        {
            DrawFlightCarrySummary(textStyle, flyInfoBytes, hudObject, scaledGlyphWidth, fontBaselineCorrection,
                                   spritePositionPx);
        }
    }
    if (((CGolfBall*)ball)->m_state || ((CGolfBall*)ball)->m_recoveryRequired || ((CGolfBall*)ball)->m_lie)
        return;
    int encodedOutcome = ~(*(int*)&((CGolfBall*)ball)->m_holeInCrypt[0] ^ *(int*)&((CGolfBall*)ball)->m_holeInCrypt[8]);
    if (encodedOutcome == 1) {
        int partyMode = ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode);
        if (partyMode != kFlightModeTwo && partyMode != kFlightDartsMode)
            return;
    }
    {
        void* groundText;
        unsigned int groundAlpha;
        float viewportHalfFactor;
        float groundInsetX;
        float groundInsetY;
        float groundTextY;
        float groundTextX;
        float pinLabelOffsetX;
        float pinLabelOffsetY;
        float viewportCenterX;
        float viewportCenterY;
        int dartsPinAlpha;
        int modeTwoPinAlpha;
        void* pinText;
        unsigned int pinAlpha;
        int carryLayerAlpha;
        int* carryLabelElementIds;
        unsigned int carryLabelIndex;
        DrawFlightGroundAndPinLabels(
            textStyle, flyInfoBytes, hudLookupTables, groundText, groundAlpha, viewportHalfFactor, groundInsetX,
            groundInsetY, groundTextY, groundTextX, pinLabelOffsetX, pinLabelOffsetY, viewportCenterX, viewportCenterY,
            dartsPinAlpha, modeTwoPinAlpha, pinText, pinAlpha, carryLayerAlpha, carryLabelElementIds, carryLabelIndex);
    }
    {
        int strokeCount;
        int strokeIndex;
        void* golfDocument;
        void* playerResult;
        char* opacityCursorBytes;
        int* strokeOpacityElementIds;
        const char* strokeDistanceFormat;
        int* strokePositionElementIds;
        int distanceOffsetBytes;
        int strokeLayerAlpha;
        unsigned int strokeTextAlpha;
        float strokeTextX;
        float strokeTextY;
        float strokeTextScale;
        float strokeDistance;
        int opacityOffsetBytes;
        int totalLayerAlpha;
        int* totalOpacityElementIds;
        unsigned int totalLabelIndex;
        float totalTextOpacity;
        unsigned int totalTextAlpha;
        float totalTextX;
        float totalTextY;
        float totalTextScale;
        float totalDistance;
        DrawFlightShotDetails(textStyle, flyInfoBytes, hudObject, hudLookupTables, scaledGlyphWidth,
                              fontBaselineCorrection, spritePositionPx, strokeCount, strokeIndex, golfDocument,
                              playerResult, opacityCursorBytes, strokeOpacityElementIds, strokeDistanceFormat,
                              strokePositionElementIds, distanceOffsetBytes, strokeLayerAlpha, strokeTextAlpha,
                              strokeTextX, strokeTextY, strokeTextScale, strokeDistance, opacityOffsetBytes,
                              totalLayerAlpha, totalOpacityElementIds, totalLabelIndex, totalTextOpacity,
                              totalTextAlpha, totalTextX, totalTextY, totalTextScale, totalDistance);
    }
    if (flyInfoBytes->dartsResultLayout) {
        textStyle->textMode = kTextAlignRight;
        void* taskRoot = g_taskManager ? Object_GetChild(g_taskManager) : 0;
        int remainingDartsResult;
        char dartsBreakdownText[3][8];
        volatile int queryWords[8];
        int holeIndex = AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentHoleIndex;
        queryWords[0] = 0;
        queryWords[1] = 0;
        queryWords[2] = (int)&remainingDartsResult;
        queryWords[3] = holeIndex;
        queryWords[4] = 0;
        queryWords[5] = 0;
        queryWords[6] = 0;
        queryWords[7] = 0;
        void* dartsSystem =
            TaskActorTable_FindByName(taskRoot, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, DartsSystem)), 0);
        if (dartsSystem) {
            void** dartsSystemVtable = *(void***)dartsSystem;
            ((void (*)(void*, void*))dartsSystemVtable[11])(dartsSystem, (void*)queryWords);
        }
        void* golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        void* playerResult = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
        sprintf((char*)(dartsTotalText), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, threeColumnIntegerFormat)),
                ((HudFlightResultFields*)playerResult)->dartsScore);
        golfDocument = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        playerResult = GolfPlayer_GetEntryByIndex(golfDocument, AsCGolfDoc(golfDocument)->currentPlayer);
        sprintf((char*)(dartsBreakdownText[0]),
                (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, threeColumnIntegerFormat)),
                ((HudFlightResultFields*)playerResult)->result.dartsTargetScore);
        sprintf((char*)(dartsBreakdownText[1]), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, slash)));
        sprintf((char*)(dartsBreakdownText[2]),
                (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, threeColumnIntegerFormat)), remainingDartsResult);
        Arr3 componentSpacing;
        componentSpacing = *(Arr3*)(hudLookupTables + 0x3cc);
        char* opacityCursorBytes = (char*)flyInfoBytes;
        for (int scoreRow = 0; scoreRow < 2; ++scoreRow) {
            int layerAlpha = (int)*(float*)(opacityCursorBytes + 0x64);
            layerAlpha &= kOpacityByteMask;
            ((CSimpleSpd*)(flyInfoBytes->dartsResultLayout))->SetOpacity(layerAlpha);
            ((CSimpleSpd*)(flyInfoBytes->dartsResultLayout))
                ->ApplyElementOpacity(s_dartsScoreOpacityElementIds[scoreRow]);
            unsigned int textAlpha = Runtime_ConvertDoubleToUnsigned(*(float*)(opacityCursorBytes + 0x64));
            textStyle->color = s_hudWhiteRgbMask | (textAlpha << 24);
            ((CSimpleSpd*)(flyInfoBytes->dartsResultLayout))
                ->GetSpritePosition2D(s_dartsScorePositionElementIds[scoreRow], spritePositionPx);
            if (scoreRow == 0) {
                sprintf((char*)((char*)flyInfoBytes->textBuffer), dartsTotalText);
                HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, spritePositionPx[0] + scaledGlyphWidth,
                                        spritePositionPx[1] - fontBaselineCorrection, kFloat_4_FlyInfo,
                                        s_flyInfoFontScale, 0, 0);
            } else {
                for (int componentIndex = 0; componentIndex < 3; ++componentIndex) {
                    sprintf((char*)((char*)flyInfoBytes->textBuffer), dartsBreakdownText[componentIndex]);
                    float componentOffsetPx =
                        componentSpacing.v[componentIndex] * (glyphWidthPx + glyphGapPx) + glyphGapPx;
                    float fontScale = s_flyInfoFontScale;
                    float numberTextX = spritePositionPx[0] + scaledGlyphWidth;
                    float componentInsetPx = fontScale * componentOffsetPx;
                    HudFont_DrawAlignedText(hudObject, flyInfoBytes->textBuffer, componentInsetPx + numberTextX,
                                            spritePositionPx[1] - fontBaselineCorrection, kFloat_4_FlyInfo, fontScale,
                                            0, 0);
                }
            }
            opacityCursorBytes += 4;
        }
    }
}
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

// Initializes the wind HUD font, text storage, and default color.
CWindInfo::CWindInfo() {
    vtbl = __vt__9CWindInfo;
    SetFont((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, font_wind_gsr)));
    alignment = 1;
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
}

// Shows wind speed or an item/region override at the right edge of the HUD.
void CWindInfo::DrawWindHudText() {
    WindTextFields* self = (WindTextFields*)this;
    void* a = PartyItemSlots_GetInstance();
    float f = kFloat_0_hud;
    WindInventoryDocFields* mgr = (WindInventoryDocFields*)g_sharedDoc;
    char* sub = mgr->subA04;
    char* player = *(char**)(sub + 0x18C);
    void* x = GolfPlayer_GetEntryByIndex(player, *(unsigned char*)(player + 0x3C1));
    unsigned int stage = ItemInventory_GetItemIdAt(sub, 4);
    if ((unsigned int)((GolfDocSnapshot8013*)x)->itemId == stage ||
        ((HudPartyItemSlots*)a)->effects.oneMeterWindEnabled) {
        self->color = (int)0xFF00FF00;
        sprintf((char*)((char*)self->text), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, oneMeterWind)));
    } else {
        if ((((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] >= 0 &&
             CWind::GetInstance()->ContainsPoint(*(const WVector*)GolfBall__Fv())) ||
            (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] < 0 &&
             CWind::GetInstance()->ContainsPoint(*(const WVector*)(s_cameraTransform + 0x24)))) {
            self->color = -1;
            sprintf((char*)self->text, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, unknownWind)));
            f = kFloat_2_805DE318;
        } else if (((HudPartyItemSlots*)a)->effects.windSpeedOverrideEnabled) {
            self->color = (int)0xFFFF0000u; // opaque red (-65536)
            sprintf((char*)self->text, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, unknownWind)));
            f = kFloat_2_805DE318;
        } else {
            self->color = -1;
            sprintf((char*)self->text, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, wholeMetersFormat)),
                    CWind::GetInstance()->GetWindSpeed());
        }
    }

    HudFont_DrawAlignedText(self, self->text, f + (AsWView(g_view)->viewportWidth - kFloat_32_0),
                            AsCOption(g_option)->hudBaselineY - kFloat_46_Hud, kFloat_1_805DE314, kFloat_1_805DE314, 0,
                            0);
}

#pragma opt_lifetimes off
#pragma use_lmw_stmw on
// Creates the reward summary, its optional event overlay, and the item-icon map.
CBonusInfo::CBonusInfo() {
    vtbl = __vt__10CBonusInfo;
    bonusOverlay = 0;
    extraOverlay = 0;
    mapSize = 0;
    root = 0;
    end = (BonusMapNode*)&root;
    SetFont((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, largeFont)));
    alignment = 2;
    void* allocation = __nwa__FUl(kTextBufferBytes);
    elapsed = kFloat_0_hud;
    textBuffer = (char*)allocation;
    earnedPang = 0;
    earnedBonusPang = 0;
    multiplierBonus = 0;
    totalReward = 0;
    bonusOverlay =
        WOverlay_Create(g_resourceManager, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, add_pang_gsr)), 0);
    if (((HudProjectEventFields*)g_project)->eventMarkEnabled)
        extraOverlay = WOverlay_Create(g_resourceManager,
                                       (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, pc_eventmark_gsr)), 0);
}
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

#pragma push
#pragma use_lmw_stmw on
// Destroys the bonus icon tree, optionally freeing its enclosing allocation.
extern "C" D4* dtor_800F66EC(D4* self, int flags) {
    if (self != 0) {
        self->~D4();
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop
// Bonus, timer, shot count, and power information.

struct TimeoutResetFields {
    char reserved_00[0x18];
    int counter;
    float timer;
};

struct TimeoutTextFields {
    void* vtable;
    char reserved_04[0x08];
    unsigned int color;
    int state;
    void* data;
    int mode;
    float opacity;
};

struct HudTextFields {
    void** volatile vtable;
    char reserved_04[8];
    unsigned int color;
    int state;
    void* data;
};
struct TimeoutAnimationFields {
    char reserved_00[0x18];
    int state;
    float value;
    float timer;
};
struct Link {
    Link* next;
    Link* prev;
};
// Same 20-byte node layout as BonusMapNode: +08 is parent pointer/color bits,
// +0C unsigned key, +10 owned overlay handle. Node storage is separate ownership.
struct Node {
    Node* left;
    Node* right;
    unsigned int parent;
    unsigned int key;
    void* resource;
};
struct Tree {
    void* a;
    Link head;
};

struct BonusScene {
    char reserved_00[0x7F8];
    void* items;
    unsigned int count;
};

struct BonusMapNode {
    BonusMapNode* left;
    BonusMapNode* right;
    char reserved_08[4];
    unsigned int key;
    void* value;
};

struct CBonusState {
    virtual void* c();
};

// Release nodes only: the destructor releases every icon payload first.
// These local expansions expose left/right/postorder ownership without moving
// the recursion boundary: root plus three descendant levels, then the shared
// tree routine.
// Inner macros evaluate their node argument once; tree is a stable header.
// Expansion also retains this compiler's switch-table label numbering later
// in the unit (function extraction renumbers otherwise identical relocations).
#define BONUS_CACHE_FREE_BOUNDARY_NODE(tree, node)             \
    {                                                          \
        Node* item = (node);                                   \
        if (item) {                                            \
            if (item->left)                                    \
                OverlayMapTree_FreeSubtree(tree, item->left);  \
            if (item->right)                                   \
                OverlayMapTree_FreeSubtree(tree, item->right); \
            appFreeUni__FPv(item);                             \
        }                                                      \
    }
// Release a parent after both subtrees at the shared-recursion boundary.
#define BONUS_CACHE_FREE_BRANCH(tree, node)                      \
    {                                                            \
        Node* branch = (node);                                   \
        if (branch) {                                            \
            BONUS_CACHE_FREE_BOUNDARY_NODE(tree, branch->left);  \
            BONUS_CACHE_FREE_BOUNDARY_NODE(tree, branch->right); \
            appFreeUni__FPv(branch);                             \
        }                                                        \
    }
// Release one root child after both branch subtrees.
#define BONUS_CACHE_FREE_ROOT_CHILD(tree, node)              \
    {                                                        \
        Node* rootChild = (node);                            \
        if (rootChild) {                                     \
            BONUS_CACHE_FREE_BRANCH(tree, rootChild->left);  \
            BONUS_CACHE_FREE_BRANCH(tree, rootChild->right); \
            appFreeUni__FPv(rootChild);                      \
        }                                                    \
    }
// Release the complete tree. node must be a local pointer: the second pass
// intentionally rereads its volatile pointer for the null test and each use.
#define BONUS_CACHE_FREE_TREE(tree, node)                   \
    {                                                       \
        if (node) {                                         \
            BONUS_CACHE_FREE_ROOT_CHILD(tree, node->left);  \
            BONUS_CACHE_FREE_ROOT_CHILD(tree, node->right); \
            appFreeUni__FPv(node);                          \
        }                                                   \
    }

#pragma push
#pragma use_lmw_stmw on
// Releases bonus overlays, cached item icons, map nodes, and base text resources.
CBonusInfo::~CBonusInfo() {
    // Phases: release owned overlay resources, release cached icon payloads,
    // free cache nodes in postorder, then destroy inherited HUD text resources.
    char* p = (char*)this;
    *(void**)p = __vt__10CBonusInfo;
    if (g_resourceManager) {
        if (bonusOverlay) {
            Object_DeleteVirtual(g_resourceManager, bonusOverlay);
            bonusOverlay = 0;
        }
    }
    if (g_resourceManager) {
        if (extraOverlay) {
            Object_DeleteVirtual(g_resourceManager, extraOverlay);
            extraOverlay = 0;
        }
    }

    void* nullResource = 0;
    Node* endNode = (Node*)&root;
    // The map header's last word is the first (leftmost) node; the sentinel is
    // &root, not a null node. The parent link's low bit stores the tree colour,
    // so mask it before following parent links to the in-order successor.
    Node* entry = *(Node**)(p + 0x3c);
    while (endNode != entry) {
        if (g_resourceManager) {
            void* res = entry->resource;
            if (res) {
                Object_DeleteVirtual(g_resourceManager, res);
                entry->resource = (void*)nullResource;
            }
        }
        Node* successor = entry->right;
        if (successor) {
            while (successor->left)
                successor = successor->left;
            entry = successor;
        } else {
            while (entry != endNode) {
                Node* par = (Node*)(entry->parent & ~1u);
                if (entry == par->left) {
                    entry = par;
                    break;
                }
                entry = par;
            }
        }
    }

    Node* root = *(Node**)(p + 0x38);
    BONUS_CACHE_FREE_TREE(&mapSize, root);
    // Clear the embedded header, then repeat the cleanup on the cleared root.
    // The volatile local keeps the original load/store shape.
    *(void**)(p + 0x3c) = p + 0x38;
    *(void**)(p + 0x38) = 0;
    Node* volatile clearedRoot = *(Node**)(p + 0x38);
    *(void**)(p + 0x34) = 0;
    BONUS_CACHE_FREE_TREE(&mapSize, clearedRoot);

    if (this) {
        *(void**)p = cInfoBaseVtable;
        if (*(void**)(4 + p)) {
            ResourceManager_DeleteResource(g_resourceManager, fontResource);
            *(void**)((4 + p)) = 0;
        }
        if (textBuffer) {
            __dla__FPv(textBuffer);
            textBuffer = 0;
        }
    }
}
#pragma pop

#undef BONUS_CACHE_FREE_TREE
#undef BONUS_CACHE_FREE_ROOT_CHILD
#undef BONUS_CACHE_FREE_BRANCH
#undef BONUS_CACHE_FREE_BOUNDARY_NODE

// Walk runtime type ancestry to test whether the active task is a golf task.
inline int bonusHasType(void* state) {
    if (state) {
        void* node = ((CBonusState*)state)->c();
        void* endNode = &s_golfTaskTypeInfo;
        while (node) {
            if (node == endNode)
                return 1;
            node = *(void**)((char*)node + 4);
        }
        return 0;
    }
    return 0;
}
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_strength_reduction off
// Read the two Pang totals, then calculate only the extra multiplier reward.
// delta advances elapsed seconds. Each of THREE task queries is intentionally
// independent and publishes g_bonusInfoCurrentTask; do not cache the first one.
// Golf-task ancestry chooses currentPlayer; otherwise the byte passed downstream
// is 0xFF (unsigned conversion of -1). GolfPlayer_GetEntryByIndex (golf_doc_turns_and_parameter_xml.cpp)
// does NOT interpret 0xFF specially or check bounds: it adds index*snapshotSize.
// This path therefore requires a valid golf context/player snapshot; a safe
// nongolf fallback is not established by the callee. AuxPart query also gets
// the selected byte, modifier selector 3; no equipment objects are owned here.
// Preserve rounding: integer sum -> float -> multiply account rate -> multiply
// percent -> divide by float 100 -> multiply equipment modifier -> truncate once
// to int -> subtract original integer sum. totalReward then recombines all three
// amounts in the existing integer addition order.
void CBonusInfo::UpdatePangRewards(float delta) {
    void* root;
    elapsed = elapsed + delta;
    void* menu = g_taskManager;
    void* state;
    if (menu)
        state = Object_GetChild(menu);
    else
        state = 0;
    g_bonusInfoCurrentTask = state;
    int found = bonusHasType(state);
    root = g_sharedDoc;
    void* rec = ((void*)AsCSharedDoc(root)->golfDoc);
    unsigned char player = found ? AsCGolfDoc(rec)->currentPlayer : -1;
    void* result = GolfPlayer_GetEntryByIndex(rec, player);
    earnedPang = ((PlayerPangTotalsFields*)result)->pangTotal;

    menu = g_taskManager;
    if (menu)
        state = Object_GetChild(menu);
    else
        state = 0;
    g_bonusInfoCurrentTask = state;
    int found2 = bonusHasType(state);
    root = g_sharedDoc;
    rec = ((void*)AsCSharedDoc(root)->golfDoc);
    player = found2 ? AsCGolfDoc(rec)->currentPlayer : -1;
    result = GolfPlayer_GetEntryByIndex(rec, player);
    earnedBonusPang = ((PlayerPangTotalsFields*)result)->bonusPangTotal;

    menu = g_taskManager;
    if (menu)
        state = Object_GetChild(menu);
    else
        state = 0;
    g_bonusInfoCurrentTask = state;
    int found3 = bonusHasType(state);
    if (found3) {
        void* r = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        player = AsCGolfDoc(r)->currentPlayer;
    } else {
        player = 0xFF;
    }
    root = g_sharedDoc;
    float modifier = CSharedDoc_ComputeAuxPartModifier(root, player, 3);
    int a = earnedPang;
    int b = earnedBonusPang;
    int sum = a + b;
    float fsum = (float)sum;
    float fac = fsum * AsCSharedDoc(root)->pangRewardMultiplier;
    float fframe = (float)AsCSharedDoc(root)->pangRewardPercent;
    fac = fac * fframe / kFloat_100_hud;
    int end = (int)(modifier * fac) - sum;
    multiplierBonus = end;
    totalReward = earnedBonusPang + (multiplierBonus + earnedPang);
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset

// Draws the reward summary and lazily creates the equipped item icons.
void CBonusInfo::DrawRewardsAndItemIcons() {
    // Phases: draw background/event overlays and numeric reward text, then
    // look up or create owned item icons and arrange them in wrapped screen rows.
    struct Rect {
        float v[4];
        // Store a rectangle while retaining its original store order.
        void set(float x, float y, float w, float h) {
            v[2] = w;
            v[0] = x;
            v[3] = h;
            v[1] = y;
        }
        // Store a rectangle in X/Y/width/height order.
        void setOrdered(float x, float y, float w, float h) {
            float xValue = x;
            v[0] = xValue;
            v[1] = y;
            __typeof__(w) _p427 = w;
            v[2] = _p427;
            v[3] = h;
        }
    };
    Rect pos;
    BonusMapNode* end;
    char* scene;
    char* strings;
    unsigned int* it;
    int rowXOffset;
    int rowYOffset;
    unsigned int id;
    register void* resource;
    register float iconHeight;

    if (bonusOverlay) {
        float v2;
        float half;
        float textScale[4];
        char* cam = (char*)g_view;

        float textPos[4];
        float v1;
        float dy;
        float rectH;
        float width;
        float height;
        height = AsWView(cam)->viewportHeight;
        width = AsWView(cam)->viewportWidth;
        half = kFloat_0_5_hud;
        rectH = kFloat_128_Hud;
        dy = height - rectH;
        v1 = (half * dy);
        v2 = half * width - kFloat_256_MessageUI;
        textPos[2] = kFloat_256_MessageUI;
        textPos[3] = rectH;
        textPos[0] = v2;
        textPos[1] = v1;
        textScale[0] = kFloat_0_hud;
        textScale[1] = kFloat_0_hud;
        textScale[2] = kFloat_1_805DE314;
        textScale[3] = kFloat_1_805DE314;
        WOverlay_DrawTransformedRect(bonusOverlay, cam, textScale, kFloat_0_hud, textPos, 0, -1, 0);
    }
    if (((HudProjectEventFields*)g_project)->eventMarkEnabled != 0) {
        if (extraOverlay) {
            float half;
            float sy;
            float vx;
            float vy;
            float height;
            float textScale[4];
            char* cam = (char*)g_view;
            float textPos[4];
            float width;
            width = AsWView(cam)->viewportWidth;
            height = AsWView(cam)->viewportHeight;
            sy = kFloat_25_805DE368 + height;
            half = kFloat_0_5_hud;
            vy = half * sy;
            vx = half * width + kGroundAtlasSandY;
            textPos[1] = vy;
            textPos[0] = vx;
            textPos[2] = kFloat_70_Hud;
            textPos[3] = kFloat_50_HudLayout;
            textScale[0] = kFloat_0_hud;
            textScale[1] = kFloat_0_hud;
            textScale[2] = kFloat_1_805DE314;
            textScale[3] = kFloat_1_805DE314;
            WOverlay_DrawTransformedRect(extraOverlay, cam, textScale, kFloat_0_hud, textPos, 0, -1, 0);
        }
    }

    alignment = 2;
    strings = golfHudFontStrings;
    sprintf((char*)((char*)textBuffer), (strings + IFF_OFFSET(GolfHudStringPool, integerFormat)), earnedPang);
    {
        char* cam = (char*)g_view;
        float x;
        float y;
        float half;
        float width;
        half = kFloat_0_5_hud;
        y = AsWView(cam)->viewportHeight;
        x = kFloat_40_BonusInfo;
        y = half * y - kFloat_53_Hud;
        width = AsWView(cam)->viewportWidth;
        x = half * width + x;
        HudFont_DrawAlignedText(this, (char*)textBuffer, x, y, kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
    }
    sprintf((char*)((char*)textBuffer), (strings + IFF_OFFSET(GolfHudStringPool, integerFormat)), earnedBonusPang);
    {
        char* cam = (char*)g_view;
        float x;
        float y;
        float half;
        float width;
        half = kFloat_0_5_hud;
        y = AsWView(cam)->viewportHeight;
        width = AsWView(cam)->viewportWidth;
        x = kFloat_40_BonusInfo;
        y = half * y - kFloat_18_805DE3C4;
        x = half * width + x;
        HudFont_DrawAlignedText(this, (char*)textBuffer, x, y, kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
    }
    resource = 0;
    alignment = (int)resource;
    sprintf((char*)((char*)textBuffer), (strings + IFF_OFFSET(GolfHudStringPool, percentFormat)), multiplierBonus);
    {
        char* cam = (char*)g_view;
        float x, half, width, y;
        y = AsWView(cam)->viewportHeight;
        half = kFloat_0_5_hud;
        x = kFloat_45_Hud;
        width = AsWView(cam)->viewportWidth;
        x = half * width + x;
        y = half * y - kFloat_18_805DE3C4;
        HudFont_DrawAlignedText(this, (char*)textBuffer, x, y, kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
    }

    {
        char* cam = (char*)g_view;
        Rect rect;
        rect.set(kFloat_0_5_hud * (AsWView(cam)->viewportWidth - kFloat_400_Hud),
                 AsWView(cam)->viewportHeight * kFloat_0_5_hud + kFloat_15_Hud, kFloat_400_Hud, kFloat_3_Hud);
        Sprite_DrawColoredRect(cam, rect.v, 0, 0xFF000000, kFloat_0_hud);
    }
    {
        char* cam = (char*)g_view;

        float yBase, dx, one, half, width, x, y;
        half = kFloat_0_5_hud;
        yBase = half * AsWView(cam)->viewportHeight + kFloat_15_Hud;
        width = AsWView(cam)->viewportWidth;
        one = kFloat_1_805DE314;
        float rect[4];
        rect[3] = one;
        dx = width - kFloat_400_Hud;
        rect[2] = kFloat_398_Hud;
        y = (one + yBase);
        x = half * dx + one;
        rect[1] = y;
        rect[0] = x;
        Sprite_DrawColoredRect(cam, rect, 0, -1, kFloat_0_hud);
    }

    scene = (char*)g_sharedDoc;
    rowXOffset = 0;
    rowYOffset = 0;
    if ((*(int*)&AsCSharedDoc(scene)->bonusItemCount) == 0) {
        float y, converted, x, half, width;
        converted = (float)(int)resource;
        char* cam = (char*)g_view;
        y = AsWView(cam)->viewportHeight;
        half = kFloat_0_5_hud;
        y = half * y + converted;
        width = AsWView(cam)->viewportWidth;
        x = kFloat_30_GolfHud;
        x = half * width + x;
        y = (kFloat_28_GolfHud + y);
        HudFont_DrawAlignedText(this, (strings + IFF_OFFSET(GolfHudStringPool, unavailable)), x, y, kFloat_3_Hud,
                                kFloat_1_805DE314, 0, 0);
        return;
    }
    CBonusInfo* info = this;
    end = (BonusMapNode*)&info->root;
    it = ((unsigned int*)AsCSharedDoc(scene)->bonusItemIds);

    // Cache contract: unsigned item id -> owned WOverlay handle. root's address
    // is the end sentinel; the lower-bound walk remembers the first key>=id,
    // then equality distinguishes a hit from an absent key. Hits borrow the
    // handle for drawing; they neither reload inventory data nor insert again.
    // The supplied item-id list is borrowed and is not modified by drawing.
    while (it != ((unsigned int*)AsCSharedDoc(scene)->bonusItemIds) + (*(int*)&AsCSharedDoc(scene)->bonusItemCount)) {
        id = *it;
        BonusMapNode* node = info->root;
        BonusMapNode* found = (BonusMapNode*)&info->root;
        resource = 0;
        while (node) {
            if (node->key >= id) {
                found = node;
                node = node->left;
            } else {
                node = node->right;
            }
        }
        if (found == end || id < found->key)
            found = (BonusMapNode*)&info->root;

        if (found != end) {
            resource = found->value;
        } else {
            char* obj = (char*)ItemInventory_FindDataWithHairStyleFallback(
                (char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, id);
            if (obj) {
                // Top six id bits choose RESOURCE PATH, not a dense array:
                // categories 5/6 use itemResourceFormat; 2 uses fashionResourceFormat.
                // Missing inventory data, unsupported category or null creation
                // leaves no cache entry, so later frames retry and draw no icon.
                switch (id >> 26) {
                case 5:
                    resource = WOverlay_Create(
                        g_resourceManager,
                        String_FormatTemporary((strings + IFF_OFFSET(GolfHudStringPool, itemResourceFormat)),
                                               ((IffItemCommon*)obj)->icon),
                        0);
                    break;
                case 6:
                    resource = WOverlay_Create(
                        g_resourceManager,
                        String_FormatTemporary((strings + IFF_OFFSET(GolfHudStringPool, itemResourceFormat)),
                                               ((IffItemCommon*)obj)->icon),
                        0);
                    break;
                case 2:
                    resource = WOverlay_Create(
                        g_resourceManager,
                        String_FormatTemporary((strings + IFF_OFFSET(GolfHudStringPool, fashionResourceFormat)),
                                               ((IffItemCommon*)obj)->icon),
                        0);
                    break;
                default:
                    resource = 0;
                    break;
                }
                if (resource) {
                    // Insertion search returns a parent, predecessor and two
                    // link flags (golf_hud_hole_info_and_fonts.cpp). It copies this key/handle pair into
                    // a map-owned node only if predecessor is absent or smaller.
                    // The destructor deletes cached handles before node storage.
                    // No local handle rollback exists if insertion throws or is
                    // skipped; preserve this ownership edge, do not claim RAII.
                    BonusMapNode* outNode;
                    unsigned int key[2];
                    unsigned char flag9;
                    unsigned char flag8;
                    key[0] = id;
                    key[1] = (unsigned int)resource;
                    void* iter = HudFontConfigMap_FindInsertionPosition(&info->mapSize, key, (unsigned int*)&outNode,
                                                                        &flag9, &flag8);
                    if (!outNode || outNode->key < key[0])
                        HudFontConfigMap_InsertAndRebalance(&info->mapSize, iter, flag9, flag8, key);
                }
            }
        }

        if (resource) {
            float adj[2];
            iconHeight = (float)WOverlay_GetHeight(resource);
            float iconWidth = (float)WOverlay_GetWidth(resource);
            char* cam = (char*)g_view;
            Rect scale;
            Rect shifted;

            Rect scale2;
            pos.set((float)rowXOffset + (kFloat_0_5_hud * AsWView(cam)->viewportWidth + kFloat_15_Hud),
                    (float)rowYOffset + (kFloat_0_5_hud * AsWView(cam)->viewportHeight + kFloat_25_805DE368), iconWidth,
                    iconHeight);
            // Fashion-path icons keep their own dimensions but draw 11 pixels
            // up/left; the logical row anchor still advances by their width.
            if ((id >> 26) == 2) {
                adj[0] = kFloat_neg11_Hud;
                adj[1] = kFloat_neg11_Hud;
                float wValue = iconHeight;
                shifted.setOrdered((pos.v[0] + adj[0]), (pos.v[1] + adj[1]), iconWidth, wValue);
                scale.setOrdered(kFloat_0_hud, kFloat_0_hud, kFloat_1_805DE314, kFloat_1_805DE314);
                WOverlay_DrawTransformedRect(resource, cam, scale.v, kFloat_0_hud, shifted.v, 0, -1, 0);
            } else {
                scale2.setOrdered(kFloat_0_hud, kFloat_0_hud, kFloat_1_805DE314, kFloat_1_805DE314);
                WOverlay_DrawTransformedRect(resource, cam, scale2.v, kFloat_0_hud, pos.v, 0, -1, 0);
            }
            // Advance X by max(icon width,40) pixels. Wrap AFTER drawing when
            // accumulated X exceeds 132; next row starts 40 pixels lower.
            int advance = WOverlay_GetWidth(resource);
            unsigned int step = 40;
            if (40 < advance)
                step = advance;
            rowXOffset += step;
            if (rowXOffset > 132) {
                rowYOffset = 40 + rowYOffset;
                rowXOffset = 0;
            }
        }
        it = (it + 1);
    }
}

// Creates the remaining-turn-time text display.
CRemainTimeInfo::CRemainTimeInfo() {
    extern char __vt__15CRemainTimeInfo[];
    vtbl = __vt__15CRemainTimeInfo;
    SetFont((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, barFont)));
    alignment = 1;
    color = 0xFF147EFF;
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
}

// Draws remaining seconds rounded upward and capped at 99, or a dash for no time limit.
void CRemainTimeInfo::ShowRemainingTimeText() {
    void* sceneRoot = g_sharedDoc;
    if ((*(int*)&AsCSharedDoc(sceneRoot)->turnTimeLimitMs) == 0) {
        sprintf((char*)((char*)textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, unavailable)));
        void* cfg = g_option;
        float one = kFloat_1_805DE314;
        float verticalOffset = kFloat_34_Hud;
        float hudBaselineY = AsCOption(cfg)->hudBaselineY;
        float textX = kFloat_88_GolfHud;
        float textY = verticalOffset + hudBaselineY;
        HudFont_DrawAlignedText(this, textBuffer, textX, textY, one, one, 0, 0);
    } else {
        void* rec = ((void*)AsCSharedDoc(sceneRoot)->golfDoc);
        unsigned int remainingMs = AsCGolfDoc(rec)->remainingTimeMs;
        unsigned int roundedSeconds = remainingMs / kMillisecondsPerSecond;
        if (remainingMs > roundedSeconds * kMillisecondsPerSecond)
            ++roundedSeconds;
        unsigned int displaySeconds = roundedSeconds > kMaxDisplayedSeconds ? kMaxDisplayedSeconds : roundedSeconds;
        sprintf((char*)((char*)textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                displaySeconds);
        void* cfg = g_option;
        float one = kFloat_1_805DE314;
        float verticalOffset = kFloat_34_Hud;
        float hudBaselineY = AsCOption(cfg)->hudBaselineY;
        float textX = kFloat_88_GolfHud;
        float textY = verticalOffset + hudBaselineY;
        HudFont_DrawAlignedText(this, textBuffer, textX, textY, one, one, 0, 0);
    }
}

// Restarts the timeout display animation.
void CTimeOutInfo::vfunc_5() {
    TimeoutResetFields* self = (TimeoutResetFields*)this;
    float timer = kFloat_0_hud;
    self->counter = 0;
    self->timer = timer;
}

// Advances timeout text through fade-in, hold, fade-out, and hidden states.
void CTimeOutInfo::UpdateFadeState(float delta) {
    TimeoutAnimationFields* self = (TimeoutAnimationFields*)this;
    switch (self->state) {
    case kTimeoutHold: {
        float timer = self->timer + delta;
        self->timer = timer;
        if (timer > kFlightSecondaryPinFadeStartTime)
            self->state = kTimeoutFadeOut;
        return;
    }
    case kTimeoutHidden: {
        self->value = kFloat_0_hud;
        return;
    }
    case kTimeoutFadeIn: {
        float value = self->value + kFloat_510_Hud * delta;
        self->value = value;
        if (value > kFloat_255_MessageUI) {
            self->value = kFloat_255_MessageUI;
            self->state = kTimeoutHold;
            self->timer = kFloat_0_hud;
        }
        return;
    }
    case kTimeoutFadeOut: {
        float value = self->value - kFloat_510_Hud * delta;
        self->value = value;
        if (value < kFloat_0_hud) {
            self->value = kFloat_0_hud;
            self->state = kTimeoutHidden;
        }
        return;
    }
    default:
        return;
    }
}

// Formats the remaining timeout allowance and updates its red text opacity.
void CTimeOutInfo::SetTimeoutCountTextAndColor() {
    TimeoutTextFields* self = (TimeoutTextFields*)this;
    if (self->mode == 3)
        return;
    void* sceneRoot = g_sharedDoc;
    void* scene =
        GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(sceneRoot)->golfDoc),
                                   *((unsigned char*)((char*)((void*)AsCSharedDoc(sceneRoot)->golfDoc) + 0x3C1)));
    unsigned char index =
        ((GolfDocSnapshot8013*)scene)->timeoutsUsed; // Timeout allowances already used by this player.
    sprintf((char*)((char*)self->data), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)), 3 - index);
    unsigned char alpha = (unsigned char)Runtime_ConvertDoubleToUnsigned(self->opacity);
    self->color = ((unsigned int)alpha << 24) | 0x00FF0000u; // red, alpha from the opacity
}

// Creates the shot-count font and numeric text buffer.
CShotCountInfo::CShotCountInfo() {
    extern char __vt__14CShotCountInfo[];
    vtbl = __vt__14CShotCountInfo;
    SetFont(golfHudFontStrings);
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
    alignment = 1;
}

// Releases shot-count font resources and text storage; the compiler adds the
// object null check and the delete from the deleting form of the destructor.
CShotCountInfo::~CShotCountInfo() {
    if (this) {
        void* resource = fontResource;
        vtbl = (void*)cInfoBaseVtable;
        if (resource) {
            ResourceManager_DeleteResource(g_resourceManager, resource);
            fontResource = 0;
        }
        if (textBuffer) {
            __dla__FPv(textBuffer);
            textBuffer = 0;
        }
    }
}

// Restarts the shot-count display at full opacity.
void CShotCountInfo::ResetFade() {
    float resetTimer = kShotCountFadeStepSeconds;
    counter = 0;
    timer = resetTimer;
    alpha = 0xFF;
}

// Fades the shot count in 60 timed alpha steps.
void CShotCountInfo::UpdateFade(float delta) {
    if (timer <= kFloat_0_hud)
        return;
    timer -= delta;
    if (!(timer < kFloat_0_hud))
        return;
    counter++;
    if (counter == kShotCountFadeSteps) {
        timer = kFloat_0_hud;
        return;
    }
    alpha -= kShotCountAlphaStep;
    timer = kShotCountFadeStepSeconds;
}

// Creates the font and text storage for the shot-distance display.
CPowerInfo::CPowerInfo() {
    extern char __vt__10CPowerInfo[];
    vtbl = __vt__10CPowerInfo;
    SetFont((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, barFont)));
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
    alignment = 1;
}

#pragma opt_lifetimes off
// Draws the horizontal ball distance in yards and its half-distance reference.
void CPowerInfo::DrawShotDistanceYards() {
    void* p = GolfBall__Fv();
    float temp[4];
    __mi__FRC7WVectorRC7WVector(temp, &shotOrigin, p);
    temp[1] = kFloat_0_hud;
    float distance = (*(WVector*)temp).Magnitude();
    float ratio = distance / kFloat_3_2_hud;
    sprintf((char*)((char*)textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, wholeYardsFormat)), ratio);
    float y0 = kFloat_8_Hud;
    float cfg0 = AsCOption(g_option)->hudBaselineY;

    float sum0 = y0 + cfg0;
    y0 = kFloat_502_GolfHud;
    HudFont_DrawAlignedText(this, textBuffer, y0, sum0 - kFloat_7_Hud, kFloat_1_805DE314, kFloat_1_1_Hud, 0, 0);
    sprintf((char*)((char*)textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, wholeYardsFormat)),
            (kFloat_0_5_hud * ratio));

    float y1 = kFloat_8_Hud;
    float cfg1 = AsCOption(g_option)->hudBaselineY;
    float x1 = (y1 + cfg1) - kFloat_7_Hud;

    y1 = kFloat_328_Hud;
    HudFont_DrawAlignedText(this, textBuffer, y1, x1, kFloat_1_805DE314, kFloat_1_1_Hud, 0, 0);
}

#pragma opt_lifetimes reset
// Accuracy and hole information.
// Constructor/destructor alias map:
// Accuracy +04 is the base-owned font resource; +14/+28 are owned text buffers,
// +18 active, +1C opacity, +20 owned impact-label overlay, +24 gauge screen X.
// Hole +14 is the base-owned text buffer. +18..+68 are the animation coordinates
// named by HoleAnimationFields below (screen X/Y, interpolation ends, scale).
// Hole +9C/+A0 course-name overlays, +A4 hole/par overlay, +A8 turn-order overlay;
// +B4 completion overlay, +B8 score-event overlay, +D8 nine owned player textures.
// Resource deletion uses the matching overlay/resource/array destructor, not free().

struct HoleFlashFields {
    char reserved_00[0x94];
    int flashCount;
    int animationState;
};

struct HoleAnimationFields {
    char reserved_00[0x18];
    float courseStartX, courseStartY, courseEndX, courseEndY;
    float courseX, courseY, courseScale, holeStartX, holeStartY;
    float holeEndX, holeEndY, holeX, holeY, holeScale, playerStartX, playerStartY;
    float playerEndX, playerEndY, playerX, playerY, playerScale;
    char reserved_6C[0x1C];
    float courseOpacity, playerOpacity, animationTime;
    int flashCount, animationState;
};

struct AccuracyFadeFields {
    char reserved_00[0x14];
    unsigned char* mainMessage;
    unsigned char active;
    char reserved_19[3];
    float opacity;
    char reserved_20[8];
    unsigned char* secondaryMessage;
};

struct AccuracyMessageFields {
    char reserved_00[0x14];
    char* mainMessage;
    unsigned char active;
    char reserved_19[3];
    float opacity;
    float field20;
    float gaugeOffsetX;
    void* secondaryMessage;
};

// Alternate projection of the hole pose: the fields are checked against
// HoleAnimationFields below. +88/+8C are alpha 0..255; +90 is elapsed seconds
// during fade/flash and the interpolation fraction while moving.
struct HoleCornerFields {
    char reserved_00[0x20];
    float courseEndX, courseEndY, courseX, courseY, courseScale;
    char reserved_34[8];
    float holeEndX, holeEndY, holeX, holeY, holeScale;
    char reserved_50[8];
    float playerEndX, playerEndY, playerX, playerY, playerScale;
    char reserved_6C[0x1C];
    float courseOpacity, playerOpacity, animationTime;
    char reserved_94[4];
    int animationState;
};

struct HoleResetFields {
    char reserved_00[0x18];
    float courseStartX, courseStartY, courseEndX, courseEndY, courseX, courseY, courseScale;
    float holeStartX, holeStartY, holeEndX, holeEndY, holeX, holeY, holeScale;
    float playerStartX, playerStartY, playerEndX, playerEndY, playerX, playerY, playerScale;
    char reserved_6C[0x1C];
    float courseOpacity, playerOpacity, animationTime;
    int flashCount;
    int animationState;
    char reserved_9C[0x40];
    unsigned char selectedPlayer;
};

// Verify the renamed projections against the offsets used by animation/draw.
// This pre-C++11 compiler has no static_assert. Anonymous enum + array bound
// emits no data and makes a displaced field a compile error. Keep this form:
// together with the cleanup expansion it retains the later switch-table label
// identities, so the acceptance gate recognizes the untouched animation body.
#define CHECK_HOLE_FIELD(type, member, offset)                                   \
    enum {                                                                       \
        type##_##member##_layout = (IFF_OFFSET(type, member) == offset) ? 1 : -1 \
    };                                                                           \
    typedef char type##_##member##_check[type##_##member##_layout]
CHECK_HOLE_FIELD(HoleCornerFields, courseEndX, 0x20);
CHECK_HOLE_FIELD(HoleCornerFields, courseEndY, 0x24);
CHECK_HOLE_FIELD(HoleCornerFields, courseX, 0x28);
CHECK_HOLE_FIELD(HoleCornerFields, courseY, 0x2C);
CHECK_HOLE_FIELD(HoleCornerFields, courseScale, 0x30);
CHECK_HOLE_FIELD(HoleCornerFields, holeEndX, 0x3C);
CHECK_HOLE_FIELD(HoleCornerFields, holeEndY, 0x40);
CHECK_HOLE_FIELD(HoleCornerFields, holeX, 0x44);
CHECK_HOLE_FIELD(HoleCornerFields, holeY, 0x48);
CHECK_HOLE_FIELD(HoleCornerFields, holeScale, 0x4C);
CHECK_HOLE_FIELD(HoleCornerFields, playerEndX, 0x58);
CHECK_HOLE_FIELD(HoleCornerFields, playerEndY, 0x5C);
CHECK_HOLE_FIELD(HoleCornerFields, playerX, 0x60);
CHECK_HOLE_FIELD(HoleCornerFields, playerY, 0x64);
CHECK_HOLE_FIELD(HoleCornerFields, playerScale, 0x68);
CHECK_HOLE_FIELD(HoleCornerFields, courseOpacity, 0x88);
CHECK_HOLE_FIELD(HoleCornerFields, playerOpacity, 0x8C);
CHECK_HOLE_FIELD(HoleCornerFields, animationTime, 0x90);
CHECK_HOLE_FIELD(HoleCornerFields, animationState, 0x98);
CHECK_HOLE_FIELD(HoleResetFields, courseStartX, 0x18);
CHECK_HOLE_FIELD(HoleResetFields, courseStartY, 0x1C);
CHECK_HOLE_FIELD(HoleResetFields, courseEndX, 0x20);
CHECK_HOLE_FIELD(HoleResetFields, holeStartX, 0x34);
CHECK_HOLE_FIELD(HoleResetFields, holeStartY, 0x38);
CHECK_HOLE_FIELD(HoleResetFields, holeEndX, 0x3C);
CHECK_HOLE_FIELD(HoleResetFields, playerStartX, 0x50);
CHECK_HOLE_FIELD(HoleResetFields, playerStartY, 0x54);
CHECK_HOLE_FIELD(HoleResetFields, flashCount, 0x94);
CHECK_HOLE_FIELD(HoleResetFields, animationState, 0x98);
CHECK_HOLE_FIELD(HoleResetFields, selectedPlayer, 0xDC);
#undef CHECK_HOLE_FIELD

// Resource projections shared by constructor/destructor views, with ownership at uses.
struct HoleOverlayResources {
    char padToOverlays[0x9C];
    void* courseName;
    void* alternateCourseName;
    void* holeAndPar;
    void* turnOrder;
    void* optionalOverlayAC;
    void* optionalSlotB0;
    void* completion;
    void* scoreEvent;
    char padToPlayerTextures[0x1C];
    void* playerTextures;
};
// Borrowed 32-byte course entry: world-space cup XYZ and this hole's par.
struct RewardPositionBits {
    int x, y, z;
};
struct RewardHoleRecord {
    char reserved00[0xC];
    union {
        Vec3f cupWorld;
        RewardPositionBits cupWorldBits;
    };
    unsigned char par;
    char reserved19[7];
};
struct RewardPlayerSummary {
    char padToStrokes[0x28];
    unsigned char strokes[18];
    unsigned char perHoleStatus[18]; // +0x3A: per-hole ground-type-2 putter-shot counter; see RewardHoleStatus.
    char reserved4C[0xA0 - 0x4C];
    unsigned int equippedItemId;
    char reservedA4[0xB4 - 0xA4];
    unsigned int rewardFlags;
    char reservedB8[0xC8 - 0xB8];
    int matchScore, pang, bonusPang;
    char padToFinishKind[0x148 - 0xD4];
    unsigned char finishRewardKind, objectHitCount;
};
enum RewardModeBounds {
    kFirstStrokeOverrideMode = 1,
    kLastStrokeOverrideMode = 2
};
typedef char RewardHoleRecordSize[(sizeof(RewardHoleRecord) == 0x20) ? 1 : -1];
typedef char RewardPlayerFinishOffset[((int)&((RewardPlayerSummary*)0)->finishRewardKind == 0x148) ? 1 : -1];
struct AccuracyResourceFields {
    void* vtable;
    void* field04;
    char reserved_08[0x0C];
    void* field14;
    char reserved_18[8];
    void* field20;
    char reserved_24[4];
    void* field28;
};

struct AccuracyDrawFields {
    char reserved_00[0x14];
    void* mainMessage;
    unsigned char active;
    char reserved_19[3];
    float opacity;
    void* textOverlay;
    float gaugeOffsetX;
    void* secondaryMessage;
};

struct HoleResourceFields {
    void* vtable;
    void* field04;
    char reserved_08[0x0C];
    void* field14;
    char reserved_18[0x84];
    void* field9C;
    void* fieldA0;
    void* fieldA4;
    void* fieldA8;
    void* fieldAC;
    char reserved_B0[4];
    void* fieldB4;
    void* fieldB8;
    char reserved_BC[0x1C];
    void* fieldD8;
};

struct HoleRectTable {
    char data[0x50];
    // Initialize the overlay resource for a named hole rectangle.
    HoleRectTable(const char* name) {
        OverlayPlayer_Init(this, name);
    }
};

// Store the hole-label position in the original Z/Y/X assignment order.
static inline void HolePosition(float* p, float z, float y, float x) {
    p[2] = z;
    p[1] = y;
    p[0] = x;
}

// Store a screen rectangle while preserving the original Y/width/X/height store order.
static inline void HoleRectangle(float* p, float x, float y, float w, float h) {
    p[1] = y;
    p[2] = w;
    p[0] = x;
    p[3] = h;
}

// Creates the impact-message font, message buffers, and named label rectangles.
CAccuracyInfo::CAccuracyInfo() {
    vtbl = __vt__13CAccuracyInfo;
    textOverlay = 0;
    secondaryMessage = 0;
    SetFont((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, barFont)));
    void* panel = __nw__FUl(0x50);
    if (panel) {
        OverlayPlayer_Init(panel, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, impact_gsr)));
    }
    textOverlay = panel;
    active = 0;
    float a[4] = {kFloat_0_hud, kFloat_0_hud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, badImpact)), a,
                                      0);
    float b[4] = {kFloat_0_hud, kFloat_18_805DE3C4, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, pangyaImpact)),
                                      b, 0);
    float c[4] = {kFloat_0_hud, kFloat_36_GolfHud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerSpin)), c,
                                      0);
    float d[4] = {kFloat_0_hud, kFloat_54_Hud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerCurve)), d,
                                      0);
    float e[4] = {kFloat_0_hud, kFloat_72_Hud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, Tomahawk)), e,
                                      0);
    float f[4] = {kFloat_0_hud, kCupHudFixedScreenY, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, fullPower)), f,
                                      0);
    float g[4] = {kFloat_0_hud, kFloat_108_hud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, superPangya)), g,
                                      0);
    float h[4] = {kFloat_0_hud, kFloat_126_Hud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, Cobra)), h, 0);
    float i[4] = {kFloat_0_hud, kFloat_144_Hud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, Spike)), i, 0);
    float j[4] = {kFloat_0_hud, kFloat_162_Hud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, PowerTomahawk)),
                                      j, 0);
    float k[4] = {kFloat_0_hud, kFloat_180_Hud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, PowerCobra)), k,
                                      0);
    float l[4] = {kFloat_0_hud, kFloat_198_GolfHud, kFloat_128_Hud, kFloat_18_805DE3C4};
    Register_NormalizedNamedRectangle(textOverlay, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, PowerSpike)), l,
                                      0);
    secondaryMessage = 0;
    textBuffer = 0;
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
    *(unsigned char*)textBuffer = 0;
    secondaryMessage = (char*)__nwa__FUl(kTextBufferBytes);
    *(unsigned char*)secondaryMessage = 0;
    alignment = 1;
    gaugeOffsetX = kFloat_0_hud;
}

// Releases impact-message overlays and both message buffers.
CAccuracyInfo::~CAccuracyInfo() {
    AccuracyResourceFields* self = (AccuracyResourceFields*)this;
    void* p = ((AccuracyDrawFields*)self)->textOverlay;
    self->vtable = __vt__13CAccuracyInfo;
    if (p) {
        OverlayPlayer_Destroy(p, 1);
        ((AccuracyDrawFields*)self)->textOverlay = 0;
    }
    if (((AccuracyDrawFields*)self)->secondaryMessage) {
        __dla__FPv(((AccuracyDrawFields*)self)->secondaryMessage);
        ((AccuracyDrawFields*)self)->secondaryMessage = 0;
    }
    if (self) {
        self->vtable = cInfoBaseVtable;
        if (self->field04) {
            ResourceManager_DeleteResource(g_resourceManager, self->field04);
            self->field04 = 0;
        }
        if (((AccuracyDrawFields*)self)->mainMessage) {
            __dla__FPv(((AccuracyDrawFields*)self)->mainMessage);
            ((AccuracyDrawFields*)self)->mainMessage = 0;
        }
    }
}

// Fades an active impact message, then clears both message lines.
void CAccuracyInfo::FadeOutMessages(float amount) {
    AccuracyFadeFields* self = (AccuracyFadeFields*)this;
    if (self->active != 0) {
        float zero = kFloat_0_hud;
        float value, speed;
        speed = kFloat_128_Hud;
        value = self->opacity;
        float next = value - speed * amount;
        self->opacity = next;
        if (next < zero) {
            self->opacity = zero;
            self->active = 0;
            *self->mainMessage = 0;
            *self->secondaryMessage = 0;
        }
    }
}

// Starts the full-power impact notification.
void CAccuracyInfo::ShowFullPower() {
    AccuracyMessageFields* self = (AccuracyMessageFields*)this;
    self->active = 1;
    self->opacity = kFloat_255_MessageUI;
    sprintf((char*)(self->mainMessage), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, fullPower)));
    self->gaugeOffsetX = kFloat_445_GolfHud;
}

// Builds impact feedback from the impact grade, special shot flags, spin, and curve.
void CAccuracyInfo::ShowCurrentShotFeedback() {
    AccuracyMessageFields* self = (AccuracyMessageFields*)this;
    // Phases: reset both message buffers and opacity, select the impact-grade
    // message, then append or replace it with shot/spin/curve labels. Early returns
    // implement priority: special-shot label before spin/curve before normal Pangya.
    float a = kFloat_255_MessageUI;
    float b = kFloat_64_Hud;
    self->active = 1;
    self->opacity = a;
    void* state = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    self->gaugeOffsetX = AsCGolfDoc(state)->shotImpactGaugeX - b;
    *self->mainMessage = 0;
    *(unsigned char*)self->secondaryMessage = 0;

    unsigned int flags;
    void* manager = g_sharedDoc;
    state = ((void*)AsCSharedDoc(manager)->golfDoc);
    switch (AsCGolfDoc(state)->impactGrade) {
    case 4:
        flags = (unsigned int&)((CGolfBall*)GolfBall__Fv())->m_powerFlags;
        if (flags & kSpecialShotMask) {
            if (((HudGolfParameters*)InstanceTable_Get())->powerMode == 2)
                strcpy(self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerPrefix)));
            else
                strcpy(self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, impactPrefix)));
            if ((unsigned int&)((CGolfBall*)GolfBall__Fv())->m_powerFlags & kTomahawkShot)
                strcat(self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, tomahawkSuffix)));
            else if ((unsigned int&)((CGolfBall*)GolfBall__Fv())->m_powerFlags & kCobraShot)
                strcat(self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, cobraSuffix)));
            else if ((unsigned int&)((CGolfBall*)GolfBall__Fv())->m_powerFlags & kSpikeShot)
                strcat(self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, spikeSuffix)));
            flags = (unsigned int&)((CGolfBall*)GolfBall__Fv())->m_powerFlags;
            if ((flags & kPowerSpinMask) != 0) {
                sprintf((char*)((char*)self->secondaryMessage),
                        (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerSpin)));
                return;
            }
            flags = (unsigned int&)((CGolfBall*)GolfBall__Fv())->m_powerFlags;
            if ((flags & kPowerCurveMask) != 0) {
                sprintf((char*)((char*)self->secondaryMessage),
                        (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerCurve)));
                return;
            }
        } else {
            flags = (unsigned int&)((CGolfBall*)GolfBall__Fv())->m_powerFlags;
            if ((flags & kPowerSpinMask) != 0) {
                sprintf((char*)(self->mainMessage), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerSpin)));
                return;
            }
            flags = (unsigned int&)((CGolfBall*)GolfBall__Fv())->m_powerFlags;
            if ((flags & kPowerCurveMask) != 0) {
                sprintf((char*)(self->mainMessage), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerCurve)));
                return;
            }
            if (((HudGolfParameters*)InstanceTable_Get())->powerMode != 0) {
                manager = g_sharedDoc;
                state = ((void*)AsCSharedDoc(manager)->golfDoc);
                if (((HudImpactFields*)state)->superPangya != 0) {
                    sprintf((char*)(self->mainMessage),
                            (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, superPangya)));
                    return;
                }
            }
            sprintf((char*)(self->mainMessage), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, pangyaImpact)));
            return;
        }
        break;
    case 0:
    case 1:
        sprintf((char*)(self->mainMessage), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, badImpact)));
        return;
    }
}

// Draws up to two impact feedback lines for a human player.
void CAccuracyInfo::DrawMessages() {
    AccuracyDrawFields* self = (AccuracyDrawFields*)this;
    if (self->active == 0)
        return;
    if (self->textOverlay == 0)
        return;
    switch (SharedDoc_IsPlayerFlag0Set(
        *((unsigned char*)*((void**)((char*)&AsCSharedDoc(g_sharedDoc)->golfDoc)) + 0x3C1))) {
    case 0:
        break;
    default:
        return;
    }
    if (strchr((const char*)self->mainMessage, 0x40)) {
        unsigned long alpha = Runtime_ConvertDoubleToUnsigned((double)self->opacity);
        ((HudTextFields*)self->textOverlay)->color = (alpha << 24) | kRgbWhite;
        float height = AsCOption(g_option)->hudBaselineY;
        float x = kFloat_5_Hud + self->gaugeOffsetX;
        float y = height - kDriveHudLabelHeight - kFloat_8_Hud;
        NamedText_DrawAlignedScaled(self->textOverlay, self->mainMessage, 0, x, y, kFloat_1_805DE314);
    }
    if (strchr((const char*)self->secondaryMessage, 0x40)) {
        unsigned long alpha = Runtime_ConvertDoubleToUnsigned((double)self->opacity);
        ((HudTextFields*)self->textOverlay)->color = (alpha << 24) | kRgbWhite;
        float height = AsCOption(g_option)->hudBaselineY;
        float x = kFloat_5_Hud + self->gaugeOffsetX;
        float y = kFloat_2_805DE318 + height;
        NamedText_DrawAlignedScaled(self->textOverlay, self->secondaryMessage, 0, x, y, kFloat_1_805DE314);
    }
}

// Builds impact feedback from supplied shot flags and power mode.
void CAccuracyInfo::ShowShotFlagFeedback(int flags, int mode) {
    AccuracyMessageFields* self = (AccuracyMessageFields*)this;
    self->active = 1;
    self->opacity = kFloat_255_MessageUI;
    self->gaugeOffsetX = kFloat_85_GolfHud;
    *(unsigned char*)self->mainMessage = 0;
    *(unsigned char*)self->secondaryMessage = 0;
    if (flags & kSpecialShotMask) {
        if (mode == 2)
            strcpy((char*)self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerPrefix)));
        else
            strcpy((char*)self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, impactPrefix)));
        if (flags & kTomahawkShot)
            strcat((char*)self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, tomahawkSuffix)));
        else if (flags & kCobraShot)
            strcat((char*)self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, cobraSuffix)));
        else if (flags & kSpikeShot)
            strcat((char*)self->mainMessage, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, spikeSuffix)));
        if (flags & kPowerSpinMask)
            sprintf((char*)((char*)self->secondaryMessage),
                    (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerSpin)));
        else if (flags & kPowerCurveMask)
            sprintf((char*)((char*)self->secondaryMessage),
                    (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerCurve)));
    } else {
        if (flags & kPowerSpinMask)
            sprintf((char*)((char*)self->mainMessage), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerSpin)));
        else if (flags & kPowerCurveMask)
            sprintf((char*)((char*)self->mainMessage),
                    (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, powerCurve)));
    }
}

// Creates course, hole, par, player-order labels, and their supporting overlays.
CHoleInfo::CHoleInfo() {
    // Phases: initialize animation/layout state, create course/hole/par/order
    // rectangles, create nine player textures, then load completion overlays.
    void* r28;
    vtbl = __vt__9CHoleInfo;
    courseName = 0;
    alternateCourseName = 0;
    holeAndPar = 0;
    turnOrder = 0;
    optionalOverlayAC = 0;
    optionalSlotB0 = 0;
    completion = 0;
    playerTextures = 0;
    SetFont((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, largeFont)));
    alignment = 0;
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
    float f1 = kFloat_0_hud;
    animationTime = f1;
    courseOpacity = f1;
    playerOpacity = f1;
    HolePosition(&courseStartX, kHoleIntroPoseZ, kHoleIntroStartY,
                 kFloat_0_5_hud * AsWView(g_view)->viewportWidth - kHoleIntroCourseInset);
    courseEndX = kHoleIntroEndX;
    courseEndY = kFloat_36_GolfHud;
    HoleRectangle(&holeStartX, kFloat_0_5_hud * AsWView(g_view)->viewportWidth - kGroundAtlasBunkerY, kHoleIntroHoleY,
                  kHoleIntroHoleWidth, kCupHeightBelowTextYOffset);
    HoleRectangle(&playerStartX, kFloat_0_5_hud * AsWView(g_view)->viewportWidth - kFloat_144_Hud, kHoleIntroPlayerY,
                  kFloat_85_GolfHud, kFloat_30_GolfHud);
    void* manager = g_sharedDoc;
    r28 = new HoleRectTable((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, start_name_gsr)));
    courseName = r28;
    r28 = new HoleRectTable((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, start_name_2_gsr)));
    alternateCourseName = r28;
    if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 3) {
        int n = ((int)AsCSharedDoc(g_sharedDoc)->partyCourseTableIndex);
        float a[4];
        a[0] = kFloat_0_hud;
        a[2] = kFloat_256_MessageUI;
        a[1] = kFloat_34_Hud * (float)n;
        a[3] = kFloat_34_Hud;
        Register_NormalizedNamedRectangle(
            alternateCourseName, (const char*)(golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, PartyCourseName)), a,
            0);
    } else {
        char* ent = (char*)((HudCourseInventoryFields*)manager)->courseEntries;
        int i = 0;
        float width = kFloat_256_MessageUI;
        float zero = kFloat_0_hud;
        float height = kFloat_34_Hud;
        for (; i < ItemInventory_GetArchiveCourseCount((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory); i++) {
            unsigned int n = *(unsigned int*)(ent + 0x54) & 0x03FFFFFF;
            float a[4];
            a[0] = zero;
            a[2] = width;
            a[3] = height;
            a[1] = height * (float)n;
            Register_NormalizedNamedRectangle(courseName, ent + 4, a, 0);
            ent += 0x108;
        }
    }
    r28 = new HoleRectTable((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, start_hole_gsr)));
    holeAndPar = r28;
    float a[4] = {kFloat_0_hud, kFloat_0_hud, kFloat_64_Hud, kFloat_32_0};
    Register_NormalizedNamedRectangle(holeAndPar,
                                      (const char*)(golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, Hole)), a, 0);
    float b[4] = {kFloat_0_hud, kFloat_32_0, kFloat_64_Hud, kFloat_32_0};
    Register_NormalizedNamedRectangle(holeAndPar,
                                      (const char*)(golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, Par)), b, 0);
    r28 = new HoleRectTable((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, start_posi_gsr)));
    turnOrder = r28;
    if (!*((unsigned char*)((char*)&AsCSharedDoc(g_sharedDoc)->golfRuleType))) {
        float c[4] = {kFloat_0_hud, kFloat_0_hud, kFlightRemainingDistanceLabelWidth, kFloat_32_0};
        Register_NormalizedNamedRectangle(
            turnOrder, (const char*)(golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, leaderLabel)), c, 0);
    } else {
        float c[4] = {kFloat_0_hud, kFloat_32_0, kFloat_64_Hud, kFloat_32_0};
        Register_NormalizedNamedRectangle(turnOrder,
                                          (const char*)(golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, vs)), c, 0);
    }
    struct Names {
        int vals[9];
    };
    Names names = *(Names*)s_holeIntroPlayerTextureIds;
    void* arr = __nwa__FUl(0x178);
    if (arr != 0) {
        arr = __construct_new_array(arr, (void*)__ct__6CBpTexFv, (void*)__dt__6CBpTexFv, 0x28, 9);
    }
    playerTextures = arr;
    int* vals = names.vals;
    for (int i = 0; i < 9; i++) {
        void* obj = (char*)playerTextures + i * 0x28;
        void** vt = *(void***)obj;
        ((void (*)(void*, int))vt[3])(obj, vals[i]);
    }
    completion = WOverlay_Create(g_resourceManager,
                                 (const char*)(golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, complete_gsr)), 0);
    scoreEvent = WOverlay_Create(g_resourceManager,
                                 (const char*)(golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, score_event_gsr)), 0);
}

// Releases the hole-intro labels, player panels, and array of nine player textures.
CHoleInfo::~CHoleInfo() {
    HoleResourceFields* self = (HoleResourceFields*)this;
    void* p = ((HoleOverlayResources*)self)->courseName;
    self->vtable = __vt__9CHoleInfo;
    if (p) {
        OverlayPlayer_Destroy(p, 1);
        ((HoleOverlayResources*)self)->courseName = 0;
    }
    if (((HoleOverlayResources*)self)->alternateCourseName) {
        OverlayPlayer_Destroy(((HoleOverlayResources*)self)->alternateCourseName, 1);
        ((HoleOverlayResources*)self)->alternateCourseName = 0;
    }
    if (((HoleOverlayResources*)self)->holeAndPar) {
        OverlayPlayer_Destroy(((HoleOverlayResources*)self)->holeAndPar, 1);
        ((HoleOverlayResources*)self)->holeAndPar = 0;
    }
    if (((HoleOverlayResources*)self)->turnOrder) {
        OverlayPlayer_Destroy(((HoleOverlayResources*)self)->turnOrder, 1);
        ((HoleOverlayResources*)self)->turnOrder = 0;
    }
    if (((HoleOverlayResources*)self)->optionalOverlayAC) {
        OverlayPlayer_Destroy(((HoleOverlayResources*)self)->optionalOverlayAC, 1);
        ((HoleOverlayResources*)self)->optionalOverlayAC = 0;
    }
    if (((HoleOverlayResources*)self)->playerTextures) {
        __destroy_new_array(((HoleOverlayResources*)self)->playerTextures, __dt__6CBpTexFv);
        ((HoleOverlayResources*)self)->playerTextures = 0;
    }
    if (g_resourceManager && ((HoleOverlayResources*)self)->completion) {
        Object_DeleteVirtual(g_resourceManager, ((HoleOverlayResources*)self)->completion);
        ((HoleOverlayResources*)self)->completion = 0;
    }
    if (g_resourceManager && ((HoleOverlayResources*)self)->scoreEvent) {
        Object_DeleteVirtual(g_resourceManager, ((HoleOverlayResources*)self)->scoreEvent);
        ((HoleOverlayResources*)self)->scoreEvent = 0;
    }
    if (self) {
        self->vtable = cInfoBaseVtable;
        if (self->field04) {
            ResourceManager_DeleteResource(g_resourceManager, self->field04);
            self->field04 = 0;
        }
        if (self->field14) {
            __dla__FPv(self->field14);
            self->field14 = 0;
        }
    }
}

// Fades in the hole intro, moves it into the corner, dims it, or flashes it.
void CHoleInfo::UpdateIntroAnimation(float amount) {
    HoleAnimationFields* self = (HoleAnimationFields*)this;
    // Phases: fade course text in, move the intro toward its corner, dim it,
    // then idle or flash according to animationState. Timers advance by amount.
    switch (self->animationState) {
    case kHoleFadeIn: {
        volatile HoleAnimationFields* pose = self;
        float time = pose->animationTime + amount;
        float top = kFloat_255_MessageUI;
        pose->animationTime = time;
        float alpha = pose->courseOpacity + top * amount;
        pose->courseOpacity = alpha;
        if (alpha > top)
            pose->courseOpacity = top;
        alpha = pose->playerOpacity + top * amount;
        pose->playerOpacity = alpha;
        if (alpha > top)
            pose->playerOpacity = top;
        float limit = kFloat_4_FlyInfo;
        if (self->animationTime > limit) {
            float zero = kFloat_0_hud;
            self->animationState = kHoleMoveToCorner;
            self->animationTime = zero;
        }
        break;
    }
    case kHoleDim: {
        self->courseOpacity -= kFloat_128_Hud * amount;
        if (self->courseOpacity < kHoleIntroDimmedOpacity) {
            self->courseOpacity = kHoleIntroDimmedOpacity;
            self->animationState = kHoleIdle;
        }
        break;
    }
    case kHoleMoveToCorner: {
        float zero = kFloat_0_hud;
        float value, speed;
        speed = kFloat_510_Hud;
        value = self->playerOpacity;
        float next = value - speed * amount;
        self->playerOpacity = next;
        if (next < zero)
            self->playerOpacity = zero;
        self->animationTime = kFlightSecondaryPinFadeStartTime * amount + self->animationTime;
        if (self->animationTime > kFloat_1_805DE314) {
            self->animationTime = kFloat_1_805DE314;
            self->animationState = kHoleDim;
        }
        float inv, t;
        float one = *(volatile const float*)&kFloat_1_805DE314;
        volatile HoleAnimationFields* pose = self;
        t = pose->animationTime;
        inv = one - t;
        float scale = kFloat_0_6_805DE33C;
        float position = inv * pose->courseStartX + t * pose->courseEndX;
        pose->courseX = position;
        {
            float inv, t;
            t = pose->animationTime;
            inv = one - t;
            float position = inv * pose->courseStartY + t * pose->courseEndY;
            pose->courseY = position;
        }
        {
            float t = pose->animationTime;
            float height = t * scale + (one - t);
            pose->courseScale = height;
        }
        {
            float inv, t;
            t = pose->animationTime;
            inv = one - t;
            float position = inv * pose->holeStartX + t * pose->holeEndX;
            pose->holeX = position;
        }
        {
            float inv, t;
            t = pose->animationTime;
            inv = one - t;
            float position = inv * pose->holeStartY + t * pose->holeEndY;
            pose->holeY = position;
        }
        {
            float t = pose->animationTime;
            float height = t * scale + (one - t);
            pose->holeScale = height;
        }
        {
            float inv, t;
            t = pose->animationTime;
            inv = one - t;
            float position = inv * pose->playerStartX + t * pose->playerEndX;
            pose->playerX = position;
        }
        {
            float inv, t;
            t = pose->animationTime;
            inv = one - t;
            float position = inv * pose->playerStartY + t * pose->playerEndY;
            pose->playerY = position;
        }
        {
            float t = pose->animationTime;
            float height = t * scale + (one - t);
            pose->playerScale = height;
        }
        break;
    }
    case kHoleFlash: {
        self->animationTime += amount;
        if (self->animationTime > kFloat_0_25_HudShared) {
            float zero, alpha, top, next;
            next = self->animationTime - kFloat_0_25_HudShared;
            top = kFloat_255_MessageUI;
            alpha = self->courseOpacity;
            zero = kFloat_0_hud;
            float newAlpha = top - alpha;
            self->animationTime = next;
            self->courseOpacity = newAlpha;
            if (newAlpha > zero && ++self->flashCount == 3)
                self->animationState = kHoleIdle;
        }
        break;
    }
    }
}

// Starts the hole-intro flashing animation.
void CHoleInfo::StartFlash() {
    HoleFlashFields* self = (HoleFlashFields*)this;
    self->animationState = 4;
    self->flashCount = 0;
}

// Restarts the hole intro and selects the first eligible player for its order panel.
void CHoleInfo::ResetIntroAndSelectPlayer() {
    HoleResetFields* self = (HoleResetFields*)this;
    float playerY, playerX, holeY, holeX, one, courseY, courseX, zero;
    zero = kFloat_0_hud;
    one = kFloat_1_805DE314;
    courseX = self->courseStartX;
    courseY = self->courseStartY;
    holeX = self->holeStartX;
    holeY = self->holeStartY;
    playerX = self->playerStartX;
    playerY = self->playerStartY;
    self->animationTime = zero;
    self->animationState = 0;
    self->courseOpacity = zero;
    self->playerOpacity = zero;
    self->courseX = courseX;
    self->courseY = courseY;
    self->courseScale = one;
    self->holeX = holeX;
    self->holeY = holeY;
    self->holeScale = one;
    self->playerX = playerX;
    self->playerY = playerY;
    self->playerScale = one;
    self->selectedPlayer = 0;
    // Fade-in starts from the centered positions at full scale and zero alpha.
    // Selection defaults to player 0 if no eligible rank-1 snapshot is found.
    // Both player lookups are retained: state 3 is excluded before rankFlags.
    unsigned char i = 0;
    while (i < AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount) {
        void* p = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), i);
        if (*(int*)((char*)p + 8) != 3) {
            void* q = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), i);
            if ((((unsigned int)((GolfDocSnapshot8013*)q)->rankFlags >> 4) & 0xF) == 1) {
                self->selectedPlayer = i;
                break;
            }
        }
        i++;
    }
}

// Places the hole labels directly at their final corner positions.
void CHoleInfo::ShowCornerLabels() {
    // This is the immediate counterpart of MoveToCorner: final positions,
    // 0.6 scale, both opacities at 255, idle state; no player re-selection.
    unsigned int idle = kHoleIdle;
    float scale = kFloat_0_6_805DE33C;
    float opacity = kFloat_255_MessageUI;
    float zero = kFloat_0_hud;
    float courseXPosition = courseEndX;
    float courseYPosition = courseEndY;
    float holeXPosition = holeEndX;
    float holeYPosition = holeEndY;
    float playerXPosition = playerEndX;
    float playerYPosition = playerEndY;
    animationTime = zero;
    animationState = idle;
    courseOpacity = opacity;
    playerOpacity = opacity;
    courseX = courseXPosition;
    courseY = courseYPosition;
    courseScale = scale;
    holeX = holeXPosition;
    holeY = holeYPosition;
    holeScale = scale;
    playerX = playerXPosition;
    playerY = playerYPosition;
    playerScale = scale;
}
// Hole display and Pang information.

struct PangPlayerTotalsFields {
    char pad00[0x24];
    void* countingSoundAlias;
    char pad28[0x8];
    void* startingPang;
    void* startingBonusPang;
    void* settledPang;
    void* settledBonusPang;
    char pad40[0x8];
    void* pangAnimationState;
    void* displayedPang;
    char pad50[0x28];
    void* displayedBonusPang;
};

struct PangScreenPositionFields {
    char pad00[0x58];
    float pangBaselineY;
    char pad5C[0x28];
    float bonusPangBaselineY;
    char pad88[0x394];
    float playerPanelPositions[4][2];
};

struct HudRect {
    float x, y, w, h;
};
struct PangCounterSprite {
    int entryOrder;
    char reserved_04[0xC];
    HudRect uv;
    unsigned char opacity;
    char reserved_21[0xB];
};
struct BonusSpriteFields {
    HudRect uv;
    char rest[0x1C];
};
struct HudInitVTable {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void setup(const char*, int, int);
    virtual void slot6();
    virtual void update();
};

struct HoleVertex {
    float x, y, z;
    unsigned char r, g, b, a;
    float u, v;
};
struct HoleQuad {
    HoleVertex p[4];
};
struct HoleItem {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void draw();
    virtual int ready();
    char pad[0x24];
};

#pragma opt_lifetimes off
// Draws course/hole/par and the mode-specific player order or versus panel.
void CHoleInfo::DrawCourseAndPlayerPanels() {
    // Phases: draw course-side labels while courseAlpha is positive, then
    // compose player-side panels/textures while playerAlpha is positive.
    // The branches have independent opacity and generate screen-space quads.
    HudRect uv;
    HoleQuad* firstQuad;
    HoleQuad* secondQuad;
    char* vertexData = (char*)courseScoreBonusPang;

    if (courseOpacity > kFloat_0_hud) {
        const char* course = (const char*)AsCSharedDoc(g_sharedDoc)->selectedCourseData;
        void* ui = (int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 3 ? alternateCourseName : courseName;
        if (ui) {
            ((HudTextFields*)ui)->color = (Runtime_ConvertDoubleToUnsigned(courseOpacity) << 24) | kRgbWhite;
            if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 3)
                NamedText_DrawAlignedScaled(ui, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, PartyCourseName)),
                                            0, courseX, courseY, courseScale);
            else
                NamedText_DrawAlignedScaled(ui, course, 0, courseX, courseY, courseScale);
        }
        ui = holeAndPar;
        if (ui) {
            ((HudTextFields*)ui)->color = (Runtime_ConvertDoubleToUnsigned(courseOpacity) << 24) | kRgbWhite;
            NamedText_DrawAlignedScaled(holeAndPar, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, Hole)), 0,
                                        holeX, holeY, holeScale);
            float scale = holeScale;
            NamedText_DrawAlignedScaled(holeAndPar, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, Par)), 0,
                                        kFloat_105_GolfHud * scale + holeX, holeY, scale);
        }
        color = (Runtime_ConvertDoubleToUnsigned(courseOpacity) << 24) | kRgbWhite;
        if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 3) {
            void* scene = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);

            sprintf((char*)(textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                    (PartyCourseTable_FindCourseHolePosition(
                        ((HudPartyCourseOrderFields*)g_sharedDoc)->courseIds[AsCGolfDoc(scene)->currentHoleIndex],
                        ((HudHoleNumberFields*)scene)->displayedHoleNumber)));
        } else {
            void* scene = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
            sprintf((char*)(textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                    ((HudHoleNumberFields*)scene)->displayedHoleNumber);
        }
        float scale = holeScale;
        HudFont_DrawAlignedText(this, textBuffer, kFloat_63_GolfHud * scale + holeX, kFloat_4_FlyInfo * scale + holeY,
                                kFloat_1_805DE314, scale, 0, 0);
        sprintf((char*)(textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, slash)));
        scale = holeScale;
        HudFont_DrawAlignedText(this, textBuffer, kFloat_96_GolfHud * scale + holeX, kFloat_4_FlyInfo * scale + holeY,
                                kFloat_0_hud, scale, 0, 0);
        void* scene = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        unsigned char* holes = (unsigned char*)AsCGolfDoc(scene)->holeRecords;
        sprintf((char*)(textBuffer), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                holes[*((unsigned char*)&AsCGolfDoc(scene)->currentHoleIndex) * 32 + 0x18]);
        scale = holeScale;
        HudFont_DrawAlignedText(this, textBuffer, kFloat_160_GolfHud * scale + holeX, kFloat_4_FlyInfo * scale + holeY,
                                kFloat_0_hud, scale, 0, 0);
    }
    // Player panel texture banks require selectedPlayer in 0..3; rule-1 text
    // calls require a live playerUI even though its color setup is conditional.
    // Player-panel admission is separate from course labels above:
    //   alpha<=0: omit; tutorialStage!=6: return after course labels;
    //   first hole AND doc+3CC==0: optional completion/record overlays only;
    //   later hole OR doc+3CC!=0: require all nine texture ready results' bit 0.
    // The +3CC selector's gameplay meaning is unresolved. It switches first-hole
    // overlay policy versus texture panels here; no broader meaning is assumed.
    if (playerOpacity > kFloat_0_hud) {
        void* scene = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (AsCGolfDoc(scene)->tutorialStage != 6)
            return;
        color = ((int)playerOpacity << 24) + kRgbWhite;
        // Completion overlay requires non-idle intro, gameMode 2..4, first hole
        // and +3CC clear. Record overlay/text additionally need 18 holes, rule 0.
        // This gate suppresses ONLY these overlays; it does not return early.
        if (animationState != kHoleIdle) {
            if ((unsigned int)(((int)AsCSharedDoc(g_sharedDoc)->gameMode) - 2) <= 2) {
                scene = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
                if (!AsCGolfDoc(scene)->unk_3CC) {
                    if (!*((unsigned char*)&AsCGolfDoc(scene)->currentHoleIndex)) {
                        alignment = 0;
                        if (completion) {
                            HudRect uv;
                            void* camera = g_view;

                            HudRect rect;
                            float half = kFloat_0_5_hud;
                            float width = AsWView(camera)->viewportWidth;
                            rect.x = half * width - kFloat_106_GolfHud;
                            rect.y = kFloat_202_GolfHud;
                            rect.w = kFloat_256_MessageUI;
                            rect.h = kFloat_32_0;
                            uv.x = kFloat_0_hud;
                            uv.y = kFloat_0_hud;
                            uv.w = kFloat_1_805DE314;
                            uv.h = kFloat_1_805DE314;
                            WOverlay_DrawTransformedRect(
                                completion, camera, &uv, kFloat_0_hud, &rect, 0,
                                (Runtime_ConvertDoubleToUnsigned(playerOpacity) << 24) | kRgbWhite, 0);
                        }
                        if (*((unsigned char*)&AsCSharedDoc(g_sharedDoc)->roundHoleCount) == 0x12) {
                            if (!*((unsigned char*)&AsCSharedDoc(g_sharedDoc)->golfRuleType)) {
                                if (scoreEvent) {
                                    HudRect uv;
                                    void* camera = g_view;

                                    HudRect rect;
                                    float half = kFloat_0_5_hud;
                                    float width = AsWView(camera)->viewportWidth;
                                    rect.x = half * width - kFloat_106_GolfHud;
                                    rect.y = kFloat_242_HoleInfo;
                                    rect.w = kFloat_256_MessageUI;
                                    rect.h = kFloat_72_Hud;
                                    uv.x = kFloat_0_hud;
                                    uv.y = kFloat_0_hud;
                                    uv.w = kFloat_1_805DE314;
                                    uv.h = kFloat_1_805DE314;
                                    float recordOpacity = playerOpacity;
                                    WOverlay_DrawTransformedRect(
                                        scoreEvent, camera, &uv, kFloat_0_hud, &rect, 0,
                                        (Runtime_ConvertDoubleToUnsigned(recordOpacity) << 24) | kRgbWhite, 0);
                                }
                                if (((int&)GetSaveDataSlot()) == 0) {
                                    if (0 == s_saveDataInitialized) {
                                        SaveData_Ctor(s_saveData);
                                        s_saveDataInitialized = 1;
                                    }
                                    ((int&)GetSaveDataSlot()) = (int)s_saveData;
                                }
                                void* manager = (void*)((int&)GetSaveDataSlot());
                                int offset = (unsigned char)SharedDoc_GetCourseId() * 8;
                                short value = *(short*)((char*)SaveData_GetCurrentRecord(manager) + offset + 0x260);
                                char* text;
                                if (value != 0x7F)
                                    text = String_FormatTemporary(
                                        (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)), value);
                                else
                                    text = (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, unavailable));
                                float half = kFloat_0_5_hud;
                                float width = AsWView(g_view)->viewportWidth;
                                HudFont_DrawAlignedText(this, text, half * width + kFloat_100_hud, kFloat_242_HoleInfo,
                                                        kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
                            }
                        }
                    }
                }
            }
        }
        scene = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (AsCGolfDoc(scene)->unk_3CC || *((unsigned char*)&AsCGolfDoc(scene)->currentHoleIndex)) {
            // Poll all nine; bitwise AND deliberately does not short-circuit.
            // A failed result returns before render state or any panel is drawn.
            unsigned int allReady = 1;
            for (int i = 0; i < 9; ++i) {
                HoleItem* item = (HoleItem*)playerTextures + i;
                allReady &= item->ready();
            }
            if (!allReady)
                return;
            UIRender_SetDefaultState();
            uv.y = kFloat_0_hud;
            uv.x = kFloat_0_hud;
            uv.w = kHudScalar0_8125;
            uv.h = kHudScalar0_6875;
            Engine_SetFlexibleVertexFormat(0x11F005);
            void* panel = turnOrder;
            if (panel)
                ((HudTextFields*)panel)->color = (Runtime_ConvertDoubleToUnsigned(playerOpacity) << 24) | kRgbWhite;
            // Texture selection table (player texture array has nine elements):
            // rule 0, gameMode 1 or party mode: selected player, or player+4
            //     when SharedDoc_IsPlayerFlag0Set; one quad, template at +468.
            // rule 1, gameMode 0/6: slots 0 and 8; two quads, template +4C8.
            // rule 1, gameMode 1: i or i+4 for i=0,1; template +528.
            // other rule/mode combinations submit no player quads.
            // 0..3/4..7 are the two per-player banks; slot 8 is used only by
            // the second 0/6 panel here. No texture artwork identity is inferred.
            // Quad copies preserve template Z/RGB; write XY, alpha, normalized
            // UVs (0..0.8125,0..0.6875), then submit four 24-byte vertices.
            // Keep per-branch floating add operand order and duplicate stores.
            unsigned char mode = *((unsigned char*)&AsCSharedDoc(g_sharedDoc)->golfRuleType);
            if (0 == mode) {
                if (AsCSharedDoc(g_sharedDoc)->gameMode == 1 || ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode)) {
                    if (turnOrder) {
                        float half = kFloat_0_5_hud;
                        float width = AsWView(g_view)->viewportWidth;
                        NamedText_DrawAlignedScaled(
                            turnOrder, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, leaderLabel)), 0,
                            (half * width - kFloat_45_Hud) - kFloat_60_Hud, kFloat_160_GolfHud, kFloat_1_805DE314);
                    }
                    if (!SharedDoc_IsPlayerFlag0Set(selectedPlayer)) {
                        HoleItem* item = (HoleItem*)playerTextures + selectedPlayer;
                        item->draw();
                    } else {
                        HoleItem* item = (HoleItem*)playerTextures + (selectedPlayer + 4);
                        item->draw();
                    }
                    float half = kFloat_0_5_hud;
                    float width = AsWView(g_view)->viewportWidth;
                    float x = half * width + kFloat_35_Hud;
                    float y = kFloat_162_Hud;
                    HoleQuad quad = *(HoleQuad*)(vertexData + 0x468);
                    quad.p[0].x = x;
                    quad.p[0].y = y;
                    quad.p[0].a = (int)playerOpacity;
                    quad.p[0].u = uv.x;
                    quad.p[0].v = uv.y;
                    quad.p[1].x = kDriveHudLabelWidth + x;
                    quad.p[1].y = y;
                    float leaderOpacity = playerOpacity;
                    quad.p[1].u = uv.w;
                    quad.p[1].a = (int)leaderOpacity;
                    quad.p[1].v = uv.y;
                    quad.p[2].x = x;
                    quad.p[2].y = kDriveHudLabelHeight + y;
                    quad.p[2].a = (int)playerOpacity;
                    quad.p[2].u = uv.x;
                    quad.p[2].v = uv.h;
                    quad.p[3].x = (x + kDriveHudLabelWidth);
                    quad.p[3].y = (kDriveHudLabelHeight + y);
                    quad.p[3].a = (int)playerOpacity;
                    quad.p[3].u = uv.w;
                    quad.p[3].v = uv.h;
                    DrawPrimitiveUP(0x98, (int)GX_GetPrimitiveCount(4, 0x98), &quad, 0x18);
                }
            } else if (1 == mode) {
                float h, w, right, gap, center, half, y;
                half = kFloat_0_5_hud;
                center = kFloat_15_Hud;
                gap = kFloat_32_0;
                firstQuad = (HoleQuad*)(vertexData + 0x4C8);
                secondQuad = (HoleQuad*)(vertexData + 0x528);
                right = kFloat_5_Hud;
                y = kFloat_164_Hud;
                w = kDriveHudLabelWidth;
                h = kDriveHudLabelHeight;
                for (int i = 0; 2 > i; ++i) {
                    unsigned int screen = AsCSharedDoc(g_sharedDoc)->gameMode;
                    if (!screen || screen == 6) {
                        if (i == 0) {
                            NamedText_DrawAlignedScaled(turnOrder,
                                                        (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, vs)), 0,
                                                        (half * AsWView(g_view)->viewportWidth + center) - gap,
                                                        kFloat_161_GolfHud, kFloat_1_805DE314);
                            void* player = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), 0);

                            sprintf((char*)(textBuffer),
                                    (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                                    HudPlayer_ScoreToPar(player));
                            float textY = kFloat_164_Hud;
                            float width = AsWView(g_view)->viewportWidth;
                            HudFont_DrawAlignedText(this, textBuffer, ((half * width + center) - gap) - center, textY,
                                                    kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
                            player = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), 1);
                            sprintf((char*)(textBuffer),
                                    (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                                    HudPlayer_ScoreToPar(player));
                            textY = kFloat_164_Hud;
                            width = AsWView(g_view)->viewportWidth;
                            HudFont_DrawAlignedText(this, textBuffer, (gap + (half * width + center)) - right, textY,
                                                    kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
                        }
                        if (!i) {
                            HoleItem* item = (HoleItem*)playerTextures;
                            item->draw();
                        } else {
                            HoleItem* item = (HoleItem*)playerTextures + 8;
                            item->draw();
                        }
                        float offset;
                        if (i == 0)
                            offset = kFloat_neg112_Hud;
                        else
                            offset = kFloat_53_Hud;
                        float x = offset + (half * AsWView(g_view)->viewportWidth + center);
                        HoleQuad quad = *firstQuad;
                        quad.p[0].x = x;
                        quad.p[0].y = y;
                        quad.p[0].a = (int)playerOpacity;
                        quad.p[0].u = uv.x;
                        quad.p[0].v = uv.y;
                        quad.p[1].x = w + x;
                        quad.p[1].y = y;
                        quad.p[1].a = (int)playerOpacity;
                        quad.p[1].u = uv.w;
                        quad.p[1].v = uv.y;
                        quad.p[2].x = x;
                        quad.p[2].y = h + y;
                        float versusOpacity = playerOpacity;
                        quad.p[2].u = uv.x;
                        quad.p[2].a = (int)versusOpacity;
                        quad.p[2].v = uv.h;
                        quad.p[3].x = w + x;
                        quad.p[3].y = (h + y);
                        quad.p[3].a = (int)playerOpacity;
                        quad.p[3].u = uv.w;
                        quad.p[3].v = uv.h;
                        DrawPrimitiveUP(0x98, (int)GX_GetPrimitiveCount(4, 0x98), &quad, 0x18);
                    } else if (screen == 1) {
                        if (i == 0) {
                            NamedText_DrawAlignedScaled(turnOrder,
                                                        (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, vs)), 0,
                                                        (half * AsWView(g_view)->viewportWidth + center) - gap,
                                                        kFloat_161_GolfHud, kFloat_1_805DE314);
                            void* player = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), 0);
                            sprintf((char*)(textBuffer),
                                    (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                                    HudPlayer_ScoreToPar(player));
                            float textY = kFloat_164_Hud;
                            float width = AsWView(g_view)->viewportWidth;
                            HudFont_DrawAlignedText(this, textBuffer, ((half * width + center) - gap) - center, textY,
                                                    kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
                            player = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), 1);
                            sprintf((char*)(textBuffer),
                                    (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                                    HudPlayer_ScoreToPar(player));
                            textY = kFloat_164_Hud;
                            width = AsWView(g_view)->viewportWidth;
                            HudFont_DrawAlignedText(this, textBuffer, (gap + (half * width + center)) - right, textY,
                                                    kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
                        }
                        if (!SharedDoc_IsPlayerFlag0Set(i)) {
                            HoleItem* item = (HoleItem*)playerTextures + i;
                            item->draw();
                        } else {
                            HoleItem* item = (HoleItem*)playerTextures + i + 4;
                            item->draw();
                        }
                        float offset;
                        if (0 == i)
                            offset = kFloat_neg112_Hud;
                        else
                            offset = kFloat_53_Hud;
                        float x = offset + (half * AsWView(g_view)->viewportWidth + center);
                        HoleQuad quad = *secondQuad;
                        float xValue2 = x;
                        quad.p[0].x = xValue2;
                        quad.p[0].y = y;
                        quad.p[0].a = (int)playerOpacity;
                        quad.p[0].u = uv.x;
                        quad.p[0].v = uv.y;
                        float xValue3 = x;
                        float wValue2 = w;
                        quad.p[1].x = wValue2 + xValue3;
                        quad.p[1].y = y;
                        quad.p[1].a = (int)playerOpacity;
                        quad.p[1].u = uv.w;
                        quad.p[1].v = uv.y;
                        quad.p[2].x = x;
                        quad.p[2].y = h + y;
                        quad.p[2].a = (int)playerOpacity;
                        quad.p[2].u = uv.x;
                        quad.p[2].v = uv.h;
                        float wValue3 = w;
                        quad.p[3].x = x + wValue3;
                        quad.p[3].y = (h + y);
                        quad.p[3].a = (int)playerOpacity;
                        quad.p[3].u = uv.w;
                        quad.p[3].v = uv.h;
                        DrawPrimitiveUP(0x98, (int)GX_GetPrimitiveCount(4, 0x98), &quad, 0x18);
                    }
                }
            }
        }
    }
}

#pragma opt_lifetimes reset

// Initializes the caller's Pang HUD, allocates subordinate resources, and initializes HUD amounts.
CPangInfo::CPangInfo() {
    vtbl = __vt__9CPangInfo;
    ((HudInitVTable*)this)->setup((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, largeFont)), 0, 0);
    alignment = 2;
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
    ((HudInitVTable*)this)->update();
    textRenderer =
        WOverlay_Create(g_resourceManager, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, pang_bonus_gsr)), 0);
    displayMode = 0;
    HudRect uv;
    uv.x = kFloat_0_hud;
    uv.y = kFloat_0_hud;
    uv.w = kFloat_0_5_hud;
    uv.h = kFloat_0_0625_Hud;
    countingSoundAlias = 0;
    struct Items {
        char pad[0xB8];
        BonusSpriteFields p[19];
    };
    for (int i = 0; i < 9; ++i) {
        uv.y = kFloat_32_0 * (float)i;
        uv.y *= kBonusUvRowStep;
        ((Items*)this)->p[i].uv = uv;
    }
    uv.x = kFloat_0_5_hud;
    for (int i = 9; i < kBonusEntryCount; ++i) {
        uv.y = kFloat_32_0 * (float)(i - 9);
        uv.y *= kBonusUvRowStep;
        ((Items*)this)->p[i].uv = uv;
    }
    uv.y = kBonusUvSecondSheetY;
    for (int i = 0; i < 2; ++i) {
        // Layout-derived offsets preserve this compiler's stores; a typed counter-array loop differs.
        enum {
            counterStride = sizeof(PangCounterSprite)
        };
        *(int*)((char*)this + IFF_OFFSET(CPangInfo, pangEntryOrder) + i * counterStride) = -1;
        *(HudRect*)((char*)this + IFF_OFFSET(CPangInfo, pangUvX) + i * counterStride) = uv;
        *((unsigned char*)this + IFF_OFFSET(CPangInfo, pangOpacity) + i * counterStride) = 255;
        *(int*)(((char*)&earnedPang) + i * 4) = 0;
        *(int*)(((char*)&startingPang) + i * 4) = 0;
        *(int*)(((char*)&pangBeforeMultiplier) + i * 4) = 0;
        uv.y -= kFloat_0_0625_Hud;
    }
    flags = 0;
    holeInOneBannerElapsed = 0.0f;
    ballBonusBannerElapsed = 0.0f;
    completionDelayElapsed = 0.0f;
    InitScreenPositions();
    counting = false;
    memset(playerValues, 0, sizeof(playerValues));
}

// Default reset hook for numeric fonts; intentionally empty.
void CFontNumDraw::vfunc_5() {}

// Releases the pang font, reward overlays, and numeric text storage.
CPangInfo::~CPangInfo() {
    vtbl = __vt__9CPangInfo;
    if (g_resourceManager != 0 && textRenderer != 0) {
        Object_DeleteVirtual(g_resourceManager, textRenderer);
        textRenderer = 0;
    }
    if (this != 0) {
        void* p = fontResource;
        vtbl = cInfoBaseVtable;
        if (p != 0) {
            ResourceManager_DeleteResource(g_resourceManager, p);
            fontResource = 0;
        }
        if (textBuffer != 0) {
            __dla__FPv(textBuffer);
            textBuffer = 0;
        }
    }
}

// Positions pang totals and mode-specific player reward panels.
void CPangInfo::InitScreenPositions() {
    PangScreenPositionFields* self = (PangScreenPositionFields*)this;
    float f0, f1, f2;
    const void* r4;
    f0 = kFloat_108_hud;
    f1 = kFloat_20_MessageUI;
    r4 = g_option;
    f2 = AsCOption(r4)->hudBaselineY;
    f2 = f2 - f0;
    f1 = f1 + f2;
    f0 = kFloat_3_Hud;
    f0 = f0 + f1;
    self->pangBaselineY = f0;
    self->bonusPangBaselineY = f0;
    float width = AsWView(g_view)->viewportWidth;
    unsigned char mode = *((unsigned char*)&AsCSharedDoc(g_sharedDoc)->golfRuleType);
    if (mode == 0) {
        float a[2] = {kFloat_neg25_Hud, kFloat_48_Hud};
        Vec2_Copy(self->playerPanelPositions[0], a);
        float b[2] = {kFloat_neg25_Hud, kFloat_141_GolfHud};
        Vec2_Copy(self->playerPanelPositions[1], b);
        float c[2] = {width - kFloat_186_Hud, kFloat_48_Hud};
        Vec2_Copy(self->playerPanelPositions[2], c);
        float d[2] = {width - kFloat_186_Hud, kFloat_141_GolfHud};
        Vec2_Copy(self->playerPanelPositions[3], d);
    } else if (mode == 1) {
        float a[2] = {kFloat_neg25_Hud, kFloat_51_Hud};
        Vec2_Copy(self->playerPanelPositions[0], a);
        float b[2] = {kFloat_neg25_Hud, kFloat_100_hud};
        Vec2_Copy(self->playerPanelPositions[1], b);
        float c[2] = {width - kFloat_186_Hud, kFloat_51_Hud};
        Vec2_Copy(self->playerPanelPositions[2], c);
        float d[2] = {width - kFloat_186_Hud, kFloat_100_hud};
        Vec2_Copy(self->playerPanelPositions[3], d);
    }
}

// Stops the count sound, snapshots current player totals, and resets reward animation.
void CPangInfo::ResetFromCurrentPlayer() {
    PangPlayerTotalsFields* self = (PangPlayerTotalsFields*)this;
    if (self->countingSoundAlias != 0)
        Sound_StopByAlias(g_soundManager, self->countingSoundAlias);
    void* scene = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    void* result = GolfPlayer_GetEntryByIndex(scene, AsCGolfDoc(scene)->currentPlayer);
    void* value = (void*)((PlayerPangTotalsFields*)result)->pangTotal;
    self->displayedPang = value;
    self->settledPang = value;
    self->startingPang = value;
    scene = ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    result = GolfPlayer_GetEntryByIndex(scene, AsCGolfDoc(scene)->currentPlayer);
    value = (void*)((PlayerPangTotalsFields*)result)->bonusPangTotal;
    self->displayedBonusPang = value;
    self->settledBonusPang = value;
    self->startingBonusPang = value;
    ((CPangInfo*)((PangAnimationFields*)self))->ResetPangAndBonusAnimation(0, 1);
}
// Pang state updates.

// Projection for fields accessed directly by the frame orchestrator. Category
// fields not listed here use the shared CPangInfo layout; document/ball access
// uses CGolfDoc/CGolfBall, so no separate integer-pointer projections are needed.
struct PangFrameFields {
    char reserved00[0x20];
    int flags;
    char reserved24[0x7C];
    int longDriveState, longDrivePang;
    char reservedA8[4];
    float longDriveAnimationTime;
    char reservedB0[0x48];
    int approachState, approachPang;
    char reserved100[4];
    float approachAnimationTime;
    char reserved108[0x74];
    int pinContactBonusState;
    char reserved180[8];
    float pinContactAnimationTime;
    char reserved18C[0x48];
    int specialShotBonusState;
    char reserved1D8[8];
    float specialShotBonusTime;
    char reserved1E4[0x218];
    float clubDistanceYards, holeDistanceYards;
    float shotTravelDistanceYards, remainingDistanceYards;
    char reserved40C[4];
    int processedTrajectoryFrame;
};

struct RowV {
    int x;
    int y;
    int z;
};

// Updates shot rewards, bonus entries, count-up animation, and completion timing.
void CPangInfo::UpdateShotRewardsAndBonuses(float dt) {
    PangFrameFields* self = (PangFrameFields*)this;
    // Admission (no writes on rejection): tutorialStage is 5 or >=6; at least
    // one gameStateFlags bit in 0x81880 is set; this HUD has nonzero flags.
    // Round bit 0x1000 additionally requires golfRuleType==1. Bit gameplay names
    // are unresolved; these masks are policy shared with CPangInfo::Draw.
    // Distances refresh only with a valid trajectory cursor, bit 0x1000 clear,
    // and display incomplete: XZ displacement/3.2 for travel, XYZ/3.2 to cup.
    // Keep both current-sample lookups and the volatile cup-word loads in order.
    //
    // Transition table below (all exclude kBonusUnavailable):
    // category                 admission             state 1 / after entries settle
    // Pang multiplier          totals not settled    queue / accept iff either total>0
    // long-drive multiplier    totals not settled    queue / no final transition here
    // long drive               travel>club           queue / accept iff >club, no recovery
    // approach, pin, special   ball stopped          queue / accept (approach sets value)
    // early match win          golfRuleType==1       queue / accept
    // hole-in-one              ball stopped          queue / accept, value=500
    // ball                     lookup result !=0     queue / accept, value=10
    // power shot               totals not settled    queue / accept, doc+2F4 value
    // round completion         round bit80 clear OR stopped: queue / accept
    // Other categories delegate in the visible order; they can set flags/queue
    // positions before later categories. Each final transition preserves upper
    // state bits, replaces the low 12 bits and clears that entry's elapsed time.
    // Once totals settle, category work stops and the two banner clocks advance.
    // Finally pulse/fade timers advance and processedTrajectoryFrame is published.
    if ((AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->tutorialStage < 6 &&
         AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->tutorialStage != 5) ||
        !(AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags & kPangRoundAdmissionMask) || !self->flags)
        return;
    if ((AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags & kPangRoundRestrictedFlag) &&
        ((unsigned)AsCSharedDoc(g_sharedDoc)->golfRuleType) != 1)
        return;
    if (((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] >= 0 &&
        !(AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags & kPangRoundRestrictedFlag) &&
        !(self->flags & kPangDisplayComplete)) {
        void* launchPosition;
        int cupYBits, cupXBits;
        RowV cupPositionBits;
        float cupDelta[3];
        float travelDelta[3];
        float remainingVector[3];
        float horizontalTravelVector[3];
        CGolfBall* ball = (CGolfBall*)GolfBall__Fv();
        launchPosition = PreData__9CGolfBallFi(ball, 0);
        void* currentPosition = PreData__9CGolfBallFi(ball, ball->m_eventPreDataIndex[0]);
        __mi__FRC7WVectorRC7WVector(travelDelta, currentPosition, launchPosition);
        __as__7WVectorFRC7WVector(horizontalTravelVector, travelDelta);
        horizontalTravelVector[1] = kFloat_0_hud;
        CGolfDoc* play = AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc);
        int holeIndex = (unsigned char)play->currentHoleIndex;
        volatile int* holeWords = (int*)((char*)play->holeRecords + holeIndex * 32);
        cupXBits = holeWords[3];
        cupYBits = holeWords[4];
        cupPositionBits.x = cupXBits;
        cupPositionBits.y = cupYBits;
        cupPositionBits.z = holeWords[5];
        void* currentPositionForCup = PreData__9CGolfBallFi(ball, ball->m_eventPreDataIndex[0]);
        __mi__FRC7WVectorRC7WVector(cupDelta, &cupPositionBits, currentPositionForCup);
        __as__7WVectorFRC7WVector(remainingVector, cupDelta);
        self->shotTravelDistanceYards = Magnitude__7WVectorCFv(horizontalTravelVector) / kFloat_3_2_hud;
        self->remainingDistanceYards = Magnitude__7WVectorCFv(remainingVector) / kFloat_3_2_hud;
    }
    ((CPangInfo*)((PangAnimationFields*)self))->AdvanceRewardDisplayTimers(dt);
    ((CPangInfo*)((PangAnimationFields*)self))->AnimateRewardEntries(dt);
    ((CPangInfo*)((PangAnimationFields*)self))->UpdateSettlementFlags();
    ((CPangInfo*)((PangAnimationFields*)self))->UpdateCompletionDelay(dt);
    if (!(self->flags & kPangTotalsSettled)) {
        int x = AsCPangInfo(self)->pangMultiplierState;
        if (x != kBonusUnavailable) {
            if (x == kRewardUnscheduled)
                ((CPangInfo*)self)->ScheduleBonusEntry(kBonusPangMultiplier);
            else if (self->flags & kPangEntriesSettled) {
                int on = AsCPangInfo(self)->earnedPang > 0 || AsCPangInfo(self)->earnedBonusPang > 0;
                int state = on ? kBonusAccepted : kBonusRejected;
                AsCPangInfo(self)->pangMultiplierState = (x & ~kBonusStateMask) | state;
                AsCPangInfo(self)->pangMultiplierAnimationTime = kFloat_0_hud;
            }
        }
        if (AsCPangInfo(self)->longDriveMultiplierState != kBonusUnavailable &&
            AsCPangInfo(self)->longDriveMultiplierState == kRewardUnscheduled)
            ((CPangInfo*)self)->ScheduleBonusEntry(kBonusLongDriveMultiplier);
        if (self->longDriveState != kBonusUnavailable) {
            if (self->shotTravelDistanceYards > self->clubDistanceYards) {
                if (self->longDriveState == kRewardUnscheduled)
                    ((CPangInfo*)self)->ScheduleBonusEntry(kBonusLongDrive);
                self->longDrivePang = (int)(self->shotTravelDistanceYards - self->clubDistanceYards);
            }
            if (self->flags & kPangEntriesSettled) {
                if (self->shotTravelDistanceYards > self->clubDistanceYards &&
                    ((CGolfBall*)GolfBall__Fv())->m_recoveryRequired == 0) {
                    int oldLongDriveState = self->longDriveState;
                    float zero = kFloat_0_hud;
                    self->longDriveState = (oldLongDriveState & ~kBonusStateMask) | kBonusAccepted;
                    self->longDriveAnimationTime = zero;
                } else {
                    int oldLongDriveState = self->longDriveState;
                    float zero = kFloat_0_hud;
                    self->longDriveState = (oldLongDriveState & ~kBonusStateMask) | kBonusRejected;
                    self->longDriveAnimationTime = zero;
                }
            }
        }
        ((CPangInfo*)self)->UpdateConditionalBonusEntry(kBonusLongDriveMultiplier, 2);
        if (self->approachState != kBonusUnavailable && ((CGolfBall*)GolfBall__Fv())->m_state == 0) {
            if (self->approachState == kRewardUnscheduled) {
                ((CPangInfo*)self)->ScheduleBonusEntry(kBonusApproach);
                int count = (int)self->remainingDistanceYards;
                int step = AsCPangInfo(self)->scoreRelativeToPar;
                self->approachPang = -(step * (10 - count));
                if (self->holeDistanceYards < kFloat_60_Hud)
                    self->approachPang = (self->approachPang + 1) / 2;
            } else if ((self->flags & kPangEntriesSettled) != 0) {
                self->approachState = (self->approachState & ~kBonusStateMask) | kBonusAccepted;
                self->approachAnimationTime = kFloat_0_hud;
            }
        }
        ((CPangInfo*)self)->UpdateLongPuttBonus();
        ((CPangInfo*)self)->UpdateChipInBonus();
        if (self->pinContactBonusState != kBonusUnavailable && ((CGolfBall*)GolfBall__Fv())->m_state == 0) {
            if (self->pinContactBonusState == kRewardUnscheduled)
                ((CPangInfo*)self)->ScheduleBonusEntry(kBonusPinContact);
            else if ((self->flags & kPangEntriesSettled) != 0) {
                self->pinContactBonusState = (self->pinContactBonusState & ~kBonusStateMask) | kBonusAccepted;
                self->pinContactAnimationTime = kFloat_0_hud;
            }
        }
        if (self->specialShotBonusState != kBonusUnavailable && ((CGolfBall*)GolfBall__Fv())->m_state == 0) {
            if (self->specialShotBonusState == kRewardUnscheduled)
                ((CPangInfo*)self)->ScheduleBonusEntry(kBonusSpecialShot);
            else if ((self->flags & kPangEntriesSettled) != 0) {
                self->specialShotBonusState = (self->specialShotBonusState & ~kBonusStateMask) | kBonusAccepted;
                self->specialShotBonusTime = kFloat_0_hud;
            }
        }
        ((CPangInfo*)self)->UpdateRecoveryBonus();
        ((CPangInfo*)self)->UpdateObjectHitMultiplier();
        ((CPangInfo*)self)->UpdateScoreBonusState();
        ((CPangInfo*)self)->UpdateConditionalBonusEntry(kBonusPowerSpin, 2);
        ((CPangInfo*)self)->UpdateConditionalBonusEntry(kBonusPowerCurve, 2);
        if (AsCSharedDoc(g_sharedDoc)->golfRuleType == 1 &&
            AsCPangInfo(self)->matchEarlyWinBonusState != kBonusUnavailable) {
            if (AsCPangInfo(self)->matchEarlyWinBonusState == kRewardUnscheduled)
                ((CPangInfo*)self)->ScheduleBonusEntry(kBonusMatchEarlyWin);
            else if (self->flags & kPangEntriesSettled) {
                AsCPangInfo(self)->matchEarlyWinBonusState =
                    (AsCPangInfo(self)->matchEarlyWinBonusState & ~kBonusStateMask) | kBonusAccepted;
                AsCPangInfo(self)->matchEarlyWinBonusAnimationTime = kFloat_0_hud;
            }
        }
        if (((CGolfBall*)GolfBall__Fv())->m_state == 0) {
            if (AsCPangInfo(self)->holeInOneBonusState != kBonusUnavailable) {
                if (AsCPangInfo(self)->holeInOneBonusState == kRewardUnscheduled)
                    ((CPangInfo*)self)->ScheduleBonusEntry(kBonusHoleInOne);
                else if (self->flags & kPangEntriesSettled) {
                    AsCPangInfo(self)->holeInOneBonusPang = 500;
                    AsCPangInfo(self)->holeInOneBonusState =
                        (AsCPangInfo(self)->holeInOneBonusState & ~kBonusStateMask) | kBonusAccepted;
                    AsCPangInfo(self)->holeInOneBonusAnimationTime = kFloat_0_hud;
                }
            }
        }
        unsigned char* slot =
            (unsigned char*)g_sharedDoc + AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer * 0x1d4;
        // FindBallIndex (item_inventory_tables.cpp) returns 0..N-1 or -1, not a Boolean.
        // Preserve this truth test: index 0 fails; other indices AND -1 pass.
        // Do not turn this into a found/missing test as a readability edit.
        if (ItemInventory_FindBallIndex((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory,
                                        *(unsigned int*)(slot + 0x88))) {
            if (AsCPangInfo(self)->ballBonusState != kBonusUnavailable) {
                if (AsCPangInfo(self)->ballBonusState == kRewardUnscheduled)
                    ((CPangInfo*)self)->ScheduleBonusEntry(kBonusBall);
                else if (self->flags & kPangEntriesSettled) {
                    AsCPangInfo(self)->ballBonusPang = 10;
                    AsCPangInfo(self)->ballBonusState =
                        (AsCPangInfo(self)->ballBonusState & ~kBonusStateMask) | kBonusAccepted;
                    AsCPangInfo(self)->ballBonusAnimationTime = kFloat_0_hud;
                }
            }
        }
        ((CPangInfo*)self)->UpdateBallBounceBonus();
        if (AsCPangInfo(self)->powerShotBonusState != kBonusUnavailable) {
            if (AsCPangInfo(self)->powerShotBonusState == kRewardUnscheduled)
                ((CPangInfo*)self)->ScheduleBonusEntry(kBonusPowerShot);
            else if (self->flags & kPangEntriesSettled) {
                AsCPangInfo(self)->powerShotBonusPang =
                    AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->powerShotBonusPang;
                AsCPangInfo(self)->powerShotBonusState =
                    (AsCPangInfo(self)->powerShotBonusState & ~kBonusStateMask) | kBonusAccepted;
                AsCPangInfo(self)->powerShotBonusAnimationTime = kFloat_0_hud;
            }
        }
        if (AsCPangInfo(self)->roundCompletionBonusState != kBonusUnavailable &&
            (!(AsCGolfDoc(AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags & 0x80) ||
             ((CGolfBall*)GolfBall__Fv())->m_state == 0)) {
            if (AsCPangInfo(self)->roundCompletionBonusState == kRewardUnscheduled)
                ((CPangInfo*)self)->ScheduleBonusEntry(kBonusRoundCompletion);
            else if (self->flags & kPangEntriesSettled) {
                AsCPangInfo(self)->roundCompletionBonusState =
                    (AsCPangInfo(self)->roundCompletionBonusState & ~kBonusStateMask) | kBonusAccepted;
                AsCPangInfo(self)->roundCompletionBonusAnimationTime = kFloat_0_hud;
            }
        }
    } else {
        if (self->flags & kPangTotalsSettled)
            AsCPangInfo(self)->holeInOneBannerElapsed += dt;
        if (self->flags & kPangTotalsSettled)
            AsCPangInfo(self)->ballBonusBannerElapsed += dt;
    }
    char* p = (char*)self;
    for (int i = 0; i < kBonusEntryCount; ++i) {
        // Pulse-rise, pulse-fall and reject-fade phases consume this clock.
        if (AsCPangInfo(p)->bonusEntryState & (kRewardPulseRise | kRewardPulseFall | kRewardRejectFade))
            AsCPangInfo(p)->bonusEntryAnimationTime += dt;
        p += 0x2c;
    }
    self->processedTrajectoryFrame = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0];
}
// Pang result calculation.
typedef Vec3f Vec3;
typedef RewardPositionBits Words3;
struct Channel {
    int state;
    char pad[40];
};
struct Channels {
    char pad[0xA0];
    Channel channels[19];
};
struct Player {
    unsigned char pad0[0x58];
    unsigned char state;
    unsigned char pad59[0x0B];
    int f64;
};
struct State {
    unsigned char pad0[0x3B6];
    unsigned char player;
};

// Borrowed per-player settings: equipped ball and swing-control word share a 0x1D4 stride.
struct HudRewardPlayerSettings {
    char reserved00[0x74];
    unsigned int equippedBallId;
    char reserved78[0x150];
    int buttonSwingSelected;
    char reserved1CC[8];
};
struct HudRewardPlayerSettingsTable {
    char reserved00[0x14];
    HudRewardPlayerSettings players[4];
};
// The +0x9C6 selector enables the twofold reward branch; other values are unresolved.
struct HudRewardMultiplierInput {
    char reserved00[0x9C6];
    unsigned char selector;
};
// Ball outcome is encoded in two words eight bytes apart, with complementary bits.
struct HudRewardOutcomeWords {
    int first;
    char reserved04[4];
    int second;
};
typedef char HudRewardPlayerSettingsStride[sizeof(HudRewardPlayerSettings) == 0x1D4 ? 1 : -1];
typedef char HudRewardEquippedBallOffset[(IFF_OFFSET(HudRewardPlayerSettingsTable, players) +
                                              IFF_OFFSET(HudRewardPlayerSettings, equippedBallId) ==
                                          0x88)
                                             ? 1
                                             : -1];
typedef char HudRewardControlOffset[(IFF_OFFSET(HudRewardPlayerSettingsTable, players) +
                                         IFF_OFFSET(HudRewardPlayerSettings, buttonSwingSelected) ==
                                     0x1DC)
                                        ? 1
                                        : -1];
// Offset-derived byte views preserve the compiler's operand order for indexed records.
// The type declarations above own the layout; reward policy below uses named inputs.
// Borrow a player's stroke count for the current hole (unsigned packed byte).
#define RewardHoleStrokes(player)        \
    (*(unsigned char*)((char*)(player) + \
                       (IFF_OFFSET(RewardPlayerSummary, strokes) + AsCGolfDoc(RewardDocument())->currentHoleIndex)))
// Borrow the per-hole ground-type-2 putter-shot counter, not a Boolean status.
// Domain: packed unsigned byte 0..255; increment truncates modulo 256 (no clamp).
// CGolfRule::EnterState's shot branch and CGolfRule_InitializeBallSimulation
// (golf_rule_round_flow_and_shot_sim.cpp) increment player[0x3A+hole] when club is putter AND final groundType
// is 2. Shot checkpoints save/restore the same byte (+0x49 in history).
// CalculateShotRewards admits long-putt rewards only at count==0, with the
// separate distance/club/score/holed gates below; all nonzero counts exclude it.
// Ground type 2's gameplay label is not established here; preserve its code.
// Required currentHoleIndex [0,18), live score record; this is not CGolfPlayer.
#define RewardHoleStatus(player)                                                           \
    (*(unsigned char*)((char*)(player) + (IFF_OFFSET(RewardPlayerSummary, perHoleStatus) + \
                                          AsCGolfDoc(RewardDocument())->currentHoleIndex)))
// Borrow one equipped ball ID using the original word-load argument evaluation.
static inline unsigned int& RewardEquippedBallWord(void* storage, int byteOffset) {
    return *(unsigned int*)((char*)storage + byteOffset);
}
// Borrow one swing-control selection word; zero admits motion-control rewards.
static inline int& RewardControlWord(void* storage, int byteOffset) {
    return *(int*)((char*)storage + byteOffset);
}
#define RewardEquippedBallAt(shared, player)                                                 \
    RewardEquippedBallWord(shared, IFF_OFFSET(HudRewardPlayerSettingsTable, players) +       \
                                       IFF_OFFSET(HudRewardPlayerSettings, equippedBallId) + \
                                       (player) * sizeof(HudRewardPlayerSettings))
#define RewardControlAt(shared, player)                                                      \
    RewardControlWord(shared, IFF_OFFSET(HudRewardPlayerSettingsTable, players) +            \
                                  IFF_OFFSET(HudRewardPlayerSettings, buttonSwingSelected) + \
                                  (player) * sizeof(HudRewardPlayerSettings))
#pragma opt_common_subs reset
// Persisted shot categories live in the score record, independently of HUD animation state.
enum ShotRewardFlags {
    kRewardLongDrive = 1,
    kRewardObjectHit = 2,
    kRewardLongDriveDouble = 4,
    kRewardSpecialShot = 8,
    kRewardPinContact = 0x10,
    kRewardChipIn = 0x20,
    kRewardLongChipIn = 0x40,
    kRewardLongPutt = 0x80,
    kRewardScore = 0x100,
    kRewardApproach = 0x200,
    kRewardPowerSpin = 0x400,
    kRewardPowerCurve = 0x800,
    kRewardHoleInOne = 0x1000,
    kRewardRoundComplete = 0x2000,
    kRewardBall = 0x4000,
    kRewardRecovery = 0x8000,
    kRewardChipInFourfold = 0x10000,
    kRewardBallBounce = 0x20000,
    kRewardPowerShot = 0x100000
};
enum RewardGolfRule {
    kRewardStrokePlay = 0,
    kRewardMatchPlay = 1
};
enum RewardPolicy {
    kRewardAssistState2 = 2,
    kRewardTutorialCourseEnd = 6,
    kRewardSpinOrCurveMask = 0xF,
    kRewardSpinMask = 0x3,
    kRewardPowerMultiplier = 2,
    kRewardFallbackStrokes = 5,
    kChipInExcludedGroundType = 2,
    kChipInQuadrupleBonusSelector = 4,
    kDistanceBonusMinimumYards = 17,
    kLongChipInMinimumYards = 200
};
// Columns in courseScoreBonusPang; column zero is selected by a one-stroke hole.
enum RewardScoreColumn {
    kScoreOneStroke = 0,
    kScoreThreeUnder = 1,
    kScoreTwoUnder = 2,
    kScoreOneUnder = 3,
    kScorePar = 4,
    kScoreFallbackMultiplier = 5,
    kScoreColumnCount = 6
};
enum ScoreRewardAtlasRow {
    kScoreOneStrokeAtlasRow = 9,
    kScoreThreeUnderAtlasRow = 10,
    kScoreTwoUnderAtlasRow = 11,
    kScoreOneUnderAtlasRow = 12,
    kScoreParAtlasRow = 13,
    kScoreFallbackAtlasRow = 14
};
enum RewardScoreToPar {
    kRewardThreeUnderPar = -3,
    kRewardTwoUnderPar = -2,
    kRewardOneUnderPar = -1,
    kRewardAtPar = 0
};
enum RewardPresentationKind {
    kRewardScorePresentation = 8,
    kRewardPowerSpinPresentation = 9,
    kRewardPowerCurvePresentation = 10,
    kRewardObjectHitPresentation = 11
};

enum SpecialShotRewardRules {
    kSpecialShotUvBit10 = 0x10,
    kSpecialShotUvBit20 = 0x20,
    kSpecialShotUvBit40 = 0x40,
    kDiscountedSpecialShotPang = 5,
    kStandardSpecialShotPang = 10
};
// Inventory indices used by this reward rule, NOT item type IDs. Item names are unverified.
// Slot group                 Pang    Artwork selection
// 1, 2, 7, 8, 9               5      ball flags + instance power mode (see rows below)
// Every other equipped item  10      same artwork; pricing never changes the shot flag priority.
enum SpecialRewardInventorySlots {
    kSpecialRewardDiscountSlot01 = 1,
    kSpecialRewardDiscountSlot02 = 2,
    kSpecialRewardDiscountSlot07 = 7,
    kSpecialRewardDiscountSlot08 = 8,
    kSpecialRewardDiscountSlot09 = 9
};
// Normalized atlas UV = column/2, row/16. These are artwork positions, not Pang values.
// m_flags is distinct from m_powerFlags; labels deliberately retain its observed bit numbers.
// Flag priority: 0x20 -> rows 7/11, 0x10 -> 8/12, 0x40 -> 10/13 (normal/power-mode-two).
// Power mode two or flag 0x40 selects column one; all other cases select column zero.
enum SpecialRewardAtlas {
    kRewardAtlasColumns = 2,
    kRewardAtlasRows = 16,
    kRewardAtlasLeftColumn = 0,
    kRewardAtlasRightColumn = 1,
    kRewardFlag20NormalRow = 7,
    kRewardFlag20PowerRow = 11,
    kRewardFlag10NormalRow = 8,
    kRewardFlag10PowerRow = 12,
    kRewardFlag40NormalRow = 10,
    kRewardFlag40PowerRow = 13,
    kRewardPowerArtworkMode = 2
};

// Borrow the current document at each expression site; preserve the original reload order.
#define RewardDocument() ((char*)AsCSharedDoc(g_sharedDoc)->golfDoc)
// Borrow the selected score record; this is not the CGolfPlayer actor.
#define RewardCurrentPlayer() \
    ((RewardPlayerSummary*)GolfPlayer_GetEntryByIndex(RewardDocument(), AsCGolfDoc(RewardDocument())->currentPlayer))
// Add a persisted category without altering the display entry's animation state.
static inline void AddShotRewardFlag(unsigned int flag) {
    RewardCurrentPlayer()->rewardFlags |= flag;
}
// Borrow the current 32-byte hole record, including cup position and par.
static inline RewardHoleRecord* RewardCurrentHole() {
    return (RewardHoleRecord*)AsCGolfDoc(RewardDocument())->holeRecords +
           AsCGolfDoc(RewardDocument())->currentHoleIndex;
}
// Reload the inventory and selected score record in the original order; both are borrowed.
static inline void LoadRewardItemLookup(char*& inventory, RewardPlayerSummary*& player) {
    inventory = (char*)AsCSharedDoc(g_sharedDoc)->itemInventory;
    player = RewardCurrentPlayer();
}
// Decode the ball outcome from its two complemented state words; one means holed.
inline int decode_win(void* ball) {
    return ((HudRewardOutcomeWords*)((CGolfBall*)ball)->m_holeInCrypt)->first ^
           ~((HudRewardOutcomeWords*)((CGolfBall*)ball)->m_holeInCrypt)->second;
}
#define BALL_IS_HOLED (decode_win(ball) == 1)
#define REWARD_EQUIPPED_BALL_ID RewardEquippedBallAt(g_sharedDoc, AsCGolfDoc(RewardDocument())->currentPlayer)
// Store the current player's selected result-presentation category.
inline void set_result(int value) {
    char* data = (char*)RewardCurrentPlayer();
    ((RewardPlayerSummary*)data)->finishRewardKind = value;
}
// Copy one score word while preserving the required volatile store.
inline void copy_score(volatile int* dest, int* src) {
    *dest = *src;
}
// Double a trajectory threshold without changing floating-point operation order.
inline float twice(float value) {
    return 2.0f * value;
}
// Set the special-shot reward entry's Pang amount.
inline void set_channel(void* rewardInfo, int value) {
    AsCPangInfo(rewardInfo)->specialShotBonusPang = value;
}
// Restrict the course-completion bonus table index to its fourteen valid entries.
inline int clamp_rank(int value) {
    if (value < 0)
        return 0;
    if (value > 13)
        return 13;
    return value;
}

#pragma opt_common_subs off
// Select the atlas column/row from completed-shot flag priority and the instance power mode.
static inline void SelectSpecialShotRewardArtwork(void* ball, char* rewardInfo) {
    if (((HudGolfParameters*)InstanceTable_Get())->powerMode == kRewardPowerArtworkMode ||
        ((unsigned int)((CGolfBall*)ball)->m_flags & kSpecialShotUvBit40))
        AsCPangInfo(rewardInfo)->specialShotBonusUvX = (float)kRewardAtlasRightColumn / kRewardAtlasColumns;
    else
        AsCPangInfo(rewardInfo)->specialShotBonusUvX = (float)kRewardAtlasLeftColumn / kRewardAtlasColumns;
    if ((unsigned int)((CGolfBall*)ball)->m_flags & kSpecialShotUvBit20) {
        if (((HudGolfParameters*)InstanceTable_Get())->powerMode == kRewardPowerArtworkMode)
            AsCPangInfo(rewardInfo)->specialShotBonusUvY = (float)kRewardFlag20PowerRow / kRewardAtlasRows;
        else
            AsCPangInfo(rewardInfo)->specialShotBonusUvY = (float)kRewardFlag20NormalRow / kRewardAtlasRows;
    } else if ((unsigned int)((CGolfBall*)ball)->m_flags & kSpecialShotUvBit10) {
        if (((HudGolfParameters*)InstanceTable_Get())->powerMode == kRewardPowerArtworkMode)
            AsCPangInfo(rewardInfo)->specialShotBonusUvY = (float)kRewardFlag10PowerRow / kRewardAtlasRows;
        else
            AsCPangInfo(rewardInfo)->specialShotBonusUvY = (float)kRewardFlag10NormalRow / kRewardAtlasRows;
    } else if ((unsigned int)((CGolfBall*)ball)->m_flags & kSpecialShotUvBit40) {
        if (((HudGolfParameters*)InstanceTable_Get())->powerMode == kRewardPowerArtworkMode)
            AsCPangInfo(rewardInfo)->specialShotBonusUvY = (float)kRewardFlag40PowerRow / kRewardAtlasRows;
        else
            AsCPangInfo(rewardInfo)->specialShotBonusUvY = (float)kRewardFlag40NormalRow / kRewardAtlasRows;
    }
}

// Choose item-dependent special-shot Pang, then select its independent artwork.
static inline void CalculateSpecialShotReward(void* ball, char* rewardInfo, char*& inventory,
                                              RewardPlayerSummary*& itemPlayer) {
    // Phase 5: special-shot reward and UV selection. Item slots alter the amount;
    // shot bits choose the artwork, not extra additions to the reward.
    if (((unsigned int)((CGolfBall*)ball)->m_flags & kSpecialShotMask) &&
        AsCGolfDoc(RewardDocument())->pangyaAssistState == kRewardAssistState2) {
        // The first matching slot selects the discounted amount. Reload the live
        // inventory/player before each call; retain these joins for the later switch tables.
        LoadRewardItemLookup(inventory, itemPlayer);
        if (itemPlayer->equippedItemId == ItemInventory_GetItemIdAt(inventory, kSpecialRewardDiscountSlot01))
            goto discountedItem;
        LoadRewardItemLookup(inventory, itemPlayer);
        if (itemPlayer->equippedItemId == ItemInventory_GetItemIdAt(inventory, kSpecialRewardDiscountSlot02))
            goto discountedItem;
        LoadRewardItemLookup(inventory, itemPlayer);
        if (itemPlayer->equippedItemId == ItemInventory_GetItemIdAt(inventory, kSpecialRewardDiscountSlot07))
            goto discountedItem;
        LoadRewardItemLookup(inventory, itemPlayer);
        if (itemPlayer->equippedItemId == ItemInventory_GetItemIdAt(inventory, kSpecialRewardDiscountSlot08))
            goto discountedItem;
        LoadRewardItemLookup(inventory, itemPlayer);
        if (itemPlayer->equippedItemId != ItemInventory_GetItemIdAt(inventory, kSpecialRewardDiscountSlot09))
            goto regularItem;
    discountedItem:
        set_channel(rewardInfo, kDiscountedSpecialShotPang);
        goto selectedItemPang;
    regularItem:
        set_channel(rewardInfo, kStandardSpecialShotPang);
    selectedItemPang:
        AsCPangInfo(rewardInfo)->earnedPang += AsCPangInfo(rewardInfo)->specialShotBonusPang;
        AddShotRewardFlag(kRewardSpecialShot);
        SelectSpecialShotRewardArtwork(ball, rewardInfo);
    } else
        AsCPangInfo(rewardInfo)->specialShotBonusState = kBonusUnavailable;
}

// Collision events have 0x24-byte stride; object ID is a signed short at +2.
struct RewardCollisionRecord {
    char reserved00[2];
    short objectId;
    char reserved04[0x20];
};
struct RewardCollisionHistory {
    char reserved00[0x21E8];
    RewardCollisionRecord* entries;
    int count;
};
typedef char RewardCollisionStride[(sizeof(RewardCollisionRecord) == 0x24) ? 1 : -1];
// Count changes between adjacent nonnegative collision IDs; this is not global deduplication.
// Negative IDs are skipped without changing the previous valid ID; count is caller-owned output.
static inline void CountRewardObjectIdRuns(RewardCollisionRecord*& entry, RewardCollisionRecord*& end,
                                           short& previousObjectId, unsigned int& objectHitRunCount, short& objectId) {
    for (; entry != end; ++entry) {
        objectId = entry->objectId;
        if (0 <= objectId) {
            if (previousObjectId != objectId) {
                previousObjectId = objectId;
                objectHitRunCount = (objectHitRunCount + 1);
            }
        }
    }
}

// Reward helpers borrow live game/HUD state; scratch references refer to caller-owned temporaries.
// Award first-stroke distance beyond the club range, then apply the super-Pangya power-mode multiplier.
static inline void CalculateLongDriveReward(char* rewardInfo, unsigned int& strokeCount, float& distanceYards,
                                            int& specialMode) {
    if (strokeCount == 1 && (unsigned int)AsCPangInfo(rewardInfo)->clubType <= 1) {
        if (distanceYards > 0.0f && !((CGolfBall*)GolfBall__Fv())->m_recoveryRequired && !specialMode) {
            AsCPangInfo(rewardInfo)->earnedPang += (int)distanceYards;
            AddShotRewardFlag(kRewardLongDrive);
        } else
            AsCPangInfo(rewardInfo)->bonusEntryState = kBonusUnavailable;
        if (((HudImpactFields*)RewardDocument())->superPangya && AsCGolfDoc(RewardDocument())->pangyaAssistState == 2 &&
            ((HudGolfParameters*)InstanceTable_Get())->powerMode) {
            AsCPangInfo(rewardInfo)->earnedPang *= 2;
            AsCPangInfo(rewardInfo)->longDriveMultiplier = 2;
            AddShotRewardFlag(kRewardLongDriveDouble);
        } else
            AsCPangInfo(rewardInfo)->longDriveMultiplierState = kBonusUnavailable;
    } else {
        AsCPangInfo(rewardInfo)->bonusEntryState = kBonusUnavailable;
        AsCPangInfo(rewardInfo)->longDriveMultiplierState = kBonusUnavailable;
    }
}

// Award holed putts or airborne chip-ins using horizontal travel in yards.
static inline void CalculatePuttAndChipInRewards(void* ball, char* rewardInfo, float& travelYards) {
    if (AsCPangInfo(rewardInfo)->holeDistanceYards > (float)kDistanceBonusMinimumYards &&
        AsCPangInfo(rewardInfo)->clubType == kClubPutter &&
        AsCPangInfo(rewardInfo)->scoreRelativeToPar <= kRewardAtPar && !RewardHoleStatus(RewardCurrentPlayer())) {
        if (BALL_IS_HOLED) {
            AsCPangInfo(rewardInfo)->earnedPang += (int)travelYards * 2;
            AddShotRewardFlag(kRewardLongPutt);
        }
    } else
        AsCPangInfo(rewardInfo)->longPuttBonusState = kBonusUnavailable;

    if (AsCPangInfo(rewardInfo)->clubType != kClubPutter &&
        ((CGolfBall*)ball)->m_groundType != kChipInExcludedGroundType &&
        AsCPangInfo(rewardInfo)->holeDistanceYards > (float)kDistanceBonusMinimumYards &&
        AsCPangInfo(rewardInfo)->scoreRelativeToPar <= kRewardAtPar) {
        if (BALL_IS_HOLED) {
            if (!((unsigned int)((CGolfBall*)ball)->m_powerFlags & kSpecialShotMask) &&
                ((HudGolfParameters*)InstanceTable_Get())->chipInBonusSelector == kChipInQuadrupleBonusSelector) {
                AsCPangInfo(rewardInfo)->earnedPang += (int)travelYards * 4;
                AddShotRewardFlag(kRewardChipInFourfold);
            } else
                AsCPangInfo(rewardInfo)->earnedPang += (int)travelYards * 2;
            if (travelYards < (float)kLongChipInMinimumYards) {
                AddShotRewardFlag(kRewardChipIn);
            } else {
                AddShotRewardFlag(kRewardLongChipIn);
            }
        }
    } else
        AsCPangInfo(rewardInfo)->chipInBonusState = kBonusUnavailable;
}

// Award an eligible equipped ball and its bounded bounce bonus; inventory and score records are borrowed.
static inline void CalculateEquippedBallRewards(char* rewardInfo, int& eligibleByHoleScore, char*& sharedDocument,
                                                char*& golfDocument) {
    if (ItemInventory_FindBallIndex((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, REWARD_EQUIPPED_BALL_ID) &&
        AsCGolfDoc(RewardDocument())->pangyaAssistState == 2 &&
        ((HudGolfParameters*)InstanceTable_Get())->club->clubType != 3) {
        eligibleByHoleScore = 0;
        switch (AsCSharedDoc(g_sharedDoc)->golfRuleType) {
        case kRewardStrokePlay: { // Admit a stroke-play score at par or better.
            char* data = (char*)RewardCurrentPlayer();
            if (RewardHoleStrokes(data) <= RewardCurrentHole()->par)
                eligibleByHoleScore = 1;
            break;
        }
        case kRewardMatchPlay: { // Apply the same par gate in match play.
            char* data = (char*)RewardCurrentPlayer();
            if (RewardHoleStrokes(data) <= RewardCurrentHole()->par)
                eligibleByHoleScore = 1;
            break;
        }
        }
        if (eligibleByHoleScore) {
            AsCPangInfo(rewardInfo)->ballBonusPang = 10;
            AsCPangInfo(rewardInfo)->earnedBonusPang += 10;
            AddShotRewardFlag(kRewardBall);
            sharedDocument = (char*)g_sharedDoc;
            golfDocument = (char*)AsCSharedDoc(sharedDocument)->golfDoc;
            unsigned int item =
                (unsigned int)ItemInventory_GetBallIdAt((char*)&AsCSharedDoc(sharedDocument)->itemInventory, 12);
            if (item == RewardEquippedBallAt(sharedDocument, AsCGolfDoc(golfDocument)->currentPlayer) ||
                (sharedDocument = (char*)g_sharedDoc, golfDocument = (char*)AsCSharedDoc(sharedDocument)->golfDoc,
                 item =
                     (unsigned int)ItemInventory_GetBallIdAt((char*)&AsCSharedDoc(sharedDocument)->itemInventory, 128),
                 item == RewardEquippedBallAt(sharedDocument, AsCGolfDoc(golfDocument)->currentPlayer))) {
                if (!((CGolfBall*)GolfBall__Fv())->m_recoveryRequired) {
                    int amount = ((CQuadTree*)CQuadTree_GetInstance())->GetBounceCount();
                    int bonus = 20;
                    if (20 >= amount)
                        bonus = amount;
                    AsCPangInfo(rewardInfo)->earnedBonusPang += bonus;
                    AsCPangInfo(rewardInfo)->ballBounceBonusPang = bonus;
                    AddShotRewardFlag(kRewardBallBounce);
                }
            } else
                AsCPangInfo(rewardInfo)->ballBounceBonusState = kBonusUnavailable;
        } else {
            AsCPangInfo(rewardInfo)->ballBonusState = kBonusUnavailable;
            AsCPangInfo(rewardInfo)->ballBounceBonusState = kBonusUnavailable;
        }
    } else {
        AsCPangInfo(rewardInfo)->ballBonusState = kBonusUnavailable;
        AsCPangInfo(rewardInfo)->ballBounceBonusState = kBonusUnavailable;
    }
}

// Save category subtotals before doubling both Pang channels in the selected mode.
static inline void ApplyShotRewardModeMultiplier(char* rewardInfo, int includeEndOfHoleFallback) {
    if (((HudRewardMultiplierInput*)g_sharedDoc)->selector == 2 && !includeEndOfHoleFallback) {
        if (AsCPangInfo(rewardInfo)->earnedPang > 0) {
            copy_score((int*)((char*)&AsCPangInfo(rewardInfo)->pangBeforeMultiplier),
                       (int*)((char*)&AsCPangInfo(rewardInfo)->earnedPang));
            AsCPangInfo(rewardInfo)->earnedPang = *(volatile int*)(rewardInfo + 0x28) * 2;
        }
        if (AsCPangInfo(rewardInfo)->earnedBonusPang > 0) {
            copy_score((int*)((char*)&AsCPangInfo(rewardInfo)->bonusPangBeforeMultiplier),
                       (int*)((char*)&AsCPangInfo(rewardInfo)->earnedBonusPang));
            AsCPangInfo(rewardInfo)->earnedBonusPang = *(volatile int*)(rewardInfo + 0x2C) * 2;
        }
        AsCPangInfo(rewardInfo)->pangMultiplier = 2;
    } else
        AsCPangInfo(rewardInfo)->pangMultiplierState = kBonusUnavailable;
}

// Award the final-hole completion bonus after the mode multiplier.
static inline void CalculateRoundCompletionReward(void* ball, char* rewardInfo, int includeEndOfHoleFallback,
                                                  int& completionMultiplier, int& courseCompletionPang) {
    if (AsCGolfDoc(RewardDocument())->tutorialStage == 6 &&
        AsCGolfDoc(RewardDocument())->currentHoleIndex ==
            (unsigned char)(AsCSharedDoc(g_sharedDoc)->roundHoleCount - 1) &&
        !AsCSharedDoc(g_sharedDoc)->golfRuleType && !(int)AsCSharedDoc(g_sharedDoc)->partyGameMode) {
        if (BALL_IS_HOLED || (0 == BALL_IS_HOLED && includeEndOfHoleFallback)) {
            unsigned int index = clamp_rank(SharedDoc_GetCourseId());

            char* mgr = (char*)g_sharedDoc;
            completionMultiplier = 3;
            courseCompletionPang = s_roundCompletionBonusByCourse[index];
            int rounds = (int)AsCSharedDoc(mgr)->roundHoleCount / 9 + 1;
            if (AsCSharedDoc(mgr)->gameMode == 0 || AsCSharedDoc(mgr)->gameMode == 6)
                completionMultiplier = (int)AsCSharedDoc(mgr)->enemyLevelIndex + 1;
            int bonus = completionMultiplier * courseCompletionPang * rounds;
            AsCPangInfo(rewardInfo)->roundCompletionBonusPang = bonus;
            AsCPangInfo(rewardInfo)->earnedBonusPang += bonus;
            AddShotRewardFlag(kRewardRoundComplete);
        } else
            AsCPangInfo(rewardInfo)->roundCompletionBonusState = kBonusUnavailable;
    } else
        AsCPangInfo(rewardInfo)->roundCompletionBonusState = kBonusUnavailable;
}

#pragma push
// Keep nested reward helpers expanded inside the original function; restore options after it.
#pragma inline_depth(8)
#pragma inline_max_size(10000)
// Award the score category, then spin/curve or object-hit multipliers; fallback handles end-of-hole failure.
static inline void CalculateScoreAndObjectRewards(void* ball, char* rewardInfo, int& strokesRelativeToPar,
                                                  unsigned int& strokeCount, unsigned int& objectHitRunCount,
                                                  int includeEndOfHoleFallback, int& courseIndex, int& scorePang,
                                                  float& scoreArtworkV, char*& scorePlayerResult,
                                                  int& fallbackCourseIndex, int*& fallbackCourseBonuses,
                                                  unsigned int& fallbackPang) {
    // Phase 1: establish the base score award before applying any multiplier.
    if (BALL_IS_HOLED && strokesRelativeToPar <= kRewardAtPar) {
        AddShotRewardFlag(kRewardScore);
        if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kRewardMatchPlay ||
            (int)AsCSharedDoc(g_sharedDoc)->partyGameMode != 0)
            AsCPangInfo(rewardInfo)->scoreBonusState = kBonusUnavailable;
        else {
            courseIndex = SharedDoc_GetCourseId();
            if (AsCGolfDoc(RewardDocument())->tutorialStage < kRewardTutorialCourseEnd)
                courseIndex = 0;
            if ((int)strokeCount == 1)
                scorePang =
                    ((int (*)[kScoreColumnCount])courseScoreBonusPang)[(unsigned char)courseIndex][kScoreOneStroke];
            else
                switch (strokesRelativeToPar) {
                case kRewardThreeUnderPar:
                    scorePang = ((
                        int (*)[kScoreColumnCount])courseScoreBonusPang)[(unsigned char)courseIndex][kScoreThreeUnder];
                    break;
                case kRewardTwoUnderPar:
                    scorePang =
                        ((int (*)[kScoreColumnCount])courseScoreBonusPang)[(unsigned char)courseIndex][kScoreTwoUnder];
                    break;
                case kRewardOneUnderPar:
                    scorePang =
                        ((int (*)[kScoreColumnCount])courseScoreBonusPang)[(unsigned char)courseIndex][kScoreOneUnder];
                    break;
                case kRewardAtPar:
                    scorePang =
                        ((int (*)[kScoreColumnCount])courseScoreBonusPang)[(unsigned char)courseIndex][kScorePar];
                    break;
                default:
                    scorePang = ((int (*)[kScoreColumnCount])
                                     courseScoreBonusPang)[(unsigned char)courseIndex][kScoreFallbackMultiplier] *
                                strokesRelativeToPar;
                    break;
                }
            AsCPangInfo(rewardInfo)->scoreBonusPang = scorePang;
            // Normalized V endpoints in the sixteen-row reward atlas (rows 9..14).
            if ((int)strokeCount == 1)
                scoreArtworkV = (float)kScoreOneStrokeAtlasRow / kRewardAtlasRows;
            else
                switch (strokesRelativeToPar) {
                case kRewardThreeUnderPar:
                    scoreArtworkV = (float)kScoreThreeUnderAtlasRow / kRewardAtlasRows;
                    break;
                case kRewardTwoUnderPar:
                    scoreArtworkV = (float)kScoreTwoUnderAtlasRow / kRewardAtlasRows;
                    break;
                case kRewardOneUnderPar:
                    scoreArtworkV = (float)kScoreOneUnderAtlasRow / kRewardAtlasRows;
                    break;
                case kRewardAtPar:
                    scoreArtworkV = (float)kScoreParAtlasRow / kRewardAtlasRows;
                    break;
                default:
                    scoreArtworkV = (float)kScoreFallbackAtlasRow / kRewardAtlasRows;
                    break;
                }
            AsCPangInfo(rewardInfo)->scoreBonusUvY = scoreArtworkV;
        }
        // Phase 2: spin takes precedence over curve. Any counted object suppresses both.
        // Match play records presentation only; other rules add the score-derived award.
        if (AsCGolfDoc(RewardDocument())->pangyaAssistState == kRewardAssistState2 &&
            AsCPangInfo(rewardInfo)->scoreRelativeToPar <= kRewardAtPar && !(unsigned char)objectHitRunCount &&
            ((unsigned int)((CGolfBall*)ball)->m_powerFlags & kRewardSpinOrCurveMask)) {
            if ((unsigned int)((CGolfBall*)ball)->m_powerFlags & kRewardSpinMask) {
                if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kRewardMatchPlay) {
                    set_result(kRewardPowerSpinPresentation);
                    AsCPangInfo(rewardInfo)->powerSpinMultiplierState = kBonusUnavailable;
                } else {
                    AsCPangInfo(rewardInfo)->earnedPang +=
                        AsCPangInfo(rewardInfo)->scoreBonusPang * kRewardPowerMultiplier;
                    AddShotRewardFlag(kRewardPowerSpin);
                }
                AsCPangInfo(rewardInfo)->powerCurveMultiplierState = kBonusUnavailable;
            } else {
                if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kRewardMatchPlay) {
                    set_result(kRewardPowerCurvePresentation);
                    AsCPangInfo(rewardInfo)->powerCurveMultiplierState = kBonusUnavailable;
                } else {
                    AsCPangInfo(rewardInfo)->earnedPang +=
                        AsCPangInfo(rewardInfo)->scoreBonusPang * kRewardPowerMultiplier;
                    AddShotRewardFlag(kRewardPowerCurve);
                }
                AsCPangInfo(rewardInfo)->powerSpinMultiplierState = kBonusUnavailable;
            }
        } else {
            if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kRewardMatchPlay) {
                if (AsCPangInfo(rewardInfo)->scoreRelativeToPar <= kRewardAtPar)
                    set_result(kRewardScorePresentation);
            } else
                AsCPangInfo(rewardInfo)->earnedPang += AsCPangInfo(rewardInfo)->scoreBonusPang;
            AsCPangInfo(rewardInfo)->powerSpinMultiplierState = kBonusUnavailable;
            AsCPangInfo(rewardInfo)->powerCurveMultiplierState = kBonusUnavailable;
        }
        // Phase 3: object-hit runs add (2^count - 1) base awards after the base path.
        if ((unsigned char)objectHitRunCount) {
            if (AsCSharedDoc(g_sharedDoc)->golfRuleType == kRewardMatchPlay) {
                if (AsCPangInfo(rewardInfo)->scoreRelativeToPar <= kRewardAtPar) {
                    set_result(kRewardObjectHitPresentation);
                    scorePlayerResult = (char*)RewardCurrentPlayer();
                    ((RewardPlayerSummary*)scorePlayerResult)->objectHitCount = objectHitRunCount;
                }
                AsCPangInfo(rewardInfo)->objectHitMultiplierState = kBonusUnavailable;
            } else {
                if (AsCPangInfo(rewardInfo)->scoreRelativeToPar <= kRewardAtPar)
                    AsCPangInfo(rewardInfo)->earnedPang +=
                        AsCPangInfo(rewardInfo)->scoreBonusPang * ((int)pow(2.0, (unsigned char)objectHitRunCount) - 1);
                AsCPangInfo(rewardInfo)->objectHitMultiplier = 1;
                AddShotRewardFlag(kRewardObjectHit);
            }
        } else
            AsCPangInfo(rewardInfo)->objectHitMultiplierState = kBonusUnavailable;
    } else {
        // Phase 4: end-of-hole fallback uses five times the course's last column.
        if (AsCPangInfo(rewardInfo)->scoreRelativeToPar + ((CGolfBall*)ball)->m_recoveryRequired >=
                kRewardFallbackStrokes &&
            includeEndOfHoleFallback && (int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 0) {
            AsCPangInfo(rewardInfo)->scoreBonusUvY = (float)kScoreFallbackAtlasRow / kRewardAtlasRows;
            fallbackCourseIndex = SharedDoc_GetCourseId();
            fallbackCourseBonuses =
                ((int (*)[kScoreColumnCount])courseScoreBonusPang)[(unsigned char)fallbackCourseIndex];
            fallbackPang = kRewardFallbackStrokes * fallbackCourseBonuses[kScoreFallbackMultiplier];
            AsCPangInfo(rewardInfo)->earnedPang += fallbackPang;
            AsCPangInfo(rewardInfo)->scoreBonusPang = fallbackPang;
        } else
            AsCPangInfo(rewardInfo)->scoreBonusState = kBonusUnavailable;
        AsCPangInfo(rewardInfo)->powerSpinMultiplierState = kBonusUnavailable;
        AsCPangInfo(rewardInfo)->powerCurveMultiplierState = kBonusUnavailable;
        if ((unsigned char)objectHitRunCount)
            AsCPangInfo(rewardInfo)->objectHitMultiplier = 1;
        else
            AsCPangInfo(rewardInfo)->objectHitMultiplierState = kBonusUnavailable;
    }
}

// Calculates shot pang and enables bonus categories from distance, lie, score, items, and shot flags.
// Requires live document/ball/player/course state. Mutates this HUD's reward
// entries, current score flags and player totals; owns none of those objects.
// includeEndOfHoleFallback enables fallback rewards and suppresses object-hit counting.
// Rejected categories store kBonusUnavailable; accepted ones add Pang and/or flags.
void CPangInfo::CalculateShotRewards(int includeEndOfHoleFallback) {
    // Phase 1: clear totals, reject disabled mode and snapshot par/stroke/club data.
    Vec3 horizontal;
    Vec3 flatPinFromFinalWorld;
    Vec3 pinFromStartWorld;
    void* ball = GolfBall__Fv();
    char* rewardInfo = (char*)this;
    (int&)AsCPangInfo(rewardInfo)->flags = 1;
    AsCPangInfo(rewardInfo)->earnedPang = 0;
    AsCPangInfo(rewardInfo)->earnedBonusPang = 0;
    char* manager = (char*)g_sharedDoc;
    char* game = ((char*)AsCSharedDoc(manager)->golfDoc);
    // CGolfDoc +3CC is a byte initialized to 0 by CGolfDoc::Init (golf_doc_turns_and_parameter_xml.cpp).
    // Nonzero suppresses shot Pang here, shot-stat accumulation in
    // GolfRule_AccumulateCurrentShotStatistics (golf_rule_round_flow_and_shot_sim.cpp), and some timeout/
    // result presentation; swing-obstruction/collision paths also consult it.
    // No nonzero producer is identified in current src/include, so a specific
    // game-mode name is unproved. Treat any nonzero as suppression, not ==1.
    // A reward mod must preserve this gate unless its producer is first resolved.
    if (AsCGolfDoc(game)->unk_3CC) {
        for (int i = 0; i < kBonusEntryCount; ++i)
            ((Channels*)rewardInfo)->channels[i].state = kBonusUnavailable;
        return;
    }

    unsigned int holeIndex = AsCGolfDoc(game)->currentHoleIndex;
    RewardHoleRecord* holeRecord = (RewardHoleRecord*)AsCGolfDoc(game)->holeRecords + holeIndex;
    int holePar = holeRecord->par;
    char* playerResult = (char*)GolfPlayer_GetEntryByIndex(game, AsCGolfDoc(game)->currentPlayer);

    unsigned int strokeCount = RewardHoleStrokes(playerResult);
    int specialMode = 0;
    if ((unsigned)((int)AsCSharedDoc(g_sharedDoc)->partyGameMode - kFirstStrokeOverrideMode) <=
        (kLastStrokeOverrideMode - kFirstStrokeOverrideMode)) {
        specialMode = 1;
        strokeCount = 1;
        strokeCount = (includeEndOfHoleFallback) ? (holePar + 5) & 255 : strokeCount;
    }
    int strokesRelativeToPar = strokeCount - holePar;
    AsCPangInfo(rewardInfo)->scoreRelativeToPar = strokesRelativeToPar;

    void* round = InstanceTable_Get();
    AsCPangInfo(rewardInfo)->clubType = ((HudGolfParameters*)round)->club->clubType;
    round = InstanceTable_Get();
    AsCPangInfo(rewardInfo)->clubDistanceYards = GolfParameter_ResolveFloat(round, 0);
    AsCPangInfo(rewardInfo)->shotGroundType = ((CGolfBall*)ball)->m_groundType;

    // Phase 2: derive pin direction and distances from ball trajectory samples.
    // Positions use game world units; divide magnitudes by 3.2 for yards.
    // horizontal/flatPinFromFinalWorld ignore Y. holeDistanceYards measures the
    // horizontal pin-to-start distance: store vector, clear holeDirectionY, THEN
    // take magnitude/3.2. Elevation is excluded even on a sloping hole; the
    // normalized holeDirection vector is also flat. Keep this order and precision.
    char* currentHoleBytes = ((char*)RewardCurrentHole());
    Words3 cupPositionBits = ((RewardHoleRecord*)currentHoleBytes)->cupWorldBits;
    __mi__FRC7WVectorRC7WVector(&pinFromStartWorld, &cupPositionBits, PreData__9CGolfBallFi(ball, 0));
    __as__7WVectorFRC7WVector(rewardInfo + 0x3E4, &pinFromStartWorld);
    AsCPangInfo(rewardInfo)->holeDirectionY = 0.0f;

    AsCPangInfo(rewardInfo)->holeDistanceYards = (Magnitude__7WVectorCFv(rewardInfo + 0x3E4)) / 3.2f;
    Normalize__7WVectorFv(rewardInfo + 0x3E4);
    void* finalTrajectorySample = PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_preDataCount);
    Vec3 startMinusFinalWorld;
    void* initialTrajectorySample = PreData__9CGolfBallFi(ball, 0);
    Vec3 pinFromFinalWorld;
    __mi__FRC7WVectorRC7WVector(&startMinusFinalWorld, initialTrajectorySample, finalTrajectorySample);
    __as__7WVectorFRC7WVector(&horizontal, &startMinusFinalWorld);
    horizontal.y = 0.0f;
    float travelYards = Magnitude__7WVectorCFv(&horizontal) / 3.2f;
    float nonnegativeTravelYards = 0.0f;
    if (nonnegativeTravelYards < travelYards)
        nonnegativeTravelYards = travelYards;
    float distanceYards = nonnegativeTravelYards - AsCPangInfo(rewardInfo)->clubDistanceYards;
    // Phase 3: long-drive and approach eligibility uses yards; putt/chip-in rewards
    // also require the complemented ball outcome to decode as holed (1).
    CalculateLongDriveReward(rewardInfo, strokeCount, distanceYards, specialMode);
    Words3 pin = RewardCurrentHole()->cupWorldBits;
    __mi__FRC7WVectorRC7WVector(&pinFromFinalWorld, &pin,
                                PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_preDataCount));
    __as__7WVectorFRC7WVector(&flatPinFromFinalWorld, &pinFromFinalWorld);
    flatPinFromFinalWorld.y = 0.0f;
    distanceYards = Magnitude__7WVectorCFv(&flatPinFromFinalWorld) / 3.2f;
    int eligible = AsCPangInfo(rewardInfo)->scoreRelativeToPar < 0;
    if (eligible)
        eligible = AsCPangInfo(rewardInfo)->clubType != 3;
    if (eligible) {
        Vec3 pos = *(Vec3*)PreData__9CGolfBallFi(ball, 0);
        eligible = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &pos) != 2;
    }
    if (eligible)
        eligible = distanceYards < 7.0f;
    if (eligible)
        eligible = BALL_IS_HOLED == 0;
    if (eligible)
        eligible = AsCPangInfo(rewardInfo)->holeDistanceYards > 30.0f;
    if (eligible) {
        if (AsCPangInfo(rewardInfo)->holeDistanceYards < 60.0f)
            AsCPangInfo(rewardInfo)->earnedPang +=
                (1 - AsCPangInfo(rewardInfo)->scoreRelativeToPar * (10 - (int)distanceYards)) / 2;
        else
            AsCPangInfo(rewardInfo)->earnedPang -=
                AsCPangInfo(rewardInfo)->scoreRelativeToPar * (10 - (int)distanceYards);
        AddShotRewardFlag(kRewardApproach);
    } else
        AsCPangInfo(rewardInfo)->approachBonusState = kBonusUnavailable;
    CalculatePuttAndChipInRewards(ball, rewardInfo, travelYards);
    // Phase 4: validate contact-sample ordering before testing pin distance.
    // These are trajectory indices; negative values represent missing events.
    unsigned int eligibleContactEvent = 0;
    int noPriorContactIndex = 0;
    unsigned int contactAtOrBeforeCurrentSample = 0;
    int hasContactSample = 0;
    Vec3 pos = *(Vec3*)PreData__9CGolfBallFi(ball, 0);
    if (GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &pos) != 2 &&
        ((CGolfBall*)ball)->m_eventPreDataIndex[9] > 0)
        hasContactSample = 1;
    if (hasContactSample && ((CGolfBall*)ball)->m_eventPreDataIndex[9] <= ((CGolfBall*)ball)->m_preDataCount)
        contactAtOrBeforeCurrentSample = 1;
    if (contactAtOrBeforeCurrentSample && ((CGolfBall*)ball)->m_eventPreDataIndex[8] < 0)
        noPriorContactIndex = 1;
    if (noPriorContactIndex) {
        int ok = (((CGolfBall*)ball)->m_eventPreDataIndex[9] < ((CGolfBall*)ball)->m_eventPreDataIndex[1] ||
                  ((CGolfBall*)ball)->m_eventPreDataIndex[1] < 0);
        if (ok)
            eligibleContactEvent = 1;
    }
    if (eligibleContactEvent) {
        Vec3 d;
        Vec3 origin = RewardCurrentHole()->cupWorld;
        __mi__FRC7WVectorRC7WVector(&d, PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_eventPreDataIndex[9]),
                                    &origin);
        float length = Magnitude__7WVectorCFv(&d);
        if (AsCPangInfo(rewardInfo)->scoreRelativeToPar <= 0 && BALL_IS_HOLED &&
            length > twice(((CGolfBall*)ball)->m_radius)) {
            AsCPangInfo(rewardInfo)->pinContactBonusPang = 100;
            AsCPangInfo(rewardInfo)->earnedPang += 100;
            AddShotRewardFlag(kRewardPinContact);
        } else
            AsCPangInfo(rewardInfo)->pinContactBonusState = kBonusUnavailable;
    } else
        AsCPangInfo(rewardInfo)->pinContactBonusState = kBonusUnavailable;
    // Explicit borrowed lookup scratch preserves the compiler's register lifetimes.
    char* inventory;
    RewardPlayerSummary* itemPlayer;
    CalculateSpecialShotReward(ball, rewardInfo, inventory, itemPlayer);
    // Independent match-only entry is disabled during ordinary shot reward calculation.
    AsCPangInfo(rewardInfo)->matchEarlyWinBonusState = kBonusUnavailable;
    // Phase 6: count distinct adjacent nonnegative IDs in 0x24-byte collision
    // records, then award score/power-spin/power-curve/object multipliers.
    unsigned int objectHitRunCount = 0;
    if (!includeEndOfHoleFallback) {
        RewardCollisionHistory* history = (RewardCollisionHistory*)CQuadTree_GetInstance();
        RewardCollisionRecord* entry = history->entries;
        RewardCollisionRecord* end = entry + history->count;
        short previousObjectId = -1;
        short objectId;
        CountRewardObjectIdRuns(entry, end, previousObjectId, objectHitRunCount, objectId);
    }
    int courseIndex;
    int scorePang;
    float scoreArtworkV;
    char* scorePlayerResult;
    int fallbackCourseIndex;
    int* fallbackCourseBonuses;
    unsigned int fallbackPang;
    CalculateScoreAndObjectRewards(ball, rewardInfo, strokesRelativeToPar, strokeCount, objectHitRunCount,
                                   includeEndOfHoleFallback, courseIndex, scorePang, scoreArtworkV, scorePlayerResult,
                                   fallbackCourseIndex, fallbackCourseBonuses, fallbackPang);
    if (BALL_IS_HOLED) {
        char* data = (char*)RewardCurrentPlayer();
        if (RewardHoleStrokes(data) == 1 && AsCSharedDoc(g_sharedDoc)->golfRuleType <= 1 &&
            (int)AsCSharedDoc(g_sharedDoc)->partyGameMode == 0) {
            AsCPangInfo(rewardInfo)->holeInOneBonusPang = 500;
            AsCPangInfo(rewardInfo)->earnedBonusPang += 500;
            AddShotRewardFlag(kRewardHoleInOne);
        } else
            AsCPangInfo(rewardInfo)->holeInOneBonusState = kBonusUnavailable;
    } else
        AsCPangInfo(rewardInfo)->holeInOneBonusState = kBonusUnavailable;
    // Phase 7: recovery, power-shot and equipped-ball bonuses. Bonus Pang
    // accumulates separately from the regular Pang awarded above.
    if (((CGolfBall*)ball)->m_eventPreDataIndex[7] > 0 &&
        ((CGolfBall*)ball)->m_eventPreDataIndex[7] < ((CGolfBall*)ball)->m_preDataCount) {
        void* point = PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_eventPreDataIndex[7]);
        if (point) {
            Vec3 position = *(Vec3*)point;
            if (GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &position) >= 3) {
                if (CourseData_IsPointOutsidePlayableRegion((char*)AsCGolfDoc(RewardDocument())->courseData, point)) {
                    char* game2 = RewardDocument();
                    if (!CourseData_IsPointOutsidePlayableRegion(
                            (char*)AsCGolfDoc(game2)->courseData,
                            PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_preDataCount))) {
                        AsCPangInfo(rewardInfo)->earnedPang += 15;
                        AddShotRewardFlag(kRewardRecovery);
                    }
                } else {
                    Vec3 finalPos = *(Vec3*)PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_preDataCount);
                    if ((unsigned char)(GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), &finalPos) + 255) <=
                        1) {
                        AsCPangInfo(rewardInfo)->earnedPang += 10;
                        AddShotRewardFlag(kRewardRecovery);
                    }
                }
            } else
                AsCPangInfo(rewardInfo)->recoveryBonusState = kBonusUnavailable;
        } else
            AsCPangInfo(rewardInfo)->recoveryBonusState = kBonusUnavailable;
    } else
        AsCPangInfo(rewardInfo)->recoveryBonusState = kBonusUnavailable;
    if (RewardControlAt(g_sharedDoc, AsCGolfDoc(RewardDocument())->currentPlayer) == 0 &&
        ((HudGolfParameters*)InstanceTable_Get())->club->clubType != 3 &&
        AsCGolfDoc(RewardDocument())->pangyaAssistState == 2) {
        int bonus = AsCGolfDoc(RewardDocument())->powerShotBonusPang;
        AsCPangInfo(rewardInfo)->powerShotBonusPang = bonus;
        AsCPangInfo(rewardInfo)->earnedBonusPang += bonus;
        AddShotRewardFlag(kRewardPowerShot);
    } else
        AsCPangInfo(rewardInfo)->powerShotBonusState = kBonusUnavailable;
    int eligibleByHoleScore;
    char* sharedDocument;
    char* golfDocument;
    CalculateEquippedBallRewards(rewardInfo, eligibleByHoleScore, sharedDocument, golfDocument);
    // Phase 8: apply the mode multiplier after category totals; save pre-multiplier
    // amounts for animation, then add the round-completion bonus.
    ApplyShotRewardModeMultiplier(rewardInfo, includeEndOfHoleFallback);
    int completionMultiplier;
    int courseCompletionPang;
    CalculateRoundCompletionReward(ball, rewardInfo, includeEndOfHoleFallback, completionMultiplier,
                                   courseCompletionPang);
    // Phase 9: commit both totals to the selected player; display entries retain details.
    GolfPlayer_ApplyShotPangReward(rewardInfo, AsCGolfDoc(RewardDocument())->currentPlayer,
                                   (ShotPangReward*)(int*)((char*)&AsCPangInfo(rewardInfo)->earnedPang));
}
#pragma pop
#pragma opt_common_subs reset

#undef RewardHoleStrokes
#undef RewardHoleStatus
#undef RewardEquippedBallAt
#undef RewardControlAt
#undef BALL_IS_HOLED
#undef REWARD_EQUIPPED_BALL_ID

// Pang accumulation.

struct PangAnimationFields;

struct PangAnimationFields {
    unsigned char pad00[0x20];
    unsigned int flags;
    unsigned char pad24[0x24];
    int pangAnimationState;
    unsigned char pad4C[8];
    float pangAnimationTime;
    unsigned char pad58[0x1C];
    int bonusPangAnimationState;
    unsigned char pad78[8];
    float bonusPangAnimationTime;
    unsigned char pad84[0x1C];
    unsigned char padA0[0x210];
    int holeInOneBonusState;
    unsigned char pad2B4[0x360 - 0x2B4];
    int ballBonusState;
    unsigned char pad364[0x43C - 0x364];
    float holeInOneBannerElapsed;
    float ballBonusBannerElapsed;
    unsigned char pad444[8];
    float completionDelayElapsed;
    int bannerLayoutCount;
};

struct PangBallStatusFields {
    unsigned char pad00[0x18];
    int value;
};

struct PangAnimationEntryFields {
    int value;
    int zero;
    int minusOne;
    float timer;
    unsigned char pad10[0x1C];
};

#undef global
#undef mgr
#undef MAN
#undef PLAYER

// Policy inputs absent from the shared header's named fields. Native Wii views,
// not serialized data: multiplierSelector shares CSharedDoc +0x9C6 padding; the win
// bonus is CPangInfo +0x1AC, immediately after matchEarlyWinBonusState.
struct RewardSelectorView {
    char prefix[0x9C6];
    unsigned char multiplierSelector;
};
struct MatchWinRewardView {
    char prefix[0x1AC];
    int earlyWinBonusPang;
};
// Match-play policy lookup (all amounts are integer Pang, not display pixels):
// golfRuleType==1 admits work; all other types leave the object unchanged.
// ResolveMatchTurnFromHoleStrokes selects the rewarded player; opponent=1-player.
// finishRewardKind==0 skips scoring. Under/equal par uses courseScoreBonusPang
// [course ID], with tutorialStage<6 forcing course 0. One stroke takes the
// oneStrokeBonus column; otherwise score-to-par selects the named columns.
// Finish kind 9/10, objectHitCount (2^count via pow), then multiplierSelector==2 apply
// their multipliers IN THAT ORDER. The selector's producer/meaning is unresolved;
// preserve the existing positive-only guards on its multiplier.
// Match lead includes THIS hole's comparison; strictly exceeding remaining holes
// clinches the match and adds remainingHoles*50. Completion occurs on the last
// hole OR either clinch; clamp course to 0..13 for s_roundCompletionBonusByCourse.
// Completion = (roundHoleCount/9+1) * modeFactor * courseCompletionBonus;
// modeFactor=enemyLevelIndex+1 in game modes 0/6, otherwise 3. Round-completion
// flag/bonus are separate from earnedPang. A losing hole can zero earnedPang
// while retaining earnedBonusPang. Settlement calls GolfPlayer_ApplyShotPangReward
// with two adjacent signed words (earnedPang,earnedBonusPang), then separately
// computes opponent additions. The final loop settles the first other player,
// NOT every player; match mode requires its two-player participant convention.
// Future policy mods must test tie/loss/clinch/final-hole, finish kind 0/9/10,
// tutorial/course limits, object-hit exponent and positive/zero weather totals.
// Repeated document/player loads and volatile reward snapshot accesses preserve
// original observation order. Volatile is compiler shape, not synchronization.

#define MATCH_SHARED_BYTES ((unsigned char*)g_sharedDoc)
#define MATCH_DOCUMENT ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)
#define MATCH_DOCUMENT_BYTES ((unsigned char*)MATCH_DOCUMENT)
#define MATCH_PLAYER_ENTRY(i) ((unsigned char*)GolfPlayer_GetEntryByIndex(MATCH_DOCUMENT, (unsigned char)(i)))

// Snapshot persisted Pang/bonus totals, reset HUD animation, and clear the displayed player's hole-in-one flag.
// playerIndex is an output; playerEntry/startingTotal are borrowed scratch retained for CW register lifetimes.
static inline void SeedMatchPlayRewardDisplay(void* self, unsigned char* rewardBytes, int& playerIndex,
                                              unsigned char*& playerEntry, int& startingTotal) {
    if (*(int*)(rewardBytes + IFF_OFFSET(CPangInfo, countingSoundAlias)))
        Sound_StopByAlias(g_soundManager,
                          (const char*)*(int*)(rewardBytes + IFF_OFFSET(CPangInfo, countingSoundAlias)));
    playerIndex = ((CGolfDoc*)MATCH_DOCUMENT)->ResolveMatchTurnFromHoleStrokes();
    playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
    startingTotal = ((RewardPlayerSummary*)playerEntry)->pang;
    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, displayedPang)) = startingTotal;
    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, settledPang)) = startingTotal;
    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, startingPang)) = startingTotal;
    playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
    startingTotal = ((RewardPlayerSummary*)playerEntry)->bonusPang;
    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, displayedBonusPang)) = startingTotal;
    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, settledBonusPang)) = startingTotal;
    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, startingBonusPang)) = startingTotal;
    ((CPangInfo*)((PangAnimationFields*)self))->ResetPangAndBonusAnimation(1, kBonusUnavailable);
    playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
    ((RewardPlayerSummary*)playerEntry)->rewardFlags &= ~kRewardHoleInOne;
}

// Calculates mode-specific hole rewards and score bonuses for the participating players.
void CPangInfo::CalculateMatchPlayHoleRewards() {
    // Read both players' persisted scores, award hole/match categories, and settle Pang pairs.
    unsigned char* rewardBytes;
    int queriedHoleIndex;
    unsigned char* playerEntry;
    int startingTotal;
    unsigned char* slot;
    int holeRecordsAddress;
    int holePar;
    int scoreBonus;
    float scoreLabelUvY;
    unsigned char finishRewardKind;
    int multiplierState;
    unsigned char objectHitCount;
    int objectHitFactor;
    int leadDifference;
    unsigned char* opponentEntry;
    unsigned char* rivalLead;
    unsigned char* rivalDeficit;
    int deficit;
    unsigned int finalHoleIndex;
    int completionCourseId;
    int clampedCourseId;
    unsigned int courseCompletionBonus;
    int completionModeFactor;
    int holeCountFactor;
    int opponentHoleIndex;
    int opponentStrokesRelativeToPar;
    int opponentStrokes;
    unsigned char* holeRecords;
    int opponentScoreBonus;
    unsigned int multiplier;
    int playerIndex, strokesRelativeToPar, opponentClinchedMatch, playerStrokes, remainingHoles, playerClinchedMatch;

    rewardBytes = (unsigned char*)this;
    // Phase 1: seed displayed totals, then calculate score and item/weather multipliers.
    if (AsCSharedDoc(g_sharedDoc)->golfRuleType == 1) {
        SeedMatchPlayRewardDisplay(this, rewardBytes, playerIndex, playerEntry, startingTotal);
        slot = MATCH_PLAYER_ENTRY(playerIndex);
        if (((RewardPlayerSummary*)slot)->finishRewardKind) {
            *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, scoreBonusState)) = 1;
            unsigned char* manager = MATCH_DOCUMENT_BYTES;
            holeRecordsAddress = (int)AsCGolfDoc(manager)->holeRecords;
            int course = AsCGolfDoc(manager)->currentHoleIndex;
            playerStrokes = ((RewardPlayerSummary*)(slot + course))->strokes[0];
            holePar = ((RewardHoleRecord*)holeRecordsAddress)[course].par;
            strokesRelativeToPar = playerStrokes - holePar;
            if (strokesRelativeToPar <= 0) {
                int courseId = SharedDoc_GetCourseId();
                if (AsCGolfDoc(MATCH_DOCUMENT)->tutorialStage < 6)
                    courseId = 0;

                if (1 == playerStrokes)
                    scoreBonus = courseScoreBonusPang[(unsigned char)courseId].oneStrokeBonus;
                else
                    switch (strokesRelativeToPar) {
                    case -3:
                        scoreBonus = courseScoreBonusPang[(unsigned char)courseId].threeUnderParBonus;
                        break;
                    case -2:
                        scoreBonus = courseScoreBonusPang[(unsigned char)courseId].twoUnderParBonus;
                        break;
                    case -1:
                        scoreBonus = courseScoreBonusPang[(unsigned char)courseId].oneUnderParBonus;
                        break;
                    case 0:
                        scoreBonus = courseScoreBonusPang[(unsigned char)courseId].parBonus;
                        break;
                    default:
                        scoreBonus = strokesRelativeToPar *
                                     courseScoreBonusPang[(unsigned char)courseId].fallbackScoreMultiplier;
                        break;
                    }

                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, scoreBonusPang)) = scoreBonus;
                if (playerStrokes == 1)
                    scoreLabelUvY = kFloat_0_5625_Hud;
                else
                    switch (strokesRelativeToPar) {
                    case -3:
                        scoreLabelUvY = kFloat_0_625_PangBanner;
                        break;
                    case -2:
                        scoreLabelUvY = kHudScalar0_6875;
                        break;
                    case -1:
                        scoreLabelUvY = kFloat_0_75_Hud;
                        break;
                    case 0:
                        scoreLabelUvY = kHudScalar0_8125;
                        break;
                    default:
                        scoreLabelUvY = kFloat_0_875_PangBonus;
                        break;
                    }
                *(float*)(rewardBytes + IFF_OFFSET(CPangInfo, scoreBonusUvY)) = scoreLabelUvY;
            } else
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, scoreBonusState)) = kBonusUnavailable;
            finishRewardKind = ((RewardPlayerSummary*)slot)->finishRewardKind;
            // Persisted presentation category (set_result in the shot-score path):
            // 0=no finish reward; 8=score,9=power spin,10=power curve,11=object hit.
            // The byte-wrapping +0xF7 test means EXACTLY {9,10}. Those add 2*score
            // bonus and enable the corresponding multiplier artwork; other nonzero
            // kinds add 1*score bonus. objectHitCount then independently adds
            // scoreBonus*(2^count-1) for <=par, not a replacement for that double.
            // Weather variant 2 doubles positive accumulated totals afterwards.
            // Unknown nonzero category codes use the ordinary single-bonus path.
            if ((unsigned char)(finishRewardKind + 0xf7) <= 1) {
                if (!(!(finishRewardKind == 9))) {
                    multiplierState = 1;
                    int bonus = *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, scoreBonusPang)) * 2;
                    int total = *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang));
                    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) = (total + bonus);
                    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, powerSpinMultiplierState)) = multiplierState;
                } else {
                    int total = *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang));
                    int bonus = *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, scoreBonusPang)) * 2;
                    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, powerCurveMultiplierState)) = (1);
                    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) = (total + bonus);
                }
            } else
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) +=
                    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, scoreBonusPang));
            objectHitCount = ((RewardPlayerSummary*)slot)->objectHitCount;
            if (objectHitCount) {
                objectHitFactor = (int)pow(kDouble_2_Hud, (double)(unsigned int)objectHitCount);
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, objectHitMultiplier)) = objectHitFactor;
                if (0 >= strokesRelativeToPar)
                    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) +=
                        *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, scoreBonusPang)) * (objectHitFactor - 1);
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, objectHitMultiplierState)) = 1;
            }
            if (((RewardSelectorView*)g_sharedDoc)->multiplierSelector == 2) {
                if (*(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) > 0) {
                    ((volatile CPangInfo*)rewardBytes)->pangBeforeMultiplier =
                        *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang));
                    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) =
                        ((volatile CPangInfo*)rewardBytes)->earnedPang * 2;
                }
                if (*(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedBonusPang)) > 0) {
                    ((volatile CPangInfo*)rewardBytes)->bonusPangBeforeMultiplier =
                        *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedBonusPang));
                    *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedBonusPang)) =
                        ((volatile CPangInfo*)rewardBytes)->earnedBonusPang * 2;
                }
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, pangMultiplierState)) = 1;
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, pangMultiplier)) = 2;
            }
        }
        // Role aliases retain the original locals' register lifetimes across phases.
#define opponentIndex strokesRelativeToPar
#define roundCompletionPang playerStrokes
        // Phase 2: compare match lead with holes remaining, including this hole's strokes.
        unsigned int course;
        opponentIndex = 1 - playerIndex;
        opponentEntry = MATCH_PLAYER_ENTRY(opponentIndex);
        playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
        playerClinchedMatch = 0;
        opponentClinchedMatch = 0;
        leadDifference =
            ((RewardPlayerSummary*)playerEntry)->matchScore - ((RewardPlayerSummary*)opponentEntry)->matchScore;
        course = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
        remainingHoles = (int)AsCSharedDoc(g_sharedDoc)->roundHoleCount - (course + 1);
        if (0 < remainingHoles) {
            rivalLead = MATCH_PLAYER_ENTRY(opponentIndex);
            queriedHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
            playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
            if (((RewardPlayerSummary*)(playerEntry + queriedHoleIndex))->strokes[0] <
                ((RewardPlayerSummary*)(rivalLead + course))->strokes[0])
                ++leadDifference;
            else {
                queriedHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
                rivalLead = MATCH_PLAYER_ENTRY(opponentIndex);
                course = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
                playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
                if (((RewardPlayerSummary*)(playerEntry + course))->strokes[0] >
                    ((RewardPlayerSummary*)(queriedHoleIndex + rivalLead))->strokes[0])
                    --leadDifference;
            }
            if (leadDifference > remainingHoles) {
                int bonus = (remainingHoles * 50);
                playerClinchedMatch = 1;
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, matchEarlyWinBonusState)) = 1;
                ((MatchWinRewardView*)rewardBytes)->earlyWinBonusPang = bonus;
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) += bonus;
            }
            if (0 == playerClinchedMatch) {
                playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
                rivalDeficit = MATCH_PLAYER_ENTRY(opponentIndex);
                deficit =
                    ((RewardPlayerSummary*)rivalDeficit)->matchScore - ((RewardPlayerSummary*)playerEntry)->matchScore;
                queriedHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
                rivalDeficit = MATCH_PLAYER_ENTRY(opponentIndex);
                course = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
                playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
                if (((RewardPlayerSummary*)(course + playerEntry))->strokes[0] >
                    ((RewardPlayerSummary*)(rivalDeficit + queriedHoleIndex))->strokes[0])
                    ++deficit;
                else {
                    queriedHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
                    rivalDeficit = MATCH_PLAYER_ENTRY(opponentIndex);
                    course = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
                    playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
                    if (((RewardPlayerSummary*)(course + playerEntry))->strokes[0] <
                        ((RewardPlayerSummary*)(rivalDeficit + queriedHoleIndex))->strokes[0])
                        --deficit;
                }
                if (deficit > remainingHoles)
                    opponentClinchedMatch = 1;
            }
        }
        // Phase 3: award completion Pang on the final hole or an already decided match.
        roundCompletionPang = 0;
        finalHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
        if (finalHoleIndex == (unsigned char)(AsCSharedDoc(g_sharedDoc)->roundHoleCount - 1) || playerClinchedMatch ||
            opponentClinchedMatch) {
            completionCourseId = (unsigned char)SharedDoc_GetCourseId();

            if (completionCourseId < 0)
                clampedCourseId = 0;
            else
                clampedCourseId = 13 < completionCourseId ? 13 : completionCourseId;
            courseCompletionBonus = s_roundCompletionBonusByCourse[clampedCourseId];
            completionModeFactor = 3;
            holeCountFactor = (int)AsCSharedDoc(g_sharedDoc)->roundHoleCount / 9 + 1;
            if (AsCSharedDoc(g_sharedDoc)->gameMode == 0 || AsCSharedDoc(g_sharedDoc)->gameMode == 6)
                completionModeFactor = *(int*)&AsCSharedDoc(g_sharedDoc)->enemyLevelIndex + 1;
            roundCompletionPang = holeCountFactor * (completionModeFactor * courseCompletionBonus);
            *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, roundCompletionBonusState)) = 1;
            *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, roundCompletionBonusPang)) = roundCompletionPang;
            playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
            ((RewardPlayerSummary*)playerEntry)->rewardFlags |= kRewardRoundComplete;
            *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedBonusPang)) +=
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, roundCompletionBonusPang));
            queriedHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
            opponentEntry = MATCH_PLAYER_ENTRY(opponentIndex);
            course = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
            playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
            if (((RewardPlayerSummary*)(playerEntry + course))->strokes[0] >
                ((RewardPlayerSummary*)(opponentEntry + queriedHoleIndex))->strokes[0])
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) = 0;
            GolfPlayer_ApplyShotPangReward(this, (unsigned char)playerIndex,
                                           (ShotPangReward*)(int*)(rewardBytes + 0x28));
        } else {
            opponentEntry = MATCH_PLAYER_ENTRY(opponentIndex);
            queriedHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
            playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
            if (((RewardPlayerSummary*)(playerEntry + queriedHoleIndex))->strokes[0] <=
                ((RewardPlayerSummary*)(finalHoleIndex + opponentEntry))->strokes[0])
                GolfPlayer_ApplyShotPangReward(this, (unsigned char)playerIndex,
                                               (ShotPangReward*)(int*)(rewardBytes + 0x28));
            else
                *(int*)(rewardBytes + IFF_OFFSET(CPangInfo, earnedPang)) = 0;
        }
        // Phase 4: compute the opponent's score/completion additions and settle them.
        int additions[2] = {0, 0};
        playerEntry = MATCH_PLAYER_ENTRY(opponentIndex);
        if (((RewardPlayerSummary*)playerEntry)->finishRewardKind) {
            opponentHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
            opponentEntry = MATCH_PLAYER_ENTRY(opponentIndex);
            queriedHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
            playerEntry = MATCH_PLAYER_ENTRY(playerIndex);
            if (((RewardPlayerSummary*)(playerEntry + queriedHoleIndex))->strokes[0] >=
                ((RewardPlayerSummary*)(opponentEntry + opponentHoleIndex))->strokes[0]) {
                queriedHoleIndex = AsCGolfDoc(MATCH_DOCUMENT)->currentHoleIndex;
                opponentEntry = MATCH_PLAYER_ENTRY(opponentIndex);
                opponentStrokes = ((RewardPlayerSummary*)(queriedHoleIndex + opponentEntry))->strokes[0];
                unsigned char* manager = MATCH_DOCUMENT_BYTES;
                holeRecords = (unsigned char*)*(void**)(manager + 0x2bc);
                opponentStrokesRelativeToPar = opponentStrokes - *(holeRecords + manager[0x3b6] * 32 + 0x18);
                if (0 >= opponentStrokesRelativeToPar) {
                    int courseId = SharedDoc_GetCourseId();
                    if (AsCGolfDoc(MATCH_DOCUMENT)->tutorialStage < 6)
                        courseId = 0;

                    if (1 == opponentStrokes)
                        opponentScoreBonus = courseScoreBonusPang[(unsigned char)courseId].oneStrokeBonus;
                    else
                        switch (opponentStrokesRelativeToPar) {
                        case -3:
                            opponentScoreBonus = courseScoreBonusPang[(unsigned char)courseId].threeUnderParBonus;
                            break;
                        case -2:
                            opponentScoreBonus = courseScoreBonusPang[(unsigned char)courseId].twoUnderParBonus;
                            break;
                        case -1:
                            opponentScoreBonus = courseScoreBonusPang[(unsigned char)courseId].oneUnderParBonus;
                            break;
                        case 0:
                            opponentScoreBonus = courseScoreBonusPang[(unsigned char)courseId].parBonus;
                            break;
                        default:
                            opponentScoreBonus = opponentStrokesRelativeToPar *
                                                 courseScoreBonusPang[(unsigned char)courseId].fallbackScoreMultiplier;
                            break;
                        }
                    additions[0] = opponentScoreBonus;
                    if (((RewardPlayerSummary*)MATCH_PLAYER_ENTRY(opponentIndex))->finishRewardKind == 9)
                        additions[0] *= 2;
                    else if (((RewardPlayerSummary*)MATCH_PLAYER_ENTRY(opponentIndex))->finishRewardKind == 10)
                        additions[0] *= 2;
                    if (((RewardPlayerSummary*)MATCH_PLAYER_ENTRY(opponentIndex))->objectHitCount) {
                        multiplier =
                            (int)pow(kDouble_2_Hud,
                                     (double)(unsigned int)((RewardPlayerSummary*)MATCH_PLAYER_ENTRY(opponentIndex))
                                         ->objectHitCount);
                        additions[0] = multiplier * additions[0];
                    }
                    if (2 == ((RewardSelectorView*)g_sharedDoc)->multiplierSelector)
                        additions[0] *= 2;
                }
            }
        }
        if (opponentClinchedMatch)
            additions[0] += 50 * remainingHoles;
        if (roundCompletionPang > 0)
            additions[1] += roundCompletionPang;
        if (additions[0] > 0 || additions[1] > 0) {
            for (int j = 0; 4 > j; ++j) {
                if (j != playerIndex) {
                    GolfPlayer_ApplyShotPangReward(this, (unsigned char)j, (ShotPangReward*)additions);
                    break;
                }
            }
        }
    }
#undef opponentIndex
#undef roundCompletionPang
#undef MATCH_PLAYER_ENTRY
#undef MATCH_DOCUMENT_BYTES
#undef MATCH_DOCUMENT
#undef MATCH_SHARED_BYTES
}

// Reset transient reward calculation and presentation for a live CPangInfo.
// initialDisplayFlags is a PangDisplayFlags bitmask, NOT an amount of Pang;
// observed callers pass 0 (fresh calculation) or kPangRewardsCalculated (1).
// initialEntryState initializes ALL 19 PangBonusIndex entries; observed callers
// pass 1 (pending calculation) or kBonusUnavailable (0x100). Other combinations are
// unvalidated: use RewardEntryAnimationPhase only with its consumer's invariants.
// Entry schema (stride 0x2C, first at self+0xA0): +0 state, +4 Pang value,
// +8 queue order (-1=unscheduled), +C elapsed seconds, +10 current HUD Y,
// +14 target HUD Y, +18..27 retained presentation fields, +28 opacity byte.
// Writes state/value/order/time/opacity = input/0/-1/0/0; leaves the other bytes.
// Total pulse states become ready (8), elapsed seconds/earned amounts/pre-multiplier
// values become zero; completion delay and shot/bounce scheduling counters reset.
// Settled/starting Pang totals remain intact. countingSoundAlias is cleared
// without stopping a sound here; the caller owns sound sequencing. No allocation.
void CPangInfo::ResetPangAndBonusAnimation(int initialDisplayFlags, int initialEntryState) {
    PangAnimationFields* self = (PangAnimationFields*)this;
    float zero = kFloat_0_hud;
    unsigned char* s = (unsigned char*)self;

    (int&)AsCPangInfo(s)->flags = initialDisplayFlags;
    AsCPangInfo(s)->countingSoundAlias = 0;
    AsCPangInfo(s)->scheduledBonusCount = 0;
    AsCPangInfo(s)->processedTrajectoryFrame = 0;
    AsCPangInfo(s)->rewardedBounceCount = 0;
    AsCPangInfo(s)->lastRewardedBounceFrame = -1;
    AsCPangInfo(s)->pangAnimationState = kRewardReady;
    AsCPangInfo(s)->pangAnimationTime = zero;
    AsCPangInfo(s)->pangBeforeMultiplier = 0;
    AsCPangInfo(s)->earnedPang = 0;
    AsCPangInfo(s)->bonusPangAnimationState = kRewardReady;
    AsCPangInfo(s)->bonusPangAnimationTime = zero;
    AsCPangInfo(s)->bonusPangBeforeMultiplier = 0;
    AsCPangInfo(s)->earnedBonusPang = 0;
    AsCPangInfo(s)->completionDelayElapsed = zero;
    unsigned char* entryGroupBase = s;
    int initializedEntries = 0;
    for (int i = 0; 3 > i; ++i) {
        for (int j = 0; j < 6; ++j) {
            unsigned char* e = (unsigned char*)&AsCPangInfo(entryGroupBase)->bonusEntryState + j * 0x2c;
            *(int*)(e + 0x00) = initialEntryState;
            *(int*)(e + 0x04) = 0;
            *(int*)(e + 0x08) = -1;
            *(float*)(e + 0x0c) = zero;
            *(unsigned char*)(e + 0x28) = 0;
        }
        initializedEntries += 6;
        entryGroupBase += 0x108;
    }
    unsigned char* e = (unsigned char*)&AsCPangInfo(s)->bonusEntryState + initializedEntries * 0x2c;
    *(int*)(e + 0x00) = initialEntryState;
    *(int*)(e + 0x04) = 0;
    *(int*)(e + 0x08) = -1;
    *(float*)(e + 0x0c) = zero;
    zero = kFloat_0_hud;
    *(unsigned char*)(e + 0x28) = 0;
}

#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// Advance ONLY the two total-counter pulse timers (Pang and bonus Pang).
// Required live self; delta is finite nonnegative elapsed seconds. Complete flag
// makes this a no-op; otherwise BOTH timers increment even in an inactive state.
// For each total: rise(2) -> fall(4) after kPangCounterPulseRiseSeconds, fall(4)
// -> ready(8) after kFloat_0_1_HudShared seconds. Comparisons are strict >;
// transition resets elapsed to zero, discards overshoot, and crosses at most
// one phase per call. No amounts, individual-entry timers or sound are changed.
// Individual elapsed times advance in CPangInfo::UpdateShotRewardsAndBonuses; AnimateRewardEntries
// consumes them. Keep volatile reload/store order and the two separate clocks.
void CPangInfo::AdvanceRewardDisplayTimers(float delta) {
    PangAnimationFields* self = (PangAnimationFields*)this;
    if (self->flags & kPangDisplayComplete)
        return;
    float zero;
    float end;
    register float timer;
    float step;
    step = kPangCounterPulseRiseSeconds;
    zero = kFloat_0_hud;
    end = kFloat_0_1_HudShared;
    int eight = 8;
    int four = 4;
    timer = self->pangAnimationTime;
    timer += delta;
    *(volatile float*)&self->pangAnimationTime = timer;
    int state = *(volatile int*)&self->pangAnimationState;
    if (state == 2 && (timer = *(volatile float*)&self->pangAnimationTime) > step) {
        self->pangAnimationState = four;
        self->pangAnimationTime = zero;
    } else if (state == 4 && (timer = *(volatile float*)&self->pangAnimationTime) > end) {
        self->pangAnimationState = eight;
        self->pangAnimationTime = zero;
    }
    timer = self->bonusPangAnimationTime;
    timer += delta;
    *(volatile float*)&self->bonusPangAnimationTime = timer;
    state = *(volatile int*)&self->bonusPangAnimationState;
    if (state == 2 && (timer = *(volatile float*)&self->bonusPangAnimationTime) > step) {
        self->bonusPangAnimationState = four;
        self->bonusPangAnimationTime = zero;
    } else if (state == 4 && (timer = *(volatile float*)&self->bonusPangAnimationTime) > end) {
        self->bonusPangAnimationState = eight;
        self->bonusPangAnimationTime = zero;
    }
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset

// Animates total and individual pang counters, positions, opacity, and count sounds.
// Once totals settle, shifted targets move upward at 96 pixels/sec. For each
// 0x2C-byte entry, finish a queue shift before its low-bit phase: pulse 2->4->8,
// accepted transfer 0x10->0x20->0x2000, rejected fade 0x40->0x80.
// Transfer adds to settled totals exactly once. Multiplier entries 1,9..11,14
// add only their extra amount; 12,13,16..18 update bonus Pang, others regular Pang.
void CPangInfo::AnimateRewardEntries(float delta) {
    PangAnimationFields* self = (PangAnimationFields*)this;
    // Phases: shift queued target positions after totals settle, then process
    // entry movement/pulses and commit accepted transfers or rejected fades.
    unsigned char* s = (unsigned char*)self;
    unsigned int flags = AsCPangInfo(s)->flags;
    if (flags & kPangDisplayComplete)
        return;
    float base = AsCOption(g_option)->hudBaselineY;
    if (flags & kPangTotalsSettled) {
        float a = kFloat_3_Hud;
        float b = kFloat_32_0;
        float c = a * delta;
        // Sliding anchor: q starts at self+2C. Thus q+A0 is entry 1
        // (long-drive multiplier), and +CC/+F8 are entries 2/3. Each
        // entry is 2C bytes: state +00, target Y +14 (HUD pixels).
        // Advancing by 84 visits entries 1..9 in three groups; entry 0
        // is excluded. Each group's first entry keeps its unconditional store.
        unsigned char* q = (unsigned char*)&AsCPangInfo(s)->earnedBonusPang;
        for (int i = 0; i < 3; ++i) {
            *(float*)(q + 0xb4) = (*(unsigned int*)(q + 0xa0) & kRewardEntryShifting) ? -(b * c - *(float*)(q + 0xb4))
                                                                                      : *(float*)(q + 0xb4);
            if (*(unsigned int*)(q + 0xcc) & kRewardEntryShifting)
                *(float*)(q + 0xe0) = -(b * c - *(float*)(q + 0xe0));
            if (*(unsigned int*)(q + 0xf8) & kRewardEntryShifting)
                *(float*)(q + 0x10c) = -(b * c - *(float*)(q + 0x10c));
            q += 0x84;
        }
        if ((unsigned int&)AsCPangInfo(s)->ballBounceBonusState & kRewardEntryShifting)
            AsCPangInfo(s)->ballBounceBonusTargetY =
                -(kFloat_32_0 * (kFloat_3_Hud * delta) - AsCPangInfo(s)->ballBounceBonusTargetY);
        if ((unsigned int&)AsCPangInfo(s)->ballBonusState & kRewardEntryShifting)
            AsCPangInfo(s)->ballBonusTargetY =
                -(kFloat_32_0 * (kFloat_3_Hud * delta) - AsCPangInfo(s)->ballBonusTargetY);
        if ((unsigned int&)AsCPangInfo(s)->powerShotBonusState & kRewardEntryShifting)
            AsCPangInfo(s)->powerShotBonusTargetY =
                -(kFloat_32_0 * (kFloat_3_Hud * delta) - AsCPangInfo(s)->powerShotBonusTargetY);
        if ((unsigned int&)AsCPangInfo(s)->pangMultiplierState & kRewardEntryShifting)
            AsCPangInfo(s)->pangMultiplierTargetY =
                -(kFloat_32_0 * (kFloat_3_Hud * delta) - AsCPangInfo(s)->pangMultiplierTargetY);
    }
    float zero = kFloat_0_hud, scale = kFloat_3_Hud, width = kFloat_32_0;
    float fade = kFloat_20_MessageUI, maxalpha = kFloat_255_MessageUI, threshold = kFloat_0_5_hud;
    float baselineOffsetPx = kFloat_108_hud;
    float one = kFloat_1_805DE314;
    float five = kFloat_5_Hud;
    double bias = kSignedToDoubleBias_PangBonusRendering;
    unsigned int i = 0;
    unsigned char* p = s;
    for (; (int)i < kBonusEntryCount; ++i, p += 0x2c) {
        unsigned int* state = (unsigned int*)((unsigned char*)&AsCPangInfo(p)->bonusEntryState);
        unsigned int raw = *state;
        if (raw & kRewardEntryShifting) {
            float step = delta / threshold;
            float t = -(fade * step - AsCPangInfo(p)->bonusEntryY);
            AsCPangInfo(p)->bonusEntryY = t;
            if (t <= AsCPangInfo(p)->bonusEntryTargetY)
                *state &= ~(unsigned int)kRewardEntryShifting;
            continue;
        }
        switch ((int)(raw & kBonusStateMask)) {
        case kRewardPulseRise:
            if (AsCPangInfo(p)->bonusEntryAnimationTime > threshold) {
                *state = kRewardPulseFall;
                AsCPangInfo(p)->bonusEntryAnimationTime = zero;
            }
            break;
        case kRewardPulseFall:
            if (AsCPangInfo(p)->bonusEntryAnimationTime > threshold) {
                *state = kRewardReady;
                AsCPangInfo(p)->bonusEntryAnimationTime = zero;
            }
            break;
        case 3:
        case kRewardHidden:
        case kBonusUnavailable:
        case kRewardEntrySettled:
            break;
        case kRewardTransfer: {
            if (!AsCPangInfo(s)->countingSoundAlias) {
                if (AsCPangInfo(s)->earnedPang > 0)
                    *(const char**)(s + 0x24) = (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, pangUpSound));
                else
                    *(const char**)(s + 0x24) = (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, pangDownSound));
                if (AsCPangInfo(s)->countingSoundAlias &&
                    !SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, *(const char**)(s + 0x24)))
                    Sound_PlayByName(g_soundManager, *(const char**)(s + 0x24), 0, 1, *(const char**)(s + 0x24), 0,
                                     kFloat_0_5_hud, kFloat_200_Hud, kFloat_1_805DE314);
            }
            float startFade = base - baselineOffsetPx;
            float next = width * (scale * delta) + AsCPangInfo(p)->bonusEntryY;
            AsCPangInfo(p)->bonusEntryY = next;
            if (next > fade + startFade) {
                switch (i) {
                case 1: {
                    int v = AsCPangInfo(p)->bonusEntryValue;
                    int old = AsCPangInfo(s)->bonusEntryValue;
                    int x = AsCPangInfo(s)->settledPang + old * (v - 1);
                    AsCPangInfo(s)->settledPang = x;
                    AsCPangInfo(s)->displayedPang = x;
                    break;
                }
                case 8: {
                    int x = AsCPangInfo(s)->settledPang + AsCPangInfo(s)->scoreBonusPang;
                    x &= ~(x >> 31);
                    AsCPangInfo(s)->settledPang = x;
                    AsCPangInfo(s)->displayedPang = x;
                    break;
                }
                case 9:
                case 10:
                case 11: {
                    int x = AsCPangInfo(s)->settledPang +
                            AsCPangInfo(s)->scoreBonusPang * (AsCPangInfo(p)->bonusEntryValue - 1);
                    AsCPangInfo(s)->settledPang = x;
                    AsCPangInfo(s)->displayedPang = x;
                    break;
                }
                case 14: {
                    int v = AsCPangInfo(s)->pangBeforeMultiplier;
                    if (v > 0) {
                        int x = AsCPangInfo(s)->settledPang + v;
                        AsCPangInfo(s)->settledPang = x;
                        AsCPangInfo(s)->displayedPang = x;
                    }
                    v = AsCPangInfo(s)->bonusPangBeforeMultiplier;
                    if (v > 0) {
                        int x = AsCPangInfo(s)->settledBonusPang + v;
                        AsCPangInfo(s)->settledBonusPang = x;
                        AsCPangInfo(s)->displayedBonusPang = x;
                    }
                    break;
                }
                case 12:
                case 13:
                case 16:
                case 17:
                case 18: {
                    int x = AsCPangInfo(s)->settledBonusPang + AsCPangInfo(p)->bonusEntryValue;
                    AsCPangInfo(s)->settledBonusPang = x;
                    AsCPangInfo(s)->displayedBonusPang = x;
                    break;
                }
                default: {
                    int x = AsCPangInfo(s)->settledPang + AsCPangInfo(p)->bonusEntryValue;
                    AsCPangInfo(s)->settledPang = x;
                    AsCPangInfo(s)->displayedPang = x;
                    break;
                }
                }
                *state = kRewardTransferFinished;
                AsCPangInfo(p)->bonusEntryOpacity = 0;
                AsCPangInfo(s)->pangAnimationState = 2;
                AsCPangInfo(s)->pangAnimationTime = zero;
                AsCPangInfo(s)->bonusPangAnimationState = 2;
                AsCPangInfo(s)->bonusPangAnimationTime = zero;
            } else {
                float cur = AsCPangInfo(p)->bonusEntryY;
                if (cur > startFade) {
                    float fraction = (cur - startFade) / fade;
                    if (fraction > one)
                        fraction = one;
                    float alpha =
                        maxalpha * (((fade + (base - baselineOffsetPx)) - AsCPangInfo(p)->bonusEntryY) / fade);
                    if (alpha < zero)
                        alpha = zero;
                    if (alpha > maxalpha)
                        alpha = maxalpha;
                    AsCPangInfo(p)->bonusEntryOpacity = (int)alpha;
                    switch (i) {
                    case 1:
                        AsCPangInfo(s)->displayedPang =
                            AsCPangInfo(s)->settledPang +
                            (int)((float)AsCPangInfo(s)->bonusEntryValue *
                                  (fraction * (float)(AsCPangInfo(p)->bonusEntryValue - 1)));
                        break;
                    case 9:
                    case 10:
                    case 11:
                        AsCPangInfo(s)->displayedPang =
                            AsCPangInfo(s)->settledPang +
                            (int)((float)AsCPangInfo(s)->scoreBonusPang *
                                  (fraction * (float)(AsCPangInfo(p)->bonusEntryValue - 1)));
                        break;
                    case 8: {
                        int x = AsCPangInfo(s)->settledPang + (int)((float)AsCPangInfo(s)->scoreBonusPang * fraction);
                        AsCPangInfo(s)->displayedPang = x & ~(x >> 31);
                        break;
                    }
                    case 14: {
                        float amount = fraction * (float)(AsCPangInfo(p)->bonusEntryValue - 1);
                        int dx = (int)((float)AsCPangInfo(s)->pangBeforeMultiplier * amount);
                        int dy = (int)((float)AsCPangInfo(s)->bonusPangBeforeMultiplier * amount);
                        AsCPangInfo(s)->displayedPang = AsCPangInfo(s)->settledPang + dx;
                        AsCPangInfo(s)->displayedBonusPang = AsCPangInfo(s)->settledBonusPang + dy;
                        break;
                    }
                    case 12:
                    case 13:
                    case 16:
                    case 17:
                    case 18:
                        AsCPangInfo(s)->displayedBonusPang =
                            AsCPangInfo(s)->settledBonusPang + (int)(fraction * (float)AsCPangInfo(p)->bonusEntryValue);
                        break;
                    default:
                        AsCPangInfo(s)->displayedPang =
                            AsCPangInfo(s)->settledPang + (int)(fraction * (float)AsCPangInfo(p)->bonusEntryValue);
                        break;
                    }
                }
            }
            break;
        }
        case kRewardTransferFinished: {
            *state = kRewardEntrySettled;
            if (AsCPangInfo(s)->countingSoundAlias)
                Sound_StopByAlias(g_soundManager, (const char*)AsCPangInfo(s)->countingSoundAlias);
            break;
        }
        case kRewardRejectFade: {
            int n = AsCPangInfo(p)->bonusEntryOrder;
            float time = AsCPangInfo(p)->bonusEntryAnimationTime;
            float t = -(fade * (float)n - (base - baselineOffsetPx));
            t = -(width * (five * time) - t);
            AsCPangInfo(p)->bonusEntryY = t;
            float alpha = maxalpha * (threshold - AsCPangInfo(p)->bonusEntryAnimationTime);
            if (alpha < zero)
                alpha = zero;
            AsCPangInfo(p)->bonusEntryOpacity = (int)alpha;
            if (AsCPangInfo(p)->bonusEntryAnimationTime > threshold)
                *state = kRewardHidden;
            break;
        }
        default:
            break;
        }
    }
}

// Marks bonus entries and pang totals settled once ball flight and reward animations finish.
void CPangInfo::UpdateSettlementFlags() {
    PangAnimationFields* self = (PangAnimationFields*)this;
    if (((PangBallStatusFields*)GolfBall__Fv())->value != 0)
        return;
    int flags = self->flags;
    if (!(flags & 8)) {
        if (flags & 4) {
            self->flags = flags | 8;
            self->bannerLayoutCount = 0;
            if (self->ballBonusState != kBonusUnavailable)
                self->ballBonusBannerElapsed = kFloat_0_hud;
            if (self->holeInOneBonusState == kBonusUnavailable)
                return;
            self->holeInOneBannerElapsed = kFloat_0_hud;
            return;
        }
        self->flags = flags | 4;
        unsigned char* p = (unsigned char*)self;
        for (int i = 0; i < kBonusEntryCount; ++i, p += 0x2c) {
            if ((unsigned int&)AsCPangInfo(p)->bonusEntryState & 0x1007) {
                self->flags &= ~4u;
                break;
            }
        }
    }
}

// Tracks when reward animations have settled before marking the display complete.
void CPangInfo::UpdateCompletionDelay(float delta) {
    PangAnimationFields* self = (PangAnimationFields*)this;
    if (((PangBallStatusFields*)GolfBall__Fv())->value != 0 || (self->flags & kPangDisplayComplete))
        return;
    if (self->ballBonusState != kBonusUnavailable && self->ballBonusBannerElapsed < kFloat_1_805DE314)
        return;
    if (self->holeInOneBonusState != kBonusUnavailable && self->holeInOneBannerElapsed < kFloat_1_805DE314)
        return;
    float zero = kFloat_0_hud;
    if (zero == self->completionDelayElapsed) {
        unsigned int* p = (unsigned int*)self;
        for (int i = 0; i < kBonusEntryCount; ++i) {
            int v = AsCPangInfo(p)->bonusEntryState;
            if (v != 0x80 && v != kBonusUnavailable && v != 0x2000)
                return;
            p = (unsigned int*)((unsigned char*)&AsCPangInfo(p)->earnedBonusPang);
        }
    }
    float value = self->completionDelayElapsed + delta;
    if (!(value > kFlightSecondaryPinFadeStartTime)) {
        self->completionDelayElapsed = value;
    } else {
        self->flags |= 0x10;
        self->completionDelayElapsed = value;
    }
}
// Pang animations and display.

struct State0 {
    char pad[0x18];
    int field18;
};
struct PangManager {
    char pad[0x8C0];
    unsigned char mode;
    char pad2[0x2CF];
    void* data;
};
struct ActorA {
    char pad[0x3B6];
    unsigned char index;
};

struct Entry {
    char pad0[0x48];
    unsigned int flags;
    int count;
    char pad1[4];
    float timer;
    float y;
    char pad2[0x14];
    unsigned char alpha;
    char pad3[3];
    float baseTimer;
};

// Queue projection: base=self+category*2C, NOT the record start. Real fields
// are +A0 state, +A4 value, +A8 queue order, +AC animation seconds, +B0 current Y,
// +B4 target Y. A shift sets bit 1000 and moves currentY toward targetY in
// AnimateRewardEntries; smaller order is nearer hudBaselineY-108, 20 pixels/row.
struct Slot {
    char pad0[0xA0];
    unsigned int flags;
    char pad1[4];
    int queueOrder;
    char pad2[4];
    float currentY;
    float targetY;
};
enum {
    kBonusRecordStride = 0x2C
};

// Queues a bonus entry and shifts its display position behind earlier entries.
void CPangInfo::ScheduleBonusEntry(int category) {
    // Require category 0..18. This publishes pulse-rise state even if already
    // scheduled; callers test state==1. It increments scheduledBonusCount once.
    // Insertion policy, in priority order (earlier cases take precedence):
    //  long drive: order 0, shift active entries 1..18;
    //  score + available object multiplier: take multiplier's old order and
    //      move that multiplier/later rows up one slot;
    //  spin/curve: take scoreOrder+1, shift active rows after score;
    //  no Pang multiplier OR round completion: append at scheduledBonusCount;
    //  otherwise: take Pang multiplier's order and move it up one slot.
    // Active for shifting means (state & 0x101)==0, not state!=Unavailable.
    // This does not reset the entry timer/opacity; reset/animation own those.
    // Hole-in-one/ball also publish a banner start in viewport pixels.
    unsigned char* p = (unsigned char*)this;
    float hudBaselineY = AsCOption(g_option)->hudBaselineY;
    if (category == kBonusLongDrive) {
        int firstLaterCategory = category + 1;
        float base = hudBaselineY - kFloat_108_hud;
        Slot* slot = (Slot*)(p + firstLaterCategory * kBonusRecordStride);
        int remaining = kBonusEntryCount - firstLaterCategory;
        for (; remaining > 0; --remaining) {
            if ((slot->flags & (kBonusUnavailable | kRewardUnscheduled)) == 0) {
                slot->flags |= kRewardEntryShifting;
                int nextOrder = slot->queueOrder + 1;
                slot->queueOrder = nextOrder;
                slot->targetY = base - kFloat_20_MessageUI * (float)nextOrder;
            }
            slot = (Slot*)((unsigned char*)slot + kBonusRecordStride);
        }
        AsCPangInfo(p)->bonusEntryOrder = 0;
    } else if (category == kBonusScore && AsCPangInfo(p)->objectHitMultiplierState != kBonusUnavailable) {
        float base = hudBaselineY - kFloat_108_hud;
        Slot* slot = (Slot*)p;
        int remaining = kBonusEntryCount;
        for (; remaining > 0; --remaining) {
            if ((slot->flags & (kBonusUnavailable | kRewardUnscheduled)) == 0 &&
                slot->queueOrder > AsCPangInfo(p)->objectHitMultiplierOrder) {
                slot->flags |= kRewardEntryShifting;
                int nextOrder = slot->queueOrder + 1;
                slot->queueOrder = nextOrder;
                slot->targetY = base - kFloat_20_MessageUI * (float)nextOrder;
            }
            slot = (Slot*)((unsigned char*)&AsCPangInfo(slot)->earnedBonusPang);
        }
        int count = AsCPangInfo(p)->objectHitMultiplierOrder;
        AsCPangInfo(p)->scoreBonusOrder = count;
        AsCPangInfo(p)->objectHitMultiplierState |= kRewardEntryShifting;
        AsCPangInfo(p)->objectHitMultiplierOrder = count + 1;
        AsCPangInfo(p)->objectHitMultiplierTargetY =
            hudBaselineY - kFloat_108_hud - kFloat_20_MessageUI * (float)(count + 1);
    } else if (category == kBonusPowerSpin || category == kBonusPowerCurve) {
        float base = hudBaselineY - kFloat_108_hud;
        Slot* slot = (Slot*)p;
        int remaining = kBonusEntryCount;
        for (; remaining > 0; --remaining) {
            if ((slot->flags & (kBonusUnavailable | kRewardUnscheduled)) == 0 &&
                slot->queueOrder > AsCPangInfo(p)->scoreBonusOrder) {
                slot->flags |= kRewardEntryShifting;
                int nextOrder = slot->queueOrder + 1;
                slot->queueOrder = nextOrder;
                slot->targetY = base - kFloat_20_MessageUI * (float)nextOrder;
            }
            slot = (Slot*)((unsigned char*)&AsCPangInfo(slot)->earnedBonusPang);
        }
        ((Slot*)(p + category * kBonusRecordStride))->queueOrder = AsCPangInfo(p)->scoreBonusOrder + 1;
    } else if (AsCPangInfo(p)->pangMultiplierState == kBonusUnavailable || category == kBonusRoundCompletion) {
        ((Slot*)(p + category * kBonusRecordStride))->queueOrder = AsCPangInfo(p)->scheduledBonusCount;
    } else {
        float base = hudBaselineY - kFloat_108_hud;
        Slot* e = (Slot*)(p + category * kBonusRecordStride);
        e->queueOrder = AsCPangInfo(p)->pangMultiplierOrder;
        AsCPangInfo(p)->pangMultiplierState |= kRewardEntryShifting;
        ++AsCPangInfo(p)->pangMultiplierOrder;
        AsCPangInfo(p)->pangMultiplierTargetY = base - kFloat_20_MessageUI * (float)AsCPangInfo(p)->pangMultiplierOrder;
    }
    Slot* current = (Slot*)(p + category * kBonusRecordStride);
    ++AsCPangInfo(p)->scheduledBonusCount;
    float base = hudBaselineY - kFloat_108_hud;
    current->currentY = base - kFloat_20_MessageUI * (float)current->queueOrder;
    current->flags = kRewardPulseRise;
    if (category == kBonusHoleInOne || category == kBonusBall) {
        float val = AsWView(g_view)->viewportWidth - kFloat_28_GolfHud - kFloat_192_6_GolfHud;
        AsCPangInfo(p)->bonusBannerStartX = (float)floor(val);
        AsCPangInfo(p)->bonusBannerStartY = current->currentY - kFloat_13_440001_GolfHud;
    }
}

// Tracks long-putt direction and distance, then accepts or rejects the final bonus.
void CPangInfo::UpdateLongPuttBonus() {
    unsigned char* p = (unsigned char*)this;
    if (AsCPangInfo(p)->longPuttBonusState == kBonusUnavailable)
        return;
    unsigned char* sys = (unsigned char*)GolfBall__Fv();
    if (((CGolfBall*)sys)->m_state != 0) {
        if (AsCPangInfo(p)->longPuttBonusState == 1) {
            sys = (unsigned char*)GolfBall__Fv();
            if (((CGolfBall*)sys)->m_eventPreDataIndex[0] > 0) {
                float vec[3];
                void* idx = PreData__9CGolfBallFi(GolfBall__Fv(), 0);

                __mi__FRC7WVectorRC7WVector(vec, GolfBall__Fv(), idx);
                vec[1] = kFloat_0_hud;
                (*(WVector*)vec).Normalize();
                float limit = Math_CosApprox(0.3490658402442932f);
                if (Vec3_DotProduct(p + 0x3E4, vec) > limit)
                    ScheduleBonusEntry(kBonusLongPutt);
                return;
            }
        }
        float x = AsCPangInfo(p)->shotTravelDistanceYards;
        AsCPangInfo(p)->longPuttBonusPang = ((int)x) * 2;
        return;
    }
    if ((AsCPangInfo(p)->flags & 4) == 0)
        return;
    sys = (unsigned char*)GolfBall__Fv();
    int r = *(unsigned int*)&((CGolfBall*)sys)->m_holeInCrypt[8];
    int l = *(unsigned int*)&((CGolfBall*)sys)->m_holeInCrypt[0];

    (unsigned int&)AsCPangInfo(p)->longPuttBonusState =
        ((((unsigned int&)AsCPangInfo(p)->longPuttBonusState & ~(unsigned int)kBonusStateMask) |
          (((~((l ^ r))) == 1) ? 0x10 : 0x40)));
    AsCPangInfo(p)->longPuttBonusAnimationTime = 0.0f;
}

// Updates the chip-in reward from shot travel and the final cup result.
void CPangInfo::UpdateChipInBonus() {
    unsigned char* p = (unsigned char*)this;
    if (AsCPangInfo(p)->chipInBonusState == kBonusUnavailable)
        return;
    unsigned char* sys = (unsigned char*)GolfBall__Fv();
    if (((CGolfBall*)sys)->m_state != 0) {
        if (AsCPangInfo(p)->chipInBonusState == 1) {
            sys = (unsigned char*)GolfBall__Fv();
            if (((CGolfBall*)sys)->m_eventPreDataIndex[0] > 0) {
                void* idx = PreData__9CGolfBallFi(GolfBall__Fv(), 0);

                float vec[3];
                __mi__FRC7WVectorRC7WVector(vec, GolfBall__Fv(), idx);
                vec[1] = kFloat_0_hud;
                (*(WVector*)vec).Normalize();

                if (Vec3_DotProduct(p + 0x3E4, vec) > (Math_CosApprox(kFloat_0_34906584_Hud)) &&
                    AsCPangInfo(p)->remainingDistanceYards < kFloat_30_GolfHud)
                    ScheduleBonusEntry(kBonusChipIn);
                return;
            }
        }
        AsCPangInfo(p)->chipInBonusPang = ((int)AsCPangInfo(p)->shotTravelDistanceYards) * 2;
        sys = (unsigned char*)GolfBall__Fv();
        if (((unsigned int&)((CGolfBall*)sys)->m_powerFlags & 0x70) == 0 &&
            *((unsigned char*)InstanceTable_Get() + 0x48) == 4)
            AsCPangInfo(p)->chipInBonusPang *= 2;
        return;
    }
    if (AsCPangInfo(p)->chipInBonusState == 1)
        (unsigned int&)AsCPangInfo(p)->chipInBonusState = kBonusUnavailable;
    if ((AsCPangInfo(p)->flags & 4) == 0)
        return;
    sys = (unsigned char*)GolfBall__Fv();

    // Complemented XOR encoding: exactly 1 means holed. Preserve the upper
    // queue-shift bits while replacing only the low twelve animation bits.
    int encodedHoleValue = *(unsigned int*)&((CGolfBall*)sys)->m_holeInCrypt[0];
    int holeXorKey = *(unsigned int*)&((CGolfBall*)sys)->m_holeInCrypt[8];
    (unsigned int&)AsCPangInfo(p)->chipInBonusState =
        ((unsigned int&)AsCPangInfo(p)->chipInBonusState & ~(unsigned int)kBonusStateMask) |
        (~(encodedHoleValue ^ holeXorKey) == 1 ? kBonusAccepted : kBonusRejected);
    AsCPangInfo(p)->chipInBonusAnimationTime = 0.0f;
}

// Awards recovery pang based on the previous and current shot positions and terrain.
void CPangInfo::UpdateRecoveryBonus() {
    unsigned char* p = (unsigned char*)this;
    int state = AsCPangInfo(p)->recoveryBonusState;
    if (state == kBonusUnavailable)
        return;
    if (state == 1) {
        int index = ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[7];
        void* second = GolfBall__Fv();
        if (((CGolfBall*)second)->m_eventPreDataIndex[0] <= index)
            return;
        ScheduleBonusEntry(kBonusRecovery);
        unsigned char* manager = (unsigned char*)g_sharedDoc;
        unsigned char* actor = ((unsigned char*)AsCSharedDoc(manager)->golfDoc);
        void* first = GolfBall__Fv();
        second = GolfBall__Fv();
        void* item = PreData__9CGolfBallFi(second, ((CGolfBall*)first)->m_eventPreDataIndex[7]);
        int result =
            CourseData_IsPointOutsidePlayableRegion(*(void**)(((unsigned char*)&AsCGolfDoc(actor)->courseData)), item);
        if (result)
            AsCPangInfo(p)->recoveryBonusPang = 0xF;
        else
            AsCPangInfo(p)->recoveryBonusPang = 0xA;
        return;
    }
    if ((AsCPangInfo(p)->flags & 4) == 0)
        return;
    unsigned char* manager = (unsigned char*)g_sharedDoc;
    unsigned char* actor = ((unsigned char*)AsCSharedDoc(manager)->golfDoc);
    void* first = GolfBall__Fv();
    void* second = GolfBall__Fv();
    void* item = PreData__9CGolfBallFi(second, ((CGolfBall*)first)->m_eventPreDataIndex[7]);
    if (CourseData_IsPointOutsidePlayableRegion(*(void**)(8 + actor), item)) {
        manager = (unsigned char*)g_sharedDoc;
        actor = ((unsigned char*)AsCSharedDoc(manager)->golfDoc);
        first = GolfBall__Fv();
        second = GolfBall__Fv();
        item = PreData__9CGolfBallFi(second, ((CGolfBall*)first)->m_preDataCount);
        int result2 =
            CourseData_IsPointOutsidePlayableRegion(*(void**)(((unsigned char*)&AsCGolfDoc(actor)->courseData)), item);

        int mode = result2 ? 0x40 : 0x10;
        unsigned int oldState = (unsigned int&)AsCPangInfo(p)->recoveryBonusState;
        (unsigned int&)AsCPangInfo(p)->recoveryBonusState = (oldState & ~(unsigned int)kBonusStateMask) | mode;
        AsCPangInfo(p)->recoveryBonusAnimationTime = kFloat_0_hud;
    } else {
        unsigned int triple[3];
        first = GolfBall__Fv();
        second = GolfBall__Fv();
        item = PreData__9CGolfBallFi(second, ((CGolfBall*)first)->m_preDataCount);
        unsigned int* ip = (unsigned int*)item;

        triple[0] = ip[0];
        triple[1] = ip[1];
        triple[2] = ip[2];
        void* table = CQuadTree_GetInstance();
        unsigned int value = (unsigned char)GetGroundType__9CQuadTreeF7WVector(table, triple);
        int isMatch = 0;
        unsigned int rawState = (unsigned int&)AsCPangInfo(p)->recoveryBonusState;
        if (1 == value)
            isMatch = 1;
        else if (value == 2)
            isMatch = 1;
        int mode = 0x40;
        if (0 != isMatch)
            mode = 0x10;
        (unsigned int&)AsCPangInfo(p)->recoveryBonusState = (rawState & ~(unsigned int)kBonusStateMask) | mode;
        AsCPangInfo(p)->recoveryBonusAnimationTime = kFloat_0_hud;
    }
}

// Accepts or rejects the score bonus after the ball stops, including match-play comparison.
void CPangInfo::UpdateScoreBonusState() {
    unsigned char* p = (unsigned char*)this;
    if (AsCPangInfo(p)->scoreBonusState != kBonusUnavailable) {
        unsigned char* sys = (unsigned char*)GolfBall__Fv();
        switch (((CGolfBall*)sys)->m_state) {
        case 0:
            break;
        default:
            return;
        }
        int current = AsCPangInfo(p)->scoreBonusState;
        if (current == 1) {
            ScheduleBonusEntry(kBonusScore);
            return;
        }
        if ((AsCPangInfo(p)->flags & 4) == 0)
            return;
        PangManager* m = (PangManager*)g_sharedDoc;
        if (m->mode == 1) {
            int a = ((CGolfDoc*)(m->data))->ResolveMatchTurnFromHoleStrokes();
            ActorA* actor = (ActorA*)((PangManager*)g_sharedDoc)->data;
            int ix = actor->index;
            void* first = GolfPlayer_GetEntryByIndex(actor, (unsigned char)(1 - a));
            actor = (ActorA*)((PangManager*)g_sharedDoc)->data;
            int jx = actor->index;
            void* second = GolfPlayer_GetEntryByIndex(actor, (unsigned char)a);
            if (*((unsigned char*)second + jx + 0x28) <= *((unsigned char*)first + ix + 0x28)) {
                unsigned int state = (unsigned int&)AsCPangInfo(p)->scoreBonusState;
                AsCPangInfo(p)->scoreBonusAnimationTime = kFloat_0_hud;
                (unsigned int&)AsCPangInfo(p)->scoreBonusState = (state & ~(unsigned int)kBonusStateMask) | 0x10;
            } else {
                unsigned int state = (unsigned int&)AsCPangInfo(p)->scoreBonusState;
                AsCPangInfo(p)->scoreBonusAnimationTime = kFloat_0_hud;
                (unsigned int&)AsCPangInfo(p)->scoreBonusState = (state & ~(unsigned int)kBonusStateMask) | 0x40;
            }
        } else {
            (unsigned int&)AsCPangInfo(p)->scoreBonusState = (current & ~(unsigned int)kBonusStateMask) | 0x10;
            AsCPangInfo(p)->scoreBonusAnimationTime = kFloat_0_hud;
        }
    }
}

// Updates a selected bonus entry after the ball stops and resolves its acceptance state.
void CPangInfo::UpdateConditionalBonusEntry(int index, int value) {
    unsigned char* p = (unsigned char*)this;
    int offset = index * 0x2C;
    unsigned char* item = (unsigned char*)((unsigned int)p + offset);
    if (*(int*)(item + 0xA0) == kBonusUnavailable)
        return;
    if (((CGolfBall*)GolfBall__Fv())->m_state == 0) {
        int state = *(int*)(item + 0xA0);
        if (state == 1) {
            ScheduleBonusEntry(index);
            *(int*)(item + 0xA4) = value;
        } else if (AsCPangInfo(p)->flags & 4) {
            if (index == 1) {
                int mode = 0x40;
                unsigned int base = state & ~(unsigned int)kBonusStateMask;
                if ((unsigned int&)AsCPangInfo(p)->bonusEntryState & 0x10)
                    mode = 0x10;
                *(unsigned int*)(p + offset + 0xA0) = base | mode;
                *(float*)(p + offset + 0xAC) = kFloat_0_hud;
            } else if (AsCSharedDoc(g_sharedDoc)->golfRuleType == 1) {
                int mode = 0x40;
                unsigned int base = state & ~(unsigned int)kBonusStateMask;
                if (AsCPangInfo(p)->scoreBonusState != kBonusUnavailable)
                    mode = 0x10;
                *(unsigned int*)(p + offset + 0xA0) = base | mode;
                *(float*)(p + offset + 0xAC) = kFloat_0_hud;
            } else {
                unsigned char* ball = (unsigned char*)GolfBall__Fv();
                int matches = 0;
                int left = *(int*)&((CGolfBall*)ball)->m_holeInCrypt[0];
                int right = *(int*)&((CGolfBall*)ball)->m_holeInCrypt[8];
                int result = ~(left ^ right);
                if (result == 1 && AsCPangInfo(p)->scoreRelativeToPar <= 0)
                    matches = 1;
                int mode = 0x40;
                if (matches)
                    mode = 0x10;
                *(unsigned int*)(p + offset + 0xA0) =
                    (*(unsigned int*)(p + offset + 0xA0) & ~(unsigned int)kBonusStateMask) | mode;
                *(float*)(p + offset + 0xAC) = kFloat_0_hud;
            }
        }
    }
}

// Doubles the object-hit multiplier for matching trajectory collision records, with a cap.
void CPangInfo::UpdateObjectHitMultiplier() {
    unsigned char* p = (unsigned char*)this;
    if (AsCPangInfo(p)->objectHitMultiplierState == kBonusUnavailable)
        return;
    unsigned char* sys = (unsigned char*)GolfBall__Fv();
    if (((CGolfBall*)sys)->m_state != 0) {
        unsigned char* manager = (unsigned char*)CQuadTree_GetInstance();
        int i = AsCPangInfo(p)->processedTrajectoryFrame;
        while (i < ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0]) {
            struct MatchEntry {
                short unused;
                short id;
                int unused4;
                int index;
                char pad[0x18];
            };
            MatchEntry* end = *(MatchEntry**)(manager + 0x21E8) + *(int*)(manager + 0x21EC);
            MatchEntry* entry = *(MatchEntry**)(manager + 0x21E8);
            for (; entry != end; ++entry) {
                if (entry->id == -1 || i != entry->index)
                    continue;
                int doubled = AsCPangInfo(p)->objectHitMultiplier * 2;
                int value = doubled > 0x2000 ? 0x2000 : doubled;
                (unsigned int&)AsCPangInfo(p)->objectHitMultiplier = value;
                if (value > 1 && AsCPangInfo(p)->objectHitMultiplierState == 1)
                    ScheduleBonusEntry(kBonusObjectHitMultiplier);
                break;
            }
            i = (i + 1);
        }
        return;
    }
    if ((AsCPangInfo(p)->flags & 4) == 0)
        return;
    sys = (unsigned char*)GolfBall__Fv();
    unsigned int raw = (unsigned int&)AsCPangInfo(p)->objectHitMultiplierState;
    int l = *(int*)&((CGolfBall*)sys)->m_holeInCrypt[0];
    int matches = 0;
    int r = *(int*)&((CGolfBall*)sys)->m_holeInCrypt[8];
    unsigned int base = raw & ~(unsigned int)kBonusStateMask;
    int equal = ~(l ^ r);
    if (equal == 1 && AsCPangInfo(p)->scoreRelativeToPar <= 0)
        matches = 1;
    int mode = 0x40;
    if (matches)
        mode = 0x10;
    float zero = kFloat_0_hud;
    (unsigned int&)AsCPangInfo(p)->objectHitMultiplierState = base | mode;
    AsCPangInfo(p)->objectHitMultiplierAnimationTime = zero;
}

// Counts eligible ball bounces for equipped bonus balls and resolves the final reward.
void CPangInfo::UpdateBallBounceBonus() {
    unsigned char* p = (unsigned char*)this;
    unsigned char* manager = (unsigned char*)g_sharedDoc;
    unsigned char* actor = ((unsigned char*)AsCSharedDoc(manager)->golfDoc);
    void* selected = ItemInventory_GetBallIdAt((unsigned char*)&AsCSharedDoc(manager)->itemInventory, 12);
    unsigned char* slot = manager + AsCGolfDoc(actor)->currentPlayer * 0x1D4;
    if (selected != *(void**)(slot + 0x88)) {
        actor = ((unsigned char*)AsCSharedDoc(manager)->golfDoc);
        manager = (unsigned char*)g_sharedDoc;
        void* selected2 = ItemInventory_GetBallIdAt((unsigned char*)&AsCSharedDoc(manager)->itemInventory, 0x80);
        unsigned char* slot2 = manager + AsCGolfDoc(actor)->currentPlayer * 0x1D4;
        if (selected2 != *(void**)(slot2 + 0x88))
            return;
    }
    if (AsCPangInfo(p)->ballBounceBonusState == kBonusUnavailable) {
        return;
    } else {
        if (((CGolfBall*)GolfBall__Fv())->m_state != 0) {
            int i = AsCPangInfo(p)->processedTrajectoryFrame;
            while (i < ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] &&
                   AsCPangInfo(p)->rewardedBounceCount < 20) {
                if (i - AsCPangInfo(p)->lastRewardedBounceFrame > 3) {
                    if (*((unsigned char*)PreData__9CGolfBallFi(GolfBall__Fv(), i) + 0x60) == 4 ||
                        *((unsigned char*)PreData__9CGolfBallFi(GolfBall__Fv(), i) + 0x60) == 1 ||
                        *((unsigned char*)PreData__9CGolfBallFi(GolfBall__Fv(), i) + 0x60) == 2) {
                        ++AsCPangInfo(p)->rewardedBounceCount;
                        AsCPangInfo(p)->lastRewardedBounceFrame = i;
                    }
                }
                i++;
            }
            AsCPangInfo(p)->ballBounceBonusPang = AsCPangInfo(p)->rewardedBounceCount;
            if (AsCPangInfo(p)->rewardedBounceCount > 0 && AsCPangInfo(p)->ballBounceBonusState == 1)
                ScheduleBonusEntry(kBonusBallBounce);
        } else if ((AsCPangInfo(p)->flags & 4) != 0) {
            unsigned int noRecoveryRequired;
            unsigned int upperStateBits;

            noRecoveryRequired = ((CGolfBall*)GolfBall__Fv())->m_recoveryRequired == 0;
            upperStateBits = (unsigned int&)AsCPangInfo(p)->ballBounceBonusState & ~(unsigned int)kBonusStateMask;
            unsigned int resolutionPhase;
            resolutionPhase = kBonusRejected;
            resolutionPhase = (noRecoveryRequired) ? kBonusAccepted : resolutionPhase;
            // Operand order retains the original register/data-flow pairing.
            unsigned int resolvedState = resolutionPhase | upperStateBits;
            float zero = kFloat_0_hud;
            (unsigned int&)AsCPangInfo(p)->ballBounceBonusState = resolvedState;
            AsCPangInfo(p)->ballBounceBonusAnimationTime = zero;
        }
    }
}

// Draws animated pang totals, individual bonus labels, and player reward panels.
void CPangInfo::Draw() {
    unsigned char* p = (unsigned char*)this;
    int phase = AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->tutorialStage;
    if (phase < 6) {
        if (phase != 5)
            return;
    }
    int v = AsCPangInfo(p)->displayedPang;
    if (v < AsCPangInfo(p)->displayedBonusPang)
        v = AsCPangInfo(p)->displayedBonusPang;
    float scale = v > 10000 ? kFloat_105_GolfHud : (v > 1000 ? kFloat_85_GolfHud : kFloat_65_GolfHud);
    union {
        struct {
            unsigned long long a2, b2, c2, a1, b1, c1;
            float pos2[2], pos1[2], xy[2], size[2];
        };
    };
    unsigned char* row = p;
    unsigned char* text = p + 0x60;
    const float totalBase = kFloat_0_6_805DE33C;
    const float totalStep = kFloat_0_1_HudShared;
    const float totalTime = kPangCounterPulseRiseSeconds;
    const float totalCap = kDriveHudVerticalLayoutScale;
    for (int i = 0; i < 2; ++i, row += 0x2C, text += 0x2C) {
        Entry* e = (Entry*)row;
        if (i == 1 && e->count == 0)
            continue;
        float pulseScale;
        if (AsCPangInfo(row)->pangAnimationState == 2) {
            pulseScale = totalBase + totalStep * e->timer / totalTime;
            if (pulseScale > totalCap)
                pulseScale = totalCap;
        } else if (AsCPangInfo(row)->pangAnimationState == 4) {
            pulseScale = totalBase + totalStep * (totalTime - e->timer) / totalTime;
            if (pulseScale < totalBase)
                pulseScale = totalBase;
        } else {
            pulseScale = totalBase;
        }
        size[0] = kFloat_256_MessageUI * pulseScale;
        size[1] = kFloat_32_0 * pulseScale;
        xy[0] = floor(AsWView(g_view)->viewportWidth - kFloat_28_GolfHud - size[0] - scale * pulseScale);
        float fy = floor(AsCPangInfo(row)->pangBaselineY - totalCap * size[1]);
        xy[0] -= kFloat_25_805DE368;
        xy[1] = fy - kFloat_30_GolfHud;
        if (i == 1)
            xy[1] += kFloat_20_MessageUI;
        unsigned int color = (unsigned int)AsCPangInfo(row)->pangOpacity << 24 | kRgbWhite;
        if (AsCPangInfo(p)->textRenderer)
            WOverlay_DrawTransformedRect(AsCPangInfo(p)->textRenderer, g_view, text, kFloat_0_hud, xy, 0, color, 0);
        xy[0] += kFloat_25_805DE368;
        AsCPangInfo(p)->color = color;
        int upper = e->count;
        upper = (upper > 99999) ? 99999 : upper;
        e->count = upper;
        sprintf((char*)AsCPangInfo(p)->textBuffer, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                e->count);
        HudFont_DrawAlignedText(this, AsCPangInfo(p)->textBuffer, AsWView(g_view)->viewportWidth - kFloat_28_GolfHud,
                                kFloat_2_805DE318 + xy[1], kFloat_3_Hud, pulseScale, 0, 0);
    }
    unsigned char* manager = (unsigned char*)g_sharedDoc;
    unsigned int flags = *(unsigned int*)(((unsigned char*)AsCSharedDoc(manager)->golfDoc) + 0x358);
    if (!(flags & kPangRoundAdmissionMask))
        return;
    if ((int&)AsCPangInfo(p)->flags == 0)
        return;
    if ((flags & kPangRoundRestrictedFlag) && *((unsigned char*)&AsCSharedDoc(manager)->golfRuleType) != 1)
        return;
    {
        row = p + 0x318;
        text = p + 0x3D0;
        for (int i = 18; i >= 0; --i, text -= 0x2C, row -= 0x2C) {
            unsigned int flags = *(unsigned int*)(row + 0xA0);
            if ((flags & (unsigned int)-0x1A2) != 0) {
                int state = flags & 0xFFF;
                float a;
                switch (state) {
                case 2: {
                    a = 0.6f + 0.4f * AsCPangInfo(row)->bonusEntryAnimationTime / kFloat_0_5_hud;
                    if (a > kFloat_1_805DE314)
                        a = kFloat_1_805DE314;
                    float q = kFloat_255_MessageUI * (AsCPangInfo(row)->bonusEntryAnimationTime / kFloat_0_5_hud);
                    if (q > kFloat_255_MessageUI)
                        q = kFloat_255_MessageUI;
                    AsCPangInfo(row)->bonusEntryOpacity = (int)q;
                    break;
                }
                case 4:
                    a = 0.6f + 0.4f * (kFloat_0_5_hud - AsCPangInfo(row)->bonusEntryAnimationTime) / kFloat_0_5_hud;
                    if (a < 0.6f)
                        a = 0.6f;
                    break;
                default:
                    a = 0.6f;
                    break;
                }
                if (AsCPangInfo(row)->bonusEntryOpacity) {
                    size[0] = kFloat_256_MessageUI * a;
                    size[1] = kFloat_32_0 * a;
                    xy[0] = floor(AsWView(g_view)->viewportWidth - kFloat_28_GolfHud - size[0] - scale * a);
                    float y = AsCPangInfo(row)->bonusEntryY - 0.7f * size[1];
                    xy[0] -= kFloat_25_805DE368;
                    xy[1] = y - kFloat_30_GolfHud;
                    unsigned int color = (AsCPangInfo(row)->bonusEntryOpacity << 24) | kRgbWhite;
                    if (AsCPangInfo(p)->textRenderer)
                        WOverlay_DrawTransformedRect(AsCPangInfo(p)->textRenderer, g_view, text, kFloat_0_hud, xy, 0,
                                                     color, 0);
                    xy[0] += kFloat_25_805DE368;
                    AsCPangInfo(p)->color = color;
                    if (i == 1 || i == 9 || i == 10 || i == 11 || i == 14)
                        sprintf((char*)AsCPangInfo(p)->textBuffer,
                                (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, multiplierFormat)),
                                AsCPangInfo(row)->bonusEntryValue);
                    else
                        sprintf((char*)AsCPangInfo(p)->textBuffer,
                                (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                                AsCPangInfo(row)->bonusEntryValue);
                    HudFont_DrawAlignedText(this, AsCPangInfo(p)->textBuffer,
                                            AsWView(g_view)->viewportWidth - kFloat_28_GolfHud,
                                            kFloat_2_805DE318 + xy[1], kFloat_3_Hud, a, 0, 0);
                }
            }
        }
    }
    // Banner paths below preserve an unusual original filter: skip current
    // player, then gate text/sprite submission on being current. With a stable
    // currentPlayer neither banner loop submits anything. Position interpolation
    // and lookups still run for other eligible players. CurrentPlayer is reloaded
    // at each test; this code neither removes the work nor repairs the filter.
    if (((unsigned int&)AsCPangInfo(p)->ballBonusState & 0x101) == 0 && (AsCPangInfo(p)->flags & 8)) {
        float opacity =
            *(AsCPangInfo(p)->ballBonusBannerElapsed < s_ballBonusBannerDuration ? (float*)(p + 0x440)
                                                                                 : &s_ballBonusBannerDuration);
        size[0] = kFloat_153_6_Hud;
        size[1] = kFloat_19_2_PangBanner;
        row = p + 0x41C;
        for (int i = 0; i < AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount;
             ++i, row += 8) {
            void* item = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
            if (*(int*)((unsigned char*)item + 8) == 3)
                continue;
            if (i == AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer)
                continue;
            if (*((unsigned char*)&AsCSharedDoc(g_sharedDoc)->golfRuleType) == 1 &&
                i == AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer)
                sprintf((char*)AsCPangInfo(p)->textBuffer,
                        (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)), 10);
            a1 = Vec2_ScaleByFloat(row, opacity);
            b1 = Vec2_ScaleByFloat(p + 0x414, kFloat_1_805DE314 - opacity);
            c1 = Vec2_Add(&b1, &a1);
            Vec2_Copy(pos1, &c1);
            if (AsCPangInfo(p)->unk_450 > 1)
                pos1[1] -= kCupHeightOneDecimalThreshold;
            xy[0] = pos1[0] - kFloat_25_805DE368;
            xy[1] = pos1[1] - kFloat_30_GolfHud;
            if (i == AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer &&
                AsCPangInfo(p)->textRenderer)
                WOverlay_DrawTransformedRect(AsCPangInfo(p)->textRenderer, g_view, p + 0x378, kFloat_0_hud, xy, 0, -1,
                                             0);
            xy[0] += kFloat_25_805DE368;
            if (i == AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer)
                HudFont_DrawAlignedText(this, AsCPangInfo(p)->textBuffer, 180.0f + xy[0], kFloat_2_805DE318 + xy[1],
                                        kFloat_3_Hud, 0.6f, 0, 0);
        }
    }
    if ((unsigned int&)AsCPangInfo(p)->holeInOneBonusState & 0x101)
        return;
    if (!(AsCPangInfo(p)->flags & 8) &&
        !(*(unsigned int*)(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc) + 0x358) & kPangRoundRestrictedFlag))
        return;
    {
        float opacity =
            *(AsCPangInfo(p)->holeInOneBannerElapsed < s_holeInOneBannerDuration ? (float*)(p + 0x43C)
                                                                                 : &s_holeInOneBannerDuration);
        sprintf((char*)AsCPangInfo(p)->textBuffer, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)),
                500);
        size[0] = kFloat_153_6_Hud;
        size[1] = kFloat_19_2_PangBanner;
        row = p + 0x41C;
        for (int i = 0; i < AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount;
             ++i, row += 8) {
            void* item = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), (unsigned char)i);
            if (*(int*)((unsigned char*)item + 8) == 3)
                continue;
            if (i == AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer)
                continue;
            a2 = Vec2_ScaleByFloat(row, opacity);
            b2 = Vec2_ScaleByFloat(p + 0x414, kFloat_1_805DE314 - opacity);
            c2 = Vec2_Add(&b2, &a2);
            Vec2_Copy(pos2, &c2);
            if (AsCPangInfo(p)->unk_450 > 1)
                pos2[1] += kCupHeightOneDecimalThreshold;
            xy[0] = pos2[0];
            xy[1] = pos2[1];
            if (i == AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer &&
                AsCPangInfo(p)->textRenderer)
                WOverlay_DrawTransformedRect(AsCPangInfo(p)->textRenderer, g_view, p + 0x2C8, kFloat_0_hud, xy, 0, -1,
                                             0);
            if (i == AsCGolfDoc(((unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer)
                HudFont_DrawAlignedText(this, AsCPangInfo(p)->textBuffer, 180.0f + xy[0], kFloat_2_805DE318 + xy[1],
                                        kFloat_3_Hud, 0.6f, 0, 0);
        }
    }
}
// Player totals, calipers, balloon, and drive information.

struct SkinsResultAnimationFields {
    char reserved_00[0x18];
    unsigned int winOverlay;
    char reserved_1C[0x08];
    unsigned int loseOverlay;
    unsigned int loseDetailOverlay;
    unsigned int drawOverlay;
    unsigned int drawDetailOverlay;
    unsigned int resultOverlay;
    unsigned int resultDetailOverlay;
    unsigned int rewardValue;
    unsigned int result;
    float opacity;
    float elapsed;
    unsigned int isActive;
};

struct PangFinishFields {
    char reserved_00[0x24];
    void* field24;
    unsigned int field28;
    volatile unsigned int field2C;
    unsigned int field30;
    volatile unsigned int field34;
    char reserved_38[0x14];
    unsigned int field4C;
    char reserved_50[0x24];
    unsigned int field74;
    unsigned int field78;
};

struct HudBaseResourceFields {
    void* vtable;
    void* field04;
    char reserved_08[0x0C];
    void* field14;
};

struct HudCurrentPlayerFields {
    char reserved_00[0x3C1];
    unsigned char index;
};

struct HudPlayerSharedFields {
    char reserved_00[0xB90];
    HudCurrentPlayerFields* state;
};

struct HudPlayerValuesFields {
    char reserved_00[0x18];
    void* values[1];
};

struct PangResetEntryFields {
    char reserved_00[8];
    int fieldA0;
    char reserved_A4[4];
    unsigned int fieldA8;
    float fieldAC;
    char reserved_B0[0x18];
    unsigned char fieldC8;
};

struct PangResetFields {
    char reserved_00[0x20];
    unsigned int field20;
    unsigned int field24;
    char reserved_28[0x10];
    unsigned int field38;
    unsigned int field3C;
    char reserved_40[8];
    unsigned int field48;
    unsigned int field4C;
    char reserved_50[4];
    float field54;
    char reserved_58[0x1C];
    unsigned int field74;
    unsigned int field78;
    char reserved_7C[4];
    float field80;
    char reserved_84[0x3C8];
    float field44C;
    char reserved_450[0x18];
    unsigned int field468;
    unsigned int field46C;
};

struct CalipersResetGaugeFields {
    char reserved_00[0x2E4];
    float calipersGaugeX;
};

struct CalipersResetSharedFields {
    char reserved_00[0xB90];
    CalipersResetGaugeFields* state;
};

struct BalloonViewportFields {
    char reserved_00[0x188];
    float field188;
};

struct DriveVTable {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual void v7();
};
struct DriveCaptureFields {
    DriveVTable* vtable;
    char reserved_04[0x14];
    float startingDistance;
    float previousStrokeDistance;
};

struct DriveCurrentPlayerFields {
    char reserved_00[0x3C1];
    unsigned char index;
};

struct DriveCaptureSharedFields {
    char reserved_00[0xB90];
    DriveCurrentPlayerFields* state;
};

struct BalloonGolfDocFields {
    char reserved_00[0x358];
    unsigned char flags[0x49];
};
struct BalloonSharedDocFields {
    char reserved_00[0xB90];
    BalloonGolfDocFields* state;
};
struct PlayerBalloonPangFields {
    char reserved_00[0x174];
    int pendingBalloonPang;
    char reserved_178[4];
    int holeBalloonPang[18];
    int balloonPangTotal;
};

struct BallFlightStatusFields;

// Resolve the current player's balloon-reward storage through the active golf document.
inline PlayerBalloonPangFields* scorePlayer() {
    BalloonGolfDocFields* s = ((BalloonSharedDocFields*)g_sharedDoc)->state;
    return (PlayerBalloonPangFields*)((void* (*)(void*, unsigned char))GolfPlayer_GetEntryByIndex)(
        s, AsCGolfDoc(s)->currentPlayer);
}
// Decode the current ball's outcome for the balloon-reward state machine.
inline int scoreMode() {
    char* p = (char*)GolfBall__Fv();
    return ~(*(unsigned int*)(p + 0x1989C) ^ *(unsigned int*)(p + 0x198A4));
}
struct ScoreVirtual {
    virtual void a();
    virtual void b();
    virtual void c();
    virtual void d();
    virtual void e();
    virtual void f();
    virtual void g();
    virtual void h();
};

struct DriveResetFields {
    char reserved_00[0x24];
    unsigned int opacity;
    unsigned int animationState;
    float elapsedSeconds;
};

struct BalloonInitVTable {
    virtual void call0();
    virtual void call1();
    virtual void call2();
    virtual void call3(const char*, int, int);
    virtual void call4();
    virtual void call5();
};

struct ShotPangReward {
    int pang;
    int bonusPang;
};
struct RewardGolfDocFields {
    char reserved_00[0x2B9];
    unsigned char count;
    char reserved_2BA[0xFC];
    unsigned char active;
    char reserved_3B7[0x0A];
    unsigned char current;
};
struct RewardSharedDocFields {
    char reserved_00[0xB90];
    RewardGolfDocFields* state;
};

// Return the active golf document used by the reward-commit helpers.
static inline RewardGolfDocFields* FEEDCState() {
    return ((RewardSharedDocFields*)g_sharedDoc)->state;
}

// Resolve a player's per-hole and accumulated Pang totals.
static inline PlayerPangTotalsFields* FEEDCPlayer(RewardGolfDocFields* state, unsigned char index) {
    return ((PlayerPangTotalsFields * (*)(RewardGolfDocFields*, unsigned char)) GolfPlayer_GetEntryByIndex)(state,
                                                                                                            index);
}

struct LeaderGolfDocFields {
    char reserved_00[0x2B9];
    unsigned char count;
    char reserved_2BA[0xFC];
    unsigned char active;
    char reserved_3B7[0x0A];
    unsigned char current;
    char reserved_3C2[10];
    unsigned char enabled;
};
struct LeaderSharedDocFields {
    char reserved_00[0xB90];
    LeaderGolfDocFields* state;
};
struct LeaderDistanceFields {
    char reserved_00[0x18];
    unsigned int ranks[5];
    unsigned char index;
    char reserved_2D[3];
    float values[256];
};
struct HolePositionWords {
    unsigned int x, y, z;
};

struct DrivePlayerCountFields {
    char reserved_00[0x2B9];
    unsigned char count;
};
struct DriveSharedDocFields {
    char reserved_00[0xB90];
    DrivePlayerCountFields* state;
};

struct CalipersIndicatorRect {
    float x, y, z, w;
    // Place the 8-by-32-pixel indicator relative to the HUD baseline.
    CalipersIndicatorRect* set(float value, COption* options) {
        float one = kFloat_1_805DE314;
        float height = options->hudBaselineY;
        float thirtyOne = kCalipersIndicatorYOffset;
        float a = value - one;
        float c = kFloat_8_Hud;
        float b = height - thirtyOne;
        float d = kFloat_32_0;
        x = a;
        y = b;
        z = c;
        w = d;
        return this;
    }
};

struct DriveAnimationFields {
    void** vtable;
    char reserved_04[0x14];
    float startingDistance, previousStrokeDistance, targetStrokeDistance;
    char reserved_24[4];
    int state;
    float elapsed;
    unsigned int holeStrokeCount;
};

struct DriveHolePlayerFields {
    char reserved_00[0x3B6];
    unsigned char currentHoleIndex;
    char reserved_3B7[0x0A];
    unsigned char current;
};
struct DriveAnimationSharedFields {
    char reserved_00[0xB90];
    DriveHolePlayerFields* state;
};
// Borrowed player result: slot 0 is the animated total; slots 1..3 are the
// stroke-indexed distance scores written by CGolfRule::ProcessBallFlight.
struct PlayerDriveDistanceFields {
    char reserved_00[0x178];
    float distanceScores[4];
};
struct BallFlightStatusFields {
    char reserved_00[0x18];
    int ballState;
};

struct CalipersOptionFields {
    char reserved_00[0x10];
    float height;
};
extern "C" void SkinsInfo_StartResultDisplay(SkinsResultAnimationFields* self, int arg, unsigned int value);
extern "C" void* __dt__11BalloonBaseFv(HudBaseResourceFields* self, int flags);
extern "C" void Hud_SetCurrentPlayerDisplayValue(void* self, void* value);
extern "C" void Hud_UpdateNearestPinPlayer(LeaderDistanceFields* self);

// Applies shot pang and bonus pang to a player, clamping total pang at zero.
extern "C" void GolfPlayer_ApplyShotPangReward(void* unused, unsigned char index, ShotPangReward* arg) {
    if (arg->pang < 0 && -arg->pang > FEEDCPlayer(FEEDCState(), index)->pangTotal) {
        PlayerPangTotalsFields* target = FEEDCPlayer(FEEDCState(), index);
        unsigned char slot = FEEDCState()->active;
        PlayerPangTotalsFields* source = FEEDCPlayer(FEEDCState(), index);
        source->holePang[slot] = -target->pangTotal;
        FEEDCPlayer(FEEDCState(), index)->pangTotal = 0;
    } else {
        unsigned char slot = FEEDCState()->active;
        PlayerPangTotalsFields* player = FEEDCPlayer(FEEDCState(), index);
        player->holePang[slot] += arg->pang;
        FEEDCPlayer(FEEDCState(), index)->pangTotal += arg->pang;
    }
    FEEDCPlayer(FEEDCState(), index)->bonusPangTotal += arg->bonusPang;
    PlayerPangTotalsFields* player = FEEDCPlayer(FEEDCState(), FEEDCState()->current);
    if (player->shotRewardFlags & 0x1000) {
        for (int i = 0; i < FEEDCState()->count; ++i)
            FEEDCPlayer(FEEDCState(), (unsigned char)i);
    }
    player = FEEDCPlayer(FEEDCState(), FEEDCState()->current);
    if (player->shotRewardFlags & 0x4000) {
        for (int i = 0; i < FEEDCState()->count; ++i)
            FEEDCPlayer(FEEDCState(), (unsigned char)i);
    }
}

// Resets pang counters and all enabled bonus entries for a new count-up animation.
void CPangInfo::ResetBonusAnimations() {
    PangResetFields* self = (PangResetFields*)this;
    float zero = kFloat_0_hud;
    unsigned int field30 = AsCPangInfo(self)->startingPang;
    unsigned int field34 = AsCPangInfo(self)->startingBonusPang;
    unsigned int zeroValue = 0;
    unsigned int minusOne = 0xFFFFFFFF;
    unsigned int eight = 8;
    self->field20 = 2;
    (unsigned int&)AsCPangInfo(self)->scheduledBonusCount = zeroValue;
    self->field24 = 0;
    (unsigned int&)AsCPangInfo(self)->processedTrajectoryFrame = zeroValue;
    (unsigned int&)AsCPangInfo(self)->rewardedBounceCount = zeroValue;
    (unsigned int&)AsCPangInfo(self)->lastRewardedBounceFrame = minusOne;
    self->field48 = eight;
    self->field4C = field30;
    self->field38 = field30;
    self->field54 = zero;
    self->field3C = field34;
    self->field78 = field34;
    self->field74 = eight;
    self->field80 = zero;
    char* item = (char*)self;
    self->field44C = zero;
    for (int i = 0; i < kBonusEntryCount; ++i) {
        unsigned int* state = (unsigned int*)((char*)&AsCPangInfo(item)->bonusEntryState);
        if (((int*)state)[0] != kBonusUnavailable)
            state[0] = 1;
        state[2] = 0xFFFFFFFF;
        AsCPangInfo(item)->bonusEntryAnimationTime = zero;
        AsCPangInfo(item)->bonusEntryOpacity = (unsigned char)zeroValue;
        item += 0x2C;
    }
    (unsigned int&)AsCPangInfo(self)->objectHitMultiplier = 1;
}

// Stops the count sound and snaps displayed totals to nonnegative final rewards.
void CPangInfo::FinishDisplayedTotals() {
    PangFinishFields* self = (PangFinishFields*)this;
    void* p = self->field24;
    if (p)
        Sound_StopByAlias(g_soundManager, p);
    volatile PangFinishFields* v = self;
    int a = v->field30 + v->field28;
    v->field4C = a > 0 ? a : 0;
    int b = v->field34 + v->field2C;
    self->field78 = b > 0 ? b : 0;
}

// Creates the calipers pointer overlays and clears the locked measurement.
CCalipers::CCalipers() {
    extern char __vt__9CCalipers[];
    vtbl = __vt__9CCalipers;
    SetFont((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, barFont)), 0, 0);
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
    alignment = 1;
    indicatorOverlay =
        WOverlay_Create(g_resourceManager, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, indicator_gsr)), 0);
    calipersOverlay =
        WOverlay_Create(g_resourceManager, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, calipers_gsr)), 0);
    isLocked = 0;
    lockedGaugeX = kFloat_0_hud;
    isPreviewActive = 0;
}

// Releases both calipers overlays and the HUD text field's font and text buffer.
CCalipers::~CCalipers() {
    vtbl = (void*)&__vt__9CCalipers;
    if (g_resourceManager && indicatorOverlay) {
        Object_DeleteVirtual(g_resourceManager, indicatorOverlay);
        indicatorOverlay = 0;
    }
    if (g_resourceManager && calipersOverlay) {
        Object_DeleteVirtual(g_resourceManager, calipersOverlay);
        calipersOverlay = 0;
    }
    // The HUD text field cleanup tests the object again, as the original does.
    if (this) {
        void* resource = fontResource;
        vtbl = (void*)&cInfoBaseVtable;
        if (resource) {
            ((void (*)(void*, void*))ResourceManager_DeleteResource)(g_resourceManager, resource);
            fontResource = 0;
        }
        if (textBuffer) {
            __dla__FPv(textBuffer);
            textBuffer = 0;
        }
    }
}

// Previews the selected player pointer on the shot gauge; confirm locks its rounded X position.
void CCalipers::UpdateGaugeMeasurement() {
    float pointerPosition[2];
    if (0 == GetInputManagerSlot()) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(&s_gameInputManager);
            __register_global_object(&s_gameInputManager, InputManager_Dtor, &s_inputManagerDtorRegistration_GolfHud);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = &s_gameInputManager;
    }
    GameInputManager* input = (GameInputManager*)GetInputManagerSlot();
    float hudBaselineY = AsCOption(g_option)->hudBaselineY;
    isPreviewActive = 0;
    unsigned int selectedPlayer = InputManager_GetSelectedPlayer(input);

    // Preview only inside the gauge rectangle; a confirm press locks the rounded X position.
    InputManager_GetPointerPosition(input, pointerPosition, selectedPlayer);
    if (pointerPosition[0] > kCalipersPointerHitLeft && pointerPosition[0] < kCalipersPointerHitRight &&
        pointerPosition[1] > hudBaselineY - kFloat_15_Hud && pointerPosition[1] < kFloat_20_MessageUI + hudBaselineY &&
        !isLocked) {
        if (Input_IsActionPressed(input, kCalipersConfirmAction, kSelectedInputPlayer)) {
            float pointerGaugeX = pointerPosition[0];
            if (pointerGaugeX < kCalipersMinimumGaugeX)
                pointerGaugeX = kCalipersMinimumGaugeX;
            else
                pointerGaugeX = (pointerGaugeX > kCalipersMaximumGaugeX) ? kCalipersMaximumGaugeX : pointerGaugeX;
            float roundedGaugeX = (float)((double (*)(double))floor)(pointerGaugeX);
            lockedGaugeX = roundedGaugeX;
            CGolfDoc* golfDoc = ((CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc);
            golfDoc->calipersGaugeX = roundedGaugeX;
            isLocked = 1;
        } else {
            float pointerGaugeX = pointerPosition[0];
            if (pointerGaugeX < kCalipersMinimumGaugeX)
                pointerGaugeX = kCalipersMinimumGaugeX;
            else
                pointerGaugeX = (pointerGaugeX > kCalipersMaximumGaugeX) ? kCalipersMaximumGaugeX : pointerGaugeX;
            float roundedGaugeX = (float)((double (*)(double))floor)(pointerGaugeX);
            CGolfDoc* golfDoc = ((CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc);
            golfDoc->calipersGaugeX = roundedGaugeX;
        }
        isPreviewActive = 1;
        // Leaving the rectangle preserves a locked value; otherwise use the no-calipers sentinel.
    } else if (isLocked) {
        CGolfDoc* golfDoc = ((CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        golfDoc->calipersGaugeX = lockedGaugeX;
    } else {
        CGolfDoc* golfDoc = ((CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc);
        golfDoc->calipersGaugeX = kCalipersNoMeasurementX;
    }
}

#pragma opt_propagation off
#pragma opt_common_subs off
// Draws the calipers distance in yards and selects the indicator atlas region by club type.
void CCalipers::DrawDistanceGauge() {
    float y;
    if (((HudGolfParameters*)InstanceTable_Get())->club->clubType == kClubPutter)
        y = kCalipersPutterLabelYOffset;
    else
        y = kCalipersClubLabelYOffset;
    CGolfDoc* golfDoc = ((CGolfDoc*)AsCSharedDoc(g_sharedDoc)->golfDoc);
    float gaugeX = golfDoc->calipersGaugeX;
    if (gaugeX > kCalipersNoMeasurementX) {
        if (gaugeX > kCalipersMaximumGaugeX) {
            gaugeX = kCalipersMaximumGaugeX;
            float clubDistanceYards = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
            sprintf(textBuffer, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, wholeYardsFormat)),
                    clubDistanceYards);
        } else {
            float clubDistanceYards = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
            float gaugeDistance = gaugeX - kCalipersNoMeasurementX;
            clubDistanceYards = gaugeDistance * clubDistanceYards;
            sprintf(textBuffer, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, wholeYardsFormat)),
                    clubDistanceYards / kCalipersGaugeDistanceSpan);
        }
        color = -0x4000;
        float scale = kFloat_1_805DE314;
        float offset = kFloat_20_MessageUI;
        COption* options = AsCOption(g_option);
        float hudBaselineY = options->hudBaselineY;
        float drawY = y + (hudBaselineY - offset);
        HudFont_DrawAlignedText(this, textBuffer, gaugeX, drawY, scale, scale, 0, 0);
        if (calipersOverlay) {
            float* selectedUvRect;
            float putterUvRect[4], nonPutterUvRect[4];
            CalipersIndicatorRect pos;
            CalipersIndicatorRect* position = pos.set(gaugeX, AsCOption(g_option));
            if (((HudGolfParameters*)InstanceTable_Get())->club->clubType == kClubPutter) {
                putterUvRect[0] = kFloat_0_5_hud;
                putterUvRect[1] = kFloat_0_hud;
                putterUvRect[2] = kFloat_0_5_hud;
                putterUvRect[3] = kFloat_1_805DE314;
                selectedUvRect = putterUvRect;
            } else {
                nonPutterUvRect[0] = kFloat_0_hud;
                nonPutterUvRect[1] = kFloat_0_hud;
                nonPutterUvRect[2] = kFloat_0_5_hud;
                nonPutterUvRect[3] = kFloat_1_805DE314;
                selectedUvRect = nonPutterUvRect;
            }
            WOverlay_DrawTransformedRect(calipersOverlay, g_view, selectedUvRect, kFloat_0_hud, position, 0, 0xB2FFC000,
                                         0);
        }
    }
}
#pragma opt_propagation reset
#pragma opt_common_subs reset

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Clears the calipers lock and restores the no-measurement gauge sentinel.
void CCalipers::ResetMeasurement() {
    CalipersResetGaugeFields* state = ((CalipersResetSharedFields*)g_sharedDoc)->state;
    state->calipersGaugeX = kCalipersNoMeasurementX;
    isLocked = 0;
    lockedGaugeX = kFloat_0_hud;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

// Reports whether an unlocked calipers pointer was previewing the shot gauge this frame.
unsigned int CCalipers::IsPreviewActive() {
    return isPreviewActive;
}

// Starts a skins-result animation and chooses win, draw, or loss overlays.
extern "C" void SkinsInfo_StartResultDisplay(SkinsResultAnimationFields* self, int arg, unsigned int value) {
    if (self->isActive != 0)
        return;
    self->result = arg;
    self->rewardValue = value;
    self->opacity = kFloat_255_MessageUI;
    self->isActive = 1;
    self->elapsed = kFloat_2_805DE318;
    if (arg > 0) {
        self->resultOverlay = self->winOverlay;
    } else if (arg == 0) {
        self->resultOverlay = self->drawOverlay;
        self->resultDetailOverlay = self->drawDetailOverlay;
    } else if (arg < 0) {
        self->resultOverlay = self->loseOverlay;
        self->resultDetailOverlay = self->loseDetailOverlay;
    }
}

// Stores a display value in the current player slot.
extern "C" void Hud_SetCurrentPlayerDisplayValue(void* self, void* value) {
    HudCurrentPlayerFields* state = ((HudPlayerSharedFields*)g_sharedDoc)->state;
    unsigned int index = state->index;
    ((HudPlayerValuesFields*)self)->values[index] = value;
}

// Selects the player nearest the pin, using stored ranks to break near-equal distances.
extern "C" void Hud_UpdateNearestPinPlayer(LeaderDistanceFields* self) {
    LeaderSharedDocFields* manager = (LeaderSharedDocFields*)g_sharedDoc;
    LeaderGolfDocFields* state;
    state = manager->state;
    if (state->enabled) {
        unsigned int* entry = (unsigned int*)AsCGolfDoc(state)->holeRecords + state->active * 8;
        float result[3];
        HolePositionWords vec = *(HolePositionWords*)(entry + 3);
        state = ((LeaderSharedDocFields*)g_sharedDoc)->state;
        void* player = ((void* (*)(void*, unsigned char))GolfPlayer_GetEntryByIndex)(state, state->current);
        __mi__FRC7WVectorRC7WVector(result, ((GolfDocSnapshot8013*)player)->ballPosition,
                                    &vec.x); // Stored player position.
        result[1] = kFloat_0_hud;
        state = ((LeaderSharedDocFields*)g_sharedDoc)->state;
        self->values[state->current] = (*(WVector*)result).Magnitude() / kFloat_3_2_hud;
        if (self->index == 0xFF)
            self->index = ((LeaderSharedDocFields*)g_sharedDoc)->state->current;
        else {
            unsigned int current;
            unsigned char old;
            unsigned int currentOffset;
            unsigned int oldOffset;
            old = self->index;
            current = ((LeaderSharedDocFields*)g_sharedDoc)->state->current;
            oldOffset = old * 4;
            currentOffset = current * 4;
            float diff = *(float*)((char*)self->values + currentOffset) - *(float*)((char*)self->values + oldOffset);
            if (kFloat_100_hud * diff < kNearestPinLeadDistanceThreshold ||
                (kFloat_100_hud * (diff > kFloat_0_hud ? diff : -diff) < kFloat_1_805DE314 &&
                 *(unsigned int*)((char*)self->ranks + currentOffset) <
                     *(unsigned int*)((char*)self->ranks + oldOffset))) {
                self->index = current;
            }
        }
    }
}

// Creates the balloon-mode pang popup and initializes its texture rectangle.
CBalloonInfo::CBalloonInfo() {
    char* base = (char*)&golfHudFontStrings;
    float zero = kFloat_0_hud;
    vtbl = (void*)&__vt__12CBalloonInfo;
    pendingPang = 0;
    textOpacity = 0xFF;
    animationState = 0;
    phaseElapsed = zero;
    countElapsed = zero;
    startingTotal = 0;
    popupOverlay = 0;
    ((BalloonInitVTable*)this)->call3((base + IFF_OFFSET(GolfHudStringPool, largeFont)), 0, 0);
    alignment = 2;
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
    void* overlay = WOverlay_Create(g_resourceManager, (base + IFF_OFFSET(GolfHudStringPool, pang_bonus_gsr)), 0);
    float small = kFloat_0_0625_Hud;
    float half = kFloat_0_5_hud;
    popupOverlay = overlay;
    uvX = half;
    uvY = half;
    uvWidth = half;
    uvHeight = small;
    volatile float values[4];
    values[0] = half;
    values[1] = half;
    values[2] = half;
    values[3] = small;
    ((BalloonInitVTable*)this)->call5();
}

// Releases shared HUD font/text storage, optionally freeing the object.
extern "C" void* __dt__11BalloonBaseFv(HudBaseResourceFields* self, int flags) {
    if (self) {
        if (self) {
            void* p = self->field04;
            self->vtable = (void*)&cInfoBaseVtable;
            if (p) {
                ResourceManager_DeleteResource(g_resourceManager, p);
                self->field04 = 0;
            }
            if (self->field14) {
                __dla__FPv(self->field14);
                self->field14 = 0;
            }
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

// Releases the balloon popup overlay and the HUD text field's font and text buffer.
CBalloonInfo::~CBalloonInfo() {
    vtbl = (void*)&__vt__12CBalloonInfo;
    if (g_resourceManager && popupOverlay) {
        Object_DeleteVirtual(g_resourceManager, popupOverlay);
        popupOverlay = 0;
    }
    // The HUD text field cleanup tests the object twice, as the original does.
    if (this) {
        if (this) {
            void* resource = fontResource;
            vtbl = (void*)&cInfoBaseVtable;
            if (resource) {
                ((void (*)(void*, void*))ResourceManager_DeleteResource)(g_resourceManager, resource);
                fontResource = 0;
            }
            if (textBuffer) {
                __dla__FPv(textBuffer);
                textBuffer = 0;
            }
        }
    }
}

// Resets the balloon popup position, opacity, pending pang, and animation timers.
void CBalloonInfo::ResetPopupAnimation() {
    float maximum = kDriveHudRowOriginAndLabelMargin;
    __typeof__(((BalloonViewportFields*)g_view)->field188) _p854 = ((BalloonViewportFields*)g_view)->field188;
    pendingPang = 0;
    __typeof__(_p854) _p451 = ((BalloonViewportFields*)g_view)->field188;
    __typeof__(_p451) _p986 = ((BalloonViewportFields*)g_view)->field188;
    float offset = kFloat_28_GolfHud;
    __typeof__(_p986) _p891 = ((BalloonViewportFields*)g_view)->field188;
    textOpacity = 0xFF;
    float zero = kFloat_0_hud;
    animationState = 0;
    phaseElapsed = zero;
    __typeof__(_p891) _p887 = ((BalloonViewportFields*)g_view)->field188;
    countElapsed = zero;
    startingTotal = 0;
    float field188Value = ((BalloonViewportFields*)g_view)->field188;
    float current = field188Value - offset;
    popupX = current;
    popupY = maximum;
}

// Animates balloon pang into player totals and commits it after the popup delay.
void CBalloonInfo::AnimateAndCommitPang(float delta) {
    // Phases: refresh changed pending Pang, wait/animate through the display
    // states, and commit to totals when the completed popup can settle.
    BalloonGolfDocFields* state = ((BalloonSharedDocFields*)g_sharedDoc)->state;
    if (((unsigned int)AsCGolfDoc(state)->gameStateFlags & kGolfHudActive) == 0)
        return;
    PlayerBalloonPangFields* player = scorePlayer();
    if (pendingPang != player->pendingBalloonPang) {
        if (pendingPang == 0 || scoreMode() == 1) {
            float zero = kFloat_0_hud;
            animationState = 1;
            phaseElapsed = zero;
        }
        pendingPang = scorePlayer()->pendingBalloonPang;
    }
    if (0 == pendingPang)
        return;
    if (((CGolfBall*)GolfBall__Fv())->m_state == 0 && 3 == animationState) {
        float x;
        float alpha;
        float low;
        float v;
        char* names = golfHudFontStrings;
        if (!SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager,
                                                          (names + IFF_OFFSET(GolfHudStringPool, pangUpSound))))
            Sound_PlayByName(g_soundManager, (names + IFF_OFFSET(GolfHudStringPool, pangUpSound)), 0, 0,
                             (names + IFF_OFFSET(GolfHudStringPool, pangUpSound)), 0, kFloat_0_5_hud, kFloat_200_Hud,
                             kFloat_1_805DE314);
        float speed;
        float a;
        float high;
        float step;
        low = kFloat_105_GolfHud;
        float b;
        high = kDriveHudRowOriginAndLabelMargin;
        alpha = kFloat_255_MessageUI;
        step = delta / kFloat_0_5_hud;
        speed = kFloat_35_Hud;
        float timeValue = popupY;
        float one;
        one = kFloat_1_805DE314;
        v = timeValue - speed * step;
        a = (v - low) / speed;
        popupY = v;
        b = (high - v) / speed;
        // Preserve temporary lifetimes here to retain the compiler's register allocation.
        x = a * alpha;
        textOpacity = (int)x;

        float bValue = b;
        unsigned int amount = (int)((float)(pendingPang) * (bValue > one ? one : bValue));
        PlayerBalloonPangFields* current = scorePlayer();

        float half = kFloat_0_5_hud;
        __typeof__(countElapsed) _p480 = countElapsed;
        float elapsedValue = countElapsed;
        current->balloonPangTotal = (startingTotal + amount);
        __typeof__(_p480) _p454 = countElapsed;
        __typeof__(delta + _p454) _p608 = _p454 + delta;
        countElapsed = _p608;
        if (countElapsed > half) {
            PlayerBalloonPangFields* p = scorePlayer();
            scorePlayer()->balloonPangTotal = startingTotal + p->pendingBalloonPang;
            p = scorePlayer();
            unsigned char index = *(unsigned char*)((char*)((BalloonSharedDocFields*)g_sharedDoc)->state + 0x3B6);
            PlayerBalloonPangFields* q = scorePlayer();
            q->holeBalloonPang[index] += p->pendingBalloonPang;
            scorePlayer()->pendingBalloonPang = 0;
            ((ScoreVirtual*)this)->f();
        }
    } else {
        startingTotal = scorePlayer()->balloonPangTotal;
    }
    phaseElapsed = phaseElapsed + delta;
    if (animationState == 1 && phaseElapsed > kFloat_0_5_hud) {
        float zero = kFloat_0_hud;
        animationState = 2;
        phaseElapsed = zero;
    } else if (2 == animationState) {
        if (phaseElapsed > kFloat_0_5_hud) {
            float zero = kFloat_0_hud;
            animationState = 3;
            phaseElapsed = zero;
        }
    }
}

// Draws the balloon pang popup with animated scale and a four-digit value cap.
void CBalloonInfo::DrawPangPopup() {
    if (pendingPang != 0) {
        float alpha = kFloat_65_GolfHud;
        float scale;
        if (animationState == 1) {
            scale = kFloat_1_805DE314;
            float value = kFloat_0_6_805DE33C + (kDriveHudHighlightAlphaDelta * phaseElapsed) / kFloat_0_5_hud;
            scale = value > scale ? scale : value;
        } else if (animationState == 2) {
            scale = kFloat_0_6_805DE33C;
            float value =
                kFloat_0_6_805DE33C + (kDriveHudHighlightAlphaDelta * (kFloat_0_5_hud - phaseElapsed)) / kFloat_0_5_hud;
            scale = value < scale ? scale : value;
        } else
            scale = kFloat_0_6_805DE33C;
        float width = kFloat_256_MessageUI * scale;
        float height = kFloat_32_0 * scale;
        volatile float size[2] = {width, height};
        float pos[2];
        pos[0] = ((double (*)(double))floor)(*(volatile float*)&popupX - width - alpha * scale);
        pos[1] = ((double (*)(double))floor)(popupY - kDriveHudVerticalLayoutScale * size[1]);
        pos[0] -= kFloat_25_805DE368;
        unsigned int textColor = (textOpacity << 24) | kRgbWhite;
        // The four UV floats are the texture rectangle the overlay is drawn with.
        if (popupOverlay)
            WOverlay_DrawTransformedRect(popupOverlay, g_view, (float*)&uvX, kFloat_0_hud, pos, 0, textColor, 0);
        color = textColor;
        pendingPang = pendingPang > kMaxBalloonPang ? kMaxBalloonPang : pendingPang;
        sprintf(textBuffer, (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, integerFormat)), pendingPang);
        float drawY = kFloat_2_805DE318 + pos[1];
        float drawX = popupX;
        HudFont_DrawAlignedText(this, (void*)textBuffer, drawX, drawY, kFloat_3_Hud, scale, 0, 0);
    }
}

// Creates one Driving Contest label overlay per player and initializes its distance display.
CDriveInfo::CDriveInfo() {
    vtbl = (void*)&__vt__10CDriveInfo;
    ((BalloonInitVTable*)this)->call3((golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, largeFont)), 0, 1);
    alignment = 2;
    textBuffer = (char*)__nwa__FUl(kTextBufferBytes);
    unsigned char count = ((DriveSharedDocFields*)g_sharedDoc)->state->count;
    playerLabels = (void**)__nwa__FUl(count * 4);
    unsigned char i = 0;
    while (i < ((DriveSharedDocFields*)g_sharedDoc)->state->count) {
        char name[16];
        sprintf((char*)(name), (golfHudFontStrings + IFF_OFFSET(GolfHudStringPool, drivePlayerResourceFormat)), i + 1);
        playerLabels[i] = WOverlay_Create(g_resourceManager, name, 0);
        ++i;
    }
    targetStrokeDistance = kFloat_0_hud;
    previousStrokeDistance = kFloat_0_hud;
    startingDistance = kFloat_0_hud;
    ((BalloonInitVTable*)this)->call5();
    holeStrokeCount = 0;
}

#pragma use_lmw_stmw on
// Releases every player drive overlay and the HUD text field's font and text buffer.
CDriveInfo::~CDriveInfo() {
    vtbl = (void*)&__vt__10CDriveInfo;
    unsigned char i = 0;
    while (i < AsCGolfDoc(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount) {
        if (g_resourceManager && playerLabels[i]) {
            Object_DeleteVirtual(g_resourceManager, playerLabels[i]);
            playerLabels[i] = 0;
        }
        ++i;
    }
    if (playerLabels) {
        __dla__FPv(playerLabels);
        playerLabels = 0;
    }
    // The HUD text field cleanup tests the object twice, as the original does.
    if (this) {
        if (this) {
            void* resource = fontResource;
            vtbl = (void*)&cInfoBaseVtable;
            if (resource) {
                ResourceManager_DeleteResource(g_resourceManager, resource);
                fontResource = 0;
            }
            if (textBuffer) {
                __dla__FPv(textBuffer);
                textBuffer = 0;
            }
        }
    }
}
#pragma use_lmw_stmw reset

// Restores drive HUD opacity to 255 and restarts its wait state/seconds clock.
void CDriveInfo::ResetState() {
    DriveResetFields* self = (DriveResetFields*)this;
    float value = kFloat_0_hud;
    self->opacity = 0xFF;
    self->animationState = 0;
    self->elapsedSeconds = value;
}

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Snapshots the current player's total Driving Contest distance score before restart;
// the animation adds/interpolates the current stroke's score, including distance bonuses.
void CDriveInfo::SnapshotStartingDistance() {
    DriveCaptureFields* self = (DriveCaptureFields*)this;
    self->previousStrokeDistance = kFloat_0_hud;
    DriveCaptureSharedFields* mgr = (DriveCaptureSharedFields*)g_sharedDoc;
    unsigned char index = mgr->state->index;
    void* result = ((void* (*)(void*, unsigned char))GolfPlayer_GetEntryByIndex)(mgr->state, index);
    self->startingDistance = ((HudFlightResultFields*)result)->result.distances[0];
    ((DriveVTable*)self)->v5();
}
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset

// Animate Driving Contest distance; delta is elapsed seconds from the HUD update.
// Required initialized self/current-player snapshot; stroke count is 0 while
// waiting and must be 1..3 before indexing the delay table (no bounds check).
// Player +178 is the displayed cumulative distance, +17C/+180/+184 are per-stroke
// distance scores. CGolfRule::ProcessBallFlight (golf_rule_round_flow_and_shot_sim.cpp) writes distance plus
// contest bonuses there; CDriveInfo::DrawPlayerDistanceRows (golf_hud_hole_info_and_fonts.cpp) formats +178 as %6.2f.
// These are display-distance scores, not Pang or raw world-space coordinates.
// Wait -> Delay if stopped, Count otherwise; changed stroke score starts it.
// Delay -> Count after table[stroke-1]; Count -> Settle after .5 seconds;
// Settle -> Done after kFlightSecondaryPinFadeStartTime (1.5 seconds). Strict > resets
// the clock and discards overshoot. Independent tests run in reverse state order,
// so a newly entered phase does not also consume its duration in this call.
// Count/Settle interpolate previousStrokeDistance toward targetStrokeDistance;
// Done commits startingDistance+targetStrokeDistance and remembers that stroke.
// Already-Done entry calls raw vtable word 7: CDriveInfo::ResetState, which sets
// Wait, clears elapsed and restores opacity (+24) to 255. The Wait
// test below then runs in this same call and checks for a newly changed score.
// The vtable has two prefix words; logical method 7 (the Boolean predicate) is
// at raw word 9, and is NOT this call. Preserve the reset-before-poll sequence.
void CDriveInfo::UpdateDistanceAnimation(float delta) {
    DriveAnimationFields* self = (DriveAnimationFields*)this;
    self->elapsed = self->elapsed + delta;
    if (self->state == kDriveDone)
        ((void (*)(DriveAnimationFields*))self->vtable[7])(self);
    if (self->state == kDriveSettle) {
        if (kFlightSecondaryPinFadeStartTime < self->elapsed) {
            self->state = kDriveDone;
            self->elapsed = kFloat_0_hud;
        }
    }
    if (self->state == kDriveCount) {
        char* base = (char*)golfHudFontStrings;
        if (!SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager,
                                                          (base + IFF_OFFSET(GolfHudStringPool, pangUpSound))))
            Sound_PlayByName(g_soundManager, (base + IFF_OFFSET(GolfHudStringPool, pangUpSound)), 0, 0,
                             (base + IFF_OFFSET(GolfHudStringPool, pangUpSound)), 0, kFloat_0_5_hud, kFloat_200_Hud,
                             kFloat_1_805DE314);
        if (kFloat_0_5_hud < self->elapsed) {
            self->elapsed = kFloat_0_hud;
            self->state = kDriveSettle;
        }
    }
    if (self->state == kDriveDelay) {
        // Stored as three 32-bit words, interpreted as binary32 seconds after
        // copying. Keep integer loads and the float reinterpretation, not an
        // integer-to-float conversion. Verified baseline: 6.5, 7.0, 7.5 seconds.
        // Increasing a word's float value delays that stroke's count-up; its
        // safe tuning range is unestablished. The table is external storage.
        int times[3];
        times[0] = s_driveRewardDelayByStrokeCount[0];
        times[1] = s_driveRewardDelayByStrokeCount[1];
        times[2] = s_driveRewardDelayByStrokeCount[2];
        if (*(float*)&times[self->holeStrokeCount - 1] < self->elapsed) {
            self->elapsed = kFloat_0_hud;
            self->state = kDriveCount;
        }
    }
    if (self->state == kDriveWait) {
        unsigned char cur = ((DriveAnimationSharedFields*)g_sharedDoc)->state->current;
        void* player = GolfPlayer_GetEntryByIndex(((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), cur);
        unsigned char holeIndex = ((DriveAnimationSharedFields*)g_sharedDoc)->state->currentHoleIndex;
        self->holeStrokeCount = *(unsigned char*)((char*)player + holeIndex + 0x28);
        if (self->holeStrokeCount) {
            float value = ((PlayerDriveDistanceFields*)GolfPlayer_GetEntryByIndex(
                               ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc),
                               ((DriveAnimationSharedFields*)g_sharedDoc)->state->current))
                              ->distanceScores[self->holeStrokeCount];
            self->targetStrokeDistance = value;
            if (self->previousStrokeDistance != value) {
                if (((BallFlightStatusFields*)GolfBall__Fv())->ballState == 0)
                    self->state = kDriveDelay;
                else
                    self->state = kDriveCount;
                self->elapsed = kFloat_0_hud;
            } else
                return;
        } else
            return;
    }
    if (self->state == kDriveDone) {
        PlayerDriveDistanceFields* p = (PlayerDriveDistanceFields*)GolfPlayer_GetEntryByIndex(
            ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), ((DriveAnimationSharedFields*)g_sharedDoc)->state->current);
        p->distanceScores[0] = self->startingDistance + self->targetStrokeDistance;
        self->previousStrokeDistance = self->targetStrokeDistance;
    } else {
        // Keep binary32 arithmetic and its parenthesization: count-up readers
        // observe +178 every update, including the starting/previous subtotal.
        float t = kFloat_0_hud;
        if (self->state == kDriveCount)
            t = self->elapsed;
        if (self->state == kDriveSettle)
            t = kFloat_0_5_hud + self->elapsed;
        if (t > kFloat_1_805DE314)
            t = kFloat_1_805DE314;
        PlayerDriveDistanceFields* p = (PlayerDriveDistanceFields*)GolfPlayer_GetEntryByIndex(
            ((void*)AsCSharedDoc(g_sharedDoc)->golfDoc), ((DriveAnimationSharedFields*)g_sharedDoc)->state->current);
        p->distanceScores[0] =
            self->previousStrokeDistance +
            (self->startingDistance + t * (self->targetStrokeDistance - self->previousStrokeDistance));
    }
}
