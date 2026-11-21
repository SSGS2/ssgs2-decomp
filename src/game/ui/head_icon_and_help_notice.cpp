#include <decomp/static_init.h>
#include <decomp/ct_member.h>
// In-round head/item icons and Help notice/panel lifetime management.
// Widescreen/frame rate mod lookup:
// - HeadIcon_DrawPlayerSlot consumes projected anchors from
//   CHeadIcon_ProjectPlayerPoint (head_icon_and_aux_menu_ui.cpp): world point -> active view -> slot
//   XY/visibility. Do not stretch world/camera-relative vectors for widescreen.
// - Icon rectangle: X=slotX-60-200/distance, Y=slotY-52-800/distance. Item image
//   adds (+28,+30) and is 42x42. Frame center=floor(slotX+4),
//   floor(slotY-1200/distance-10), half-extents (26,11), UI pixels X right/Y down.
//   These scalars are in u80184750_data.cpp; 640 is a WORLD distance cull, not
//   horizontal resolution. Keep inverse-distance terms in their current units.
// - Texture coordinates: 4x4 atlas in 256 pixels, cell=64; frame UV extent
//   (.8125,.6875) is independent of its 52x22 pixel size. head_icon_and_aux_menu_ui.cpp owns the atlas.
// - Help panel: origin raw float words (177,243); width=256+72, cap height=8;
//   constructor UV rows split at .25/.5/.75. u80184750_data.cpp owns these words.
// - Timing: HeadIcon_UpdatePlayerTimers expires at elapsed>3 simulation seconds,
//   except kind 0x1D; NoticeQueue_UpdateTimers subtracts delta from active unpaused
//   notices. Both pause on their documented gates; neither counts display frames.
// Required receivers/resources and player indices 0..3 are trusted; visibility
// driver assumes the document's playerCount is within the four-player storage.
// Nine CBpTex objects are created/destroyed by CHeadIcon_ctor/dtor (head_icon_and_aux_menu_ui.cpp).
// Here their virtual slot 4 is IsLoaded; slot 3 binds the selected texture. The
// readiness loop deliberately calls ALL nine, even after one reports not ready.
// No viewport/scissor/aspect setter lives here. Keep projection/rectangle/UV
// adjustments separate, bind before quad submission, and retain floor/int casts.
// Notice copies own their heap wide text. Successful help teardown deletes the
// message context, then panel (which releases its overlay through the manager),
// then vector entries/storage. Failed construction has the separate ledger at
// CHelpMsg_Ctor; it does not have full rollback. No thread locks here.
// Clock provenance: CMainFrame_UpdateVirtualHooks (main_frame_letter_and_mouse_cursor.cpp) publishes
// seconds from 1/30 or 1/25 s; CProjectG_Update (project_g_app_and_devices.cpp) multiplies by its
// dimensionless timeScale. CTask_UpdateCurrentTaskWithFadeGates (ctask_transition_command_queue.cpp),
// CGolfTask_UpdatePauseMenuActorsAndScorecard (golf_task_loading_and_pause.cpp), CTask_ProcessActors (task_manager_and_actors.cpp) and
// CScreen_OnProcess (ingame_hud_screen.cpp) forward that delta to both timers here.
// HOME, task loading/fade and golf pause gates may omit calls; timeScale=0 passes
// zero. Thus these are admitted gameplay seconds, not wall time or draw calls.
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/WFixedFont.h"
#include "classes/GolfDocSnapshot8013.h"
// Borrow the existing shared-document ABI view; no lifetime change.
static inline CSharedDoc* AsCSharedDoc(const void* p) { return (CSharedDoc*)p; }
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char noticeTemplateStrings[];
}
}

// Translation unit 0x80184750-0x80185E48 (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "sdk_protos.h"
#include "game/wvector.h"
#pragma opt_common_subs off

extern "C" {

void BaseObject_ctor(void*);
void appFreeUni__FPv(void*);
void* appMallocDummy__FUl(unsigned long);
void* __nwa__FUl(unsigned long);
void* __nw__FUl(unsigned long);
void __dla__FPv(void*);
void WString_ReserveCapacity(void*, unsigned int);
void WString_ReplaceRange(void*, unsigned int, unsigned int, const unsigned short*, const unsigned short*, const void*);
void* WOverlay_Create(void*, const char*, int);
void WFixedFont_CreateMsgEcbResource(void*, void*, int, int);
void* CMsgDraw_Ctor(char*, void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void BaseObject_dtor(void*, int);
void __dl__FPv(void*);
int SharedDoc_IsPlayerFlag0Set(unsigned char);

extern float kFloat_6_2831855, s_initializedTwoPi_80184F48;
extern const float kFloat_0_0_HeadIcon, kFloat_3_HeadIcon;
enum {
    kHeadIconAnimationCount = 9, kMaximumDisplayedPlayers = 4,
    kHeadIconNone = 0xffff, kPersistentHeadIcon = 0x1d,
    kInactivePlayer = 3, kDrawAllPlayerIcons = 0x20000,
    kDrawTurnPlayerIcon = 0x1000, kGameStateHidingIconFrame = 0x200,
    kHeadIconAtlasColumns = 4, kHeadIconAtlasCellCount = 16,
    kHeadIconAtlasPixels = 256, kHeadIconQuadFormat = 0x11f005,
    kHeadIconQuadVertices = 4, kHeadIconTriangleStrip = 0x98,
    kHeadIconInventoryVariantEntry = 14, kHeadIconInventoryOmittedEntry = 6
};
struct HelpUpdateNode;
struct HelpUpdateTable {
    void* unused[6];
    int (*update)(HelpUpdateNode*);
};
struct HelpUpdateNode {
// CBpTex slots: 3 = bind the chosen frame texture before the quad submission; 4 = IsLoaded readiness, not an
// animation-time test (head_icon_and_aux_menu_ui.cpp CHeadIcon_ctor, __vt__6CBpTex).
    virtual void unused0();
    virtual void unused1();
    virtual void unused2();
    // CBpTex slot 3: bind the chosen frame texture before the quad submission.
    // Constructor/vtable evidence: head_icon_and_aux_menu_ui.cpp CHeadIcon_ctor and
    // phase1_class_tables.cpp __vt__6CBpTex map this slot to CBpTex_Bind.
    // Method names remain ABI-view placeholders.
    virtual void unused3();
    // CBpTex slot 4: IsLoaded readiness, not an animation-time isLoaded.
    virtual int isLoaded();
    char baseStorage[0x24];
};
// One shared per-player record for timer update AND projected icon drawing.
// Stored at object+0x14, stride0x30: timer+0, anchors+8, relative vector+0x20.
struct HeadIconPlayerSlot {
    unsigned short kind;
    char alignmentAfterKind[2];
    float elapsed;
    float x, y, z;
    char reserved14[0xC];
    char transform[0x10];
};
typedef HeadIconPlayerSlot HelpTimer;
// Owner view used by both paths; timers/players are spelling aliases of the SAME
// four elements, not overlapping partial objects or an extra fifth player.
struct HelpUpdateState {
    char baseStorage[0x14];
    union {
        HeadIconPlayerSlot timers[kMaximumDisplayedPlayers];
        HeadIconPlayerSlot players[kMaximumDisplayedPlayers];
    };
    void* font;
    float uv[4];
    float rect[4];
    char tree[0xC];
    HelpUpdateNode* nodes;
};
// Head-icon update and draw paths are two views of the same object, independent of notices.
typedef HelpUpdateState HeadIconUpdateView;
typedef CGolfDoc HelpGameState;
typedef CSharedDoc HelpGame;
typedef GolfDocSnapshot8013 HelpPlayer;
struct ChatMessageResourceView { char base[0x10]; void* resource; };
struct HeadIconVisibility { char base[0x10]; unsigned char enabled; };
HelpPlayer* GolfPlayer_GetEntryByIndex(HelpGameState*, unsigned char);
unsigned int CGolfDoc_ResolveMatchTurnFromHoleStrokes(HelpGameState*);
void HeadIcon_DrawPlayerSlot(void*, unsigned char);
void Object_DeleteVirtual(void*, void*);
extern HelpGame* g_sharedDoc;

extern void* g_helpMsg;
extern void* g_chatMsg;
extern void* g_resourceManager;
extern char __vt__11CHlepWindow[], __vt__8CHelpMsg[], s_helpMsgSingletonBaseVtable[], kHelpOverlayFilename[];
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[], noticeTemplateStrings[];
extern int kFloat_177_HelpOverlayOriginX, kFloat_243_HelpOverlayOriginY;
extern const float kFloat_0_Notice;
extern float kFloat_1_Notice, kFloat_0_25_HelpOverlayUV, kFloat_0_5_HelpOverlay;
extern float kFloat_0_75_Help, kFloat_256_Help, kFloat_8_HelpOverlayHeight;

// A 24-byte owned notice: the first 12 bytes are a small/heap wide string,
// followed by remaining simulation seconds, active and paused words. The high
// flag bit selects owned heap text; otherwise those 12 bytes hold inline text.
// HelpEntryCopy deep-copies the heap form; the vector destroys strings before
// freeing its buffer. Expiry only clears active; it does not remove the entry.
struct HelpEntry {
    unsigned int flags;
    unsigned int length;
    unsigned short* text;
    float remainingTime;
    int active;
    int paused;
    // Construct an empty, inactive, unpaused notice with zero remaining time.
    HelpEntry() {
        flags = 0;
        length = 0;
        text = 0;
        remainingTime = 0.0f;
        active = 0;
        paused = 0;
    }
    // Free heap-form wide text; inline text needs no allocation release.
    ~HelpEntry() {
        if (flags >> 31) appFreeUni__FPv(text);
    }
};

// Copy a notice entry, duplicating heap-backed wide text and preserving its timer flags.
// Copy CONSTRUCTION into nonoverlapping raw/empty destination storage only;
// assignment over an owned heap string would leak it. s is a live valid notice.
// On success d owns an independent string. The catch only rethrows: it neither
// destroys d nor restores its former bytes; timer fields are written last.
// Callers increment their constructed count only after success. Their vector
// guards destroy that completed prefix, not the in-progress slot; a future
// throwing operation added after reserve must arrange that slot's text cleanup.
// Current ReserveCapacity/ReplaceRange implementations: golf_caddie_dialogue_strings.cpp/party_main_task_and_phf_helpers.cpp.
// For a valid disjoint source, reserve validates length<=0x7FFFFFFE and leaves
// capacity>length; the following replace at position0 into length0 fits in place
// with no overlapping tail or allocation. Thus its normal copy path cannot throw
// after ownership is acquired. Reserve growth failure occurs before publication.
// That ordering, not the rethrow catch, protects this copy-construction use.
// The staged allocator byte is unused by ReplaceRange; its indeterminate volatile
// load is retained to preserve the ABI/compiler shape, not a resource selector.
inline void HelpEntryCopy(HelpEntry* d, const HelpEntry* s) {
    try {
    if (!(s->flags >> 31)) {
        unsigned int length = s->length;
        d->flags = s->flags;
        d->length = length;
        d->text = s->text;
    } else {
        d->flags = 0;
        d->length = 0;
        d->text = 0;
        WString_ReserveCapacity(d, s->length);
        volatile char allocator;
        char allocatorTag = allocator;
        WString_ReplaceRange(d, 0, 0, s->text, s->text + s->length, &allocatorTag);
    }
    d->remainingTime = s->remainingTime;
    d->active = s->active;
    d->paused = s->paused;
    } catch (...) { __throw(0, 0, 0); }
}

struct HelpVector {
    HelpEntry* data;
    unsigned int size;
    unsigned int capacity;
};

struct HelpObj {
    void* vtable;         // 0x00
    char baseStorage[0x0C]; // 0x04: fields of the base interface, unused here
    void* messageFont;            // 0x10
    void* panel;            // 0x14
    unsigned int enabled;     // 0x18
    HelpVector vec;       // 0x1C
};

// The length_error exception machinery, as the game's MSL headers inline it.
struct HelpVectorAllocation {
    char* pointer;
    char* allocator;
    // Guard an exception-message array until the shared owner takes it.
    HelpVectorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Delete the message array unless ownership was transferred.
    ~HelpVectorAllocation() { if (pointer) __dla__FPv(pointer); }
};
struct HelpVectorControlBase {
    void* table;
    unsigned int strong, weak;
    // Initialize one strong and one weak reference with the shared-array deleter.
    HelpVectorControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct HelpVectorControl : HelpVectorControlBase {
    char* pointer;
    // Retain the message-array pointer in its shared control block.
    HelpVectorControl(char* p) { pointer = p; }
};
struct HelpVectorErrorBase {
    void* table;
    // Install the logic-error base's runtime dispatch table.
    HelpVectorErrorBase() { table = __vt__Q23std11logic_error; }
    // Destroy the std::exception base without freeing this storage.
    ~HelpVectorErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : HelpVectorErrorBase {
    char* text;
    void* control;
    // Copy the fixed length-error message into reference-counted array storage.
    length_error() {
        const char* message = noticeTemplateStrings;
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        unsigned int n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            HelpVectorAllocation allocation(p, &allocator.value);
            control = new HelpVectorControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}

// Construct and throw the vector length exception using the game's exception ABI.
inline void HelpVectorThrow() {
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct();
        exception_runtime::__throw(exception_runtime::noticeTemplateStrings + 0x14, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

struct HelpVectorBuffer {
    HelpEntry* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* owner;
    unsigned int offset;
    // Begin an empty guarded replacement buffer for notice-vector growth.
    HelpVectorBuffer(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Destroy constructed notices in reverse order, then free the buffer.
    ~HelpVectorBuffer() {
        if (data) {
            unsigned int n = size;
            size = 0;
            HelpEntry* p = data + n;
            while (n != 0) {
                --p;
                if (p) p->~HelpEntry();
                --n;
            }
            appFreeUni__FPv(data);
        }
    }
};

// Return the larger requested growth by reference, preserving the allocator expression.
inline const unsigned int& HelpMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

struct HelpRow {
    float x, y, z, w;
    // Store four rectangle/row components in call-site order.
    HelpRow(float a, float b, float c, float d) : x(a), y(b), z(c), w(d) {}
};
// Store four row components, narrowing the original double-valued arguments to floats.
inline void HelpSetRow(float* p, double a, double b, double c, double d) {
    p[0] = (float)a; p[1] = (float)b; p[2] = (float)c; p[3] = (float)d;
}
struct HelpRender {
    char bytes[0x4bc];
    // Construct the message renderer from the font owner's message resource.
    HelpRender(void* owner) {
        CMsgDraw_Ctor((char*)this, AsWFixedFont(owner)->messageResource);
    }
};
struct HelpRenderInterface {
    virtual ~HelpRenderInterface();
};

// The 0x84-byte panel/view object.
struct HelpView {
    void* vtable;          // 0x00
    int f04;               // 0x04
    int f08;               // 0x08
    void* render;          // 0x0C
    void* resource;        // 0x10
    unsigned int strFlags; // 0x14
    unsigned int strLen;   // 0x18
    unsigned short* strText; // 0x1C
    float m[6][4];         // 0x20 .. 0x7F
    int f80;               // 0x80
    // Create the overlay/renderer and initialize UV rows plus pixel rectangles.
    HelpView() {
        vtable = __vt__11CHlepWindow;
        f04 = 1;
        f08 = 0;
        render = 0;
        resource = 0;
        strFlags = 0;
        strLen = 0;
        strText = 0;
        f80 = -1;
        resource = WOverlay_Create(g_resourceManager, kHelpOverlayFilename, 0);
        const double zero = kFloat_0_Notice;
        const double one = kFloat_1_Notice;
        const double quarter = kFloat_0_25_HelpOverlayUV;
        const double half = kFloat_0_5_HelpOverlay;
        const double threequarters = kFloat_0_75_Help;
        const double width = kFloat_256_Help;
        const double height = kFloat_8_HelpOverlayHeight;
        HelpSetRow(m[0], zero, zero, one, quarter);
        HelpSetRow(m[1], zero, quarter, one, half);
        HelpSetRow(m[2], zero, threequarters, one, quarter);
        HelpSetRow(m[3], zero, zero, width, height);
        HelpSetRow(m[4], zero, zero, width, zero);
        HelpSetRow(m[5], zero, zero, width, height);
        HelpRow row0(0.0f, 0.0f, 1.0f, 0.25f);
        HelpRow row1(0.0f, 0.25f, 1.0f, 0.5f);
        HelpRow row2(0.0f, 0.75f, 1.0f, 0.25f);
        HelpRow row3(0.0f, 0.0f, 256.0f, 8.0f);
        HelpRow row4(0.0f, 0.0f, 256.0f, 0.0f);
        HelpRow row5(0.0f, 0.0f, 256.0f, 8.0f);
        m[3][2] += 72.0f;
        m[4][2] += 72.0f;
        m[5][2] += 72.0f;
        void* owner = ((ChatMessageResourceView*)g_chatMsg)->resource;
        render = new HelpRender(owner);
    }
};

#pragma push
void* HelpMsg_dtor(void*, int);
void* HelpVector_dtor(HelpVector*, int);
// Create three notice entries, a message font and the overlay panel.
// Fresh HelpObj storage and live g_chatMsg/resource manager required. Returns
// self only after panel setup; singleton publication precedes all acquisitions,
// so g_helpMsg must not be used reentrantly during construction.
// Acquisition/failure ledger (there is no transaction for font/panel ownership):
// - vector growth: HelpVectorBuffer owns replacement until commit; catch below
//   destroys committed entries/storage, clears g_helpMsg and destroys the base;
// - WFixedFont_CreateMsgEcbResource (pet_mesh_and_fixed_font.cpp) publishes messageFont before its
//   table/format setup. A later exception leaves that owned context unreleased;
// - new HelpView publishes panel only after its constructor returns. If its
//   HelpRender construction throws, new frees the failed panel allocation, but
//   HelpView has no member guard to release its already-created overlay;
// - a null panel allocation is not recovered: v->m below requires nonnull.
// Exceptions propagate. The catch does NOT call CHelpMsg_dtor or delete an
// already-created messageFont. Caller must not run normal destruction on the
// failed object: vec.data can point to storage already freed by that catch.
// Only successful construction admits CHelpMsg_dtor (font -> panel -> vector
// -> singleton/base); reclaiming failed raw storage alone does not reclaim the
// leaked acquisitions. This documents the existing failure behavior.
HelpObj* CHelpMsg_Ctor(void* p) {
    // Phase 1: publish the singleton with an empty owned notice vector.
    HelpObj* self = (HelpObj*)p;
    BaseObject_ctor(self);
    self->vtable = s_helpMsgSingletonBaseVtable;
    g_helpMsg = self;
    self->vtable = __vt__8CHelpMsg;
    self->messageFont = 0;
    self->panel = 0;
    self->enabled = 1;
    self->vec.data = 0;
    self->vec.size = 0;
    self->vec.capacity = 0;
    try {

        // Phase 2: resize to three slots. Growth constructs new entries, copies
        // old strings backward, swaps buffers, then destroys the old entries.
        if (self->vec.size < 3) {
            unsigned int count = 3 - self->vec.size;
            // Original 0x80184750 unit: fill in place or grow, then share initialization.
            do {
                if (count > self->vec.capacity || self->vec.size > self->vec.capacity - count) {
                    if (self->vec.size + count - self->vec.capacity > 0x0AAAAAAAu - self->vec.capacity) HelpVectorThrow();
                    if (self->vec.capacity < 0x038E38E3u) {
                    } else if (self->vec.capacity < 0x071C71C6u) {
                    }
                } else {
                    HelpEntry* dst = self->vec.data + self->vec.size;
                    while (count != 0) {
                        {
                            HelpEntry e;
                            if (dst) HelpEntryCopy(dst, &e);
                        }
                        --count;
                        ++dst;
                        ++self->vec.size;
                    }
                    break;
                }
                unsigned int needed = self->vec.size + count - self->vec.capacity;
                HelpVectorBuffer temp(&self->vec.capacity);
                if (needed > 0x0AAAAAAAu - self->vec.capacity) HelpVectorThrow();
                unsigned int newcap;
                if (self->vec.capacity < 0x038E38E3u) {
                    newcap = self->vec.capacity + HelpMax((3 * (self->vec.capacity + 1)) / 5, needed);
                } else if (self->vec.capacity < 0x071C71C6u) {
                    newcap = self->vec.capacity + HelpMax((self->vec.capacity + 1) / 2, needed);
                } else {
                    newcap = 0x0AAAAAAAu;
                }
                if (newcap > 0x0AAAAAAAu) HelpVectorThrow();

                HelpEntry* newdata = (HelpEntry*)appMallocDummy__FUl(newcap * 0x18);
                temp.data = newdata;
                temp.capacity = newcap;
                unsigned int oldsize = self->vec.size;
                temp.offset = oldsize;
                HelpEntry* dst = newdata + oldsize + temp.size;
                while (count != 0) {
                    {
                        HelpEntry e;
                        if (dst) HelpEntryCopy(dst, &e);
                    }
                    --count;
                    ++dst;
                    temp.size = temp.size + 1;
                }
                // New tail entries are complete; preserve existing entries by deep copy.
                HelpEntry* olddata = self->vec.data;
                HelpEntry* src = olddata + self->vec.size;
                HelpEntry* back = temp.data + temp.offset;
                while (src > olddata) {
                    --back;
                    --src;
                    if (back) HelpEntryCopy(back, src);
                    --temp.offset;
                    temp.size = temp.size + 1;
                }
                HelpEntry* start = self->vec.data + temp.offset;
                HelpEntry* dead = start + self->vec.size;
                unsigned int oldcap = self->vec.capacity;
                unsigned int size = self->vec.size;
                HelpEntry* data = self->vec.data;
                // Commit the new buffer; the temporary will free the retired buffer.
                self->vec.capacity = temp.capacity;
                temp.capacity = oldcap;
                self->vec.data = temp.data;
                self->vec.size = temp.size;
                temp.data = data;
                temp.size = size;
                while (dead > start) {
                    --dead;
                    if (dead) dead->~HelpEntry();
                }
                temp.size = 0;
            } while (0);
        } else if (self->vec.size > 3) {
            unsigned int count = self->vec.size - 3;
            HelpEntry* dst = self->vec.data + self->vec.size;
            self->vec.size -= count;
            while (count != 0) {
                --dst;
                if (dst) dst->~HelpEntry();
                --count;
            }
        }
        // Phase 3: create the message font and panel after all notice slots exist.
        WFixedFont_CreateMsgEcbResource(((ChatMessageResourceView*)g_chatMsg)->resource, &self->messageFont, 0, 0);

        HelpView* v = new HelpView();
        self->panel = v;

        int originXBits = kFloat_177_HelpOverlayOriginX;
        int originYBits = kFloat_243_HelpOverlayOriginY;
        float originX = *(float*)&originXBits;
        float originY = *(float*)&originYBits;
        v->m[5][0] = originX;
        v->m[4][0] = originX;
        v->m[3][0] = originX;
        v->m[3][1] = originY;
        v->f08 = 1;
        return self;
    } catch (...) {
        // Vector/base unwind only; font/panel acquisition is not rolled back.
        // HelpVector_dtor frees data without clearing its pointer: no second dtor.
        HelpVector_dtor(&self->vec, -1);
        HelpMsg_dtor(self, -1);
        throw;
    }
}

#pragma pop

// Advance player icon timers after all animation nodes are ready, expiring nonpersistent icons.
void HeadIcon_UpdatePlayerTimers(HeadIconUpdateView* self, float deltaSeconds) {
    // All nine animations are polled (bitwise AND, no short circuit). Timers
    // advance only if all are ready, for at most four non-inactive player slots.
    // deltaSeconds is the scaled gameplay delta traced at the opening index.
    int ready = 1;
    int i = 0;
    int offset = 0;
    for (; i < kHeadIconAnimationCount; ++i, offset += sizeof(HelpUpdateNode)) {
        ready &= ((HelpUpdateNode*)((char*)self->nodes + offset))->isLoaded();
    }
    if (ready) {
        ready = kMaximumDisplayedPlayers;
        if ((int)((CGolfDoc*)g_sharedDoc->golfDoc)->playerCount <= kMaximumDisplayedPlayers) ready = ((CGolfDoc*)g_sharedDoc->golfDoc)->playerCount;
        HelpTimer* timer = self->timers;
        for (int i = 0; i < ready; ++i) {
            if (GolfPlayer_GetEntryByIndex(((CGolfDoc*)g_sharedDoc->golfDoc), (unsigned char)i)->playerState != kInactivePlayer) {
                timer->elapsed += deltaSeconds;
                if (timer->elapsed > kFloat_3_HeadIcon && timer->kind != kPersistentHeadIcon) {
                    timer->kind = kHeadIconNone;
                    timer->elapsed = kFloat_0_0_HeadIcon;
                }
            }
            ++timer;
        }
    }
}

// Initialize this help/icon module's two-pi cache.
void HelpIcons_InitAngleCache() {
    s_initializedTwoPi_80184F48 = kFloat_6_2831855;
}

// Clear the help singleton and destroy its base object.
void* HelpMsg_dtor(void* self, int deleting) {
    if (self) {
        g_helpMsg = 0;
        BaseObject_dtor(self, 0);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Destroy wide-string entries, free vector storage and optionally delete the vector wrapper.
void* HelpVector_dtor(HelpVector* self, int deleting) {
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    if (self->data) {
                        HelpEntry* p = self->data + self->size;
                        unsigned int n = self->size;
                        self->size -= n;
                        while (n != 0) {
                            --p;
                            if (p) p->~HelpEntry();
                            --n;
                        }
                        appFreeUni__FPv(self->data);
                    }
                }
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Release the help font, panel and notice entries, then destroy the singleton base.
void* CHelpMsg_dtor(HelpObj* self, int deleting) {
    if (self) {
        self->vtable = __vt__8CHelpMsg;
        if (self->messageFont) {
            delete (HelpRenderInterface*)self->messageFont;
            self->messageFont = 0;
        }
        if (self->panel) {
            delete (HelpRenderInterface*)self->panel;
            self->panel = 0;
        }
        if (&self->vec) {
            if (&self->vec) {
                if (&self->vec) {
                    if (&self->vec) {
                        if (self->vec.data) {
                            HelpEntry* p = self->vec.data + self->vec.size;
                            unsigned int n = self->vec.size;
                            self->vec.size -= n;
                            while (n != 0) {
                                --p;
                                if (p) p->~HelpEntry();
                                --n;
                            }
                            appFreeUni__FPv(self->vec.data);
                        }
                    }
                }
            }
        }
        HelpMsg_dtor(self, 0);
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

// Destroy panel text rendering, release the overlay resource and free owned wide text.
void* CHlepWindow_dtor(HelpView* self, int deleting) {
    if (self) {
        self->vtable = __vt__11CHlepWindow;
        if (self->render) {
            delete (HelpRenderInterface*)self->render;
            self->render = 0;
        }
        if (g_resourceManager) {
            if (self->resource) {
                Object_DeleteVirtual(g_resourceManager, self->resource);
                self->resource = 0;
            }
        }
        if (&self->strFlags) {
            if (self->strFlags >> 31) appFreeUni__FPv(self->strText);
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

#pragma push
#pragma opt_common_subs on
// Select all eligible players or the turn player according to game/rule flags.
// Required live head-icon object may disable all drawing via enabled. playerCount
// must fit four slots; the all-player path does not clamp it like the timer path.
// This delegates coordinate work to stored projection anchors; it changes neither
// timers nor ownership. Inactive-player filtering differs by golfRuleType.
void HeadIcon_DrawVisiblePlayerSlots(void* self) {
    if (((HeadIconVisibility*)self)->enabled) {
        HelpGame* game = g_sharedDoc;
        unsigned int flags = ((CGolfDoc*)game->golfDoc)->gameStateFlags;
        if (flags & kDrawAllPlayerIcons) {
            for (unsigned char i = 0; i < ((CGolfDoc*)g_sharedDoc->golfDoc)->playerCount; ++i) {
                if (g_sharedDoc->golfRuleType || GolfPlayer_GetEntryByIndex(((CGolfDoc*)g_sharedDoc->golfDoc), i)->playerState != kInactivePlayer) {
                    HeadIcon_DrawPlayerSlot(self, i);
                }
            }
        } else if (flags & kDrawTurnPlayerIcon) {
            if (game->golfRuleType == 1) {
                HeadIcon_DrawPlayerSlot(self, CGolfDoc_ResolveMatchTurnFromHoleStrokes(((CGolfDoc*)game->golfDoc)));
            } else {
                HeadIcon_DrawPlayerSlot(self, ((CGolfDoc*)game->golfDoc)->currentPlayer);
            }
        } else {
            HeadIcon_DrawPlayerSlot(self, ((CGolfDoc*)game->golfDoc)->currentPlayer);
        }
    }
}
#pragma pop

#pragma push
#pragma opt_common_subs on
// Count down active/unpaused notices unless the current player's flag suppresses updates.
// self requires a constructed owned notice vector; g_sharedDoc may be null (no-op).
// delta is scaled gameplay seconds (opening clock chain). remainingTime<0
// clears active; zero stays active, and NaN also clears active via the negated >=.
// Expiry retains the entry and its owned text. No callbacks or erasure occur.
void NoticeQueue_UpdateTimers(HelpObj* self, float delta) {
    if (g_sharedDoc && !SharedDoc_IsPlayerFlag0Set(((CGolfDoc*)g_sharedDoc->golfDoc)->currentPlayer)) {
        HelpEntry* entry = self->vec.data;
        for (; entry != self->vec.data + self->vec.size; ++entry) {
            if (entry->active && !entry->paused) {
                entry->remainingTime -= delta;
                if (!(entry->remainingTime >= kFloat_0_Notice)) entry->active = 0;
            }
        }
    }
}
#pragma pop

// Draw and update use the same owner/element schema declared above.
// transform is camera-relative; x/y are projected pixel anchors, z is visibility.
typedef char HeadIconPlayerSlotSize[sizeof(HeadIconPlayerSlot) == 0x30 ? 1 : -1];
typedef HelpUpdateState HelpDrawState;
typedef char HeadIconDrawFontOffset[((int)&((HelpDrawState*)0)->font == 0xD4) ? 1 : -1];
typedef char HeadIconDrawNodesOffset[((int)&((HelpDrawState*)0)->nodes == 0x104) ? 1 : -1];
// Icon choice inputs: kind is the timed atlas selector; GolfDocSnapshot8013::itemId
// at +0xA0 is a raw inventory ID, NOT the inventory ordinal. ItemInventory_GetItemIdAt
// resolves ordinal 14 (variant cell at u=.5) and 6 (omit fallback icon). Other
// nonzero IDs use HeadIconMap; missing keys insert g_headIconMapDefaultValue.
// This is the complete local selection domain; player-facing names for ordinals
// 14/6 are not established and are unnecessary for replacing their icon resources.
struct HelpTreeNode {
    char treeLinksAndColor[0xc];
    unsigned int key;
    void* value;
};
struct HelpTreePair { unsigned int key; void* value; };
// Interleaved quad vertices: x/y/z, packed color bits, u/v (24-byte stride).
struct HeadIconVertex { float x, y, z; unsigned int colorBits; float u, v; };
struct HelpVertices { HeadIconVertex v[4]; };
typedef char HeadIconVertexSize[sizeof(HeadIconVertex) == 0x18 ? 1 : -1];
extern const HelpVertices kHeadIconQuadTemplate;
extern char s_cameraTransform[];
extern void* g_view;
extern void* g_headIconMapDefaultValue;
float Magnitude__7WVectorCFv(void*);
float __ml__FRC7WVectorRC7WVector(void*, void*);
void WOverlay_DrawTransformedRect(void*, void*, float*, float, float*, int, int, int);
unsigned int ItemInventory_GetItemIdAt(void*, int);
extern const float kFloat_64_HeadIcon, kFloat_640_HeadIcon, kFloat_60_HeadIcon, kFloat_200_HeadIcon;
extern const float kFloat_52_HeadIcon, kFloat_800_HeadIcon, kFloat_0_5_HeadIcon, kFloat_30_HeadIcon;
extern const float kFloat_28_HeadIcon, kFloat_42_HeadItemIconSize, kFloat_1_HeadIconUV, kFloat_0_8125_HeadIconUV;
extern const float kFloat_0_6875_HeadIconUV, kFloat_4_HeadIconFrameOffsetX, kFloat_1200_HeadIcon, kFloat_10_HeadIcon;
extern const float kFloat_26_HeadIconFrameHalfWidth, kFloat_11_HeadIconFrameHalfHeight;
HelpTreeNode* HeadIconMap_FindInsertionPosition(void*, unsigned int*, HelpTreeNode**, unsigned char*, unsigned char*);
HelpTreeNode* HudFontConfigMap_InsertAndRebalance(void*, HelpTreeNode*, unsigned char, unsigned char, HelpTreePair*);
// Find an item-icon map entry, inserting the shared default resource if absent.
// Required tree/key are borrowed; returns a borrowed {key,resource} pair, not a
// new owned texture. Insertion allocates/rebalances map nodes and may throw.
// Key is an inventory item ID. Do not confuse this map with a font registry even
// though the shared insertion symbol retains the HudFontConfigMap prefix.
// tree is CHeadIcon's embedded overlay map (head_icon_and_aux_menu_ui.cpp), not a global resource
// cache. The pair address survives rebalancing/insertion but expires on erase
// or CHeadIcon_dtor; copying it does not extend the overlay lifetime. Do not
// free the pair/node/resource from this lookup's consumer. The constructor
// installs WOverlay_Create results in these nodes; CHeadIcon_dtor releases each
// nonnull overlay with Object_DeleteVirtual(g_resourceManager, overlay), then
// frees the nodes (manager must remain live for that release).
// Missing keys copy the CURRENT default pointer, with no retain/addref. Its
// stock definition in phase1_shared_data is zero: insertion may return a null
// value and is not successful resource acquisition. Replacing the default with
// a nonnull shared object would give multiple nodes the same release obligation;
// such a mod needs an ownership policy, not just a different default pointer.
inline HelpTreePair* HelpLookup(void* tree, unsigned int* key) {
    HelpTreeNode* found;
    unsigned char insertLeft, noPredecessor;
    HelpTreeNode* insertionPosition = HeadIconMap_FindInsertionPosition(tree, key, &found, &insertLeft, &noPredecessor);
    HelpTreePair* pair;
    if (!found || (pair = (HelpTreePair*)&found->key)->key < *key) {
        HelpTreePair insert;
        insert.key = *key;
        insert.value = g_headIconMapDefaultValue;
        pair = (HelpTreePair*)&HudFontConfigMap_InsertAndRebalance(tree, insertionPosition, insertLeft, noPredecessor, &insert)->key;
    }
    return pair;
}
void UIRender_SetDefaultState();
void Engine_SetFlexibleVertexFormat(unsigned int);
int GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, int, void*, int);

#pragma push
#pragma opt_common_subs on
// Draw a player's timed/item icon and bind/draw its frame at the projected anchor.
// index must be 0..3; resources and projected slot must exist. Polls all nine
// texture readiness methods first. Not ready, negative projection visibility,
// camera-backfacing or distance>640 returns without drawing; distance must be >0
// for inverse-distance terms. No timing, world projection or ownership changes.
// kind!=0xFFFF chooses a timed atlas cell (kind modulo 16); otherwise inventory
// entry 14 selects an atlas variant, entry 6 omits the fallback, other IDs draw
// a map-supplied item image. Missing map keys INSERT a default resource.
// Frame omission uses whole-state equality 0x200. Texture selection follows the
// player's flag and modes 0/6; no animation stepping occurs in this draw routine.
// Writes scratch UV/rect, may mutate the item-resource map, and changes render
// state/texture binding without restoring a saved snapshot. Preserve call order.
void HeadIcon_DrawPlayerSlot(void* p, unsigned char index) {
    // Caller supplies a valid player slot. slot.x/y are projected pixel anchors;
    // slot.z is the projection visibility test. transform holds the relative
    // vector used for distance and the camera-forward dot-product cull.
    HelpDrawState* self = (HelpDrawState*)p;
    // One object view owns the prefix and four real records; slot is an element.
    HeadIconPlayerSlot* slot = &self->players[index];
    void* context;
    unsigned int* player;
    int offset;
    int ready = 1;
    int i = 0;
    offset = 0;
    float iconUV[4];
    for (; i < kHeadIconAnimationCount; ++i, offset += sizeof(HelpUpdateNode)) {
        ready &= ((HelpUpdateNode*)((char*)self->nodes + offset))->isLoaded();
    }
    if (!ready) return;
    if (slot->z < kFloat_0_0_HeadIcon) return;
    float distance = (*(WVector*)slot->transform).Magnitude();
    float visible = (*(WVector*)slot->transform * *(WVector*)(s_cameraTransform + 0x18));
    if (visible < kFloat_0_0_HeadIcon || distance > kFloat_640_HeadIcon) return;
    // Phase 1: choose a timed atlas icon, or fall back to the player's item icon.
    // The code assumes distance != 0 for the inverse-distance screen offsets.
    if (slot->kind < kHeadIconNone) {
        int kind = slot->kind;
        self->uv[0] = (kFloat_64_HeadIcon * (kind % kHeadIconAtlasColumns)) / (float)kHeadIconAtlasPixels;
        self->uv[1] = (kFloat_64_HeadIcon * ((kind % kHeadIconAtlasCellCount) / kHeadIconAtlasColumns)) / (float)kHeadIconAtlasPixels;
        self->rect[0] = (int)(slot->x - kFloat_60_HeadIcon - kFloat_200_HeadIcon / distance);
        self->rect[1] = (int)(slot->y - kFloat_52_HeadIcon - kFloat_800_HeadIcon / distance);
        WOverlay_DrawTransformedRect(self->font, g_view, self->uv, kFloat_0_0_HeadIcon, self->rect, 0, -1, 0);
    } else if (*(unsigned int*)&((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(((CGolfDoc*)g_sharedDoc->golfDoc), index))->itemId) {
        self->uv[1] = kFloat_0_0_HeadIcon;
        self->rect[0] = (int)(slot->x - kFloat_60_HeadIcon - kFloat_200_HeadIcon / distance);
        self->rect[1] = (int)(slot->y - kFloat_52_HeadIcon - kFloat_800_HeadIcon / distance);
        context = AsCSharedDoc(g_sharedDoc)->itemInventory;
        player = (unsigned int*)GolfPlayer_GetEntryByIndex(((CGolfDoc*)g_sharedDoc->golfDoc), index);
        unsigned int expected = ItemInventory_GetItemIdAt(context, kHeadIconInventoryVariantEntry);
        if (*(unsigned int*)&((GolfDocSnapshot8013*)player)->itemId == expected) {
            self->uv[0] = kFloat_0_5_HeadIcon;
            WOverlay_DrawTransformedRect(self->font, g_view, self->uv, kFloat_0_0_HeadIcon, self->rect, 0, -1, 0);
        } else {
            context = AsCSharedDoc(g_sharedDoc)->itemInventory;
            player = (unsigned int*)GolfPlayer_GetEntryByIndex(((CGolfDoc*)g_sharedDoc->golfDoc), index);
            expected = ItemInventory_GetItemIdAt(context, kHeadIconInventoryOmittedEntry);
            if (*(unsigned int*)&((GolfDocSnapshot8013*)player)->itemId != expected) {
            self->uv[0] = kFloat_0_0_HeadIcon;
            WOverlay_DrawTransformedRect(self->font, g_view, self->uv, kFloat_0_0_HeadIcon, self->rect, 0, -1, 0);
            HelpTreePair* pair = HelpLookup(self->tree, (unsigned int*)&((GolfDocSnapshot8013*)GolfPlayer_GetEntryByIndex(((CGolfDoc*)g_sharedDoc->golfDoc), index))->itemId);
            float uv[4];
            HelpRow rect(kFloat_28_HeadIcon + self->rect[0], kFloat_30_HeadIcon + self->rect[1], kFloat_42_HeadItemIconSize, kFloat_42_HeadItemIconSize);
            uv[0] = kFloat_0_0_HeadIcon;
            uv[1] = kFloat_0_0_HeadIcon;
            uv[2] = kFloat_1_HeadIconUV;
            uv[3] = kFloat_1_HeadIconUV;
            WOverlay_DrawTransformedRect(pair->value, g_view, uv, kFloat_0_0_HeadIcon, &rect.x, 0, -1, 0);
            }
        }
    }
    // Phase 2: draw the frame unless the entire state word equals 0x200.
    // This is an equality test, not a bit-mask test. Animation slots 0..3 are
    // normal, 4..7 flagged-player variants, and 8 the shared variant in modes 0/6.
    if ((unsigned int)((CGolfDoc*)g_sharedDoc->golfDoc)->gameStateFlags != kGameStateHidingIconFrame) {
        UIRender_SetDefaultState();
        iconUV[1] = kFloat_0_0_HeadIcon;
        iconUV[0] = kFloat_0_0_HeadIcon;
        iconUV[2] = kFloat_0_8125_HeadIconUV;
        iconUV[3] = kFloat_0_6875_HeadIconUV;
        Engine_SetFlexibleVertexFormat(kHeadIconQuadFormat);
        if (SharedDoc_IsPlayerFlag0Set(index)) {
            unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
            if (mode == 0 || mode == 6) self->nodes[8].unused3();
            else self->nodes[index + 4].unused3();
        } else self->nodes[index].unused3();
        float x = floor(kFloat_4_HeadIconFrameOffsetX + slot->x);
        float y = floor(slot->y - kFloat_1200_HeadIcon / distance - kFloat_10_HeadIcon);
        HelpVertices vertices = kHeadIconQuadTemplate;
        vertices.v[0].x = x - kFloat_26_HeadIconFrameHalfWidth;
        vertices.v[0].y = y - kFloat_11_HeadIconFrameHalfHeight;
        vertices.v[0].u = iconUV[0];
        vertices.v[0].v = iconUV[1];
        vertices.v[1].x = kFloat_26_HeadIconFrameHalfWidth + x;
        vertices.v[1].y = y - kFloat_11_HeadIconFrameHalfHeight;
        vertices.v[1].u = iconUV[2];
        vertices.v[1].v = iconUV[1];
        vertices.v[2].x = x - kFloat_26_HeadIconFrameHalfWidth;
        vertices.v[2].y = kFloat_11_HeadIconFrameHalfHeight + y;
        vertices.v[2].u = iconUV[0];
        vertices.v[2].v = iconUV[3];
        vertices.v[3].x = kFloat_26_HeadIconFrameHalfWidth + x;
        vertices.v[3].y = kFloat_11_HeadIconFrameHalfHeight + y;
        vertices.v[3].u = iconUV[2];
        vertices.v[3].v = iconUV[3];
        int count = GX_GetPrimitiveCount(kHeadIconQuadVertices, kHeadIconTriangleStrip);
        DrawPrimitiveUP(kHeadIconTriangleStrip, count, &vertices, 0x18);
    }
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462148", _ctors_u80184750_0, HelpIcons_InitAngleCache);
#pragma pop
