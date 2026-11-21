// Golf letters: Pangya/timeout/wait/replay overlays, scripted messages and captions.
// Frame rate lookup: CLetter_OnProcess has TWO clocks. Scripted letters wait for
// Bp completion, then kDefaultHoldThreshold=20 or kScaledHoldThreshold=17 calls;
// overlays instead use dt for opacity and resource time. OnDisplay also changes
// blink state, so drawing is not a pure read. No presentation interval is set here.
// Widescreen lookup: OnLoad positions wait/demo at (.5W,.18H), replay at (45,88),
// fast-forward at (45,60); AddNamedOverlay defaults to (.5W,.25H). Coordinates
// are viewport/layout units, X right/Y down, truncated to integers for origins.
// The source dimensions, UV extents, viewport anchors and caption offsets must
// stay consistent. kFloat_* pools live in u80193E80_data.cpp and phase1_shared_data.cpp;
// kLetterCaptionExtraGlyphAdvance is GLYPH SPACING, despite its legacy name.
// CLetter.h is the shared ABI; Obj/FadeObj/EffectOwner below overlay the same
// receiver with display/fade/container views; the draw/caption prefixes overlay one Effect.
// Lifecycle: OnLoad owns overlays/font, HandleMsg (main_frame_letter_and_mouse_cursor.cpp) selects borrowed
// pointers into that registry, OnDestroy/dtor (sun_rays_lens_flare_effects.cpp) release resources.
// Mutexes protect ring/hash operations, not the entire display/process lifecycle.
// Translation unit 0x80193E80-0x80194F9C (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CLetter.h"
#include "classes/WView.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"

// Borrow the shared letter ABI view; caller retains storage and resources.
// A cast-only macro is retained: an inline function changes CodeWarrior's
// emitted cleanup/call shape in this unit (checked on isolated objects).
#define AsCLetter(p) ((CLetter*)(p))

enum LetterFontLayout {
    kLetterGlyphHeight = 32, kLetterGlyphWidth = 20,
    kLetterAtlasWidth = 256, kLetterAtlasHeight = 64, kLetterAtlasImageCount = 1,
    kLetterNameFirstByteMask = 0x2a, kLetterNameLastByteMask = 0x15,
    kLetterNameLengthShift = 6
};
extern "C" {

extern void HeadIcon_DrawVisiblePlayerSlots(void*);
extern void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, int, int, int);
extern void __ct__7CBpCtrlFv(void*);
extern void CBpCtrl_dtor(void);
extern void* CBpCtrl_CreateEntry(void*, const char*, void*, int);
extern int CBpCtrl_PollAllEntriesLoaded(void*);
extern void __register_global_object(void*, void*, void*);
extern void* IdMap_Lookup(void*, void*);
extern void BpObject_StartScriptByName(void*, void*, int);
extern int BpObject_IsCurrentScriptComplete(void*);
extern int EventTiming_IsDurationScalingEnabled(void);
extern int CInputManager_HasAnyMonitoredAction(void*);
extern void BpObject_TryInvokeCurrentCallback(void*, int);
extern void BpObject_Draw(void*);
extern char* String_FormatTemporary(const char*, ...);
// Retained Wii ABI grouping: the third argument occupies the drawFlags GPR;
// the four floats occupy x/y/extraGlyphAdvance/glyphHeightScale FPRs. The
// definition in main_frame_letter_and_mouse_cursor.cpp spells the same register arguments in another order.
extern void CLetter_DrawText(void*, char*, float, float, float, float, int, int);
extern void* __nw__FUl(unsigned int);
extern void __dl__FPv(void*);
extern void* WOverlay_Create(void*, const char*, int);
extern void Object_DeleteVirtual(void*, void*);
extern int WOverlay_GetWidth(void*);
extern int WOverlay_GetHeight(void*);
extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);
extern char* strcpy(char*, const char*);
extern void* appMallocUni__FUl(unsigned int);
extern int ResourceManager_CreateWTitleFont(void*);
extern void FontAtlas_InitImagesAndGlyphWidths(void*, void*, int, float);
extern int strcmp(const char*, const char*);
extern unsigned int strlen(const char*);

extern void* g_headIcon;
extern unsigned char s_letterBlinkPhase;
extern unsigned char s_letterBlinkOverlayEnabled;
extern void* g_view;
extern void* g_resourceManager;
extern signed char s_bpCtrlInitialized;
extern void* g_sharedDoc;
extern void* g_inputPointer;
extern const float kFloat_1_Letter; // Shared 1: font scale, unit UV extent and blink threshold, not a frame-rate scalar.
extern const float kFloat_0_18_Rival; // .18 viewport-height wait/demo anchor.
extern const float kFloat_45_Rival; // 45 viewport units: replay/fast-forward X center.
extern const float kFloat_0_5_Rival; // Shared .5: center subtraction, viewport X anchor and blink reset.
extern const float kFloat_0_Letter;
extern const float kFloat_88_Letter; // 88: replay Y center AND fast-forward destination X when replay is present.
extern const float kFloat_60_LetterOverlay; // 60 viewport units: fast-forward Y center.
extern const float kFloat_13_Letter; // 13 viewport units: fast-forward destination X without replay.
extern const float kFloat_5_LetterOverlay; // 5 viewport units: caption X/Y margin.
extern const float kFloat_0_25_LetterOverlay; // .25 viewport height: default overlay/caption Y anchor.
extern const float kLetterCaptionExtraGlyphAdvance; // 3 extra glyph-advance units; leading float of eight-byte slot.
extern const float kFloat_255_LetterFade; // 255 alpha units: full opacity AND fade numerator.
extern char s_bpCtrl[];
extern void CBpCtrl_dtor(void);
extern char s_bpCtrlDtorRegistration_ItemWindow[];
struct LetterStringPool {
    char animationResource[0x12];
    char pangyaName[8];
    char pangyaResource[0x13];
    char timeoutName[0xD];
    char timeoutResource[0xD];
    char waitName[5];
    char waitResource[0xA];
    char replayName[7];
    char replayResource[0x11];
    char fastForwardName[0xC];
    char fastForwardResource[0x12];
    char demoName[5];
    char demoResource[0xA];
    char fontResource[0xD];
    char fontCharacters[0x13];
    char captionFormat[3];
    char tail[0xC8 - 0xBE];
};
extern LetterStringPool golfLetterOverlayStrings;
extern char s_letterAnimationScriptTriplets[];

struct Obj {
    char pad[0x18];
    void* currentMessage;
    char pad1c[0x38];
    float messageAlpha;
    void* messageState;
    char pad5c[4];
    unsigned char messageDisplayEnabled;
    char pad61[3];
    void* bpEntryId;
    void* scriptedMessageIndex;
    char pad6c[0x34];
    void* fastForwardIcon;
    void* replayIcon;
    float replayBlinkTimer;
};

// Same prefix as Effect: +0 WOverlay handle, +4 destination {X,Y,width,height},
// +14 source {left,top,width,height} in normalized UV units. These are sprite
// rectangles, not a font/size/string tuple. Preserve the original pointer ABI.
struct LetterOverlayDrawPrefix { void* resource; float destination[4]; float sourceUv[4]; };
// +4C options overlay the same Effect's trailing storage. CLetter_DrawText
// consumes +54 as glyph height scale and +58 as packed color, not alpha/style.
// +50 is a signed value formatted with %d; it never owns or points to text.
struct LetterCaptionOptions { char pad[0x4c]; unsigned char showCaption; int captionValue; float glyphHeightScale; int packedColor; };

struct InstanceLock_u80193E80 {
    void* target;
    // Acquire a borrowed recursive mutex for the scope.
    InstanceLock_u80193E80(void* p) : target(p) { OSLockMutex(p); }
    // Release the same mutex without destroying it.
    ~InstanceLock_u80193E80() { OSUnlockMutex(target); }
};

struct Node { void* data; const char* name; unsigned char flag; char pad[3]; Node* next; Node* prev; Node* hashNext; };
struct Anim { void* ptr; };
struct Effect {
    void* resource;
    // Begin with no owned overlay; remaining fields are filled by the insertion path.
    Effect() : resource(0) {}
    float screenX;
    float screenY;
    float displayWidth;
    float displayHeight;
    float left;
    float top;
    float right;
    float bottom;
    float time;
    char name[32];
    unsigned char dismissOnInput;
    char pad49[3];
    unsigned char showCaption;
    char rest[15];
};
// Three rings share Node::next/prev: allocationHead links each allocation's
// slot 0 (reserved as a block header); freeNodes links unused slots 1..count-1;
// activeEffects links checked-out slots. A null head means empty, otherwise
// prev/next form a circular ring with no extra sentinel node. hashNext separately
// chains active names, which borrow Effect::name; the Effect owns its WOverlay.
// All mutations use the same recursive mutex, including nested guard scopes.
struct EffectOwner {
    char pad00[0x1c]; Node* activeEffects;
    char pad20[4]; Node* allocationHead; Node* freeNodes; int count; unsigned int nameHashMask;
    char pad34[4]; Node** nameBuckets;
    char pad3c[0x0c];
    // +3c is mutex storage in CLetter, not a Node sentinel.
};

void OverlayEffects_AddNamedOverlay(void*, const char*, const char*, float*, unsigned char);

// Look up a named overlay under the recursive name-table mutex.
inline Effect* FindEffect(void* owner, const unsigned char* key) {
    InstanceLock_u80193E80 guard(AsCLetter(owner)->nameHashMutex);
    if (!key || !((CLetter*)owner)->nameHashBuckets) return 0;
    void* mutex = AsCLetter(owner)->nameHashMutex;
    OSLockMutex(mutex);
    int nameLength = (int)strlen((const char*)key);
    int lastNameByte = nameLength > 0 ? nameLength - 1 : 0;
    unsigned int bucketIndex = (((key[0] & kLetterNameFirstByteMask) | (key[lastNameByte] & kLetterNameLastByteMask)) | (nameLength << kLetterNameLengthShift)) & AsCLetter(owner)->nameHashMask;
    OSUnlockMutex(mutex);
    Node* overlayNode = ((Node**)((CLetter*)owner)->nameHashBuckets)[bucketIndex];
    while (overlayNode) {
        if (!strcmp(overlayNode->name, (const char*)key)) return (Effect*)overlayNode->data;
        overlayNode = overlayNode->hashNext;
    }
    return 0;
}

#pragma opt_common_subs off

// Load the letter animation, position its overlays, and build the title font.
// Required initialized owner; resources remain registry-owned until OnDestroy.
// The Bp poll is a synchronous busy wait with no timeout. Keep resource creation
// before placement: failed WOverlay loads simply leave the corresponding name absent.
// Font configuration contains a pointer to local label only for this synchronous
// initializer; do not defer its use beyond this call without giving it stable storage.
void CLetter_OnLoad(void* p) {
    // Phase 1: create the Bp animation entry and wait until all its resources load.
    char* self = (char*)p;
    float f30, f31;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ItemWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_CreateEntry(Sing_805DC5F0(), golfLetterOverlayStrings.animationResource, (AsCLetter(self)->pad_061 + 3), 0);
    do {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ItemWindow);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
    } while (!CBpCtrl_PollAllEntriesLoaded(Sing_805DC5F0()));

    // Phase 2: create named overlays, then place selected ones in viewport pixels.
    OverlayEffects_AddNamedOverlay(self, golfLetterOverlayStrings.pangyaName, golfLetterOverlayStrings.pangyaResource, 0, 0);
    OverlayEffects_AddNamedOverlay(self, golfLetterOverlayStrings.timeoutName, golfLetterOverlayStrings.timeoutResource, 0, 0);
    OverlayEffects_AddNamedOverlay(self, golfLetterOverlayStrings.waitName, golfLetterOverlayStrings.waitResource, 0, 0);

    {
        void* screen = g_view;
        const unsigned char* key = (const unsigned char*)(golfLetterOverlayStrings.waitName);
        float h = ((WView*)screen)->viewportHeight;
        float w = ((WView*)screen)->viewportWidth;
        f30 = kFloat_0_18_Rival * h;
        f31 = kFloat_0_5_Rival * w;
        Effect* found = FindEffect(self, key);
        if (found) {
            found->screenX = (float)(int)(f31 - kFloat_0_5_Rival * found->displayWidth);
            found->screenY = (float)(int)(f30 - kFloat_0_5_Rival * found->displayHeight);
        }
    }

    OverlayEffects_AddNamedOverlay(self, golfLetterOverlayStrings.replayName, golfLetterOverlayStrings.replayResource, 0, 0);

    {
        const unsigned char* key = (const unsigned char*)(golfLetterOverlayStrings.replayName);
        Effect* found = FindEffect(self, key);
        if (found) {
            found->screenX = (float)(int)(kFloat_45_Rival - kFloat_0_5_Rival * found->displayWidth);
            found->screenY = (float)(int)(kFloat_88_Letter - kFloat_0_5_Rival * found->displayHeight);
        }
    }

    OverlayEffects_AddNamedOverlay(self, golfLetterOverlayStrings.fastForwardName, golfLetterOverlayStrings.fastForwardResource, 0, 0);

    {
        const unsigned char* key = (const unsigned char*)(golfLetterOverlayStrings.fastForwardName);
        Effect* found = FindEffect(self, key);
        if (found) {
            found->screenX = (float)(int)(kFloat_45_Rival - kFloat_0_5_Rival * found->displayWidth);
            found->screenY = (float)(int)(kFloat_60_LetterOverlay - kFloat_0_5_Rival * found->displayHeight);
        }
    }

    OverlayEffects_AddNamedOverlay(self, golfLetterOverlayStrings.demoName, golfLetterOverlayStrings.demoResource, 0, 0);

    {
        void* screen = g_view;
        const unsigned char* key = (const unsigned char*)(golfLetterOverlayStrings.demoName);
        float h = ((WView*)screen)->viewportHeight;
        float w = ((WView*)screen)->viewportWidth;
        f30 = kFloat_0_18_Rival * h;
        f31 = kFloat_0_5_Rival * w;
        Effect* found = FindEffect(self, key);
        if (found) {
            found->screenX = (float)(int)(f31 - kFloat_0_5_Rival * found->displayWidth);
            found->screenY = (float)(int)(f30 - kFloat_0_5_Rival * found->displayHeight);
        }
    }

    // Phase 3: build the title-font atlas configuration and initialize its glyph data.
    AsCLetter(self)->titleFont = (void*)ResourceManager_CreateWTitleFont(g_resourceManager);
    char* label = golfLetterOverlayStrings.fontResource;
    AsCLetter(self)->fontImageNames = &label;
    // This final atlas-configuration word is passed to the font initializer with the dimensions below.
    AsCLetter(self)->unk_94 = 0;
    // Texels: changing atlas dimensions also requires matching fontResource data,
    // character order and glyph sizes. These are not the output resolution.
    AsCLetter(self)->fontGlyphHeight = kLetterGlyphHeight;
    AsCLetter(self)->fontGlyphWidth = kLetterGlyphWidth;
    AsCLetter(self)->fontImageCount = kLetterAtlasImageCount;
    AsCLetter(self)->fontTextureWidth = kLetterAtlasWidth;
    AsCLetter(self)->fontTextureHeight = kLetterAtlasHeight;
    AsCLetter(self)->fontCharacters = golfLetterOverlayStrings.fontCharacters;
    FontAtlas_InitImagesAndGlyphWidths(AsCLetter(self)->titleFont, ((char*)&AsCLetter(self)->fontImageNames), 0, kFloat_1_Letter);
}

#pragma opt_common_subs reset

// Acquire the shared recursive name/list mutex for a ring operation.
inline void LockList(EffectOwner* o) { OSLockMutex(AsCLetter(o)->nameHashMutex); }
// Release one acquisition of the shared recursive name/list mutex.
inline void UnlockList(EffectOwner* o) { OSUnlockMutex(AsCLetter(o)->nameHashMutex); }

// Splice one pooled node into a ring under the existing recursive mutex.
// A null head becomes a one-node ring; ownership stays with the pool.
static inline void LinkOverlayPoolRing(EffectOwner* owner, Node* node, Node*& head) {
    LockList(owner);
    if (!head) {
        node->next = node;
        node->prev = node;
        UnlockList(owner);
        head = node;
    } else {
        node->prev = head;
        node->next = head->next;
        head->next = node;
        node->next->prev = node;
        UnlockList(owner);
    }
}

// Create a centered overlay and insert its pooled node into the list and name hash.
// name/resource are required NUL-terminated strings, borrowed during the call;
// strlen(name) <= 31 (Effect::name[32], unchecked strcpy). Optional rect points
// to {sourceX,sourceY,sourceWidth,sourceHeight} in source texture pixels, NOT LTRB
// endpoints: components 2/3 become displayWidth/Height. WOverlay_DrawTransformedRect
// consumes the normalized {origin,extent} at Effect+14. Require nonzero source
// dimensions; for an in-image crop use X,Y>=0, W,H>=0, X+W<=width, Y+H<=height.
// No crop clamping occurs. Null rect selects the whole image. flag!=0 enables
// input dismissal. Requires a valid pool configuration with count > 1.
void OverlayEffects_AddNamedOverlay(void* ownerArg, const char* name, const char* resource, float* rect, unsigned char flag) {
    EffectOwner* owner;
    Effect* overlay;
    void* overlayResource;
    float width;
    float height;
    Node* overlayNode;
    Node* nodes;
    Node* allocationRing;
    int i;
    Node* freeSlot;
    Node* next;
    int nameLength;
    int lastNameByte;
    unsigned int bucketIndex;
    owner = (EffectOwner*)ownerArg;
    // Phase 1: create the overlay before publishing any pool or hash links.
    overlay = new Effect();
    overlayResource = (void*)WOverlay_Create(g_resourceManager, resource, 0);
    overlay->resource = overlayResource;
    if (!overlayResource) {
        if (overlay) {
            if (overlayResource) {
                Object_DeleteVirtual(g_resourceManager, overlayResource);
                overlay->resource = 0;
            }
            delete overlay;
        }
        return;
    }
    strcpy(overlay->name, name);
    overlay->time = kFloat_0_Letter;
    width = (float)WOverlay_GetWidth(overlay->resource);
    height = (float)WOverlay_GetHeight(overlay->resource);
    if (rect) {
        overlay->left = rect[0] / width;
        overlay->top = rect[1] / height;
        overlay->right = rect[2] / width;
        overlay->bottom = rect[3] / height;
    } else {
        struct { float x, y, w, h; } def = { kFloat_0_Letter, kFloat_0_Letter, kFloat_1_Letter, kFloat_1_Letter };
        overlay->left = def.x;
        overlay->top = def.y;
        overlay->right = def.w;
        overlay->bottom = def.h;
    }
    overlay->displayWidth = width * overlay->right;
    overlay->displayHeight = height * overlay->bottom;
    overlay->screenX = (float)(int)(((WView*)g_view)->viewportWidth * kFloat_0_5_Rival - kFloat_0_5_Rival * overlay->displayWidth);
    overlay->screenY = (float)(int)(((WView*)g_view)->viewportHeight * kFloat_0_25_LetterOverlay - kFloat_0_5_Rival * overlay->displayHeight);
    overlay->dismissOnInput = flag;
    overlay->showCaption = 0;

    // Phase 2: reserve a free slot under the outer recursive lock. Growing the
    // pool adds one block header to allocationHead and its remaining slots to freeNodes.
    const unsigned char* key = (const unsigned char*)overlay->name;
    InstanceLock_u80193E80 lock1(AsCLetter(owner)->nameHashMutex);
    {
        InstanceLock_u80193E80 lock2(AsCLetter(owner)->nameHashMutex);
        if (!owner->freeNodes) {
            nodes = (Node*)appMallocUni__FUl((unsigned int)owner->count * sizeof(Node));
            allocationRing = owner->allocationHead;
            LinkOverlayPoolRing(owner, nodes, allocationRing);
            owner->allocationHead = allocationRing;
            if (owner->count > 1) {
            i = 1;
            freeSlot = nodes + 1;
            for (; i < owner->count; ++i) {
                Node* head = owner->freeNodes;
                LinkOverlayPoolRing(owner, freeSlot, head);
                owner->freeNodes = head;
                freeSlot = freeSlot + 1;
            }
            }
        }
        overlayNode = owner->freeNodes;
        LockList(owner);
        overlayNode->prev->next = overlayNode->next;
        overlayNode->next->prev = overlayNode->prev;
        next = overlayNode->prev;
        if (next == overlayNode) next = 0;
        UnlockList(owner);
        owner->freeNodes = next;
    }
    // Phase 3: the checked-out node owns the Effect; publish the borrowed name
    // in its hash bucket, then splice the same node into the active ring.
    overlayNode->data = overlay;
    overlayNode->flag = 0;
    overlayNode->name = (const char*)key;
    if (key) {
        LockList(owner);
        nameLength = (int)strlen((const char*)key);
        lastNameByte = nameLength > 0 ? nameLength - 1 : 0;
        bucketIndex = (((key[0] & kLetterNameFirstByteMask) | (key[lastNameByte] & kLetterNameLastByteMask)) | (nameLength << kLetterNameLengthShift)) & owner->nameHashMask;
        UnlockList(owner);
        LockList(owner);
        overlayNode->hashNext = owner->nameBuckets[bucketIndex];
        owner->nameBuckets[bucketIndex] = overlayNode;
        UnlockList(owner);
    }
    Node* head = owner->activeEffects;
    LockList(owner);
    if (!head) {
        overlayNode->next = overlayNode;
        overlayNode->prev = overlayNode;
        UnlockList(owner);
    } else {
        overlayNode->prev = head;
        overlayNode->next = head->next;
        head->next = overlayNode;
        overlayNode->next->prev = overlayNode;
        UnlockList(owner);
        overlayNode = head;
    }
    owner->activeEffects = overlayNode;
}

enum LetterStates { kLetterHidden, kLetterFadeIn, kLetterHolding, kLetterFadeOut };
// Display admission uses the COMMITTED CGolfDoc state word, not a property of
// the requested letter. CGolfRule_EnterState (golf_rule_round_flow_and_shot_sim.cpp) publishes that word
// after its drop-confirmation gate. 0x20000 is its PostRoundMenus state: entry
// settles rewards/award camera; CGolfRule_ProcessPostRoundMenus drives that phase.
// CLetter_HandleMsg (main_frame_letter_and_mouse_cursor.cpp) still selects/starts letters in those states;
// OnDisplay suppresses only the selected overlay and caption while EITHER bit
// below is set. Head icons, replay/fast-forward icons and scripted BP drawing
// happen earlier and are not suppressed by this test. OnProcess still advances
// fades/timers: clearing the state does not restart the hidden interval.
// 0x400 is the other accepted suppression state. EnterState has only common
// publication/reset for it; no direct request of 0x400 was found in src/include.
// Its gameplay label is unestablished; preserve its explicit admission policy
// rather than relabeling it as pause/replay. Mask tests also reject combined states.
enum LetterMasks {
    kCancelLetterMask = 0x8210,
    kSuppressLetterTextFlag20000 = 0x20000, // Post-round menu/award phase.
    kSuppressLetterTextFlag400 = 0x400,     // Unnamed suppression state; producer scope above.
    kHideLetterTextMask = kSuppressLetterTextFlag20000 | kSuppressLetterTextFlag400
};
enum LetterScriptLayout { kLetterScriptRecordCount = 16, kLetterScriptWords = 3, kFadeInScriptWord = 1, kFadeOutScriptWord = 2 };
enum LetterHoldUpdates { kScaledHoldThreshold = 17, kDefaultHoldThreshold = 20 };
// Thresholds count COMPLETED-script calls only. Compare-before-increment means
// threshold N schedules fade-out on completed hold call N+1 (starting at zero),
// then the following process call starts the fade-out script. Larger holds last
// longer at a fixed update cadence. Safe retuning ranges have not been established.

struct FadeObj {
    char pad[0x18];
    void* effect;
    char pad1c[0x38];
    float opacity; // +54: opacity in [0,255], clamped after every active update.
    int state;
    float messageFadeDurationSeconds; // +5c: finite and >0 for forward overlay fades.
    char pad60[4];
    void* bpEntryId; // +64: one Bp entry ID passed to IdMap_Lookup, not an array.
    int scriptedMessageIndex; // +68: known-message/script-triplet index (0 selects the overlay fade path).
    int completedScriptHoldUpdates; // +6c: number of holding updates after the Bp script completes.
    char rest[0x38];
    float elapsed;
};

#pragma opt_strength_reduction off
// Advance scripted letters or timed alpha fades, including input dismissal.
// scriptedMessageIndex aliases CLetter::scriptedMessageIndex at +68.
// Nonzero indices must be in [1,15].
// dt is seconds: CMainFrame_UpdateVirtualHooks (main_frame_letter_and_mouse_cursor.cpp) supplies 1/updateRate
// to actor processing. It is added to +A8, alpha integrates dt/duration, and timed
// overlays subtract dt from Effect::time; both times/duration are seconds. Duration
// +5C defaults to .5 in CLetter_OnPreLoadInit. The ABI does not validate dt or
// nonzero duration. Script completion/counter changes must remain independent
// of dt; an exactly-zero hold timer does NOT start fade-out (strict <0 test).
// Required self; currentMessage and Bp entry remain borrowed. Cancelling by
// game flags hides the letter without freeing its registry resource.
// Forward-fade admission: finite dt>=0 seconds and finite duration>0 seconds;
// construction supplies 0.5s and no other source writer of +5C was found. The
// engine supplies a positive reciprocal update-rate step. Zero dt leaves alpha
// unchanged but still runs script/input/counter logic; it is not a pause API.
// Positive duration has no enforced upper bound; larger values slow the fade.
// A negative duration reverses the alpha step: fade-in clamps back to zero and
// never reaches holding; fade-out rises/clamps to 255 and never becomes hidden
// for positive dt. Negative dt similarly reverses normal fades and hold time.
// Zero/NaN duration is unsupported, with no finite-value check or recovery.
// Keep strict >255 and <0 completion tests: landing exactly on an endpoint
// waits for a later positive step. The final clamp alone does not finish a fade.
void CLetter_OnProcess(FadeObj* self, float dt) {
    self->elapsed += dt;
    if (self->scriptedMessageIndex == 0 && !self->effect) return;
    void* manager = g_sharedDoc;
    manager = *(void**)&((CSharedDoc*)manager)->golfDoc;
    if ((((CGolfDoc*)manager)->gameStateFlags & kCancelLetterMask) != 0) self->state = kLetterHidden;

    // The word-copy view preserves the original paired loads; record access below
    // gives the same bytes their script-selection roles. Column 0 is unresolved.
    // Phase 1: copy 16 triplets {unconsumed word, fade-in script, fade-out script}.
    // Only columns 1/2 are consumed here. Column 0 is not interpreted by this routine.
    struct ScriptRecord { int unconsumed; void* fadeIn; void* fadeOut; };
    union { int words[48]; ScriptRecord records[kLetterScriptRecordCount]; } scripts;
    int i = 24;
    int* dst = scripts.words - 1;
    int* src = (int*)s_letterAnimationScriptTriplets - 1;
    do {
        int a = *++src;
        int b = *++src;
        *++dst = a;
        *++dst = b;
    } while (--i != 0);
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ItemWindow);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* bp = IdMap_Lookup(Sing_805DC5F0(), self->bpEntryId);
    // Phase 2: scripted messages transition on script completion; overlays use
    // opacity/time/input. The hold counter is in calls, independent of dt.
    switch (self->state) {
    case kLetterHidden:
        break;
    case kLetterFadeIn:
        if (self->scriptedMessageIndex) {
            BpObject_StartScriptByName(bp, (void*)scripts.words[self->scriptedMessageIndex * kLetterScriptWords + kFadeInScriptWord], 1);
            self->state = kLetterHolding;
        } else {
            float inv = kFloat_255_LetterFade / self->messageFadeDurationSeconds;
            self->opacity += inv * dt;
            if (self->opacity > kFloat_255_LetterFade) self->state = kLetterHolding;
        }
        break;
    case kLetterFadeOut:
        if (self->scriptedMessageIndex) {
            BpObject_StartScriptByName(bp, (void*)scripts.words[self->scriptedMessageIndex * kLetterScriptWords + kFadeOutScriptWord], 1);
            self->state = kLetterHidden;
        } else {
            float inv = kFloat_255_LetterFade / self->messageFadeDurationSeconds;
            self->opacity -= inv * dt;
            if (self->opacity < kFloat_0_Letter) { self->opacity = kFloat_0_Letter; self->state = kLetterHidden; }
        }
        break;
    case kLetterHolding:
        if (self->scriptedMessageIndex) {
            if (!BpObject_IsCurrentScriptComplete(bp)) break;
            // Compare before incrementing: fade-out begins on a later completed-script
            // update once the counter has reached the selected threshold.
            int threshold = EventTiming_IsDurationScalingEnabled() ? kScaledHoldThreshold : kDefaultHoldThreshold;
            if (self->completedScriptHoldUpdates < threshold) {
                self->completedScriptHoldUpdates++;
            } else {
                self->state = kLetterFadeOut;
                self->completedScriptHoldUpdates = 0;
            }
        } else {
            if (((Effect*)self->effect)->time > kFloat_0_Letter) {
                ((Effect*)self->effect)->time = ((Effect*)self->effect)->time - dt;
                if (((Effect*)self->effect)->time < kFloat_0_Letter) self->state = kLetterFadeOut;
            }
            if (((Effect*)self->effect)->dismissOnInput && CInputManager_HasAnyMonitoredAction(g_inputPointer)) self->state = kLetterFadeOut;
        }
        break;
    }
    // Phase 3: publish bounded opacity, including overshoot from a large dt.
    if (self->opacity < kFloat_0_Letter) self->opacity = kFloat_0_Letter;
    else if (self->opacity > kFloat_255_LetterFade) self->opacity = kFloat_255_LetterFade;
}
#pragma opt_strength_reduction reset

#pragma opt_propagation off
// Draw head icons, blinking prompts, the active letter, and its optional caption.
// Required self/view and loaded resources. Changes global blink phase/enabled
// bytes and +A8 when >1 (reset to .5, not zero); repeated draws therefore have
// side effects. +A8 is advanced by OnProcess's dt. Keep threshold/reset together.
// Draw callbacks run before messages; preserve their order and fresh Bp lookup.
void CLetter_OnDisplay(Obj* self) {
    if (g_headIcon) HeadIcon_DrawVisiblePlayerSlots(g_headIcon);
    if (self->messageDisplayEnabled) {
        if (self->replayBlinkTimer > kFloat_1_Letter) {
            unsigned char _old = *(volatile unsigned char*)&s_letterBlinkPhase;
            unsigned char _t = _old ^ 1;
            s_letterBlinkOverlayEnabled = 1;
            self->replayBlinkTimer = kFloat_0_5_Rival;
            *(volatile unsigned char*)&s_letterBlinkPhase = _t != 0;
        }
        if (s_letterBlinkPhase) {
            if (s_letterBlinkOverlayEnabled) { if (self->replayIcon) {
                LetterOverlayDrawPrefix* overlay = (LetterOverlayDrawPrefix*)self->replayIcon;
                WOverlay_DrawTransformedRect(overlay->resource, g_view, overlay->sourceUv, kFloat_0_Letter, (char*)overlay->destination, 0, -1, 0);
            } }
        } else if (s_letterBlinkOverlayEnabled) s_letterBlinkOverlayEnabled = 0;
    }
    if (self->fastForwardIcon) {
        if (self->replayIcon) {
            ((LetterOverlayDrawPrefix*)self->fastForwardIcon)->destination[0] = kFloat_88_Letter;
        } else {
            ((LetterOverlayDrawPrefix*)self->fastForwardIcon)->destination[0] = kFloat_13_Letter;
        }
        LetterOverlayDrawPrefix* overlay = (LetterOverlayDrawPrefix*)self->fastForwardIcon;
        WOverlay_DrawTransformedRect(overlay->resource, g_view, overlay->sourceUv, kFloat_0_Letter, (char*)overlay->destination, 0, -1, 0);
    }
    if (self->scriptedMessageIndex) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ItemWindow);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* p = IdMap_Lookup(Sing_805DC5F0(), self->bpEntryId);
        BpObject_TryInvokeCurrentCallback(p, 0);
        BpObject_Draw(p);
    }
    if (self->messageDisplayEnabled) {
        if (self->messageState) {
            if (self->currentMessage) {
                void* p = g_sharedDoc;
                p = *(void**)&((CSharedDoc*)p)->golfDoc;
                int gameStateFlags = ((CGolfDoc*)p)->gameStateFlags;
                if (((gameStateFlags & kSuppressLetterTextFlag20000) | (gameStateFlags & kSuppressLetterTextFlag400)) == 0) {
                    if (self->currentMessage) {
                    // Draw prefix and caption metadata borrow the selected registry Effect.
                    LetterOverlayDrawPrefix* overlay = (LetterOverlayDrawPrefix*)self->currentMessage;
                    int _color = (((int)(self->messageAlpha)) << 24) + 0x00FFFFFF;
                    WOverlay_DrawTransformedRect(overlay->resource, g_view, overlay->sourceUv, kFloat_0_Letter, (char*)overlay->destination, 0, _color, 0);
                    LetterCaptionOptions* options = (LetterCaptionOptions*)self->currentMessage;
                    if (options->showCaption) {
                        float f31 = ((WView*)g_view)->viewportHeight;
                        float f30 = ((WView*)g_view)->viewportWidth;
                        char* s = String_FormatTemporary(golfLetterOverlayStrings.captionFormat, options->captionValue);
                        CLetter_DrawText(self, s, kFloat_0_5_Rival * f30 + kFloat_5_LetterOverlay,
                            kFloat_0_25_LetterOverlay * f31 + kFloat_5_LetterOverlay,
                            kLetterCaptionExtraGlyphAdvance,
                            options->glyphHeightScale,
                            0,
                            options->packedColor);
                    }
                    }
                }
            }
        }
    }
}

#pragma opt_propagation reset
} // extern "C"
