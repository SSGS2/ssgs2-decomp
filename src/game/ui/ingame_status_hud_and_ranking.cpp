#include <decomp/static_init.h>
#include <decomp/local_class.h>
// In-round player HUD and Results ranking resource lifetime / layout animation.
// CIngameStatus_Update observes golf-state transitions; UpdateAndDraw advances
// BP callbacks, places number/layout groups and draws them. ViewRank owns its
// layouts/message wrappers; registries own BP entries and number renderers.
// Shared layouts: CIngameStatus, ViewRank, CSharedDoc, CGolfDoc, WFixedFont.
// Pools: u803280FC_data.cpp and the existing ingameStatusStrings/rankingViewStrings.
// Modder lookup:
// - Frame rate/HUD animation: CIngameStatus_UpdateAndDraw invokes the BP callback
//   once in its final animation phase, plus another call in the visible/default
//   branch while tutorialStage<kStatusDefaultScriptTutorialLimit. Keep both calls:
//   callback count per presentation is observable. BP scripts status_IN/status_OUT
//   control showing/hiding; BpObject_IsCurrentScriptComplete changes HUD state.
// - Delta-driven counter: Update forwards delta to shotCountInfo ONLY in Visible;
//   its virtual tick consumes seconds (CShotCountInfo_UpdateFade in golf_hud_flight_info_and_pang_rewards.cpp).
//   This is a separate timing path from draw-driven BP callbacks.
// - Widescreen placement: UpdateAndDraw samples s_ingameStatusPositionGroupNames
//   from the BP layout and passes float XY pairs in B,A,C order to the embedded
//   layout interface at +0x88: icon, label, gauges (CIngameStatus2d_SetIconLabelGaugePositions in
//   create_new_account_and_ingame_status.cpp). Preserve this argument order and group identities.
// - Darts slash: kFloat_5_5_StatusSlashOffsetX in the data pool adds 5.5 layout
//   units to the R_NUM_SLASH_K1 anchor; the offset stays 5.5 layout units as aspect changes.
// - Ranking assets/numbers: RankTextOffset selects layout/BP/message/font paths;
//   kRankingNumberEntryCount and kRankingMenuScriptIndex describe the registration.
// Neither unit sets output resolution, VI retrace interval or camera projection.
// Preserve asynchronous readiness gates, show/hide vs accessory-state protocols,
// per-player score representations, callback count and destructor/registry order.
// UI anchors are layout coordinates (+X right,+Y down), not world positions.
extern "C" char s_bpCtrlDtorRecord_Menu[];
extern "C" char s_bpCtrlHandleDtorRegistration_Ranking[];
extern "C" {
extern unsigned char s_statusOpacityGroupNames[]; // HUD opacity group names.
extern unsigned char s_statusPartyGroupNames[]; // Party HUD group names.
extern unsigned char s_statusTintGroupNames[]; // HUD tint group names.
}
// Byte offsets within the existing pooled strings; storage and encodings stay unchanged.
enum RankTextOffset {
    kRankTextLayoutFile = 0xc,   // ranking layout
    kRankTextRankBpFile = 0x1b,   // ranking BpObject
    kRankTextScoreBpFile = 0x2a,   // score BpObject
    kRankTextMessageFile = 0x40,   // ranking messages
    kRankTextNumberFont = 0x4c,   // ranking number glyphs
};

// This TU's destructor registrations share one storage block.
struct RankGlobalFields {
    char reserved[0x0C];
    char numberRegistryDtor[0x0C];
    char bpControllerDtor[0x0C];
    char menuScriptDtor[0x0C];
    char resourceTableDtor[0x0C];
    char reserved3C[4];
    char messagePath[1];                 // start of the existing writable path buffer
};
struct StatusLayoutInterface {
    char reserved[0x88];
    void** methods;
};
struct StatusSharedPlayer {
    char reserved[0x120];
    void* characterId;
    char reserved124[0xB0];
};
struct StatusBallFields {
    char reserved[0x18];
    int state;
};
enum StatusDataOffset {
    kStatusOpacityGroupNames = 0x48,
    kStatusPartyGroupNames = 0xB8,
    kStatusTintGroupNames = 0x13C
};

#include "classes/GolfDocSnapshot8013.h"

// Party score storage uses the same word for darts points and distance.
struct PartyPlayerFields {
    char reserved[0x178];
    union {
        int dartsPoints;
        float distance;
    }
    score;
    int holePoints[18];
    int totalPoints;
};
struct StatusNumberFields {
    char reserved[0x28];
    int enabled;
};
struct AccessoryHudFields {
    char reserved[0x68];
    void* display;
};
struct AccessoryDisplayFields {
    char reserved[0x10];
    int state;
};
struct ChatFontFields {
    char reserved[0x10];
    void* fixedFont;
};

enum StatusState {
    kStatusVisible, kStatusHidden, kStatusShowing, kStatusHiding
};
enum StatusTextEntry {
    kStatusStrokeNumber = 0xD, kStatusScoreNumber = 0xE,
    kStatusDartsTargetNumber = 0xF, kStatusDartsPointsNumber = 0x11,
    kStatusPlayerRecordStride = 0x1D4,
    kStatusDefaultScriptTutorialLimit = 6,
    kRankingNumberEntryCount = 16,
    kRankingMenuScriptIndex = 0x12
};
enum StatusPartyMode {
    kPartyNone, kPartyDarts, kPartyDistance, kPartyPoints
};
// Individual CGolfDoc::gameStateFlags bits; values/roles agree with GolfRuleState
// and its dispatch in golf_rule_round_flow_and_shot_sim.cpp. Eligibility sets below are composite masks.
enum StatusGolfStateBit {
    kStatusPlayerPreview = 0x2,
    kStatusShowTrigger = 0x8, // aiming, before entering a swing
    kStatusBeginButtonSwing = 0x10,
    kStatusButtonSwing = 0x20,
    kStatusBallFlight = 0x80,
    kStatusHoleResults = 0x2000,
    kStatusMotionSwing = 0x08000000
};
enum StatusTransitionMask {
    // Hide when leaving preview/aim/button-swing/motion-swing for a target
    // outside that set AND outside the button-swing startup state.
    kStatusHideSourceMask = kStatusPlayerPreview | kStatusShowTrigger |
        kStatusButtonSwing | kStatusMotionSwing,
    kStatusShowSourceMask = ~(kStatusShowTrigger | kStatusBeginButtonSwing |
        kStatusButtonSwing | kStatusMotionSwing),
    kStatusHideTargetMask = ~(kStatusPlayerPreview | kStatusShowTrigger |
        kStatusBeginButtonSwing | kStatusButtonSwing | kStatusMotionSwing)
};
// The accessory display has a different state machine (party_goods_item_display.cpp):
// HUD request             HUD state       accessory command
// show                   Showing=2       StartIn=2
// hide with animation    Hiding=3        StartOut=5
// hide at hole results   Hidden=1        Hide=7
// Equal values for show do not make these interchangeable protocols.
enum StatusAccessoryCommand {
    kStatusAccessoryStartIn = 2,
    kStatusAccessoryStartOut = 5,
    kStatusAccessoryHide = 7
};

// Byte offsets within the existing pooled strings; storage and encodings stay unchanged.
enum StatusTextOffset {
    kStatusTextNumberFont = 0x15,   // score_num11_20.gsr
    kStatusTextTargetPoints = 0x28,   // R_NUM_TARGETPOINT_K3_LC
    kStatusTextDefaultScript = 0x40,   // default
    kStatusTextShowScript = 0x48,   // status_IN
    kStatusTextHideScript = 0x52,   // status_OUT
    kStatusTextDartsActor = 0x5d,   // DartsSystem
    kStatusTextSlash = 0x69,   // /
    kStatusTextSlashGroup = 0x6b,   // R_NUM_SLASH_K1
};

#include "classes/CGolfDoc.h"
// Borrow the current round's document; direct expansion preserves the HUD draw
// callback path's original reload scheduling. The HUD owns no player records.
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#include "classes/WFixedFont.h"
// Borrow the persistent document with the same direct-cast form for those reloads.
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/ViewRank.h"
// Borrow ranking fields; direct expansion also retains the compiler-generated
// local-string destructor's symbol identity in the exception cleanup tables.
#define AsViewRank(p) ((ViewRank*)(p))
// Translation unit 0x803280FC-0x803297B8 (19 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CIngameStatus.h"

// Borrow HUD fields; direct expansion preserves UpdateAndDraw's load ordering
// around virtual calls. An inline-function adapter changes its generated code.
#define AsCIngameStatus(p) ((CIngameStatus*)(p))

struct Names289FC {
    void* labelGroupName;
    void* iconGroupName;
    void* gaugeGroupName;
    void* unusedFourthName;
};
extern "C" {
    extern char ingameStatusNumberGroupName[];
    extern Names289FC s_ingameStatusPositionGroupNames;
    extern void* g_sharedDoc;
    extern char ingameStatusStrings[256];
    extern char s_ingameStatusNumberEntries[];
    int Layout_PollLoadComplete(void* self);
    int BpObject_PollLoad(void* self);
    void* GolfPlayer_GetEntryByIndex(void* self, int index);
    void BpObject_TryInvokeCurrentCallback(void* self, int enabled);
    void BpObject_Draw(void* self);
    void BpObject_StartScriptByName(void* self, void* name, int flag);
    int BpObject_IsCurrentScriptComplete(void* self);
    void UITextTable_SetInteger(void* self, int id, int value);
    void* Object_GetChild(void* self);
    void* TaskActorTable_FindByName(void* a, void* b);
    void UIGroup_GetPosition(void* self, void* out);
    void HudFont_DrawAlignedTextWithVariant(void* a, void* b, int c, int d, int i, float e, float f, float g, float h);
    void Layout_DrawWithOpacity(void* self, float value);
    void UITextTable_Draw(void* self);
    void* BpObject_GetOrCreateNamedGroup(void* self, void* name);
    extern void* g_taskManager;
    extern float kFloat_1_IngameStatus_805E1548;
    extern float kFloat_5_5_StatusSlashOffsetX;
    extern float kZero_StatusSlashDrawArgumentSlot;
    void UIGroup_SetEnabled(void* self, int value);
    void UIGroup_SetOpacity(void* self, int value);
    void UIGroup_SetTintColor(void* self, int value);
    // Legacy symbol returns the party-item slot manager (party_goods_item_display.cpp), whose
    // display field is at +0x68; it is not the pad-history CAccState object.
    void* PartyItemSlots_GetInstance(void);
    void* GolfBall__Fv(void);
    void BpObject_SetEnabled(void* self, int enabled);
    void CFontNumDraw_RegisterEntries(void* a, void* b, int c, void* d, void* e);

    // Observe round-state changes and tick the visible shot counter; preserve transition priority and delta gating.
    // statusObject is required. delta seconds is forwarded unchanged to the counter;
    // it is not a presentation-frame count. Unready layout
    // is polled through its virtual interface then returns; unready BP returns.
    // Reads borrowed current-player/document storage; writes HUD/accessory state,
    // visibility, group opacity/tint and saved flags. No ownership transfer.
    void CIngameStatus_Update(void* statusObject, float delta) {
        // Phase 1: wait for both UI resources, then configure each group once.
        // delta is forwarded to shotCountInfo only while the HUD is visible.
        char* statusDataBase = (char*)ingameStatusNumberGroupName;
        void* self = statusObject;
        struct Names4 {
            void* a;
            void* b;
            void* c;
            void* d;
        };
        struct Names5 {
            void* v[5];
        };
        struct Receiver {
            char pad[0x88];
            virtual void slot0() DECOMP_LOCAL_VIRTUAL;
            virtual void slot1() DECOMP_LOCAL_VIRTUAL;
            // Unused signature retained for compiler-generated local symbol IDs;
            // the called view below declares the verified no-argument method.
            virtual void slot2(Names4 names) DECOMP_LOCAL_VIRTUAL;
            virtual void pollAndUpdate() DECOMP_LOCAL_VIRTUAL;
            virtual void slot4() DECOMP_LOCAL_VIRTUAL;
            virtual void slot5() DECOMP_LOCAL_VIRTUAL;
            virtual void slot6() DECOMP_LOCAL_VIRTUAL;
            virtual void setGaugeEnabled(int enabled) DECOMP_LOCAL_VIRTUAL;
        };
        if (Layout_PollLoadComplete(AsCIngameStatus(self)->layout) == 0) {
            ((Receiver*)AsCIngameStatus(self)->layout)->pollAndUpdate();
            return;
        }
        if (BpObject_PollLoad(AsCIngameStatus(self)->bpObject) == 0) return;
        if (AsCIngameStatus(self)->initialized == 0) {
            void* recv = AsCIngameStatus(self)->layout;
            struct Recv2 {
                char pad[0x88];
                virtual void slot0() DECOMP_LOCAL_VIRTUAL;
                virtual void slot1() DECOMP_LOCAL_VIRTUAL;
                virtual void resetElements() DECOMP_LOCAL_VIRTUAL;
            };
            // The vptr is at +0x88; MWCC virtual declarations account for the
            // two ABI header words before method 0. CIngameStatus2d_ResetElementsAndEnableGauge
            // takes only self and hides all 21 elements. names4 is a separate
            // saved value for the opacity loop below, not an implicit argument.
            Names4 names4 = *(Names4*)(((char*)(statusDataBase + kStatusOpacityGroupNames)));
            ((Recv2*)recv)->resetElements();
            if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == 1) {
                ((Receiver*)AsCIngameStatus(self)->layout)->setGaugeEnabled(0);
                Names5 names5 = *(Names5*)(((char*)(statusDataBase + kStatusPartyGroupNames)));
                for (unsigned i = 0; i < 5; ++i) {
                    void* item = BpObject_GetOrCreateNamedGroup(AsCIngameStatus(self)->bpObject, names5.v[i]);
                    UIGroup_SetEnabled(item, 0);
                }
            } else {
                ((Receiver*)AsCIngameStatus(self)->layout)->setGaugeEnabled(1);
            }
            void** p4 = (void**)&names4;
            for (unsigned i = 0; i < 4; ++i) {
                void* item = BpObject_GetOrCreateNamedGroup(AsCIngameStatus(self)->bpObject, p4[i]);
                UIGroup_SetOpacity(item, 0);
            }
            CFontNumDraw_RegisterEntries(AsCIngameStatus(self)->textTable, s_ingameStatusNumberEntries, 5, AsCIngameStatus(self)->bpObject, ingameStatusStrings + kStatusTextNumberFont);
            {
                struct Setter {
                    virtual void slot0() DECOMP_LOCAL_VIRTUAL;
                    virtual void slot1() DECOMP_LOCAL_VIRTUAL;
                    virtual void slot2() DECOMP_LOCAL_VIRTUAL;
                    virtual void setText(void*, int, int) DECOMP_LOCAL_VIRTUAL;
                };
                Setter* obj = (*(Setter**)&AsCIngameStatus(self)->textTable);
                obj->setText(ingameStatusStrings + kStatusTextNumberFont, 0, 1);
            }
            Names5 tags = *(Names5*)(((char*)(statusDataBase + kStatusTintGroupNames)));
            for (unsigned i = 0; i < 5; ++i) {
                void* item = BpObject_GetOrCreateNamedGroup(AsCIngameStatus(self)->bpObject, tags.v[i]);
                UIGroup_SetTintColor(item, -1);
            }
            void* item = BpObject_GetOrCreateNamedGroup(AsCIngameStatus(self)->bpObject, ingameStatusStrings + kStatusTextTargetPoints);
            UIGroup_SetTintColor(item, 0xff00f0ff);
            AsCIngameStatus(self)->initialized = 1;
        }
        // Phase 2: translate game-state changes into HUD/accessory show/hide requests.
        void* sharedDoc = g_sharedDoc;
        unsigned previousFlags = AsCIngameStatus(self)->savedPlayerFlags;
        unsigned currentFlags = (*(unsigned*)&AsCGolfDoc((void*)AsCSharedDoc(sharedDoc)->golfDoc)->gameStateFlags);
        if (previousFlags != currentFlags) {
            if ((previousFlags & kStatusShowSourceMask) != 0 && (currentFlags & kStatusShowTrigger) != 0) {
                AsCIngameStatus(self)->state = kStatusShowing;
                BpObject_SetEnabled(AsCIngameStatus(self)->bpObject, 1);
                void* accessoryHud = PartyItemSlots_GetInstance();
                if (((AccessoryHudFields*)accessoryHud)->display)
                    ((AccessoryDisplayFields*)((AccessoryHudFields*)accessoryHud)->display)->state = kStatusAccessoryStartIn;
            } else if ((currentFlags & kStatusHideTargetMask) != 0 && (previousFlags & kStatusHideSourceMask) != 0) {
                AsCIngameStatus(self)->state = kStatusHiding;
                void* accessoryHud = PartyItemSlots_GetInstance();
                if (((AccessoryHudFields*)accessoryHud)->display)
                    ((AccessoryDisplayFields*)((AccessoryHudFields*)accessoryHud)->display)->state = kStatusAccessoryStartOut;
            }
            AsCIngameStatus(self)->savedPlayerFlags = (*(unsigned*)&AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags);
        }
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) != 0 && AsCIngameStatus(self)->state == kStatusHidden &&
        ((*(unsigned*)&AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags) & kStatusBallFlight) != 0) {
            if (((StatusBallFields*)GolfBall__Fv())->state == 0) {
                AsCIngameStatus(self)->state = kStatusShowing;
                BpObject_SetEnabled(AsCIngameStatus(self)->bpObject, 1);
            }
        }
        if (AsCIngameStatus(self)->state == kStatusVisible &&
        (*(unsigned*)&AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->gameStateFlags) == kStatusHoleResults) {
            BpObject_SetEnabled(AsCIngameStatus(self)->bpObject, 0);
            ((StatusNumberFields*)AsCIngameStatus(self)->textTable)->enabled = 0;
            void* accessoryHud = PartyItemSlots_GetInstance();
            if (((AccessoryHudFields*)accessoryHud)->display)
                ((AccessoryDisplayFields*)((AccessoryHudFields*)accessoryHud)->display)->state = kStatusAccessoryHide;
            AsCIngameStatus(self)->state = kStatusHidden;
        }
        if (AsCIngameStatus(self)->state == kStatusVisible) {
            struct Ticker {
                virtual void slot0() DECOMP_LOCAL_VIRTUAL;
                virtual void tick(float dt) DECOMP_LOCAL_VIRTUAL;
            };
            Ticker* obj = (*(Ticker**)&AsCIngameStatus(self)->shotCountInfo);
            if (obj) obj->tick(delta);
        }
    }

    // Refresh player HUD anchors/scores and draw it; preserve BP callback frequency and B,A,C anchor order.
    // self must be constructed; uninitialized returns without drawing. Current
    // player/hole indices must be valid for the loaded round. Darts requires the
    // DartsSystem callback to fill targetScore; absent actor has no fallback.
    // Borrows player/BP/layout storage, changes scripts, enabled flags and number
    // values. Group XY and the slash anchor are float layout coordinates.
    // No time delta is consumed here: BP advancement follows invocation count.
    void CIngameStatus_UpdateAndDraw(void* self) {
        struct Pair {
            // Equivalent data is {float x,y}; word storage preserves the
            // original bulk-copy instruction scheduling between the six pairs.
            int xFloatBits, yFloatBits;
        };
        struct Receiver {
            char pad[0x88];
            virtual void slot0() DECOMP_LOCAL_VIRTUAL;
            virtual void slot1() DECOMP_LOCAL_VIRTUAL;
            virtual void slot2() DECOMP_LOCAL_VIRTUAL;
            virtual void draw() DECOMP_LOCAL_VIRTUAL;
            virtual void slot4() DECOMP_LOCAL_VIRTUAL;
            virtual void set(void*, float) DECOMP_LOCAL_VIRTUAL;
            virtual void update(Pair*, Pair*, Pair*) DECOMP_LOCAL_VIRTUAL;
        };
        struct Callback {
            virtual void slot0() DECOMP_LOCAL_VIRTUAL;
            virtual void slot1() DECOMP_LOCAL_VIRTUAL;
            virtual void slot2() DECOMP_LOCAL_VIRTUAL;
            virtual void slot3() DECOMP_LOCAL_VIRTUAL;
            virtual void slot4() DECOMP_LOCAL_VIRTUAL;
            virtual void slot5() DECOMP_LOCAL_VIRTUAL;
            virtual void slot6() DECOMP_LOCAL_VIRTUAL;
            virtual void slot7() DECOMP_LOCAL_VIRTUAL;
            virtual void targetScore() DECOMP_LOCAL_VIRTUAL;
            virtual void handleDartsRequest(void*) DECOMP_LOCAL_VIRTUAL;
        };
        void* status = self;
        if (AsCIngameStatus(status)->initialized == 0) return;
        // Phase 1: sample the current player. Party modes reuse score storage:
        // points uses totalPoints, distance truncates its float, darts uses two fields.
        Names289FC strings = s_ingameStatusPositionGroupNames;
        void* playerRecord = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
        unsigned char holeStrokes;
        int scoreValue = 0;
        int showStandardScore = 1;
        void* golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        holeStrokes = ((GolfDocSnapshot8013*)playerRecord)->holeStrokeCounts[AsCGolfDoc(golfDoc)->currentHoleIndex];
        switch ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode)) {
            case kPartyPoints:
            scoreValue = ((PartyPlayerFields*)GolfPlayer_GetEntryByIndex(golfDoc, AsCGolfDoc(golfDoc)->currentPlayer))->totalPoints;
            break;
            case kPartyDarts:
            showStandardScore = 0;
            break;
            case kPartyDistance:
            // Driving Contest score in display-distance units: shot XZ distance
            // /3.200000047683716 plus awarded bonus distances, not world units.
            // CDriveSystem_HandleMsg request 0 (drive_contest_and_darts.cpp) truncates the shot to
            // hundredths and returns a separate 100-unit hole-in bonus; the rule
            // (golf_rule_round_flow_and_shot_sim.cpp) stores shot+accumulated bonus in per-stroke +178 entries.
            // CDriveInfo_UpdateDistanceAnimation (golf_hud_flight_info_and_pang_rewards.cpp) animates/commits their
            // total to this +178 float. This HUD cast truncates toward zero AGAIN
            // to whole score units (positive 123.99 -> 123), with no rounding or
            // conversion. Physical yard equivalence of bonus score is not proven;
            // treat it as the contest's distance score, not measured flight length.
            // Requires finite value representable by int; NaN/overflow unchecked.
            scoreValue = (int)((PartyPlayerFields*)GolfPlayer_GetEntryByIndex(golfDoc, AsCGolfDoc(golfDoc)->currentPlayer))->score.distance;
            break;
            default:
            scoreValue = (int)((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(golfDoc, AsCGolfDoc(golfDoc)->currentPlayer))->roundResultWords[0];
            break;
        }
        ((StatusNumberFields*)AsCIngameStatus(status)->textTable)->enabled = 0;
        BpObject_SetEnabled(AsCIngameStatus(status)->bpObject, 1);
        // Phase 2: enter/finish scripts once per transition; hidden disables the BP object.
        int state = AsCIngameStatus(status)->state;
        switch (state) {
            case kStatusHidden:
            ((StatusNumberFields*)AsCIngameStatus(status)->textTable)->enabled = 0;
            BpObject_SetEnabled(AsCIngameStatus(status)->bpObject, 0);
            break;
            case kStatusVisible:
            if (AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->tutorialStage < kStatusDefaultScriptTutorialLimit) {
                BpObject_StartScriptByName(AsCIngameStatus(status)->bpObject, (char*)&ingameStatusStrings + kStatusTextDefaultScript, 1);
                BpObject_TryInvokeCurrentCallback(AsCIngameStatus(status)->bpObject, 0);
            }
            break;
            case kStatusShowing:
            if (AsCIngameStatus(status)->animationState != kStatusShowing) {
                BpObject_StartScriptByName(AsCIngameStatus(status)->bpObject, (char*)&ingameStatusStrings + kStatusTextShowScript, 1);
                AsCIngameStatus(status)->animationState = kStatusShowing;
            } else if (BpObject_IsCurrentScriptComplete(AsCIngameStatus(status)->bpObject)) {
                BpObject_StartScriptByName(AsCIngameStatus(status)->bpObject, (char*)&ingameStatusStrings + kStatusTextDefaultScript, 1);
                AsCIngameStatus(status)->state = kStatusVisible;
                AsCIngameStatus(status)->animationState = kStatusVisible;
            }
            break;
            case kStatusHiding:
            if (AsCIngameStatus(status)->animationState != kStatusHiding) {
                BpObject_StartScriptByName(AsCIngameStatus(status)->bpObject, (char*)&ingameStatusStrings + kStatusTextHideScript, 1);
                AsCIngameStatus(status)->animationState = kStatusHiding;
            } else if (BpObject_IsCurrentScriptComplete(AsCIngameStatus(status)->bpObject)) {
                AsCIngameStatus(status)->state = kStatusHidden;
                AsCIngameStatus(status)->animationState = kStatusHidden;
            }
            break;
        }
        BpObject_TryInvokeCurrentCallback(AsCIngameStatus(status)->bpObject, 0);
        state = AsCIngameStatus(status)->state;
        Pair iconAnchorWords, labelAnchorWords, gaugeAnchorWords;
        float xy[2];
        Pair iconPositionWords, labelPositionWords, gaugePositionWords;
        // Phase 3: refresh numbers and layout anchors for visible/animating states.
        // A/B/C are the first three names in s_ingameStatusPositionGroupNames.
        // CIngameStatus2d_SetIconLabelGaugePositions consumes B for the current icon, A for its label,
        // C for gauge elements 0x11..0x14; update therefore takes B,A,C. The group
        // reader returns float XY after adding its immediate parent-item offset,
        // without recursive ancestor transforms or scale/rotation (ui_group_and_bp_edit_sprite.cpp).
        if (state == kStatusShowing || state == kStatusHiding || state == kStatusVisible) {
            UITextTable_SetInteger(AsCIngameStatus(status)->textTable, kStatusStrokeNumber, holeStrokes);
            if (showStandardScore) UITextTable_SetInteger(AsCIngameStatus(status)->textTable, kStatusScoreNumber, scoreValue);
            else {
                void* dartsActorOrTaskContext = g_taskManager;
                if (dartsActorOrTaskContext) dartsActorOrTaskContext = Object_GetChild(dartsActorOrTaskContext);
                else dartsActorOrTaskContext = 0;
                int targetScore;
                int holeIndex = AsCGolfDoc((void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentHoleIndex;
                // Required live DartsSystem actor: task child name lookup ->
                // __vt__12CDartsSystem slot +0x2C -> CDartsSystem_HandleMsg
                // (drive_contest_and_darts.cpp), request 0. Its DartsInfo consumes only the first
                // 0x10 bytes: unused +0, opcode +4, writable int* +8, hole +C.
                // The remaining four zero words preserve this caller's packet
                // extent; the receiver does not read them. Hole outside 0..17
                // selects hole zero; initialized aimTable/holeEntryIds and a
                // nonnull CMiniGameAim_GetEntry result are required. Handler
                // writes the first nonzero target value, or original slot zero
                // if all are zero, synchronously; no output pointer is retained.
                // The lookup's null guard does not validate targetScore: absent
                // actor leaves it uninitialized, then it is consumed below.
                // Caller must ensure actor admission, not rely on a zero score.
                struct DartsTargetRequest {
                    int unused, opcode;
                    int* targetOutput;
                    int holeIndex;
                    int reserved0, reserved1, reserved2, reserved3;
                };
                DartsTargetRequest params;
                params.unused = 0;
                params.opcode = 0;
                params.targetOutput = &targetScore;
                params.holeIndex = holeIndex;
                params.reserved0 = 0;
                params.reserved1 = 0;
                params.reserved2 = 0;
                params.reserved3 = 0;
                dartsActorOrTaskContext = TaskActorTable_FindByName(dartsActorOrTaskContext, (char*)&ingameStatusStrings + kStatusTextDartsActor);
                if (dartsActorOrTaskContext) {
                    ((Callback*)dartsActorOrTaskContext)->handleDartsRequest(&params);
                }
                // The DartsSystem callback must fill targetScore; this path has no fallback.
                UITextTable_SetInteger(AsCIngameStatus(status)->textTable, kStatusDartsTargetNumber, targetScore);
                void* p = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
                void* currentPlayer = GolfPlayer_GetEntryByIndex(p, AsCGolfDoc(p)->currentPlayer);
                UITextTable_SetInteger(AsCIngameStatus(status)->textTable, kStatusDartsPointsNumber, ((PartyPlayerFields*)currentPlayer)->score.dartsPoints);
            }
            ((StatusNumberFields*)AsCIngameStatus(status)->textTable)->enabled = 1;
            UIGroup_GetPosition(BpObject_GetOrCreateNamedGroup(AsCIngameStatus(status)->bpObject, strings.labelGroupName), &labelAnchorWords);
            UIGroup_GetPosition(BpObject_GetOrCreateNamedGroup(AsCIngameStatus(status)->bpObject, strings.iconGroupName), &iconAnchorWords);
            UIGroup_GetPosition(BpObject_GetOrCreateNamedGroup(AsCIngameStatus(status)->bpObject, strings.gaugeGroupName), &gaugeAnchorWords);
            gaugePositionWords = gaugeAnchorWords;
            labelPositionWords = labelAnchorWords;
            iconPositionWords = iconAnchorWords;
            ((*(Receiver**)&AsCIngameStatus(status)->layout))->update(&iconPositionWords, &labelPositionWords, &gaugePositionWords);
            void* p = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            void* currentPlayer = GolfPlayer_GetEntryByIndex(p, AsCGolfDoc(p)->currentPlayer);
            ((*(Receiver**)&AsCIngameStatus(status)->layout))->set(currentPlayer, ((GolfDocSnapshot8013*)currentPlayer)->powerShotMeter);
        }
        // Phase 4: draw BP first; layout/numbers follow while not hidden.
        BpObject_Draw(AsCIngameStatus(status)->bpObject);
        if (AsCIngameStatus(status)->state != kStatusHidden) {
            ((*(Receiver**)&AsCIngameStatus(status)->layout))->draw();
            Layout_DrawWithOpacity(AsCIngameStatus(status)->layout, kFloat_1_IngameStatus_805E1548);
            UITextTable_Draw(AsCIngameStatus(status)->textTable);
            if (!showStandardScore) {
                char temp[16];
                sprintf(temp, (char*)&ingameStatusStrings + kStatusTextSlash);
                UIGroup_GetPosition(BpObject_GetOrCreateNamedGroup(AsCIngameStatus(status)->bpObject, (char*)&ingameStatusStrings + kStatusTextSlashGroup), xy);
                HudFont_DrawAlignedTextWithVariant(AsCIngameStatus(status)->textTable, temp, 1, 0, 0, kFloat_5_5_StatusSlashOffsetX + xy[0], xy[1], kZero_StatusSlashDrawArgumentSlot, kFloat_1_IngameStatus_805E1548);
            }
        }
        if (AsCIngameStatus(status)->animationState == kStatusVisible && BpObject_IsCurrentScriptComplete(AsCIngameStatus(status)->bpObject)) {
            BpObject_SetEnabled(AsCIngameStatus(status)->bpObject, 0);
            ((StatusNumberFields*)AsCIngameStatus(status)->textTable)->enabled = 0;
        }
    }
}   // extern "C"
extern "C" {
    extern float s_statusUiTwoPiSlotA;
    extern float s_statusUiTwoPiSlotB;
    extern float kFloat_6_2831855;
    extern float s_viewRankTwoPiSlot;

    void BpObject_SetEnabled(void* self, int enabled);
    void CShotCountInfo_ResetFade(void* self);
    void* CFontNumDraw_CreateInVector(void* self);
    extern void* g_sharedDoc;
    void Layout_Update(void* self);
    void CPackTextureResource_PollLoad(void* self);
    void* ItemInventory_FindCharacterIndex(void* a, void* b);
    int SharedDoc_IsPlayerFlag0Set(int x);
    int CBpCtrl_PollAllEntriesLoaded(void* self);
    int Layout_PollLoadComplete(void* self);
    int CSharedFilePtr_GetLoadedBuffer(void* self);
    int CPackTextureResource_IsLoadFinished(void* self);
    void __ct__7CBpCtrlFv(void* self);
    void CBpCtrl_dtor(void* self);
    void* CBpCtrl_CreateEntry(void* self, void* name, void* out, int flags);
    void* IdMap_Lookup(void* self, int id);
    void CIngameStatus2d_Ctor(void* self);
    void __ct__14CShotCountInfoFv(void* self);
    void PointerVector_DestroyStorage(void* self);
    extern char ingameStatusStrings[256];
    extern signed char s_bpCtrlInitialized;
    extern char s_bpCtrl[];
    extern char s_bpCtrlDtorRecord_Menu[];
    extern signed char s_pointerVectorInitialized;
    extern char s_fontNumDrawRegistry[];
    extern char s_fontNumDrawRegistryDtorRegistration_FontNumDraw[];
    extern void* g_chatMsg;
    extern char rankingViewStrings[];
    extern char s_rankingNumberEntries[];
    extern signed char s_bpCtrlHandleInitialized;
    extern int s_bpCtrlHandle;
    extern char s_bpCtrlHandleDtorRegistration_Ranking[]; // Ranking control-handle destructor-registration record.
    extern char s_bpCtrlDtorRegistration_803280FC[];

    void CBpCtrl_DeleteEntryById(void* self, int x);
    void Layout_Destroy(void* self, int x);
    void CSharedFileHandle_Destroy(void* self, int x);
    void CBpCtrlHandle_ReleaseEntry(void* self);
    void UITextTable_ClearEntries(void* self);
    void PointerVector_InitReserved(void* self);
    void* PointerArray_DeleteValue(void* a, void* b);
    void __dl__FPv(void* p);
    void* __nw__FUl(unsigned long n);
    void CMsgDraw_Ctor(void* self, void* arg);
    void ViewRank_StartRankingAnimation(void* self);
    void ViewRank_ComputeCharacterRanks(void* self);
    void ViewRank_RefreshPlayerScores(void* self);
    void* IdMap_Lookup(void* a, int b);
    void CFontNumDraw_RegisterEntries(void* a, void* b, int c, void* d, void* e);
    void CBpCtrlHandle_SetScriptIndex(void* a, int b);
    void CBpCtrlHandle_Dtor(void* self);
    void PointerVector_DestroyStorage(void* self);
    void WFixedFont_CreateMsgEcbResource(void* a, void* b, void* c, int d);
    void MPBase1_Ctor(void* self);
    void* ResourceTable_FindEntryByName(void* self, void* name, void* out);
    void* CPackTextureResource_Init(void* self, void* name, void* data, void* handle);
    void Layout_Init(void* self, int count);
    void Layout_LoadExtended(void* self, int index, void* name);
    void NString_AssignCString(void* self, void* name);
    void* SharedFile_Create(void* self, int flags);
    void appFreeUni__FPv(void* self);

    void __dt__16CWarningLogoBaseFv(void* self);
    void Allocation_Dtor(void* self);
    extern signed char s_resourceTableInitialized;
    extern void* s_resourceTableStorage;
    extern char __vt__8ViewRank[];
    extern char s_inputManagerDtorRegistration_Ranking[];
    extern char rankingViewStrings[];
    extern char s_bpCtrlDtorRecord_Menu[];
    extern char s_fontNumDrawRegistryDtorRegistration_FontNumDraw[];
    extern char kFloat_200_ClubSubMenu[];
    extern char s_fontNumDrawRegistry[];
    extern signed char s_pointerVectorInitialized;
    void PointerVector_InitReserved(void* self);
    void* CFontNumDraw_CreateInVector(void* self);

    // Request the HUD hide animation.
    void CIngameStatus_BeginHide(void* self) {
        AsCIngameStatus(self)->state = kStatusHiding;
    }

    // Release ranking layouts, text, textures, BpObject entries, and the shared menu script handle.
    // Null self is accepted and returns null without touching any global service.
    // Otherwise require a fully constructed ViewRank, with nullable layout/text/
    // texture/file pointers and absent BP IDs=-1, but NONNULL numberDraw:
    // UITextTable_ClearEntries dereferences it before the later null guard.
    // That guard does not make partial construction safe. flag>0 frees the wrapper;
    // flag<=0 only tears down resources. Both return the ORIGINAL address as an
    // ABI artifact: it owns no live rank view afterwards, and is dangling when
    // deleting. Do not dereference it, use it as a result, or destroy it twice
    // (numberDraw is removed from its registry but not cleared locally).
    // The global menu entry is released last, not restored to a prior index.
    void* ViewRank_dtor(void* self, int flag) {
        struct VirtualObject {
            virtual ~VirtualObject() DECOMP_LOCAL_VIRTUAL;
        };
        struct GlobalPair {
            int a, b;
            // Mark both registry handles absent before their first use.
            void init() {
                a = b = -1;
            }
        };
        void* registrationBase = s_inputManagerDtorRegistration_Ranking;
        void* rankView = self;
        int deleteFlag = flag;
        if (self != 0) {
            *(void**)self = __vt__8ViewRank;
            if (AsViewRank(rankView)->layout != 0) {
                Layout_Destroy(AsViewRank(rankView)->layout, 1);
                AsViewRank(rankView)->layout = 0;
            }
            VirtualObject* messageDraw = (*(VirtualObject**)&AsViewRank(rankView)->messageDraw);
            if (messageDraw != 0) {
                delete messageDraw;
                AsViewRank(rankView)->messageDraw = 0;
            }
            VirtualObject* taggedText = (*(VirtualObject**)&AsViewRank(rankView)->taggedText);
            if (taggedText != 0) {
                delete taggedText;
                AsViewRank(rankView)->taggedText = 0;
            }
            if (AsViewRank(rankView)->sharedFile != 0) {
                CSharedFileHandle_Destroy(AsViewRank(rankView)->sharedFile, 1);
                AsViewRank(rankView)->sharedFile = 0;
            }
            VirtualObject* textureResource = (*(VirtualObject**)&AsViewRank(rankView)->textureResource);
            if (textureResource != 0) {
                delete textureResource;
                AsViewRank(rankView)->textureResource = 0;
            }
            if (AsViewRank(rankView)->entryId != -1) {
                if (Sing_805DC5F0() == 0) {
                    if (s_bpCtrlInitialized == 0) {
                        __ct__7CBpCtrlFv(s_bpCtrl);
                        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (((RankGlobalFields*)registrationBase)->bpControllerDtor));
                        s_bpCtrlInitialized = 1;
                    }
                    Sing_805DC5F0() = s_bpCtrl;
                }
                CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsViewRank(rankView)->entryId);
                AsViewRank(rankView)->entryId = -1;
            }
            if (AsViewRank(rankView)->secondaryEntryId != -1) {
                if (Sing_805DC5F0() == 0) {
                    if (s_bpCtrlInitialized == 0) {
                        __ct__7CBpCtrlFv(s_bpCtrl);
                        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (((RankGlobalFields*)registrationBase)->bpControllerDtor));
                        s_bpCtrlInitialized = 1;
                    }
                    Sing_805DC5F0() = s_bpCtrl;
                }
                CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsViewRank(rankView)->secondaryEntryId);
                AsViewRank(rankView)->secondaryEntryId = -1;
            }
            UITextTable_ClearEntries(AsViewRank(rankView)->numberDraw);
            if (AsViewRank(rankView)->numberDraw != 0) {
                if (Sing_805DC830() == 0) {
                    if (s_pointerVectorInitialized == 0) {
                        PointerVector_InitReserved(s_fontNumDrawRegistry);
                        __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, ((RankGlobalFields*)registrationBase)->numberRegistryDtor);
                        s_pointerVectorInitialized = 1;
                    }
                    Sing_805DC830() = s_fontNumDrawRegistry;
                }
                PointerArray_DeleteValue(Sing_805DC830(), AsViewRank(rankView)->numberDraw);
            }
            if (Sing_805DC5F8() == 0) {
                if (s_bpCtrlHandleInitialized == 0) {
                    ((GlobalPair*)&s_bpCtrlHandle)->init();
                    __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (((RankGlobalFields*)registrationBase)->menuScriptDtor));
                    s_bpCtrlHandleInitialized = 1;
                }
                Sing_805DC5F8() = &s_bpCtrlHandle;
            }
            CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());
            if (deleteFlag > 0) __dl__FPv(rankView);
        }
        return rankView;
    }

    // Initialize this UI subsystem angle constant to one full turn.
    void StatusUi_InitAngleConstantA() {
        s_statusUiTwoPiSlotA = kFloat_6_2831855;
    }

    // Initialize the second UI subsystem angle constant to one full turn.
    void StatusUi_InitAngleConstantB() {
        s_statusUiTwoPiSlotB = kFloat_6_2831855;
    }

    // Initialize the ranking subsystem angle constant to one full turn.
    void ViewRank_InitAngleConstant() {
        s_viewRankTwoPiSlot = kFloat_6_2831855;
    }

    // Create the ranking layout, score groups, message resource handle, and number renderer.
    // Required fresh storage and live resource/BP/number registries. Acquisition
    // and failure ledger (ordinary function, no constructor-wide unwind guard):
    // 1. MPBase1_Ctor, then null owner pointers/IDs=-1; globals may be lazily
    //    registered and remain process-owned even if subsequent work fails.
    // 2. ResourceTable_FindEntryByName supplies archive data/handle to the pack;
    //    textureResource publishes only after new Obj20 construction returns.
    // 3. layout publishes after new Layout5C; LoadExtended follows publication.
    //    Null layout is still passed to the load API: allocation success is a
    //    normal-use requirement, not a checked constructor failure result.
    // 4. Each CBpCtrl_CreateEntry publishes its ID separately; return codes are
    //    ignored. Failure can leave earlier entries/resources installed.
    // 5. Local owns temporary path text and frees it on scope exit/unwind.
    //    SharedFile_Create's handle publishes next (nullable, unchecked), then
    //    CFontNumDraw_CreateInVector publishes registry-owned numberDraw.
    // Return self does not promise readiness or nonnull acquisitions; vfunc_2
    // polls them. messageDraw/taggedText remain null until activation (vfunc_3).
    // A throwing new-expression frees its own failed object allocation; none
    // of the previously published owners/BP IDs or the explicit MPBase1 call
    // is rolled back here. Exceptions propagate. Do not use ViewRank_dtor as
    // general partial-construction recovery: it unconditionally clears entries
    // through numberDraw, which is zero until the FINAL acquisition. Even after
    // return, null numberDraw makes normal destruction unsafe. After successful
    // acquisition, dtor order is layout -> messageDraw -> taggedText -> file ->
    // pack -> both BP entries -> clear/remove numberDraw -> shared menu handle.
    void* ViewRank_Ctor(void* self) {
        // Phase 1: initialize the base and absent/null resource handles.
        struct Fields {
            void** vtable;
            char pad[0x14];
            int layout;
            int numberDraw;
            int messageDraw;
            int taggedText;
            int entryId, secondaryEntryId, activationDelay, numberEntriesDirty;
            char pad38[0x24];
            int sharedFile;
            int textureResource;
        };
        Fields* rankView = (Fields*)self;
        void* registrationBase = s_inputManagerDtorRegistration_Ranking;
        char* buffer;
        int zero;
        int negative;
        int initial;
        MPBase1_Ctor(self);
        negative = -1;
        zero = 0;
        initial = (int)Sing_805DC620();
        rankView->vtable = (void**)__vt__8ViewRank;
        rankView->layout = zero;
        rankView->numberDraw = zero;
        rankView->messageDraw = zero;
        rankView->taggedText = zero;
        rankView->entryId = negative;
        rankView->secondaryEntryId = negative;
        rankView->activationDelay = zero;
        rankView->numberEntriesDirty = 1;
        rankView->sharedFile = zero;
        rankView->textureResource = zero;
        void* out = (void*)zero;
        if (initial == 0) {
            if (s_resourceTableInitialized == 0) {
                s_resourceTableStorage = 0;
                __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, (((RankGlobalFields*)registrationBase)->resourceTableDtor));
                s_resourceTableInitialized = 1;
            }
            Sing_805DC620() = &s_resourceTableStorage;
        }
        void* handle = ResourceTable_FindEntryByName(Sing_805DC620(), rankingViewStrings, &out);
        // Phase 2: borrow archive data to construct the owned texture pack and layout.
        struct Obj20 {
            char bytes[0x20];
            // Construct a texture-pack owner using borrowed archive data and its handle.
            Obj20(void* name, void* data, void* handle) {
                CPackTextureResource_Init(this, name, data, handle);
            }
        };
        Obj20* objp = new Obj20(rankingViewStrings, out, handle);
        void* obj = objp;
        rankView->textureResource = (int)obj;
        struct Layout5C {
            char bytes[0x5c];
            // Initialize layout storage in the supplied engine layout mode.
            Layout5C(int n) {
                Layout_Init(this, n);
            }
        };
        Layout5C* layoutp = new Layout5C(5);
        void* layout = layoutp;
        rankView->layout = (int)layout;
        Layout_LoadExtended(layout, 0, (char*)rankingViewStrings + kRankTextLayoutFile);
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (((RankGlobalFields*)registrationBase)->bpControllerDtor));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        // Phase 3: register the ranking and score BP objects; IDs are registry handles.
        CBpCtrl_CreateEntry(Sing_805DC5F0(), (char*)rankingViewStrings + kRankTextRankBpFile, &rankView->entryId, 0);
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (((RankGlobalFields*)registrationBase)->bpControllerDtor));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        CBpCtrl_CreateEntry(Sing_805DC5F0(), (char*)rankingViewStrings + kRankTextScoreBpFile, &rankView->secondaryEntryId, 0);
        struct Local {
            int a, b, c;
            // Initialize an empty temporary engine string for the message-file path.
            Local() : a(0), b(0), c(0) {
            }
            // Release only heap storage after the path has been copied out.
            ~Local() {
                if (((unsigned)a) >> 31) appFreeUni__FPv((void*)c);
            }
        };
        {
            Local local;
            NString_AssignCString(&local, (char*)rankingViewStrings + kRankTextMessageFile);
            strcpy(((char*)((unsigned)registrationBase + (int)&((RankGlobalFields*)0)->messagePath)), !(((unsigned)local.a) >> 31) ? (char*)&local + 1 : (char*)local.c);
            buffer = ((char*)registrationBase + (int)&((RankGlobalFields*)0)->messagePath);
        }
        // Phase 4: retain the message-file handle and register the number renderer.
        void* resource = SharedFile_Create(buffer, 0);
        rankView->sharedFile = (int)resource;
        if (Sing_805DC830() == 0) {
            if (s_pointerVectorInitialized == 0) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, ((RankGlobalFields*)registrationBase)->numberRegistryDtor);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        rankView->numberDraw = (int)CFontNumDraw_CreateInVector(Sing_805DC830());
        return rankView;
    }

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
    // Create the status layout, number renderer, and shot counter for the current player.
    // Fresh self and live BP/number registries are required. Publication order:
    // absent/null prefix -> BP entry ID + borrowed BP lookup -> owned 2D layout
    // -> layout init -> registry-owned textTable -> optional player setup ->
    // owned shotCountInfo -> savedPlayerFlags. initialized stays 0 until Update
    // finishes loading/registering fonts; return self is not a readiness signal.
    // No allocation/status checks or constructor-wide unwind guard: a null layout
    // is immediately dereferenced, a null textTable survives until later use,
    // and a null shotCountInfo is stored (its update/reset are nullable). A new
    // whose constructor throws cleans its own failed allocation; previously
    // published resources/registry entries are not rolled back here. In particular
    // shotCountInfo is not initialized until the end, so partial self cannot be
    // passed to the normal cleanup as a recovery strategy. Successful services
    // and allocations are normal-use preconditions; exceptions propagate.
    // Normal CIngameStatus_Dtor below deletes shotCountInfo -> BP entry -> layout
    // -> clears/removes textTable from its registry -> optional object storage. The
    // borrowed BP pointer/player/document never transfer ownership to the HUD.
    void* CIngameStatus_Ctor(void* self) {
        struct Obj8C {
            char bytes[0x8c];
            // Construct the owned 2D status layout.
            Obj8C() {
                CIngameStatus2d_Ctor(this);
            }
        };
        struct Obj24 {
            char bytes[0x24];
            // Construct the owned shot-count animation helper.
            Obj24() {
                __ct__14CShotCountInfoFv(this);
            }
        };
        void* status;
        void* controller;
        void* zeroValue;
        void* missingEntry;
        void* golfDoc;
        void* resource;
        int alternateMode;
        zeroValue = (void*)0;
        missingEntry = (void*)-1;
        controller = Sing_805DC5F0();
        status = self;
        // This transport word retains a single compiler lifetime: hidden-state
        // integer 1 here, owned layout below, inventory ordinal at Setup, then
        // owned shot-counter. Changing to independent locals changes exception
        // range metadata in this pragma block. Only the ordinal use is scalar;
        // no code dereferences that value as a resource pointer.
        resource = (void*)1;
        AsCIngameStatus(status)->state = (int)resource;
        AsCIngameStatus(status)->bpEntryId = (int)missingEntry;
        (*(int*)&AsCIngameStatus(status)->bpObject) = (int)zeroValue;
        AsCIngameStatus(status)->animationState = (int)resource;
        AsCIngameStatus(status)->initialized = (int)zeroValue;
        (*(int*)&AsCIngameStatus(status)->textTable) = (int)zeroValue;
        (*(int*)&AsCIngameStatus(status)->layout) = (int)zeroValue;
        if (controller == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_803280FC);
                s_bpCtrlInitialized = (signed char)(size_t)resource;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        CBpCtrl_CreateEntry(Sing_805DC5F0(), (void*)ingameStatusStrings, (char*)&AsCIngameStatus(status)->bpEntryId, 0);
        controller = Sing_805DC5F0();
        if (controller == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_803280FC);
                s_bpCtrlInitialized = (signed char)1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        AsCIngameStatus(status)->bpObject = IdMap_Lookup(Sing_805DC5F0(), AsCIngameStatus(status)->bpEntryId);

        resource = new Obj8C(); // owned status layout; published before initialization
        AsCIngameStatus(status)->layout = resource;
        {
            void** vtable = ((StatusLayoutInterface*)resource)->methods;
            typedef void (*Init)(void*, int);
            ((Init)vtable[3])(resource, 0);
        }
        controller = Sing_805DC830();
        if (controller == 0) {
            if (s_pointerVectorInitialized == 0) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_fontNumDrawRegistryDtorRegistration_FontNumDraw);
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        AsCIngameStatus(status)->textTable = CFontNumDraw_CreateInVector(Sing_805DC830());
        void* player = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (player != 0) {
            unsigned char idx = AsCGolfDoc(player)->currentPlayer;
            void* characterId = ((StatusSharedPlayer*)g_sharedDoc)[idx].characterId;
            resource = ItemInventory_FindCharacterIndex(AsCSharedDoc(g_sharedDoc)->itemInventory, characterId); // inventory ordinal as ABI word
            unsigned mode = AsCSharedDoc(g_sharedDoc)->gameMode;
            int standardMode = 1;
            if (mode != 0 && mode != 6) standardMode = 0;
            golfDoc = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            alternateMode = (standardMode == 0) ? 1 : 0;
            unsigned char currentPlayerIndex = AsCGolfDoc(golfDoc)->currentPlayer;
            int type = SharedDoc_IsPlayerFlag0Set(currentPlayerIndex);
            void* obj = AsCIngameStatus(status)->layout;
            void** vt = ((StatusLayoutInterface*)obj)->methods;
            typedef void (*Setup)(void*, int, unsigned char, int, int);
            ((Setup)vt[6])(obj, (int)resource, currentPlayerIndex, type, alternateMode);
        }
        resource = new Obj24(); // owned shot counter, no further ordinal use
        AsCIngameStatus(status)->shotCountInfo = resource;
        (*(int*)&AsCIngameStatus(status)->savedPlayerFlags) = 0;
        return status;
    }
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset

    // Reset the HUD and animation state to visible.
    void CIngameStatus_ResetState(void* self) {
        AsCIngameStatus(self)->state = kStatusVisible;
        AsCIngameStatus(self)->animationState = kStatusVisible;
    }

    // Request the HUD show animation and enable its BpObject.
    void CIngameStatus_BeginShow(void* self) {
        AsCIngameStatus(self)->state = kStatusShowing;
        BpObject_SetEnabled(AsCIngameStatus(self)->bpObject, 1);
    }

    // Reset the shot-counter fade when the counter object exists.
    void CIngameStatus_ResetShotCounterFade(void* self) {
        void* shotCountInfo = AsCIngameStatus(self)->shotCountInfo;
        if (shotCountInfo == 0) return;
        CShotCountInfo_ResetFade(shotCountInfo);
    }

    // Test whether golf-state flags permit the HUD transition condition.
    int GolfState_AllowsStatusTransition() {
        void* sharedDoc = g_sharedDoc;
        int gameFlags = AsCGolfDoc((void*)AsCSharedDoc(sharedDoc)->golfDoc)->gameStateFlags;
        int remainingFlags = gameFlags & kStatusHideTargetMask;
        return remainingFlags == 0;
    }

    // Return whether the HUD is visible or running its show animation.
    int CIngameStatus_IsShownOrShowing(void* self) {
        int state = AsCIngameStatus(self)->state;
        // 0x80328514: states 1 and 3 share the hidden result.
        if (state == kStatusHidden || state == kStatusHiding)
        return 0;
        return 1;
    }

    // Advance ranking layout and packed-texture loading.
    void ViewRank_UpdateLayoutAndPollTexture(void* self) {
        void* rankView = self;
        Layout_Update(AsViewRank(self)->layout);
        void* textureResource = AsViewRank(rankView)->textureResource;
        if (textureResource != 0) {
            CPackTextureResource_PollLoad(textureResource);
        }
    }

    // Refresh the status layout with the current character, player, and game-mode flags.
    // initialized==0 or null golfDoc is a no-op. Otherwise require live inventory,
    // shared-player storage for currentPlayer 0..3 and a CIngameStatus2d layout
    // created by CIngameStatus_Ctor's Obj8C. Its +0x18 vtable slot (index 6 with
    // the two-word header) is CIngameStatus2d_SetIconAndLabelElements (create_new_account_and_ingame_status.cpp); the local
    // virtual signature transports scalar words, NOT a character object pointer.
    // Arguments: character inventory ordinal matching s_ingameStatusIconIds[8]
    // (permutation 6,3,2,1,7,0,5,4); currentPlayer ordinal 0..3; flag0 Boolean;
    // alternateMode Boolean (gameMode other than 0/6). FindCharacterIndex can
    // return -1: unmatched IDs leave the previous iconElement unchanged.
    // Receiver writes iconElement to matching table SLOT 0..7, then labelElement
    // to player+8 in standard mode, player+13 for alternate with flag0, otherwise
    // 12. All fit the layout's 21 elements (0..20). No pointers retained, resource
    // acquisition, drawing or status return; updates only these two selections.
    void CIngameStatus_RefreshPlayer(void* self) {
        // setup(character inventory index, player ordinal, flag0, alternate-mode flag).
        // Inputs are borrowed; setup does not receive ownership of the player record.
        void* status = self;
        int initialized = AsCIngameStatus(self)->initialized;
        if (initialized == 0) return;
        void* sharedDoc = g_sharedDoc;
        void* golfDoc = (*(void**)&AsCSharedDoc(sharedDoc)->golfDoc);
        if (golfDoc == 0) return;
        unsigned char playerIndex = AsCGolfDoc(golfDoc)->currentPlayer;
        void* inventory = AsCSharedDoc(sharedDoc)->itemInventory;
        // Shared player blocks are 0x1D4 bytes; characterId lies +0x120 within each.
        void* playerStorage = (char*)sharedDoc + (playerIndex * kStatusPlayerRecordStride);
        void* characterId = ((StatusSharedPlayer*)playerStorage)->characterId;
        void* characterIndex = ItemInventory_FindCharacterIndex(inventory, characterId);
        void* sharedDocAfterLookup = g_sharedDoc;
        int standardMode = 1;
        // gameMode is an integer ID carried through a legacy pointer-shaped view.
        // IDs 0 and 6 both request the standard layout; every other ID requests alternate.
        void* gameMode = (*(void**)&AsCSharedDoc(sharedDocAfterLookup)->gameMode);
        if (gameMode != 0 && gameMode != (void*)6) {
            standardMode = 0;
        }
        void* golfDocAfterLookup = (*(void**)&AsCSharedDoc(sharedDocAfterLookup)->golfDoc);
        int partyMode = (standardMode == 0) ? 1 : 0;
        void* character = characterIndex;
        unsigned char currentPlayer = AsCGolfDoc(golfDocAfterLookup)->currentPlayer;
        int playerFlag = SharedDoc_IsPlayerFlag0Set(currentPlayer);
        struct Receiver {
            char pad[0x88];
            virtual void slot0() DECOMP_LOCAL_VIRTUAL;
            virtual void slot1() DECOMP_LOCAL_VIRTUAL;
            virtual void slot2() DECOMP_LOCAL_VIRTUAL;
            virtual void slot3() DECOMP_LOCAL_VIRTUAL;
            virtual void setup(void*, unsigned char, int, int) DECOMP_LOCAL_VIRTUAL;
        };
        Receiver* obj = (*(Receiver**)&AsCIngameStatus(status)->layout);
        obj->setup(character, AsCGolfDoc(golfDocAfterLookup)->currentPlayer, playerFlag, partyMode);
    }

    // Report ranking readiness: 1 iff every present pack texture, all shared BP
    // entries, required layout, and required shared-file buffer are loaded; else 0.
    // Required constructed self; textureResource is optional and skipped if null.
    // This is an observation hook, not overall activation: vfunc_3 consumes ready
    // resources. It still polls ALL four gates after an earlier false result.
    // Side effect: lazily constructs/publishes the process-lifetime BP controller
    // and registers its destructor; PollAllEntriesLoaded may advance pending BP
    // loads. No self field is written and no output reference is retained/released.
    // Readiness of other objects' BP entries in that shared registry also matters.
    int ViewRank_PollLoadComplete(void* self) {
        int allLoaded = 1;
        void* rankView = self;
        void* textureResource = AsViewRank(self)->textureResource;
        if (textureResource != 0) {
            int textureReady = CPackTextureResource_IsLoadFinished(textureResource);
            if (textureReady == 0) allLoaded = 0;
        }
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_Menu);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        if (CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()) == 0) allLoaded = 0;
        if (Layout_PollLoadComplete(AsViewRank(rankView)->layout) == 0) allLoaded = 0;
        if (CSharedFilePtr_GetLoadedBuffer(AsViewRank(rankView)->sharedFile) == 0) allLoaded = 0;
        return allLoaded;
    }

    // Release status UI objects and unregister its BpObject and number renderer.
    void* CIngameStatus_Dtor(void* self, int flag) {
        struct VirtualObject {
            virtual ~VirtualObject() DECOMP_LOCAL_VIRTUAL;
        };
        struct Receiver {
            char pad[0x88];
            virtual ~Receiver() DECOMP_LOCAL_VIRTUAL;
        };
        void* status = self;
        int deleteFlag = flag;
        if (self != 0) {
            VirtualObject* shotCountInfo = (*(VirtualObject**)&AsCIngameStatus(self)->shotCountInfo);
            if (shotCountInfo != 0) {
                delete shotCountInfo;
                AsCIngameStatus(status)->shotCountInfo = 0;
            }
            if (AsCIngameStatus(status)->bpEntryId != -1) {
                if (Sing_805DC5F0() == 0) {
                    if (s_bpCtrlInitialized == 0) {
                        __ct__7CBpCtrlFv(s_bpCtrl);
                        __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_803280FC);
                        s_bpCtrlInitialized = 1;
                    }
                    Sing_805DC5F0() = s_bpCtrl;
                }
                CBpCtrl_DeleteEntryById(Sing_805DC5F0(), AsCIngameStatus(status)->bpEntryId);
                AsCIngameStatus(status)->bpEntryId = -1;
            }
            Receiver* statusLayout = (*(Receiver**)&AsCIngameStatus(status)->layout);
            if (statusLayout != 0) {
                delete statusLayout;
                AsCIngameStatus(status)->layout = 0;
            }
            if (AsCIngameStatus(status)->textTable != 0) {
                UITextTable_ClearEntries(AsCIngameStatus(status)->textTable);
                if (Sing_805DC830() == 0) {
                    if (s_pointerVectorInitialized == 0) {
                        PointerVector_InitReserved(s_fontNumDrawRegistry);
                        __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, s_fontNumDrawRegistryDtorRegistration_FontNumDraw);
                        s_pointerVectorInitialized = 1;
                    }
                    Sing_805DC830() = s_fontNumDrawRegistry;
                }
                PointerArray_DeleteValue(Sing_805DC830(), AsCIngameStatus(status)->textTable);
            }
            if (deleteFlag > 0) {
                __dl__FPv(status);
            }
        }
        return status;
    }

    // Activate Results ranking messages/numbers and its menu script; preserve initialization before publication.
    // self requires ready message/layout resources (vfunc_2); borrowed file bytes
    // create retained tagged-text data. Reuses renderers if already present,
    // registers numbers only while dirty, then recomputes ranks/scores and clears
    // selectionResult. There is no load-failure rollback in this activation hook.
    void ViewRank_Activate(void* self) {
        struct VirtualObject {
            virtual ~VirtualObject() DECOMP_LOCAL_VIRTUAL;
        };
        struct Object4BC {
            char bytes[0x4bc];
            // Construct the ranking message renderer from the borrowed text factory.
            Object4BC(void* arg) {
                CMsgDraw_Ctor(this, arg);
            }
        };
        struct Fields {
            char pad[0x24];
            int taggedText;
        };
        struct GlobalPair {
            int a, b;
            // Mark the shared menu-script handles absent before registration.
            void init() {
                b = -1;
                a = -1;
            }
        };
        Fields* rankView = (Fields*)self;
        VirtualObject* textureResource = (*(VirtualObject**)&AsViewRank(self)->textureResource);
        if (textureResource != 0) {
            delete textureResource;
            AsViewRank(rankView)->textureResource = 0;
        }
        int taggedText = rankView->taggedText;
        void* g = g_chatMsg;
        void* fixedFont = ((ChatFontFields*)g)->fixedFont;
        if (taggedText == 0) {
            void* sharedFile = AsViewRank(rankView)->sharedFile;
            int messageResource = CSharedFilePtr_GetLoadedBuffer(sharedFile);
            WFixedFont_CreateMsgEcbResource(fixedFont, &rankView->taggedText, (void*)messageResource, 1);
        }
        if ((*(int*)&AsViewRank(rankView)->messageDraw) == 0) {
            void* messageDraw = new Object4BC(AsWFixedFont(fixedFont)->messageResource);
            AsViewRank(rankView)->messageDraw = messageDraw;
        }
        ViewRank_StartRankingAnimation(rankView);
        if (AsViewRank(rankView)->numberEntriesDirty != 0) {
            if (Sing_805DC5F0() == 0) {
                if (s_bpCtrlInitialized == 0) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_Menu);
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            void* bpObject = IdMap_Lookup(Sing_805DC5F0(), AsViewRank(rankView)->entryId);
            void* numberGroups = bpObject;
            CFontNumDraw_RegisterEntries(AsViewRank(rankView)->numberDraw, s_rankingNumberEntries, kRankingNumberEntryCount, numberGroups, (char*)rankingViewStrings + kRankTextNumberFont);
            AsViewRank(rankView)->numberEntriesDirty = 0;
        }
        ViewRank_ComputeCharacterRanks(rankView);
        ViewRank_RefreshPlayerScores(rankView);
        if (Sing_805DC5F8() == 0) {
            if (s_bpCtrlHandleInitialized == 0) {
                ((GlobalPair*)&s_bpCtrlHandle)->init();
                __register_global_object((void*)&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_Ranking);
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = (void*)&s_bpCtrlHandle;
        }
        CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kRankingMenuScriptIndex);
        AsViewRank(rankView)->selectionResult = 0;
    }

}   // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462620", _ctors_u803280FC_0, StatusUi_InitAngleConstantA);
STATIC_INIT_ENTRY(".CRT$XCU80462620", _ctors_u803280FC_1, StatusUi_InitAngleConstantB);
STATIC_INIT_ENTRY(".CRT$XCU80462620", _ctors_u803280FC_2, ViewRank_InitAngleConstant);
#pragma pop

// Verify the partial views against the offsets used by this unit.
typedef char PartyPlayerFields_score_offset[((int)&((PartyPlayerFields*)0)->score == 0x178) ? 1 : -1];
typedef char PartyPlayerFields_holePoints_offset[((int)&((PartyPlayerFields*)0)->holePoints == 0x17c) ? 1 : -1];
typedef char PartyPlayerFields_totalPoints_offset[((int)&((PartyPlayerFields*)0)->totalPoints == 0x1c4) ? 1 : -1];
typedef char RankGlobalFields_messagePath_offset[((int)&((RankGlobalFields*)0)->messagePath == 0x40) ? 1 : -1];
typedef char StatusLayoutInterface_methods_offset[((int)&((StatusLayoutInterface*)0)->methods == 0x88) ? 1 : -1];
typedef char StatusSharedPlayer_characterId_offset[((int)&((StatusSharedPlayer*)0)->characterId == 0x120) ? 1 : -1];
