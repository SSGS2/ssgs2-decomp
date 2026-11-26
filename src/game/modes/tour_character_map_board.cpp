// Tour character-map board: resources, model previews, navigation, scripted
// reveals, encounters/course setup, rewards, Change Character, shop and saves.
// CTourMain::ProcessItemMenu is the update entry; DrawItemMenu is the draw entry.
// Shared layouts are in classes/OpenItemMenu*.h, CCameraController.h and
// SavePlayerRecord.h. Scalar companion: the data section; model placement and
// its per-call movement scalars: tour_board_preview_controller.cpp/the data section.
// Modding lookup:
// - Widescreen/aspect: OpenItemMenuState_UpdateCameraOffsets reads 640x480 view,
//   1024x512 board and 300x200 tracking margins (kOpenItemCameraBound0..5 in the
//   data companion). Map translation is NEGATIVE camera offset in UpdateVisuals.
//   recognizes these call contexts. A camera-bound edit alone does not change
//   the engine projection, viewport/scissor or 2D resource layout.
// - Model/board placement: UpdateItemMenuVisuals forwards dt, map origin and
//   controller position to TourModelPreview_UpdatePlacementForward; DrawItemMenu
//   selects visibility. Preserve UI/world unprojection in tour_board_preview_controller.cpp.
// - Frame rate/script pacing: InitItemMap may execute the initial script and
//   preview update before normal updates. ProcessSelectedMapItem uses two
//   independent event clocks: linkFrames/itemFrames, strict >20/>8 delay tests,
//   alpha steps +6/-32 PER CALL; ProcessItemMenu separately uses an 8-tick fade.
//   Animation dt and script counters are not interchangeable clock domains.
// - Model/course loading: PollItemMapResources + OpenItemMenu_BuildEntries bind
//   resource tables; PrepareSelectedEncounter publishes players, course/hole/pin
//   arrays. Preserve RNG calls/pin count, index domains and async readiness gates.
// - Texture/layout edits: s_tourMapItemGroupNames [35][5], pooled archive paths,
//   32-pixel icon rows, 85-pixel link rows, 40-pixel portrait frames. Pixel format
//   belongs to loaded assets/decoder; this file never requests a texture format.
// Methods borrow self/context/save records; owned wrappers/windows are published
// as each allocation succeeds. There is no transactional rollback of the menu.
// Pointer/word views preserve the 32-bit Wii ABI, float bits and callback order.
extern "C" char s_bpCtrlDtorRegistration_8051D0EC[];
extern "C" char s_bpPlayerDtorRegistration_TourItemMenu[];
extern "C" char s_effectFadeDtorRegistration_TourItemMenu[];
extern "C" char s_pangyaEnemyDataDtorRegistration_TourItemMenu[];
#include "iff/IffStruct.h"
#include "classes/SavePlayerRecord.h"
#include "classes/CEnemyRenderRecord.h"
struct TourVectorFields {
    unsigned int data, size, capacity;
};
struct TourRenderContextFields {
    SavePlayerRecord players[4];
    TourVectorFields vectors[4];
    CEnemyRenderRecord records[4];
    int count;
};
struct TourEnemyPrefix {
    char reserved[0x18];
    int state;
    signed char levelIndex;
};
struct TourResultVectors {
    TourVectorFields first, second, third;
};
struct TourRuleNameFields {
    char reserved[0x8C1];
    char ruleName[1];
};
struct TourRuleNameEntry {
    void* name;
    unsigned int flags;
};
struct TourRuleNames {
    char reserved[0x764];
    TourRuleNameEntry rules[3];
};
// Shared-document hole setup consists of parallel pin, hole-number and course tables.
struct TourRoundHoleFields {
    char documentPrefix[0x818];
    char pinIndices[18];
    char holeNumbers[18];
    char tablePadding;
    char courseIds[18];
};
// Partial shared-document view for the byte copied from encounter record +0x43.
struct TourEncounterWeatherFields {
    char reserved[0x9C6];
    unsigned char selectedWeather;
};
enum TourRoundSetup {
    kTourHoleLimit = 18,
    kTourPinPositionCount = 3
};
typedef char TourRound_pin_offset[IFF_OFFSET(TourRoundHoleFields, pinIndices) == 0x818 ? 1 : -1];
typedef char TourRound_hole_offset[IFF_OFFSET(TourRoundHoleFields, holeNumbers) == 0x82A ? 1 : -1];
typedef char TourRound_course_offset[IFF_OFFSET(TourRoundHoleFields, courseIds) == 0x83D ? 1 : -1];

#include "classes/SavePlayerRecord.h"
#include "classes/CPangyaEnemyData.h"
#include "classes/CSharedFileHandleHolder.h"
#include "classes/CSharedFileHandleOwner.h"
#include "classes/CTourWindow.h"
#include "classes/CTourVSWindow.h"
#include "classes/CCoinOpenItemData.h"
#include "classes/CMapData.h"
#include "classes/CTourMain.h"
#include "classes/CFlashMemoryMenuSave.h"
#include "classes/CPackTextureResource.h"
#include "classes/CPackedFile.h"
#include "classes/CPangya2Shop.h"
#include "classes/CCharacterPreviewTable.h"
#include "classes/CBpPlayer.h"
#include "classes/CEffectFade.h"
#include "classes/CTourHintWindow.h"
#include "classes/CBpCtrl.h"
#include "classes/CFontNumDraw.h"
#include "classes/CBpCtrlHandle.h"
// Tour workflow contract:
// state is the current phase; nonzero pendingState is committed at the next
// ProcessItemMenu call. Direct state assignments intentionally fall through to
// the next case in the same call. Zero pendingState means no requested change.
// ProcessItemMenu returns 0 while running, 1 to enter an encounter, 2 to change
// map, -1 to leave the tour. Context.nextState is the phase to resume on return.
// mode is a map/resource-table index [0,11), not a TourMenuState or golf mode.
// selectedItem indexes a map's 35 persistent item flags. -1 is the absent-key
// sentinel; lookup helpers may INSERT a default entry and do not validate keys.
// Loaded map/coin wrappers borrow archive data; archive handles and dynamically
// created windows/previews belong to the menu. Singleton getters return aliases.
// Normal route: load 1..3 -> reveal 4..6 -> navigate 7 -> preview/event 8..12 ->
// encounter 13 -> return/events/rewards 14..21 -> navigation. Submenus use 22..33.
// Submenu admission/completion/ownership (all keep result 0 while waiting):
// 22: navigation requested character selection; wait for menu-owned tourWindow's
//     load, hide map layers, initialize selection -> 23 immediately.
// 23: tourWindow consumes input/delta until accepted; disable its registered UI,
//     release caption script entry -> queue 24. Window stays owned for reuse.
// 24: read context.saveRecord's character/caddie. Changed model requests preview
//     reload -> 25; unchanged -> 4. CameraController owns preview resource IDs.
// 25: poll preview; on ready reset animation/play voice, clear context.nextState
//     -> queue 4. No timeout; voice temporary strings are released before return.
// 26: navigation requested shop; request fade, delete prior shop, allocate a shop
//     with a copied save snapshot -> 27 immediately. Menu owns shop; its snapshot
//     is separate from the context's borrowed working record.
// 27: process shop until its initialized word +8 is nonzero; hide map -> queue 28.
// 28: process until complete; copy shop snapshot INTO context.saveRecord and request
//     fade -> queue 29. Completion includes the shop's own exit/cancel policy.
// 29: delete/null shop -> queue 32. No shop snapshot survives this state.
// 30: navigation requested description; allocate menu-owned messageResourceHolder,
//     show selected preview's hint and start fade -> 31 immediately.
// 31: wait for hint completion; clear global hint objects before deleting/nulling
//     holder -> queue 6. Global hint layout is borrowed, not owned by holder.
// 32: reached after shop; submit context.saveRecord to SaveData, delete old save
//     UI, create menu-owned flashMemorySaveMenu -> 33 immediately.
// 33: process until save UI completes; delete/null it, copy SaveData's current
//     record BACK into context.saveRecord -> queue 24 (refresh preview/equipment).
// These phases do not handle allocation failure: 26/27 and 30 immediately pass
// newly published pointers to consumers. Null/throw is not a cancel transition.
// Menu teardown releases outstanding children if abandoned; it does not roll back
// shop/save publication. Keep context/save record alive through every child phase.
// Per-state trigger -> next-state/result map. "Queue" commits next invocation;
// "now" enters an adjacent fall-through case in this invocation. Unmet triggers keep result 0.
enum TourMenuState {
    kTourInactive = 0,                 // No workflow action; remain idle (result 0).
    kTourLoadArchives = 1,             // Create archive wrappers -> queue WaitMapResources.
    kTourWaitMapResources = 2,         // Archive/UI creation ready -> queue WaitUiResources.
    kTourWaitUiResources = 3,          // All UI ready: initialize map -> queue StartFadeOut.
    kTourStartFadeOut = 4,             // Enable layers/start fade -> enter WaitFadeOut now.
    kTourWaitFadeOut = 5,              // Fade state != 3 -> queue context.nextState or EnableNavigation.
    kTourEnableNavigation = 6,         // Enable controls/script -> enter Navigate now.
    kTourNavigate = 7,                 // Input handler queues preview, shop, character, description or exit.
    kTourStartPreview = 8,             // Start action/voice -> enter WaitPreview now.
    kTourWaitPreview = 9,              // Action animation finished -> queue StartBeforeEvent.
    kTourStartBeforeEvent = 10,        // No before-event -> queue PrepareRewards; otherwise ProcessBeforeEvent.
    kTourProcessBeforeEvent = 11,      // Script +1 -> rewards; -1 -> navigation; +2 -> result 2/map change.
    kTourPrepareRewards = 12,          // Queue unclaimed rewards; encounter -> EnterMatch, else StartAfterEvent.
    kTourEnterMatch = 13,              // Publish link class; result 1/encounter; context resumes ProcessMatchReturn.
    kTourProcessMatchReturn = 14,      // Choose success/failure feedback -> queue WaitReturnAnimation.
    kTourWaitReturnAnimation = 15,     // Animation done -> after-event on success/party, else navigation.
    kTourStartAfterEvent = 16,         // No after-event -> completion; otherwise queue ProcessAfterEvent.
    kTourProcessAfterEvent = 17,       // Script +1 -> completion; -1 -> navigation; zero keeps waiting.
    kTourStartCompletionEvent = 18,    // Start completion effect 1 -> queue ProcessCompletionEvent.
    kTourProcessCompletionEvent = 19,  // Script +1 -> rewards; cancel/party-tier-zero -> navigation.
    kTourStartRewardDelivery = 20,     // Requests exist -> queue WaitRewardDelivery; otherwise navigation.
    kTourWaitRewardDelivery = 21,      // Delivery complete: clear processing/item-enabled -> navigation.
    kTourOpenCharacterSelect = 22,     // Window ready: hide layers/open selection -> SelectCharacter now.
    kTourSelectCharacter = 23,         // Selection accepted -> queue ApplyCharacterSelection.
    kTourApplyCharacterSelection = 24, // Model changed -> WaitCharacterPreview; unchanged -> StartFadeOut.
    kTourWaitCharacterPreview = 25,    // Preview ready: reset/voice -> queue StartFadeOut.
    kTourOpenShop = 26,                // Fade/create shop snapshot -> enter WaitShopReady now.
    kTourWaitShopReady = 27,           // Shop initialized -> hide layers and queue ProcessShop.
    kTourProcessShop = 28,             // Shop done: copy snapshot to save record/fade -> queue CloseShop.
    kTourCloseShop = 29,               // Delete shop -> queue OpenSave.
    kTourOpenDescription = 30,         // Create message holder/show hint -> enter ProcessDescription now.
    kTourProcessDescription = 31,      // Hint done: delete holder -> queue EnableNavigation.
    kTourOpenSave = 32,                // Submit working save record/create save UI -> ProcessSave now.
    kTourProcessSave = 33,             // Save UI done: copy current saved record -> ApplyCharacterSelection.
    kTourExit = 34,                    // Clear context resume state; result -1/leave tour.
};
// Layout IDs are local to the five-element navigation layout:
// 0 shop control, 1 character control, 2/3 copies of 0/1's rectangles, 4 portrait.
// The number BP group contains named left/right and shop/character icon groups;
// these names are a separate namespace from layout IDs. Numeric text entry 7 is
// the coin count. Portrait frames and source rectangles are 40 pixels wide.
enum TourNavigationElement {
    kTourShopControl = 0,
    kTourCharacterControl = 1,
    kTourShopPairedRegion = 2,
    kTourCharacterPairedRegion = 3,
    kTourCharacterPortrait = 4,
    kTourNavigationElementCount = 5,
    kTourCoinNumberEntry = 7
};
// Columns in s_tourMapItemGroupNames: base icon, companion visual, reward
// overlay, direction-3 connection, direction-1 connection. The compass meaning
// of neighbor indices 0..3 is intentionally left to map/camera data.
enum TourItemVisualColumn {
    kTourItemIcon = 0,
    kTourItemCompanion = 1,
    kTourItemRewardOverlay = 2,
    kTourDirection3Link = 3,
    kTourDirection1Link = 4
};
// Clock constants are update/script counts, not seconds. Larger delay postpones
// alpha changes; larger opacity
// step makes the fade faster after the delay. Completion still waits for effects.
enum TourMapLimits {
    kTourMapCount = 11,
    kTourItemsPerMap = 35,
    kTourVisualColumns = 5,
    kTourSelectionBytes = 0x4A,
    kTourLinkRevealDelayUpdates = 20,
    kTourItemFadeDelayUpdates = 8,
    kTourLinkOpacityStep = 6,
    kTourItemOpacityStep = 32
};
enum TourVisualGeometry {
    kTourIconRowShift = 5, // 32 texture pixels per icon kind.
    kTourConnectionRowPixels = 85,
    kTourScreenFadeUpdates = 8,
    kTourConnectionCodeRadix = 100, // low two decimal digits=item, quotient chooses column.
    kTourOpaqueAlpha = 255,
    kTourLinkRevealEvent = 3,
    kTourItemBurstEvent = 5,
    kTourItemBurstFinishedEvent = 4
};
// Hovered portrait is full scale and shifted right 3 layout pixels; resting is
// 0.85 scale, shifted right 6/down 2. Values affect only the navigation portrait.
static const float kTourPortraitHoveredScale = 1.0f;
static const float kTourPortraitFramePixels = 40.0f; // Atlas X stride, not screen width.
static const float kTourPortraitRestingScale = 0.85f;
static const float kTourPortraitHoveredOffsetX = 3.0f;
static const float kTourPortraitRestingOffsetX = 6.0f;
static const float kTourPortraitRestingOffsetY = 2.0f;
enum TourScriptResult {
    kTourScriptRunning = 0,
    kTourScriptContinue = 1,
    kTourScriptChangeMap = 2,
    kTourScriptCancel = -1
};
enum TourWindowPhase {
    kTourBeginVersus = 0,
    kTourProcessVersus = 1,
    kTourBeginHint = 2,
    kTourProcessHint = 3,
    kTourNoWindow = 4
};
// Map rewards use inventory indices plus counts, not IFF type IDs or Pang prices.
struct InventoryReward {
    short inventoryIndex;
    short count;
};
struct TourRewardFields {
    char reserved[0x44];
    signed char holeNumbers[18];
    short coinReward;
    unsigned int selectionFlags;
    InventoryReward setItems[4];
    InventoryReward parts[4];
};
struct CoinEquipmentFields {
    short coinThreshold;
    InventoryReward clubSet, ball, caddie;
    InventoryReward caddieItems[3];
    InventoryReward setItems[8];
};
// Persistent per-map record: byte flags, pad, signed selected item, then
// 35 unsigned-short item flags; 11 records occupy 0x32E bytes. History stores
// map indices and the coin counter is separate from the save record's Pang.
struct TourSelectionFields {
    unsigned char flags;
    char reserved;
    short selectedItem;
    unsigned short items[35];
};
struct TourProgressFields {
    TourSelectionFields maps[11];
    unsigned char mapHistory[8];
    signed char historyIndex;
    char reserved337;
    unsigned short coinCount; // tour progression coins, independent of Pang
    unsigned char completedEventFlags;
};
// View at (map record + item-index * sizeof(short)); +4 names that item's flag word.
// The leading bytes are address bias only, not a second per-item record header.
struct TourProgressItemFlagView {
    char addressBias[4];
    unsigned short flags;
};
// Integer-key tree layout: self+0x10 holds (count, root, first). The ADDRESS of
// root is the end sentinel; a node has left/right/parent-and-color at +0/+4/+8,
// followed by (int key, item pointer) at +0xC/+0x10. Mask parent bit 0 before
// following it. Traversal is in key order; node ownership differs from the item
// pointer's lifetime. These views do not imply an extra sentinel allocation.
struct TourTreeFields {
    char reserved[0x10];
    struct {
        unsigned int size;
        void* root;
        void* first;
    } items;
};
struct TourTreeNodeFields {
    void* left;
    void* right;
    unsigned int parentAndColor;
    int key;
    void* item;
};
struct TourMapRecordPrefix {
    short index;
    short connectionStyles[4];
    signed char iconKind;
    char reservedB[2];
    unsigned char matchType;
    char reservedE[2];
    int beforeEvent;
    char reserved14[0x10];
    int afterEvent;
};
// Script event channels are independent: linkEvent/linkValue drive connection
// reveal, itemEvent/itemValue drive reward bursts. *Frames are script/update
// counts and *Opacity are 0..255; this unit reads frame counts, not dt to advance
// them. A link value encodes item = value % 100 and visual column by value / 100.
struct TourEventFields {
    void* saveRecord;
    unsigned char linkEvent;
    char reserved5[3];
    int linkValue;
    unsigned char linkOpacity;
    char reservedD[3];
    unsigned int linkFrames;
    unsigned char itemEvent;
    char reserved15[3];
    int itemValue;
    unsigned char itemOpacity;
    char reserved1D[3];
    unsigned int itemFrames;
};
struct TourNumberFields {
    char reserved[0x28];
    int enabled;
};
struct TourPreviewFields {
    char reserved[0x3C];
    void* preview;
};
struct TourTableHeader {
    char reserved[8];
    int recordCount;
};
// Map-table binary boundary (CSharedFileHandleHolder -> SharedFilePtr_GetLoadedBuffer
// in tour_change_character_window.cpp, then InitItemMap -> BuildEntries here): bytes are borrowed from
// mapFile for the entire node lifetime. No byte swap, validation or relocation
// of row contents occurs at this boundary. Table_GetEntry (endian_format_and_binary_table.cpp) computes
// base+0x20+rowIndex*header.entryStride (+0C); recordCount is signed +08 and
// dataOffset is signed +10. Require aligned, native BIG-ENDIAN Wii words/shorts,
// nonnegative count, stride>=0x88, and enough bytes for every row and referenced
// payload. No particular tag/version at +00/+04 is checked by these consumers;
// a replacement must preserve the existing format rather than invent a version.
// String words are signed 32-bit RELATIVE OFFSETS despite OpenItemRecord's
// void* spelling: resolve as tableBase+dataOffset+word, then skip the 8-byte
// OffsetStringNode header to its terminated text. A zero word is NOT a null
// pointer; only dataOffset==0 makes Table_ResolveDataOffset return null.
// The source views below/header views overlay ONE serialized row. Full consumed
// extent is [00,88); bytes with no identified consumer are explicitly reserved.
// This is the decoded-buffer schema, not an assertion about archive compression
// or byte order before the shared-file provider produces that buffer.
struct TourMapSerializedRow {
    short itemIndex; // +00: -1 absent, otherwise [0,35)
    // +02: left/right/up/down CONNECTION STYLES, -1 disables the edge;
    // style==2 also hides selected lit link visuals in UpdateEntryVisuals.
    // These are not destination IDs: GetNeighborIndex (tour_task_item_map_and_model_preview.cpp) computes
    // destination as index +/-1 or +/-5 on the fixed 5-column, 7-row grid.
    short connectionStyles[4];
    signed char iconKind;        // +0A: atlas Y=kind*32; kind==1 blocks confirm
    signed char flag13, flag14;  // +0B/+0C: nonzero initializes reward/hidden flags
    signed char previewMode;     // +0D: 0 none, 1..6 supported; link-active class 1/5/6
    short previewModelIndex;     // +0E: [0,8) for modes 1/5/6; modes 2/3/4 use mode table
    int beforeEvent;             // +10: -1 absent, otherwise narrowed to u8
    int primaryTextOffsets[2];   // +14/+18: first/second encounter text
    int secondaryTextOffsets[2]; // +1C/+20: paired secondary text
    int afterEvent;              // +24: -1 absent, otherwise narrowed to u8
    signed char partyMode;       // +28: 1 score / 2 distance / 3 count
    unsigned char reserved29;
    // +2A..3B are nine party shorts: three {target,comparison,reward} triples.
    // The same bytes also supply the player's previewModelIndex at +2C and
    // caddieIndex at +2F in PollItemMapResources; preserve these overlapping roles.
    short partyTriples[9];
    short golfRuleType, enemyRecord; // +3C/+3E: -1 no golf; enemy table index
    unsigned char reserved40[2];
    unsigned char courseId, weather; // +42/+43: nonparty encounter fields
    // Party setup instead reads signed +42 as course selector and unsigned +44
    // as its extra selection argument (TourPartyConfig_ApplyToSharedDoc).
    signed char holeNumbers[18];              // +44..55: nonparty holes, 0 unused
    short tourCoins;                          // +56: first completion coins
    unsigned int playerReward;                // +58: first completion player reward
    InventoryReward setItems[4];              // +5C..6B: inventory index/count pairs
    InventoryReward parts[4];                 // +6C..7B: inventory index/count pairs
    InventoryReward clubSet, ball, character; // +7C/+80/+84: index/count pairs
};
typedef char TourMapRow_size[sizeof(TourMapSerializedRow) == 0x88 ? 1 : -1];
typedef char TourMapRow_text[IFF_OFFSET(TourMapSerializedRow, primaryTextOffsets) == 0x14 ? 1 : -1];
typedef char TourMapRow_party[IFF_OFFSET(TourMapSerializedRow, partyTriples) == 0x2A ? 1 : -1];
typedef char TourMapRow_rewards[IFF_OFFSET(TourMapSerializedRow, setItems) == 0x5C ? 1 : -1];
struct TourItemRequestFields {
    char reserved[0x24];
    int processing;
};

enum TourMapFlags {
    kNumericRewardCountedBit = 4, // Set by TourMap_CommitNodeCompletionAndRewards (tour_main_task.cpp).
    kMapInitialized = 0x20,
    kRewardClaimed = 0x200,
    kMapItemEnabled = 0x400,
    kAfterEventSeen = 0x800,
    kBeforeEventSeen = 0x1000
};

// Byte offsets within the existing pooled strings; storage and encodings stay unchanged.
enum TourMapTextOffset {
    kTourText_tour_map_common_2d_zip = 0x0,        // tour_map_common_2d.zip
    kTourText_tour_map_common_data_zip = 0x17,     // tour_map_common_data.zip
    kTourText_tour_map_format_b2d = 0x30,          // tour_map_format.b2d
    kTourText_coin_open_item_data_ecb = 0x44,      // coin_open_item_data.ecb
    kTourText_tour_ingm_b2d = 0x5c,                // tour_ingm.b2d
    kTourText_tour_ingm_spd = 0x6a,                // tour_ingm.spd
    kTourText_map_format = 0x78,                   // map_format
    kTourText_tour_ingm_def = 0x83,                // tour_ingm_def
    kTourText_common_num_0_gsr = 0x91,             // common_num_0.gsr
    kTourText_tour_sea_wave = 0xa2,                // tour_sea_wave
    kTourText_tour_sea_wave_2 = 0xb0,              // tour_sea_wave_2
    kTourText_R_btn_l = 0xc0,                      // R_btn_l
    kTourText_R_btn_r = 0xc8,                      // R_btn_r
    kTourText_R_clubh_icon = 0xd0,                 // R_clubh_icon
    kTourText_R_chara_icon = 0xdd,                 // R_chara_icon
    kTourText_vector_at_index_out_of_range = 0xea, // vector::at index out of range
    kTourText_std_exception_std_logic_error_std_out_of_rang =
        0x108,                                  // !std::exception!!std::logic_error!!std::out_of_range!!
    kTourText_ui_icon_click = 0x13f,            // ui_icon_click
    kTourText_EmptyName = 0x14d,                // empty name
    kTourText_tree_insert_length_error = 0x14e, // tree::insert length error
    kTourText_std_exception_std_logic_error_std_length_erro =
        0x168,                                           // !std::exception!!std::logic_error!!std::length_error!!
    kTourText_basic_string_reserve_length_error = 0x19f, // basic_string::reserve length_error
    kTourText_basic_string_out_of_range = 0x1c2,         // basic_string: out_of_range
    kTourText_basic_string_length_error = 0x1dd,         // basic_string: length_error
};

#include "classes/CBpObject.h"
#include "classes/CEnemyRenderRecord.h"
#include "classes/CEnemyRenderTable.h"
#include "classes/CCameraController.h"
#include "classes/CSharedDoc.h"
#include "classes/OpenItemEntry.h"
#include "classes/OpenItemFlags.h"
#include "classes/OpenItemMenuContext.h"
#include "classes/OpenItemMenuState.h"
#include "classes/OpenItemRecord.h"
#include "classes/OpenItemTreeNode.h"
#include "classes/OpenItemTreePair.h"

// Borrow the CBpObject field view; byte-cursor callers retain the original ABI.
static inline CBpObject* AsCBpObject(void* object) {
    return (CBpObject*)object;
}
// Borrow the CCameraController field view; byte-cursor callers retain the original ABI.
static inline CCameraController* AsCCameraController(void* object) {
    return (CCameraController*)object;
}
// Borrow the CSharedDoc field view; byte-cursor callers retain the original ABI.
static inline CSharedDoc* AsCSharedDoc(void* object) {
    return (CSharedDoc*)object;
}
// Borrow the OpenItemEntry field view; byte-cursor callers retain the original ABI.
static inline OpenItemEntry* AsOpenItemEntry(void* object) {
    return (OpenItemEntry*)object;
}
// Borrow the OpenItemFlags field view; byte-cursor callers retain the original ABI.
static inline OpenItemFlags* AsOpenItemFlags(void* object) {
    return (OpenItemFlags*)object;
}
// Borrow the OpenItemMenuContext field view; byte-cursor callers retain the original ABI.
static inline OpenItemMenuContext* AsOpenItemMenuContext(void* object) {
    return (OpenItemMenuContext*)object;
}
// Borrow the OpenItemMenuState field view; byte-cursor callers retain the original ABI.
static inline OpenItemMenuState* AsOpenItemMenuState(void* object) {
    return (OpenItemMenuState*)object;
}
// Borrow the OpenItemRecord field view; byte-cursor callers retain the original ABI.
static inline OpenItemRecord* AsOpenItemRecord(void* object) {
    return (OpenItemRecord*)object;
}
// Borrow the OpenItemTreeNode field view; byte-cursor callers retain the original ABI.
static inline OpenItemTreeNode* AsOpenItemTreeNode(void* object) {
    return (OpenItemTreeNode*)object;
}
// Borrow the OpenItemTreePair field view; byte-cursor callers retain the original ABI.
static inline OpenItemTreePair* AsOpenItemTreePair(void* object) {
    return (OpenItemTreePair*)object;
}

#include "game/vector.h"

#include "game/singletons.h"
#include <decomp/cw_bitfields.h>

extern "C" {

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u802EC144 {
    char* base;
    bool active;
    // Arm partial render-context cleanup until reset completes successfully.
    explicit RenderContextConstructionCleanup_u802EC144(void* object) : base((char*)object), active(true) {}
    // Destroy records and vector storage only if construction failed.
    ~RenderContextConstructionCleanup_u802EC144() {
        if (active) {
            CharacterPreviewTable_dtor((char*)((TourRenderContextFields*)base)->records, -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)((char*)&((TourRenderContextFields*)base)->vectors[index]);
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Initialize the shared render context with cleanup if construction throws.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u802EC144 cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}

void __dl__FPv(void*);
void appFreeUni__FPv(void*);
void ItemRequestState_Ctor(void*);
void ItemRequestState_Dtor();
void __register_global_object(void*, void (*)(), void*);
void ItemRequestState_RequestSetItemByInventoryIndex(void*, int, short, short);
void ItemRequestState_RequestPartByInventoryIndex(void*, int, short, short);
void ItemRequestState_RequestClubSetByInventoryIndex(void*, int, short, short);
void ItemRequestState_RequestBallByInventoryIndex(void*, int, short, short);
void ItemRequestState_RequestCharacterByInventoryIndex(void*, int, short, short);
void ItemRequestState_RequestCaddieByInventoryIndex(void*, int, short, short);
void ItemRequestState_RequestCaddieItemByInventoryIndex(void*, int, short, short);
void TourModelPreview_UpdatePlacementForward(void*, float, float*, float*);
// Refresh node icons, unlock markers, and connections from persistent item flags.
void OpenItemMenu_UpdateEntryVisuals(void*);
void Particle2D_UpdateTrailAndBurst(void*, float);
void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
extern signed char s_renderContextInitialized;
extern void* g_sharedDoc;
extern signed char s_pangyaEnemyDataInitialized;
extern char s_renderContext[0x2364], s_pangyaEnemyData[0x10], tourMapOpenItemStrings[0x1F8],
    s_renderContextDtorRegistration_TourItemMenu[0xC], s_pangyaEnemyDataDtorRegistration_TourItemMenu[0x24];
void SaveData_InitPlayerRecord(void*);

void __construct_array(void*, void (*)(), void (*)(), int, int);
void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void RenderContext_ResetPlayerRecords(void*);
void __dt__13RenderContextFv(void*);
int EventScript_GetCharacterIndex();
int TourPartyConfig_ApplyToSharedDoc(void*, int);
void GolfPlayerList_BuildFromRenderTable(void*);
void __dt__16CPangyaEnemyDataFv(void*);
void EnemyData_BuildCharacterSlot(void*, void*, int, int);
int rand();
void SharedDoc_SelectCourse(int);
void SharedStringState_SetFirstStrings(int, void*, void*, int);
void SharedStringState_SetSecondStrings(int, void*, void*);
void* Table_ResolveDataOffset(void*, void*);
void* OffsetStringNode_GetText(void*);
void SharedStringState_SetWordValue(unsigned int);
void SharedStringState_SetShortValue(unsigned int);
void SharedStringState_SetContext(int);
void ItemRequestState_RequestSetItemByInventoryIndex(void*, int, short, short);
void ItemRequestState_RequestPartByInventoryIndex(void*, int, short, short);
void strcpy(char*, const char*);
void __register_global_object(void*, void (*)(), void*);
extern signed char s_itemRequestStateInitialized;
extern float kFloat_0_OpenItemMenu;
// IEEE binary32 WORDS, not integer distances: 4/5=300/200 tracking margins,
// 2/3=1024/512 board extents, 0/1=640/480 view extents in layout pixels.
// Values are defined in the data section; keep X/Y settings coordinated
// with board dimensions and the viewport/projection used for model placement.
extern unsigned int kOpenItemCameraBound4, kOpenItemCameraBound5, kOpenItemCameraBound2, kOpenItemCameraBound3,
    kOpenItemCameraBound0, kOpenItemCameraBound1;
extern char s_itemRequestState[0x90], s_itemRequestStateDtorRegistration_TourItemMenu[0xC];
extern signed char s_bpPlayerInitialized;
extern signed char s_bpCtrlInitialized;
extern unsigned int kOpenItemLayoutValue40A, kOpenItemLayoutValue40B;
extern char s_bpPlayer[], s_bpCtrl[], s_bpPlayerDtorRegistration_TourItemMenu[], s_bpCtrlDtorRegistration_8051D0EC[];
void __dt__9CBpPlayerFv();
int EventText_PollMessageResources(void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor();
void BpObject_Draw(void*);
void TourTwinkle_DrawParticles(void*, void*);
void CameraController_RenderPreview(void*, void*);
void TourBossPreview_RenderIfVisible(void*);
void Layout_UpdateCursorHover(void*, float);
void UITextTable_Draw(void*);
void BpObject_TryInvokeCurrentCallback(void*, void*);
void* Layout_GetElementSprite(void*, int);
void Layout_GetElementPosition(void*, int, void*);
void Layout_CollectHovered(void*, void*, int);
void Layout_CollectActivated(void*, void*, int);
void Layout_SetElementScale(void*, int, void*);
void Layout_SetElementPosition(void*, int, void*);
void Sprite_SetTextureRegionSize2D(void*, void*);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementFeedbackState(void*, int, int);
void UITextTable_SetInteger(void*, int, unsigned short);
void* IdMap_Lookup(void*, void*);
int BpObject_IsCurrentScriptComplete(void*);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void SharedStringState_SetCharacterIndex(int);
void SharedStringState_SetCaddieIndex(int);
void TourItemMenu_Init(void*, void*);
void fn_802FF430(void*);
void fn_802FF6F4(void*);
void TourWindow_EmptyHook0(void*);
void TourWindow_EmptyHook1(void*);
void TourParticleEffects_Init(void*);
void TourParticleEffects_EmptyHook(void*);
void TourLoadEffect_InitTextures(void*);
void Layout_Init(void*, int);
void Layout_LoadExtended(void*, int, void*);
void SoundManager_RequestBgmChange(void*, void*, int, int);
void PointerVector_InitReserved(void*);
void PointerVector_DestroyStorage();
extern signed char s_bpPlayerInitialized;
extern signed char s_bpCtrlInitialized;
extern signed char s_pointerVectorInitialized;
extern void* g_soundManager;
extern char s_tourMapResourceDescriptors[], s_bpCtrl[], s_fontNumDrawRegistry[0xC];
extern char s_itemMenuSingletonRegistration_8051D0E0[];
void UIGroup_SetEnabled(void*, int);
void UIGroup_SetSourcePosition(void*, float*);
void UIGroup_SetTintColor(void*, int);
int OpenItemEntry_GetFlagBit14(void*);
int OpenItemEntry_GetFlagBit15(void*);
void Layout_ReadElementRect(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
void Layout_SetElementSourceSize(void*, int, void*);
void Layout_SetElementPlayerFlag(void*, int, int, int);
int BpObject_PollLoad(void*);
int CameraController_PollPreview(void*);
int SharedFilePtr_HasLoadedBuffer(void*);
int TourLoadEffect_AreTexturesReady(void*);
void Layout_UpdateExtended(void*);
int Layout_PollLoadComplete(void*);
void* __nw__FUl(unsigned int);
void* TourBossPreview_InitState(void*, const void*);
void* IntegerKeyTree_FindInsertionPosition(void*, int*, int*, unsigned char*, unsigned char*);
void* BpEntryTree_InsertNode(void*, void*, unsigned char, unsigned char, int*);
void OpenItemEntry_SetFlagBit15(void*, int);
void OpenItemEntry_SetDirectionEnabled(void*, int, signed char);
void OpenItemEntry_SetFlagBit13(void*, int);
void OpenItemEntry_SetFlagBit14(void*, int);
void UIGroup_GetPosition(void*, float*);
void OpenItemEntry_SetPosition(void*, float*);
void* Table_GetEntry(void*, int);
void OpenItemEntry_SetRecord(void*, void*);
void TourBossPreview_LoadModel(void*);
extern float kOpenItemLabelOffsetX, kOpenItemLabelOffsetY;
struct F0BTable {
    const char* names[35][5];
};
extern F0BTable s_tourMapItemGroupNames;
extern void* g_bpEntryDefaultMappedValue;
extern void* g_soundManager;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[], s_bpCtrlDtorRegistration_8051D0EC[];
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor();
void __register_global_object(void*, void (*)(), void*);
void* IdMap_Lookup(void*, void*);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
// Hide map navigation icons and restore their layout rectangles.
// Create map nodes, initialize their direction and unlock flags, then attach records and previews.
void OpenItemMenu_BuildEntries(void*);
void* SharedFilePtr_GetLoadedBuffer(void*);
void TourEventMessage_ReplaceEffect(void*, int);
// Process scripted map events, connection reveals, item effects, and hint/versus windows.
void Object_DeleteAndClear(void*);
void* BpObject_StartScriptByName(void*, const char*, int);
int SoundManager_IsAliasPrefixOrStreamStemActive(void*, const char*);
extern signed char s_effectFadeInitialized;
extern char s_effectFade[0x458], s_effectFadeDtorRegistration_TourItemMenu[0x24], __vt__8CMapData[0x138],
    __vt__17CCoinOpenItemData[0x88];
extern char s_tourMapModeScriptNames[];
extern char s_tourCoinNumberEntries[0x10], s_tourMapMusicNames[0x2C];
extern float kFloat_0_5_OpenItemMenu, kFloat_200_OpenItemMenu, kFloat_1_OpenItemMenu;
void __dt__11CEffectFadeFv();
void UIGroup_SetEnabled(void*, int);

// Lazily create the global BpCtrl (the BpObject controller singleton) and publish it in its slot.
static inline void EnsureBpCtrl() {
    if (!s_bpCtrlInitialized) {
        ((CBpCtrl*)&s_bpCtrl)->__ct();
        __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
        s_bpCtrlInitialized = 1;
    }
    Sing_805DC5F0() = &s_bpCtrl;
}

// Build map items, restore the selected node, and initialize map numbers and ambient sounds.
// Requires completed archive/UI loads and a valid character-map index 0..10.
// dt is forwarded in the animation caller's clock; it is not a presentation
// interval selector. First-use preview update and initial script execution
// are deliberate; preserve their order and RNG wave selection when changing
void CTourMain::InitItemMap(float dt) {
    float mapTranslation[2] = {0.0f, 0.0f};
    // Phase 1: reset preview/fade and start the mode, map-item and number scripts.
    ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->ResetPreview();
    if (!Sing_805DC660()) {
        if (!s_effectFadeInitialized) {
            ((CEffectFade*)&s_effectFade)->__ct();
            __register_global_object(&s_effectFade, __dt__11CEffectFadeFv, &s_effectFadeDtorRegistration_TourItemMenu);
            s_effectFadeInitialized = 1;
        }
        Sing_805DC660() = &s_effectFade;
    }
    ((CEffectFade*)Sing_805DC660())->ClearColor();
    /* 0x802EC994-0x802EC998: r3 = the fade singleton (ours passed whatever r3 held, garbage when it was already built) */
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    void* mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->modeGroup);
    unsigned char mode = AsOpenItemMenuState(this)->tourCharacterIndex;
    BpObject_StartScriptByName(mgr, *(const char**)((char*)&s_tourMapModeScriptNames + mode * 4), 1);
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->itemGroup);
    BpObject_StartScriptByName(mgr, (char*)tourMapOpenItemStrings + kTourText_map_format, 1);
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->itemGroup);
    BpObject_TryInvokeCurrentCallback(mgr, mapTranslation);
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->numberGroup);
    BpObject_StartScriptByName(mgr, (char*)tourMapOpenItemStrings + kTourText_tour_ingm_def, 1);
    // Phase 2: wrap borrowed map/coin tables, build nodes and run the initial script.
    void* p = __nw__FUl(8);
    if (p) {
        void* v = SharedFilePtr_GetLoadedBuffer(AsOpenItemMenuState(this)->mapFile);
        *(void**)p = 0;
        if (v)
            *(void**)p = v;
        *(void**)((char*)p + 4) = &__vt__8CMapData;
    }
    (*(void**)&AsOpenItemMenuState(this)->mapData) = p;
    OpenItemMenu_BuildEntries(this);
    p = __nw__FUl(8);
    if (p) {
        void* v = SharedFilePtr_GetLoadedBuffer(AsOpenItemMenuState(this)->itemFile);
        *(void**)p = 0;
        if (v)
            *(void**)p = v;
        *(void**)((char*)p + 4) = &__vt__17CCoinOpenItemData;
    }
    AsOpenItemMenuState(this)->coinItemData = p;
    TourEventMessage_ReplaceEffect(AsOpenItemMenuState(this)->messageFile, 0);
    ProcessSelectedMapItem();
    Object_DeleteAndClear(AsOpenItemMenuState(this)->messageFile);
    // Phase 3: initialize first-use selection, then restore the persisted selected item.
    void* flags = AsOpenItemMenuState(this)->selectionRecord;
    if (((*(unsigned char*)flags >> 5) & 1) == 0) {
        short index = AsOpenItemMenuState(this)->initialSelection;
        int key = index;
        void* node = 0;
        unsigned char a = 0, b = 0;
        void* value =
            IntegerKeyTree_FindInsertionPosition((void*)&((TourTreeFields*)this)->items, &key, (int*)&node, &a, &b);
        int* tail = node ? (int*)((void*)&((TourTreeNodeFields*)node)->key) : 0;
        if (!tail || key > *tail) {
            int pair[2] = {key, (int)g_bpEntryDefaultMappedValue};
            node = BpEntryTree_InsertNode((void*)&((TourTreeFields*)this)->items, value, a, b, pair);
            tail = (int*)((void*)&((TourTreeNodeFields*)node)->key);
        }
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->SetItemEntry(AsOpenItemTreePair(tail)->item);
        key = index;
        node = 0;
        a = 0;
        b = 0;
        value = IntegerKeyTree_FindInsertionPosition((void*)&((TourTreeFields*)this)->items, &key, (int*)&node, &a, &b);
        tail = node ? (int*)((void*)&((TourTreeNodeFields*)node)->key) : 0;
        if (!tail || *tail < key) {
            int pair[2] = {key, (int)g_bpEntryDefaultMappedValue};
            node = BpEntryTree_InsertNode((void*)&((TourTreeFields*)this)->items, value, a, b, pair);
            tail = (int*)((void*)&((TourTreeNodeFields*)node)->key);
        }
        OpenItemEntry_SetFlagBit15(AsOpenItemTreePair(tail)->item, 1);
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->Update(dt, &AsOpenItemMenuState(this)->cameraOffsetX);
        short* cur = (short*)((char*)flags + 2);
        if (*cur < 0)
            *cur = index;
        AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->nextState = 0;
    }
    // Reacquire through the inserting lookup before moving the camera to the saved selection.
    short key = AsOpenItemFlags(flags)->selectedItem;
    int k = key;
    void* node = 0;
    unsigned char a = 0, b = 0;
    void* value = IntegerKeyTree_FindInsertionPosition((void*)&((TourTreeFields*)this)->items, &k, (int*)&node, &a, &b);
    int* tail = node ? (int*)((void*)&((TourTreeNodeFields*)node)->key) : 0;
    if (!tail || *tail < k) {
        int pair[2] = {k, (int)g_bpEntryDefaultMappedValue};
        node = BpEntryTree_InsertNode((void*)&((TourTreeFields*)this)->items, value, a, b, pair);
        tail = (int*)((void*)&((TourTreeNodeFields*)node)->key);
    }
    ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->SetItemEntry(AsOpenItemTreePair(tail)->item);
    ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
        ->Update(dt, &AsOpenItemMenuState(this)->cameraOffsetX);
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->numberGroup);
    ((CFontNumDraw*)(AsOpenItemMenuState(this)->numberDraw))
        ->RegisterEntries(s_tourCoinNumberEntries, 1, mgr, (char*)tourMapOpenItemStrings + kTourText_common_num_0_gsr);
    ((TourNumberFields*)AsOpenItemMenuState(this)->numberDraw)->enabled = 1;
    {
        const char* const* tbl = (const char* const*)&s_tourMapItemGroupNames;
        for (int outer = 0; outer < kTourItemsPerMap; ++outer) {
            for (int inner = 0; inner < kTourVisualColumns; ++inner) {
                const char* str = tbl[outer * kTourVisualColumns + inner];
                if (str[0] != 0) {
                    if (!Sing_805DC5F0())
                        EnsureBpCtrl();
                    void* mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->itemGroup);
                    if (mgr) {
                        void* node = BpObject_GetOrCreateNamedGroup(mgr, str);
                        if (node)
                            UIGroup_SetEnabled(node, 0);
                    }
                }
            }
        }
    }
    // Phase 4: hide all group visuals before rebuilding node state and starting ambient audio.
    ResetItemMapControls();
    OpenItemMenu_UpdateEntryVisuals(this);
    mode = AsOpenItemMenuState(this)->tourCharacterIndex;
    const char* name = *(const char**)((char*)&s_tourMapMusicNames + mode * 4);
    if (name && !SoundManager_IsAliasPrefixOrStreamStemActive(g_soundManager, name))
        Sound_PlayByName(g_soundManager, name, 0, 1, (int)name, 0, 0.5f, 200.0f, 1.0f);
    if (mode == 0 || mode == 5 || mode == 8) {
        int randomValue = rand();
        int waveVariant = (int)(2.0f * (float)randomValue / 32768.0f);
        const char* text = (char*)tourMapOpenItemStrings + kTourText_tour_sea_wave_2;
        text = ((signed char)waveVariant) ? (char*)tourMapOpenItemStrings + kTourText_tour_sea_wave : text;
        Sound_PlayByName(g_soundManager, text, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
    }
    *(unsigned char*)flags |= kMapInitialized;
}

// Create the menu-owned windows, particle system and navigation layout after archive readiness.
// allocation is the caller's scratch pointer; each published member takes ownership immediately.
static inline void CreateTourItemWindowsAndLayout(void* self, void*& allocation) {
    allocation = __nw__FUl(0x28);
    if (allocation)
        TourItemMenu_Init(allocation, *(*(void***)&AsOpenItemMenuState(self)->menuContext));
    AsOpenItemMenuState(self)->tourWindow = allocation;
    ((CTourWindow*)allocation)->ResetNumberDraw();
    ((CTourWindow*)(AsOpenItemMenuState(self)->tourWindow))->LoadResources();
    allocation = __nw__FUl(1);
    if (allocation)
        fn_802FF430(allocation);
    AsOpenItemMenuState(self)->tourHintWindow = allocation;
    fn_802FF6F4(allocation);
    ((CTourHintWindow*)(AsOpenItemMenuState(self)->tourHintWindow))->RecreateLayoutEntries();
    allocation = __nw__FUl(1);
    if (allocation)
        TourWindow_EmptyHook0(allocation);
    AsOpenItemMenuState(self)->tourVsWindow = allocation;
    TourWindow_EmptyHook1(allocation);
    ((CTourVSWindow*)(AsOpenItemMenuState(self)->tourVsWindow))->LoadUIResources();
    allocation = __nw__FUl(0x5c);
    if (allocation)
        TourParticleEffects_Init(allocation);
    AsOpenItemMenuState(self)->particleEffects = allocation;
    TourParticleEffects_EmptyHook(allocation);
    TourLoadEffect_InitTextures(AsOpenItemMenuState(self)->particleEffects);
    allocation = __nw__FUl(0x5c);
    if (allocation)
        Layout_Init(allocation, 5);
    AsOpenItemMenuState(self)->layout = allocation;
    Layout_LoadExtended(allocation, 0, (char*)tourMapOpenItemStrings + kTourText_tour_ingm_spd);
}

// Load the selected map archives and create their UI resources when ready.
int CTourMain::PollItemMapResources() {
    unsigned char mode0 = AsOpenItemMenuState(this)->tourCharacterIndex;
    unsigned int table[55];
    char* g = (char*)&s_itemMenuSingletonRegistration_8051D0E0;
    // Phase 1: poll the two texture archives and two data archives; return 0 until all load.
    ((CPackTextureResource*)(AsOpenItemMenuState(this)->mapTexture))->PollLoad();
    ((CPackTextureResource*)(AsOpenItemMenuState(this)->itemTexture))->PollLoad();
    ((CPackedFile*)(AsOpenItemMenuState(this)->mapPackedFile))->PollLoad();
    ((CPackedFile*)(AsOpenItemMenuState(this)->itemPackedFile))->PollLoad();
    if (!((CPackTextureResource*)(AsOpenItemMenuState(this)->mapTexture))->IsLoadFinished())
        return 0;
    if (!((CPackTextureResource*)(AsOpenItemMenuState(this)->itemTexture))->IsLoadFinished())
        return 0;
    if (!((CPackedFile*)(AsOpenItemMenuState(this)->mapPackedFile))->IsLoadFinished())
        return 0;
    if (!((CPackedFile*)(AsOpenItemMenuState(this)->itemPackedFile))->IsLoadFinished())
        return 0;
    // Resource descriptor rows are five words (20 bytes): map-data path,
    // message path, BP-player path, mode-group path and BGM name, per map.
    const unsigned int* src = (const unsigned int*)&s_tourMapResourceDescriptors;
    unsigned int* sp = (unsigned int*)src - 1;
    unsigned int* dp = table - 1;
    int ctr = 0x1b;
    do {
        unsigned int a = sp[1];
        unsigned int b = sp[2];
        // Retain the source-advance temporary to preserve the descriptor-copy register schedule.
        unsigned int* nextSourceWords = sp + 2;
        sp = nextSourceWords;
        dp[1] = a;
        *(dp += 2) = b;
    } while (--ctr);
    dp[1] = sp[1];
    // Phase 2: create the character camera and borrowed file-handle wrappers.
    void* data = *(*(void***)&AsOpenItemMenuState(this)->menuContext);
    signed char a = AsOpenItemRecord(data)->previewModelIndex;

    void* p = __nw__FUl(0x30);
    if (p) {
        try {
            ((CCameraController*)p)->__ct(a);
        } catch (...) {
            __dl__FPv(p);
            throw;
        }
    }
    AsOpenItemMenuState(this)->cameraController = p;
    SharedStringState_SetCharacterIndex(a);
    SharedStringState_SetCaddieIndex((AsOpenItemRecord(data)->caddieIndex));

    p = __nw__FUl(4);
    if (p) {
        try {
            ((CSharedFileHandleHolder*)p)->__ct((void*)*(unsigned int*)((char*)table + mode0 * 20));
        } catch (...) {
            __dl__FPv(p);
            throw;
        }
    }
    AsOpenItemMenuState(this)->mapFile = p;
    p = __nw__FUl(8);
    if (p) {
        try {
            ((CSharedFileHandleOwner*)p)->__ct((void*)*(unsigned int*)((char*)table + mode0 * 20 + 4));
        } catch (...) {
            __dl__FPv(p);
            throw;
        }
    }
    AsOpenItemMenuState(this)->messageFile = p;
    if (!Sing_805DC670()) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)&s_bpPlayer)->__ct();
            __register_global_object(&s_bpPlayer, __dt__9CBpPlayerFv, (g + 0x90));
            s_bpPlayerInitialized = 1;
        }
        Sing_805DC670() = &s_bpPlayer;
    }
    ((CBpPlayer*)Sing_805DC670())->ReleaseResources();
    if (Sing_805DC670() == 0) {
        if (!s_bpPlayerInitialized) {
            ((CBpPlayer*)&s_bpPlayer)->__ct();
            __register_global_object(&s_bpPlayer, __dt__9CBpPlayerFv, (g + 0x90));
            s_bpPlayerInitialized = 1;
        }
        Sing_805DC670() = &s_bpPlayer;
    }
    unsigned char mode = AsOpenItemMenuState(this)->tourCharacterIndex;

    ((CBpPlayer*)Sing_805DC670())->StartLoad((const char*)*(unsigned int*)((char*)table + 20 * (mode) + 8), 1, 0);
    if (0 == Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, (g + 0xc));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }

    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry((void*)*(unsigned int*)((char*)table + (mode * 20) + 12),
                      (char*)&AsOpenItemMenuState(this)->modeGroup, 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, (g + 0xc));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry((char*)tourMapOpenItemStrings + kTourText_tour_map_format_b2d,
                      (char*)&AsOpenItemMenuState(this)->itemGroup, 0);
    p = __nw__FUl(4);
    if (p) {
        try {
            ((CSharedFileHandleHolder*)p)
                ->__ct((void*)((char*)tourMapOpenItemStrings + kTourText_coin_open_item_data_ecb));
        } catch (...) {
            __dl__FPv(p);
            throw;
        }
    }
    AsOpenItemMenuState(this)->itemFile = p;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, (g + 0xc));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry((char*)tourMapOpenItemStrings + kTourText_tour_ingm_b2d,
                      (char*)&AsOpenItemMenuState(this)->numberGroup, 0);
    // Phase 3: create tour/hint/versus windows, particles, navigation layout and number renderer.
    CreateTourItemWindowsAndLayout(this, p);

    SoundManager_RequestBgmChange(g_soundManager, (void*)*(unsigned int*)((char*)table + (20 * mode) + 16), 0, 1);
    if (!Sing_805DC830()) {
        if (!s_pointerVectorInitialized) {
            PointerVector_InitReserved(&s_fontNumDrawRegistry);
            __register_global_object(&s_fontNumDrawRegistry, PointerVector_DestroyStorage, (g + 0x48));
            s_pointerVectorInitialized = 1;
        }
        Sing_805DC830() = &s_fontNumDrawRegistry;
    }
    AsOpenItemMenuState(this)->numberDraw = CFontNumDraw::CreateInVector((PtrVector*)Sing_805DC830());
    return 1;
}

// Advance item animations, camera-relative UI positions, and map particles.
// Requires initialized menu and nonnull nodes; all preview nodes are borrowed
// from the owned tree. Walk in key order without mutation. mapTranslation
// is viewport layout pixels (-cameraOffsetX,-cameraOffsetY); origin and
// controller position passed to previews are board layout pixels. dt forwards
// to model animation and particles, while automatic preview spin is per call.
// Duplicate mode callbacks below are preserved: they may publish script state.
void CTourMain::UpdateItemMenuVisuals(float dt) {
    float mapTranslation[2] = {-*&AsOpenItemMenuState(this)->cameraOffsetX, -AsOpenItemMenuState(this)->cameraOffsetY};
    if (0 == Sing_805DC5F0()) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    void* mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->modeGroup);
    BpObject_TryInvokeCurrentCallback(mgr, mapTranslation);
    if (Sing_805DC5F0() == 0) {
        if (0 == s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->modeGroup);
    // Loop the map animation script and retrigger wave ambience only on maps 0, 5 and 8.
    if (BpObject_IsCurrentScriptComplete(mgr)) {
        if (!Sing_805DC5F0())
            EnsureBpCtrl();
        mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->modeGroup);
        AsCBpObject(mgr)->callbackInvoked = 0;
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)&s_bpCtrl)->__ct();
                __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = &s_bpCtrl;
        }
        mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->modeGroup);
        unsigned char mode = AsOpenItemMenuState(this)->tourCharacterIndex;

        BpObject_StartScriptByName(mgr, *(const char**)((char*)&s_tourMapModeScriptNames + (mode) * 4), 1);
        if (!Sing_805DC5F0()) {
            if (0 == s_bpCtrlInitialized) {
                ((CBpCtrl*)&s_bpCtrl)->__ct();
                __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = &s_bpCtrl;
        }
        mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->modeGroup);
        BpObject_TryInvokeCurrentCallback(mgr, mapTranslation);
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)&s_bpCtrl)->__ct();
                __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = &s_bpCtrl;
        }
        mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->modeGroup);
        AsCBpObject(mgr)->callbackInvoked = 0;
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)&s_bpCtrl)->__ct();
                __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = &s_bpCtrl;
        }
        mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->modeGroup);
        BpObject_TryInvokeCurrentCallback(mgr, mapTranslation);
        mode = AsOpenItemMenuState(this)->tourCharacterIndex;
        if (mode == 0 || mode == 5 || 8 == mode) {
            int randomValue = rand();

            const char* text = (const char*)tourMapOpenItemStrings + kTourText_tour_sea_wave_2;
            if ((signed char)((int)(2.0f * (float)randomValue / 32768.0f)))
                text = (const char*)tourMapOpenItemStrings + kTourText_tour_sea_wave;
            Sound_PlayByName(g_soundManager, text, 0, 0, 0, 0, 0.5f, 200.0f, 1.0f);
        }
    }
    if (!Sing_805DC5F0()) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->itemGroup);

    BpObject_TryInvokeCurrentCallback(mgr, mapTranslation);
    // Apply negative camera translation to BP layers; update item previews in map coordinates.
    float pos[2];
    ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetPosition(pos);
    void* sentinel = (void*)&((TourTreeFields*)this)->items.root;
    void* node = AsOpenItemMenuState(this)->firstItemNode;
    while (node != sentinel) {
        void* child = AsOpenItemTreeNode(node)->item;
        if (child)
            TourModelPreview_UpdatePlacementForward(child, dt, &AsOpenItemMenuState(this)->cameraOffsetX, pos);
        void* next = AsOpenItemTreeNode(node)->right;
        if (!(!(next))) {
            while (*(void**)next)
                next = *(void**)next;
            node = next;
        } else {
            while (node != sentinel) {
                void* parent = (void*)(AsOpenItemTreeNode(node)->parentAndColor & ~1u);
                if (node == *(void**)parent)
                    break;
                node = parent;
            }
            node = (void*)(AsOpenItemTreeNode(node)->parentAndColor & ~1u);
        }
    }
    OpenItemMenu_UpdateEntryVisuals(this);
    Particle2D_UpdateTrailAndBurst(AsOpenItemMenuState(this)->particleEffects, dt);
}

// Hide named navigation icons and restore the paired layout rectangles/portrait source size.
void CTourMain::ResetItemMapControls() {
    void* owner = AsOpenItemMenuState(this)->numberGroup;
    char* base = (char*)tourMapOpenItemStrings;
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    void* selected = IdMap_Lookup(Sing_805DC5F0(), owner);
    if (selected)
        selected = BpObject_GetOrCreateNamedGroup(selected, base + kTourText_R_btn_l);
    else
        selected = 0;
    if (selected)
        UIGroup_SetEnabled(selected, 0);
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    selected = IdMap_Lookup(Sing_805DC5F0(), owner);
    if (selected)
        selected = BpObject_GetOrCreateNamedGroup(selected, base + kTourText_R_btn_r);
    else
        selected = 0;
    if (selected)
        UIGroup_SetEnabled(selected, 0);
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    selected = IdMap_Lookup(Sing_805DC5F0(), owner);
    if (selected)
        selected = BpObject_GetOrCreateNamedGroup(selected, base + kTourText_R_clubh_icon);
    else
        selected = 0;
    if (selected)
        UIGroup_SetEnabled(selected, 0);
    if (!Sing_805DC5F0())
        EnsureBpCtrl();
    selected = IdMap_Lookup(Sing_805DC5F0(), owner);
    if (selected)
        selected = BpObject_GetOrCreateNamedGroup(selected, base + kTourText_R_chara_icon);
    else
        selected = 0;
    if (selected)
        UIGroup_SetEnabled(selected, 0);
    // The two constants are the float bit patterns for a 40-by-40 source rectangle.
    unsigned int sourceSizeBits[2] = {kOpenItemLayoutValue40A, kOpenItemLayoutValue40B};
    int rect[4]; // Rectangle copy requires four words; source size has only two.
    Layout_ReadElementRect(AsOpenItemMenuState(this)->layout, kTourShopControl, rect);
    Layout_WriteElementRect(AsOpenItemMenuState(this)->layout, kTourShopPairedRegion, rect);
    Layout_ReadElementRect(AsOpenItemMenuState(this)->layout, kTourCharacterControl, rect);
    Layout_WriteElementRect(AsOpenItemMenuState(this)->layout, kTourCharacterPairedRegion, rect);
    Layout_SetElementSourceSize(AsOpenItemMenuState(this)->layout, kTourCharacterPortrait, &sourceSizeBits[0]);
    Layout_SetElementPlayerFlag(AsOpenItemMenuState(this)->layout, kTourCharacterPortrait, 0, 5);
}

// data borrows an OpenItemEntry whose first word is its map record.
// Returns 0 for an item with no encounter; 1 after preparing shared state.
// `golfRuleType` at record +0x3C supplies the golf rule, while `enemyRecord`
// at +0x3E indexes the enemy data table. Shared field names are retained here.
// Configure the selected tour encounter, player render records, course holes, and return options.
// Record contract: native-endian runtime table entry, at least 0x88 bytes;
// OpenItemRecord and TourRewardFields are complementary views of that same
// entry, NOT independent allocations. Table_ResolveDataOffset resolves its
// string-offset words relative to mapData's data region; they are not pointers.
// self's map index must be [0,11), selectedItem [0,35); data/map/save required.
// +28 is the party mode union discriminator (1 score/2 distance/3 count in
// TourPartyConfig_ApplyToSharedDoc, minigame_aim_and_party_tiers.cpp); +2A..3A are three
// {target,comparison,reward} short triples in that branch. +3C is golf-rule
// code (-1 = absent); +3E is an enemy-table record index consumed unchecked
// by EnemyData_BuildCharacterSlot. Nonparty course +42 must resolve through
// SharedDoc_SelectCourse; +44 has 18 signed hole bytes, 0 = unused, otherwise
// a hole number for that course. Nonzero holes consume exactly one rand each.
// This routine counts nonzero entries but does NOT compact holes, so producers
// must populate a leading contiguous range for roundHoleCount consumers.
// +43 is weather: scene effects accept 0 clear/1 cloudy/2 rain/3 snow/4 special
// (course_scene_render_and_geometry.cpp). Course resource suffixes only index 0..2 (golf_bg_hole_resources.cpp); 3/4 use
// its fallback resource path. Other bytes have no established valid meaning.
// No enum/index/file-length validation here. A replacement map must preserve
// those consumer domains and the table's entry stride/endian conversion.
int CTourMain::PrepareSelectedEncounter(void* data) {
    void* record = *(void**)data;
    if (AsOpenItemRecord(record)->golfRuleType == -1 && AsOpenItemRecord(record)->unk_28 <= 0)
        return 0;
    if (Sing_805DC648() == 0) {
        if (s_renderContextInitialized == 0) {
            char* renderContext = s_renderContext;
            for (char* playerRecordCursor = renderContext;
                 playerRecordCursor < (char*)((TourRenderContextFields*)renderContext)->vectors;
                 playerRecordCursor += 0x828)
                SaveData_InitPlayerRecord(playerRecordCursor);
            ((IntVectorArray*)((char*)((TourRenderContextFields*)renderContext)->vectors))->__ct();
            __construct_array((char*)((TourRenderContextFields*)renderContext)->records,
                              (void (*)())PartyItemRecord_InitDefaults, (void (*)())PlayerRenderRecord_dtor, 0xa4, 4);
            ((TourRenderContextFields*)renderContext)->count = 0;
            InitializeNewRenderContext(renderContext);
            __register_global_object(renderContext, (void (*)())__dt__13RenderContextFv,
                                     s_renderContextDtorRegistration_TourItemMenu);
            s_renderContextInitialized = 1;
        }
        Sing_805DC648() = s_renderContext;
    }
    // Phase 1: seed the local player render record from the selected save snapshot.
    void* sharedDocument = g_sharedDoc;
    void* playerRenderRecords = (void*)((TourRenderContextFields*)Sing_805DC648())->records;
    (*(int*)&AsCSharedDoc(sharedDocument)->gameMode) = 6;
    int characterIndex = (signed char)EventScript_GetCharacterIndex();
    *(int*)playerRenderRecords = characterIndex;
    ((CEnemyRenderRecord*)playerRenderRecords)->flags &= 0x7fff; // original +0x1E
    ((TourEnemyPrefix*)playerRenderRecords)->state = 0;
    ((CCharacterPreviewRecord*)playerRenderRecords)
        ->FillFromSaveData(characterIndex, *(*(void***)&AsOpenItemMenuState(this)->menuContext));
    int characterMapIndex = AsOpenItemMenuState(this)->tourCharacterIndex;
    int selectedItemIndex = AsOpenItemMenuState(this)->selectedItem;
    TourProgressItemFlagView* progressItem =
        (TourProgressItemFlagView*)((char*)AsOpenItemMenuState(this)->tourProgress +
                                    characterMapIndex * sizeof(TourSelectionFields) +
                                    selectedItemIndex * sizeof(unsigned short));
    // Bit 4 = numeric rewards already counted. tour_main_task.cpp's
    // TourMap_CommitNodeCompletionAndRewards adds SharedStringState word to
    // player's pendingReward and short to tour coinCount, then sets 0x10 in
    // the same map/item halfword and publishes SaveData_SetModeRecords.
    // Clear: publish first-completion +56/+58 rewards; set: publish zeros.
    // Party branch instead divides tier rewards by five when this is set
    // (minigame_aim_and_party_tiers.cpp), so repeats still award reduced party rewards. Bit 9 marks
    // inventory reward application separately; don't clear both to replay a node.
    int useRecordEncounterValues = !((progressItem->flags >> kNumericRewardCountedBit) & 1);
    // Phase 2: party config may consume setup; otherwise create the opponent and hole tables.
    int partyConfigHandledSetup = TourPartyConfig_ApplyToSharedDoc(record, useRecordEncounterValues);
    if (partyConfigHandledSetup) {
        ((CEnemyRenderRecord*)playerRenderRecords)->caddieInventoryIndex = -1; // original +0xC
        ((CEnemyRenderTable*)playerRenderRecords)->count = 1;                  // original +0x290
        GolfPlayerList_BuildFromRenderTable(playerRenderRecords);
    } else if (AsOpenItemRecord(record)->golfRuleType != -1) {
        if (!Sing_805DC678()) {
            if (!s_pangyaEnemyDataInitialized) {
                ((CPangyaEnemyData*)s_pangyaEnemyData)->Init();
                __register_global_object(s_pangyaEnemyData, (void (*)())__dt__16CPangyaEnemyDataFv,
                                         s_pangyaEnemyDataDtorRegistration_TourItemMenu);
                s_pangyaEnemyDataInitialized = 1;
            }
            Sing_805DC678() = s_pangyaEnemyData;
        }
        EnemyData_BuildCharacterSlot(Sing_805DC678(), (char*)playerRenderRecords + sizeof(CEnemyRenderRecord),
                                     AsOpenItemRecord(record)->enemyRecord, 0);
        ((CEnemyRenderTable*)playerRenderRecords)->count = 2; // original +0x290
        GolfPlayerList_BuildFromRenderTable(playerRenderRecords);
        (*(int*)&AsCSharedDoc(g_sharedDoc)->enemyLevelIndex) =
            (signed char)*(unsigned char*)((char*)playerRenderRecords + sizeof(CEnemyRenderRecord) +
                                           (int)&((TourEnemyPrefix*)0)->levelIndex);
        int configuredHoleCount = 0;
        for (int holeIndex = 0; holeIndex < kTourHoleLimit; ++holeIndex) {
            ((TourRoundHoleFields*)g_sharedDoc)->courseIds[holeIndex] = 0;
            ((TourRoundHoleFields*)g_sharedDoc)->holeNumbers[holeIndex] = 0;
            ((TourRoundHoleFields*)g_sharedDoc)->pinIndices[holeIndex] = 0;
            signed char holeNumber =
                *(signed char*)((char*)record + (int)&((TourRewardFields*)0)->holeNumbers + holeIndex);
            if (holeNumber) {
                ((TourRoundHoleFields*)g_sharedDoc)->courseIds[holeIndex] = AsOpenItemRecord(record)->courseId;
                ((TourRoundHoleFields*)g_sharedDoc)->holeNumbers[holeIndex] = holeNumber;
                int pinDraw = rand();
                ((TourRoundHoleFields*)g_sharedDoc)->pinIndices[holeIndex] = pinDraw % kTourPinPositionCount;
                ++configuredHoleCount;
            }
        }
        AsCSharedDoc(g_sharedDoc)->roundHoleCount = configuredHoleCount;
        SharedDoc_SelectCourse(AsOpenItemRecord(record)->courseId);
        // Record +43 -> document +9C6: weather code; consumer domains above.
        // First-completion reward fields below retain their short/word storage.
        ((TourEncounterWeatherFields*)g_sharedDoc)->selectedWeather = AsOpenItemRecord(record)->weatherCode;
        short golfRuleIndex = AsOpenItemRecord(record)->golfRuleType;
        AsCSharedDoc(g_sharedDoc)->golfRuleType = golfRuleIndex;
        if ((unsigned char)golfRuleIndex < 3)
            strcpy(((TourRuleNameFields*)g_sharedDoc)->ruleName,
                   (char*)((TourRuleNames*)g_sharedDoc)->rules[(unsigned char)golfRuleIndex].name);
        AsCSharedDoc(g_sharedDoc)->courseRecordUpdatesEnabled = 0;
        (*(int*)&AsCSharedDoc(g_sharedDoc)->turnTimeLimitMs) = 0;
        AsCSharedDoc(g_sharedDoc)->unk_9CC = 0;
    }
    // Phase 3: resolve pooled event text, publish coin/string values, then discard old render objects.
    void* menuContext = AsOpenItemMenuState(this)->menuContext;
    if (AsOpenItemMenuContext(menuContext)->useDefaultNames) {
        SharedStringState_SetFirstStrings(0, (char*)tourMapOpenItemStrings + kTourText_EmptyName,
                                          (char*)tourMapOpenItemStrings + kTourText_EmptyName, 0);
        SharedStringState_SetSecondStrings(0, (char*)tourMapOpenItemStrings + kTourText_EmptyName,
                                           (char*)tourMapOpenItemStrings + kTourText_EmptyName);
    } else {
        /* 0x802F3BE8-0x802F3BF4 (also 0x802F3C1C/0x802F3C54/0x802F3C7C): OffsetStringNode_GetText gets Table_ResolveDataOffset's result in r3 and returns it + 8 */
        void* secondaryText = Table_ResolveDataOffset(*(void**)(*(void**)&AsOpenItemMenuState(this)->mapData),
                                                      AsOpenItemRecord(record)->firstSecondaryStringOffset);
        if (secondaryText)
            secondaryText = OffsetStringNode_GetText(secondaryText);
        else
            secondaryText = 0;
        void* primaryText = Table_ResolveDataOffset(*(void**)(*(void**)&AsOpenItemMenuState(this)->mapData),
                                                    AsOpenItemRecord(record)->firstPrimaryStringOffset);
        if (primaryText)
            primaryText = OffsetStringNode_GetText(primaryText);
        else
            primaryText = 0;
        SharedStringState_SetFirstStrings(0, primaryText, secondaryText, 0);
        secondaryText = Table_ResolveDataOffset(*(void**)(*(void**)&AsOpenItemMenuState(this)->mapData),
                                                AsOpenItemRecord(record)->secondSecondaryStringOffset);
        if (secondaryText)
            secondaryText = OffsetStringNode_GetText(secondaryText);
        else
            secondaryText = 0;
        primaryText = Table_ResolveDataOffset(*(void**)(*(void**)&AsOpenItemMenuState(this)->mapData),
                                              AsOpenItemRecord(record)->secondPrimaryStringOffset);
        if (primaryText)
            primaryText = OffsetStringNode_GetText(primaryText);
        else
            primaryText = 0;
        SharedStringState_SetSecondStrings(0, primaryText, secondaryText);
    }
    int eventShortValue = useRecordEncounterValues ? AsOpenItemRecord(record)->firstCompletionTourCoins : 0;
    SharedStringState_SetWordValue(useRecordEncounterValues ? AsOpenItemRecord(record)->firstCompletionPlayerReward
                                                            : 0);
    SharedStringState_SetShortValue((unsigned short)eventShortValue);
    SharedStringState_SetContext(0);
    CCharacterPreviewTable::DestroyRenderObjects(playerRenderRecords);
    return 1;
}

// Queue unclaimed inventory rewards for the selected map item; indices are not IFF type IDs.
void OpenItemMenu_QueueSelectedItemRewards(void* self, void* data) {
    void* d;
    short index;
    void* base;
    unsigned short flags;
    int i;
    void* obj;
    short* p;
    d = data;
    index = AsOpenItemMenuState(self)->selectedItem;
    base = AsOpenItemMenuState(self)->selectionRecord;
    flags = ((TourSelectionFields*)base)->items[index];
    if (((flags >> 9) & 1) == 0) {
        if (Sing_805DC958() == 0) {
            if (s_itemRequestStateInitialized == 0) {
                ItemRequestState_Ctor(&s_itemRequestState);
                __register_global_object(&s_itemRequestState, ItemRequestState_Dtor,
                                         &s_itemRequestStateDtorRegistration_TourItemMenu);
                s_itemRequestStateInitialized = 1;
            }
            Sing_805DC958() = &s_itemRequestState;
        }
        obj = Sing_805DC958();
        p = (short*)((TourRewardFields*)d)->setItems;
        i = 0;
        for (; i < 4; ++i, p += 2)
            ItemRequestState_RequestSetItemByInventoryIndex(obj, 0, p[0], p[1]);
        i = 0;
        p = (short*)((TourRewardFields*)d)->parts;
        for (; i < 4; ++i, p += 2)
            ItemRequestState_RequestPartByInventoryIndex(obj, 0, p[0], p[1]);
        ItemRequestState_RequestClubSetByInventoryIndex(obj, 0, AsOpenItemRecord(d)->rewardClubSetIndex,
                                                        AsOpenItemRecord(d)->rewardClubSetCount);
        ItemRequestState_RequestBallByInventoryIndex(obj, 0, AsOpenItemRecord(d)->rewardBallIndex,
                                                     AsOpenItemRecord(d)->rewardBallCount);
        ItemRequestState_RequestCharacterByInventoryIndex(obj, 0, AsOpenItemRecord(d)->rewardCharacterIndex,
                                                          AsOpenItemRecord(d)->rewardCharacterCount);
    }
}

// Return success only after all map, window, character, and layout resources are ready.
int OpenItemMenu_PollResourcesLoaded(void* self) {
    if (!CameraController_PollPreview(AsOpenItemMenuState(self)->cameraController))
        return 0;
    if (!SharedFilePtr_HasLoadedBuffer(AsOpenItemMenuState(self)->mapFile))
        return 0;
    if (!((CSharedFileHandleOwner*)(AsOpenItemMenuState(self)->messageFile))->IsLoaded())
        return 0;
    if (Sing_805DC670() == 0) {
        if (s_bpPlayerInitialized == 0) {
            ((CBpPlayer*)&s_bpPlayer)->__ct();
            __register_global_object(&s_bpPlayer, __dt__9CBpPlayerFv, &s_bpPlayerDtorRegistration_TourItemMenu);
            s_bpPlayerInitialized = 1;
        }
        Sing_805DC670() = &s_bpPlayer;
    }
    if (!EventText_PollMessageResources(Sing_805DC670()))
        return 0;
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    /* 0x802EC768-0x802EC76C: the looked-up object stays in r3 for BpObject_PollLoad */
    if (!BpObject_PollLoad(IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->modeGroup)))
        return 0;
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    /* 0x802EC7D8-0x802EC7DC: the looked-up object stays in r3 for BpObject_PollLoad */
    if (!BpObject_PollLoad(IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->itemGroup)))
        return 0;
    if (!SharedFilePtr_HasLoadedBuffer(AsOpenItemMenuState(self)->itemFile))
        return 0;
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    /* 0x802EC860-0x802EC864: the looked-up object stays in r3 for BpObject_PollLoad */
    if (!BpObject_PollLoad(IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->numberGroup)))
        return 0;
    if (!((CTourWindow*)(AsOpenItemMenuState(self)->tourWindow))->IsLoadComplete())
        return 0;
    if (!((CTourHintWindow*)(AsOpenItemMenuState(self)->tourHintWindow))->AreResourcesLoaded())
        return 0;
    if (!((CTourVSWindow*)(AsOpenItemMenuState(self)->tourVsWindow))->IsReady())
        return 0;
    if (!TourLoadEffect_AreTexturesReady(AsOpenItemMenuState(self)->particleEffects))
        return 0;
    Layout_UpdateExtended(AsOpenItemMenuState(self)->layout);
    return Layout_PollLoadComplete(AsOpenItemMenuState(self)->layout) != 0;
}

// Free the three event-result vectors, then optionally free their owning object.
void* TourResultVectors_dtor(void* p, int shouldDelete) {
    if (p != 0) {
        volatile unsigned int* fields = (volatile unsigned int*)p;
        if ((void*)&((TourResultVectors*)p)->third != 0 && (void*)&((TourResultVectors*)p)->third != 0 &&
            (void*)&((TourResultVectors*)p)->third != 0 && (void*)&((TourResultVectors*)p)->third != 0) {
            unsigned int* fp = (unsigned int*)p;
            unsigned int _v = fp[6];
            if (_v != 0) {
                unsigned int* fp2 = (unsigned int*)p;
                fp2[7] -= fp2[7];
                appFreeUni__FPv((void*)_v);
            }
        }
        if ((void*)&((TourResultVectors*)p)->second != 0 && (void*)&((TourResultVectors*)p)->second != 0 &&
            (void*)&((TourResultVectors*)p)->second != 0 && (void*)&((TourResultVectors*)p)->second != 0) {
            unsigned int* fp = (unsigned int*)p;
            unsigned int _v = fp[3];
            if (_v != 0) {
                unsigned int _t = fp[4];
                fp[4] = _t - _t;
                appFreeUni__FPv((void*)_v);
            }
        }
        if ((void*)&((TourResultVectors*)p)->first != 0 && (void*)&((TourResultVectors*)p)->first != 0 &&
            (void*)&((TourResultVectors*)p)->first != 0 && (void*)&((TourResultVectors*)p)->first != 0) {
            unsigned int* fp = (unsigned int*)p;
            unsigned int _v = fp[0];
            if (_v != 0) {
                unsigned int _t = fp[1];
                fp[1] = _t - _t;
                appFreeUni__FPv((void*)_v);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(p);
    }
    return p;
}

// Queue the equipment bundle unlocked by a tour coin threshold.
void OpenItemMenu_QueueEquipmentRecordRequests(void*, void* data) {
    if (Sing_805DC958() == 0) {
        if (s_itemRequestStateInitialized == 0) {
            ItemRequestState_Ctor(&s_itemRequestState);
            __register_global_object(&s_itemRequestState, ItemRequestState_Dtor,
                                     &s_itemRequestStateDtorRegistration_TourItemMenu);
            s_itemRequestStateInitialized = 1;
        }
        Sing_805DC958() = &s_itemRequestState;
    }
    short* rewardPair = (short*)((CoinEquipmentFields*)data)->setItems;
    int i = 0;
    void* obj = Sing_805DC958();
    for (; i < 8; ++i) {
        ItemRequestState_RequestSetItemByInventoryIndex(obj, 1, rewardPair[0], rewardPair[1]);
        rewardPair += 2;
    }
    short* values = (short*)data;
    ItemRequestState_RequestClubSetByInventoryIndex(obj, 1, values[1], values[2]);
    ItemRequestState_RequestBallByInventoryIndex(obj, 1, values[3], values[4]);
    ItemRequestState_RequestCaddieByInventoryIndex(obj, 1, values[5], values[6]);
    int j = 0;
    rewardPair = (short*)((CoinEquipmentFields*)data)->caddieItems;
    for (; j < 3; ++j) {
        ItemRequestState_RequestCaddieItemByInventoryIndex(obj, 1, rewardPair[0], rewardPair[1]);
        rewardPair += 2;
    }
}

// Keep the map viewport within its bounds while following the preview camera.
void OpenItemMenuState_UpdateCameraOffsets(void* self) {
    float pos[2];
    unsigned int bounds[6];
    bounds[4] = kOpenItemCameraBound4;
    bounds[5] = kOpenItemCameraBound5;
    bounds[2] = kOpenItemCameraBound2;
    bounds[3] = kOpenItemCameraBound3;
    bounds[0] = kOpenItemCameraBound0;
    bounds[1] = kOpenItemCameraBound1;
    ((CCameraController*)(AsOpenItemMenuState(self)->cameraController))->GetPosition(pos);
    // bounds[0,1] = viewport extent; [2,3] = map extent; [4,5] = tracking
    // margins, all in map UI units. Clamp at map edges; otherwise move only
    // when the camera leaves the interior tracking band, X then Y.
    float f1 = *(float*)&bounds[2], f2 = *(float*)&bounds[4], f3 = pos[0];
    float delta = f1 - f2;
    if (f3 > delta)
        AsOpenItemMenuState(self)->cameraOffsetX = f1 - *(float*)&bounds[0];
    else if (f3 < f2)
        AsOpenItemMenuState(self)->cameraOffsetX = kFloat_0_OpenItemMenu;
    else {
        float d = f3 - AsOpenItemMenuState(self)->cameraOffsetX;
        float range = *(float*)&bounds[0] - f2;
        if (d > range)
            AsOpenItemMenuState(self)->cameraOffsetX = f3 - range;
        else if (d < f2)
            AsOpenItemMenuState(self)->cameraOffsetX = f3 - f2;
    }
    f1 = *(float*)&bounds[3];
    f2 = *(float*)&bounds[5];
    f3 = pos[1];
    delta = f1 - f2;
    if (f3 > delta)
        AsOpenItemMenuState(self)->cameraOffsetY = f1 - *(float*)&bounds[1];
    else if (f3 < f2)
        AsOpenItemMenuState(self)->cameraOffsetY = kFloat_0_OpenItemMenu;
    else {
        float d = f3 - AsOpenItemMenuState(self)->cameraOffsetY;
        float range = *(float*)&bounds[1] - f2;
        if (d > range)
            AsOpenItemMenuState(self)->cameraOffsetY = f3 - range;
        else if (d < f2)
            AsOpenItemMenuState(self)->cameraOffsetY = f3 - f2;
    }
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_31() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_30() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_29() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_28() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_27() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_26() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_25() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_24() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_23() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_22() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_21() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_20() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_19() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_18() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_17() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_16() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_15() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_14() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_13() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_12() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_11() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_10() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_9() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_8() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_7() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_6() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_5() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_4() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_3() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_2() {
    return 1;
}

// This coin-table interface slot reports success without additional work.
int CCoinOpenItemData::vfunc_1() {
    return 1;
}

// Destroy the coin-reward table wrapper; the loaded table is owned elsewhere.
CCoinOpenItemData::~CCoinOpenItemData() {}

// Destroy the map table wrapper; the loaded table is owned elsewhere.
void* CMapData::vfunc_0(void* ptr, int shouldDelete) {
    if (ptr != 0 && shouldDelete > 0) {
        __dl__FPv(ptr);
    }
    return ptr;
}

struct F263C_MapPair {
    int key;
    void* value;
};
struct F263C_Flags {
    CW_BITS3(unsigned short pad : 11, unsigned short bit : 1, unsigned short rest : 4)
};
struct F263C_Self {
    void* mapFile;
    void* itemFile;
    void* messageFile;
    void* cameraController;
    char map[0x10];
    void* tourHintWindow;
    void* tourVsWindow;
    void* particleEffects;
    void* messageResourceHolder;
    void* flashMemorySaveMenu;
    void* mapData;
    void* coinItemData;
    void* preview;
    void* shop;
    void* numberDraw;
    void* menuContext;
    void* progress;
};
// Find a map item pair, inserting the default value when its key is absent.
static inline F263C_MapPair* FindOrInsertTourItemPair(void* self, const int& k) {
    int* node;
    unsigned char a, b;
    void* it = IntegerKeyTree_FindInsertionPosition((void*)((int)self + 0x10), (int*)&k, (int*)&node, &a, &b);
    F263C_MapPair* pair;
    if (node == 0 || (pair = (F263C_MapPair*)((unsigned int)node + 0xc), node[3] < k)) {
        F263C_MapPair value;
        value.key = k;
        value.value = g_bpEntryDefaultMappedValue;
        pair =
            (F263C_MapPair*)((char*)BpEntryTree_InsertNode((void*)((unsigned int)self + 0x10), it, a, b, (int*)&value) +
                             0xc);
    }
    return pair;
}
// Return the map item for a key, creating a default tree entry if necessary.
static inline void* FindOrInsertTourItem(void* self, const int& k) {
    return FindOrInsertTourItemPair(self, k)->value;
}
extern signed char s_tourVsWindowInitialized;
extern signed char s_tourHintWindowInitialized;
extern char s_tourVSWindow[], s_tourHintWindow[];
void Destroy__13CTourVSWindowFi();
void CTourHintWindow_dtor();
// Acquire the shared tour versus window, constructing it on first use.
static inline void* GetTourVersusWindow(char* base) {
    if (!Sing_805DC690()) {
        if (!s_tourVsWindowInitialized) {
            ((CTourVSWindow*)s_tourVSWindow)->__ct();
            __register_global_object(s_tourVSWindow, Destroy__13CTourVSWindowFi, (base + 0x78));
            s_tourVsWindowInitialized = 1;
        }
        Sing_805DC690() = s_tourVSWindow;
    }
    return Sing_805DC690();
}
// Acquire the shared tour hint window, constructing it on first use.
static inline void* GetTourHintWindow(char* base) {
    if (!Singleton_TourHintWindow()) {
        if (!s_tourHintWindowInitialized) {
            ((CTourHintWindow*)s_tourHintWindow)->__ct();
            __register_global_object(s_tourHintWindow, CTourHintWindow_dtor, (base + 0x6c));
            s_tourHintWindowInitialized = 1;
        }
        Singleton_TourHintWindow() = s_tourHintWindow;
    }
    return Singleton_TourHintWindow();
}
struct F263C_Pair {
    short key;
    short pad;
    int value;
};
struct F263C_Vector {
    F263C_Pair* data;
    unsigned int size;
    int capacity;
    // Start an empty owned vector of (item key, replacement value) script edits.
    F263C_Vector() : data(0), size(0), capacity(0) {}
    // Destroy vector storage, not just logical contents; used only during result cleanup.
    void clear() {
        unsigned int* fp = (unsigned int*)this;
        unsigned int p = fp[0];
        if (p) {
            unsigned int t = *(volatile unsigned int*)(fp + 1);
            fp[1] = t - t;
            appFreeUni__FPv((void*)p);
        }
    }
};
struct F263C_Results {
    F263C_Vector flag14Edits, unconsumedEdits, flag10Edits;
    short initialSelectionOverride;
    signed char requestedCharacterMap;
    short targetItemKey;
    int continueRequested;
    void* previewSlot;
    void* eventContext;
    // Initialize script outputs to absent selection/map/key and continue enabled.
    F263C_Results(void* p)
        : initialSelectionOverride(-1), requestedCharacterMap(-1), targetItemKey(-1), continueRequested(1) {
        eventContext = 0;
        previewSlot = p;
    }
    // Free all three script output vectors in reverse construction order.
    ~F263C_Results() {
        flag10Edits.clear();
        unconsumedEdits.clear();
        flag14Edits.clear();
    }
};
// Resolve a named item or connection group for a map node and visual column.
static inline void* F263C_Object(void* self, short key, int col, char* base) {
    F0BTable table = s_tourMapItemGroupNames;
    const char* name = table.names[key][col];
    if (!(signed char)*name)
        return 0;
    void* handle = AsOpenItemMenuState(self)->itemGroup;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, (base + 0xc));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* manager = IdMap_Lookup(Sing_805DC5F0(), handle);
    void* result;
    if (manager == 0)
        result = 0;
    else {
        result = BpObject_GetOrCreateNamedGroup(manager, name);
    }
    return result;
}
struct F263C_Marker {
    virtual void f0();
    virtual void f4();
    virtual void update(float);
};
// Run one script step, apply its map edits and effects, and update hint/versus windows.
// Returns TourScriptResult; a change-map result skips the remaining current-map work.
// self/context/effect resources required, item keys 0..34 and requested map
// -1(no change)/0..10. Script result vectors own their buffers and release on
// return; item pointers and table strings are borrowed. A missing key may
// INSERT a default node, so these queries are not read-only tree lookups.
// Fade clocks are script-populated context counts, not dt here. linkFrames=0
// starts the trail, >20 adds 6 alpha/call (clamp 255); itemFrames=0 starts
// burst at 255, >8 subtracts 32/call (clamp 0). Effect completion controls
// event transitions independently of alpha. Preserve strict boundaries and
// repeated zero-count behavior; a 60fps mod must coordinate clock producers.
int CTourMain::ProcessSelectedMapItem() {
    int TourScript_RunWithResultContext(void*, void*);
    void FadeOverlay_Init(void*);
    void ResourceState_Dtor();
    void FadeState_RequestFramebufferCapture(void*);
    void FadeState_StartFadeIn(void*, int);
    extern signed char s_fadeOverlayInitialized;
    extern char s_fadeOverlay[];
    extern char s_itemMenuSingletonRegistration_8051D0E0[];
    // Phase 1: run script and capture edits; update any active scripted preview.
    F263C_Results result(&((F263C_Self*)this)->preview);
    result.eventContext = AsOpenItemMenuState(this)->menuContext;
    char* base = s_itemMenuSingletonRegistration_8051D0E0;
    int scriptResult = kTourScriptRunning;
    if (TourScript_RunWithResultContext(AsOpenItemMenuState(this)->messageFile, &result))
        scriptResult = result.continueRequested ? kTourScriptContinue : kTourScriptCancel;
    F263C_Marker* marker = *(F263C_Marker**)((unsigned int)this + IFF_OFFSET(F263C_Self, preview));
    if (marker) {
        marker->update(kFloat_0_OpenItemMenu);
    }
    AsOpenItemMenuState(this)->initialSelection = (result.initialSelectionOverride == -1)
                                                      ? AsOpenItemMenuState(this)->initialSelection
                                                      : result.initialSelectionOverride;
    signed char requestedCharacterMap = result.requestedCharacterMap;
    short key = result.targetItemKey;
    // A requested map change persists its selection/history and returns before current-map edits.
    // Keep the direct field cast on this comparison: routing it through the
    // inline view reverses cmpw operand allocation despite equivalent C++.
    if (requestedCharacterMap >= 0 && ((OpenItemMenuState*)this)->tourCharacterIndex != requestedCharacterMap) {
        if (!Sing_805DC608()) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, (base + 0x24));
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = &s_fadeOverlay;
        }
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        if (!Sing_805DC608()) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, ResourceState_Dtor, (base + 0x24));
                s_fadeOverlayInitialized = 1;
            }
            Sing_805DC608() = &s_fadeOverlay;
        }
        FadeState_StartFadeIn(Sing_805DC608(), 0);
        ((TourProgressFields*)AsOpenItemMenuState(this)->tourProgress)->maps[(int)requestedCharacterMap].selectedItem =
            key;
        ((TourProgressFields*)AsOpenItemMenuState(this)->tourProgress)
            ->mapHistory[((TourProgressFields*)AsOpenItemMenuState(this)->tourProgress)->historyIndex] =
            (unsigned char)requestedCharacterMap;
        return kTourScriptChangeMap;
    }
    if (key >= 0) {
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->SetItemEntry(FindOrInsertTourItem(this, key));
    }
    // Phase 2: apply hidden/opened flag edits; lookups preserve insertion behavior.
    for (F263C_Pair* p = result.flag14Edits.data; p != result.flag14Edits.data + result.flag14Edits.size; ++p) {
        if (p->value != OpenItemEntry_GetFlagBit14(FindOrInsertTourItem(this, p->key)))
            OpenItemEntry_SetFlagBit14(FindOrInsertTourItem(this, p->key), p->value);
    }
    int OpenItemEntry_GetFlagBit10(void*);
    void OpenItemEntry_SetFlagBit10(void*, int);
    for (F263C_Pair* p = result.flag10Edits.data; p != result.flag10Edits.data + result.flag10Edits.size; ++p) {
        if (p->value != OpenItemEntry_GetFlagBit10(FindOrInsertTourItem(this, p->key)))
            OpenItemEntry_SetFlagBit10(FindOrInsertTourItem(this, p->key), p->value);
    }
    int TourVSWindow_HasStatus(void*, int);
    void TourVSWindow_SetStatus(void*, int);
    if (TourVSWindow_HasStatus(GetTourVersusWindow(base), 1)) {
        TourVSWindow_SetStatus(GetTourVersusWindow(base), 2);
        char* info = *(char**)FindOrInsertTourItem(this, AsOpenItemMenuState(this)->selectedItem);
        short v = 0;
        if (*(short*)(info + 0x3e) != -1) {
            if (!Sing_805DC678()) {
                if (!s_pangyaEnemyDataInitialized) {
                    ((CPangyaEnemyData*)s_pangyaEnemyData)->Init();
                    __register_global_object(s_pangyaEnemyData, (void (*)())__dt__16CPangyaEnemyDataFv, (base + 0x3c));
                    s_pangyaEnemyDataInitialized = 1;
                }
                Sing_805DC678() = s_pangyaEnemyData;
            }
            v = (short)((int*)Table_GetEntry(*(void**)Sing_805DC678(), *(short*)(info + 0x3e)))[1];
        }
        ((CTourVSWindow*)GetTourVersusWindow(base))
            ->SetEntryIndices((short)AsCCameraController(AsOpenItemMenuState(this)->cameraController)->modelIndex,
                              v); // original +0x20
        AsOpenItemMenuState(this)->windowState = kTourBeginVersus;
    } else if (((CTourHintWindow*)(GetTourHintWindow(base)))->IsState(1)) {
        ((CTourHintWindow*)(GetTourHintWindow(base)))->SetState(2);
        AsOpenItemMenuState(this)->windowState = kTourBeginHint;
    }
    short OpenItemEntry_GetNeighborIndex(void*, int);
    void OpenItemEntry_GetPosition(void*, float*);
    void gtl_Vec2_Subtract(float*, float*, float*);
    float Vec2_Length(float*);
    void Particle2D_SetTrailEndpoints(void*, float*, float*);
    void TourLoadEffect_StartOpen(void*);
    int TourLoadEffect_IsTrailFinished(void*);
    void UIGroup_SetOpacity(void*, int);
    // Phase 3: reveal a connection from the nearer endpoint, then fade in its visual.
    if (((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkEvent == kTourLinkRevealEvent) {
        int code = ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkValue;
        int col = 3;
        if (code / kTourConnectionCodeRadix)
            col = 4;
        int first = code % kTourConnectionCodeRadix;
        int second = (short)OpenItemEntry_GetNeighborIndex(FindOrInsertTourItem(this, first), col == 4 ? 1 : 3);
        if (first != -1 && second != -1) {
            if ((int)((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkFrames == 0) {
                float b[2], a[2], delta[2], pos[2];
                OpenItemEntry_GetPosition(FindOrInsertTourItem(this, first), a);
                OpenItemEntry_GetPosition(FindOrInsertTourItem(this, second), b);
                ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetPosition(pos);
                gtl_Vec2_Subtract(delta, a, pos);
                float dist = Vec2_Length(delta);
                gtl_Vec2_Subtract(delta, b, pos);
                if (dist < Vec2_Length(delta))
                    Particle2D_SetTrailEndpoints(((F263C_Self*)this)->particleEffects, a, b);
                else
                    Particle2D_SetTrailEndpoints(((F263C_Self*)this)->particleEffects, b, a);
                ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkOpacity = 0;
                TourLoadEffect_StartOpen(
                    *(void**)((unsigned int)this + IFF_OFFSET(OpenItemMenuState, particleEffects)));
            }
            if (((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkFrames > kTourLinkRevealDelayUpdates) {
                int alpha =
                    ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkOpacity + kTourLinkOpacityStep;
                ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkOpacity =
                    alpha < kTourOpaqueAlpha ? alpha : kTourOpaqueAlpha;
            }
            void* obj = F263C_Object(this, (short)first, col, base);
            if (obj)
                UIGroup_SetOpacity(obj, ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkOpacity);
            if (TourLoadEffect_IsTrailFinished(((F263C_Self*)this)->particleEffects))
                ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->linkEvent = 0;
        }
    }
    void Particle2D_SetBurstOrigin(void*, float*);
    void TourLoadEffect_StartBurst(void*);
    int TourLoadEffect_IsBurstFinished(void*);
    // Reward burst starts opaque; alpha falls only when the script count is >8.
    if (((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemEvent == kTourItemBurstEvent) {
        int k = ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemValue;
        if ((int)((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemFrames == 0) {
            float pos[2];
            OpenItemEntry_GetPosition(FindOrInsertTourItem(this, k), pos);
            Particle2D_SetBurstOrigin(((F263C_Self*)this)->particleEffects, pos);
            ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemOpacity = kTourOpaqueAlpha;
            TourLoadEffect_StartBurst(((F263C_Self*)this)->particleEffects);
        }
        if (((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemFrames > kTourItemFadeDelayUpdates) {
            int alpha = ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemOpacity - kTourItemOpacityStep;
            ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemOpacity = alpha < 0 ? 0 : alpha;
        }
        void* obj = F263C_Object(this, (short)k, kTourItemRewardOverlay, base);
        if (obj)
            UIGroup_SetOpacity(obj, ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemOpacity);
        if (TourLoadEffect_IsBurstFinished(((F263C_Self*)this)->particleEffects)) {
            OpenItemEntry_SetFlagBit13(FindOrInsertTourItem(this, k), 0);
            ((TourEventFields*)AsOpenItemMenuState(this)->menuContext)->itemEvent = kTourItemBurstFinishedEvent;
        }
    }
    void TourVSWindow_ProcessLayoutCallback(void*);
    void TourHint_ShowFormattedText(void*, void*, int);
    // Phase 4: begin/process versus or hint windows; begin cases intentionally fall through.
    switch (AsOpenItemMenuState(this)->windowState) {
    case kTourBeginVersus:
        AsOpenItemMenuState(this)->windowState = kTourProcessVersus;
    case kTourProcessVersus:
        TourVSWindow_ProcessLayoutCallback(AsOpenItemMenuState(this)->tourVsWindow);
        if ((signed char)scriptResult || TourVSWindow_HasStatus(GetTourVersusWindow(base), 3)) {
            TourVSWindow_SetStatus(GetTourVersusWindow(base), 0);
            AsOpenItemMenuState(this)->windowState = kTourNoWindow;
        }
        break;
    case kTourBeginHint: {
        void* item = FindOrInsertTourItem(this, AsOpenItemMenuState(this)->selectedItem);
        F263C_Flags* flags = (F263C_Flags*)((char*)AsOpenItemMenuState(this)->tourProgress +
                                            AsOpenItemMenuState(this)->tourCharacterIndex * kTourSelectionBytes +
                                            AsOpenItemMenuState(this)->selectedItem * 2 + 4);
        TourHint_ShowFormattedText(AsOpenItemMenuState(this)->tourHintWindow, *(void**)item, !flags->bit);
        AsOpenItemMenuState(this)->windowState = kTourProcessHint;
    }
    case kTourProcessHint:
        ((CTourHintWindow*)(AsOpenItemMenuState(this)->tourHintWindow))->UpdateNextIndicatorAndCallback();
        if ((signed char)scriptResult || ((CTourHintWindow*)(GetTourHintWindow(base)))->IsState(3)) {
            char* info = *(char**)FindOrInsertTourItem(this, AsOpenItemMenuState(this)->selectedItem);
            ((CTourHintWindow*)(GetTourHintWindow(base)))->SetState(0);
            if ((int)*(unsigned char*)(info + 0x28) == 1)
                ((TourProgressFields*)AsOpenItemMenuState(this)->tourProgress)->completedEventFlags |= 0x40;
            if ((int)*(unsigned char*)(info + 0x28) == 2)
                *(unsigned char*)((char*)AsOpenItemMenuState(this)->tourProgress + 0x33a) |= 0x80;
            AsOpenItemMenuState(this)->windowState = kTourNoWindow;
        }
        break;
    case kTourNoWindow:
        break;
    }
#undef F263C_PTR
#undef F263C_EVENT
    return scriptResult;
}

// Allocate all indexed map items, then bind persistent flags, neighbors, visuals and previews.
void OpenItemMenu_BuildEntries(void* self) {
    /* 0x802F0730-0x802F0800: while loop (count checked first at 0x802F07E0); CMapData::GetItemIndexAt gets *(self+0x34) (r3 from 0x802F07E0), not self */
    for (int i = 0;; ++i) {
        {
            void* first = *AsOpenItemMenuState(self)->mapData; // original +0x34
            if (i >= (first ? ((TourTableHeader*)first)->recordCount : 0))
                break;
        }
        short index = ((CMapData*)(AsOpenItemMenuState(self)->mapData))->GetItemIndexAt(i); // original +0x34
        if (index >= 0) {
            void* item = __nw__FUl(0x74);
            if (item) {
                void* base = AsOpenItemMenuState(self)->selectionRecord;
                TourBossPreview_InitState(item, (char*)base + index * 2 + 4);
            }
            int entryIndex = index;
            void* node = 0;
            unsigned char a = 0, b = 0;
            void* value = IntegerKeyTree_FindInsertionPosition((void*)&((TourTreeFields*)self)->items, &entryIndex,
                                                               (int*)&node, &a, &b);
            int* tail = node ? (int*)((void*)&((TourTreeNodeFields*)node)->key) : 0;
            if (!tail || *tail < entryIndex) {
                int pair[2] = {entryIndex, (int)g_bpEntryDefaultMappedValue};
                node = BpEntryTree_InsertNode((void*)&((TourTreeFields*)self)->items, value, a, b, pair);
                tail = (int*)((void*)&((TourTreeNodeFields*)node)->key);
            }
            AsOpenItemTreePair(tail)->item = item;
        }
    }
    // All items now exist: bind neighbor flags/records and place their visual/model previews.
    struct Entry {
        void* names[5];
    };
    const Entry* entries = (const Entry*)&s_tourMapItemGroupNames;
    /* 0x802F0834/0x802F0B24-0x802F0B44: while loop again, same list argument */
    for (int j = 0;; ++j) {
        {
            void* first = *AsOpenItemMenuState(self)->mapData; // original +0x34
            if (j >= (first ? ((TourTableHeader*)first)->recordCount : 0))
                break;
        }
        short index = ((CMapData*)(AsOpenItemMenuState(self)->mapData))->GetItemIndexAt(j); // original +0x34
        if (index >= 0) {
            int entryIndex = index;
            void* node = 0;
            unsigned char a = 0, b = 0;
            void* value = IntegerKeyTree_FindInsertionPosition((void*)&((TourTreeFields*)self)->items, &entryIndex,
                                                               (int*)&node, &a, &b);
            int* tail = node ? (int*)((void*)&((TourTreeNodeFields*)node)->key) : 0;
            if (!tail || entryIndex > *tail) {
                int pair[2] = {entryIndex, (int)g_bpEntryDefaultMappedValue};
                node = BpEntryTree_InsertNode((void*)&((TourTreeFields*)self)->items, value, a, b, pair);
                tail = (int*)((void*)&((TourTreeNodeFields*)node)->key);
            }
            void* object = AsOpenItemTreePair(tail)->item;
            void* base = AsOpenItemMenuState(self)->selectionRecord;
            if (((*(unsigned char*)base >> 5) & 1) == 0) {
                OpenItemEntry_SetFlagBit15(object, 0);
                void* list2 = AsOpenItemMenuState(self)->mapData; // original +0x34
                short v0 = ((CMapData*)list2)->GetDirection0StyleAt(j);
                short v1 = ((CMapData*)list2)->GetDirection1StyleAt(j);
                short v2 = ((CMapData*)list2)->GetDirection2StyleAt(j);
                short v3 = ((CMapData*)list2)->GetDirection3StyleAt(j);
                OpenItemEntry_SetDirectionEnabled(object, 0, v0 != -1);
                OpenItemEntry_SetDirectionEnabled(object, 1, v1 != -1);
                OpenItemEntry_SetDirectionEnabled(object, 2, v2 != -1);
                OpenItemEntry_SetDirectionEnabled(object, 3, v3 != -1);
                signed char v4 = ((CMapData*)list2)->GetItemFlag13At(j);
                if (v4)
                    OpenItemEntry_SetFlagBit13(object, 1);
                else
                    OpenItemEntry_SetFlagBit13(object, 0);
                signed char v5 = ((CMapData*)list2)->GetItemFlag14At(j);
                if (v5)
                    OpenItemEntry_SetFlagBit14(object, 1);
                else
                    OpenItemEntry_SetFlagBit14(object, 0);
                ((TourSelectionFields*)base)->items[index] |= kMapItemEnabled;
                if (j == 0)
                    AsOpenItemMenuState(self)->initialSelection = index;
            }
            // global init + table lookup + BpObject_GetOrCreateNamedGroup
            const Entry* e = &entries[index];
            if (((signed char)*((char*)e->names[0])) != 0) {
                if (!Sing_805DC5F0())
                    EnsureBpCtrl();
                void* mgr = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->itemGroup);
                if (mgr) {
                    void* node2 = BpObject_GetOrCreateNamedGroup(mgr, (const char*)e->names[0]);
                    float p[2];
                    UIGroup_GetPosition(node2, p);
                    p[0] += kOpenItemLabelOffsetX;
                    p[1] += kOpenItemLabelOffsetY;
                    OpenItemEntry_SetPosition(object, p);
                }
            }
            /* 0x802F0AF4-0x802F0B1C: runs for every valid index, after the position, whatever bit 5 of *(self+0x50) */
            AsOpenItemEntry(object)->index = index;
            OpenItemEntry_SetRecord(object, Table_GetEntry(*AsOpenItemMenuState(self)->mapData, j)); // original +0x34
            TourBossPreview_LoadModel(object);
        }
    }
}

// Look up a node visual by item index and column; an empty group name has no visual.
static inline void* F0BObject(void* self, short index, int column) {
    F0BTable table = *(const F0BTable*)&s_tourMapItemGroupNames;
    const char* name = table.names[index][column];
    if (!(signed char)*name)
        return 0;
    void* handle = AsOpenItemMenuState(self)->itemGroup;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* manager = IdMap_Lookup(Sing_805DC5F0(), handle);
    void* result;
    if (manager == 0)
        result = 0;
    else {
        result = BpObject_GetOrCreateNamedGroup(manager, name);
    }
    return result;
}
struct F0BPair {
    int key;
    void* value;
};
// Find or insert a map node pair for the requested integer key.
static inline F0BPair* F0BLookup(void* map, int key) {
    void* node;
    unsigned char a, b;
    int k = key;
    void* it = IntegerKeyTree_FindInsertionPosition(map, &k, (int*)&node, &a, &b);
    F0BPair* pair;
    if (node == 0 || (pair = (F0BPair*)((void*)&((TourTreeNodeFields*)node)->key), ((int*)node)[3] < k)) {
        F0BPair value = {k, g_bpEntryDefaultMappedValue};
        pair = (F0BPair*)((char*)BpEntryTree_InsertNode(map, it, a, b, (int*)&value) + 0xc);
    }
    return pair;
}
// Return the item object stored in the map tree for a key.
static inline void* F0BItem(void* map, int key) {
    return F0BLookup(map, key)->value;
}
// Refresh each node icon/reward overlay and both connection visuals from persistent flags.
void OpenItemMenu_UpdateEntryVisuals(void* self) {
    short OpenItemEntry_GetNeighborIndex(void*, int);
    int OpenItemEntry_GetFlagBit13(void*);
    void UIGroup_GetSourcePosition(void*, float*);
    void UIGroup_SetOpacity(void*, int);
    extern unsigned char s_tourMapIconAtlasRows[];
    struct Node {
        Node* next;
        Node* prev;
        unsigned int flags;
        unsigned int value;
    };
    struct Entry {
        const char* names[5];
    };
    Node* head = (Node*)((void*)&((TourTreeFields*)self)->items.root);
    Node* node = (Node*)AsOpenItemMenuState(self)->firstItemNode;
    for (; node != head;) {
        void* item = AsOpenItemTreeNode(node)->item;
        void* info = AsOpenItemEntry(item)->record;
        // Flag 14 hides the icon and connections; flag 13 shows the reward
        // overlay; flag 15 marks opened nodes and selects the lit link row.
        if (item) {
            short index = AsOpenItemEntry(item)->index;
            void* obj = F0BObject(self, index, kTourItemIcon);
            if (obj) {
                int p[2];
                p[0] = p[1] = 0;
                ((float*)p)[1] = (float)(((TourMapRecordPrefix*)info)->iconKind << kTourIconRowShift);
                UIGroup_SetSourcePosition(obj, (float*)p);
                if (OpenItemEntry_GetFlagBit14(item))
                    UIGroup_SetEnabled(obj, 0);
                else
                    UIGroup_SetEnabled(obj, 1);
                if (OpenItemEntry_GetFlagBit15(item) || (int)(unsigned char)((TourMapRecordPrefix*)info)->iconKind == 4)
                    UIGroup_SetTintColor(obj, -1);
                else
                    UIGroup_SetTintColor(obj, 0xff404040);
            }
            obj = F0BObject(self, index, kTourItemCompanion);
            if (obj) {
                if (OpenItemEntry_GetFlagBit14(item))
                    UIGroup_SetEnabled(obj, 0);
                else
                    UIGroup_SetEnabled(obj, 1);
            }
            obj = F0BObject(self, index, kTourItemRewardOverlay);
            if (obj) {
                if (OpenItemEntry_GetFlagBit13(item) && !OpenItemEntry_GetFlagBit14(item))
                    UIGroup_SetEnabled(obj, 1);
                else
                    UIGroup_SetEnabled(obj, 0);
            }
            // Both endpoints must be visible; either opened endpoint lights the connection.
            short link = OpenItemEntry_GetNeighborIndex(item, 3);
            if (link != -1) {
                obj = F0BObject(self, index, kTourDirection3Link);
                if (obj) {
                    bool hidden = OpenItemEntry_GetFlagBit14(item) != 0;
                    if (!hidden)
                        hidden = OpenItemEntry_GetFlagBit14(F0BItem((void*)&((TourTreeFields*)self)->items, link)) != 0;
                    if (hidden)
                        UIGroup_SetEnabled(obj, 0);
                    else
                        UIGroup_SetEnabled(obj, 1);
                    bool active = OpenItemEntry_GetFlagBit15(item) != 0;
                    if (!active)
                        active = OpenItemEntry_GetFlagBit15(F0BItem((void*)&((TourTreeFields*)self)->items, link)) != 0;
                    if (active) {
                        float p[2];
                        UIGroup_GetSourcePosition(obj, p);
                        p[1] = (float)(s_tourMapIconAtlasRows[AsOpenItemMenuState(self)->tourCharacterIndex] *
                                       kTourConnectionRowPixels);
                        UIGroup_SetSourcePosition(obj, p);
                        bool special = ((TourMapRecordPrefix*)info)->connectionStyles[3] == 2;
                        if (!special)
                            special = *(short*)((char*)*(void**)F0BItem((void*)&((TourTreeFields*)self)->items, link) +
                                                6) == 2;
                        if (special)
                            UIGroup_SetEnabled(obj, 0);
                    } else {
                        float p[2];
                        UIGroup_GetSourcePosition(obj, p);
                        p[1] = 0.0f;
                        UIGroup_SetSourcePosition(obj, p);
                    }
                }
            }
            link = OpenItemEntry_GetNeighborIndex(item, 1);
            if (link != -1) {
                obj = F0BObject(self, index, kTourDirection1Link);
                if (obj) {
                    bool hidden = OpenItemEntry_GetFlagBit14(item) != 0;
                    if (!hidden)
                        hidden = OpenItemEntry_GetFlagBit14(F0BItem((void*)&((TourTreeFields*)self)->items, link)) != 0;
                    if (hidden)
                        UIGroup_SetEnabled(obj, 0);
                    else
                        UIGroup_SetEnabled(obj, 1);
                    bool active = OpenItemEntry_GetFlagBit15(item) != 0;
                    if (!active)
                        active = OpenItemEntry_GetFlagBit15(F0BItem((void*)&((TourTreeFields*)self)->items, link)) != 0;
                    if (active) {
                        float p[2];
                        UIGroup_GetSourcePosition(obj, p);
                        p[1] = (float)(s_tourMapIconAtlasRows[AsOpenItemMenuState(self)->tourCharacterIndex] *
                                       kTourConnectionRowPixels);
                        UIGroup_SetSourcePosition(obj, p);
                        bool special = ((TourMapRecordPrefix*)info)->connectionStyles[1] == 2;
                        if (!special)
                            special = *(short*)((char*)*(void**)F0BItem((void*)&((TourTreeFields*)self)->items, link) +
                                                2) == 2;
                        if (special)
                            UIGroup_SetEnabled(obj, 0);
                    } else {
                        float p[2];
                        UIGroup_GetSourcePosition(obj, p);
                        p[1] = 0.0f;
                        UIGroup_SetSourcePosition(obj, p);
                    }
                }
            }
            void* state = AsOpenItemMenuState(self)->menuContext;
            if (AsOpenItemMenuContext(state)->event == 2) {
                int value = AsOpenItemMenuContext(state)->eventValue;
                int column = 3;
                if (value / kTourConnectionCodeRadix)
                    column = 4;
                obj = F0BObject(self, (short)(value % kTourConnectionCodeRadix), column);
                if (obj)
                    UIGroup_SetOpacity(obj, 0);
            }
        }
        Node* last = *(Node**)((char*)node + 4);
        if (last) {
            Node* next;
            while ((next = last->next) != 0)
                last = next;
            node = last;
        } else {
            Node* parent;
            while (node != (parent = (Node*)(node->flags & ~1u))->next) {
                node = parent;
            }
            node = parent;
        }
    }
}

// Apply portrait hover sizing/placement and the selected 40-pixel atlas frame.
// pos/scale/sourceOffset are borrowed two-float output buffers; units are UI pixels.
static inline void ApplyTourPortraitLayout(void* self, void* portraitSprite, int hoveredControl, float* positionPixels,
                                           float* portraitScale, float* sourceOffsetPixels) {
    if (hoveredControl == kTourCharacterControl) {
        portraitScale[0] = portraitScale[1] = kTourPortraitHoveredScale;
        positionPixels[0] += kTourPortraitHoveredOffsetX;
    } else {
        portraitScale[0] = portraitScale[1] = kTourPortraitRestingScale;
        positionPixels[0] += kTourPortraitRestingOffsetX;
        positionPixels[1] += kTourPortraitRestingOffsetY;
    }
    Layout_SetElementScale(AsOpenItemMenuState(self)->layout, kTourCharacterPortrait, portraitScale);
    Layout_SetElementPosition(AsOpenItemMenuState(self)->layout, kTourCharacterPortrait, positionPixels);
    Sprite_SetTextureRegionSize2D(portraitSprite, sourceOffsetPixels);
}

// Update hovered and activated map controls, character icon frames, and the coin counter.
void CTourMain::UpdateItemMapLayout() {
    extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12out_of_range[];
    unsigned int strlen(const char*);
    void* __nwa__FUl(unsigned int);
    void __dla__FPv(void*);
    void __throw(void*, void*, void (*)(void*)) __attribute__((noreturn));
    void __dt__Q23std12out_of_rangeFv(void*);
    void __dt__Q23std9exceptionFv(void*);
    struct ExceptionObject {
        void* values[3];
        // Access one ABI word in the compatible exception representation.
        void*& operator[](int i) {
            return values[i];
        }
        // Destroy only the standard exception base of the stack object.
        ~ExceptionObject() {
            __dt__Q23std9exceptionFv(this);
        }
    };
    struct BufferGuard {
        void* pointer;
        char* allocator;
        // Release exception text unless a shared representation took ownership.
        ~BufferGuard() {
            if (pointer)
                __dla__FPv(pointer);
        }
    };
    struct Results {
        int values[3];
        // Access data-address/count/capacity words of the collected ID vector.
        int& operator[](int i) {
            return values[i];
        }
        // Release collected IDs; layout objects remain borrowed.
        ~Results() {
            if (values[0]) {
                values[1] -= values[1];
                appFreeUni__FPv((void*)values[0]);
            }
        }
    };
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    void* manager = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(this)->numberGroup);
    BpObject_TryInvokeCurrentCallback(manager, 0);
    Layout_UpdateExtended(AsOpenItemMenuState(this)->layout);
    float x[2] = {0.0f, 0.0f};
    // Phase 1: select the portrait atlas frame and its hover-dependent size/placement.
    int frame = (int)AsCCameraController(AsOpenItemMenuState(this)->cameraController)->modelIndex; // original +0x20
    x[0] = kTourPortraitFramePixels * (float)frame;
    void* selected = Layout_GetElementSprite(AsOpenItemMenuState(this)->layout, kTourCharacterPortrait);
    float pos[2];
    Layout_GetElementPosition(AsOpenItemMenuState(this)->layout, kTourCharacterPairedRegion, pos);
    int hover;
    {
        Results hovered = {{0, 0, 0}};
        Layout_CollectHovered(AsOpenItemMenuState(this)->layout, &hovered, 5);
        if (hovered[1]) {
            if (!*(volatile int*)&hovered[1]) {
                char zero = 0;
                ExceptionObject exception;
                exception[0] = &__vt__Q23std11logic_error;
                const char* name = tourMapOpenItemStrings + kTourText_vector_at_index_out_of_range;
                unsigned int n = strlen(name) + 1;
                char* buffer = (char*)__nwa__FUl(n * 2 - n);
                exception[1] = buffer;
                {
                    BufferGuard guard = {buffer, &zero};
                    int* ref = (int*)__nw__FUl(16);
                    if (ref) {
                        ref[1] = 1;
                        ref[2] = 1;
                        ref[0] = (int)&s_sharedCharDeleterVtable;
                        ref[3] = (int)buffer;
                    }
                    exception[2] = ref;
                    guard.pointer = 0;
                }
                strcpy((char*)exception[1], name);
                exception[0] = &__vt__Q23std12out_of_range;
                __throw(tourMapOpenItemStrings + kTourText_std_exception_std_logic_error_std_out_of_rang, &exception,
                        __dt__Q23std12out_of_rangeFv);
            }
            hover = *(int*)hovered[0];
        } else
            hover = -1;
    }
    float scale[2];
    ApplyTourPortraitLayout(this, selected, hover, pos, scale, x);
    // Phase 2: show active feedback for the first activated control, or restore idle states.
    int index;
    {
        Results activated = {{0, 0, 0}};
        Layout_CollectActivated(AsOpenItemMenuState(this)->layout, &activated, 0);
        if (activated[1]) {
            if (!*(volatile int*)&activated[1]) {
                char zero = 0;
                ExceptionObject exception;
                exception[0] = &__vt__Q23std11logic_error;
                const char* name = tourMapOpenItemStrings + kTourText_vector_at_index_out_of_range;
                unsigned int n = strlen(name) + 1;
                char* buffer = (char*)__nwa__FUl(n * 2 - n);
                exception[1] = buffer;
                {
                    BufferGuard guard = {buffer, &zero};
                    int* ref = (int*)__nw__FUl(16);
                    if (ref) {
                        ref[1] = 1;
                        ref[2] = 1;
                        ref[0] = (int)&s_sharedCharDeleterVtable;
                        ref[3] = (int)buffer;
                    }
                    exception[2] = ref;
                    guard.pointer = 0;
                }
                strcpy((char*)exception[1], name);
                exception[0] = &__vt__Q23std12out_of_range;
                __throw(tourMapOpenItemStrings + kTourText_std_exception_std_logic_error_std_out_of_rang, &exception,
                        __dt__Q23std12out_of_rangeFv);
            }
            index = *(int*)activated[0];
        } else
            index = -1;
    }
    if (index != -1) {
        Layout_SetElementValueState(AsOpenItemMenuState(this)->layout, index, 2);
        Layout_SetElementFeedbackState(AsOpenItemMenuState(this)->layout, index, 2);
    } else {
        Layout_SetElementValueState(AsOpenItemMenuState(this)->layout, kTourShopControl, 0);
        Layout_SetElementFeedbackState(AsOpenItemMenuState(this)->layout, kTourShopControl, 1);
        Layout_SetElementValueState(AsOpenItemMenuState(this)->layout, kTourCharacterControl, 0);
        Layout_SetElementFeedbackState(AsOpenItemMenuState(this)->layout, kTourCharacterControl, 1);
    }
    // Phase 3: refresh the numeric coin display from persistent tour progress.
    UITextTable_SetInteger(AsOpenItemMenuState(this)->numberDraw, kTourCoinNumberEntry,
                           ((TourProgressFields*)AsOpenItemMenuState(this)->tourProgress)->coinCount);
}

void MPBase1_DrawIfInitialized(void*);
// Acquire the BpObject controller used by the map rendering helpers.
static inline void* GetBpCtrl() {
    if (Sing_805DC5F0() == 0) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, &s_bpCtrlDtorRegistration_8051D0EC);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
    return Sing_805DC5F0();
}
// Draw map layers, node previews, particles, navigation controls, and number entries.
// All resources must be loaded. Traversal borrows tree nodes and masks the
// parent color bit; root-address sentinel is not a heap node. Render state is
// inherited from the caller; keep model unprojection and 2D draw contexts in
// agreement when widening the viewport. This helper does not advance dt.
static inline void DrawTourMapScene(void* self) {
    BpObject_Draw(IdMap_Lookup(GetBpCtrl(), AsOpenItemMenuState(self)->modeGroup));
    BpObject_Draw(IdMap_Lookup(GetBpCtrl(), AsOpenItemMenuState(self)->itemGroup));
    void* node = AsOpenItemMenuState(self)->firstItemNode;
    while (node != (void*)&((TourTreeFields*)self)->items.root) {
        void* child = AsOpenItemTreeNode(node)->item;
        if (child)
            TourBossPreview_RenderIfVisible(child);
        void* r = AsOpenItemTreeNode(node)->right;
        if (r) {
            while (*(void**)r)
                r = *(void**)r;
            node = r;
        } else {
            void* parent;
            while (node != *(void**)(parent = (void*)((unsigned int)AsOpenItemTreeNode(node)->parentAndColor & ~1u)))
                node = parent;
            node = parent;
        }
    }
    void* object = AsOpenItemMenuState(self)->particleEffects;
    if (object)
        TourTwinkle_DrawParticles(object, (void*)&AsOpenItemMenuState(self)->cameraOffsetX);
    CameraController_RenderPreview(AsOpenItemMenuState(self)->cameraController,
                                   (void*)&AsOpenItemMenuState(self)->cameraOffsetX);
    BpObject_Draw(IdMap_Lookup(GetBpCtrl(), AsOpenItemMenuState(self)->numberGroup));
    Layout_UpdateCursorHover(AsOpenItemMenuState(self)->layout, kFloat_1_OpenItemMenu);
    UITextTable_Draw(AsOpenItemMenuState(self)->numberDraw);
}
struct TourPreviewDrawable {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void Draw();
};
// Draw the active tour state and its current hint, versus, shop, description, or save window.
// Visibility policy only: load/model-change states suppress the map, event
// states add hint/versus/preview overlays, and submenus replace or overlay
// the map as listed. Child draw calls can update hover/render state; preserve
// their order. No local aspect-ratio multiply or viewport/scissor setter.
void CTourMain::DrawItemMenu() {
    switch ((*(unsigned int*)&AsOpenItemMenuState(this)->state)) {
    case kTourInactive:
    case kTourLoadArchives:
    case kTourWaitMapResources:
    case kTourWaitUiResources:
    case kTourApplyCharacterSelection:
    case kTourWaitCharacterPreview:
        break;
    case kTourStartFadeOut:
    case kTourWaitFadeOut:
    case kTourEnableNavigation:
    case kTourNavigate:
    case kTourStartPreview:
    case kTourWaitPreview:
    case kTourPrepareRewards:
    case kTourEnterMatch:
    case kTourProcessMatchReturn:
    case kTourWaitReturnAnimation:
    case kTourStartRewardDelivery:
    case kTourWaitRewardDelivery:
    case kTourOpenCharacterSelect:
    case kTourExit:
        DrawTourMapScene(this);
        break;
    case kTourStartBeforeEvent:
    case kTourProcessBeforeEvent:
    case kTourStartAfterEvent:
    case kTourProcessAfterEvent:
    case kTourStartCompletionEvent:
    case kTourProcessCompletionEvent: {
        DrawTourMapScene(this);
        switch (AsOpenItemMenuState(this)->windowState) {
        case kTourBeginVersus:
        case kTourProcessVersus:
            ((CTourVSWindow*)(AsOpenItemMenuState(this)->tourVsWindow))->DrawGlobal();
            break;
        case kTourBeginHint:
        case kTourProcessHint:
            ((CTourHintWindow*)(AsOpenItemMenuState(this)->tourHintWindow))->DrawCurrentLayout();
            break;
        }
        TourPreviewDrawable* v = (TourPreviewDrawable*)((TourPreviewFields*)this)->preview;
        if (v)
            v->Draw();
        break;
    }
    case kTourSelectCharacter:
        ((CTourWindow*)(AsOpenItemMenuState(this)->tourWindow))->Draw();
        break;
    case kTourOpenShop:
    case kTourWaitShopReady:
        DrawTourMapScene(this);
        break;
    case kTourProcessShop:
    case kTourCloseShop: {
        void* v = AsOpenItemMenuState(this)->shop;
        if (v)
            MPBase1_DrawIfInitialized(v);
        break;
    }
    case kTourOpenDescription:
    case kTourProcessDescription: {
        DrawTourMapScene(this);
        void* v = AsOpenItemMenuState(this)->messageResourceHolder;
        if (v)
            CTourHintWindow::DrawSingletonPage(v);
        break;
    }
    case kTourOpenSave:
    case kTourProcessSave: {
        void* v = AsOpenItemMenuState(this)->flashMemorySaveMenu;
        if (v)
            MPBase1_DrawIfInitialized(v);
        break;
    }
    }
}
#if 0
    // Retained disabled reference implementation of map drawing.
    static void F2FC_Unused(void* self) {
        if (!Sing_805DC5F0()) EnsureBpCtrl();
        void* manager = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->modeGroup);
        BpObject_Draw(manager);
        if (!Sing_805DC5F0()) EnsureBpCtrl();
        manager = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->itemGroup);
        BpObject_Draw(manager);
        void* node = AsOpenItemMenuState(self)->firstItemNode;
        void* sentinel = (void*)&((TourTreeFields*)self)->items.root;
        while (node != sentinel) {
            void* child = AsOpenItemTreeNode(node)->item;
            if (child) TourBossPreview_RenderIfVisible(child);
            node = AsOpenItemTreeNode(node)->right;
            if (node) while (AsOpenItemTreeNode(node)->left) node = AsOpenItemTreeNode(node)->left;
            else {
                do {
                    void* parent = (void*)((unsigned int)AsOpenItemTreeNode(node)->parentAndColor & ~1u);
                    if (node == *(void**)parent) break;
                    node = parent;
                }
                while (1);
                node = (void*)((unsigned int)AsOpenItemTreeNode(node)->parentAndColor & ~1u);
            }
        }
        void* object = AsOpenItemMenuState(self)->particleEffects;
        if (object) TourTwinkle_DrawParticles(object, (void*)&AsOpenItemMenuState(self)->cameraOffsetX);
        CameraController_RenderPreview(AsOpenItemMenuState(self)->cameraController, (void*)&AsOpenItemMenuState(self)->cameraOffsetX);
        if (!Sing_805DC5F0()) EnsureBpCtrl();
        manager = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->numberGroup);
        BpObject_Draw(manager);
        Layout_UpdateCursorHover(AsOpenItemMenuState(self)->layout, kFloat_1_OpenItemMenu);
        UITextTable_Draw(AsOpenItemMenuState(self)->numberDraw);
    }
#endif

struct F186CSnd {
    int a;
    int b;
};
// Acquire the shared menu script handle and register its destructor on first use.
static inline void* F186C_Snd(char* g) {
    extern signed char s_bpCtrlHandleInitialized;
    extern int s_bpCtrlHandle;
    extern char s_itemMenuSingletonRegistration_8051D0E0[];
    void CBpCtrlHandle_Dtor();
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            int* s = &s_bpCtrlHandle;
            s[1] = -1;
            s_bpCtrlHandle = -1;
            __register_global_object(&s_bpCtrlHandle, CBpCtrlHandle_Dtor, (g + 0x18));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = &s_bpCtrlHandle;
    }
    return Sing_805DC5F8();
}
// Acquire the shared screen fade overlay and register its destructor on first use.
static inline void* F186C_Bg(char* g) {
    extern signed char s_fadeOverlayInitialized;
    extern char s_fadeOverlay[];
    extern char s_itemMenuSingletonRegistration_8051D0E0[];
    void ResourceState_Dtor();
    void FadeOverlay_Init(void*);
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(&s_fadeOverlay);
            __register_global_object(&s_fadeOverlay, ResourceState_Dtor, (g + 0x24));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = &s_fadeOverlay;
    }
    return Sing_805DC608();
}
struct F186CPair {
    int key;
    void* value;
};
// Find the navigation target item, inserting a default tree entry when missing.
static inline void* F186C_Get(void* self, int key) {
    extern void* g_bpEntryDefaultMappedValue;
    int* node;
    unsigned char a, b;
    int k = key;
    void* it = IntegerKeyTree_FindInsertionPosition((void*)&((TourTreeFields*)self)->items, &k, (int*)&node, &a, &b);
    F186CPair* p;
    if (node == 0 || (p = (F186CPair*)((void*)&((TourTreeNodeFields*)node)->key), node[3] < k)) {
        F186CPair np;
        np.key = k;
        np.value = g_bpEntryDefaultMappedValue;
        p = (F186CPair*)((char*)BpEntryTree_InsertNode((void*)&((TourTreeFields*)self)->items, it, a, b, (int*)&np) +
                         0xc);
    }
    return p->value;
}
struct F186CSelf {
    char pad[0x10];
    char map[0x10];
};
// Original layout pointer at self +0x78.
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementEnabled(void*, int, int);
// Enable or hide shop/character controls, their paired regions and the portrait.
static inline void SetTourNavigationEnabled(void* self, int enabled) {
    Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(self)->layout, kTourShopControl, enabled);
    Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(self)->layout, kTourShopPairedRegion, enabled);
    Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(self)->layout, kTourCharacterControl, enabled);
    Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(self)->layout, kTourCharacterPairedRegion, enabled);
    Layout_SetElementEnabled(AsOpenItemMenuState(self)->layout, kTourCharacterPortrait, enabled);
}

void __dla__FPv(void*);
void __dt__Q23std9exceptionFv(void*);
struct F186CExc {
    void* values[3];
    // Access one word of the compatible exception object.
    void*& operator[](int i) {
        return values[i];
    }
    // Destroy the stack exception base without deleting its storage.
    ~F186CExc() {
        __dt__Q23std9exceptionFv(this);
    }
};
struct F186CGuard {
    void* pointer;
    char* allocator;
    // Release an untransferred exception-message buffer.
    ~F186CGuard() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct F186CResults {
    int values[3];
    // Access the data-address/count/capacity words of collected control IDs.
    int& operator[](int i) {
        return values[i];
    }
    // Free the collected control-ID vector, not layout objects.
    ~F186CResults() {
        if (values[0]) {
            values[1] -= values[1];
            appFreeUni__FPv((void*)values[0]);
        }
    }
};

// Handle directional map movement and activate shop, character, description, or exit controls.
// Entry from ProcessItemMenu's Navigate phase only: self/context/save record,
// selected camera item, layout and map nodes must be loaded and remain live.
// Neighbor keys must be -1 or actual populated keys in [0,35); F186C_Get may
// insert a default mapped value for a missing key and does NOT validate it.
// No status/output argument; writes pendingState, cameraMovePending, saved
// selectedItem and camera heading/animation/target. State commits NEXT update.
// Input policy: held actions 0/1/2/3 select left/right/up/down in that priority;
// pressed 4 confirms, 6 exits, 0x24 opens description. -1 selects the input
// manager's CURRENT channel (game_input_manager_cursor_rumble.cpp); synthetic presses can bypass action
// blocks, while the game-hook veto still wins. Moving camera disables
// controls/releases the caption BP entry,
// marks persistence pending and returns before action activation. At rest it
// persists the arrived item, then restores controls when no new move begins.
// Confirm collects hover IDs for the selected player (layout sentinel 5,
// not a fifth player) into owned {data,count,capacity}
// scratch (F186CResults); the first ID selects shop=0 or character=1, empty
// selects the map node (-1), other IDs do nothing. The vector owns only IDs,
// never layout elements; its destructor releases data on return or unwind.
// Map confirm is refused when record+0A==1. Accepted actions disable controls
// and release caption; character/exit request fade, shop/character/help play
// icon-click audio. No callback retains the local hover vector/exception data.
// Allocation/lookup exceptions propagate with prior camera/UI/global changes
// intact. The count's volatile reread preserves the original vector-at guard:
// if it becomes zero after the outer nonempty test, build/throw out_of_range.
// F186CGuard owns the text until its shared deleter record is installed;
// F186CExc owns stack exception cleanup, not a hovered item. No locking here;
// input/layout/node mutation must be serialized with this workflow.
void CTourMain::HandleItemMapNavigation() {
    extern signed char s_inputManagerInitialized;
    extern signed char s_bpCtrlHandleInitialized;
    extern signed char s_fadeOverlayInitialized;
    extern char s_gameInputManager[];
    extern char s_itemMenuSingletonRegistration_8051D0E0[];
    extern char tourMapOpenItemStrings[0x1F8], __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[],
        __vt__Q23std12out_of_range[], s_fadeOverlay[];
    extern float kOpenItemCameraReverseHeading, kOpenItemLeftNeighborHeading, kOpenItemRightNeighborHeading;
    void* InputManager_Init(void*);
    int Input_IsActionButtonDown(void*, int, int);
    int Input_IsActionPressed(void*, int, int);
    void Layout_SetElementPlayerFlagsAndState(void*, int, int);
    void Layout_SetElementEnabled(void*, int, int);
    short OpenItemEntry_GetNeighborIndex(void*, int);
    int OpenItemEntry_GetFlagBit14(void*);
    int OpenItemEntry_GetFlagBit13(void*);
    int OpenItemEntry_GetFlagBit15(void*);
    void CBpCtrlHandle_Dtor();
    void InputManager_Dtor();
    void ResourceState_Dtor();
    void FadeOverlay_Init(void*);
    void FadeState_RequestFramebufferCapture(void*);
    void FadeState_StartFadeIn(void*, int);
    void Layout_CollectHovered(void*, void*, int);
    void Sound_PlayByName(void*, const char*, int, int, int, int, float, float, float);
    unsigned int strlen(const char*);
    void* __nwa__FUl(unsigned int);
    void* __nw__FUl(unsigned int);
    void strcpy(char*, const char*);
    void __throw(void*, void*, void (*)(void*)) __attribute__((noreturn));
    void __dt__Q23std12out_of_rangeFv(void*);
    char* g = s_itemMenuSingletonRegistration_8051D0E0;
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, g);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* manager = GetInputManagerSlot();
    short selected = -1;
    if (!manager)
        return;
    // While moving, suppress menu controls and defer persistence until the camera settles.
    if (AsCCameraController(AsOpenItemMenuState(this)->cameraController)->moving) {
        // original +0x18
        SetTourNavigationEnabled(this, 0);
        ((CBpCtrlHandle*)(F186C_Snd(g)))->ReleaseEntry();
        AsOpenItemMenuState(this)->cameraMovePending = 1;
        return;
    }
    // Phase 1: select a directional neighbor and reject hidden/reward-blocked/unopened routes.
    if (Input_IsActionButtonDown(manager, 0, -1)) {
        selected = OpenItemEntry_GetNeighborIndex(
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry(), 0);
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->SetTargetHeading(kFloat_0_OpenItemMenu);
    } else if (Input_IsActionButtonDown(manager, 1, -1)) {
        selected = OpenItemEntry_GetNeighborIndex(
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry(), 1);
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->SetTargetHeading(kOpenItemCameraReverseHeading);
    } else if (Input_IsActionButtonDown(manager, 2, -1)) {
        selected = OpenItemEntry_GetNeighborIndex(
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry(), 2);
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->SetTargetHeading(kOpenItemLeftNeighborHeading);
    } else if (Input_IsActionButtonDown(manager, 3, -1)) {
        selected = OpenItemEntry_GetNeighborIndex(
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry(), 3);
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->SetTargetHeading(kOpenItemRightNeighborHeading);
    }
    if (selected != -1) {
        void* v1;
        void* v2;
        void* v3;
        v1 = F186C_Get(this, selected);
        int c = OpenItemEntry_GetFlagBit14(v1) != 0;
        if (!c) {
            v2 = F186C_Get(this, selected);
            c = OpenItemEntry_GetFlagBit13(v2) != 0;
        }
        if (!c) {
            c = OpenItemEntry_GetFlagBit15(
                    ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry()) == 0;
            if (c) {
                v3 = F186C_Get(this, selected);
                c = OpenItemEntry_GetFlagBit15(v3) == 0;
            }
        }
        if (c)
            selected = -1;
    }
    // Phase 2: persist the settled selection, restore controls, or begin the next move.
    if (AsOpenItemMenuState(this)->cameraMovePending) {
        AsOpenItemMenuState(this)->cameraMovePending = 0;
        void* list = ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry();
        AsOpenItemFlags(AsOpenItemMenuState(this)->selectionRecord)->selectedItem = AsOpenItemEntry(list)->index;
        if (selected == -1) {
            ((CBpCtrlHandle*)(F186C_Snd(g)))->SetScriptIndex(11);
            SetTourNavigationEnabled(this, 1);
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->SetAnimationByIndex(0);
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->ResetPositionToItem();
        }
    }
    if (selected != -1) {
        void* v4;
        void* v5;
        int c = OpenItemEntry_GetFlagBit15(
                    ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry()) == 0;
        if (c) {
            v4 = F186C_Get(this, selected);
            c = OpenItemEntry_GetFlagBit15(v4) == 0;
        }
        if (c)
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->SetAnimationByIndex(0);
        else {
            v5 = F186C_Get(this, selected);
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->TryMoveToItem(v5);
        }
    }
    // While moving, suppress menu controls and defer persistence until the camera settles.
    if (AsCCameraController(AsOpenItemMenuState(this)->cameraController)->moving) {
        // original +0x18
        SetTourNavigationEnabled(this, 0);
        ((CBpCtrlHandle*)(F186C_Snd(g)))->ReleaseEntry();
        AsOpenItemMenuState(this)->cameraMovePending = 1;
        return;
    }
    // Phase 3: activate the hovered shop/character control or the selected map item; handle exit/help.
    if (Input_IsActionPressed(manager, 4, -1)) {
        int action;
        {
            F186CResults vec = {{0, 0, 0}};
            Layout_CollectHovered(AsOpenItemMenuState(this)->layout, &vec, 5);
            if (vec[1]) {
                if (!*(volatile int*)&vec[1]) {
                    char zero = 0;
                    F186CExc exception;
                    exception[0] = &__vt__Q23std11logic_error;
                    const char* name = tourMapOpenItemStrings + kTourText_vector_at_index_out_of_range;
                    unsigned int n = strlen(name) + 1;
                    char* buffer = (char*)__nwa__FUl(n * 2 - n);
                    exception[1] = buffer;
                    {
                        F186CGuard guard = {buffer, &zero};
                        int* ref = (int*)__nw__FUl(16);
                        if (ref) {
                            ref[1] = 1;
                            ref[2] = 1;
                            ref[0] = (int)&s_sharedCharDeleterVtable;
                            ref[3] = (int)buffer;
                        }
                        exception[2] = ref;
                        guard.pointer = 0;
                    }
                    strcpy((char*)exception[1], name);
                    exception[0] = &__vt__Q23std12out_of_range;
                    __throw(tourMapOpenItemStrings + kTourText_std_exception_std_logic_error_std_out_of_rang,
                            &exception, __dt__Q23std12out_of_rangeFv);
                }
                action = *(int*)vec[0];
            } else
                action = -1;
        }
        switch (action) {
        case -1:
            if ((int)*(
                    unsigned char*)((char*)*(void**)((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
                                        ->GetItemEntry() +
                                    0xa) == (int)true)
                return;
            SetTourNavigationEnabled(this, 0);
            ((CBpCtrlHandle*)(F186C_Snd(g)))->ReleaseEntry();
            AsOpenItemMenuState(this)->pendingState = kTourStartPreview;
            break;
        case 0:
            Sound_PlayByName(g_soundManager, tourMapOpenItemStrings + kTourText_ui_icon_click, 0, 0, 0, 0,
                             kFloat_0_5_OpenItemMenu, kFloat_200_OpenItemMenu, kFloat_1_OpenItemMenu);
            SetTourNavigationEnabled(this, 0);
            ((CBpCtrlHandle*)(F186C_Snd(g)))->ReleaseEntry();
            AsOpenItemMenuState(this)->pendingState = kTourOpenShop;
            break;
        case 1:
            Sound_PlayByName(g_soundManager, tourMapOpenItemStrings + kTourText_ui_icon_click, 0, 0, 0, 0,
                             kFloat_0_5_OpenItemMenu, kFloat_200_OpenItemMenu, kFloat_1_OpenItemMenu);
            FadeState_RequestFramebufferCapture(F186C_Bg(g));
            FadeState_StartFadeIn(F186C_Bg(g), 0);
            SetTourNavigationEnabled(this, 0);
            ((CBpCtrlHandle*)(F186C_Snd(g)))->ReleaseEntry();
            AsOpenItemMenuState(this)->pendingState = kTourOpenCharacterSelect;
            break;
        }
    } else if (Input_IsActionPressed(manager, 6, -1)) {
        SetTourNavigationEnabled(this, 0);
        ((CBpCtrlHandle*)(F186C_Snd(g)))->ReleaseEntry();
        FadeState_RequestFramebufferCapture(F186C_Bg(g));
        FadeState_StartFadeIn(F186C_Bg(g), 0);
        AsOpenItemMenuState(this)->pendingState = kTourExit;
    } else if (Input_IsActionPressed(manager, 0x24, -1)) {
        SetTourNavigationEnabled(this, 0);
        ((CBpCtrlHandle*)(F186C_Snd(g)))->ReleaseEntry();
        Sound_PlayByName(g_soundManager, tourMapOpenItemStrings + kTourText_ui_icon_click, 0, 0, 0, 0,
                         kFloat_0_5_OpenItemMenu, kFloat_200_OpenItemMenu, kFloat_1_OpenItemMenu);
        AsOpenItemMenuState(this)->pendingState = kTourOpenDescription;
    }
}

struct S_ED074 {
    unsigned w0;
    unsigned len;
    char* ptr;
    // Read the inline byte length or heap length from this tagged string.
    unsigned size() const {
        return !(w0 >> 31) ? (*(unsigned char*)this & 0x7f) : len;
    }
    // Borrow the string text; any mutation/free invalidates the result.
    char* data() {
        return !(w0 >> 31) ? (char*)this + 1 : ptr;
    }
};
struct TourMapResources {
    const char* itemDataArchive;
    const char* itemTextureArchive;
};
struct A_ED074 {
    char c;
};
extern "C" unsigned int strlen(const char*);
extern "C" void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, void*);
// Append a C string to the tagged Metrowerks string without changing its allocator state.
inline void ED_APPC(S_ED074* s, const char* p, const A_ED074& a) {
    unsigned n = s->size();
    A_ED074 c = a;
    String_ReplaceRange(s, n, 0, p, p + strlen(p), &c);
}
// Append another tagged Metrowerks string, supporting inline and heap storage.
inline void ED_APPS(S_ED074* s, S_ED074* t, const A_ED074& a) {
    unsigned n = s->size();
    char* d;
    unsigned m;
    if (!(t->w0 >> 31)) {
        d = (char*)t + 1;
        m = *(unsigned char*)t & 0x7f;
    } else {
        d = t->ptr;
        m = t->len;
    }
    char* e = d + m;
    A_ED074 c = a;
    String_ReplaceRange(s, n, 0, d, e, &c);
}
extern signed char s_fadeOverlayInitialized, s_bpCtrlHandleInitialized, s_resourceTableInitialized,
    s_saveDataInitialized;
extern char s_fadeOverlay[], s_saveData[];
extern int s_bpCtrlHandle;
extern void* s_resourceTableStorage;
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void CBpCtrlHandle_Dtor();
void Allocation_Dtor();
void* SaveData_Ctor(void*);
// Initialize this shared service before the state uses it.
static inline void EnsureTourBpController(char* base) {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)&s_bpCtrl)->__ct();
            __register_global_object(&s_bpCtrl, CBpCtrl_dtor, (base + 0xc));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = &s_bpCtrl;
    }
}
// Initialize this shared service before the state uses it.
static inline void EnsureTourFadeOverlay(char* base) {
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(&s_fadeOverlay);
            __register_global_object(&s_fadeOverlay, ResourceState_Dtor, (base + 0x24));
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = &s_fadeOverlay;
    }
}
// Initialize this shared service before the state uses it.
static inline void EnsureTourMenuScript(char* base) {
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            (&s_bpCtrlHandle)[1] = -1;
            s_bpCtrlHandle = -1;
            __register_global_object(&s_bpCtrlHandle, CBpCtrlHandle_Dtor, (base + 0x18));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = &s_bpCtrlHandle;
    }
}
// Initialize this shared service before the state uses it.
static inline void EnsureTourResourceTable(char* base) {
    if (!Sing_805DC620()) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, Allocation_Dtor, (base + 0x54));
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
}
// Initialize this shared service before the state uses it.
static inline void EnsureTourItemRequests(char* base) {
    if (!Sing_805DC958()) {
        if (!s_itemRequestStateInitialized) {
            ItemRequestState_Ctor(s_itemRequestState);
            __register_global_object(s_itemRequestState, ItemRequestState_Dtor, (base + 0x84));
            s_itemRequestStateInitialized = 1;
        }
        Sing_805DC958() = s_itemRequestState;
    }
}
// Initialize this shared service before the state uses it.
static inline void EnsureTourSaveData(char* base) {
    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = s_saveData;
    }
}
// Return the persistent flags for the selected item, preserving reference stores.
static inline unsigned short& SelectedMapItemFlags(void* self) {
    return ((TourSelectionFields*)AsOpenItemMenuState(self)->selectionRecord)
        ->items[AsOpenItemMenuState(self)->selectedItem];
}
void BpObject_SetEnabled(void*, int);
// Enable or hide all three BpObject layers and their number renderer together.
static inline void SetTourMapLayersEnabled(void* self, int enabled, char* base) {
    void* layer;
    EnsureTourBpController(base);
    layer = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->modeGroup);
    if (layer)
        BpObject_SetEnabled(layer, enabled);
    EnsureTourBpController(base);
    layer = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->itemGroup);
    if (layer)
        BpObject_SetEnabled(layer, enabled);
    EnsureTourBpController(base);
    layer = IdMap_Lookup(Sing_805DC5F0(), AsOpenItemMenuState(self)->numberGroup);
    if (layer)
        BpObject_SetEnabled(layer, enabled);
    {
        void* numbers = AsOpenItemMenuState(self)->numberDraw;
        if (numbers)
            ((TourNumberFields*)numbers)->enabled = enabled;
    }
}
// Destroy and clear an owned menu object through its original virtual slot.
static inline void DeleteTourMenuObject(void** slot) {
    if (*slot) {
        void* object = *slot;
        if (object)
            (*(void (***)(void*, int))object)[2](object, 1);
    }
    *slot = 0;
}

// Request a phase for the next ProcessItemMenu invocation, after its common visual updates.
static inline void QueueTourPhase(void* menu, int phase) {
    AsOpenItemMenuState(menu)->pendingState = phase;
}
// Publish a phase immediately; the caller deliberately falls into the next switch case.
static inline void EnterTourPhaseNow(void* menu, int phase) {
    AsOpenItemMenuState(menu)->state = phase;
}
enum EncounterClassification {
    kFirstLinkActiveMatchType = 1,
    kLastLinkActiveMatchTypeIndex = 5,
    kLinkActiveMatchTypeIndices = (1 << 0) | (1 << 4) | (1 << 5)
};
// matchType 1,5,6 publish linkActive=1; all other bytes publish 0. The precise
// gameplay labels of these three classes are unverified; do not infer them from iconKind.
// The byte subtraction intentionally wraps type 0 to 255 before the range test.
static inline unsigned char LinkActiveMatchTypeIndex(void* pairAddress) {
    return ((TourMapRecordPrefix*)AsOpenItemEntry(((F263C_MapPair*)pairAddress)->value)->record)->matchType + 0xff;
}

extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern signed char s_bpCtrlInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle;
extern signed char s_fadeOverlayInitialized;
extern signed char s_itemRequestStateInitialized;
extern signed char s_saveDataInitialized;
extern char s_itemMenuSingletonRegistration_8051D0E0[], s_fadeOverlay[];
extern char s_bpCtrl[];
extern char tourVoiceResourceName[];
extern TourMapResources s_tourMapArchiveNames[kTourMapCount];
extern float kOpenItemCameraReverseHeading;
void OpenItemMenuState_UpdateCameraOffsets(void*);
int OpenItemMenu_PollResourcesLoaded(void*);
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void FadeState_StartFadeOut(void*, int);
int FadeState_GetState(void*);
void BpObject_SetEnabled(void*, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementEnabled(void*, int, int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor();
void CBpCtrlHandle_Dtor();
void Allocation_Dtor();
void ItemRequestState_Dtor();
void Object_DeleteAndClear(void*);
void TourEventMessage_ReplaceEffect(void*, int);
void* ResourceTable_FindEntryByName(void*, const char*, int*);
void* __nw__FUl(unsigned int);
void String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, void*);
void Character_GetAssetNameSuffix(void*, int);
void Event_GetAssetNameSuffix(void*, int);
void appFreeUni__FPv(void*);
unsigned int strlen(const char*);
int SharedStringState_GetContext();
int ItemRequestState_ClearEntries(void*);
int ItemRequestState_GetEntryCount(void*);
void ItemRequestState_ApplyPendingRewards(void*);
int ItemRequestState_IsComplete(void*);
void OpenItemMenu_QueueSelectedItemRewards(void*, void*);
void OpenItemEntry_SetFlagBit15(void*, int);

void FadeState_RequestFramebufferCapture(void*);
void FadeState_StartFadeIn(void*, int);

void TourItemMenu_DisableRegisteredObject(void*);
void SharedStringState_SetCharacterIndex(int);
void SharedStringState_SetCaddieIndex(int);
int CameraController_PollPreview(void*);
int MPBase1_Process(void*, float);
void MsgResourceHolder_Ctor(void*);
void TourPosition_ShowDescriptionHint(void*, void*);
void TourHintWindow_StartGlobalFadeIn(void*);
int TourHint_UpdateFadeAndInput(void*);
void TourHintWindow_ClearGlobalMessageAndObjects(void*);
void MsgResourceHolder_Dtor(void*, int);
void* SaveData_Ctor(void*);
extern char s_saveData[];
void SaveData_SetCurrentRecord(void*, void*);
void* SaveData_GetCurrentRecord(void*);

// Reset event channels and begin the selected before-event, or queue encounter setup.
static inline void StartTourBeforeEvent(void* self, void* e) {
    AsOpenItemMenuState(self)->selectedItem =
        AsOpenItemEntry(((CCameraController*)(AsOpenItemMenuState(self)->cameraController))->GetItemEntry())
            ->index; // original +4
    AsOpenItemMenuContext(AsOpenItemMenuState(self)->menuContext)->useDefaultNames = 0;
    (*(char*)&AsOpenItemMenuContext(AsOpenItemMenuState(self)->menuContext)->event) = 0;
    AsOpenItemMenuContext(AsOpenItemMenuState(self)->menuContext)->itemEvent = 4;
    if (((TourMapRecordPrefix*)e)->beforeEvent == -1) {
        QueueTourPhase(self, kTourPrepareRewards);
        return;
    }
    TourEventMessage_ReplaceEffect(AsOpenItemMenuState(self)->messageFile,
                                   (unsigned char)((TourMapRecordPrefix*)e)->beforeEvent);
    QueueTourPhase(self, kTourProcessBeforeEvent);
}

// Begin the selected after-event, or queue the completion event when absent.
static inline void StartTourAfterEvent(void* self, void* e) {
    AsOpenItemMenuState(self)->selectedItem =
        AsOpenItemEntry(((CCameraController*)(AsOpenItemMenuState(self)->cameraController))->GetItemEntry())
            ->index; // original +4
    if (((TourMapRecordPrefix*)e)->afterEvent == -1) {
        QueueTourPhase(self, kTourStartCompletionEvent);
        return;
    }
    TourEventMessage_ReplaceEffect(AsOpenItemMenuState(self)->messageFile,
                                   (unsigned char)((TourMapRecordPrefix*)e)->afterEvent);
    QueueTourPhase(self, kTourProcessAfterEvent);
}

// Start reward processing when requests exist; otherwise disable the item and resume navigation.
static inline void StartTourRewardDelivery(void* self, char* base) {
    EnsureTourItemRequests(base);
    if (ItemRequestState_GetEntryCount(Sing_805DC958()) > 0) {
        EnsureTourItemRequests(base);
        ((TourItemRequestFields*)Sing_805DC958())->processing = 1;
        QueueTourPhase(self, kTourWaitRewardDelivery);
    } else {
        SelectedMapItemFlags(self) &= ~kMapItemEnabled;
        QueueTourPhase(self, kTourEnableNavigation);
    }
}

// Wait for reward delivery, then clear processing/item flags and resume navigation.
static inline void PollTourRewardDelivery(void* self, char* base) {
    EnsureTourItemRequests(base);
    if (ItemRequestState_IsComplete(Sing_805DC958())) {
        EnsureTourItemRequests(base);
        ((TourItemRequestFields*)Sing_805DC958())->processing = 0;
        SelectedMapItemFlags(self) &= ~kMapItemEnabled;
        QueueTourPhase(self, kTourEnableNavigation);
    }
}

enum TourVoiceEvent {
    kTourVoicePreview = 0,
    kTourVoiceSuccess = 1,
    kTourVoiceFailure = 2,
    kTourVoiceCharacterChanged = 3,
    kTourVoiceRewardSuccess = 4
};

// Build the current character's event voice name and preserve the emitted release-before-play order.
// All tagged strings/allocator tags and the text pointer are caller scratch to retain stack lifetimes.
static inline void PlayTourCharacterEventVoice(int characterIndex, int eventIndex, S_ED074& eventSuffix,
                                               S_ED074& characterSuffix, S_ED074& resourceName, A_ED074& eventAllocator,
                                               A_ED074& characterAllocator, A_ED074& prefixAllocator, char*& text) {
    resourceName.w0 = 0;
    resourceName.len = 0;
    resourceName.ptr = 0;
    ED_APPC(&resourceName, tourVoiceResourceName, prefixAllocator);
    Character_GetAssetNameSuffix(&characterSuffix, characterIndex);
    ED_APPS(&resourceName, &characterSuffix, characterAllocator);
    if ((characterSuffix.w0 >> 31))
        appFreeUni__FPv(characterSuffix.ptr);
    Event_GetAssetNameSuffix(&eventSuffix, eventIndex);
    ED_APPS(&resourceName, &eventSuffix, eventAllocator);
    if ((eventSuffix.w0 >> 31))
        appFreeUni__FPv(eventSuffix.ptr);
    text = resourceName.data();
    if ((resourceName.w0 >> 31))
        appFreeUni__FPv(resourceName.ptr);
    Sound_PlayByName(g_soundManager, text, 0, 0, 0, 0, kFloat_0_5_OpenItemMenu, kFloat_200_OpenItemMenu,
                     kFloat_1_OpenItemMenu);
}

#pragma push
#pragma always_inline on
// Apply saved character/caddie selection and reload the preview only when its model changes.
static inline void ApplyTourCharacterSelection(void* self) {
    char* p = *(char**)AsOpenItemMenuState(self)->menuContext;
    unsigned char a = AsSavePlayerRecord(p)->characterIndex, b = AsSavePlayerRecord(p)->caddieIndex;
    SharedStringState_SetCharacterIndex((signed char)a);
    SharedStringState_SetCaddieIndex((signed char)b);
    if ((signed char)a != (*(int*)&AsCCameraController(AsOpenItemMenuState(self)->cameraController)->modelIndex)) {
        ((CCameraController*)(AsOpenItemMenuState(self)->cameraController))->SetModelIndex((signed char)a);
        QueueTourPhase(self, kTourWaitCharacterPreview);
        return;
    }
    AsOpenItemMenuContext(AsOpenItemMenuState(self)->menuContext)->nextState = 0;
    QueueTourPhase(self, kTourStartFadeOut);
    return;
}
#pragma pop

#pragma push
#pragma always_inline on
// Commit a completed before-event and select rewards, navigation or a map change.
static inline void PollTourBeforeEvent(void* self, int& result) {
    int r = ((CTourMain*)self)->ProcessSelectedMapItem();
    if ((signed char)r) {
        Object_DeleteAndClear(AsOpenItemMenuState(self)->messageFile);
        SelectedMapItemFlags(self) |= kBeforeEventSeen;
        if ((signed char)r == 2) {
            result = 2;
            AsOpenItemMenuContext(AsOpenItemMenuState(self)->menuContext)->nextState = 0;
        } else if ((signed char)r == 1)
            QueueTourPhase(self, kTourPrepareRewards);
        else if ((signed char)r == -1)
            QueueTourPhase(self, kTourEnableNavigation);
    }
    return;
}
#pragma pop

#pragma push
#pragma always_inline on
// Resolve the completion script, then choose reward delivery or navigation.
static inline void PollTourCompletionEvent(void* self) {
    int r = ((CTourMain*)self)->ProcessSelectedMapItem();
    if ((signed char)r) {
        Object_DeleteAndClear(AsOpenItemMenuState(self)->messageFile);
        (*(char*)&AsOpenItemMenuContext(AsOpenItemMenuState(self)->menuContext)->event) = 0;
        AsOpenItemMenuContext(AsOpenItemMenuState(self)->menuContext)->itemEvent = 4;
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) &&
            !(*(int*)&AsCSharedDoc(g_sharedDoc)->partyResultTier)) {
            QueueTourPhase(self, kTourEnableNavigation);
            return;
        }
        if ((signed char)r == -1) {
            SelectedMapItemFlags(self) &= ~kMapItemEnabled;
            QueueTourPhase(self, kTourEnableNavigation);
        } else if ((signed char)r == 1)
            QueueTourPhase(self, kTourStartRewardDelivery);
    }
    return;
}
#pragma pop

#pragma push
#pragma always_inline on
// Commit after-event completion and choose navigation or the completion effect.
static inline void PollTourAfterEvent(void* self) {
    int r = ((CTourMain*)self)->ProcessSelectedMapItem();
    if ((signed char)r) {
        Object_DeleteAndClear(AsOpenItemMenuState(self)->messageFile);
        SelectedMapItemFlags(self) |= kAfterEventSeen;
        if ((signed char)r == 1)
            QueueTourPhase(self, kTourStartCompletionEvent);
        else if ((signed char)r == -1)
            QueueTourPhase(self, kTourEnableNavigation);
    }
    return;
}
#pragma pop

// Advance the tour map workflow through events, encounters, rewards, shop, character selection, and saving.
// self requires initialized OpenItemMenuState/context; return meanings and
// per-state ownership/admission are mapped by TourMenuState above. dt is
// forwarded unchanged to animation/particles/shop/save consumers. Common
// visual updates precede transitions; do not double-step them on fallthrough.
// dt is finite NONNEGATIVE simulation seconds, forwarded from
// CTourMain_OnProcess(deltaSeconds), tour_main_task.cpp, on CMainFrame's seconds hook
// (main_frame_letter_and_mouse_cursor.cpp; baseline reciprocal 30 or 25 updates/s). It is not milliseconds,
// a frame count or wall-clock timestamp. No clamp/upper bound is enforced;
// negative/NaN steps are unsupported. Zero still polls input/resources and
// advances per-call script/fade/movement counters. Large dt can skip animation
// crossings; this workflow does not subdivide it to catch them up. Preserve
// seconds-driven animation alongside the independent per-call controls above.
int CTourMain::ProcessItemMenu(float dt) {
    float delta = dt;
    char* base = s_itemMenuSingletonRegistration_8051D0E0;
    int result = 0;
    // Commit the previous call's requested phase before any common visual update.
    if (AsOpenItemMenuState(this)->pendingState) {
        AsOpenItemMenuState(this)->state = AsOpenItemMenuState(this)->pendingState;
        QueueTourPhase(this, kTourInactive);
    }
    // Visual updates precede workflow logic for these states; navigation updates itself.
    switch (AsOpenItemMenuState(this)->state) {
    case kTourStartFadeOut:
    case kTourWaitFadeOut:
    case kTourEnableNavigation:
    case kTourStartPreview:
    case kTourWaitPreview:
    case kTourStartBeforeEvent:
    case kTourProcessBeforeEvent:
    case kTourPrepareRewards:
    case kTourProcessMatchReturn:
    case kTourWaitReturnAnimation:
    case kTourStartAfterEvent:
    case kTourProcessAfterEvent:
    case kTourStartCompletionEvent:
    case kTourProcessCompletionEvent:
    case kTourStartRewardDelivery:
    case kTourWaitRewardDelivery:
    case kTourOpenShop:
    case kTourWaitShopReady:
    case kTourOpenDescription:
    case kTourProcessDescription:
        UpdateItemMenuVisuals(delta);
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->Update(delta, &AsOpenItemMenuState(this)->cameraOffsetX);
        OpenItemMenuState_UpdateCameraOffsets(this);
        UpdateItemMapLayout();
        break;
    }
    // Transition policy: QueueTourPhase waits until the NEXT invocation; EnterTourPhaseNow
    // takes effect here and the explicitly adjacent case runs in THIS invocation.
    // Immediate pairs: StartFadeOut->WaitFadeOut, EnableNavigation->Navigate,
    // StartPreview->WaitPreview, OpenCharacterSelect->SelectCharacter,
    // OpenShop->WaitShopReady, OpenDescription->ProcessDescription, OpenSave->ProcessSave.
    // Deferred routes (each arrow is a later invocation):
    // load: archives -> map ready -> UI ready -> fade -> navigation
    // encounter: preview -> before event -> rewards/setup -> return 1, resume at MatchReturn
    // return: result animation -> after event -> completion event -> deliver rewards -> navigation
    // submenus: character/apply -> fade; shop -> close -> save -> apply; description -> navigation.
    // Nonzero Context.nextState overrides the normal post-fade destination. Result=2/-1
    // hands control to the caller for map change/exit, and clears that resume destination.
    // Advance the current workflow phase; begin/wait pairs deliberately share this tick.
    switch (AsOpenItemMenuState(this)->state) {
    // Loading: resolve resources, then wait until map and UI dependencies are ready.
    case kTourLoadArchives: { // Create archive wrappers -> queue WaitMapResources.
        TourMapResources mapResources[11];
        int sz0, sz1;
        void* d;
        void* o;
        struct T11 {
            TourMapResources entries[11];
        };
        *(T11*)mapResources = *(T11*)s_tourMapArchiveNames;
        sz0 = 0;
        sz1 = 0;
        EnsureTourResourceTable(base);
        d = ResourceTable_FindEntryByName(Sing_805DC620(), tourMapOpenItemStrings, &sz0);
        o = __nw__FUl(0x20);
        if (o)
            ((CPackTextureResource*)o)->Init((const char*)tourMapOpenItemStrings, (void**)sz0, (unsigned int)d);
        AsOpenItemMenuState(this)->mapTexture = o;
        EnsureTourResourceTable(base);
        d = ResourceTable_FindEntryByName(
            Sing_805DC620(), mapResources[AsOpenItemMenuState(this)->tourCharacterIndex].itemTextureArchive, &sz1);
        o = __nw__FUl(0x20);
        if (o)
            ((CPackTextureResource*)o)
                ->Init((const char*)(mapResources[AsOpenItemMenuState(this)->tourCharacterIndex].itemTextureArchive),
                       (void**)sz1, (unsigned int)d);
        AsOpenItemMenuState(this)->itemTexture = o;
        AsOpenItemMenuState(this)->mapPackedFile =
            CPackedFile::Create(tourMapOpenItemStrings + kTourText_tour_map_common_data_zip);
        AsOpenItemMenuState(this)->itemPackedFile =
            CPackedFile::Create((void*)mapResources[AsOpenItemMenuState(this)->tourCharacterIndex].itemDataArchive);
        QueueTourPhase(this, kTourWaitMapResources);
        break;
    }
    case kTourWaitMapResources: // Archive/UI creation ready -> queue WaitUiResources.
        if (PollItemMapResources())
            QueueTourPhase(this, kTourWaitUiResources);
        break;
    case kTourWaitUiResources: // All UI ready: initialize map -> queue StartFadeOut.
        if (OpenItemMenu_PollResourcesLoaded(this)) {
            InitItemMap(delta);
            QueueTourPhase(this, kTourStartFadeOut);
        }
        break;
    // Reveal the loaded map and restore navigation/script controls.
    case kTourStartFadeOut: // Enable layers/start fade -> enter WaitFadeOut now.
        SetTourMapLayersEnabled(this, 1, base);
        EnsureTourFadeOverlay(base);
        FadeState_StartFadeOut(Sing_805DC608(), kTourScreenFadeUpdates);
        EnterTourPhaseNow(this, kTourWaitFadeOut);
    // Fall through: poll the phase just entered in this invocation.
    case kTourWaitFadeOut: // Fade state != 3 -> queue context.nextState or EnableNavigation.
        EnsureTourMenuScript(base);
        ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(11);
        EnsureTourFadeOverlay(base);
        if (FadeState_GetState(Sing_805DC608()) != 3) {
            int n = AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->nextState;
            if (n)
                QueueTourPhase(this, n);
            else
                QueueTourPhase(this, kTourEnableNavigation);
        }
        break;
    case kTourEnableNavigation: // Enable controls/script -> enter Navigate now.
        Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(this)->layout, kTourShopControl, 1);
        Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(this)->layout, kTourShopPairedRegion, 1);
        Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(this)->layout, kTourCharacterControl, 1);
        Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(this)->layout, kTourCharacterPairedRegion, 1);
        Layout_SetElementEnabled(AsOpenItemMenuState(this)->layout, kTourCharacterPortrait, 1);
        EnsureTourMenuScript(base);
        ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(11);
        EnterTourPhaseNow(this, kTourNavigate);
    // Fall through: poll the phase just entered in this invocation.
    case kTourNavigate: // Input handler queues preview, shop, character, description or exit.
        UpdateItemMenuVisuals(delta);
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->Update(delta, &AsOpenItemMenuState(this)->cameraOffsetX);
        OpenItemMenuState_UpdateCameraOffsets(this);
        HandleItemMapNavigation();
        UpdateItemMapLayout();
        break;
    // Encounter entry: preview, optional before-event, rewards and player/hole setup.
    case kTourStartPreview: // Start action/voice -> enter WaitPreview now.
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))
            ->SetTargetHeading(kOpenItemCameraReverseHeading);
        ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->SetAnimationByIndex(4);
        // Keep tagged-string storage in this case; the shared helper preserves release-before-play.
        {
            S_ED074 eventSuffix;
            S_ED074 characterSuffix;
            S_ED074 resourceName;
            A_ED074 eventAllocator, characterAllocator, prefixAllocator;
            int characterIndex =
                (signed char)AsCCameraController(AsOpenItemMenuState(this)->cameraController)->modelIndex;

            char* text;
            PlayTourCharacterEventVoice(characterIndex, kTourVoicePreview, eventSuffix, characterSuffix, resourceName,
                                        eventAllocator, characterAllocator, prefixAllocator, text);
        }
        EnterTourPhaseNow(this, kTourWaitPreview);
    // Fall through: poll the phase just entered in this invocation.
    case kTourWaitPreview: // Action animation finished -> queue StartBeforeEvent.
        if (!((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->IsTourActionAnimation())
            QueueTourPhase(this, kTourStartBeforeEvent);
        break;
    case kTourStartBeforeEvent: { // No before-event -> queue PrepareRewards; otherwise ProcessBeforeEvent.
        void* eventRecord =
            *(void**)((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry();
        StartTourBeforeEvent(this, eventRecord);
        break;
    }
    case kTourProcessBeforeEvent: // Script +1 -> rewards; -1 -> navigation; +2 -> result 2/map change.
        PollTourBeforeEvent(this, result);
        break;
    // Queue only unclaimed rewards. Coin-threshold bundles are awarded
    // when this item's increment crosses a threshold, not on every visit.
    case kTourPrepareRewards: { // Queue unclaimed rewards; encounter -> EnterMatch, else StartAfterEvent.
        EnsureTourItemRequests(base);
        ItemRequestState_ClearEntries(Sing_805DC958());
        OpenItemMenu_QueueSelectedItemRewards(
            this, *(void**)((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry());
        if (!((SelectedMapItemFlags(this) >> 9) & 1)) {
            void* e = *(void**)((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry();
            int i = 0;
            short w = AsOpenItemRecord(e)->firstCompletionTourCoins;
            for (; i < (*(*(void***)&AsOpenItemMenuState(this)->coinItemData)
                            ? *(int*)((char*)*(*(void***)&AsOpenItemMenuState(this)->coinItemData) + 8)
                            : 0);
                 i++) {
                // original 802ED9D0 lwz r3,0(r3) with r3 = [self+0x38] (802EDA0C): the coin item table that bounds the loop, not e
                short* q = (short*)Table_GetEntry(*(*(void***)&AsOpenItemMenuState(this)->coinItemData), i);
                int lo = ((TourProgressFields*)AsOpenItemMenuState(this)->tourProgress)->coinCount;
                if (lo < *q && lo + w >= *q)
                    OpenItemMenu_QueueEquipmentRecordRequests(this, q);
            }
        }
        EnsureTourItemRequests(base);
        ItemRequestState_GetEntryCount(Sing_805DC958());
        if (!PrepareSelectedEncounter(
                ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry())) {
            void* e = ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry();
            if (!(*(int*)&AsCCameraController(AsOpenItemMenuState(this)->cameraController)->moving))
                OpenItemEntry_SetFlagBit15(e, 1);
            EnsureTourItemRequests(base);
            ItemRequestState_ApplyPendingRewards(Sing_805DC958());
            SelectedMapItemFlags(this) |= kRewardClaimed;
            QueueTourPhase(this, kTourStartAfterEvent);
            break;
        }
        EnsureTourFadeOverlay(base);
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        EnsureTourFadeOverlay(base);
        FadeState_StartFadeIn(Sing_805DC608(), 0);
        QueueTourPhase(this, kTourEnterMatch);
        break;
    }
    case kTourEnterMatch: { // Publish link class; result 1/encounter; context resumes ProcessMatchReturn.
        int key = AsOpenItemMenuState(this)->selectedItem;
        int it;
        unsigned char b1, b0;
        char* node;
        int f;
        void* m = IntegerKeyTree_FindInsertionPosition((void*)&((TourTreeFields*)this)->items, &key, &it, &b0, &b1);
        if (!it || ((TourTreeNodeFields*)it)->key < key) {
            int pr[2];
            pr[0] = key;
            pr[1] = (int)g_bpEntryDefaultMappedValue;
            node = (char*)BpEntryTree_InsertNode((void*)&((TourTreeFields*)this)->items, m, b0, b1, pr) + 0xc;
        } else
            node = (char*)&((TourTreeNodeFields*)it)->key;
        f = 0;
        {
            unsigned char matchTypeIndex = LinkActiveMatchTypeIndex(node);
            if (matchTypeIndex <= kLastLinkActiveMatchTypeIndex &&
                ((1 << matchTypeIndex) & kLinkActiveMatchTypeIndices))
                f = 1;
        }
        AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->linkActive = f;
        result = 1;
        AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->nextState = kTourProcessMatchReturn;
        break;
    }
    // Encounter return: choose success/failure animation and keep rewards only on success.
    case kTourProcessMatchReturn: // Choose success/failure feedback -> queue WaitReturnAnimation.
        AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->linkActive = 0;
        Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(this)->layout, kTourShopControl, 0);
        Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(this)->layout, kTourShopPairedRegion, 0);
        Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(this)->layout, kTourCharacterControl, 0);
        Layout_SetElementPlayerFlagsAndState(AsOpenItemMenuState(this)->layout, kTourCharacterPairedRegion, 0);
        Layout_SetElementEnabled(AsOpenItemMenuState(this)->layout, kTourCharacterPortrait, 0);
        EnsureTourMenuScript(base);
        ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
        if (SharedStringState_GetContext()) {
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->SetAnimationByIndex(2);
            EnsureTourItemRequests(base);
            if (ItemRequestState_GetEntryCount(Sing_805DC958()) > 0) {
                // Reward-success voice; keep string construction in the caller's scope.
                S_ED074 eventSuffix;
                S_ED074 characterSuffix;
                S_ED074 resourceName;
                A_ED074 eventAllocator, characterAllocator, prefixAllocator;
                int characterIndex =
                    (signed char)AsCCameraController(AsOpenItemMenuState(this)->cameraController)->modelIndex;

                char* text;
                PlayTourCharacterEventVoice(characterIndex, kTourVoiceRewardSuccess, eventSuffix, characterSuffix,
                                            resourceName, eventAllocator, characterAllocator, prefixAllocator, text);
            } else {
                // Ordinary success voice; preserve the original release-before-play order.
                S_ED074 eventSuffix;
                S_ED074 characterSuffix;
                S_ED074 resourceName;
                A_ED074 eventAllocator, characterAllocator, prefixAllocator;
                int characterIndex =
                    (signed char)AsCCameraController(AsOpenItemMenuState(this)->cameraController)->modelIndex;

                char* text;
                PlayTourCharacterEventVoice(characterIndex, kTourVoiceSuccess, eventSuffix, characterSuffix,
                                            resourceName, eventAllocator, characterAllocator, prefixAllocator, text);
            }
        } else {
            EnsureTourItemRequests(base);
            ItemRequestState_ClearEntries(Sing_805DC958());
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->SetAnimationByIndex(3);
            // Keep tagged-string storage here while sharing voice construction.
            {
                S_ED074 eventSuffix;
                S_ED074 characterSuffix;
                S_ED074 resourceName;
                A_ED074 eventAllocator, characterAllocator, prefixAllocator;
                int characterIndex =
                    (signed char)AsCCameraController(AsOpenItemMenuState(this)->cameraController)->modelIndex;

                char* text;
                PlayTourCharacterEventVoice(characterIndex, kTourVoiceFailure, eventSuffix, characterSuffix,
                                            resourceName, eventAllocator, characterAllocator, prefixAllocator, text);
            }
        }
        QueueTourPhase(this, kTourWaitReturnAnimation);
        break;
    case kTourWaitReturnAnimation: // Animation done -> after-event on success/party, else navigation.
        if (!((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->IsEitherActionAnimation()) {
            if (SharedStringState_GetContext() || (*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode))
                QueueTourPhase(this, kTourStartAfterEvent);
            else
                QueueTourPhase(this, kTourEnableNavigation);
        }
        break;
    // Post-encounter events complete before queued reward delivery begins.
    case kTourStartAfterEvent: { // No after-event -> completion; otherwise queue ProcessAfterEvent.
        void* eventRecord =
            *(void**)((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry();
        StartTourAfterEvent(this, eventRecord);
        break;
    }
    case kTourProcessAfterEvent: // Script +1 -> completion; -1 -> navigation; zero keeps waiting.
        PollTourAfterEvent(this);
        break;
    case kTourStartCompletionEvent: // Start completion effect 1 -> queue ProcessCompletionEvent.
        TourEventMessage_ReplaceEffect(AsOpenItemMenuState(this)->messageFile, 1);
        QueueTourPhase(this, kTourProcessCompletionEvent);
        break;
    case kTourProcessCompletionEvent: // Script +1 -> rewards; cancel/party-tier-zero -> navigation.
        PollTourCompletionEvent(this);
        break;
    // Start and poll the request-state reward UI; clear the item-enabled flag when done.
    case kTourStartRewardDelivery: // Requests exist -> queue WaitRewardDelivery; otherwise navigation.
        StartTourRewardDelivery(this, base);
        break;
    case kTourWaitRewardDelivery: // Delivery complete: clear processing/item-enabled -> navigation.
        PollTourRewardDelivery(this, base);
        break;
    // Character selection hides map layers, applies the save snapshot and reloads preview.
    case kTourOpenCharacterSelect: // Window ready: hide layers/open selection -> SelectCharacter now.
        if (!((CTourWindow*)(AsOpenItemMenuState(this)->tourWindow))->IsLoadComplete())
            break;
        EnsureTourMenuScript(base);
        ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(13);
        SetTourMapLayersEnabled(this, 0, base);
        ((CTourWindow*)(AsOpenItemMenuState(this)->tourWindow))->InitializeCharacterSelection();
        EnterTourPhaseNow(this, kTourSelectCharacter);
    // Fall through: poll the phase just entered in this invocation.
    case kTourSelectCharacter: // Selection accepted -> queue ApplyCharacterSelection.
        if (((CTourWindow*)(AsOpenItemMenuState(this)->tourWindow))->UpdateSelectionHover(delta)) {
            TourItemMenu_DisableRegisteredObject(AsOpenItemMenuState(this)->tourWindow);
            EnsureTourMenuScript(base);
            ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
            QueueTourPhase(this, kTourApplyCharacterSelection);
        }
        break;
    case kTourApplyCharacterSelection: // Model changed -> WaitCharacterPreview; unchanged -> StartFadeOut.
        ApplyTourCharacterSelection(this);
        break;
    case kTourWaitCharacterPreview: // Preview ready: reset/voice -> queue StartFadeOut.
        if (CameraController_PollPreview(AsOpenItemMenuState(this)->cameraController)) {
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->ResetPreview();
            ((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->SetAnimationByIndex(4);
            // Keep tagged-string storage here while sharing voice construction.
            {
                S_ED074 eventSuffix;
                S_ED074 characterSuffix;
                S_ED074 resourceName;
                A_ED074 eventAllocator, characterAllocator, prefixAllocator;
                int characterIndex =
                    (signed char)AsCCameraController(AsOpenItemMenuState(this)->cameraController)->modelIndex;

                char* text;
                PlayTourCharacterEventVoice(characterIndex, kTourVoiceCharacterChanged, eventSuffix, characterSuffix,
                                            resourceName, eventAllocator, characterAllocator, prefixAllocator, text);
            }
            AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->nextState = 0;
            QueueTourPhase(this, kTourStartFadeOut);
        }
        break;
    // Shop owns a working snapshot of context.saveRecord; copy it back on shop completion.
    case kTourOpenShop: { // Fade/create shop snapshot -> enter WaitShopReady now.
        void* o;
        EnsureTourFadeOverlay(base);
        FadeState_RequestFramebufferCapture(Sing_805DC608());
        EnsureTourFadeOverlay(base);
        FadeState_StartFadeIn(Sing_805DC608(), 0);
        DeleteTourMenuObject(&AsOpenItemMenuState(this)->shop);
        o = __nw__FUl(0x8e0);
        if (o)
            ((CPangya2Shop*)o)->Init(AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->saveRecord, 0);
        AsOpenItemMenuState(this)->shop = o;
        EnterTourPhaseNow(this, kTourWaitShopReady);
    }
    case kTourWaitShopReady: // Shop initialized -> hide layers and queue ProcessShop.
        MPBase1_Process(AsOpenItemMenuState(this)->shop, delta);
        if (*(int*)((char*)AsOpenItemMenuState(this)->shop + 8)) {
            SetTourMapLayersEnabled(this, 0, base);
            QueueTourPhase(this, kTourProcessShop);
        }
        break;
    case kTourProcessShop: // Shop done: copy snapshot to save record/fade -> queue CloseShop.
        if (MPBase1_Process(AsOpenItemMenuState(this)->shop, delta)) {
            CPangya2Shop::CopySnapshotToRecord(
                AsOpenItemMenuState(this)->shop,
                AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->saveRecord);
            EnsureTourFadeOverlay(base);
            FadeState_RequestFramebufferCapture(Sing_805DC608());
            EnsureTourFadeOverlay(base);
            FadeState_StartFadeIn(Sing_805DC608(), 0);
            QueueTourPhase(this, kTourCloseShop);
        }
        break;
    case kTourCloseShop: // Delete shop -> queue OpenSave.
        DeleteTourMenuObject(&AsOpenItemMenuState(this)->shop);
        QueueTourPhase(this, kTourOpenSave);
        break;
    // Description owns its message holder only while the hint is open.
    case kTourOpenDescription: { // Create message holder/show hint -> enter ProcessDescription now.
        void* o = __nw__FUl(0xc);
        if (o)
            MsgResourceHolder_Ctor(o);
        AsOpenItemMenuState(this)->messageResourceHolder = o;
        TourPosition_ShowDescriptionHint(
            AsOpenItemMenuState(this)->messageResourceHolder,
            *(void**)((CCameraController*)(AsOpenItemMenuState(this)->cameraController))->GetItemEntry());
        TourHintWindow_StartGlobalFadeIn(AsOpenItemMenuState(this)->messageResourceHolder);
        EnterTourPhaseNow(this, kTourProcessDescription);
    }
    case kTourProcessDescription: // Hint done: delete holder -> queue EnableNavigation.
        if (TourHint_UpdateFadeAndInput(AsOpenItemMenuState(this)->messageResourceHolder)) {
            TourHintWindow_ClearGlobalMessageAndObjects(AsOpenItemMenuState(this)->messageResourceHolder);
            if (AsOpenItemMenuState(this)->messageResourceHolder)
                MsgResourceHolder_Dtor(AsOpenItemMenuState(this)->messageResourceHolder, 1);
            AsOpenItemMenuState(this)->messageResourceHolder = 0;
            QueueTourPhase(this, kTourEnableNavigation);
        }
        break;
    // Save submits the working record, then refreshes it from the current saved snapshot.
    case kTourOpenSave: // Submit working save record/create save UI -> ProcessSave now.
        EnsureTourSaveData(base);
        SaveData_SetCurrentRecord(GetSaveDataSlot(),
                                  AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->saveRecord);
        DeleteTourMenuObject(&AsOpenItemMenuState(this)->flashMemorySaveMenu);
        AsOpenItemMenuState(this)->flashMemorySaveMenu = CFlashMemoryMenuSave::Create();
        EnterTourPhaseNow(this, kTourProcessSave);
    // Fall through: poll the phase just entered in this invocation.
    case kTourProcessSave: // Save UI done: copy current saved record -> ApplyCharacterSelection.
        if (MPBase1_Process(AsOpenItemMenuState(this)->flashMemorySaveMenu, delta)) {
            // Copy view of SavePlayerRecord; word/halfword groups preserve CodeWarrior copy order.
            // throughUnlockedCourses covers equipment +0x2C and course bits +0x258.
            struct TourSnapshotCopy {
                int throughUnlockedCourses[151];
                int words25C[28];
                short halfwords2CC[208];
                int words46C[16];
                char bytes4AC[24];
                char byte4C4, byte4C5, byte4C6, byte4C7, byte4C8, byte4C9;
                char bytes4CA[8];
                char byte4D2, byte4D3, byte4D4;
                int words4D8[5];
                short halfwords4EC[414];
            };
            typedef char Snapshot_size[(sizeof(TourSnapshotCopy) == sizeof(SavePlayerRecord)) ? 1 : -1];
            DeleteTourMenuObject(&AsOpenItemMenuState(this)->flashMemorySaveMenu);
            EnsureTourSaveData(base);
            *(TourSnapshotCopy*)AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->saveRecord =
                *(TourSnapshotCopy*)SaveData_GetCurrentRecord(GetSaveDataSlot());
            QueueTourPhase(this, kTourApplyCharacterSelection);
        }
        break;
    case kTourExit: // Clear context resume state; result -1/leave tour.
        result = -1;
        AsOpenItemMenuContext(AsOpenItemMenuState(this)->menuContext)->nextState = 0;
        break;
    }
    return result;
}

} // extern "C"

// Verify the partial views against the offsets used by this unit.
typedef char InventoryReward_count_offset[((int)&((InventoryReward*)0)->count == 0x2) ? 1 : -1];
typedef char TourRewardFields_holeNumbers_offset[((int)&((TourRewardFields*)0)->holeNumbers == 0x44) ? 1 : -1];
typedef char TourRewardFields_setItems_offset[((int)&((TourRewardFields*)0)->setItems == 0x5c) ? 1 : -1];
typedef char TourRewardFields_parts_offset[((int)&((TourRewardFields*)0)->parts == 0x6c) ? 1 : -1];
typedef char CoinEquipmentFields_caddieItems_offset[((int)&((CoinEquipmentFields*)0)->caddieItems == 0xe) ? 1 : -1];
typedef char CoinEquipmentFields_setItems_offset[((int)&((CoinEquipmentFields*)0)->setItems == 0x1a) ? 1 : -1];
typedef char TourSelectionFields_items_offset[((int)&((TourSelectionFields*)0)->items == 0x4) ? 1 : -1];
typedef char TourProgressFields_mapHistory_offset[((int)&((TourProgressFields*)0)->mapHistory == 0x32e) ? 1 : -1];
typedef char TourProgressFields_historyIndex_offset[((int)&((TourProgressFields*)0)->historyIndex == 0x336) ? 1 : -1];
typedef char TourProgressFields_coinCount_offset[((int)&((TourProgressFields*)0)->coinCount == 0x338) ? 1 : -1];
typedef char
    TourProgressFields_completedEventFlags_offset[((int)&((TourProgressFields*)0)->completedEventFlags == 0x33a) ? 1
                                                                                                                 : -1];
typedef char TourTreeFields_items_offset[((int)&((TourTreeFields*)0)->items == 0x10) ? 1 : -1];
typedef char TourTreeNodeFields_key_offset[((int)&((TourTreeNodeFields*)0)->key == 0xc) ? 1 : -1];
typedef char TourMapRecordPrefix_iconKind_offset[((int)&((TourMapRecordPrefix*)0)->iconKind == 0xa) ? 1 : -1];
typedef char TourMapRecordPrefix_beforeEvent_offset[((int)&((TourMapRecordPrefix*)0)->beforeEvent == 0x10) ? 1 : -1];
typedef char TourMapRecordPrefix_afterEvent_offset[((int)&((TourMapRecordPrefix*)0)->afterEvent == 0x24) ? 1 : -1];
typedef char TourEventFields_itemEvent_offset[((int)&((TourEventFields*)0)->itemEvent == 0x14) ? 1 : -1];
typedef char TourEventFields_itemFrames_offset[((int)&((TourEventFields*)0)->itemFrames == 0x20) ? 1 : -1];
typedef char TourRenderContextFields_vectors_offset[((int)&((TourRenderContextFields*)0)->vectors == 0x20a0) ? 1 : -1];
typedef char TourRenderContextFields_records_offset[((int)&((TourRenderContextFields*)0)->records == 0x20d0) ? 1 : -1];
typedef char TourRenderContextFields_count_offset[((int)&((TourRenderContextFields*)0)->count == 0x2360) ? 1 : -1];
