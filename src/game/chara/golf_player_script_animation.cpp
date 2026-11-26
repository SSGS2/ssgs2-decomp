// Golf player script commands: playing named animations, showing and hiding part layers,
// interpolation, positional sounds, club attachment, and power-shot preparation.
// Also parsing of conditioned and bone-reference commands, and destruction of pending player data.

#include "classes/GolfDocSnapshot8013.h"
#include "classes/GolfPlayerAnimationState.h"
#define AsGolfPlayerAnimationState(p) ((GolfPlayerAnimationState*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/wvector.h"
#include "classes/CGolfPlayer.h"

#define AsCGolfPlayer(p) ((CGolfPlayer*)(p))

extern "C" {

enum PlayerScriptConstants {
    kNoAnimationMode = 0,
    kPreparationAnimationMode = 2,
    kPowerShotAnimationMode = 3,
    kClubHiddenFlag = 0x80,
    kUseLockedAttachmentFlag = 0x100,
    kScriptTokenCapacity = 0x40,
    kMaximumPartTokens = 0x40,
    kScriptCloseArguments = ')',
    kScriptAnimationNamePrefix = '@',
    kShotResultImmediateFlag = 0x40,
    kShotResultBallRecordFlag = 0x20000000,
    // Combined mask tested against CGolfDoc::gameStateFlags (+0x358): bits 0x8, 0x100, 0x200 and 0x40000.
    // Power-shot preparation does nothing unless one is set; the meaning of each bit is not established.
    kPowerShotEligibleStateMask = 0x40308,
    kSinglePowerShotMode = 1,
    kDoublePowerShotMode = 2,
    kDoublePowerUnavailableNotice = 8,
    kInsufficientPowerMeterNotice = 4,
    kRumbleDuration = 10,
    kWoodClub = 0,
    kIronClub = 1,
    kPutterClub = 3
};

// Byte-address form prevents MWCC from hoisting this address across animation callbacks.
enum {
    kPlayerAnimationNameOffset = (int)&((CGolfPlayer*)0)->shotAnimationPrefix
};
// First byte of CGolfPlayer::shotAnimationPrefix (+0x65); nonzero means a prefix string is set,
// which is what allows power-shot preparation to proceed below.
struct ShotAnimationPrefixView {
    char pad[0x65];
    signed char firstChar;
};

// The model manager's internals are opaque; array indexing uses its verified ABI size.
// Shared player records start after CSharedDoc's 0x10-byte prefix and have a 0x1D4 stride.
// Only the fields used by this unit are asserted here.
struct SharedPlayerSlotView {
    char pad_000[0x1BC];
    void* clubSetId;
    char pad_1C0[0xC];
    int swingModelVariant;
    char pad_1D0[4];
};
struct SharedPlayerArrayView {
    char pad_000[0x10];
    SharedPlayerSlotView players[4];
};
struct AttachmentCalibration {
    float ballDistance[3];
    float gripOffset;
};
struct PlayerAttachmentTablesView {
    char pad_000[0x128];
    AttachmentCalibration swing[4][4];
    AttachmentCalibration club[4];
};
typedef char SharedPlayerSlotView_size[(sizeof(SharedPlayerSlotView) == 0x1D4) ? 1 : -1];

enum {
    kPlayerModelSlotSize = 0x2A0,
    kAnimationStateStride = 0xB4
};
struct PlayerModelSlot {
    char storage[kPlayerModelSlotSize];
};
// Locate a player's model-manager slot using its verified array stride.
inline void* playerModelAt(void* player, int playerIndex) {
    return (PlayerModelSlot*)AsCGolfPlayer(player)->playerModels + playerIndex;
}
// Locate a player's animation state using the shared typed layout.
inline GolfPlayerAnimationState* playerAnimationAt(void* player, int playerIndex) {
    return (GolfPlayerAnimationState*)AsCGolfPlayer(player)->animationStates + playerIndex;
}
// InstanceTable_Get exposes the selected club kind and the current power-shot mode.
struct SelectedClubView {
    char pad_000[0xA];
    unsigned char kind;
};
struct PlayerInstanceView {
    char pad_000[4];
    SelectedClubView* selectedClub;
    char pad_008[0x3C];
    int powerShotMode;
};

void* PlayerModel_GetPuppet(void*);
void* ObjectList_GetSelected(void*);
void WPuppet_ParseMaterialLayerAnimationCommand(void*, void*, void*);
void* InstanceTable_Get(void*, void*);
void* ClubModel_FindCachedForItem(void*, void*);
void* WPuppet_FindNamedBoneEntry(void*, void*);
void* String_ParseQuotedToken(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void Sound_PlayByName(void*, void*, void*, int, int, void*, float, float, float);
void Matrix_CopyAffine(void*, void*);
void* Matrix_TransformPoint(void*, void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void __ml__FfRC7WVector(void*, void*, float);
void __apl__7WVectorFRC7WVector(void*, void*);
void PlayerModel_ConfigureNamedPuppetLayer(void*, int, int, void*);
void* strchr(const char*, int);
void strncpy(void*, const char*, unsigned int);
void strcat(char*, const char*);
int ResourceModel_HasPartModelNamed(void*, const char*);
void CGolfPlayer_SetPlayerAnimation(void*, const char*, float, int);
int CString_Compare(const char*, const char*);
int sscanf(const char*, const char*, ...);
char* strcpy(char*, const char*);
unsigned int strlen(const char*);
int ParseConditionedQuotedToken(void*, void*, void*, void*, void*);
void FxAttachment_ParseBoneReferences(void*, void*, int, void*, void*, int, void*);
void* GolfPlayer_GetEntryByIndex(void*, int);
void Sound_StopByAlias(void*, const char*);
char* String_FormatTemporary(const char*, ...);
void NoticeQueue_ShowById(void*, int, int);
void IActor_DispatchMessage(void*, void*, const char*, int, float*, int, int, int, int);
void IntStringTree_DestroySubtree(void*, void*);
void appFreeUni__FPv(void*);
void __dt__18NativeBase80201D18Fv(void*, int);
void __dl__FPv(void*);
extern CSharedDoc* g_sharedDoc;
extern char s_cameraTransform[];
extern unsigned int kFloat_100_GolfSound;
extern unsigned int kGolfPlayerSoundOptionWord1;
extern void* g_soundManager;
extern float kFloat_0_5_GolfPlayer;
extern float kFloat_200_GolfSound;
extern float kFloat_1_GolfPlayer;
extern const float kFloat_0_GolfPlayer;
extern float kAnimationFramesPerSecond_GolfPlayer; // 30 frames per second for animation interpolation
extern float kSinglePowerShotMeterRequired;        // 33: meter required for single power-shot preparation
extern float kDoublePowerShotMeterRequired;        // 66: meter required for double power-shot preparation
extern void* g_helpMsg;
extern signed char kInitialPartCommandToken[];
extern char golfPlayerStrings[];
// Byte offsets in the shared Shift-JIS string pool.
enum GolfPlayerTextOffset {
    kPlayerTextClubBone = 0x12F,
    kPlayerTextScreenActor = 0x143,
    kPlayerTextPowerShotReadySound = 0x14A,
    kPlayerTextSpotActor = 0x16B,
    kPlayerTextReadyAnimationFormat = 0x177,
    kPlayerTextPartModelSuffix = 0x5E0,
    kPlayerTextInterpolationFormat = 0x5E6,
    kPlayerTextUnplayableSound = 0x5EF,
    kPlayerTextDoublePowerReadyAnimationFormat = 0x5FF,
    kPlayerTextPowerReadyAnimationFormat = 0x60D,
};

// SetPlayerAnimation call sites here also supply an extra float in f1; its units are unresolved.
// Play the named player animation using the recovered four-argument call ABI.
inline void PlayPlayerScriptAnimation(void* player, const char* name, int index, float value) {
    ((void (*)(void*, const char*, float, int))CGolfPlayer_SetPlayerAnimation)(player, name, value, index);
}
// Borrow the current instance table through its no-argument accessor ABI.
inline void* CurrentPlayerInstances() {
    return ((void* (*)())InstanceTable_Get)();
}

// Call-site ABI adapters: keep argument evaluation at the actual call.
typedef void (*PlayerUiSoundCall)(void*, const char*, int, int, float, float, float, void*, void*);
#define PlayerUiSoundTarget Sound_PlayByName
typedef void (*PlayerActorMessageCall)(void*, void*, const char*, int, float*, int, int, int);

#pragma push
#pragma use_lmw_stmw on
#pragma scheduling on
struct Token64 {
    signed char b[0x40];
};

// Part-token contract for both mode handlers: commandCursor borrows mutable
// cursor storage and is advanced after each token. playerIndex selects the
// 0x2A0-byte model slot. At most 64 tokens; each must be 1..63 bytes plus
// NUL. The parser receives no destination size: scripts must respect it.
// Modes 0/1 are passed to ConfigureNamedPuppetLayer, not visibility booleans;
// their visual distinction is not established by these callers.
// Parse up to 64 part-model tokens and apply mode zero to each named part.
void CGolfPlayer::ShowNamedPartLayers(int* commandCursor, int playerIndex) {
    char token[kScriptTokenCapacity];
    *(Token64*)token = *(Token64*)kInitialPartCommandToken;

    int modelOffset = playerIndex * kPlayerModelSlotSize;
    int tokenCount = 0;
    do {
        *commandCursor = (int)String_ParseQuotedToken((void*)*commandCursor, token);
        if (strlen(token) != 0 && strlen(token) < kScriptTokenCapacity) {
            PlayerModel_ConfigureNamedPuppetLayer((char*)playerModels + modelOffset, 0, 0, token);
            if (*commandCursor == 0 || **(signed char**)commandCursor == kScriptCloseArguments)
                break;
            if (++tokenCount >= kMaximumPartTokens)
                break;
        } else {
            break;
        }
    } while (1);
}

#pragma pop
// Play an animation when any semicolon-separated candidate part model exists for the player.
void CGolfPlayer::PlayAnimationIfPartPresent(void* player, int* commandCursor, int playerIndex) {
    char partCandidates[0x400];
    char animationName[0x100];
    partCandidates[0] = 0;
    animationName[0] = 0;
    *commandCursor = (int)String_ParseQuotedToken((void*)*commandCursor, partCandidates);
    *commandCursor = (int)String_ParseQuotedToken((void*)*commandCursor, animationName);
    if (!*partCandidates || !*animationName)
        return;
    {
        int candidateOffset = 0;
        char partModelName[0x28];
        for (;;) {
            __typeof__(partCandidates + candidateOffset) candidateBegin = partCandidates + candidateOffset;
            char* separator = (char*)strchr(candidateBegin, ';');
            unsigned int candidateLength;
            if (separator)
                candidateLength = (unsigned int)(separator - partCandidates - candidateOffset);
            else {
                candidateLength = strlen(partCandidates + candidateOffset);
                if (!candidateLength)
                    break;
            }
            strncpy(partModelName, partCandidates + candidateOffset, candidateLength);
            partModelName[candidateLength] = 0;
            strcat(partModelName, golfPlayerStrings + kPlayerTextPartModelSuffix);
            if (ResourceModel_HasPartModelNamed(playerModelAt(player, playerIndex), partModelName)) {
                PlayPlayerScriptAnimation(player, animationName, playerIndex, kFloat_0_GolfPlayer);
                playerAnimationAt(player, playerIndex)->pendingAnimationName = 0;
                break;
            }
            if (!separator)
                break;
            candidateOffset += candidateLength + 1;
        }
    }
}

struct NamedAnimationView {
    char pad_000[0x18];
    const char* name;
};

// Parse start/end/duration values, optionally gated by animation name, into an interpolation rate per second.
// Borrowed script cursor is advanced by String_ParseQuotedToken. The first token
// is either the numeric payload or @animationName followed by that payload; a
// name mismatch returns before consuming the payload. The pooled sscanf format
// is "%f %f %f": whitespace-separated start, end, duration in 30-Hz frames.
// Values themselves have caller-defined units. Store start and (end-start)/seconds;
// this parser does not advance the value. Application timing is outside this unit.
// sscanf return is unchecked: start/end must parse; omitted duration stays zero
// and produces a zero rate. Malformed start/end has no defined fallback.
void CGolfPlayer::ParseInterpolationCommand(void* player, int* commandCursor, int playerIndex) {
    float startValue;
    char text[kScriptTokenCapacity];

    text[0] = 0;
    *commandCursor = (int)String_ParseQuotedToken((void*)*commandCursor, text);
    if (!strlen(text) || strlen(text) >= kScriptTokenCapacity)
        return;
    char value[kScriptTokenCapacity];
    value[0] = 0;
    if (text[0] == kScriptAnimationNamePrefix) {
        void* animationState = playerAnimationAt(player, playerIndex);
        void* object = AsGolfPlayerAnimationState(animationState)->animationObject;
        if (object == 0 || CString_Compare(text + 1, ((NamedAnimationView*)object)->name))
            return;
        *commandCursor = (int)String_ParseQuotedToken((void*)*commandCursor, value);
        if (!strlen(value) || strlen(value) >= kScriptTokenCapacity)
            return;
    } else
        strcpy(value, text);
    if (!strlen(value) || strlen(value) >= kScriptTokenCapacity)
        return;

    float endValue;
    float durationFramesOrSeconds = kFloat_0_GolfPlayer;
    sscanf(value, golfPlayerStrings + kPlayerTextInterpolationFormat, &startValue, &endValue, &durationFramesOrSeconds);
    float durationValue = durationFramesOrSeconds;
    float initialValue = startValue;
    playerAnimationAt(player, playerIndex)->interpolationStart = initialValue;
    durationFramesOrSeconds = durationValue / kAnimationFramesPerSecond_GolfPlayer;
    if (durationFramesOrSeconds > kFloat_0_GolfPlayer) {
        float durationSeconds = durationFramesOrSeconds;
        playerAnimationAt(player, playerIndex)->interpolationRatePerSecond = (endValue - startValue) / durationSeconds;
    } else {
        playerAnimationAt(player, playerIndex)->interpolationRatePerSecond = kFloat_0_GolfPlayer;
    }
}

struct PlayerSoundState {
    char pad_000[0x40];
    WVector position;
    char pad_04C[0x18];
};
typedef char PlayerSoundState_size[(sizeof(PlayerSoundState) == 0x64) ? 1 : -1];

// Parse and play a positional sound at the selected/current player's position.
void CGolfPlayer::PlaySoundCommand(int* commandCursor, int playerIndex) {
    char soundName[kScriptTokenCapacity];
    *commandCursor = (int)String_ParseQuotedToken((void*)*commandCursor, soundName);
    if (strlen(soundName) == 0)
        return;
    if (strlen(soundName) >= kScriptTokenCapacity)
        return;
    if (playerIndex < 0)
        playerIndex = AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer; // +0x3C1
    PlayerSoundState* playerState = (PlayerSoundState*)playerStates + playerIndex;
    char cameraDisplacement[0x10];
    __mi__FRC7WVectorRC7WVector(cameraDisplacement, &playerState->position, s_cameraTransform + 0x24);
    void* soundOptions[2] = {(void*)kFloat_100_GolfSound, (void*)kGolfPlayerSoundOptionWord1};
    Sound_PlayByName(g_soundManager, soundName, &playerState->position, 0, 0, soundOptions, kFloat_0_5_GolfPlayer,
                     kFloat_200_GolfSound, kFloat_1_GolfPlayer);
}

#pragma push
#pragma scheduling on
// Enter or leave power-shot preparation using the player meter, updating animation, sound, and help notices.
// In plain C:
//   if (!(gameStateFlags & kPowerShotEligibleStateMask)) return;
//   if (double mode requested but disabled for this player) { play "unplayable" sound + notice 8; return; }
//   if (meter >= threshold for the requested mode (33 / 66) || document override) {
//       if (no shot-animation prefix) return;
//       if (already in power-shot mode) cancel it (stop sound, reset animation, clear mode, notify screen actor)
//       else start it (play ready sound, set mode, play the ready animation for single/double mode);
//       return;
//   }
//   insufficient meter: cancel an active preparation (also notifying the spot actor), else show notice 4.
// The assignments inside the big condition (player1, currentGolfDoc, player2) are the per-mode meter lookups.
void CGolfPlayer::UpdatePowerShotPreparation(void* player) {
    void* self = player;
    void* golfDoc = (void*)g_sharedDoc->golfDoc;                                   // +0xB90
    unsigned int gameFlags = *(unsigned int*)&AsCGolfDoc(golfDoc)->gameStateFlags; // +0x358
    if ((gameFlags & kPowerShotEligibleStateMask) == 0)
        return;
    // Mode 1 requires 33 meter points; mode 2 requires 66. The restriction byte requests notice 8.
    if (AsCGolfPlayer(self)->doublePowerShotDisabled == 1 &&
        AsCGolfPlayer(self)->requestedPowerShotMode == kDoublePowerShotMode) {
        if ((unsigned char)(g_helpMsg != 0) != 1)
            return;
        ((PlayerUiSoundCall)PlayerUiSoundTarget)(g_soundManager, golfPlayerStrings + kPlayerTextUnplayableSound, 0, 0,
                                                 kFloat_0_5_GolfPlayer, kFloat_200_GolfSound, kFloat_1_GolfPlayer,
                                                 (void*)0, (void*)0);
        NoticeQueue_ShowById(g_helpMsg, kDoublePowerUnavailableNotice, 0);
        return;
    }
    // Phase 2: short-circuit meter queries in mode order, then check the override.
    // Comma expressions retain reloads/call timing; singleton/animation callbacks
    // must not be cached across these branches. Function-pointer casts adapt opaque
    // declarations to the argument registers used by these particular call sites.
    void* player1;
    void* currentGolfDoc;
    void* player2;
    if ((AsCGolfPlayer(self)->requestedPowerShotMode == kSinglePowerShotMode &&
         (player1 = GolfPlayer_GetEntryByIndex(golfDoc, AsCGolfDoc(golfDoc)->currentPlayer),
          ((GolfDocSnapshot8013*)player1)->powerShotMeter >= kSinglePowerShotMeterRequired)) ||
        (AsCGolfPlayer(self)->requestedPowerShotMode == kDoublePowerShotMode &&
         (currentGolfDoc = (void*)g_sharedDoc->golfDoc,
          player2 = GolfPlayer_GetEntryByIndex(currentGolfDoc, AsCGolfDoc(currentGolfDoc)->currentPlayer),
          ((GolfDocSnapshot8013*)player2)->powerShotMeter >= kDoublePowerShotMeterRequired)) ||
        AsCGolfDoc((void*)g_sharedDoc->golfDoc)->powerShotMeterOverride !=
            0) { // Document override bypasses the meter test.
        if (!((ShotAnimationPrefixView*)self)->firstChar)
            return;
        {
            void* animationStates = AsCGolfPlayer(self)->animationStates;
            int playerIndex = AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer;
            void* animationState = (char*)animationStates + playerIndex * kAnimationStateStride;
            // An eligible request toggles preparation off if already active, else on.
            if (AsGolfPlayerAnimationState(animationState)->animationMode ==
                kPowerShotAnimationMode) { // +0x60: sound/animation state
                Sound_StopByAlias(g_soundManager, golfPlayerStrings + kPlayerTextPowerShotReadySound);
                AsGolfPlayerAnimationState((char*)AsCGolfPlayer(self)->animationStates +
                                           AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer *
                                               kAnimationStateStride)
                    ->animationMode = kNoAnimationMode;
                char* text = String_FormatTemporary(golfPlayerStrings + kPlayerTextReadyAnimationFormat,
                                                    (char*)self + kPlayerAnimationNameOffset);
                PlayPlayerScriptAnimation(self, text, -1, kFloat_0_GolfPlayer);
                ((PlayerInstanceView*)CurrentPlayerInstances())->powerShotMode = 0;
                float zero = kFloat_0_GolfPlayer;
                ((PlayerActorMessageCall)IActor_DispatchMessage)(self, self, golfPlayerStrings + kPlayerTextScreenActor,
                                                                 9, (float*)&zero, 0, 0, 0);
                return;
            } else {
                ((PlayerUiSoundCall)PlayerUiSoundTarget)(
                    g_soundManager, golfPlayerStrings + kPlayerTextPowerShotReadySound, 0, 0, kFloat_0_5_GolfPlayer,
                    kFloat_200_GolfSound, kFloat_1_GolfPlayer, golfPlayerStrings + kPlayerTextPowerShotReadySound,
                    (void*)0);
                AsGolfPlayerAnimationState((char*)AsCGolfPlayer(self)->animationStates +
                                           AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer *
                                               kAnimationStateStride)
                    ->animationMode = kPowerShotAnimationMode;
                int powerShotMode = AsCGolfPlayer(self)->requestedPowerShotMode;
                void* instanceTable = CurrentPlayerInstances();
                ((PlayerInstanceView*)instanceTable)->powerShotMode = powerShotMode;
                instanceTable = CurrentPlayerInstances();
                int storedMode = ((PlayerInstanceView*)instanceTable)->powerShotMode;
                char* text;
                if (storedMode == kDoublePowerShotMode &&
                    !((PlayerInstanceView*)CurrentPlayerInstances())->selectedClub->kind)
                    text = String_FormatTemporary(golfPlayerStrings + kPlayerTextDoublePowerReadyAnimationFormat,
                                                  (char*)self + kPlayerAnimationNameOffset);
                else
                    text = String_FormatTemporary(golfPlayerStrings + kPlayerTextPowerReadyAnimationFormat,
                                                  (char*)self + kPlayerAnimationNameOffset);
                PlayPlayerScriptAnimation(self, text, -1, kFloat_0_GolfPlayer);
                return;
            }
        }
    }
    {
        // Phase 3: insufficient meter cancels an existing preparation; otherwise notify.
        void* instanceTable = CurrentPlayerInstances();
        if (((PlayerInstanceView*)instanceTable)->powerShotMode != 0) {
            Sound_StopByAlias(g_soundManager, golfPlayerStrings + kPlayerTextPowerShotReadySound);
            void* animationStates = AsCGolfPlayer(self)->animationStates;
            int playerIndex = AsCGolfDoc((void*)g_sharedDoc->golfDoc)->currentPlayer;
            void* animationState = (char*)animationStates + playerIndex * kAnimationStateStride;
            AsGolfPlayerAnimationState(animationState)->animationMode = kNoAnimationMode;
            char* text = String_FormatTemporary(golfPlayerStrings + kPlayerTextReadyAnimationFormat,
                                                (char*)self + kPlayerAnimationNameOffset);
            PlayPlayerScriptAnimation(self, text, -1, kFloat_0_GolfPlayer);
            ((PlayerInstanceView*)CurrentPlayerInstances())->powerShotMode = 0;
            float zero = kFloat_0_GolfPlayer;
            ((PlayerActorMessageCall)IActor_DispatchMessage)(self, self, golfPlayerStrings + kPlayerTextSpotActor, 5, 0,
                                                             0, 0, 0);
            ((PlayerActorMessageCall)IActor_DispatchMessage)(self, self, golfPlayerStrings + kPlayerTextScreenActor, 9,
                                                             (float*)&zero, 0, 0, 0);
            return;
        }
        if (g_helpMsg != 0) {
            ((PlayerUiSoundCall)PlayerUiSoundTarget)(g_soundManager, golfPlayerStrings + kPlayerTextUnplayableSound, 0,
                                                     0, kFloat_0_5_GolfPlayer, kFloat_200_GolfSound,
                                                     kFloat_1_GolfPlayer, (void*)0, (void*)0);
            NoticeQueue_ShowById(g_helpMsg, kInsufficientPowerMeterNotice, 0);
        }
        return;
    }
}

#pragma pop
// WPuppet_FindNamedBoneEntry returns a world/bind-pose pair.
struct ClubBoneWorldPose {
    char pad_000[0x78];
    float matrix[12];
};
struct ClubBoneBindPose {
    char pad_000[0x18];
    WVector position;
};
struct ClubBoneEntry {
    ClubBoneWorldPose* worldPose;
    ClubBoneBindPose* bindPose;
};

// Capture the club-bone axes/world position, add the grip offset, and select locked-axis rendering.
void CGolfPlayer::LockClubAttachment(void* command, int playerIndex) {
    if (!attachmentPositionLocked) {
        void* puppet = PlayerModel_GetPuppet(playerModelAt(this, playerIndex));
        void* clubBone = WPuppet_FindNamedBoneEntry(puppet, golfPlayerStrings + kPlayerTextClubBone);
        Matrix_CopyAffine((char*)&attachmentDirection, ((ClubBoneEntry*)clubBone)->worldPose->matrix);
        WVector bonePosition;
        Matrix_TransformPoint(&bonePosition, &((ClubBoneEntry*)clubBone)->bindPose->position,
                              ((ClubBoneEntry*)clubBone)->worldPose->matrix);
        (*(WVector*)&attachmentPosition) = bonePosition;

        int clubKind = ((PlayerInstanceView*)CurrentPlayerInstances())->selectedClub->kind;
        float gripOffset = ((PlayerAttachmentTablesView*)this)->swing[playerIndex][clubKind].gripOffset;
        WVector reachOffset = gripOffset * (*(WVector*)&attachmentDirection);
        (*(WVector*)&attachmentPosition) += reachOffset;
    }
    void* animationState = playerAnimationAt(this, playerIndex);
    // Render the club from the axes/position just captured above (not a dirty bit).
    AsGolfPlayerAnimationState(animationState)->flags |= kUseLockedAttachmentFlag;
}

// Parse up to 64 part-model tokens and apply mode one to each named part.
void CGolfPlayer::HideNamedPartLayers(int* commandCursor, int playerIndex) {
    char buffer[kScriptTokenCapacity] = {};
    int modelOffset = playerIndex * kPlayerModelSlotSize;
    int tokenCount = 0;
    for (;;) {
        *commandCursor = (int)String_ParseQuotedToken((void*)*commandCursor, buffer);
        if (strlen(buffer) != 0 && strlen(buffer) < kScriptTokenCapacity) {
            PlayerModel_ConfigureNamedPuppetLayer((char*)playerModels + modelOffset, 0, 1, buffer);
            if (*commandCursor == 0 || **(signed char**)commandCursor == kScriptCloseArguments)
                break;
            if (++tokenCount >= kMaximumPartTokens)
                break;
        } else
            break;
    }
}

// Apply the supplied command when the player has both a puppet and a selected model.
void CGolfPlayer::ParseMaterialLayerAnimationCommand(void* player, void* command, int playerIndex) {
    int offset = playerIndex * kPlayerModelSlotSize;
    void* object = player;
    char* entry = (char*)AsCGolfPlayer(object)->playerModels + offset;
    void* puppet = PlayerModel_GetPuppet(entry);
    entry = (char*)AsCGolfPlayer(object)->playerModels + offset;
    void* selectedModel = ObjectList_GetSelected(entry);
    if (puppet != 0 && selectedModel != 0)
        WPuppet_ParseMaterialLayerAnimationCommand(puppet, command, selectedModel);
}

// Parse a conditioned token against the player's model and resolved club resource.
int CGolfPlayer::ParseConditionedCommandToken(void* command, int playerIndex, void* condition) {
    void* instanceTable = InstanceTable_Get(this, command);
    SharedPlayerSlotView* info = &((SharedPlayerArrayView*)g_sharedDoc)->players[playerIndex];
    void* clubResource = ClubModel_FindCachedForItem(instanceTable, info->clubSetId);
    void* entry = playerModelAt(this, playerIndex);
    return ParseConditionedQuotedToken(&sub_vtbl, command, condition, entry, clubResource); // +0x18
}

// Forward an animation command with the player's puppet, animation object, and club resource.
void CGolfPlayer::ParseBoneReferenceCommand(void* command, int playerIndex) {
    void* instanceTable = InstanceTable_Get(this, command);
    SharedPlayerSlotView* info = &((SharedPlayerArrayView*)g_sharedDoc)->players[playerIndex];
    void* clubResource = ClubModel_FindCachedForItem(instanceTable, info->clubSetId);
    void* entry = playerModelAt(this, playerIndex);
    void* puppet = PlayerModel_GetPuppet(entry);
    void* animationState = playerAnimationAt(this, playerIndex);
    FxAttachment_ParseBoneReferences(&sub_vtbl, command, playerIndex, puppet,
                                     AsGolfPlayerAnimationState(animationState)->animationObject, 1,
                                     clubResource); // +0x18, animationState +4
}

enum {
    kPendingNodeKeyOffset = 0xC,
    kPendingNodeStringOffset = 0x10
};
struct DtorNode {
    DtorNode* first;
    DtorNode* second;
    char pad08[4];
    int playerIndex;
    unsigned int flags;
    char pad14[4];
    void* payload;
};

struct VecNode {
    unsigned int flags;
    char pad04[4];
    void* payload;
};

// Destroy heap-backed strings in the loaded-sound-name vector in reverse order.
inline void DestroyVecElements(CGolfPlayer* base) {
    int count = (*(int*)&AsCGolfPlayer(base)->loadedSoundNameCount);
    VecNode* data = (*(VecNode**)&AsCGolfPlayer(base)->loadedSoundNames);
    VecNode* end = data + count;
    (*(int*)&AsCGolfPlayer(base)->loadedSoundNameCount) -= count;
    while (count != 0) {
        --end;
        if (end && (end->flags >> 31))
            appFreeUni__FPv(end->payload);
        --count;
    }
}

// Each pending-model node owns a string and two child nodes.
// Preserve the original three-level expansion; deeper branches use shared tree cleanup.
// Release a pending-model node's owned string and node storage.
inline void destroyPendingNode(DtorNode* node, void* owner) {
    if ((char*)node + kPendingNodeKeyOffset && (char*)node + kPendingNodeStringOffset && ((node->flags >> 31) != 0))
        appFreeUni__FPv(node->payload);
    appFreeUni__FPv(node);
}
// Release both remaining pending-model subtrees, then the owning string/node.
inline void destroyPendingLeaf(DtorNode* node, void* owner) {
    if (node->first)
        IntStringTree_DestroySubtree(owner, node->first);
    if (node->second)
        IntStringTree_DestroySubtree(owner, node->second);
    if ((char*)node + kPendingNodeKeyOffset && (char*)node + kPendingNodeStringOffset && ((node->flags >> 31) != 0))
        appFreeUni__FPv(node->payload);
    appFreeUni__FPv(node);
}

#pragma push
#pragma use_lmw_stmw on
// Release pending model loads, loaded sound names, attached IDs, and the player's base object.
CGolfPlayer::~CGolfPlayer() {
    CGolfPlayer* base = this;
    void* owner = &base->pendingModelLoadCount;
    if (owner && owner && owner && owner && owner && owner) {
        DtorNode* root = (*(DtorNode**)&AsCGolfPlayer(base)->pendingModelLoadRoot);
        if (root) {
            DtorNode* leftSubtree = root->first;
            if (leftSubtree) {
                DtorNode* leftBranch = leftSubtree->first;
                if (leftBranch) {
                    DtorNode* leftLeaf = leftBranch->first;
                    if (leftLeaf)
                        destroyPendingLeaf(leftLeaf, &base->pendingModelLoadCount);
                    leftLeaf = leftBranch->second;
                    if (leftLeaf)
                        destroyPendingLeaf(leftLeaf, &base->pendingModelLoadCount);
                    destroyPendingNode(leftBranch, &base->pendingModelLoadCount);
                }
                DtorNode* rightBranch = leftSubtree->second;
                if (rightBranch) {
                    DtorNode* leftLeaf = rightBranch->first;
                    if (leftLeaf)
                        destroyPendingLeaf(leftLeaf, &base->pendingModelLoadCount);
                    DtorNode* rightLeaf = rightBranch->second;
                    if (rightLeaf)
                        destroyPendingLeaf(rightLeaf, &base->pendingModelLoadCount);
                    destroyPendingNode(rightBranch, &base->pendingModelLoadCount);
                }
                destroyPendingNode(leftSubtree, &base->pendingModelLoadCount);
            }
            DtorNode* rightSubtree = root->second;
            if (rightSubtree) {
                DtorNode* leftBranch = rightSubtree->first;
                if (leftBranch) {
                    DtorNode* leftLeaf = leftBranch->first;
                    if (leftLeaf)
                        destroyPendingLeaf(leftLeaf, &base->pendingModelLoadCount);
                    DtorNode* rightLeaf = leftBranch->second;
                    if (rightLeaf)
                        destroyPendingLeaf(rightLeaf, &base->pendingModelLoadCount);
                    destroyPendingNode(leftBranch, &base->pendingModelLoadCount);
                }
                DtorNode* rightBranch = rightSubtree->second;
                if (rightBranch) {
                    DtorNode* leftLeaf = rightBranch->first;
                    if (leftLeaf)
                        destroyPendingLeaf(leftLeaf, &base->pendingModelLoadCount);
                    DtorNode* rightLeaf = rightBranch->second;
                    if (rightLeaf)
                        destroyPendingLeaf(rightLeaf, &base->pendingModelLoadCount);
                    destroyPendingNode(rightBranch, &base->pendingModelLoadCount);
                }
                destroyPendingNode(rightSubtree, &base->pendingModelLoadCount);
            }
            destroyPendingNode(root, &base->pendingModelLoadCount);
        }
    }
    // Phase 2: free the sound-name vector, circular cleanup nodes, and attached
    // ID buffers. Each container owns its allocation; list nodes have no payload cleanup.
    void* sentinel;
    void** vector = (void**)(&base->loadedSoundNames);
    if (vector && vector && vector && vector && base->loadedSoundNames) {
        DestroyVecElements(base);
        appFreeUni__FPv(base->loadedSoundNames);
    }
    // Circular list cleanup is handled below with its sentinel links.
    void** ownedListCount = (void**)(&base->cleanupListCount);
    if (ownedListCount && ownedListCount && ownedListCount && ownedListCount) {
        void** current = (*(void***)&AsCGolfPlayer(base)->cleanupListNext);
        sentinel = &base->cleanupListPrev;
        if (current != sentinel) {
            void** first = *(void***)sentinel;
            *(void**)((char*)current[0] + 4) = first[1];
            *(void**)first[1] = current[0];
            while (current != sentinel) {
                void* item = current;
                current = (void**)current[1];
                appFreeUni__FPv(item);
                --*(int*)(&base->cleanupListCount);
            }
        }
    }
    void** sequenceStorage = (void**)(&base->attachedSequenceIds);
    if (sequenceStorage && sequenceStorage && sequenceStorage && sequenceStorage && sequenceStorage) {
        void* ids = (*(void**)&AsCGolfPlayer(base)->attachedSequenceIds);
        if (ids) {
            char* end = (char*)ids + AsCGolfPlayer(base)->attachedSequenceCount;
            AsCGolfPlayer(base)->attachedSequenceCount -= end - (char*)ids;
            appFreeUni__FPv(ids);
        }
    }
    void** sprayStorage = (void**)(&base->attachedSprayIds);
    if (sprayStorage && sprayStorage && sprayStorage && sprayStorage && sprayStorage) {
        void* ids = (*(void**)&AsCGolfPlayer(base)->attachedSprayIds);
        if (ids) {
            char* end = (char*)ids + AsCGolfPlayer(base)->attachedSprayCount;
            AsCGolfPlayer(base)->attachedSprayCount -= end - (char*)ids;
            appFreeUni__FPv(ids);
        }
    }
    __dt__18NativeBase80201D18Fv(this, 0);
}
#pragma pop

} // extern "C"
