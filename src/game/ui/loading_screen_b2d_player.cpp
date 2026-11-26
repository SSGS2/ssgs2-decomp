// Loading indicator: plays the Now Loading animation, choosing its BP files, scripts and nodes
// and pulsing the opacity of its sprites. Loads and releases the shared loading file and texture.
// Also holds the pending node-name and item-ID vectors and their exception cleanup.

#include <decomp/ct_member.h>
// Loading / Now Loading BP animation player (0x8028FE2C). This is a menu/HUD
// consumer of BP draw contexts: it chooses files, scripts and nodes, and pulses
// sprite opacity. CBpCtrl owns the resolved BP objects; shared loading file and
// texture lifetime are controlled by LoadingScreen_LoadB2dResources and
// LoadingScreen_ReleaseSharedResources.
// Mod lookup:
// - Widescreen/resolution: no width, aspect ratio, viewport or scissor is set in
//   this unit. LoadingScreenStringPool selects the BP/layout resources;
//   B2dPlayer_UpdateAndDraw calls BpObject_Draw in the CURRENT render context.
//   Edit the layout/its enclosing draw context for placement, keeping pointer and
//   HUD coordinate mapping together. Opacity and container sizes are not pixels.
// - Loading artwork: LoadingScreen_LoadB2dResources uses the pool's bpFile and
//   textureFile paths. PollResourceReadiness requires BOTH texture and file buffer.
//   CreatePlayer uses 13 s_loadingNodeNames, raw item IDs 1012/1013 from
//   kLoadingHiddenItemIdWords, and one s_loadingPulseNodeNames entry.
// - Pulse / frame rate: SetPendingPulseNodeNames stores low8(int(255/low8(divisor)));
//   low8(divisor)==0 disables it; increasing it in1..255 slows the quantized pulse.
//   Loading's divisor20 gives step12; UpdateAndDraw advances ONCE PER CALL, with
//   phase >255 subtracting510 and abs(phase) as byte opacity. It has no dt input.
//   Repeating draw calls therefore changes pulse speed and script/callback cadence.
// - Playback: ConfigurePlayback copies nullable script names and sets the state
//   map below. repeatCount becomes signed short(count-1), not a frame duration.
// Shared scalar/ID ownership definitions are in the data section. Packed pools
// and name tables retain original symbol identities/relocation addends.
// API: live required player except dtor (null accepted); owning handle arguments
// are writable void** slots, never raw players. No concurrent calls: callbacks,
// controller removals and vector iteration assume one menu thread. All name/ID
// inputs are borrowed only while copying and must not alias the destination.
// Packed strings: 12 native Wii bytes. High header bit means heap ownership;
// otherwise low7 bits of FIRST BYTE are inline length and bytes1..11 hold text
// including terminator. Heap length is word1 and pointer word2; lengths count bytes.
// Vector size/capacity count elements (12-byte strings or 4-byte words), not bytes.
// Growth follows ~3/5, then 1/2 increments with overflow ceilings. Staging guards
// preserve committed prefixes on exceptions; a failed setter is not an atomic
// rollback of every already-appended value. Allocation failure is not recovered.
// Compiler preservation: empty allocator scratch byte copies, explicit nested
// destruction and count-count expressions retain CodeWarrior's unwind/access
// shape. Do not initialize away or consolidate that scaffolding as a tuning edit.
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char loadingScreenStrings[];
}
}

#include "game/singletons.h"
#include "game/exception.h"
#include "classes/CB2dPlayerSimple.h"
#include <decomp/cw_bitfields.h>
#include "classes/CNowReloading.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CB2dPlayerHandle.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpTex.h"
// Byte views below retain the packed-string access width; original storage +8/+0x14/+0x20/+0x2C.

// Borrow the shared field schema. Retain a cast macro here: the low-optimization
// playback paths emit real calls for an inline cast function (verified on copies),
// changing code and stack/EH layout. This adapter performs no work or ownership change.
#define AsCB2dPlayerSimple(p) ((CB2dPlayerSimple*)(p))
// Fixed byte arrays preserve the original packed pool and relocation addends.
struct LoadingScreenStringPool {
    char bpFile[12];
    char initialScript[8];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char textureFile[12];
    char reserveLengthError[35];
    char stringOutOfRange[27];
    char outOfRangeType[55];
    char stringLengthError[27];
    unsigned char padding[1];
};
// The first byte of each 12-byte string packs heap ownership and inline length.
struct PackedStringStorage {
    unsigned int flags;
    unsigned int length;
    void* heapData;
};
struct InlineStringBytes {
    unsigned char flags;
    char characters[11];
};
// Container ownership: name vectors own packed strings and their heap buffers;
// pendingItemIds owns raw words; resolvedNodes owns only its pointer allocation
// (BP controller entries own the nodes). All vectors store data/count/capacity.
// Growth staging owns a replacement until swap, then owns/frees the old buffer.
// Byte-copy plus zeroing transfers packed-string ownership without double-free.
#define PLAYER_MEMBER_OFFSET(field) ((int)&((CB2dPlayerSimple*)0)->field)
#define PLAYER_STRING_BYTES(player, field) ((InlineStringBytes*)&AsCB2dPlayerSimple(player)->field)
// Named storage operations; macros deliberately keep each original load/expression
// at its use site in the low-optimization playback dispatcher. They evaluate the
// player argument once; pass a plain pointer, never a side-effecting expression.
#define PLAYER_SCRIPT_IS_HEAP(player, field) ((int)((unsigned int)AsCB2dPlayerSimple(player)->field >> 31))
#define PLAYER_SCRIPT_INLINE_LENGTH(player, field) (PLAYER_STRING_BYTES(player, field)->flags & kInlineStringLengthMask)
#define LOADING_STRING(base, member) ((base) + (int)&((LoadingScreenStringPool*)0)->member)
// Allocation ceilings and growth cutoffs for 12-byte strings and 4-byte words.
static const unsigned int kMaxStringVectorElements = 0x15555555U;
static const unsigned int kStringVectorSmallGrowthLimit = 0x071C71C7U;
static const unsigned int kStringVectorHalfGrowthLimit = 0x0E38E38EU;
static const unsigned int kMaxWordVectorElements = 0x3FFFFFFFU;
static const unsigned int kWordVectorSmallGrowthLimit = 0x15555555U;
static const unsigned int kWordVectorHalfGrowthLimit = 0x2AAAAAAAU;

enum PlayerState {
    kPlayerIdle = 0,
    kPlayerCreateEntry = 1,
    kPlayerWaitForLoad = 2,
    kPlayerPlaying = 3,
    kPlayerFinished = 4,
    kPlayerStateLimit = 5
};
// Update contract: idle on entry deletes the owned player; finished on entry
// becomes idle, draws once, then deletes on the next update. Create-entry can
// fall through load and playback in one call. Load pending stays in state 2.
// State 3 pulses and advances scripts: initial -> repeat -> requested. A negative
// repeat counter is unlimited; a queued request interrupts repeat at completion.
// Once repeats end, a missing request name still waits until pendingScript is
// set. A named request waits for that flag, plays once, then finishes because
// its name was consumed and the flag remains set. Out-of-range states only draw.
enum PlayerPulse {
    kOpacityFull = 255,
    kPulsePeriod = 510,
    kInlineStringLengthMask = 0x7F
};
enum LoadingPlayerDefaults {
    kPlayerAllocationBytes = 0x7C,
    kLoadingHiddenNodeCount = 13,
    kLoadingHiddenItemCount = 2,
    kLoadingPulseNodeCount = 1,
    kLoadingOpacityDivisor = 20
};
// The entry slot is used for name/ID lookup and as the controller removal key.
struct BpEntrySlotView {
    char reserved[4];
    void* slotKey;
};

struct PadObj78 {
    char pad[0x78];
};
struct DelObj78 : public PadObj78 {
    virtual void destroy(int);
};

// Texture virtual interface; preserve the slot order used by CBpTex.
struct CallbackObj {
    virtual void destroy(int) = 0;
    virtual void setTexturePath(const char*) = 0;
    virtual void m2(int) = 0;
    virtual void m3() = 0;
    virtual int isReady() = 0;
};

extern "C" {

extern int s_loadingScreenResourcesRequested;
extern int s_loadingScreenResourcesReady;
extern void* g_loadingScreenTexture;
extern void* g_loadingScreenSharedFile;
extern void* __vt__16CB2dPlayerSimple[];
extern char loadingScreenStrings[];

void BpSprite_SetEnabled(void* object, void* y);
void BpSprite_SetOpacity(void* object, void* y);
int abs(int x);
int CSharedFilePtr_GetLoadedBuffer(void* object);
void* SharedFile_Create(void* object, int y);
void* __nw__FUl(unsigned int size);
void* __nwa__FUl(int x);
void* appMallocDummy__FUl(int x);
void __dl__FPv(void* object);
void BpObject_StartScriptByName(void* object, const void* y, int z);
int BpObject_PollLoad(void* object);
int BpObject_IsCurrentScriptComplete(void* object);
void BpObject_TryInvokeCurrentCallback(void* object, int y);
void BpObject_Draw(void* object);
void* IdMap_Lookup(void* a, int b);
void CBpCtrl_dtor(void* object);
void __register_global_object(void* object, void* y, void* z);
void appFreeUni__FPv(void* object);
void __dla__FPv(void* object);
void String_ReserveCapacity(void* object, unsigned int y);
void* BpSlot_FindNodeByName(void* object, void* y);
void* BpSlot_FindItemById(void* object, void* y);
void String_ReplaceRange(void* object, int y, int z, const char* p, const char* q, void* r);
int strlen(const char* x);
char* strcpy(char* dst, const char* src);
void* memcpy(void* dst, const void* src, unsigned int n);
void* memset(void* dst, int v, unsigned int n);

// Legacy double declaration preserves an eight-byte symbol; payload is two raw
// 32-bit IDs (1012,1013), never a double or float conversion (see data companion).
extern double kLoadingHiddenItemIdWords;
extern const char* s_loadingNodeNames[];
extern int s_loadingPulseNodeNames;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_Menu8028FE2C[];

struct OpacityPulseView {
    char pad00[0x40];
    void** items;
    int count;
    char pad48[0x2C];
    int phase; // 0x74: triangle-wave opacity phase (the player's own playback state is a different member)
};

struct InitObj80290424 {
    char data[0x28];
    // Initialize the texture wrapper before setting its resource path.
    InitObj80290424() {
        ((CBpTex*)this)->__ct();
    }
};

// Report whether the loading indicator owns no animation player.
int CNowReloading::IsPlayerAbsent(void* object) {
    int v = *(int*)object;
    return v == 0;
}

// Mark the owned animation player's queued script for playback.
void CB2dPlayerHandle::RequestPendingScript(void* object) {
    int* p = *(int**)object;
    if (p != 0) {
        AsCB2dPlayerSimple(p)->pendingScript = 1;
    }
}

// Report true for no player or for a player in its active playback state.
int LoadingIndicator_IsAbsentOrState3(void* object) {
    int* p = *(int**)object;
    // 0x8028FF00: no player and an active player both satisfy this query.
    if (!(p == 0 || AsCB2dPlayerSimple(p)->state == kPlayerPlaying))
        return 0;
    return 1;
}

// Apply the requested enabled value to every resolved sprite.
void CB2dPlayerSimple::SetSpritesEnabled(void* enabledValue) {
    void** p = (*(void***)&resolvedNodes);
    while (p != (*(void***)&resolvedNodes) + resolvedNodeCount) {
        BpSprite_SetEnabled(*p, enabledValue);
        ++p;
    }
}

// Reset the triangle-wave phase and set all resolved sprites to full opacity.
void CB2dPlayerSimple::ResetOpacityPulse(void* object) {
    OpacityPulseView* obj = (OpacityPulseView*)object;
    obj->phase = kOpacityFull;
    void* value = (void*)abs(kOpacityFull);
    void** item = obj->items;
    while (item != obj->items + obj->count) {
        BpSprite_SetOpacity(*item, value);
        ++item;
    }
}

// Initialize empty script strings, node/id vectors, playback state, and opacity pulse step.
void CB2dPlayerSimple::Init() {
    (*(int*)&bpFileNameStorageFlags) = 0;
    vtable = __vt__16CB2dPlayerSimple;
    bpFileNameLength = 0;
    (*(int*)&bpFileNameHeap) = 0;
    (*(int*)&initialScriptStorageFlags) = 0;
    initialScriptLength = 0;
    (*(int*)&initialScriptHeap) = 0;
    (*(int*)&requestedScriptStorageFlags) = 0;
    requestedScriptLength = 0;
    (*(int*)&requestedScriptHeap) = 0;
    (*(int*)&repeatScriptStorageFlags) = 0;
    repeatScriptLength = 0;
    (*(int*)&repeatScriptHeap) = 0;
    (*(int*)&resolvedNodes) = 0;
    resolvedNodeCount = 0;
    resolvedNodeCapacity = 0;
    (*(int*)&pendingNodeNames) = 0;
    pendingNodeNameCount = 0;
    pendingNodeNameCapacity = 0;
    (*(int*)&pendingItemIds) = 0;
    pendingItemIdCount = 0;
    pendingItemIdCapacity = 0;
    (*(int*)&pendingPulseNodeNames) = 0;
    pendingPulseNodeNameCount = 0;
    pendingPulseNodeNameCapacity = 0;
    bpObject = 0;
    state = kPlayerIdle;
    pulseStep = 0;
}

// Replace the shared loading animation file and texture, then begin readiness polling.
void LoadingScreen_LoadB2dResources(void) {
    if (g_loadingScreenSharedFile != 0) {
        CSharedFileHandle::Destroy(g_loadingScreenSharedFile, 1);
        g_loadingScreenSharedFile = 0;
    }
    g_loadingScreenSharedFile = SharedFile_Create(loadingScreenStrings, 0);
    if (g_loadingScreenTexture != 0) {
        if (g_loadingScreenTexture != 0) {
            ((CallbackObj*)g_loadingScreenTexture)->destroy(1);
        }
        g_loadingScreenTexture = 0;
    }
    InitObj80290424* obj = new InitObj80290424;
    g_loadingScreenTexture = obj;
    ((CallbackObj*)obj)->setTexturePath(LOADING_STRING(loadingScreenStrings, textureFile));
    s_loadingScreenResourcesRequested = 1;
    s_loadingScreenResourcesReady = 0;
}

// Mark resources ready only once both the texture and shared loading file have loaded.
void LoadingScreen_PollResourceReadiness(void) {
    if (s_loadingScreenResourcesRequested != 0 && s_loadingScreenResourcesReady == 0) {
        s_loadingScreenResourcesReady = 1;
        if (((CallbackObj*)g_loadingScreenTexture)->isReady() == 0) {
            s_loadingScreenResourcesReady = 0;
        }
        if (CSharedFilePtr_GetLoadedBuffer(g_loadingScreenSharedFile) == 0) {
            s_loadingScreenResourcesReady = 0;
        }
    }
}

// Replace the loading animation player and queue its named nodes, item IDs, and pulse node.
// object is a writable owning player slot. Destroys its previous player first;
// allocation failure is not safely handled by the following configuration calls.
void LoadingIndicator_CreatePlayer(void* object) {
    void** slot = (void**)object;
    void* old = *slot;
    if (old != 0) {
        if (old != 0) {
            ((DelObj78*)old)->destroy(1);
        }
        *slot = 0;
    }

    void* obj = __nw__FUl(kPlayerAllocationBytes);
    if (obj != 0) {
        ((CB2dPlayerSimple*)obj)->Init();
    }
    *slot = obj;
    char* name = loadingScreenStrings;
    CB2dPlayerSimple::ConfigurePlayback(obj, name, name + (int)&((LoadingScreenStringPool*)0)->initialScript, 0, 0, 1);
    CB2dPlayerSimple::SetPendingNodeNames(*slot, s_loadingNodeNames, kLoadingHiddenNodeCount);
    CB2dPlayerSimple::SetPendingItemIds(*slot, (const float*)&kLoadingHiddenItemIdWords, kLoadingHiddenItemCount);
    CB2dPlayerSimple::SetPendingPulseNodeNames(*slot, (const char**)&s_loadingPulseNodeNames, kLoadingPulseNodeCount,
                                               kLoadingOpacityDivisor);
}

// Start the first available initial/repeat/requested script; a queued request is gated by pendingScript.
// Return 1 also when a requested name exists but waits for its flag; only no
// named script returns 0. Does not own or retain the BP entry separately.
int CB2dPlayerSimple::StartPendingScript(void* object) {
    struct InlineHeader {
        CW_BITS2(unsigned char flag : 1, unsigned char length : 7)
    };
    char* self = (char*)object;
    int inlineValue = ((unsigned int)__cntlzw((AsCB2dPlayerSimple(self)->initialScriptStorageFlags) >> 31)) >> 5;
    int value =
        inlineValue
            ? ((*(unsigned char*)&AsCB2dPlayerSimple(self)->initialScriptStorageFlags) & kInlineStringLengthMask)
            : AsCB2dPlayerSimple(self)->initialScriptLength;
    if (value != 0) {
        void* param = inlineValue ? (void*)PLAYER_STRING_BYTES(self, initialScriptStorageFlags)->characters
                                  : AsCB2dPlayerSimple(self)->initialScriptHeap;
        BpObject_StartScriptByName((*(void**)&AsCB2dPlayerSimple(self)->bpObject), param, 1);
        return 1;
    }

    inlineValue = ((unsigned int)__cntlzw((AsCB2dPlayerSimple(self)->repeatScriptStorageFlags) >> 31)) >> 5;
    value = inlineValue
                ? ((*(unsigned char*)&AsCB2dPlayerSimple(self)->repeatScriptStorageFlags) & kInlineStringLengthMask)
                : AsCB2dPlayerSimple(self)->repeatScriptLength;
    if (value != 0) {
        void* param = inlineValue ? (void*)PLAYER_STRING_BYTES(self, repeatScriptStorageFlags)->characters
                                  : AsCB2dPlayerSimple(self)->repeatScriptHeap;
        BpObject_StartScriptByName((*(void**)&AsCB2dPlayerSimple(self)->bpObject), param, 1);
        return 1;
    }

    inlineValue = ((unsigned int)__cntlzw((AsCB2dPlayerSimple(self)->requestedScriptStorageFlags) >> 31)) >> 5;
    value = inlineValue
                ? ((*(unsigned char*)&AsCB2dPlayerSimple(self)->requestedScriptStorageFlags) & kInlineStringLengthMask)
                : AsCB2dPlayerSimple(self)->requestedScriptLength;
    if (value != 0) {
        if (AsCB2dPlayerSimple(self)->pendingScript != 0) {
            void* param = inlineValue ? (void*)PLAYER_STRING_BYTES(self, requestedScriptStorageFlags)->characters
                                      : AsCB2dPlayerSimple(self)->requestedScriptHeap;
            BpObject_StartScriptByName((*(void**)&AsCB2dPlayerSimple(self)->bpObject), param, 1);
            if ((AsCB2dPlayerSimple(self)->requestedScriptStorageFlags >> 31) == 0) {
                PLAYER_STRING_BYTES(self, requestedScriptStorageFlags)->characters[0] = 0;
                ((InlineHeader*)(&AsCB2dPlayerSimple(self)->requestedScriptStorageFlags))->length = 0;
            } else {
                *(*(unsigned char**)&AsCB2dPlayerSimple(self)->requestedScriptHeap) = 0;
                (*(void**)&AsCB2dPlayerSimple(self)->requestedScriptLength) = 0;
            }
        }
        return 1;
    }
    return 0;
}

struct InlineStringHeaderView {
    CW_BITS2(unsigned char flag : 1, unsigned char length : 7)
};

// Remove the BP entry, clear script strings, and discard queued node names and item IDs.
void CB2dPlayerSimple::UnloadAndClearPendingState(void* object) {
    // Phase 1: remove the controller entry before dropping borrowed node pointers.
    char* self = (char*)object;
    if ((*(void**)&AsCB2dPlayerSimple(self)->bpObject) != 0) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_Menu8028FE2C);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))
            ->DeleteEntryById((int)((BpEntrySlotView*)(*(void**)&AsCB2dPlayerSimple(self)->bpObject))->slotKey);
        (*(void**)&AsCB2dPlayerSimple(self)->bpObject) = 0;
    }

    // Phase 2: empty all four script strings, preserving any heap allocations.
    if ((AsCB2dPlayerSimple(self)->bpFileNameStorageFlags >> 31) == 0) {
        PLAYER_STRING_BYTES(self, bpFileNameStorageFlags)->characters[0] = 0;
        ((InlineStringHeaderView*)((((&AsCB2dPlayerSimple(self)->bpFileNameStorageFlags)))))->length = 0;
    } else {
        *(*(unsigned char**)&AsCB2dPlayerSimple(self)->bpFileNameHeap) = 0;
        (*(void**)&AsCB2dPlayerSimple(self)->bpFileNameLength) = 0;
    }
    if ((AsCB2dPlayerSimple(self)->initialScriptStorageFlags >> 31) == 0) {
        PLAYER_STRING_BYTES(self, initialScriptStorageFlags)->characters[0] = 0;
        ((InlineStringHeaderView*)(&AsCB2dPlayerSimple(self)->initialScriptStorageFlags))->length = 0;
    } else {
        *(*(unsigned char**)&AsCB2dPlayerSimple(self)->initialScriptHeap) = 0;
        (*(void**)&AsCB2dPlayerSimple(self)->initialScriptLength) = 0;
    }
    if ((AsCB2dPlayerSimple(self)->requestedScriptStorageFlags >> 31) == 0) {
        PLAYER_STRING_BYTES(self, requestedScriptStorageFlags)->characters[0] = 0;
        ((InlineStringHeaderView*)(&AsCB2dPlayerSimple(self)->requestedScriptStorageFlags))->length = 0;
    } else {
        *(*(unsigned char**)&AsCB2dPlayerSimple(self)->requestedScriptHeap) = 0;
        (*(void**)&AsCB2dPlayerSimple(self)->requestedScriptLength) = 0;
    }
    if ((AsCB2dPlayerSimple(self)->repeatScriptStorageFlags >> 31) == 0) {
        PLAYER_STRING_BYTES(self, repeatScriptStorageFlags)->characters[0] = 0;
        ((InlineStringHeaderView*)(&AsCB2dPlayerSimple(self)->repeatScriptStorageFlags))->length = 0;
    } else {
        *(*(unsigned char**)&AsCB2dPlayerSimple(self)->repeatScriptHeap) = 0;
        (*(void**)&AsCB2dPlayerSimple(self)->repeatScriptLength) = 0;
    }

    // Phase 3: destroy queued name strings; clear word/pointer counts without
    // destroying the external item IDs or BP nodes they denote.
    char* ptr;
    unsigned int count = AsCB2dPlayerSimple(self)->pendingNodeNameCount;
    ptr = (*(char**)&AsCB2dPlayerSimple(self)->pendingNodeNames) + count * 0xC;
    AsCB2dPlayerSimple(self)->pendingNodeNameCount = count - AsCB2dPlayerSimple(self)->pendingNodeNameCount;
    while (count) {
        ptr -= 0xC;
        if (ptr) {
            if ((((PackedStringStorage*)ptr)->flags >> 31) != 0) {
                appFreeUni__FPv(((PackedStringStorage*)ptr)->heapData);
            }
        }
        --count;
    }

    char* ptr2;

    unsigned int count2 = AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount;
    unsigned int oldCount = AsCB2dPlayerSimple(self)->pendingItemIdCount;
    unsigned int clearedCount2 = count2 - AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount;
    AsCB2dPlayerSimple(self)->pendingItemIdCount -= oldCount;
    ptr2 = (*(char**)&AsCB2dPlayerSimple(self)->pendingPulseNodeNames) + count2 * 0xC;
    AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount = clearedCount2;
    while (count2 != 0) {
        ptr2 -= 0xC;
        if (ptr2 != 0) {
            if ((((PackedStringStorage*)ptr2)->flags >> 31) != 0) {
                appFreeUni__FPv(*(void**)(8 + ptr2));
            }
        }
        count2--;
    }

    unsigned int finalValue = (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCount);
    AsCB2dPlayerSimple(self)->resolvedNodeCount -= finalValue;
}

// Clear prior playback and queue new file/script names, repeat count, and BP-entry creation.
// bpFileName must be a valid C string. initialScript may be null; the two *Address
// arguments are nullable C-string addresses carried as 32-bit ints, not IDs.
// Strings are copied, never retained; repeatCount is stored as short(count-1).
// This clears the previous queues but keeps their allocations for reuse.
// Repeat input contract (counts are PLAYS OF repeatScript, excluding initial):
// new callers should use 0 for indefinite or 1..32768 for finite playback. The
// stock LoadingIndicator_CreatePlayer passes 1 and no repeat name; it therefore
// waits for a pending request after its initial script, rather than self-closing.
// The subtraction requires repeatCount != INT_MIN (-2147483648). This Wii
// compiler narrows the result to signed low16 bits; no validation/clamp occurs:
// input                  stored remaining     repeat plays before request wait
// 1..32768               0..32767             input (first play is uncounted)
// 0                      -1                   indefinite until pending request
// -32767..-1             -32768..-2            same indefinite behavior
// -32768                 32767                32768 (wrap back to finite)
// 32769..65536           -32768..-1            indefinite (wrap)
// 65537                  0                    1 (wrap)
// Other arithmetic-safe ints follow signed16(input-1); this is not a wider
// supported play-count range. These counts matter only with a repeat name.
// Positive remaining values decrement on RESTART, not on the first play.
// A pending request can stop finite or indefinite repetition at a completion
// boundary; it never cuts the current script mid-play. See chooser table in
// B2dPlayer_UpdateAndDraw for the initial-to-first-repeat priority exception.
void CB2dPlayerSimple::ConfigurePlayback(void* object, const char* bpFileName, const char* initialScript,
                                         int requestedScriptAddress, int repeatScriptAddress, int repeatCount) {
    // Phase 1: unregister the old BP object, then clear strings and queued nodes.
    char* self = (char*)object;
    if ((*(void**)&AsCB2dPlayerSimple(self)->bpObject) != 0) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_Menu8028FE2C);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))
            ->DeleteEntryById((int)((BpEntrySlotView*)(*(void**)&AsCB2dPlayerSimple(self)->bpObject))->slotKey);
        (*(void**)&AsCB2dPlayerSimple(self)->bpObject) = 0;
    }

    if ((AsCB2dPlayerSimple(self)->bpFileNameStorageFlags >> 31) == 0) {
        PLAYER_STRING_BYTES(self, bpFileNameStorageFlags)->characters[0] = 0;
        (*(unsigned char*)&AsCB2dPlayerSimple(self)->bpFileNameStorageFlags) =
            (unsigned char)((*(unsigned char*)&AsCB2dPlayerSimple(self)->bpFileNameStorageFlags) &
                            ~kInlineStringLengthMask);
    } else {
        *(*(unsigned char**)&AsCB2dPlayerSimple(self)->bpFileNameHeap) = 0;
        (*(void**)&AsCB2dPlayerSimple(self)->bpFileNameLength) = 0;
    }
    if ((AsCB2dPlayerSimple(self)->initialScriptStorageFlags >> 31) == 0) {
        PLAYER_STRING_BYTES(self, initialScriptStorageFlags)->characters[0] = 0;
        (*(unsigned char*)&AsCB2dPlayerSimple(self)->initialScriptStorageFlags) =
            (unsigned char)((*(unsigned char*)&AsCB2dPlayerSimple(self)->initialScriptStorageFlags) &
                            ~kInlineStringLengthMask);
    } else {
        *(*(unsigned char**)&AsCB2dPlayerSimple(self)->initialScriptHeap) = 0;
        (*(void**)&AsCB2dPlayerSimple(self)->initialScriptLength) = 0;
    }
    if ((AsCB2dPlayerSimple(self)->requestedScriptStorageFlags >> 31) == 0) {
        PLAYER_STRING_BYTES(self, requestedScriptStorageFlags)->characters[0] = 0;
        (*(unsigned char*)&AsCB2dPlayerSimple(self)->requestedScriptStorageFlags) =
            (unsigned char)((*(unsigned char*)&AsCB2dPlayerSimple(self)->requestedScriptStorageFlags) &
                            ~kInlineStringLengthMask);
    } else {
        *(*(unsigned char**)&AsCB2dPlayerSimple(self)->requestedScriptHeap) = 0;
        (*(void**)&AsCB2dPlayerSimple(self)->requestedScriptLength) = 0;
    }
    if ((AsCB2dPlayerSimple(self)->repeatScriptStorageFlags >> 31) == 0) {
        PLAYER_STRING_BYTES(self, repeatScriptStorageFlags)->characters[0] = 0;
        (*(unsigned char*)&AsCB2dPlayerSimple(self)->repeatScriptStorageFlags) =
            (unsigned char)((*(unsigned char*)&AsCB2dPlayerSimple(self)->repeatScriptStorageFlags) &
                            ~kInlineStringLengthMask);
    } else {
        *(*(unsigned char**)&AsCB2dPlayerSimple(self)->repeatScriptHeap) = 0;
        (*(void**)&AsCB2dPlayerSimple(self)->repeatScriptLength) = 0;
    }

    int count = AsCB2dPlayerSimple(self)->pendingNodeNameCount;
    char* ptr = (*(char**)&AsCB2dPlayerSimple(self)->pendingNodeNames) + count * 0xC;
    AsCB2dPlayerSimple(self)->pendingNodeNameCount = count - count;
    while (count != 0) {
        ptr -= 0xC;
        if (ptr != 0 && (((PackedStringStorage*)ptr)->flags >> 31) != 0) {
            appFreeUni__FPv(((PackedStringStorage*)ptr)->heapData);
        }
        --count;
    }

    int count2 = AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount;
    char* ptr2 = (*(char**)&AsCB2dPlayerSimple(self)->pendingPulseNodeNames) + count2 * 0xC;
    AsCB2dPlayerSimple(self)->pendingItemIdCount = 0;
    AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount = 0;
    while (count2 != 0) {
        ptr2 -= 0xC;
        if (ptr2 != 0 && (((PackedStringStorage*)ptr2)->flags >> 31) != 0) {
            appFreeUni__FPv(((PackedStringStorage*)ptr2)->heapData);
        }
        --count2;
    }
    AsCB2dPlayerSimple(self)->resolvedNodeCount =
        AsCB2dPlayerSimple(self)->resolvedNodeCount - AsCB2dPlayerSimple(self)->resolvedNodeCount;

    // Phase 2: copy each supplied name into the player's packed string storage.
    int fileNameLength;
    if ((AsCB2dPlayerSimple(self)->bpFileNameStorageFlags >> 31) == 0) {
        fileNameLength = (*(unsigned char*)&AsCB2dPlayerSimple(self)->bpFileNameStorageFlags) & kInlineStringLengthMask;
    } else {
        fileNameLength = AsCB2dPlayerSimple(self)->bpFileNameLength;
    }

    char buf0;
    buf0 = *(char*)((char*)&buf0 + 4);
    *(char*)((char*)&buf0) = buf0;
    String_ReplaceRange(self + PLAYER_MEMBER_OFFSET(bpFileNameStorageFlags), 0, fileNameLength, bpFileName,
                        (const char*)((const char*)bpFileName + strlen(bpFileName)), &buf0);

    if (initialScript != 0) {
        int initialScriptLength;
        if ((AsCB2dPlayerSimple(self)->initialScriptStorageFlags >> 31) == 0) {
            initialScriptLength =
                (*(unsigned char*)&AsCB2dPlayerSimple(self)->initialScriptStorageFlags) & kInlineStringLengthMask;
        } else {
            initialScriptLength = AsCB2dPlayerSimple(self)->initialScriptLength;
        }
        char buf1;
        buf1 = *(char*)((char*)&buf1 + 4);
        *(char*)((char*)&buf1) = buf1;
        String_ReplaceRange(self + PLAYER_MEMBER_OFFSET(initialScriptStorageFlags), 0, initialScriptLength,
                            initialScript, (const char*)((const char*)initialScript + strlen(initialScript)), &buf1);
    }
    if (requestedScriptAddress != 0) {
        int requestedScriptLength;
        if ((AsCB2dPlayerSimple(self)->requestedScriptStorageFlags >> 31) == 0) {
            requestedScriptLength =
                (*(unsigned char*)&AsCB2dPlayerSimple(self)->requestedScriptStorageFlags) & kInlineStringLengthMask;
        } else {
            requestedScriptLength = AsCB2dPlayerSimple(self)->requestedScriptLength;
        }
        char buf2;
        buf2 = *(char*)((char*)&buf2 + 4);
        *(char*)((char*)&buf2) = buf2;
        String_ReplaceRange(
            self + PLAYER_MEMBER_OFFSET(requestedScriptStorageFlags), 0, requestedScriptLength,
            (const char*)requestedScriptAddress,
            (const char*)((const char*)requestedScriptAddress + strlen((const char*)requestedScriptAddress)), &buf2);
    }
    if (repeatScriptAddress != 0) {
        int repeatScriptLength;
        if ((AsCB2dPlayerSimple(self)->repeatScriptStorageFlags >> 31) == 0) {
            repeatScriptLength =
                (*(unsigned char*)&AsCB2dPlayerSimple(self)->repeatScriptStorageFlags) & kInlineStringLengthMask;
        } else {
            repeatScriptLength = AsCB2dPlayerSimple(self)->repeatScriptLength;
        }
        char buf3;
        buf3 = *(char*)((char*)&buf3 + 4);
        *(char*)((char*)&buf3) = buf3;
        String_ReplaceRange(self + PLAYER_MEMBER_OFFSET(repeatScriptStorageFlags), 0, repeatScriptLength,
                            (const char*)repeatScriptAddress,
                            (const char*)((const char*)repeatScriptAddress + strlen((const char*)repeatScriptAddress)),
                            &buf3);
    }

    // A negative remaining count repeats indefinitely; a positive count decrements after each play.
    short repeatsAfterFirst = (short)(repeatCount - 1);
    AsCB2dPlayerSimple(self)->scriptRepeatsRemaining = repeatsAfterFirst;
    AsCB2dPlayerSimple(self)->pendingScript = 0;
    AsCB2dPlayerSimple(self)->state = kPlayerCreateEntry;
}

// Destroy the loading file and texture and clear both resource status flags.
void LoadingScreen_ReleaseSharedResources(void) {
    if (s_loadingScreenResourcesRequested != 0) {
        if (g_loadingScreenSharedFile != 0) {
            CSharedFileHandle::Destroy(g_loadingScreenSharedFile, 1);
            g_loadingScreenSharedFile = 0;
        }
        if (g_loadingScreenTexture != 0) {
            if (g_loadingScreenTexture != 0) {
                ((CallbackObj*)g_loadingScreenTexture)->destroy(1);
            }
            g_loadingScreenTexture = 0;
        }
        s_loadingScreenResourcesRequested = 0;
        s_loadingScreenResourcesReady = 0;
    }
}

// Subtract a destroyed element count without changing the original reference-width operation.
static inline void DtorShrink(unsigned int* p, unsigned int n) {
    *p = *p - n;
}

struct DtorStr {
    unsigned int flags;
    unsigned int length;
    char* data;

    // Clear logical contents while preserving the existing allocation for reuse.
    void clear() {
        if ((flags >> 31) == 0) {
            ((char*)this)[1] = 0;
            ((char*)this)[0] &= ~kInlineStringLengthMask;
        } else {
            *data = 0;
            length = 0;
        }
    }
    // Free the string buffer only when the packed header marks heap ownership.
    ~DtorStr() {
        if ((flags >> 31) != 0) {
            appFreeUni__FPv(data);
        }
    }
};

struct DtorStrVec {
    DtorStr* begin;
    unsigned int size;
    unsigned int cap;

    // Clear logical contents while preserving the existing allocation for reuse.
    void clear() {
        DtorStr* p;
        unsigned int n = size;
        p = begin + n;
        size = size - n;
        while (n != 0) {
            --p;
            p->~DtorStr();
            --n;
        }
    }
    // Destroy owned strings in reverse order, then free their vector allocation.
    ~DtorStrVec() {
        if (begin != 0) {
            unsigned int n = size;
            DtorStr* p = begin + n;
            size = size - n;
            while (n != 0) {
                --p;
                p->~DtorStr();
                --n;
            }
            appFreeUni__FPv(begin);
        }
    }
};

struct DtorVec {
    char* begin;
    unsigned int size;
    unsigned int cap;

    // Clear logical contents while preserving the existing allocation for reuse.
    void clear() {
        char* e = begin + size;
        size -= e - begin;
    }
    // Clear logical size and free the vector allocation (elements are non-owning words).
    ~DtorVec() {
        if (begin != 0) {
            clear();
            appFreeUni__FPv(begin);
        }
    }
};

template <class T> struct DtorWrap {
    T t;
    // Trigger implicit member destruction at this level of the nested unwind wrapper.
    ~DtorWrap() {}
};

struct DtorPlayer {
    char** bpEntry;
    unsigned int state;
    DtorStr bpFileName;
    DtorStr initialScript;
    DtorStr requestedScript;
    DtorStr repeatScript;
    char pad38[8];
    DtorVec resolvedNodes;
    DtorStrVec pendingNodeNames;
    DtorVec pendingItemIds;
    DtorStrVec pendingPulseNodeNames;
    char pad70[8];
    void* vtable;
};

// Unregister the BP entry, clear all owned strings/vectors, and optionally free the player.
// flag > 0 frees self, so the returned original pointer may be dangling; null is safe.
CB2dPlayerSimple::~CB2dPlayerSimple() {
    DtorPlayer* self = (DtorPlayer*)this;
    self->vtable = __vt__16CB2dPlayerSimple;
    if (self->bpEntry != 0) {
        if (((int&)Sing_805DC5F0()) == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_Menu8028FE2C);
                s_bpCtrlInitialized = 1;
            }
            ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
        }
        ((CBpCtrl*)(((int&)Sing_805DC5F0())))->DeleteEntryById((int)((BpEntrySlotView*)self->bpEntry)->slotKey);
        self->bpEntry = 0;
    }
    // Phase 2: clear logical contents, then run nested unwind-wrapper
    // destructors to release each allocation in reverse member order.
    self->bpFileName.clear();
    self->initialScript.clear();
    self->requestedScript.clear();
    self->repeatScript.clear();
    self->pendingNodeNames.clear();
    self->pendingItemIds.clear();
    self->pendingPulseNodeNames.clear();
    self->resolvedNodes.clear();
    ((DtorWrap<DtorWrap<DtorWrap<DtorStrVec> > >*)&self->pendingPulseNodeNames)->~DtorWrap();
    ((DtorWrap<DtorWrap<DtorWrap<DtorWrap<DtorVec> > > >*)&self->pendingItemIds)->~DtorWrap();
    ((DtorWrap<DtorWrap<DtorWrap<DtorStrVec> > >*)&self->pendingNodeNames)->~DtorWrap();
    ((DtorWrap<DtorWrap<DtorWrap<DtorWrap<DtorVec> > > >*)&self->resolvedNodes)->~DtorWrap();
    self->repeatScript.~DtorStr();
    self->requestedScript.~DtorStr();
    self->initialScript.~DtorStr();
    self->bpFileName.~DtorStr();
}

extern void* __vt__Q23std11logic_error[];
extern void* s_sharedCharDeleterVtable[];
extern void* __vt__Q23std12length_error[];
extern void __dt__Q23std12length_errorFv(void*);
extern void __throw(void*, void*, void*);

struct QueuedNodeName {
    unsigned int flags;
    unsigned int length;
    char* data;

    // Free this queued name only when it uses heap string storage.
    ~QueuedNodeName() {
        if ((flags >> 31) != 0) {
            appFreeUni__FPv(data);
        }
    }
};

struct QueuedNodeNameVector {
    QueuedNodeName* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* sourceCapacity;
    unsigned int offset;

    // Start a guarded replacement vector for ordinary queued node names.
    QueuedNodeNameVector(unsigned int* p) : begin(0), size(0), capacity(0), sourceCapacity(p), offset(0) {}

    // Destroy staged name strings and release their allocation on unwind or after swap.
    ~QueuedNodeNameVector() {
        if (begin != 0) {
            unsigned int count = size;
            QueuedNodeName* item = begin + count;
            size -= count;
            while (count != 0) {
                --item;
                if (item != 0 && (item->flags >> 31) != 0) {
                    appFreeUni__FPv(item->data);
                }
                --count;
            }
            appFreeUni__FPv(begin);
        }
    }
};

struct LengthErrorObject80290FFC {
    void* vtable;
    char pad08[8];
    char field10;
    char pad11[3];
    char field14;
    char pad15[3];
};

struct ErrorCleanup91 {
    char* data;
    char* flag;
    // Guard a diagnostic allocation until its control block is constructed.
    ErrorCleanup91(char* p, char* f) : data(p), flag(f) {}
    // Free diagnostic text when control-block construction did not take ownership.
    ~ErrorCleanup91() {
        if (data)
            __dla__FPv(data);
    }
};
// Empty allocator object: only its zeroed state byte is observable (it shapes the original unwind tables).
struct EmptyAllocator {
    volatile char value;
    char pad[3];
    // Initialize the observable allocator-state byte used by the original unwind code.
    EmptyAllocator() : value(0) {}
};
struct ErrorOwner91 {
    void* vtable;
    int refs;
    int owners;
    char* data;
    // Own copied diagnostic text with one strong and one weak reference.
    ErrorOwner91(char* p) : refs(1), owners(1) {
        vtable = s_sharedCharDeleterVtable;
        data = p;
    }
};
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct ErrorBase91 {
    void* vtable;
    char* data;
    ErrorOwner91* owner;
    // Invoke the exception base destructor without deallocating this local object.
    ~ErrorBase91() {
        __dt__Q23std9exceptionFv(this, 0);
    }
    // Copy diagnostic text and attach it to a guarded reference-counted owner.
    ErrorBase91(const char* text) {
        vtable = __vt__Q23std11logic_error;
        struct {
            char value;
            char pad[3];
        } flag;
        flag.value = 0;
        int n = strlen(text) + 1;
        char* copy = (char*)__nwa__FUl((n << 1) - n);
        data = copy;
        {
            ErrorCleanup91 cleanup(copy, &flag.value);
            owner = new ErrorOwner91(copy);
            cleanup.data = 0;
        }
        strcpy(data, text);
    }
    // Copy diagnostic text and attach it to a guarded reference-counted owner.
    ErrorBase91(const char* text, int) {
        EmptyAllocator allocator;
        vtable = __vt__Q23std11logic_error;
        struct {
            char value;
            char pad[3];
        } flag;
        flag.value = 0;
        int n = strlen(text) + 1;
        char* copy = (char*)__nwa__FUl((n << 1) - n);
        data = copy;
        {
            ErrorCleanup91 cleanup(copy, &flag.value);
            owner = new ErrorOwner91(copy);
            cleanup.data = 0;
        }
        strcpy(data, text);
    }
};
}
namespace std {
struct length_error : ErrorBase91 {
    // Build the matching length-error ABI variant and install its vtable.
    length_error(const char* text) : ErrorBase91(text) {
        vtable = __vt__Q23std12length_error;
    }
    // Build the matching length-error ABI variant and install its vtable.
    length_error(const char* text, int flag) : ErrorBase91(text, flag) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {

struct PendingNameString {
    unsigned int flags;
    unsigned int length;
    char* data;

    // Free this pulse-node name only when it uses heap string storage.
    ~PendingNameString() {
        if ((flags >> 31) != 0) {
            appFreeUni__FPv(data);
        }
    }
};

struct QueuedPulseNameVector {
    PendingNameString* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* sourceCapacity;
    unsigned int offset;

    // Start a guarded replacement vector for queued pulse-node names.
    QueuedPulseNameVector(unsigned int* p) : begin(0), size(0), capacity(0), sourceCapacity(p), offset(0) {}

    // Destroy staged pulse names and free their vector allocation.
    ~QueuedPulseNameVector() {
        if (begin != 0) {
            unsigned int count = size;
            PendingNameString* item = begin + count;
            EmptyAllocator clearAllocator;
            size -= count;
            while (count != 0) {
                --item;
                if (item != 0 && (item->flags >> 31) != 0) {
                    appFreeUni__FPv(item->data);
                }
                --count;
            }
            appFreeUni__FPv(begin);
        }
    }
};

// Copy inline words in store order; source data remains a reference until its final load.
static inline void CopyQueuedInlineWords(unsigned int& destinationFlags, unsigned int& destinationLength,
                                         char*& destinationData, unsigned int flags, unsigned int length,
                                         char* const& sourceData) {
    destinationFlags = flags;
    destinationLength = length;
    destinationData = sourceData;
}

} // extern "C"
// Destroy queued string contents backwards; caller already removed them from logical size.
// Keep the cursor/count in the caller frame so the two vector ABI variants retain their layout.
template <class NameString>
static inline void DestroyPendingNameContents(NameString*& cursor, unsigned int& remaining) {
    while (remaining != 0) {
        --cursor;
        if (cursor != 0 && (cursor->flags >> 31) != 0) {
            appFreeUni__FPv(cursor->data);
        }
        --remaining;
    }
}
extern "C" {

// Initialize an empty destination and reserve for a heap-name copy; caller owns unwind cleanup.
static inline void ReserveQueuedHeapName(void* destination, unsigned int& flags, unsigned int& length, char*& data,
                                         const unsigned int& sourceLength) {
    flags = 0;
    length = 0;
    data = 0;
    String_ReserveCapacity(destination, sourceLength);
}

// Setter comparison guide:
// Both name setters execute CLEAR -> OWN TEXT -> APPEND/GROW -> COMMIT -> RETIRE.
// Their string/guard types preserve separate ABI instantiations. The sole policy
// difference is the pulse setter's final divisor-to-step calculation; queued text
// ownership and capacity growth are the same. Alter queue policy in both setters.
// Queue-copy policy shared by the two name setters below:
//   inline source -> copy its three words (characters are contained in them);
//   heap source   -> reserve independent storage and copy characters;
//   vector growth -> move existing words and ZERO their old ownership tags.
// Keep allocator scratch and try scopes at each call site: extracting the string
// constructor or capacity policy changes MWCC's stack/EH layout. The reserve,
// move, commit and retire boundaries below delimit the storage machinery from
// the setter policy (which queue to fill and, for pulse names, which step to store).
// Construct and throw the name-vector exception using caller-owned aligned storage.
inline void ThrowPendingNameLengthError(std::length_error* error) {
    error->__ct(LOADING_STRING(loadingScreenStrings, vectorLengthError), 0);
    exception_runtime::__throw(LOADING_STRING(exception_runtime::loadingScreenStrings, lengthErrorType), error,
                               (void*)exception_runtime::__dt__Q23std12length_errorFv);
}
// Storage must remain at the call site to preserve original stack/unwind layout.
#define THROW_PENDING_NAME_LENGTH_ERROR()                             \
    do {                                                              \
        union {                                                       \
            double alignment;                                         \
            unsigned char bytes[sizeof(std::length_error)];           \
        } storage;                                                    \
        std::length_error* error = (std::length_error*)storage.bytes; \
        ThrowPendingNameLengthError(error);                           \
    } while (0)

// Copy the pulse-node name array into the pending queue and set the opacity increment.
// nodeCount and opacityDivisor are truncated to their LOW EIGHT BITS. Supply 0..255
// names, each a valid borrowed C string; copies become player-owned. The caller's
// array must not alias the old queue. The stored opacity increment is
// low8(int(255.0 / low8(opacityDivisor))); a zero divisor disables pulsing.
void CB2dPlayerSimple::SetPendingPulseNodeNames(void* object, const char** nodeNames, int nodeCount,
                                                int opacityDivisor) {
    // Phase 1: destroy old queued names while keeping vector capacity.
    char* self = (char*)object;
    EmptyAllocator clearAllocator;
    unsigned int oldCount = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount);
    PendingNameString* oldEnd = (PendingNameString*)AsCB2dPlayerSimple(self)->pendingPulseNodeNames + oldCount;
    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount) -= oldCount;
    DestroyPendingNameContents(oldEnd, oldCount);

    // Phase 2: copy each name, append in place when possible, otherwise reserve
    // a guarded replacement, construct the suffix, move/zero the old prefix,
    // swap ownership and release old storage. Each append commits separately.
    unsigned char nameIndex = 0;
    if ((unsigned char)nodeCount != 0) {
        while (nameIndex < (unsigned char)nodeCount) {
            const char* text = nodeNames[nameIndex];
            char allocatorScratch[8];
            {
                // OWN TEXT: temporary owns a copy until its destructor runs after append.
                PendingNameString item;
                item.flags = 0;
                item.length = 0;
                item.data = 0;
                unsigned int length = strlen(text);
                String_ReserveCapacity(&item, length);
                allocatorScratch[0] = allocatorScratch[4];
                String_ReplaceRange(&item, 0, 0, text, text + length, allocatorScratch);

                unsigned int count = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount);
                unsigned int capacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCapacity);
                EmptyAllocator appendAllocator;
                // Append with spare capacity: copy one value, then publish its count.
                if (count < capacity) {
                    PendingNameString* slot =
                        (PendingNameString*)AsCB2dPlayerSimple(self)->pendingPulseNodeNames + count;
                    if (slot != 0) {
                        try {
                            unsigned int itemFlags = item.flags;
                            if ((itemFlags >> 31) == 0) {
                                unsigned int itemLength = item.length;
                                CopyQueuedInlineWords(slot->flags, slot->length, slot->data, itemFlags, itemLength,
                                                      item.data);
                            } else {
                                ReserveQueuedHeapName(slot, slot->flags, slot->length, slot->data, item.length);
                                char appendStringAllocator[12];
                                appendStringAllocator[0] = appendStringAllocator[4];
                                String_ReplaceRange(slot, 0, 0, item.data, item.data + item.length,
                                                    appendStringAllocator);
                            }
                        } catch (...) {
                            throw;
                        }
                    }
                    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount) =
                        (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount) + 1;
                } else {
                    EmptyAllocator growAllocator;
                    EmptyAllocator rangeAllocator;
                    unsigned int capacityForError = capacity;
                    if (kMaxStringVectorElements - capacityForError < 1) {
                        THROW_PENDING_NAME_LENGTH_ERROR();
                    }

                    // Reserve: until swap, staging owns every newly constructed name.
                    unsigned int* capacityAddress =
                        (unsigned int*)(self + PLAYER_MEMBER_OFFSET(pendingPulseNodeNameCapacity));
                    unsigned int required =
                        (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount) + 1 - *capacityAddress;
                    QueuedPulseNameVector staging(capacityAddress);
                    unsigned int growthCapacity = *capacityAddress;
                    if (required > kMaxStringVectorElements - growthCapacity) {
                        THROW_PENDING_NAME_LENGTH_ERROR();
                    }

                    unsigned int newCapacity;
                    if (growthCapacity < kStringVectorSmallGrowthLimit) {
                        unsigned int growthThreeFifths = ((growthCapacity + 1) * 3) / 5;
                        unsigned int* growthChoice;
                        if (growthThreeFifths < required) {
                            growthChoice = &required;
                        } else {
                            growthChoice = &growthThreeFifths;
                        }
                        newCapacity = growthCapacity + *growthChoice;
                    } else if (growthCapacity < kStringVectorHalfGrowthLimit) {
                        unsigned int growthHalf = (growthCapacity + 1) >> 1;
                        unsigned int* growthChoice;
                        if (growthHalf < required) {
                            growthChoice = &required;
                        } else {
                            growthChoice = &growthHalf;
                        }
                        newCapacity = growthCapacity + *growthChoice;
                    } else {
                        newCapacity = kMaxStringVectorElements;
                    }
                    EmptyAllocator allocationAllocator;
                    if (newCapacity > kMaxStringVectorElements) {
                        THROW_PENDING_NAME_LENGTH_ERROR();
                    }

                    staging.begin = (PendingNameString*)appMallocDummy__FUl(newCapacity * sizeof(PendingNameString));
                    staging.capacity = newCapacity;
                    staging.offset = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount);
                    PendingNameString* newSlot = staging.begin + staging.offset + staging.size;
                    unsigned int loopCount = 1;
                    EmptyAllocator copyAllocator;
                    while (loopCount != 0) {
                        if (newSlot != 0) {
                            try {
                                unsigned int itemFlags = item.flags;
                                if ((itemFlags >> 31) == 0) {
                                    unsigned int itemLength = item.length;
                                    CopyQueuedInlineWords(newSlot->flags, newSlot->length, newSlot->data, itemFlags,
                                                          itemLength, item.data);
                                } else {
                                    ReserveQueuedHeapName(newSlot, newSlot->flags, newSlot->length, newSlot->data,
                                                          item.length);
                                    char growthStringAllocator[12];
                                    growthStringAllocator[0] = growthStringAllocator[4];
                                    String_ReplaceRange(newSlot, 0, 0, item.data, item.data + item.length,
                                                        growthStringAllocator);
                                }
                            } catch (...) {
                                throw;
                            }
                        }
                        --loopCount;
                        ++newSlot;
                        ++staging.size;
                    }

                    PendingNameString* oldData = (PendingNameString*)AsCB2dPlayerSimple(self)->pendingPulseNodeNames;
                    EmptyAllocator moveAllocator;
                    int movedCount =
                        (int)((char*)(oldData +
                                      (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount)) -
                              (char*)oldData) /
                        (int)sizeof(PendingNameString);
                    staging.offset -= movedCount;
                    // Move: raw copy transfers the old prefix's heap ownership;
                    // zeroing source tags prevents its later destruction from freeing it.
                    memcpy(staging.begin + staging.offset, oldData, movedCount * sizeof(PendingNameString));
                    memset(oldData, 0, movedCount * sizeof(PendingNameString));
                    staging.size += movedCount;

                    // Commit: player takes the new buffer; staging takes the old one.
                    unsigned int previousSize = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount);
                    PendingNameString* swapData =
                        (*(PendingNameString**)&AsCB2dPlayerSimple(self)->pendingPulseNodeNames);
                    PendingNameString* newData = staging.begin;
                    unsigned int oldCapacity =
                        (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCapacity);
                    staging.begin = swapData;
                    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCapacity) = staging.capacity;
                    staging.capacity = oldCapacity;
                    AsCB2dPlayerSimple(self)->pendingPulseNodeNames = newData;
                    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount) = staging.size;
                    staging.size = previousSize;

                    // Retire: moved-from strings have zero tags; release old storage.
                    PendingNameString* oldStart = swapData + staging.offset;
                    EmptyAllocator destroyAllocator;
                    PendingNameString* p = oldStart + staging.size;
                    while (p > oldStart) {
                        --p;
                        if (p != 0 && (p->flags >> 31) != 0) {
                            appFreeUni__FPv(p->data);
                        }
                    }
                    staging.size = 0;
                }
            }
            ++nameIndex;
        }
    }

    // Phase 3: convert the caller's divisor to the per-update opacity increment.
    int result;
    if ((unsigned char)opacityDivisor != 0) {
        result = (int)(255.0f / (float)(unsigned char)opacityDivisor) & 0xFF;
    } else {
        result = 0;
    }
    AsCB2dPlayerSimple(self)->pulseStep = (unsigned char)result;
}

#pragma push
#pragma use_lmw_stmw on
// Replace queued node-name strings with copies of the caller's name array.
// nodeCount is truncated to eight bits (unlike unsigned itemCount's full-width
// comparison). Valid calls supply 0..255 nonnull C strings in nonaliasing storage.
// Strings are copied into owned storage; later resolution discards their contents.
void CB2dPlayerSimple::SetPendingNodeNames(void* object, const char** nodeNames, int nodeCount) {
    // Phase 1: destroy old name contents; the vector allocation stays reusable.
    char* self = (char*)object;
    EmptyAllocator clearAllocator;
    unsigned int oldCount = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount);
    QueuedNodeName* oldEnd = (QueuedNodeName*)AsCB2dPlayerSimple(self)->pendingNodeNames + oldCount;
    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount) -= oldCount;
    DestroyPendingNameContents(oldEnd, oldCount);

    // Phase 2: copy each name and append. The growth branch constructs a suffix
    // in guarded storage, moves/zeros the old prefix, swaps, then frees old storage.
    unsigned char nameIndex = 0;
    if ((unsigned char)nodeCount != 0) {
        while (nameIndex < (unsigned char)nodeCount) {
            const char* text = ((const char**)nodeNames)[nameIndex];
            char allocatorScratch[8];
            {
                // OWN TEXT: temporary owns a copy until its destructor runs after append.
                QueuedNodeName item;
                item.flags = 0;
                item.length = 0;
                item.data = 0;
                unsigned int length = strlen(text);
                String_ReserveCapacity(&item, length);
                allocatorScratch[0] = allocatorScratch[4];
                String_ReplaceRange(&item, 0, 0, text, text + length, allocatorScratch);

                unsigned int count = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount);
                unsigned int capacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCapacity);
                EmptyAllocator appendAllocator;
                // Append with spare capacity: copy one value, then publish its count.
                if (count < capacity) {
                    QueuedNodeName* slot = (QueuedNodeName*)AsCB2dPlayerSimple(self)->pendingNodeNames + count;
                    if (slot != 0) {
                        try {
                            unsigned int itemFlags = item.flags;
                            if ((itemFlags >> 31) == 0) {
                                unsigned int itemLength = item.length;
                                CopyQueuedInlineWords(slot->flags, slot->length, slot->data, itemFlags, itemLength,
                                                      item.data);
                            } else {
                                ReserveQueuedHeapName(slot, slot->flags, slot->length, slot->data, item.length);
                                char appendStringAllocator[12];
                                appendStringAllocator[0] = appendStringAllocator[4];
                                String_ReplaceRange(slot, 0, 0, item.data, item.data + item.length,
                                                    appendStringAllocator);
                            }
                        } catch (...) {
                            throw;
                        }
                    }
                    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount) =
                        (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount) + 1;
                } else {
                    EmptyAllocator growAllocator;
                    EmptyAllocator rangeAllocator;
                    unsigned int capacityForError = capacity;
                    if (kMaxStringVectorElements - capacityForError < 1) {
                        THROW_PENDING_NAME_LENGTH_ERROR();
                    }

                    // Reserve: until swap, staging owns every newly constructed name.
                    unsigned int* capacityAddress =
                        (unsigned int*)(self + PLAYER_MEMBER_OFFSET(pendingNodeNameCapacity));
                    unsigned int required =
                        (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount) + 1 - *capacityAddress;
                    QueuedNodeNameVector staging(capacityAddress);
                    unsigned int growthCapacity = *capacityAddress;
                    if (required > kMaxStringVectorElements - growthCapacity) {
                        THROW_PENDING_NAME_LENGTH_ERROR();
                    }

                    unsigned int newCapacity;
                    if (growthCapacity < kStringVectorSmallGrowthLimit) {
                        unsigned int growthThreeFifths = ((growthCapacity + 1) * 3) / 5;
                        unsigned int* growthChoice;
                        if (growthThreeFifths < required) {
                            growthChoice = &required;
                        } else {
                            growthChoice = &growthThreeFifths;
                        }
                        newCapacity = growthCapacity + *growthChoice;
                    } else if (growthCapacity < kStringVectorHalfGrowthLimit) {
                        unsigned int growthHalf = (growthCapacity + 1) >> 1;
                        unsigned int* growthChoice;
                        if (growthHalf < required) {
                            growthChoice = &required;
                        } else {
                            growthChoice = &growthHalf;
                        }
                        newCapacity = growthCapacity + *growthChoice;
                    } else {
                        newCapacity = kMaxStringVectorElements;
                    }
                    EmptyAllocator allocationAllocator;
                    if (newCapacity > kMaxStringVectorElements) {
                        THROW_PENDING_NAME_LENGTH_ERROR();
                    }

                    staging.begin = (QueuedNodeName*)appMallocDummy__FUl(newCapacity * sizeof(QueuedNodeName));
                    staging.capacity = newCapacity;
                    staging.offset = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount);
                    QueuedNodeName* newSlot = staging.begin + staging.offset + staging.size;
                    unsigned int loopCount = 1;
                    EmptyAllocator copyAllocator;
                    while (loopCount != 0) {
                        if (newSlot != 0) {
                            try {
                                unsigned int itemFlags = item.flags;
                                if ((itemFlags >> 31) == 0) {
                                    unsigned int itemLength = item.length;
                                    CopyQueuedInlineWords(newSlot->flags, newSlot->length, newSlot->data, itemFlags,
                                                          itemLength, item.data);
                                } else {
                                    ReserveQueuedHeapName(newSlot, newSlot->flags, newSlot->length, newSlot->data,
                                                          item.length);
                                    char growthStringAllocator[12];
                                    growthStringAllocator[0] = growthStringAllocator[4];
                                    String_ReplaceRange(newSlot, 0, 0, item.data, item.data + item.length,
                                                        growthStringAllocator);
                                }
                            } catch (...) {
                                throw;
                            }
                        }
                        --loopCount;
                        ++newSlot;
                        ++staging.size;
                    }

                    QueuedNodeName* oldData = (QueuedNodeName*)AsCB2dPlayerSimple(self)->pendingNodeNames;
                    EmptyAllocator moveAllocator;
                    int movedCount =
                        (int)((char*)(oldData + (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount)) -
                              (char*)oldData) /
                        (int)sizeof(QueuedNodeName);
                    staging.offset -= movedCount;
                    // Move: raw copy transfers the old prefix's heap ownership;
                    // zeroing source tags prevents its later destruction from freeing it.
                    memcpy(staging.begin + staging.offset, oldData, movedCount * sizeof(QueuedNodeName));
                    memset(oldData, 0, movedCount * sizeof(QueuedNodeName));
                    staging.size += movedCount;

                    // Commit: player takes the new buffer; staging takes the old one.
                    unsigned int previousSize = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount);
                    QueuedNodeName* swapData = (*(QueuedNodeName**)&AsCB2dPlayerSimple(self)->pendingNodeNames);
                    QueuedNodeName* newData = staging.begin;
                    unsigned int oldCapacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCapacity);
                    staging.begin = swapData;
                    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCapacity) = staging.capacity;
                    staging.capacity = oldCapacity;
                    AsCB2dPlayerSimple(self)->pendingNodeNames = newData;
                    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingNodeNameCount) = staging.size;
                    staging.size = previousSize;

                    // Retire: moved-from strings have zero tags; release old storage.
                    QueuedNodeName* oldStart = swapData + staging.offset;
                    EmptyAllocator destroyAllocator;
                    QueuedNodeName* p = oldStart + staging.size;
                    while (p > oldStart) {
                        --p;
                        if (p != 0 && (p->flags >> 31) != 0) {
                            appFreeUni__FPv(p->data);
                        }
                    }
                    staging.size = 0;
                }
            }
            ++nameIndex;
        }
    }
}
#pragma pop
#undef THROW_VECTOR_LENGTH_ERROR

struct PendingItemIdVector {
    unsigned int* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* sourceCapacity;
    unsigned int offset;

    // Initialize an empty guarded word buffer for pending item-ID growth.
    PendingItemIdVector(unsigned int* p) : begin(0), size(0), capacity(0), sourceCapacity(p), offset(0) {}

    // Clear staged word count and release the currently owned allocation.
    ~PendingItemIdVector() {
        if (begin != 0) {
            unsigned int count = size;
            size -= count;
            appFreeUni__FPv(begin);
        }
    }
};

// Select the larger item-vector growth bound without copying its storage.
inline const unsigned int& MaxItemVectorGrowth(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

#pragma push
#pragma use_lmw_stmw on
// Replace queued item-ID words, preserving their float-sized storage representation.
// ABI: itemIds points to itemCount aligned 32-bit ID BIT PATTERNS, not floats to
// convert. The float* signature is retained; unsigned loads copy the words and
// ResolvePendingNodes later passes each word as a pointer-encoded lookup key.
// Source is borrowed only during this copy and must not alias the destination
// vector. itemCount must be <=255: the unsigned-char loop index wraps at 256,
// so larger counts cannot terminate. Zero count clears the queue without reading.
// Capacity is retained; reallocation commits each appended ID independently.
// New caller example: unsigned int ids[]={1012u,1013u}; pass (const float*)ids
// under this legacy ABI, NOT float values {1012.0f,1013.0f}. The latter encode
// 0x447D0000/0x447D4000, entirely different lookup keys. LoadingIndicator_CreatePlayer
// uses kLoadingHiddenItemIdWords from the data section; follow that bit-copy
// convention. The pointer-shaped BpSlot_FindItemById argument is an opaque 32-bit
// integer key, never dereferenced here. No float arithmetic touches those words.
void CB2dPlayerSimple::SetPendingItemIds(void* object, const float* itemIds, unsigned int itemCount) {
    // Phase 1: clear logical size. Existing allocation remains reusable.
    char* self = (char*)object;
    unsigned int growthSmall;
    unsigned int growthHalf;
    unsigned int required;
    (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount) =
        (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount) -
        (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount);

    unsigned char itemIndex = 0;
    while (itemIndex < itemCount) {
        unsigned int count = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount);
        unsigned int capacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCapacity);
        if (capacity > count) {
            unsigned int next = count + 1;
            (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount) = next;
            ((unsigned int*)AsCB2dPlayerSimple(self)->pendingItemIds)[next - 1] =
                ((const unsigned int*)itemIds)[itemIndex];
        } else {
            // Phase 2 (growth): validate capacity, allocate a guarded buffer,
            // append the new word, move the old prefix, then swap ownership.
            if (kMaxWordVectorElements - capacity < 1) {
                char* errorText = LOADING_STRING(loadingScreenStrings, vectorLengthError);
                {
                    union {
                        double alignment;
                        unsigned char bytes[sizeof(std::length_error)];
                    } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct(errorText);
                    exception_runtime::__throw(LOADING_STRING(exception_runtime::loadingScreenStrings, lengthErrorType),
                                               error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
            }
            required = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount) + 1 -
                       (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCapacity);
            unsigned int oldCapacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCapacity);
            PendingItemIdVector staging((unsigned int*)(self + PLAYER_MEMBER_OFFSET(pendingItemIdCapacity)));
            if (required > kMaxWordVectorElements - oldCapacity) {
                char* errorText = LOADING_STRING(loadingScreenStrings, vectorLengthError);
                {
                    union {
                        double alignment;
                        unsigned char bytes[sizeof(std::length_error)];
                    } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct(errorText);
                    exception_runtime::__throw(LOADING_STRING(exception_runtime::loadingScreenStrings, lengthErrorType),
                                               error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
            }
            unsigned int newCapacity;
            if (oldCapacity < kWordVectorSmallGrowthLimit) {
                growthSmall = ((oldCapacity + 1) * 3) / 5;
                newCapacity = oldCapacity + MaxItemVectorGrowth(growthSmall, required);
            } else if (oldCapacity < kWordVectorHalfGrowthLimit) {
                growthHalf = (oldCapacity + 1) >> 1;
                newCapacity = oldCapacity + MaxItemVectorGrowth(growthHalf, required);
            } else {
                newCapacity = kMaxWordVectorElements;
            }
            if (newCapacity > kMaxWordVectorElements) {
                char* errorText = LOADING_STRING(loadingScreenStrings, vectorLengthError);
                {
                    union {
                        double alignment;
                        unsigned char bytes[sizeof(std::length_error)];
                    } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct(errorText);
                    exception_runtime::__throw(LOADING_STRING(exception_runtime::loadingScreenStrings, lengthErrorType),
                                               error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
            }
            staging.begin = (unsigned int*)appMallocDummy__FUl(newCapacity * 4);
            staging.offset = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount);
            staging.capacity = newCapacity;
            (staging.begin + staging.offset)[staging.size] = ((const unsigned int*)itemIds)[itemIndex];
            ++staging.size;

            // Move: existing IDs are raw words; zero old words before freeing.
            unsigned int* oldData = (unsigned int*)AsCB2dPlayerSimple(self)->pendingItemIds;
            unsigned int moved =
                (unsigned int)(((char*)(oldData + (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount)) -
                                (char*)oldData) /
                               4);
            staging.offset -= moved;
            memcpy(staging.begin + staging.offset, oldData, moved * 4);
            memset(oldData, 0, moved * 4);
            staging.size += moved;

            // Commit: the player takes the new allocation; staging frees the old one.
            (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount) = 0;
            unsigned int* swapData = (unsigned int*)AsCB2dPlayerSimple(self)->pendingItemIds;
            unsigned int* newData = staging.begin;
            unsigned int swapCapacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCapacity);
            staging.begin = swapData;
            (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCapacity) = staging.capacity;
            staging.capacity = swapCapacity;
            AsCB2dPlayerSimple(self)->pendingItemIds = newData;
            (*(unsigned int*)&AsCB2dPlayerSimple(self)->pendingItemIdCount) = staging.size;
            staging.size = 0;
        }
        ++itemIndex;
    }
}
#pragma pop

struct ErrorCleanup922FC {
    char* data;
    char* flag;
    // Guard resolved-node vector diagnostic text before control-block construction.
    ErrorCleanup922FC(char* p, char* f) : data(p), flag(f) {}
    // Release diagnostic text if its shared owner was not installed.
    ~ErrorCleanup922FC() {
        if (data)
            __dla__FPv(data);
    }
};
struct ErrorOwner922FC {
    void* vtable;
    int refs;
    int owners;
    char* data;
    // Attach diagnostic text to a control block with one strong and weak reference.
    ErrorOwner922FC(char* p) : refs(1), owners(1) {
        vtable = s_sharedCharDeleterVtable;
        data = p;
    }
};
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct ErrorBase922FC : std::exception {
    // Install the exception vtable expected by the CodeWarrior ABI.
    void SetTable(const void* vtable) {
        *(const void**)this = vtable;
    }

    char* data;
    ErrorOwner922FC* owner;
    // Copy diagnostic text and transfer it to its shared control block.
    ErrorBase922FC(const char* text) {
        SetTable(__vt__Q23std11logic_error);
        struct {
            char value;
            char pad[3];
        } flag;
        flag.value = 0;
        int n = strlen(text) + 1;
        char* copy = (char*)__nwa__FUl((n << 1) - n);
        data = copy;
        {
            ErrorCleanup922FC cleanup(copy, &flag.value);
            owner = new ErrorOwner922FC(copy);
            cleanup.data = 0;
        }
        strcpy(data, text);
    }
};
}
namespace std {}
extern "C" {
// Construct and throw the resolved-node vector's length exception in local storage.
inline void ThrowResolvedNodeLengthError(std::length_error* error) {
    error->__ct(LOADING_STRING(loadingScreenStrings, vectorLengthError));
    exception_runtime::__throw(LOADING_STRING(exception_runtime::loadingScreenStrings, lengthErrorType), error,
                               (void*)exception_runtime::__dt__Q23std12length_errorFv);
}
// Keep storage at the caller: moving it into the inline function changes stack
// offsets in the generated code. The operation itself lives in the helper above.
#define THROW_LENGTH_922FC()                                          \
    do {                                                              \
        union {                                                       \
            double alignment;                                         \
            unsigned char bytes[sizeof(std::length_error)];           \
        } storage;                                                    \
        std::length_error* error = (std::length_error*)storage.bytes; \
        ThrowResolvedNodeLengthError(error);                          \
    } while (0)

struct PointerStage922FC {
    void** data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* sourceCapacity;
    unsigned int offset;
    // Initialize a guarded replacement buffer of borrowed BP node pointers.
    PointerStage922FC(unsigned int* p) : data(0), size(0), capacity(0), sourceCapacity(p), offset(0) {}
    // Free pointer-vector storage without destroying the borrowed BP nodes.
    ~PointerStage922FC() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

// Select the larger resolved-node vector growth bound.
inline const unsigned int& MaxResolvedNodeGrowth(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Append a resolved BP node, growing its pointer vector with exception cleanup if needed.
inline void AppendResolvedNode(char* self, void* result) {
    unsigned int size = (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCount);
    unsigned int capacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCapacity);
    if (size < capacity) {
        void** data = (*(void***)&AsCB2dPlayerSimple(self)->resolvedNodes);
        void** end = data + ++(*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCount);
        end[-1] = result;
    } else {
        if (kMaxWordVectorElements - capacity < 1) {
            THROW_LENGTH_922FC();
        }
        unsigned int required = (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCount) + 1 -
                                (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCapacity);
        PointerStage922FC staging((unsigned int*)(self + PLAYER_MEMBER_OFFSET(resolvedNodeCapacity)));
        unsigned int oldCapacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCapacity);
        if (required > kMaxWordVectorElements - oldCapacity) {
            THROW_LENGTH_922FC();
        }
        unsigned int newCapacity;
        if (oldCapacity < kWordVectorSmallGrowthLimit) {
            const unsigned int smallGrowth = (oldCapacity + 1) * 3 / 5;
            newCapacity = oldCapacity + MaxResolvedNodeGrowth(smallGrowth, required);
        } else if (oldCapacity < kWordVectorHalfGrowthLimit) {
            const unsigned int halfGrowth = (oldCapacity + 1) / 2;
            newCapacity = oldCapacity + MaxResolvedNodeGrowth(halfGrowth, required);
        } else
            newCapacity = kMaxWordVectorElements;
        if (newCapacity > kMaxWordVectorElements) {
            THROW_LENGTH_922FC();
        }
        staging.data = (void**)appMallocDummy__FUl(newCapacity * 4);
        staging.offset = (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCount);
        staging.capacity = newCapacity;
        (staging.data + staging.offset)[staging.size++] = result;
        void** oldData = (*(void***)&AsCB2dPlayerSimple(self)->resolvedNodes);
        int moved =
            (int)((char*)(oldData + (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCount)) - (char*)oldData) /
            4;
        staging.offset -= moved;
        memcpy(staging.data + staging.offset, oldData, moved * 4);
        memset(oldData, 0, moved * 4);
        staging.size += moved;
        (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCount) = 0;
        void** swapData = (*(void***)&AsCB2dPlayerSimple(self)->resolvedNodes);
        void** newData = staging.data;
        staging.data = swapData;
        unsigned int swapCapacity = (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCapacity);
        (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCapacity) = staging.capacity;
        staging.capacity = swapCapacity;
        (*(void***)&AsCB2dPlayerSimple(self)->resolvedNodes) = newData;
        (*(unsigned int*)&AsCB2dPlayerSimple(self)->resolvedNodeCount) = staging.size;
        staging.size = 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Resolve queued names and item IDs against the current BP slot, then discard the queues.
// bpObject must be a loaded controller entry. Every queued name/ID must resolve
// to a sprite for its later consumers: null lookup results are appended unchanged,
// and SetSpritesEnabled/ResetOpacityPulse do not filter them. Outputs borrow BP
// nodes; replacing/removing that entry invalidates them. Name storage is released,
// ID logical size cleared, and vector capacity retained. Throws may leave a prefix.
// Replacement-artwork identity map (not a texture pixel-name lookup):
// - LoadingIndicator_CreatePlayer -> SetPendingNodeNames uses 13 string keys in
//   R_win_rightdown, R_win__midup, R_win_middown, R_press_b_btn, R_pagechange,
//   R_page_up, R_page_down, R_win_lmiddown, R_win_rmiddown, R_win__base.
//   (This list must agree with that source table when renaming assets.)
// - SetPendingItemIds uses numeric IDs 1012/1013 from kLoadingHiddenItemIdWords
//   (the data section); these are not ordinal positions in the sprite array.
// - ResolvePendingPulseNodes uses R_loading_green: s_loadingPulseNodeNames in
// - phase1_program_strings.cpp loadingScreenStrings selects loading.b2d,
//   initial label "loading" and loading.gsr. BpObject_PollLoad (bp_object_sprite_resources.cpp)
//   registers B2D command work and runs the sprite-registration script;
//   BpSlot_RegisterWork (bp_sprite_controller_and_drawing.cpp) exposes that generated sprite array.
// - BpSlot_FindNodeByName (bp_sprite_animation_and_draw_callbacks.cpp) compares sprite.name at +8 with strcmp
//   through the linked list; FindItemById scans count records of 0x1C8 bytes
//   comparing sprite.id at +4. Missing keys report diagnostics then return null.
// Preserve those names/IDs and script label in replacement B2D artwork, together
// with valid references to replacement texture/UV data. Reordering alone need
// not change IDs; renaming/re-exporting with new IDs requires updating these
// request tables too. Replacing only loading.gsr cannot supply missing B2D nodes.
void CB2dPlayerSimple::ResolvePendingNodes(void* object) {
    unsigned int inputCount;
    // Phase 1: append lookups by name; nodes are borrowed from this BP slot.
    char* self = (char*)object;
    void* owner = ((BpEntrySlotView*)(*(void**)&AsCB2dPlayerSimple(self)->bpObject))->slotKey;

    char* input = (*(char**)&AsCB2dPlayerSimple(self)->pendingNodeNames);

    for (char* p = input; p != (*(char**)&AsCB2dPlayerSimple(self)->pendingNodeNames) +
                                   AsCB2dPlayerSimple(self)->pendingNodeNameCount * 0xC;
         p += 0xC) {
        void* name;
        if (!((((PackedStringStorage*)p)->flags >> 31) == 0)) {
            name = ((PackedStringStorage*)p)->heapData;
        } else {
            name = (void*)((p + 1));
        }
        void* result = BpSlot_FindNodeByName(owner, name);

        AppendResolvedNode(self, result);
    }

    // Phase 2: destroy consumed queued strings, leaving capacity for reuse.
    inputCount = AsCB2dPlayerSimple(self)->pendingNodeNameCount;
    input = (*(char**)&AsCB2dPlayerSimple(self)->pendingNodeNames) + inputCount * 0xC;
    AsCB2dPlayerSimple(self)->pendingNodeNameCount = inputCount - inputCount;
    while (inputCount) {
        input -= 0xC;
        if (0 != input) {
            if ((((PackedStringStorage*)input)->flags >> 31) != 0) {
                appFreeUni__FPv(((PackedStringStorage*)input)->heapData);
            }
        }
        inputCount = inputCount - 1;
    }

    // Phase 3: append ID lookups after named nodes; ID words are encoded keys,
    // not addresses of strings despite this legacy char** iteration view.
    char** itemIdCursor = (*(char***)&AsCB2dPlayerSimple(self)->pendingItemIds);
    unsigned int itemIdCount;
    for (; itemIdCursor !=
           (*(char***)&AsCB2dPlayerSimple(self)->pendingItemIds) + AsCB2dPlayerSimple(self)->pendingItemIdCount;
         ++itemIdCursor) {
        void* result = BpSlot_FindItemById(owner, *itemIdCursor);

        AppendResolvedNode(self, result);
    }

    itemIdCount = AsCB2dPlayerSimple(self)->pendingItemIdCount;
    AsCB2dPlayerSimple(self)->pendingItemIdCount = itemIdCount - itemIdCount;
}

#pragma pop
#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Resolve pulse-node names into the sprite list and release the queued names.
// Same loaded-entry/nonnull-result contract as ResolvePendingNodes. Appends to
// the existing resolved vector; caller chooses whether to clear its size first.
void CB2dPlayerSimple::ResolvePendingPulseNodes(void* object) {
    char* self = (char*)object;
    char* items = (*(char**)&AsCB2dPlayerSimple(self)->pendingPulseNodeNames);
    void* owner = ((BpEntrySlotView*)(*(void**)&AsCB2dPlayerSimple(self)->bpObject))->slotKey;
    int itemCount = AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount;
    char* end = items + itemCount * 0xC;
    for (char* p = items; p != end; p += 0xC) {
        void* str;
        if ((((PackedStringStorage*)p)->flags >> 31) != 0) {
            str = ((PackedStringStorage*)p)->heapData;
        } else {
            str = (void*)(p + 1);
        }
        void* res = BpSlot_FindNodeByName(owner, str);
        AppendResolvedNode(self, res);
    }

    int count = AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount;
    AsCB2dPlayerSimple(self)->pendingPulseNodeNameCount = count - count;
    char* ptr = (*(char**)&AsCB2dPlayerSimple(self)->pendingPulseNodeNames) + count * 0xC;
    while (count != 0) {
        ptr -= 0xC;
        if (ptr != 0 && (((PackedStringStorage*)ptr)->flags >> 31) != 0) {
            appFreeUni__FPv(((PackedStringStorage*)ptr)->heapData);
        }
        --count;
    }
}
#pragma opt_propagation reset
#pragma pop

#pragma dont_inline on
#pragma opt_propagation off
// Advance creation/load/script playback, pulse sprite opacity, then draw or release the player.
// Controls loading animation cadence in UPDATE CALLS, not seconds; preserve
// same-call create/load fallthrough, deferred idle deletion, and callback-before-draw.
// Storage/phase lookup for this dispatcher (single canonical layout in
// CB2dPlayerSimple.h, no separate overlay object): +0 bpObject borrowed from BP
// controller; +4 state; +8/+14/+20/+2C four packed strings (file/initial/request/
// repeat). PackedStringStorage/InlineStringBytes view the SAME twelve bytes;
// PLAYER_STRING_BYTES is only the inline branch, PLAYER_SCRIPT_IS_HEAP tests its
// word header. +38 repeat count; +3C pending request; +40 resolved node vector;
// +4C/+58/+64 pending names/ID words/pulse names; +70/+74 pulse step/phase;
// +78 deleting interface. owning slot -> player -> BP object -> sprite nodes are
// four distinct lifetimes. ID words are unrelated to the packed-string header.
// Phase exits: drawThisFrame skips further state work but still invokes callback
// and draws; resetToIdle writes state0 THEN uses that same draw tail. Only idle
// on ENTRY deletes player. Same-call work is create -> poll -> initialize nodes
// -> script/pulse chooser. Starting/repeating/waiting keeps playback active;
// finishing resets to idle in this call (kPlayerFinished is transient on this path).
// Creation failure contract (CBpCtrl::CreateEntry in bp_ctrl_entry_manager.cpp): null path or
// exhausted32 IDs returns0 without publishing an ID; path-setup failure may return0
// AFTER registering an owned object and publishing its ID. This dispatcher
// assigns bpObject ONLY on nonzero status, but moves to WaitForLoad either way.
// Fresh Init sets bpObject=0: failure then skips node resolution/poll/script,
// marks playback complete, resets to idle and draws nothing; NEXT call deletes.
// A reused player retains its previous bpObject on failure, so that borrowed
// object must still be valid; it may be polled/drawn and have scripts advanced.
// Newly registered failure entries stay controller-owned even if never assigned
// to this player. No rollback/retry or leak-free failure guarantee is supplied.
// Draw tail checks null, invokes callback, reloads bpObject and draws WITHOUT a
// second null check. Callback must preserve a valid binding/player for that call.
void B2dPlayer_UpdateAndDraw(void* object) {
    // Player states (player->state): 0 idle -> 1 create BP entry -> 2 wait for load ->
    // 3 playing -> 4 finished. One call can run several steps in the same frame (the gotos below are the
    // state-machine exits: drawThisFrame keeps the state, resetToIdle sets state 0). Idle on entry means
    // "delete the player and clear the slot"; every other state draws the BP object once.
    // object is a mutable owning player slot (void**). The slot is cleared only
    // on deletion; controller entries own BP objects, and player node pointers borrow them.
    // Decision contract on script completion:
    //   completed phase       pending=0                  pending!=0
    //   initial+repeat name   start first repeat         start first repeat
    //   repeat, remaining!=0  restart; positive count--  clear repeat, choose request
    //   repeat, remaining==0  clear repeat, choose request (either pending value)
    //   requested name        wait, keep name            start once, consume name
    //   no requested name     wait                      finish
    // Initial name is consumed even without repeat; continue to request rows then.
    // Initial->first-repeat ignores BOTH pending and remaining count. The next
    // completion honors pending before any restart: even indefinite repetition
    // stops then. A negative remaining count never decrements. A request with a
    // name stays pending while it plays; its name is gone, so its completion
    // takes the final row and finishes. With no request signal even a finite
    // repeat sequence waits indefinitely after its last play. Request is latched
    // by CB2dPlayerHandle::RequestPendingScript, cleared only by configuration/
    // reset paths, not by starting the requested script. Test both early and late
    // requests, absent initial/repeat/request names, count0/1/32768 and wrap inputs.
    // Exit table (entry state -> tail action):
    //   idle: delete/clear owning slot; create/wait/playing: continue same-frame work;
    //   finished: reset state then draw; unknown states: draw without state repair.
    // drawThisFrame does not imply a new script was started. resetToIdle still draws
    // once; the following call observes idle on entry and owns deletion.
    // One-shot consumption empties strings but retains their heap capacity.
    char* owner = (char*)object; // owning slot; tail reads it before callback/draw or deletion.
    CB2dPlayerSimple* player = *(CB2dPlayerSimple**)owner;
    if (player == 0)
        return;
    int state = player->state;
    int shouldDeletePlayer; // 1 only for idle ON ENTRY; other states draw once.
    int phaseReady;         // Creation/load phase may advance this call.
    int playbackDone;       // No current script remains; reset to idle before draw.
    int keepPlaybackActive; // Inner chooser: 1 starts a script OR waits for a request.
    // 0x8028FF28: creation, loading, and playback advance in one update.
    // Non-idle states retain their original comparison order and same-frame transitions.
    if (kPlayerIdle == state) {
        shouldDeletePlayer = 1;
    } else {
        {
            // Phase 1: creation/load. Labelled exits preserve same-frame
            // fallthrough while spelling out draw versus reset-to-idle.
            if (state == kPlayerPlaying) {
                // Playback is already active; skip creation and loading.
            } else {
                if (state >= kPlayerPlaying) {
                    if (kPlayerStateLimit <= state)
                        goto drawThisFrame;
                    goto resetToIdle;
                }
                if (state != kPlayerCreateEntry) {
                    if (state >= kPlayerCreateEntry) {
                        // The only remaining valid early state waits for its BP object.
                    } else {
                        goto drawThisFrame;
                    }
                } else {
                    if (0 == ((int&)Sing_805DC5F0())) {
                        if (0 == s_bpCtrlInitialized) {
                            ((CBpCtrl*)s_bpCtrl)->__ct();
                            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor,
                                                     s_bpCtrlDtorRegistration_Menu8028FE2C);
                            s_bpCtrlInitialized = 1;
                        }
                        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
                    }
                    {
                        unsigned int fileNameFlags = (*(int*)&player->bpFileNameStorageFlags);
                        void* controller = (void*)((int&)Sing_805DC5F0());

                        int entryId = 0;
                        int fileNameIsHeap = (int)((unsigned int)fileNameFlags >> 31);
                        int createResult =
                            ((CBpCtrl*)controller)
                                ->CreateEntry(
                                    fileNameIsHeap
                                        ? player->bpFileNameHeap
                                        : (void*)PLAYER_STRING_BYTES(player, bpFileNameStorageFlags)->characters,
                                    &entryId, 0);
                        // 0x8028FF28: entry creation shares the state transition below.
                        if (createResult == 0) {
                            phaseReady = 1;
                        } else {
                            void* entry = IdMap_Lookup(controller, entryId);
                            (*(void**)&player->bpObject) = entry;
                            phaseReady = 1;
                        }
                        if (!phaseReady)
                            goto drawThisFrame;
                        player->state = kPlayerWaitForLoad;
                    }
                }
                {
                    void* bpObject = (*(void**)&player->bpObject);
                    if (bpObject == 0) {
                        phaseReady = 1;
                    } else if (0 == (BpObject_PollLoad(bpObject))) {
                        phaseReady = 0;
                    } else {
                        {
                            int _t = player->resolvedNodeCount;
                            player->resolvedNodeCount = _t - _t;
                        }
                        CB2dPlayerSimple::ResolvePendingNodes(player);
                        ((CB2dPlayerSimple*)player)->SetSpritesEnabled(0);
                        {
                            int _t = player->resolvedNodeCount;
                            player->resolvedNodeCount = _t - _t;
                        }
                        CB2dPlayerSimple::ResolvePendingPulseNodes(player);
                        CB2dPlayerSimple::ResetOpacityPulse(player);
                        int startedScript = CB2dPlayerSimple::StartPendingScript(player);
                        if (startedScript == 0)
                            CB2dPlayerSimple::UnloadAndClearPendingState(player);
                        phaseReady = 1;
                    }
                    if (0 == phaseReady)
                        goto drawThisFrame;
                    player->state = kPlayerPlaying;
                }
            }
            // Phase 2: pulse opacity and, at script completion, choose the
            // next initial/repeat/requested script. Inner breaks only finish
            // their local decision block; the labelled exits choose the tail.
            {
                int initialScriptLength;
                // 0x8028FF28: animation readiness shares the completion test.
                do {
                    void* bpObject = (*(void**)&player->bpObject);
                    if (bpObject == 0) {
                        playbackDone = 1;
                        break;
                    }
                    unsigned char pulseStep = player->pulseStep;
                    if (pulseStep != 0) {
                        int pulsePhase = player->pulsePhase /* 0x74 */;
                        int sum = (pulsePhase + pulseStep);
                        player->pulsePhase /* 0x74 */ = sum;
                        if (sum > kOpacityFull) {
                            int wrappedPhase = player->pulsePhase /* 0x74 */;
                            player->pulsePhase /* 0x74 */ = wrappedPhase - kPulsePeriod;
                        }

                        int opacity = abs((player->pulsePhase /* 0x74 */));
                        char* sprite = (*(char**)&player->resolvedNodes);
                        while (sprite != (*(char**)&player->resolvedNodes) + player->resolvedNodeCount * 4) {
                            BpSprite_SetOpacity(*(void**)sprite, (void*)opacity);
                            sprite = sprite + 4;
                        }
                    }
                    bpObject = (*(void**)&player->bpObject);
                    int scriptFinished = BpObject_IsCurrentScriptComplete(bpObject);
                    if (scriptFinished == 0) {
                        playbackDone = 0;
                        break;
                    }

                    // Priority 1: an initial name marks the script just completed.
                    // Consume it once and prefer repeat before considering a request.
                    do {
                        int initialScriptFlags = (*(int*)&player->initialScriptStorageFlags);
                        unsigned int initialScriptIsHeap = (int)((unsigned int)initialScriptFlags >> 31);

                        if (0 == initialScriptIsHeap)
                            initialScriptLength = PLAYER_SCRIPT_INLINE_LENGTH(player, initialScriptStorageFlags);
                        else
                            initialScriptLength = player->initialScriptLength;
                        if (initialScriptLength) {

                            if (!initialScriptIsHeap) {
                                PLAYER_STRING_BYTES(player, initialScriptStorageFlags)->characters[0] = 0;
                                {

                                    PLAYER_STRING_BYTES(player, initialScriptStorageFlags)->flags =
                                        (unsigned char)((PLAYER_STRING_BYTES(player, initialScriptStorageFlags)
                                                             ->flags) &
                                                        ~kInlineStringLengthMask);
                                }
                            } else {
                                void* initialScript = player->initialScriptHeap;
                                *(unsigned char*)initialScript = 0;
                                player->initialScriptLength = 0;
                            }
                            {
                                int repeatScriptLength;

                                int repeatScriptIsHeap = PLAYER_SCRIPT_IS_HEAP(player, repeatScriptStorageFlags);
                                // cntlzw(0)>>5 == 1; cntlzw(1)>>5 == 0. Keep the
                                // original heap->inline Boolean conversion shape.
                                int cntl = __cntlzw(repeatScriptIsHeap);
                                int repeatScriptIsInline = (int)((unsigned int)cntl >> 5);

                                if (repeatScriptIsInline)
                                    repeatScriptLength = PLAYER_SCRIPT_INLINE_LENGTH(player, repeatScriptStorageFlags);
                                else
                                    repeatScriptLength = player->repeatScriptLength;
                                if (repeatScriptLength != 0) {
                                    void* scriptName;
                                    void* scriptObject = (*(void**)&player->bpObject);
                                    if (!repeatScriptIsInline)
                                        scriptName = player->repeatScriptHeap;
                                    else
                                        scriptName =
                                            (void*)PLAYER_STRING_BYTES(player, repeatScriptStorageFlags)->characters;
                                    BpObject_StartScriptByName(scriptObject, scriptName, 1);
                                    keepPlaybackActive = 1;
                                    break;
                                }
                            }
                        } else {
                            int repeatScriptLength;
                            // Priority 2: an already-running repeat consumes one positive
                            // repeat count; negative counts remain unlimited.

                            unsigned int repeatScriptIsHeap = PLAYER_SCRIPT_IS_HEAP(player, repeatScriptStorageFlags);
                            int cntl = __cntlzw(repeatScriptIsHeap);

                            int repeatScriptIsInline = (int)((unsigned int)cntl >> 5);
                            if (repeatScriptIsInline)
                                repeatScriptLength = PLAYER_SCRIPT_INLINE_LENGTH(player, repeatScriptStorageFlags);
                            else
                                repeatScriptLength = player->repeatScriptLength;
                            if (0 != repeatScriptLength) {

                                if (0 == (player->scriptRepeatsRemaining) || (player->pendingScript) != 0) {
                                    if (repeatScriptIsHeap == 0) {
                                        PLAYER_STRING_BYTES(player, repeatScriptStorageFlags)->characters[0] = 0;
                                        {
                                            unsigned char inlineFlags =
                                                PLAYER_STRING_BYTES(player, repeatScriptStorageFlags)->flags;
                                            PLAYER_STRING_BYTES(player, repeatScriptStorageFlags)->flags =
                                                (unsigned char)(inlineFlags & ~kInlineStringLengthMask);
                                        }
                                    } else {
                                        void* repeatScript = player->repeatScriptHeap;
                                        *(unsigned char*)repeatScript = 0;
                                        player->repeatScriptLength = 0;
                                    }
                                } else {
                                    void* scriptName;
                                    void* scriptObject = (*(void**)&player->bpObject);
                                    if (!repeatScriptIsInline)
                                        scriptName = player->repeatScriptHeap;
                                    else
                                        scriptName =
                                            (void*)PLAYER_STRING_BYTES(player, repeatScriptStorageFlags)->characters;
                                    BpObject_StartScriptByName(scriptObject, scriptName, 1);
                                    short repeatsRemaining = player->scriptRepeatsRemaining;
                                    if (repeatsRemaining > 0) {
                                        player->scriptRepeatsRemaining = (short)(repeatsRemaining - 1);
                                    }
                                    keepPlaybackActive = 1;
                                    break;
                                }
                            }
                        }

                        // Priority 3: keep waiting on a named requested script until
                        // pendingScript is set. Starting it clears its name, not its flag.
                        {
                            int requestedScriptFlags = (*(int*)&player->requestedScriptStorageFlags);

                            unsigned int requestedScriptLength;
                            int savedScriptFlags = requestedScriptFlags;
                            int cntl = __cntlzw((int)((unsigned int)savedScriptFlags >> 31));
                            unsigned int requestedScriptIsInline = (int)((unsigned int)cntl >> 5);

                            if (requestedScriptIsInline)
                                requestedScriptLength =
                                    PLAYER_SCRIPT_INLINE_LENGTH(player, requestedScriptStorageFlags);
                            else
                                requestedScriptLength = player->requestedScriptLength;
                            if (0 != requestedScriptLength) {
                                int scriptPending = player->pendingScript;
                                if (0 != scriptPending) {
                                    do {
                                        if (requestedScriptIsInline != 0) {
                                            {
                                                void* scriptName =
                                                    (void*)PLAYER_STRING_BYTES(player, requestedScriptStorageFlags)
                                                        ->characters;
                                                void* scriptObject = (*(void**)&player->bpObject);
                                                BpObject_StartScriptByName(scriptObject, scriptName, 1);
                                            }
                                            if (!((int)((unsigned int)requestedScriptFlags >> 31))) {
                                                PLAYER_STRING_BYTES(player, requestedScriptStorageFlags)
                                                    ->characters[0] = 0;
                                                {
                                                    unsigned char inlineFlags =
                                                        PLAYER_STRING_BYTES(player, requestedScriptStorageFlags)->flags;
                                                    PLAYER_STRING_BYTES(player, requestedScriptStorageFlags)->flags =
                                                        (unsigned char)(inlineFlags & ~kInlineStringLengthMask);
                                                }
                                                break;
                                            }
                                        } else {
                                            {
                                                void* scriptObject = (*(void**)&player->bpObject);
                                                void* scriptName = player->requestedScriptHeap;
                                                BpObject_StartScriptByName(scriptObject, scriptName, 1);
                                            }
                                            if (!((int)((unsigned int)requestedScriptFlags >> 31))) {
                                                PLAYER_STRING_BYTES(player, requestedScriptStorageFlags)
                                                    ->characters[0] = 0;
                                                {

                                                    PLAYER_STRING_BYTES(player, requestedScriptStorageFlags)->flags =
                                                        (unsigned char)((PLAYER_STRING_BYTES(
                                                                             player, requestedScriptStorageFlags)
                                                                             ->flags) &
                                                                        ~kInlineStringLengthMask);
                                                }
                                                break;
                                            }
                                        }
                                        {
                                            void* requestedScript = player->requestedScriptHeap;
                                            *(unsigned char*)requestedScript = 0;
                                            player->requestedScriptLength = 0;
                                        }
                                    } while (0);
                                }
                                keepPlaybackActive = 1;
                            } else {

                                int cntl = __cntlzw((player->pendingScript));
                                keepPlaybackActive = (int)((unsigned int)cntl >> 5);
                            }
                        }

                    } while (0);
                    {

                        // Convert "start or wait" into completion once at the chooser boundary.
                        playbackDone = (int)((unsigned int)(__cntlzw(keepPlaybackActive)) >> 5);
                    }
                } while (0);
                if (playbackDone == 0)
                    goto drawThisFrame;
                player->state = kPlayerFinished;
            }
        }
    resetToIdle:
        player->state = kPlayerIdle;
    drawThisFrame:
        shouldDeletePlayer = 0;
    }
    // Phase 3: all non-idle entry states draw this frame, even after becoming
    // idle. Only idle ON ENTRY takes the deletion branch and clears owner.
    if (shouldDeletePlayer == 0) {
        void* obj2 = *(void**)owner;
        void* bpObject = (obj2 != 0) ? *(void**)obj2 : 0;
        if (bpObject != 0) {
            BpObject_TryInvokeCurrentCallback(bpObject, 0);
            bpObject = *(void**)obj2;
            BpObject_Draw(bpObject);
        }
    } else {
        void* o = *(void**)owner;
        if (o != 0) {
            ((void (*)(void*, int))(*(void***)((char*)&AsCB2dPlayerSimple(o)->vtable))[2])(o, 1);
        }
        *(void**)owner = 0;
    }
    return;
}
#pragma opt_propagation reset
#pragma dont_inline off

} // extern "C"
