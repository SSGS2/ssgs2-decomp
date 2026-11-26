// In-round HUD: distance-to-pin/party points, cup height indicators and atlas text.
// Widescreen lookup (all scalar definitions are in the data section):
// - Distance anchor: SetScreenAnchor => (0.5*viewportWidth+20,
//   kFloat_0_25_HudShared*viewportHeight+25); factor baseline .25.
//   WView viewport units feed layout/screen coordinates; X right, Y down in quads.
// - Pin placement: CHoleCupInfo::UpdateScreenPositionAndDraw projects world pin via WView inputTransform
//   then ProjectViewPoint. X is projected; Y is fixed kCupHudFixedScreenY=90 unless
//   the ENTIRE state word equals 0x200. This is not a viewport/scissor setter.
// - Height text: X+16; Y offsets level/above/below=26/17/37; indicator is 64x64
//   centered at X (half-width 32), Y-47. Keep text and marker adjustments together.
// - Font placement: HudFont_DrawAlignedText takes character SPACING and scale;
//   kHudNumericCharacterSpacingFactor=6 multiplies display scale at callers, then the
//   helper multiplies spacing by scale again. It does not set glyph width.
// - Frame rate: distance fade=510 alpha units/second, hold=2 seconds, move=.5
//   seconds. Delta drives these; tutorial/input/visibility can interrupt the cycle.
// - Distance conversion: world horizontal length/3.2 gives yards; cup height then
//   multiplies kYardsToMeters_HoleCupHud. Do not scale world data for widescreen.
// Preserve projection/cull order, exact-state tests, glyph advance/spacing order,
// byte-color packing, float precision and modal child update-before-visibility.
// WView, CameraTransform800D and CGolfDoc headers define shared fields. Opaque
// scratch views here sometimes exceed xyz; the called point transform writes xyz.
// All receivers/outputs require live storage except documented null cases/dtors.
// UI/render methods borrow the active view/document and require valid player/hole
// indices. Returned resource addresses are not transferable ownership.
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/CameraTransform800D.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
// Original unit 0x800F30AC-0x800F47F4: manager mode checks read CSharedDoc +0xAD8.
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))

#include "game/types.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CInputManager.h"
#include "classes/CFontNumDraw.h"
#include "classes/CDistToHoleInfo.h"
#include "classes/CHoleCupInfo.h"
#include "classes/CInfoBase.h"
#include "classes/CClubRangeInfo.h"

// Acquire the SDK mutex through the recovered untyped ABI.
inline void OSLockMutex(void* p) {
    OSLockMutex((OSMutex*)p);
}
// Release the SDK mutex through the recovered untyped ABI.
inline void OSUnlockMutex(void* p) {
    OSUnlockMutex((OSMutex*)p);
}
// Search borrowed text through the recovered untyped ABI.
inline void* strchr(void* p, int c) {
    return strchr((const char*)p, c);
}

extern "C" {

enum DistanceHudState {
    kDistanceAtPin = 0,
    kDistanceMoveToPin = 1,
    kDistanceFadeIn = 2,
    kDistanceFadeOut = 3,
    kDistanceHold = 4,
    kDistanceHidden = 5
};
enum CupHeightDirection {
    kCupBelow = 0,
    kCupLevel = 1,
    kCupAbove = 2
};
enum TextAlignment {
    kTextLeft = 0,
    kTextCentered = 1,
    kTextRight = 2
};
// The Wii draws this word (alpha<<24 | 0xFF3232) red.
enum {
    kPartyPointsMode = 2,
    kDistanceColorRgb = 0xFF3232,
    kGolfStateHudPaused = 2,
    kGolfStatesResetDistance = 0x8210,
    kGolfStatePointerLocked = 4,
    kGolfStateUseProjectedY = 0x200
};
enum DistanceHudStorage {
    kHudNumericTextBufferBytes = 0x20
};
enum FontRegistryConstants {
    kRegisteredFontConfigurationCount = 0x1A,
    kFontNameHashLengthShift = 6,
    kFontNameFirstByteHashMask = 0x2A,
    kFontNameLastByteHashMask = 0x15,
    kRegisteredFontInfoBytes = 0x48,
    kFontInfoPrefixClearBytes = 0x20,
    kFontRegistryNodeBytes = 0x18
};
// This file contains several offset-specific views of the same HUD objects.
// The +0x1C state/+0x20 timer/+0x24 interpolation prefix is shared; do not merge
// partial views by concatenating them. The draw view's font pair is borrowed
// while drawing; the corresponding destructor releases the retained font.
struct HudPlayerEntry {
    char reserved_00[0x1C];
    Vec3f position;
    unsigned char holeStrokeCounts[18];
    char reserved_3A[0x13E];
    float partyPoints;
};
struct HudFontPair {
    void* font;
    void* configuration;
};
struct HudTextState {
    void* vtable;
    HudFontPair defaultFont;
    unsigned int color;
    int alignment;
};
typedef char HudPlayerEntry_points_offset[((int)&((HudPlayerEntry*)0)->partyPoints == 0x178) ? 1 : -1];
struct HudFontConfig {
    char reserved_00[0x1C];
    const char* glyphOrder;
};
struct HudFontScale {
    char reserved_00[0x38];
    float scale;
};

struct HoleCupScreenPosition {
    char reserved_00[0x18];
    float screenX;
    float screenY;
};

// Copy the last drawn cup-indicator screen anchor to two required float outputs.
// No projection occurs here; values can be stale when the draw path was culled.
void CHoleCupInfo::GetScreenPosition(float* screenX, float* screenY) {
    HoleCupScreenPosition* self = (HoleCupScreenPosition*)this;
    *screenX = self->screenX;
    *screenY = self->screenY;
}

extern WView* g_view;
extern float kFloat_0_5_hud;
extern float kFloat_20_MessageUI;
extern float kFloat_0_25_HudShared;
extern float kFloat_25_805DE368;

struct DistanceHudAnchor {
    char reserved_00[0x28];
    float startX;
    float startY;
};

#pragma opt_lifetimes off
// Set the distance display's start anchor in viewport-derived UI coordinates.
// Writes startX/startY only; no viewport or projection state changes. The .5 word
// is shared with text centering and timing; .25 is also used by HUD animation in
// golf_hud_flight_info_and_pang_rewards.cpp. A future mod needing independent values should introduce new storage.
void CDistToHoleInfo::SetScreenAnchor() {
    DistanceHudAnchor* self = (DistanceHudAnchor*)this;
    WView* view;
    float margin;
    float verticalAnchorFactor;
    float position;
    float viewportWidth;
    view = g_view;
    position = kFloat_0_5_hud;
    viewportWidth = view->viewportWidth;
    margin = kFloat_20_MessageUI;
    verticalAnchorFactor = kFloat_0_25_HudShared;
    position = position * viewportWidth + margin;
    margin = kFloat_25_805DE368;
    self->startX = position;
    view = g_view;
    position = view->viewportHeight;
    self->startY = verticalAnchorFactor * position + margin;
}
#pragma opt_lifetimes reset

struct DistanceHudTransition {
    char reserved_00[0x1C];
    int state;
    char reserved_20[0x14];
    float endX;
    float endY;
    float endScale;
    float currentX;
    float currentY;
    float currentScale;
    float alpha;
};

struct HoleCupDistanceOverlay {
    char reserved_00[0x34];
    DistanceHudTransition* child;
};

extern float kFloat_255_MessageUI;

// Snap the child distance overlay to its end position at full opacity.
void CHoleCupInfo::SnapDistanceOverlay() {
    HoleCupDistanceOverlay* self = (HoleCupDistanceOverlay*)this;
    DistanceHudTransition* child = self->child;
    if (child != 0) {
        child->alpha = kFloat_255_MessageUI;
        child->state = 0;
        child->currentX = child->endX;
        child->currentY = child->endY;
        child->currentScale = child->endScale;
    }
}

void HudFont_DrawAlignedText(void*, const char*, float, float, float, float, void*, void*);

struct DistanceHudDrawState {
    char reserved_00[0x0C];
    unsigned int color;
    int alignment;
    void* textBuffer;
    float displayValue;
    int state;
    char reserved_20[4];
    float interpolation;
    float startX, startY, startScale;
    float endX, endY, endScale;
    float currentX, currentY, currentScale;
    float alpha;
    float projectedXOffset;
    float reserved_54;
    HudFontPair alternateFont;
    float reserved_60;
    float reserved_64;
};
extern CSharedDoc* g_sharedDoc;
struct HolePinWords {
    unsigned int reserved_00[3];
    unsigned int x, y, z;
    char reserved_18[8];
};
struct VectorWords {
    unsigned int v[3];
};
// VectorWords copies three float bit patterns without conversion. TransformStorage is
// an ABI view, not the destination's size: the point transform writes only xyz here.
// Pin pipeline: hole record xyz (world) -> inputTransform -> view xyz -> projected xy.
struct TransformStorage {
    char data[0x30];
};
struct ProjectedPointStorage {
    float x, y, z, w;
    float unknown[10];
};
struct Matrix3x3 {
    float m[9];
};
void* __mi__FRC7WVectorRC7WVector(Vec3f*, Vec3f*, Vec3f*);
float __ml__FRC7WVectorRC7WVector(void*, void*);
extern float kPartyPointsOneDecimalThreshold;
extern float kHudNumericCharacterSpacingFactor;
extern float kFloat_20_MessageUI;
extern float kFloat_0_hud;
extern const float kFloat_1_805DE314;
extern const char s_cameraTransform[];
extern const char golfHudFontStrings[];

// Packed string-pool view; sizes include terminators and intervening bytes.
struct DistanceHudStrings {
    char prefix[0x41];
    char windFont[0xF]; // +0x41
    char reserved_50[0x35];
    char largeFont[0xD]; // +0x85
    char reserved_92[0x164];
    char distanceFont[0xD]; // +0x1F6
    char reserved_203[0x24];
    char pointsUnit[0x2];          // +0x227
    char pointsUnitFont[0x11];     // +0x229
    char pointsTwoDecimals[0x6];   // +0x23A
    char pointsOneDecimal[0x6];    // +0x240
    char distanceOneDecimal[0x7];  // +0x246
    char yardsUnit[0x2];           // +0x24D
    char distanceWholeNumber[0x7]; // +0x24F
    char reserved_256[0x30];
    char heightOneDecimal[0x6];  // +0x286
    char heightNearZero[0x6];    // +0x28C
    char heightTwoDecimals[0x6]; // +0x292
};
// Borrow named ranges within the existing HUD string pool.
inline const DistanceHudStrings& HudStrings() {
    return *(const DistanceHudStrings*)golfHudFontStrings;
}

void* Matrix_TransformPoint(TransformStorage*, Vec3f*, void*);
void WView_ProjectViewPoint(void*, ProjectedPointStorage*, TransformStorage*, int);

#pragma opt_propagation off
#pragma opt_lifetimes off
// Draw distance or party points, projecting the pin and interpolating HUD position and scale.
// Hidden returns untouched. AtPin culls behind-camera pins; PartyPoints AtPin
// also draws nothing. Otherwise writes color/alignment/text and current pose.
// Projection is world -> inputTransform -> view -> projected xy. The transition
// blends X toward pinX+9, Y toward endY, and scale toward endScale; exact state
// 0x200 overrides rendered Y with projected Y. Normal draws retain endY (91).
// Fonts/text are borrowed while drawing. Render state changes are delegated;
// font scale is restored by HudFont_DrawAlignedText, color/alignment are retained.
void CDistToHoleInfo::Draw() {
    DistanceHudDrawState* self = (DistanceHudDrawState*)this;
    float total, one, inverse, weight;
    if (self->state == kDistanceHidden)
        return;
    if (self->state == kDistanceAtPin) {
        CSharedDoc* global = (CSharedDoc*)g_sharedDoc;
        CGolfDoc* golfDoc = (CGolfDoc*)global->golfDoc;
        HolePinWords* record = (HolePinWords*)golfDoc->holeRecords + golfDoc->currentHoleIndex;
        WVector position;
        Vec3f origin;
        *(VectorWords*)&origin = *(VectorWords*)&record->x;
        __mi__FRC7WVectorRC7WVector((Vec3f*)&position, (Vec3f*)&origin,
                                    (Vec3f*)AsCameraTransform800D(s_cameraTransform)->position);
        if ((position * *(WVector*)((void*)&AsCameraTransform800D(s_cameraTransform)->basis[6])) < kFloat_0_hud)
            return;
    }
    int alpha = (int)self->alpha;
    unsigned int color = ((unsigned int)alpha << 24) | kDistanceColorRgb;
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyPointsMode) {
        if (self->state == kDistanceAtPin)
            return;
        float points = *(volatile float*)&self->displayValue;
        float pointsPrecisionThreshold = *(volatile float*)&kPartyPointsOneDecimalThreshold;
        self->color = color;
        float twenty = kFloat_20_MessageUI;
        self->alignment = 2;
        if (points < pointsPrecisionThreshold)
            sprintf((char*)self->textBuffer, HudStrings().pointsTwoDecimals, points);
        else
            sprintf((char*)self->textBuffer, HudStrings().pointsOneDecimal, points);
        float pointsX = self->startX + twenty;
        float pointsY = self->startY;
        float pointsSpacing = kHudNumericCharacterSpacingFactor * self->startScale;
        float pointsHeight = self->startScale;
        const char* pointsText = (const char*)self->textBuffer;
        HudFont_DrawAlignedText(self, pointsText, pointsX, pointsY, pointsSpacing, pointsHeight, 0, 0);
        self->alignment = 0;
        sprintf((char*)self->textBuffer, HudStrings().pointsUnit);
        float unitX = self->endX + twenty;
        float unitY = self->endY;
        float unitSpacing = kHudNumericCharacterSpacingFactor * self->endScale;
        float unitHeight = self->endScale;
        const char* unitText = (const char*)self->textBuffer;
        HudFont_DrawAlignedText(self, unitText, unitX, unitY, unitSpacing, unitHeight, 0, &self->alternateFont);
        return;
    }
    self->color = color;
    // The transform output is a point in view space, despite its storage-view cast.
    Vec3f pinViewPosition;
    ProjectedPointStorage transformed;
    CSharedDoc* global = (CSharedDoc*)g_sharedDoc;
    CGolfDoc* golfDoc = (CGolfDoc*)global->golfDoc;
    HolePinWords* record = (HolePinWords*)golfDoc->holeRecords + golfDoc->currentHoleIndex;
    Vec3f origin;
    *(VectorWords*)&origin = *(VectorWords*)&record->x;
    Matrix_TransformPoint((TransformStorage*)&pinViewPosition, &origin, (char*)AsWView(g_view)->inputTransform);
    WView_ProjectViewPoint((void*)g_view, &transformed, (TransformStorage*)&pinViewPosition, 0);
    one = kFloat_1_805DE314;
    weight = self->interpolation;
    inverse = one - weight;
    total = transformed.x + self->projectedXOffset;
    self->currentX = inverse * self->startX + weight * total;
    self->currentY = inverse * self->startY + weight * self->endY;
    self->currentScale = inverse * self->startScale + weight * self->endScale;
    float yArg = self->currentY;
    if ((unsigned int)((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->gameStateFlags == kGolfStateUseProjectedY)
        yArg = transformed.y;
    float drawX = self->currentX;
    float drawScale = self->currentScale;
    float drawSpacing = kHudNumericCharacterSpacingFactor * drawScale;
    const char* distanceText = (const char*)self->textBuffer;
    HudFont_DrawAlignedText(self, distanceText, drawX, yArg, drawSpacing, drawScale, 0, 0);
}

#pragma opt_propagation reset
#pragma opt_lifetimes reset

struct HudFontSelection {
    char reserved_00[0x0C];
    int color;
    int alignment;
};
struct SelectedFontPair {
    void* volatile atlas;
    void* configuration;
};
// Lookup overlay: +0 is a configuration pointer in hash nodes; returned configurations expose +0x44.
struct FontRegistryLookupView {
    FontRegistryLookupView* configuration;
    const char* name;
    char reserved_08[0x0C];
    FontRegistryLookupView* bucketNext;
    char reserved_18[0x2C];
    int halfScale;
};
struct FontRegistryLookupTable {
    char reserved_00[0x14];
    unsigned int bucketMask;
    char reserved_18[4];
    FontRegistryLookupView** buckets;
    char mutexStorage[4];
    FontRegistryLookupView* find(const char*);
};
extern FontRegistryLookupTable g_hudFontConfigurationRegistry;
extern void* g_resourceManager;
extern float kFloat_0_5_hud;
void TableLock_ctor(void*, int, int);
void __dt__9TableLockFv(void*, int);
int ResourceManager_CreateWTitleFont(void*);
void FontAtlas_InitImagesAndGlyphWidths(void*, void*, int, float);
int strcmp(const char*, const char*);
float WFont_DrawAtlasGlyph(void*, void*, void*, float, float, void*, void*);
float WTitleFont_GetAtlasGlyphAdvance(void*, void*, void*);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
void* appMallocUni__FUl(unsigned int);
void* memset(void*, int, unsigned int);

struct RegisteredFontInfo {
    const char** imageNames; // 0x00: address of the recognized name-table element
    int imageCount;          // 0x04: count, never a resource-name pointer
    int cellWidth;           // 0x08: pixels
    int cellHeight;          // 0x0C: pixels
    int atlasWidth;          // 0x10: pixels
    int atlasHeight;         // 0x14: pixels
    int configuredRows;      // 0x18: zeroed metadata; not the scan-loop row count
    const char* glyphOrder;  // 0x1C: borrowed single-byte character lookup
    int cachedIndex;         // 0x20
    char name[0x20];         // 0x24
    int halfScale;           // 0x44
};

struct FontRegistryNode {
    RegisteredFontInfo* configuration; // 0x00
    const char* name;                  // 0x04
    unsigned char flags;               // 0x08
    char reserved_09[3];
    FontRegistryNode* volatile next; // 0x0C
    FontRegistryNode* volatile prev; // 0x10
    FontRegistryNode* bucketNext;    // 0x14
};

struct FontRegistryTable {
    FontRegistryNode* activeNodes; // 0x00
    char reserved_04[4];
    FontRegistryNode* allocatedBlocks; // 0x08
    FontRegistryNode* freeNodes;       // 0x0C
    int nodesPerBlock;                 // 0x10
    unsigned int bucketMask;           // 0x14
    char reserved_18[4];
    FontRegistryNode** buckets; // 0x1C
    char mutexStorage[4];       // 0x20
};

struct InstanceLock_u800F30AC {
    void* lock;
    // Hold one recursive mutex acquisition for this scope.
    InstanceLock_u800F30AC(void* l) {
        lock = l;
        OSLockMutex(l);
    }
    // Release this scope's acquisition.
    ~InstanceLock_u800F30AC() {
        OSUnlockMutex(lock);
    }
};

struct TableLock {
    char data[8];
    // Enter the table-level synchronization scope.
    TableLock(int a, int b) {
        TableLock_ctor(this, a, b);
    }
    ~TableLock();
};

struct G0Lock {
    volatile void* lock;
    // Acquire the outer search lock; callers must release it explicitly.
    G0Lock(void* l) {
        lock = l;
        OSLockMutex(l);
    }
    // No automatic unlock: each search exit performs its own release.
    ~G0Lock() {}
    // Release the acquisition made by this guard's constructor.
    void unlock() {
        OSUnlockMutex((void*)lock);
    }
};

// Splice a borrowed node after head under the mutex; return the circular-list head.
inline FontRegistryNode* FontRegistry_InsertAfter(void* lock, FontRegistryNode* head, FontRegistryNode* node) {
    void* lk = lock;
    OSLockMutex(lk);
    if (!head) {
        node->next = node;
        node->prev = node;
        OSUnlockMutex(lk);
        return node;
    }
    node->prev = head;
    node->next = head->next;
    head->next = node;
    node->next->prev = node;
    OSUnlockMutex(lk);
    return head;
}

// Unlink a node without freeing it; return its previous node, or null for a singleton.
inline FontRegistryNode* FontRegistry_Detach(void* lock, FontRegistryNode* n) {
    FontRegistryNode* nx;
    void* lk = lock;
    OSLockMutex(lk);
    n->prev->next = n->next;
    n->next->prev = n->prev;
    nx = n->prev;
    if (nx == n)
        nx = 0;
    OSUnlockMutex(lk);
    return nx;
}

extern const char* g_hudFontConfigurationNames[0x1A];

#pragma push
#pragma use_lmw_stmw on
// Atlas schema: the first 0x20 bytes are InitArg in title_font_atlas_and_fixed_font.cpp, consumed by
// FontAtlas_InitImagesAndGlyphWidths through CFontNumDraw::SelectFont below.
// CInfoBase construction in golf_hud_spot_markers_and_fonts.cpp supplies count=1 and halfScale=0 for all
// 23 registrations. imageNames points INTO g_hudFontConfigurationNames at the
// matched index, not to the name argument. count must fit the available suffix
// of that 26-pointer array; each entry must name an atlas image. Cell sizes must
// be positive; atlas extents must fit the loaded image and contain a full cell.
// Partial right/bottom cells are discarded; cell/image/count/byte products must
// fit signed int and 32-bit allocation sizes. glyphOrder is NUL-terminated bytes
// in image, row, column traversal order; every found index must fit the generated
// advance arrays. Keep dimensions, image order and character order together.
// halfScale affects the configured width of the first ten glyphs (0=>1, else .5);
// it does not halve atlas dimensions. configuredRows stays zero here.
// Cache a recognized font configuration in the locked registry, ignoring duplicates.
// First ABI receiver is unused. name is required NUL-terminated text <=31 bytes
// (copied into name[32] without a bounds check). glyphOrder is retained borrowed
// until all fonts using the configuration are released. Dimensions/count are
// integer values in GPR ABI slots, not pointers, despite the earlier types.
// halfScale is zero/nonzero. Duplicate or unrecognized names publish nothing;
// the latter frees its temporary info. No status is returned and allocations are
// unchecked. Accepted info and node become registry-owned; node blocks are cached.
// Initialized registry requires a valid bucket table/mask and nodesPerBlock>1;
// the first slot of each new block tracks allocation, remaining slots are free.
// The lock is recursive; retain acquisition/release order in the phases below.
// This function declares font metadata, not a font atlas or projection setting.
void CInfoBase::RegisterNamedInfo(const char* name, int cellWidth, int cellHeight, int imageCount, int atlasWidth,
                                  int atlasHeight, const char* glyphOrder, int halfScale) {
    FontRegistryTable* registry;
    FontRegistryTable* table;
    FontRegistryNode* found;
    FontRegistryNode* allocatedBlock;
    RegisteredFontInfo* entry;
    unsigned int typeIndex;
    FontRegistryNode* node;
    void* hashMutex;
    FontRegistryNode* freeNode;
    char lock[8];
    TableLock_ctor(lock, 2, 1);

    registry = (FontRegistryTable*)&g_hudFontConfigurationRegistry;

    // Phase 1: search while holding one outer acquisition. The hash calculation
    // briefly takes a second recursive acquisition and releases only that one.
    {
        G0Lock g0(&registry->mutexStorage);
        if (name == 0 || registry->buckets == 0) {
            g0.unlock();
            found = 0;
        } else {
            unsigned int hash;
            {
                const unsigned char* str = (const unsigned char*)name;
                hashMutex = &registry->mutexStorage;
                OSLockMutex(hashMutex);
                int len = strlen(name);
                int last = len > 0 ? len - 1 : 0;
                hash = ((len << kFontNameHashLengthShift) |
                        ((str[0] & kFontNameFirstByteHashMask) | (str[last] & kFontNameLastByteHashMask))) &
                       registry->bucketMask;
                OSUnlockMutex(hashMutex);
            }
            found = registry->buckets[hash];
            // Lock invariant: the outer acquisition is held at each loop body.
            // A hit unlocks before break; exhaustion executes the comma expression
            // once, unlocks and clears found. The null-input path unlocks above.
            // G0Lock's destructor does nothing, so there is no second release.
            while (found != 0 || (g0.unlock(), found = 0, false)) {
                if (strcmp(found->name, name) == 0) {
                    found = (FontRegistryNode*)found->configuration;
                    g0.unlock();
                    break;
                }
                found = found->bucketNext;
            }
        }
    }
    if (found != 0) {
        __dt__9TableLockFv(lock, -1);
        return;
    }
    // Phase 2: duplicates are gone; create only recognized configurations.
    entry = (RegisteredFontInfo*)__nw__FUl(kRegisteredFontInfoBytes);
    memset(entry, 0, kFontInfoPrefixClearBytes);
    for (typeIndex = 0; typeIndex < kRegisteredFontConfigurationCount; ++typeIndex) {
        if (strcmp(g_hudFontConfigurationNames[typeIndex], name) == 0) {
            entry->imageNames = &g_hudFontConfigurationNames[typeIndex];
            break;
        }
    }
    if (entry->imageNames != 0) {
        entry->cellWidth = cellWidth;
        entry->cellHeight = cellHeight;
        entry->imageCount = imageCount;
        entry->atlasWidth = atlasWidth;
        entry->atlasHeight = atlasHeight;
        entry->glyphOrder = glyphOrder;
        entry->halfScale = halfScale;
        entry->cachedIndex = -1;
        strcpy(entry->name, name);
        {
            table = (FontRegistryTable*)&g_hudFontConfigurationRegistry;
            const char* namebuf = entry->name;
            InstanceLock_u800F30AC g1(&table->mutexStorage);

            {
                InstanceLock_u800F30AC g2(&table->mutexStorage);
                if (!table->freeNodes) {
                    allocatedBlock =
                        (FontRegistryNode*)appMallocUni__FUl(table->nodesPerBlock * kFontRegistryNodeBytes);
                    table->allocatedBlocks =
                        FontRegistry_InsertAfter(&table->mutexStorage, table->allocatedBlocks, allocatedBlock);
                    for (int i = 1; i < table->nodesPerBlock; i++) {
                        freeNode = allocatedBlock + i;
                        table->freeNodes = FontRegistry_InsertAfter(&table->mutexStorage, table->freeNodes, freeNode);
                    }
                }
                node = table->freeNodes;
                table->freeNodes = FontRegistry_Detach(&table->mutexStorage, node);
            }
            node->configuration = entry;
            node->flags = 0;
            node->name = namebuf;
            if (namebuf != 0) {
                unsigned int hash;
                {
                    const unsigned char* str = (const unsigned char*)namebuf;
                    void* lk = &table->mutexStorage;
                    OSLockMutex(lk);
                    int len = strlen(namebuf);
                    int last = len > 0 ? len - 1 : 0;
                    hash = ((len << kFontNameHashLengthShift) |
                            ((str[0] & kFontNameFirstByteHashMask) | (str[last] & kFontNameLastByteHashMask))) &
                           table->bucketMask;
                    OSUnlockMutex(lk);
                }
                {
                    void* lk = &table->mutexStorage;
                    OSLockMutex(lk);
                    node->bucketNext = table->buckets[hash];
                    table->buckets[hash] = node;
                    OSUnlockMutex(lk);
                }
            }
            table->activeNodes = FontRegistry_InsertAfter(&table->mutexStorage, table->activeNodes, node);
        }
    } else {
        __dl__FPv(entry);
    }
    __dt__9TableLockFv(lock, -1);
}
#pragma pop

// Search a locked registry bucket and return a borrowed configuration, or null.
// Null name or absent buckets returns null. Empty names hash their NUL byte.
// Successful pointer remains registry-owned; caller must not delete it. Explicit
// unlocks balance the original recursive acquisition; preserve exception/lock ABI.
inline FontRegistryLookupView* FontRegistryLookupTable::find(const char* name) {
    FontRegistryLookupView* entry;
    FontRegistryLookupTable* global = this;
    InstanceLock_u800F30AC lock(global->mutexStorage);
    if (name == 0 || global->buckets == 0) {
        return 0;
    } else {
        unsigned int hash;
        {
            const unsigned char* str = (const unsigned char*)name;
            void* hashLock = global->mutexStorage;
            OSLockMutex(hashLock);
            int length = strlen(name);
            int last = length > 0 ? length - 1 : 0;
            hash = ((length << 6) | ((str[0] & kFontNameFirstByteHashMask) | (str[last] & kFontNameLastByteHashMask))) &
                   global->bucketMask;
            OSUnlockMutex(hashLock);
        }
        entry = global->buckets[hash];
        while (entry != 0) {
            if (strcmp(entry->name, name) == 0) {
                entry = entry->configuration;
                return entry;
            }
            entry = entry->bucketNext;
        }
        return 0;
    }
}

// Resolve a named font configuration, create its atlas, and reset color/alignment.
// name must exist in the registry (missing config is dereferenced); atlas creation
// is also unchecked. result may be null to select self's default pair. The result
// retains the created atlas; configuration is borrowed from the registry. Caller
// releases its atlas as the HUD destructors do. atlasOptions is forwarded opaque.
// Registry mutation/lookup is under the recovered TableLock; no lock escapes.
// Destination precondition: result (or self.defaultFont when null) must be empty
// or have had its atlas released via ResourceManager_DeleteResource. Selection
// overwrites both words without releasing the former atlas. Re-selection into an
// occupied pair leaks that reference; this is not an assign-and-release API.
// Borrower/teardown contract: the lookup mutex protects the search, not the
// returned pointer's lifetime. HudFontConfigRegistry_dtor (golf_hud_hole_info_and_fonts.cpp) frees the
// allocation-block ring and buckets; it neither waits for HUD draws nor releases
// the separately allocated RegisteredFontInfo values. Those values have no
// deletion in that destructor, but this is not a supported post-teardown lookup:
// buckets are null and registration/search state is no longer live. Stop users
// and release atlas references (CInfoBase_dtor/CDistToHoleInfo::~CDistToHoleInfo) BEFORE global
// registry/resource-manager teardown. TableLock(2,1) and the recursive instance
// mutex do not pin a returned pair or make concurrent drawing/destruction safe.
// No shutdown join/borrow count is implemented here; callers serialize lifetime.
void CFontNumDraw::SelectFont(const char* name, SelectedFontPair* result, void* atlasOptions) {
    HudFontSelection* self = (HudFontSelection*)this;
    TableLock tableLock(2, 1);
    if (result == 0)
        result = (SelectedFontPair*)&((HudTextState*)self)->defaultFont;
    result->configuration = g_hudFontConfigurationRegistry.find(name);
    result->atlas = (void*)ResourceManager_CreateWTitleFont(g_resourceManager);
    FontRegistryLookupView* config = (FontRegistryLookupView*)result->configuration;
    FontAtlas_InitImagesAndGlyphWidths(result->atlas, config, (int)atlasOptions,
                                       config->halfScale ? kFloat_0_5_hud : kFloat_1_805DE314);
    self->color = -1;
    self->alignment = 0;
}

// Measure glyph advances for alignment, draw each known glyph, then restore the font scale.
// Null/empty text does nothing. text is borrowed NUL-terminated single-byte glyph
// codes, not Unicode; unknown glyphs contribute neither advance nor spacing.
// fontOverride may be null (use self default) or a borrowed {atlas,configuration}
// pair. Both resources must exist. drawOptions is passed through to each glyph.
// x/y are UI screen coordinates, characterSpacing is scaled by textScale here.
// Center/right measure known glyph advances plus spacing, subtract one trailing
// spacing, then shift X by half/all the total. Even a nonempty string with no
// recognized glyphs follows that subtract step. No text/resource ownership moves.
// Scale restoration is on normal return; drawOptions/render state are not saved.
void HudFont_DrawAlignedText(void* self, const char* text, float x, float y, float characterSpacing, float textScale,
                             void* drawOptions, void* fontOverride) {
    if (text != 0 && strlen(text) != 0) {
        void** result = (void**)fontOverride;
        if (result == 0)
            result = (void**)&((HudTextState*)self)->defaultFont;
        void* atlas = result[0];
        void* configuration = result[1];
        void* glyphOrder = (void*)((HudFontConfig*)configuration)->glyphOrder;
        float savedScale = ((HudFontScale*)atlas)->scale;
        characterSpacing *= textScale;
        float alignmentOffset = kFloat_0_hud;
        ((HudFontScale*)atlas)->scale = textScale;
        const char* measureCursor = text;
        int alignment = ((HudTextState*)self)->alignment;
        switch (alignment) {
        case kTextLeft:
            break;
        case kTextCentered:
            while (*measureCursor) {
                void* glyph = strchr(glyphOrder, (signed char)*measureCursor++);
                if (glyph) {
                    float glyphAdvance =
                        WTitleFont_GetAtlasGlyphAdvance(atlas, g_view, (void*)((char*)glyph - (char*)glyphOrder));
                    alignmentOffset += characterSpacing + glyphAdvance;
                }
            }
            alignmentOffset -= characterSpacing;
            alignmentOffset *= kFloat_0_5_hud;
            break;
        case kTextRight:
            while (*measureCursor) {
                void* glyph = strchr(glyphOrder, (signed char)*measureCursor++);
                if (glyph) {
                    float glyphAdvance =
                        WTitleFont_GetAtlasGlyphAdvance(atlas, g_view, (void*)((char*)glyph - (char*)glyphOrder));
                    alignmentOffset += characterSpacing + glyphAdvance;
                }
            }
            alignmentOffset -= characterSpacing;
            break;
        }
        x = x - alignmentOffset;
        const char* drawCursor = text;
        while (*drawCursor) {
            void* glyph = strchr(glyphOrder, (signed char)*drawCursor++);
            if (glyph) {
                float glyphAdvance = WFont_DrawAtlasGlyph(atlas, g_view, (void*)((char*)glyph - (char*)glyphOrder), x,
                                                          y, drawOptions, (void*)((HudTextState*)self)->color);
                x += glyphAdvance + characterSpacing;
            }
        }
        ((HudFontScale*)atlas)->scale = savedScale;
    }
}

struct FontVariantState {
    char pad[0xCC];
    unsigned int variantIndex;
};
#pragma opt_propagation off
// Temporarily select one of the font's glyph variants while drawing, then restore it.
void HudFont_DrawAlignedTextWithVariant(void* self, const char* text, int useAlternateVariant, void* drawOptions,
                                        void* fontOverride, float x, float y, float characterSpacing, float textScale) {
    unsigned int savedVariant;
    FontVariantState* object;
    void** result;
    if (fontOverride == 0) {
        result = (void**)&((HudTextState*)self)->defaultFont;
    } else {
        result = (void**)fontOverride;
    }
    object = (FontVariantState*)*result;
    savedVariant = object->variantIndex;
    object->variantIndex = useAlternateVariant != 0;
    HudFont_DrawAlignedText(self, text, x, y, characterSpacing, textScale, drawOptions, result);
    object->variantIndex = savedVariant;
}
#pragma opt_propagation reset

struct HoleCupOverlayVTable {
    void* reserved_00;
    void* reserved_04;
    void* reserved_08;
    void (*method0C)();
};

struct HoleCupOverlayInterface {
    virtual void v0();
    virtual void method0C();
};

struct HoleCupOverlayOwner {
    char reserved_00[0x34];
    HoleCupOverlayInterface* child;
};

// Advance the cup indicator overlay if present.
// Slot 0x10 is update(float dt); the frame time reaches the child in FPR1 on the PowerPC.
void CHoleCupInfo::UpdateDistanceOverlay() {
    HoleCupOverlayOwner* self = (HoleCupOverlayOwner*)this;
    HoleCupOverlayInterface* child = self->child;
    if (child != 0) {
        child->method0C();
    }
}

struct DistanceHudRevealState {
    char reserved_00[0x1C];
    int state;
    float timer;
    char reserved_24[0x28];
    float alpha;
};

extern float kFloat_0_hud;
extern float kFloat_255_MessageUI;

// Finish fading in immediately and begin moving the distance display toward the pin.
void CDistToHoleInfo::BeginMoveToPin() {
    DistanceHudRevealState* self = (DistanceHudRevealState*)this;
    if (self->state == kDistanceHold || kDistanceFadeIn == self->state) {
        float maximum;
        float zero;
        zero = kFloat_0_hud;
        maximum = kFloat_255_MessageUI;
        self->timer = zero;
        self->alpha = maximum;
        self->state = kDistanceMoveToPin;
    }
}

struct DistanceHudOwnedResources {
    void* vtable;
    void* fontResource;
    char reserved_08[0x0C];
    void* textBuffer;
    char reserved_18[0x40];
    void* alternateFont;
};

extern char __vt__15CDistToHoleInfo[];
extern char cInfoBaseVtable[];
extern char __vt__12CHoleCupInfo[];

extern void* g_resourceManager;
void ResourceManager_DeleteResource(void*, void*);
void __dla__FPv(void*);
void __dl__FPv(void*);

// Release distance-font resources and text storage, then optionally delete the display.
CDistToHoleInfo::~CDistToHoleInfo() {
    DistanceHudOwnedResources* self = (DistanceHudOwnedResources*)this;
    void* alternateFontResource = self->alternateFont;
    self->vtable = &__vt__15CDistToHoleInfo;
    if (alternateFontResource != 0) {
        ResourceManager_DeleteResource(g_resourceManager, alternateFontResource);
        self->alternateFont = 0;
    }
    if (self != 0) {
        void* defaultFontResource = self->fontResource;
        self->vtable = &cInfoBaseVtable;
        if (defaultFontResource != 0) {
            ResourceManager_DeleteResource(g_resourceManager, defaultFontResource);
            self->fontResource = 0;
        }
        if (self->textBuffer != 0) {
            __dla__FPv(self->textBuffer);
            self->textBuffer = 0;
        }
    }
}

struct ClubRangeVTable {
    void* reserved_00;
    void* reserved_04;
    void* reserved_08;
    void* reserved_0C;
    void* reserved_10;
    void (*selectFont)(void*, const char*, int, int);
};

struct ClubRangeConstructionView {
    // Table +0x14 = CFontNumDraw::SelectFont: (receiver, name, optional output-pair address, atlas advance-mode
    // option). Zero output uses self+4/+8; zero option selects measured advances. Writes the font/config pair,
    // color=-1, alignment=left; no status return.
    virtual void v0();
    virtual void v4();
    virtual void v8();
    // Table +0x14 = CFontNumDraw::SelectFont (phase1_class_tables.cpp): same
    // receiver, name, optional output-pair address, atlas advance-mode option.
    // Zero output uses self+4/+8; zero option selects measured advances. Writes
    // that font/config pair, color=-1 and alignment=left; no status return.
    virtual void selectFont(const char*, int, int);
    int fontResource;
    char reserved_08[8];
    int alignment;
    void* textBuffer;
    Vec3f rangePoint;
};

extern ClubRangeVTable __vt__14CClubRangeInfo;
extern const char golfHudFontStrings[];
void Vec3_Zero(void*);
void* __nwa__FUl(unsigned int);

// Create the club-range font, zero its world point and allocate numeric text storage.
// Fresh 0x24-byte object only. CInfoBase construction (golf_hud_spot_markers_and_fonts.cpp) first clears
// font/config/text pointers. selectFont selects the default atlas as documented at
// CFontNumDraw::SelectFont; its name/config/atlas lookups are unchecked. Only after
// that succeeds are rangePoint/alignment initialized and 32 text bytes published.
// No text terminator/content is written here. A null buffer is stored without a
// failure result; callers that format text require successful allocation. If the
// constructor throws, C++ destroys the completed CInfoBase subobject (releasing
// its published font and text); it does not run the derived destructor. A throw
// inside font initialization can still leave callee-internal partial work; this
// constructor provides no stronger transaction. Normal teardown: CClubRangeInfo::~CClubRangeInfo.
CClubRangeInfo::CClubRangeInfo() {
    ClubRangeConstructionView* self = (ClubRangeConstructionView*)this;
    *(ClubRangeVTable**)self = &__vt__14CClubRangeInfo;
    self->selectFont(HudStrings().windFont, 0, 0);
    Vec3_Zero(&self->rangePoint);
    self->alignment = kTextCentered;
    self->textBuffer = __nwa__FUl(0x20);
}

struct HoleCupHeightState {
    char reserved_00[0x20];
    float heightDifferenceMeters;
    char reserved_24[0x10];
    struct Child {
        virtual void v0();
        virtual void v4();
        virtual void v8();
        virtual void vC();
        virtual void v10();
        virtual void update();
    }* heightOverlay;
};

struct HolePinPosition {
    unsigned int reserved_00;
    unsigned int reserved_04;
    unsigned int reserved_08;
    float pinX;
    float pinY;
    float pinZ;
    char reserved_18[8];
};

struct BallHeightState {
    char reserved_00[0xC0];
    float heightOffset;
};
extern CSharedDoc* g_sharedDoc;
extern float kFloat_3_2_hud;
extern float kYardsToMeters_HoleCupHud;
BallHeightState* GolfBall__Fv(void*);
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);

// Compute pin height minus player/ball-adjusted height, in meters, and update the overlay.
// Writes heightDifferenceMeters=((pinY-(playerY-ball.heightOffset))/3.2)*yardsToMeters.
// The optional child update occurs after the write. Positions are borrowed world
// coordinates; no allocation, projection, culling or height-indicator draw occurs.
void CHoleCupInfo::UpdateHeightDifference() {
    HoleCupHeightState* self = (HoleCupHeightState*)this;
    float current = GolfBall__Fv(self)->heightOffset;
    float target =
        ((HudPlayerEntry*)GolfPlayer_GetEntryByIndex((CGolfDoc*)((void*)g_sharedDoc->golfDoc),
                                                     ((CGolfDoc*)((void*)g_sharedDoc->golfDoc))->currentPlayer))
            ->position.y;
    float difference = target - current;
    unsigned int index = ((CGolfDoc*)((void*)g_sharedDoc->golfDoc))->currentHoleIndex;
    HolePinPosition* record = (HolePinPosition*)((CGolfDoc*)((void*)g_sharedDoc->golfDoc))->holeRecords + index;
    Vec3f position;
    *(VectorWords*)&position = *(VectorWords*)&record->pinX;
    float amount = (position.y - difference) / kFloat_3_2_hud;
    self->heightDifferenceMeters = kYardsToMeters_HoleCupHud * amount;
    if (self->heightOverlay != 0) {
        self->heightOverlay->update();
    }
}

struct HoleCupOwnedResources {
    void* vtable;
    void* fontResource;
    char reserved_08[0x0C];
    void* textBuffer;
    char reserved_18[0x0C];
    void* heightIndicators[3];
    char reserved_30[4];
    struct Child {
        virtual void destroy(int);
    }* heightOverlay;
};

void Object_DeleteVirtual(void*, void*);

#pragma push
#pragma use_lmw_stmw on
// Release cup-indicator images, overlay and text resources, then optionally delete the display.
CHoleCupInfo::~CHoleCupInfo() {
    HoleCupOwnedResources* self = (HoleCupOwnedResources*)this;
    self->vtable = &__vt__12CHoleCupInfo;
    for (int i = 0; i < 3; ++i) {
        if (self->heightIndicators[i] != 0) {
            Object_DeleteVirtual(g_resourceManager, self->heightIndicators[i]);
            self->heightIndicators[i] = 0;
        }
    }
    if (self->heightOverlay != 0) {
        if (self->heightOverlay != 0) {
            self->heightOverlay->destroy(1);
        }
        self->heightOverlay = 0;
    }
    if (self != 0) {
        self->vtable = &cInfoBaseVtable;
        if (self->fontResource != 0) {
            ResourceManager_DeleteResource(g_resourceManager, self->fontResource);
            self->fontResource = 0;
        }
        if (self->textBuffer != 0) {
            __dla__FPv(self->textBuffer);
            self->textBuffer = 0;
        }
    }
}
#pragma pop

struct DistanceHudUpdateState {
    char reserved_00[0x14];
    void* textBuffer;
    float displayValue;
    int state;
    float timer;
    float interpolation;
    float startX;
    float startY;
    float startScale;
    char reserved_34[0x0C];
    float currentX;
    float currentY;
    float currentScale;
    float alpha;
    float projectedXOffset;
};

extern float kFloat_0_hud;
extern float kFloat_100_hud, kDistanceHudProjectedXOffset;
void* __mi__FRC7WVectorRC7WVector(Vec3f*, Vec3f*, Vec3f*);
float Magnitude__7WVectorCFv(Vec3f*);

// Recompute horizontal pin distance or party points, format the text, and begin fading in.
// Required textBuffer is the owned 32-byte numeric buffer. Normal distance is
// XZ length/3.2 yards: below 100 show a decimal, otherwise whole yards. Party mode
// copies points and formats them later in draw. Writes timer/interpolation/alpha
// to zero, state FadeIn and current pose from start pose; no previous pose is kept.
void CDistToHoleInfo::Reset() {
    DistanceHudUpdateState* self = (DistanceHudUpdateState*)this;
    if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyPointsMode) {
        CGolfDoc* golfDoc = (CGolfDoc*)((void*)g_sharedDoc->golfDoc);
        void* playerEntry = GolfPlayer_GetEntryByIndex(golfDoc, golfDoc->currentPlayer);
        self->displayValue = ((HudPlayerEntry*)playerEntry)->partyPoints;
    } else {
        Vec3f position;
        Vec3f origin;
        void* playerEntry = GolfPlayer_GetEntryByIndex((void*)((void*)g_sharedDoc->golfDoc),
                                                       ((CGolfDoc*)((void*)g_sharedDoc->golfDoc))->currentPlayer);
        CGolfDoc* golfDoc = (CGolfDoc*)((void*)g_sharedDoc->golfDoc);
        unsigned int holeIndex = golfDoc->currentHoleIndex;
        HolePinPosition* record = (HolePinPosition*)golfDoc->holeRecords + holeIndex;
        *(VectorWords*)&origin = *(VectorWords*)&record->pinX;
        __mi__FRC7WVectorRC7WVector(&position, &origin, &((HudPlayerEntry*)playerEntry)->position);
        position.y = kFloat_0_hud;
        float distanceYards = Magnitude__7WVectorCFv(&position);
        float limit = kFloat_100_hud;
        float divisor = kFloat_3_2_hud;
        distanceYards = distanceYards / divisor;
        self->displayValue = distanceYards;
        if (distanceYards < limit) {
            const char* fmt = HudStrings().distanceOneDecimal;
            // The unit text format is "%.1f%s" / "%.0f%s" (value, then unit).
            sprintf((char*)self->textBuffer, fmt, HudStrings().yardsUnit, distanceYards);
        } else {
            const char* fmt = HudStrings().distanceWholeNumber;
            sprintf((char*)self->textBuffer, fmt, HudStrings().yardsUnit, distanceYards);
        }
        self->projectedXOffset = kDistanceHudProjectedXOffset;
    }
    float zero = kFloat_0_hud;
    int two = kDistanceFadeIn;
    self->timer = zero;
    self->interpolation = zero;
    self->alpha = zero;
    self->state = two;
    self->currentX = self->startX;
    self->currentY = self->startY;
    self->currentScale = self->startScale;
}

struct DistanceHudVTable {
    char reserved_00[0x14];
    void (*selectFont)(void*, const char*, void*, int);
};
struct DistanceHudConstructionView {
    virtual void v0();
    virtual void v4();
    virtual void v8();
    // Same +0x14 SelectFont contract as ClubRangeConstructionView. Output0 uses
    // the default pair; storage points at the empty alternate pair at self+58.
    virtual void selectFont(const char*, void*, int);
    char reserved_04[0x0C];
    int alignment;
    void* textBuffer;
    char reserved_18[4];
    int state;
    float timer;
    float interpolation;
    float startX, startY, startScale, endX;
    float endY, endScale;
    char reserved_40[0x18];
    unsigned char storage[0x20];
};

extern const float kDistanceHudStartX, kFloat_256_MessageUI, kDistanceHudStartScale, kDistanceHudEndY,
    kFloat_0_6_805DE33C;
extern float kFloat_0_hud;
extern const float kPartyPointsHudAnchorY, kPartyPointsHudScale;
void* memset(void*, int, unsigned int);

extern "C" void* __dt__9CInfoBaseFv(void*, int);
// Create the distance display fonts and select anchors appropriate to the party mode.
// Fresh object; owns font resource(s) and a 32-byte numeric buffer.
// Base cleanup runs if construction throws; allocation/status are not checked.
// Default start=(417,256), scale=.77, endY=91, endScale=.6; party points instead
// use startY=endY=253 and scale=.67, with endX=startX+1. Shared scalar definitions
// retain their exact binary32 values; these decimal values describe their roles.
CDistToHoleInfo::CDistToHoleInfo() {
    DistanceHudConstructionView* self = (DistanceHudConstructionView*)this;
    unsigned int five;
    float zero;
    float v3C;
    float v38;
    float v30;
    float v2C;
    float v28;
    DistanceHudVTable* vtable;
    ((CInfoBase*)((ClubRangeConstructionView*)self))->__ct();
    try {
        v28 = *(volatile const float*)&kDistanceHudStartX;
        vtable = (DistanceHudVTable*)__vt__15CDistToHoleInfo;
        v2C = *(volatile const float*)&kFloat_256_MessageUI;
        v30 = *(volatile const float*)&kDistanceHudStartScale;
        v38 = *(volatile const float*)&kDistanceHudEndY;
        five = kDistanceHidden;
        v3C = *(volatile const float*)&kFloat_0_6_805DE33C;
        zero = *(volatile const float*)&kFloat_0_hud;
        *(DistanceHudVTable**)self = vtable;
        self->startX = v28;
        self->startY = v2C;
        self->startScale = v30;
        self->endY = v38;

        self->endScale = v3C;
        self->state = five;
        self->interpolation = zero;
        memset(self->storage, 0, 8);
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) == kPartyPointsMode) {
            self->selectFont(HudStrings().distanceFont, 0, 1);
            self->selectFont(HudStrings().pointsUnitFont, self->storage, 1);
            float f0;
            float f1;
            float f2;
            float f3;
            f3 = kDistanceHudStartX;
            f0 = kFloat_1_805DE314;
            f2 = kPartyPointsHudAnchorY;
            f1 = kPartyPointsHudScale;
            f0 = f0 + f3;
            self->startX = f3;
            self->startY = f2;
            self->startScale = f1;
            self->endX = f0;
            self->endY = f2;
            self->endScale = f1;
        } else {
            self->selectFont(HudStrings().largeFont, 0, 0);
            self->alignment = kTextCentered;
        }
        self->textBuffer = __nwa__FUl(kHudNumericTextBufferBytes);
    } catch (...) {
        __dt__9CInfoBaseFv(self, -1);
        throw;
    }
}

struct DistanceHudAnimationState {
    char reserved_00[0x1C];
    int state;
    float timer;
    float interpolation;
    char reserved_28[0x24];
    float alpha;
};
struct HolePositionWords {
    char reserved_00[0x0C];
    unsigned int a, b, c;
    char reserved_18[8];
};
extern void* g_inputPointer;
extern float kFloat_2_805DE318, kFloat_255_MessageUI, kFloat_510_Hud, kFloat_0_5_hud;
void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
// Returns whether any of nine monitored input actions is active.
float __ml__FRC7WVectorRC7WVector(void*, void*);
void GolfTipsDialog_RequestClose(int, float);

// Animate distance opacity and position; input actions and pin visibility can end the hold state.
// Admission/editing map (evaluated EVERY call, before the state switch):
// gameStateFlags bit 0x2 -> return, preserving state/timers/alpha/interpolation.
// Otherwise any bit of 0x8210 (0x10|0x200|0x8000) forces AtPin, as does tutorial
// stage 0..4, or stage 5 while the CURRENT player's holeStrokeCounts[0]==1.
// That count is the first-hole slot, not currentHoleIndex and not a general
// "first stroke" Boolean. Stage 5 with count!=1 and stages>=6 allow the cycle.
// CGolfRule tutorial transitions in golf_rule_round_flow_and_shot_sim.cpp produce the numeric stage; no
// lesson-title meaning is assumed here. A forced AtPin writes alpha=255 and
// interpolation=1 but leaves timer untouched. Hidden is also overridden by this
// gate. Only admitted FadeIn/Hold/MoveToPin/FadeOut consume deltaSeconds; editing
// 2s/.5s/510 cannot change suppressed or forced calls. Keep pause priority, mask
// tests and the later exact-whole-word ==4 pointer test distinct. Test both stage-5
// count cases, stages 4/6, pause+reset together, and party mode's double fade step.
void CDistToHoleInfo::UpdateAnimation(float deltaSeconds) {
    DistanceHudAnimationState* self = (DistanceHudAnimationState*)this;
    // Gate the whole animation first: paused frames do not advance time; reset
    // flags and early tutorial stages force the full-opacity pin-attached state.
    CSharedDoc* g = (CSharedDoc*)g_sharedDoc;
    CGolfDoc* golfDoc = (CGolfDoc*)g->golfDoc;
    unsigned int flags = golfDoc->gameStateFlags;
    if (flags & kGolfStateHudPaused) {
        return;
    }
    if ((flags & kGolfStatesResetDistance) || (golfDoc->tutorialStage < 6 && golfDoc->tutorialStage != 5) ||
        (golfDoc->tutorialStage == 5 &&
         ((HudPlayerEntry*)GolfPlayer_GetEntryByIndex((void*)golfDoc, golfDoc->currentPlayer))->holeStrokeCounts[0] ==
             1)) {
        self->state = kDistanceAtPin;
    }
    // State cycle: FadeIn -> Hold (2 s) -> MoveToPin (0.5 s) -> AtPin.
    // Input or a pin behind the camera can send Hold to FadeOut instead.
    // Alpha uses 0..255 at 510 units/s; interpolation uses 0..1.
    switch (self->state) {
    case kDistanceAtPin: {
        float one;
        float full;
        full = kFloat_255_MessageUI;
        one = kFloat_1_805DE314;
        self->alpha = full;
        self->interpolation = one;
        break;
    }
    case kDistanceFadeIn: {
        float alpha;
        float max;
        float rate;
        rate = kFloat_510_Hud;
        alpha = self->alpha + rate * deltaSeconds;
        max = kFloat_255_MessageUI;
        self->alpha = alpha;
        if (alpha > max) {
            float zero = kFloat_0_hud;
            self->alpha = max;
            self->timer = zero;
            self->state = kDistanceHold;
        }
        break;
    }
    case kDistanceHold: {
        // The timer may select MoveToPin, but the visibility/input test below
        // still runs on this frame and can override that choice with FadeOut.
        float two;
        float timer;
        timer = self->timer;
        two = kFloat_2_805DE318;
        timer += deltaSeconds;
        self->timer = timer;
        if (timer > two) {
            float zero = kFloat_0_hud;
            self->state = kDistanceMoveToPin;
            self->timer = zero;
        }
        if (CInputManager::HasAnyMonitoredAction(g_inputPointer) == 0 ||
            (unsigned int)((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->gameStateFlags == kGolfStatePointerLocked) {
            CGolfDoc* p = (CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc;
            HolePositionWords* rec = (HolePositionWords*)p->holeRecords + p->currentHoleIndex;
            Vec3f result;
            Vec3f origin;
            *(VectorWords*)&origin = *(VectorWords*)&rec->a;
            __mi__FRC7WVectorRC7WVector(&result, &origin, (Vec3f*)AsCameraTransform800D(s_cameraTransform)->position);
            if (!(__ml__FRC7WVectorRC7WVector(&result, (void*)&AsCameraTransform800D(s_cameraTransform)->basis[6]) <
                  kFloat_0_hud)) {
                break;
            }
        }
        GolfTipsDialog_RequestClose(-2, kFloat_0_5_hud);
        self->state = kDistanceFadeOut;
        break;
    }
    case kDistanceMoveToPin:
        if ((*(int*)&AsCSharedDoc(g_sharedDoc)->partyGameMode) != kPartyPointsMode) {
            float limit;
            float timer;
            timer = self->timer;
            limit = kFloat_0_5_hud;
            timer += deltaSeconds;
            self->timer = timer;
            if (timer > limit) {
                self->timer = limit;
                self->state = kDistanceAtPin;
            }
            self->interpolation = self->timer / kFloat_0_5_hud;
            if (CInputManager::HasAnyMonitoredAction(g_inputPointer) &&
                (unsigned int)((CGolfDoc*)((CSharedDoc*)g_sharedDoc)->golfDoc)->gameStateFlags !=
                    kGolfStatePointerLocked) {
                self->state = kDistanceAtPin;
            }
            break;
        } else {
            self->alpha -= kFloat_510_Hud * deltaSeconds;
            self->state = kDistanceFadeOut;
        }
        // Intentional fallthrough for party points: subtract alpha again this
        // frame. Normal distance mode takes the break above after interpolation.
    case kDistanceFadeOut: {
        float zero;
        float alpha;
        float rate;
        rate = kFloat_510_Hud;
        alpha = self->alpha;
        zero = kFloat_0_hud;
        alpha -= rate * deltaSeconds;
        self->alpha = alpha;
        if (alpha < zero) {
            float one;
            float full;
            full = kFloat_255_MessageUI;
            one = kFloat_1_805DE314;
            self->alpha = full;
            self->interpolation = one;
            self->state = kDistanceAtPin;
        }
        break;
    }
    case kDistanceHidden:
        break;
    default:
        break;
    }
}
struct HoleCupDrawState {
    char reserved_00[0x18];
    float x, y, heightDifferenceMeters;
    void* heightIndicators[3];
    int heightDirection;
    struct Child {
        virtual void v0();
        virtual void v4();
        virtual void update();
    }* child;
};
struct HolePosition {
    char reserved_00[0x0C];
    float x, y, z;
};
struct HoleProjectionMatrix {
    char data[0x30];
};
struct HudRectangle {
    float x, y, z, w;
};
extern WView* g_view;
extern const float kCupHeightOneDecimalThreshold, kCupHudFixedScreenY, kFloat_0_1_HudShared, kCupHeightTextXOffset;
extern const float kCupHeightLevelTextYOffset, kFloat_3_Hud, kCupHeightAboveTextYOffset, kCupHeightBelowTextYOffset;
extern const float kCupHeightIndicatorYOffset, kFloat_32_0, kFloat_64_Hud;
extern double kCupHeightNegativeZeroThreshold;
struct HudMousePosition {
    char pad[0x94];
    float x, y;
};
extern HudMousePosition* g_mouseCursor;
void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, int, int, int);

#pragma opt_lifetimes off
// Project the pin, publish its cursor anchor and draw cup-height text/indicator.
// Controls widescreen font/pin placement; self and world pin/document/view required.
// Optional child is updated BEFORE behind-camera culling; on cull x/y, mouse cursor,
// direction and draw outputs stay unchanged. Otherwise writes self x/y/direction
// and g_mouseCursor x/y, then draws the chosen nullable indicator.
// Height is meters: |h|>10 => one decimal, -.01<h<0 => special near-zero string,
// otherwise two decimals. |h|<.1 selects level, positive above, negative below.
// The .1 pool word kFloat_0_1_HudShared also controls Pang pulse time and
// scale (golf_hud_flight_info_and_pang_rewards.cpp): changing it globally changes all three roles. Separate future
// settings need separate storage. Keep native float operations and double threshold.
// This function consumes projection; it does not install aspect/viewport/scissor.
void CHoleCupInfo::UpdateScreenPositionAndDraw() {
    HoleCupDrawState* self = (HoleCupDrawState*)this;
    unsigned int b;
    int a;
    struct Screen {
        float x;
        float y;
        char pad[0x38];
    } screen;
    char text[0x20];
    WVector transformed;
    Vec3f origin;
    Vec3f pinViewPosition;
    Vec3f second;
    HudRectangle color;

    if (self->child)
        self->child->update();
    CGolfDoc* golfDoc = (CGolfDoc*)((void*)g_sharedDoc->golfDoc);
    HolePositionWords* record = (HolePositionWords*)golfDoc->holeRecords + golfDoc->currentHoleIndex;
    *(VectorWords*)&origin = *(VectorWords*)&record->a;
    __mi__FRC7WVectorRC7WVector((Vec3f*)&transformed, (Vec3f*)&origin,
                                (Vec3f*)AsCameraTransform800D(s_cameraTransform)->position);

    if (((transformed * *(WVector*)((void*)&AsCameraTransform800D(s_cameraTransform)->basis[6]))) < kFloat_0_hud)
        return;
    if ((self->heightDifferenceMeters > kFloat_0_hud ? self->heightDifferenceMeters : -self->heightDifferenceMeters) >
        kCupHeightOneDecimalThreshold) {
        sprintf(text, HudStrings().heightOneDecimal, self->heightDifferenceMeters);
    } else if (kCupHeightNegativeZeroThreshold < self->heightDifferenceMeters &&
               self->heightDifferenceMeters < kFloat_0_hud) {
        sprintf(text, HudStrings().heightNearZero);
    } else {
        sprintf(text, HudStrings().heightTwoDecimals, self->heightDifferenceMeters);
    }
    golfDoc = (CGolfDoc*)((void*)g_sharedDoc->golfDoc);
    record = (HolePositionWords*)golfDoc->holeRecords + golfDoc->currentHoleIndex;
    *(VectorWords*)&second = *(VectorWords*)&record->a;
    ((void (*)(void*, void*, void*))Matrix_TransformPoint)(&pinViewPosition, &second,
                                                           (char*)AsWView(g_view)->inputTransform);
    WView_ProjectViewPoint(g_view, (ProjectedPointStorage*)&screen, (TransformStorage*)&pinViewPosition, 0);
    self->x = screen.x;
    self->y = (unsigned int)((CGolfDoc*)((void*)g_sharedDoc->golfDoc))->gameStateFlags == kGolfStateUseProjectedY
                  ? screen.y
                  : kCupHudFixedScreenY;
    HudMousePosition* global = g_mouseCursor;
    float threshold;
    float screenX;
    float y;
    screenX = self->x;
    y = (float)(double)self->y;
    threshold = kFloat_0_hud;
    global->x = screenX;
    global->y = y;

    // This shared HUD constant is a height tolerance in meters here;
    // golf_hud_flight_info_and_pang_rewards.cpp also uses it for Pang time and scale.
    if ((self->heightDifferenceMeters > threshold ? self->heightDifferenceMeters : -self->heightDifferenceMeters) <
        kFloat_0_1_HudShared) {
        self->heightDirection = kCupLevel;
        HudFont_DrawAlignedText(self, text, kCupHeightTextXOffset + self->x, self->y - kCupHeightLevelTextYOffset,
                                kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
    } else if (self->heightDifferenceMeters > kFloat_0_hud) {
        self->heightDirection = kCupAbove;
        HudFont_DrawAlignedText(self, text, kCupHeightTextXOffset + self->x, self->y - kCupHeightAboveTextYOffset,
                                kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
    } else {
        self->heightDirection = kCupBelow;
        HudFont_DrawAlignedText(self, text, kCupHeightTextXOffset + self->x, self->y - kCupHeightBelowTextYOffset,
                                kFloat_3_Hud, kFloat_1_805DE314, 0, 0);
    }
    if (self->heightIndicators[self->heightDirection] != 0) {
        HudRectangle size;
        float one;
        float zero;
        float extent;
        float xdiff;
        float sx;
        float sy;
        extent = self->y;
        sy = extent - kCupHeightIndicatorYOffset;
        sx = self->x;
        zero = kFloat_0_hud;
        extent = kFloat_64_Hud;
        one = kFloat_1_805DE314;
        xdiff = kFloat_32_0;
        size.y = sy;
        xdiff = sx - xdiff;
        size.z = extent;
        size.x = xdiff;
        size.w = extent;
        color.x = zero;
        color.y = zero;
        color.z = one;
        color.w = one;
        WOverlay_DrawTransformedRect(self->heightIndicators[self->heightDirection], g_view, &color, zero, &size, 0, -1,
                                     0);
    }
}
#pragma opt_lifetimes reset

} // extern "C"
