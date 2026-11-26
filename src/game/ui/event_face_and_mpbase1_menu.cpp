// Event scripts and facial animation: running an event script to completion, and the facial
// keyframe tracks (CEventFace) that drive a model's material animation.
//
// Also the base menu process (MPBase1) and the fading menu base (IFadeMenuBase) that screens use.

#include <decomp/static_init.h>
#include "classes/MPBase1Layout.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "game/singletons.h"
#include "classes/CEventHandle.h"
#include <decomp/cw_bitfields.h>
#include "classes/CEventFace.h"
#include "classes/CEventEffect.h"
#include "classes/CBpPlayer.h"

#define AsCEventHandle(p) ((CEventHandle*)(p))

extern "C" {

extern const char __vt__10CEventFace[];
extern const char kFacialMaterialAnimationTag[];
extern unsigned char s_conteModelSlots[];
extern signed char s_courseSlotTableInitialized;

void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void __dla__FPv(void*);
void CourseSlotTable_InitEmpty(void*);
void* SlotTable_GetEntry(void*, unsigned char);
unsigned char CourseSlotTable_IsExtraSlotRange(void*, unsigned char);
void WPuppet_ApplyNamedMaterialLayerAnimation(void*, void*, void*, void*, float);
void* WPuppet_FindAnimationRangeByName(void*, const char*);
void* WPuppet_GetAnimationNameSize(void*, unsigned short);
char* __nwa__FUl(void*);
void WPuppet_CopyAnimationName(void*, char*, unsigned short, void*);
void String_ReplaceRange(void*, int, unsigned int, char*, char*, char*);
void* WPuppet_GetAnimationValueByName(void*, void*, void*);

// Facial animation uses two independent byte-pair tracks: duration, material frame.
struct FaceKeyframe {
    unsigned char durationFrames;
    unsigned char materialFrame;
};
struct FaceTrackState {
    unsigned char remainingFrames;
    unsigned char keyframeIndex;
    unsigned char finished;
    unsigned char loop;
};
enum {
    kPrimaryFaceTrack = 0,
    kSecondaryFaceTrack = 1,
    kFirstExtraModelSlot = 5,
    kFacialModelKindCount = 8,
    kMenuFadeOutFrames = 15
};
struct FacePartStringPool {
    char faceParts[10];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char stringOutOfRange[27];
    char outOfRangeType[55];
    char stringLengthError[27];
};

// A tick is one UpdateTracks call, not elapsed seconds. Each channel owns its key vector.
// remainingFrames counts down only when frameHold is null; a zero-duration key stalls
// at that key. finished blocks subsequent ticks even if a countdown remains.
struct FaceAnimationState {
    unsigned char modelSlot;
    unsigned char alignmentPadding[3];
    unsigned int activeTrack;
    void* channel0Keys;
    unsigned int channel0Count;
    unsigned int channel0Capacity;
    void* channel1Keys;
    unsigned int channel1Count;
    unsigned int channel1Capacity;
    FaceTrackState tracks[2];
    void* frameHold[2];
    const char* vtable;
};

struct ResourcePair {
    void* first;
    void* second;
};
// Address-only view based on the original destructor-registration label.
// Each category has eight {primary, secondary} borrowed resource pointers;
// preserve that label as the relocation base rather than substituting adjacent globals.
struct FaceResourcePairRegion {
    unsigned char registrationPrefix[0x10];
    ResourcePair category0[kFacialModelKindCount];
    ResourcePair category1[kFacialModelKindCount];
};
enum FaceKeyVectorOffset {
    kPrimaryKeysOffset = (int)&((FaceAnimationState*)0)->channel0Keys,
    kSecondaryKeysOffset = (int)&((FaceAnimationState*)0)->channel1Keys
};
extern ResourcePair s_conteModelResourcePairs_Mode0[];
extern ResourcePair s_conteCategory1ResourcePairs[];
void ScriptByteMap_FreeSubtree(void*, void*);
int EventScript_RunAndSweep(void*);
void* memset(void*, int, unsigned int);
void* __nw__FUl(unsigned int);

struct CEventHandleVTableView {
    void* pad[2];
    void (*destroy)(void*, int);
};
extern CEventHandleVTableView __vt__12CEventHandle;
extern unsigned char facePartsStrings[];

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
};

struct EventScriptHeader {
    unsigned char signature[8];
    unsigned int relativeCodeOffset;
};
struct EventRuntimeVector {
    void* begin;
    unsigned int size;
    unsigned int capacity;
};
struct EventHandleStorage {
    unsigned char scriptRegisters[0x408];
    unsigned int nameFlags;
    unsigned int nameLength;
    void* nameData;
    void* argument;
    void* tree;
    unsigned int resourceSlots[6];
    EventRuntimeVector runtimeVectors[5];
    unsigned int byteMapCount;
    TreeNode* byteMapRoot;
    TreeNode* byteMapFirst;
    char effectState[0x14];
    unsigned int resourceMapCount;
    TreeNode* resourceMapRoot;
    TreeNode* resourceMapFirst;
    unsigned int completionState;
    unsigned int pendingState;
    void* eventVtable;
};
typedef char EventStorage_vectors_offset[((int)&((EventHandleStorage*)0)->runtimeVectors == 0x434) ? 1 : -1];
typedef char EventStorage_effect_offset[((int)&((EventHandleStorage*)0)->effectState == 0x47C) ? 1 : -1];
typedef char EventStorage_vtable_offset[((int)&((EventHandleStorage*)0)->eventVtable == 0x4A4) ? 1 : -1];

struct CEventHandleView {
    char scriptRegisters[0x408];
    char scriptName[0x2C];
    char runtimeVectors[0x3C];
    void* byteMapCount;
    void* byteMapRoot;
    void* byteMapFirst;
    char effectState[0x14];
    void* resourceMapCount;
    void* resourceMapRoot;
    void* resourceMapFirst;
    void* completionState;
    void* pendingState;
    void* vtable;
};
struct HandleBase8023 {
    char data[0x4A4];
};
struct HandleVirtual8023 : HandleBase8023 {
    virtual ~HandleVirtual8023();
};

extern float kFloat_6_2831855;
extern float s_twoPi_EventFace;
extern float s_twoPi_MenuProcess;
extern float s_twoPi_FadingMenuProcess;
extern unsigned char s_mpBase1Vtable[];
extern unsigned char s_fadeOverlay[];
extern signed char s_fadeOverlayInitialized;
extern unsigned char s_fadeOverlayDtorRegistration_8023F8DC[];
extern signed char s_bpPlayerInitialized;
extern unsigned char s_bpPlayer[];
extern unsigned char s_bpPlayerDtorRegistration[];
void __dt__9CBpPlayerFv();
void FadeOverlay_Init(void*);
void ResourceState_Dtor();
void FadeState_StartFadeOut(void*, int);
void FadeState_StartFadeIn(void*, int);
void FadeState_RequestFramebufferCapture(void*);
void* appMallocDummy__FUl(unsigned int);
void* __alloca(unsigned int);
void __dt__Q23std12length_errorFv();
extern unsigned char __vt__Q23std11logic_error[];
extern unsigned char __vt__Q23std12length_error[];
extern unsigned char s_sharedCharDeleterVtable[];

void __dt__Q33std6detail16char_array_guardFv(void*);
struct CStrRep {
    void* vt;
    int refs1;
    int refs2;
    char* data;
};
struct CStdString {
    char* data;
    CStrRep* rep;
};
struct CLengthError {
    void* vt;
    CStdString text;
};
struct ErrorAllocation8023 {
    char* data;
    void* allocator;
    // Delete an exception-message array that has not transferred ownership.
    ~ErrorAllocation8023() {
        if (data)
            __dla__FPv(data);
    }
};

#define THROW_TOO_LONG(m__)                                                       \
    do {                                                                          \
        const char* mptr__ = (const char*)(m__);                                  \
        CLengthError err__;                                                       \
        volatile unsigned char flag__ = 0;                                        \
        unsigned char dummy__ = 0;                                                \
        err__.vt = __vt__Q23std11logic_error;                                     \
        unsigned int n__ = strlen(mptr__) + 1;                                    \
        unsigned int n2__ = n__ + n__;                                            \
        char* buf__ = __nwa__FUl((void*)(n2__ - n__));                            \
        err__.text.data = buf__;                                                  \
        ErrorAllocation8023 tmp__ = {buf__, &dummy__};                            \
        CStrRep* rep__ = (CStrRep*)__nw__FUl(0x10);                               \
        if (rep__ != 0) {                                                         \
            rep__->refs1 = 1;                                                     \
            rep__->refs2 = 1;                                                     \
            rep__->vt = s_sharedCharDeleterVtable;                                \
            rep__->data = buf__;                                                  \
        }                                                                         \
        err__.text.rep = rep__;                                                   \
        tmp__.data = 0;                                                           \
        strcpy(buf__, mptr__);                                                    \
        err__.vt = __vt__Q23std12length_error;                                    \
        __throw(((FacePartStringPool*)facePartsStrings)->lengthErrorType, &err__, \
                (void*)__dt__Q23std12length_errorFv);                             \
    } while (0)

// Stored track header is ONLY begin/size/capacity (12 bytes, capacity in pairs).
// PairVector's trailing pcap/oldSize belong only to the temporary used while growing;
// never copy sizeof(PairVector) over a stored track. begin owns 2*capacity bytes.
struct PairVector {
    unsigned char* begin;
    unsigned int size;
    unsigned int capacity;
    unsigned int* pcap;
    unsigned int oldSize;
    // Initialize an empty temporary key-pair buffer.
    PairVector() : begin(0), size(0), capacity(0), pcap(0), oldSize(0) {}
    // Free the temporary buffer; after swap this is the old key storage.
    ~PairVector() {
        size = 0;
        if (begin)
            appFreeUni__FPv(begin);
    }
};

// Append to the selected header: fast path writes duration/frame and returns from
// the caller. Growth allocates -> appends -> copies old pairs backward -> swaps;
// tmp then releases only the old block. Explicit arguments replace caller-local capture.
#define PUSH_BACK(self, off, durationFrames, materialFrame)                             \
    do {                                                                                \
        PairVector* v__ = (PairVector*)((char*)self + (off));                           \
        unsigned int sz__ = v__->size;                                                  \
        unsigned int cp__ = v__->capacity;                                              \
        volatile unsigned char f1__ = 0;                                                \
        if (sz__ < cp__) {                                                              \
            unsigned int ns__ = sz__ + 1;                                               \
            unsigned char* e__ = v__->begin + (ns__ - 1) * 2;                           \
            v__->size = ns__;                                                           \
            e__[0] = durationFrames;                                                    \
            e__[1] = materialFrame;                                                     \
            return;                                                                     \
        }                                                                               \
        volatile unsigned char f2__ = 0;                                                \
        unsigned int rem__ = 0x7FFFFFFFu - cp__;                                        \
        volatile unsigned char f3__ = 0;                                                \
        if (rem__ < 1u)                                                                 \
            THROW_TOO_LONG(((FacePartStringPool*)facePartsStrings)->vectorLengthError); \
        unsigned int need__ = sz__ + 1 - cp__;                                          \
        PairVector tmp__;                                                               \
        tmp__.pcap = &v__->capacity;                                                    \
        if (need__ > 0x7FFFFFFFu - cp__)                                                \
            THROW_TOO_LONG(((FacePartStringPool*)facePartsStrings)->vectorLengthError); \
        unsigned int newCap__;                                                          \
        if (cp__ < 0x2AAAAAAA) {                                                        \
            unsigned int grown__ = ((cp__ + 1) * 4 - (cp__ + 1)) / 5;                   \
            unsigned int* pick__ = &grown__;                                            \
            if (grown__ < need__)                                                       \
                pick__ = &need__;                                                       \
            newCap__ = cp__ + *pick__;                                                  \
        } else if (cp__ < 0x55555554) {                                                 \
            unsigned int grown__ = (cp__ + 1) >> 1;                                     \
            unsigned int* pick__ = &grown__;                                            \
            if (grown__ < need__)                                                       \
                pick__ = &need__;                                                       \
            newCap__ = cp__ + *pick__;                                                  \
        } else {                                                                        \
            newCap__ = 0x7FFFFFFF;                                                      \
        }                                                                               \
        if (newCap__ > 0x7FFFFFFFu)                                                     \
            THROW_TOO_LONG(((FacePartStringPool*)facePartsStrings)->vectorLengthError); \
        tmp__.begin = (unsigned char*)appMallocDummy__FUl(newCap__ * 2);                \
        tmp__.capacity = newCap__;                                                      \
        tmp__.oldSize = v__->size;                                                      \
        unsigned char* dst__ = tmp__.begin + tmp__.oldSize * 2;                         \
        unsigned char* src__ = v__->begin + tmp__.oldSize * 2;                          \
        dst__[0] = durationFrames;                                                      \
        dst__[1] = materialFrame;                                                       \
        tmp__.size = 1;                                                                 \
        while (src__ > v__->begin) {                                                    \
            dst__ -= 2;                                                                 \
            src__ -= 2;                                                                 \
            if (dst__) {                                                                \
                dst__[0] = src__[0];                                                    \
                dst__[1] = src__[1];                                                    \
            }                                                                           \
            --tmp__.oldSize;                                                            \
            ++tmp__.size;                                                               \
        }                                                                               \
        unsigned char* oldBegin__ = v__->begin;                                         \
        unsigned int oldCap__ = v__->capacity;                                          \
        v__->begin = tmp__.begin;                                                       \
        v__->size = tmp__.size;                                                         \
        v__->capacity = tmp__.capacity;                                                 \
        tmp__.begin = oldBegin__;                                                       \
        tmp__.capacity = oldCap__;                                                      \
        tmp__.size = 0;                                                                 \
    } while (0)

struct FacialAnimationRange {
    char rangeHeader[0x30];
    unsigned short animationIndex;
};

struct ConteModelSlot {
    unsigned char kind;
    unsigned char alignmentPadding[3];
    void* model;
    void* animationSource;
    unsigned char modelStateStorage[0x38];
    union {
        unsigned int w;
        unsigned char b[4];
        struct {
            CW_BITS2(unsigned char heap : 1, unsigned char len : 7)
        } s;
    } materialName;
    unsigned int materialNameLength;
    void* materialNameData;
};

// ABI-only menu view: m2 initialize -> m3 start -> m5 process -> m6 result.
// m7 runs while waiting (including before each initialization attempt). m4 is the
// initialized-only hook used by MPBase1_DrawIfInitialized; its release name conflicts
// with draw callers, so its shared name is kept as-is.
// +0x10 reservedState aliases MPBase1Layout::unk_10: only cleared here; role unknown.
struct MenuProcessInterface {
    virtual void m0();
    virtual void m1();
    virtual int initialize();
    virtual void start();
    virtual void release();
    virtual void* process(float);
    virtual void onProcessResult(void*);
    virtual void onWaiting();
    int running;
    int initialized;
    int autoStart;
    int reservedState;
    unsigned char complete;
};

#define INSERT_FREE(N3, BASEP)                     \
    do {                                           \
        void* v = (void*)(N3)->left;               \
        if (v)                                     \
            ScriptByteMap_FreeSubtree((BASEP), v); \
        v = (void*)(N3)->right;                    \
        if (v)                                     \
            ScriptByteMap_FreeSubtree((BASEP), v); \
        appFreeUni__FPv(N3);                       \
    } while (0)

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Construct an event handle, run its byte-swapped script to completion, then destroy it.
void EventScript_RunBlocking(void* arg) {
    const char* str;
    TreeNode* tree;
    struct {
        char buf[8];
        unsigned int x;
    } local;
    local.x = ((EventScriptHeader*)arg)->relativeCodeOffset;
    unsigned char* p = (unsigned char*)&local.x;
    unsigned char d;
    unsigned char b;
    unsigned char a;
    unsigned char c;
    a = p[0];
    b = p[3];
    c = p[1];
    d = p[2];
    p[0] = b;
    p[3] = a;
    p[1] = d;
    p[2] = c;
    tree = (TreeNode*)((char*)arg + local.x);
    // Phase 1: resolve the byte-swapped relative script offset; allocate the owned handle.
    // arg and the resulting script pointer remain borrowed until the blocking run finishes.
    CEventHandle* self = (CEventHandle*)__nw__FUl(0x4A8);
    if (self) {
        str = (const char*)facePartsStrings;
        (*(const char**)&AsCEventHandle(self)->eventVtable) = (const char*)&__vt__12CEventHandle;
        (*(void**)&AsCEventHandle(self)->nameFlags) = 0;
        (*(void**)&AsCEventHandle(self)->nameLength) = 0;
        AsCEventHandle(self)->nameData = 0;
        ((EventHandleStorage*)self)->runtimeVectors[0].begin = 0;
        ((EventHandleStorage*)self)->runtimeVectors[0].size = 0;
        ((EventHandleStorage*)self)->runtimeVectors[0].capacity = 0;
        ((EventHandleStorage*)self)->runtimeVectors[1].begin = 0;
        ((EventHandleStorage*)self)->runtimeVectors[1].size = 0;
        ((EventHandleStorage*)self)->runtimeVectors[1].capacity = 0;
        ((EventHandleStorage*)self)->runtimeVectors[2].begin = 0;
        ((EventHandleStorage*)self)->runtimeVectors[2].size = 0;
        ((EventHandleStorage*)self)->runtimeVectors[2].capacity = 0;
        ((EventHandleStorage*)self)->runtimeVectors[3].begin = 0;
        ((EventHandleStorage*)self)->runtimeVectors[3].size = 0;
        ((EventHandleStorage*)self)->runtimeVectors[3].capacity = 0;
        ((EventHandleStorage*)self)->runtimeVectors[4].begin = 0;
        ((EventHandleStorage*)self)->runtimeVectors[4].size = 0;
        ((EventHandleStorage*)self)->runtimeVectors[4].capacity = 0;
        ((EventHandleStorage*)self)->byteMapCount = 0;
        ((EventHandleStorage*)self)->byteMapRoot = 0;
        ((CEventHandleView*)self)->byteMapFirst = (char*)&((CEventHandleView*)self)->byteMapRoot;
        ((CEventEffect*)(((EventHandleStorage*)self)->effectState))->Init();
        ((CEventHandleView*)self)->resourceMapCount = 0;
        ((CEventHandleView*)self)->resourceMapRoot = 0;
        ((CEventHandleView*)self)->resourceMapFirst = (char*)&((CEventHandleView*)self)->resourceMapRoot;
        int n;
        if ((AsCEventHandle(self)->nameFlags >> 31) == 0)
            n = (*(unsigned char*)&AsCEventHandle(self)->nameFlags) & 0x7f;
        else
            n = AsCEventHandle(self)->nameLength;
        local.buf[4] = local.buf[0];
        String_ReplaceRange(&AsCEventHandle(self)->nameFlags, 0, n, (char*)str, (char*)str + strlen(str),
                            &local.buf[4]);
        AsCEventHandle(self)->argument = arg;
        AsCEventHandle(self)->tree = tree;
        // Phase 2: attach the borrowed script and clear registers, slots and runtime counts.
        // Vector blocks are retained; erasing their counts does not destroy pointed-to resources.
        memset(self, 0, 8);
        memset(&((EventHandleStorage*)self)->scriptRegisters[8], 0, 0x400);
        for (int o = 0; o <= 5 * 4; o += 4)
            *(unsigned int*)((char*)((EventHandleStorage*)self)->resourceSlots + o) = 0;
        {
            int c = (*(int*)&((EventHandleStorage*)self)->runtimeVectors[0].size);
            ((EventHandleStorage*)self)->runtimeVectors[0].size =
                c - ((EventHandleStorage*)self)->runtimeVectors[0].size;
        }
        {
            int c = (*(int*)&((EventHandleStorage*)self)->runtimeVectors[1].size);
            ((EventHandleStorage*)self)->runtimeVectors[1].size =
                c - ((EventHandleStorage*)self)->runtimeVectors[1].size;
        }
        {
            int c = (*(int*)&((EventHandleStorage*)self)->runtimeVectors[2].size);
            ((EventHandleStorage*)self)->runtimeVectors[2].size =
                c - ((EventHandleStorage*)self)->runtimeVectors[2].size;
        }
        {
            int c = (*(int*)&((EventHandleStorage*)self)->runtimeVectors[3].size);
            ((EventHandleStorage*)self)->runtimeVectors[3].size =
                c - ((EventHandleStorage*)self)->runtimeVectors[3].size;
        }
        {
            int c = (*(int*)&((EventHandleStorage*)self)->runtimeVectors[4].size);
            ((EventHandleStorage*)self)->runtimeVectors[4].size =
                c - ((EventHandleStorage*)self)->runtimeVectors[4].size;
        }
        {
            // Phase 3: clear the two node-owning maps in postorder, then reset their sentinels.
            // INSERT_FREE releases descendants via ScriptByteMap_FreeSubtree before each node.
            TreeNode* n3;
            TreeNode* root;
            TreeNode* n1;
            TreeNode* n2;
            root = (TreeNode*)((CEventHandleView*)self)->byteMapRoot;
            if (root) {
                n1 = root->left;
                if (n1) {
                    n2 = n1->left;
                    if (n2) {
                        n3 = n2->left;
                        if (n3)
                            INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->byteMapCount);
                        n3 = n2->right;
                        if (n3)
                            INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->byteMapCount);
                        appFreeUni__FPv(n2);
                    }
                    n3 = n1->right;
                    if (n3) {
                        n2 = n3->left;
                        if (n2)
                            INSERT_FREE(n2, (char*)&((CEventHandleView*)self)->byteMapCount);
                        n2 = n3->right;
                        if (n2)
                            INSERT_FREE(n2, (char*)&((CEventHandleView*)self)->byteMapCount);
                        appFreeUni__FPv(n3);
                    }
                    appFreeUni__FPv(n1);
                }
                n3 = root->right;
                if (n3) {
                    n1 = n3->left;
                    if (n1) {
                        n2 = n1->left;
                        if (n2)
                            INSERT_FREE(n2, (char*)&((CEventHandleView*)self)->byteMapCount);
                        n2 = n1->right;
                        if (n2)
                            INSERT_FREE(n2, (char*)&((CEventHandleView*)self)->byteMapCount);
                        appFreeUni__FPv(n1);
                    }
                    n1 = n3->right;
                    if (n1) {
                        n2 = n1->left;
                        if (n2)
                            INSERT_FREE(n2, (char*)&((CEventHandleView*)self)->byteMapCount);
                        n2 = n1->right;
                        if (n2)
                            INSERT_FREE(n2, (char*)&((CEventHandleView*)self)->byteMapCount);
                        appFreeUni__FPv(n1);
                    }
                    appFreeUni__FPv(n3);
                }
                appFreeUni__FPv(root);
                ((CEventHandleView*)self)->byteMapCount = 0;
                ((CEventHandleView*)self)->byteMapRoot = 0;
                ((CEventHandleView*)self)->byteMapFirst = (char*)&((CEventHandleView*)self)->byteMapRoot;
            }
            (*(unsigned int*)&AsCEventHandle(self)->pendingResource /* embedded effect state */) = 0;
        }
        TreeNode* root;
        TreeNode* n1;
        TreeNode* n2;
        TreeNode* n3;
        root = (TreeNode*)((CEventHandleView*)self)->resourceMapRoot;
        if (root) {
            n1 = root->left;
            if (n1) {
                n2 = n1->left;
                if (n2) {
                    n3 = n2->left;
                    if (n3)
                        INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->resourceMapCount);
                    n3 = n2->right;
                    if (n3)
                        INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->resourceMapCount);
                    appFreeUni__FPv(n2);
                }
                n2 = n1->right;
                if (n2) {
                    n3 = n2->left;
                    if (n3)
                        INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->resourceMapCount);
                    n3 = n2->right;
                    if (n3)
                        INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->resourceMapCount);
                    appFreeUni__FPv(n2);
                }
                appFreeUni__FPv(n1);
            }
            n1 = root->right;
            if (n1) {
                n2 = n1->left;
                if (n2) {
                    n3 = n2->left;
                    if (n3)
                        INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->resourceMapCount);
                    n3 = n2->right;
                    if (n3)
                        INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->resourceMapCount);
                    appFreeUni__FPv(n2);
                }
                n2 = n1->right;
                if (n2) {
                    n3 = n2->left;
                    if (n3)
                        INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->resourceMapCount);
                    n3 = n2->right;
                    if (n3)
                        INSERT_FREE(n3, (char*)&((CEventHandleView*)self)->resourceMapCount);
                    appFreeUni__FPv(n2);
                }
                appFreeUni__FPv(n1);
            }
            appFreeUni__FPv(root);
            ((CEventHandleView*)self)->resourceMapCount = 0;
            ((CEventHandleView*)self)->resourceMapRoot = 0;
            ((CEventHandleView*)self)->resourceMapFirst = (char*)&((CEventHandleView*)self)->resourceMapRoot;
        }
        ((EventHandleStorage*)self)->completionState = 0;
        ((EventHandleStorage*)self)->pendingState = 0;
    }
    // Phase 4: synchronously run/sweep to completion, then destroy the temporary handle.
    // No yielding or ownership transfer of arg occurs here.
    while (!EventScript_RunAndSweep(self)) {
    }
    if (self)
        delete (HandleVirtual8023*)self;
}

#pragma pop

// Initialize empty facial keyframe tracks and clear their playback flags.
CEventFace::CEventFace() {
    FaceAnimationState* self = (FaceAnimationState*)this;
    self->channel0Keys = 0;
    self->vtable = __vt__10CEventFace;
    self->channel0Capacity = 0;
    self->channel1Keys = 0;
    self->channel1Capacity = 0;
    self->tracks[0].finished = 0;
    self->tracks[0].keyframeIndex = 0;
    self->tracks[0].remainingFrames = 0;
    self->tracks[0].loop = 0;
    self->tracks[1].finished = 0;
    self->tracks[1].keyframeIndex = 0;
    self->tracks[1].remainingFrames = 0;
    self->tracks[1].loop = 0;
    self->channel0Count = 0;
    self->channel1Count = 0;
}

// Free both facial keyframe vectors and optionally delete the controller.
CEventFace::~CEventFace() {
    FaceAnimationState* self = (FaceAnimationState*)this;
    int c0 = *(int*)&self->channel0Count;
    int c1 = *(int*)&self->channel1Count;
    self->vtable = __vt__10CEventFace;
    self->channel0Count = c0 - *(unsigned int*)&self->channel0Count;
    self->channel1Count = c1 - *(unsigned int*)&self->channel1Count;
    if (&self->channel1Keys != 0 && &self->channel1Keys != 0 && &self->channel1Keys != 0 && &self->channel1Keys != 0 &&
        self->channel1Keys != 0) {
        int c3 = *(int*)&self->channel1Count;
        self->channel1Count = c3 - *(unsigned int*)&self->channel1Count;
        appFreeUni__FPv(self->channel1Keys);
    }
    if (&self->channel0Keys != 0 && &self->channel0Keys != 0 && &self->channel0Keys != 0 && &self->channel0Keys != 0 &&
        self->channel0Keys != 0) {
        int c2 = *(int*)&self->channel0Count;
        self->channel0Count = c2 - *(unsigned int*)&self->channel0Count;
        appFreeUni__FPv(self->channel0Keys);
    }
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_lifetimes off
// Resolve MT_FACIAL_ANIMATION and cache its material animation name/value.
static inline void* configureEventSlot(ConteModelSlot* obj) {
    unsigned int size;
    void* node;
    char* str;
    char* copy;
    char buf[12];
    unsigned int length;
    void* res;

    if ((obj->materialName.w >> 31) == 0) {
        obj->materialName.b[1] = 0;
        obj->materialName.s.len = 0;
    } else {
        *(unsigned char*)obj->materialNameData = 0;
        obj->materialNameLength = 0;
    }
    if (obj->model == 0 || obj->animationSource == 0 || obj->kind >= kFacialModelKindCount) {
        res = 0;
    } else {
        node = WPuppet_FindAnimationRangeByName(obj->animationSource, kFacialMaterialAnimationTag);
        if (node == 0) {
            res = 0;
        } else {
            str = (char*)WPuppet_GetAnimationNameSize(obj->animationSource,
                                                      ((FacialAnimationRange*)node)->animationIndex);
            copy = __nwa__FUl(str);
            WPuppet_CopyAnimationName(obj->animationSource, copy, ((FacialAnimationRange*)node)->animationIndex, str);
            if ((obj->materialName.w >> 31) == 0) {
                size = obj->materialName.b[0] & 0x7F;
            } else {
                size = obj->materialNameLength;
            }
            buf[4] = buf[0];
            length = strlen(copy);
            String_ReplaceRange(&obj->materialName.b[0], 0, size, copy, copy + length, &buf[4]);
            if (copy != 0)
                __dla__FPv(copy);
            res = WPuppet_GetAnimationValueByName(obj->animationSource, &buf[8],
                                                  ((obj->materialName.w >> 31) == 0) ? (void*)&obj->materialName.b[1]
                                                                                     : obj->materialNameData);
        }
    }
    return res;
}

// Select a model slot and facial track, reset playback, and set its loop byte.
void CEventFace::SelectModelAndTrack(unsigned char* input) {
    FaceAnimationState* self = (FaceAnimationState*)this;
    ConteModelSlot* obj;
    unsigned int trackIndex;
    unsigned char zero;

    // Layout of `input`: [1] = model slot, [2] = track number (0 or non-zero = second), [3] = loop byte.
    // Phase 1: make sure the global model-slot table (GetConteModelSlotsSlot) exists; the comma expression below only
    // forces the slot byte to be read before the table pointer, as in the original code.
    if (GetConteModelSlotsSlot() == 0) {
        if (s_courseSlotTableInitialized == 0) {
            CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
    }
    unsigned char requestedSlot = input[1];
    void* tbl = (requestedSlot, GetConteModelSlotsSlot());
    self->modelSlot = requestedSlot;
    obj = (ConteModelSlot*)SlotTable_GetEntry(tbl, input[1]);
    if (obj == 0 || configureEventSlot(obj) == 0)
        self->modelSlot = kFirstExtraModelSlot;
    // Phase 2: pick the track and reset its keyframe state.
    trackIndex = input[2];
    self->activeTrack = trackIndex;
    if (trackIndex == 0) {
        int oldCount = *(int*)&self->channel0Count;
        zero = 0;
        self->channel0Count = oldCount - *(unsigned int*)&self->channel0Count;
        unsigned char loopByte = input[3];
        self->tracks[0].finished = zero;
        self->tracks[0].keyframeIndex = zero;
        self->tracks[0].remainingFrames = zero;
        self->tracks[0].loop = loopByte;
    } else {
        int oldCount = *(int*)&self->channel1Count;
        zero = 0;
        self->channel1Count = oldCount - *(unsigned int*)&self->channel1Count;
        unsigned char loopByte = input[3];
        self->tracks[1].finished = zero;
        self->tracks[1].keyframeIndex = zero;
        self->tracks[1].remainingFrames = zero;
        self->tracks[1].loop = loopByte;
    }
    self->frameHold[0] = 0;
    self->frameHold[1] = 0;
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma pop

// Apply a track's final material frame, then clear its queued keyframes.
void CEventFace::ApplyLastFrameAndClearTrack(unsigned char index) {
    FaceAnimationState* self = (FaceAnimationState*)this;
    unsigned int offset = (unsigned int)index << 2;
    (*(unsigned int*)((char*)self->frameHold + offset)) = 0;
    if (index == 0) {
        unsigned int count = self->channel0Count;
        if (count != 0 && self->tracks[0].finished == 0) {
            unsigned char type = ((FaceKeyframe*)self->channel0Keys)[count - 1].materialFrame;
            if (GetConteModelSlotsSlot() == 0) {
                if (s_courseSlotTableInitialized == 0) {
                    CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
                    s_courseSlotTableInitialized = 1;
                }
                GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
            }
            void* obj = SlotTable_GetEntry(GetConteModelSlotsSlot(), self->modelSlot);
            if (obj != 0) {
                void* selected = 0;
                if (self->modelSlot < kFirstExtraModelSlot) {
                    if (GetConteModelSlotsSlot() == 0) {
                        if (s_courseSlotTableInitialized == 0) {
                            CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
                            s_courseSlotTableInitialized = 1;
                        }
                        GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
                    }
                    void* cached = GetConteModelSlotsSlot();
                    unsigned char* second = (unsigned char*)SlotTable_GetEntry(cached, self->modelSlot);
                    if (second != 0) {
                        switch (CourseSlotTable_IsExtraSlotRange(cached, self->modelSlot)) {
                        case 0:
                            selected = s_conteModelResourcePairs_Mode0[*second].first;
                            break;
                        case 1:
                            selected = s_conteCategory1ResourcePairs[*second].first;
                            break;
                        }
                    }
                }
                if (((ConteModelSlot*)obj)->model != 0 && ((ConteModelSlot*)obj)->animationSource != 0) {
                    int isByte = !(((ConteModelSlot*)obj)->materialName.w >> 31);
                    unsigned int size = isByte ? (((ConteModelSlot*)obj)->materialName.b[0] & 0x7f)
                                               : ((ConteModelSlot*)obj)->materialNameLength;
                    if (size != 0) {
                        void* data = isByte ? (void*)&((ConteModelSlot*)obj)->materialName.b[1]
                                            : ((ConteModelSlot*)obj)->materialNameData;
                        float f = (float)type;
                        WPuppet_ApplyNamedMaterialLayerAnimation(
                            ((ConteModelSlot*)obj)->model, ((ConteModelSlot*)obj)->animationSource, data, selected, f);
                    }
                }
            }
        }
        int c = *(int*)&self->channel0Count;
        self->channel0Count = c - *(unsigned int*)&self->channel0Count;
    } else {
        unsigned int count = self->channel1Count;
        if (count != 0 && self->tracks[1].finished == 0) {
            unsigned char type = ((FaceKeyframe*)self->channel1Keys)[count - 1].materialFrame;
            if (GetConteModelSlotsSlot() == 0) {
                if (s_courseSlotTableInitialized == 0) {
                    CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
                    s_courseSlotTableInitialized = 1;
                }
                GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
            }
            void* obj = SlotTable_GetEntry(GetConteModelSlotsSlot(), self->modelSlot);
            if (obj != 0) {
                void* selected = (void*)1;
                if (self->modelSlot < kFirstExtraModelSlot) {
                    if (GetConteModelSlotsSlot() == 0) {
                        if (s_courseSlotTableInitialized == 0) {
                            CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
                            s_courseSlotTableInitialized = 1;
                        }
                        GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
                    }
                    void* cached = GetConteModelSlotsSlot();
                    unsigned char* second = (unsigned char*)SlotTable_GetEntry(cached, self->modelSlot);
                    if (second != 0) {
                        switch (CourseSlotTable_IsExtraSlotRange(cached, self->modelSlot)) {
                        case 0:
                            selected = s_conteModelResourcePairs_Mode0[*second].second;
                            break;
                        case 1:
                            selected = s_conteCategory1ResourcePairs[*second].second;
                            break;
                        }
                    }
                }
                if (((ConteModelSlot*)obj)->model != 0 && ((ConteModelSlot*)obj)->animationSource != 0) {
                    int isByte = !(((ConteModelSlot*)obj)->materialName.w >> 31);
                    unsigned int size = isByte ? (((ConteModelSlot*)obj)->materialName.b[0] & 0x7f)
                                               : ((ConteModelSlot*)obj)->materialNameLength;
                    if (size != 0) {
                        void* data = isByte ? (void*)&((ConteModelSlot*)obj)->materialName.b[1]
                                            : ((ConteModelSlot*)obj)->materialNameData;
                        float f = (float)type;
                        WPuppet_ApplyNamedMaterialLayerAnimation(
                            ((ConteModelSlot*)obj)->model, ((ConteModelSlot*)obj)->animationSource, data, selected, f);
                    }
                }
            }
        }
        int c = *(int*)&self->channel1Count;
        self->channel1Count = c - *(unsigned int*)&self->channel1Count;
    }
}

// Set a track's frame-hold pointer and apply its last queued material frame.
// index must be 0 or 1: frameHold is indexed before the primary/secondary branch.
// value is borrowed and tested only for null; it is neither retained nor freed.
// Clearing a hold returns immediately. A nonnull hold pauses countdown but can apply
// the last queued frame now; no queued keys are removed by this function.
void CEventFace::SetTrackFrameHold(unsigned char index, void* value) {
    FaceAnimationState* self = (FaceAnimationState*)this;
    self->frameHold[index] = value;
    if (value == 0)
        return;

    if (index == 0) {
        unsigned int count = self->channel0Count;
        if (count == 0)
            return;
        unsigned char mode = self->tracks[0].finished;
        if (mode != 0)
            return;

        unsigned char materialFrame = ((FaceKeyframe*)self->channel0Keys)[count - 1].materialFrame;

        if (GetConteModelSlotsSlot() == 0) {
            if (s_courseSlotTableInitialized == 0) {
                CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
                s_courseSlotTableInitialized = 1;
            }
            GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
        }
        void* obj = SlotTable_GetEntry(GetConteModelSlotsSlot(), self->modelSlot);
        if (obj == 0)
            return;

        void* selectedResource = 0;
        if (self->modelSlot < kFirstExtraModelSlot) {
            if (GetConteModelSlotsSlot() == 0) {
                if (s_courseSlotTableInitialized == 0) {
                    CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
                    s_courseSlotTableInitialized = 1;
                }
                GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
            }
            void* cached = GetConteModelSlotsSlot();
            unsigned char* resourceSlot = (unsigned char*)SlotTable_GetEntry(cached, self->modelSlot);
            if (resourceSlot != 0) {
                switch (CourseSlotTable_IsExtraSlotRange(cached, self->modelSlot)) {
                case 0:
                    selectedResource = s_conteModelResourcePairs_Mode0[*resourceSlot].first;
                    break;
                case 1:
                    selectedResource = s_conteCategory1ResourcePairs[*resourceSlot].first;
                    break;
                }
            }
        }

        void* model = ((ConteModelSlot*)obj)->model;
        if (model == 0)
            return;
        void* animationSource = ((ConteModelSlot*)obj)->animationSource;
        if (animationSource == 0)
            return;
        unsigned int flags = ((ConteModelSlot*)obj)->materialName.w;
        int inlineMaterialName = !(flags >> 31);
        unsigned int materialNameLength = inlineMaterialName ? (((ConteModelSlot*)obj)->materialName.b[0] & 0x7f)
                                                             : ((ConteModelSlot*)obj)->materialNameLength;
        if (materialNameLength == 0)
            return;
        void* materialName = inlineMaterialName ? (void*)&((ConteModelSlot*)obj)->materialName.b[1]
                                                : ((ConteModelSlot*)obj)->materialNameData;
        float frameValue = (float)materialFrame;
        WPuppet_ApplyNamedMaterialLayerAnimation(model, animationSource, materialName, selectedResource, frameValue);
    } else {
        unsigned int count = self->channel1Count;
        if (count == 0)
            return;
        unsigned char mode = self->tracks[1].finished;
        if (mode != 0)
            return;

        unsigned char materialFrame = ((FaceKeyframe*)self->channel1Keys)[count - 1].materialFrame;

        if (GetConteModelSlotsSlot() == 0) {
            if (s_courseSlotTableInitialized == 0) {
                CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
                s_courseSlotTableInitialized = 1;
            }
            GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
        }
        void* obj = SlotTable_GetEntry(GetConteModelSlotsSlot(), self->modelSlot);
        if (obj == 0)
            return;

        void* selectedResource = (void*)1;
        if (self->modelSlot < kFirstExtraModelSlot) {
            if (GetConteModelSlotsSlot() == 0) {
                if (s_courseSlotTableInitialized == 0) {
                    CourseSlotTable_InitEmpty((void*)s_conteModelSlots);
                    s_courseSlotTableInitialized = 1;
                }
                GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
            }
            void* cached = GetConteModelSlotsSlot();
            unsigned char* resourceSlot = (unsigned char*)SlotTable_GetEntry(cached, self->modelSlot);
            if (resourceSlot != 0) {
                switch (CourseSlotTable_IsExtraSlotRange(cached, self->modelSlot)) {
                case 0:
                    selectedResource = s_conteModelResourcePairs_Mode0[*resourceSlot].second;
                    break;
                case 1:
                    selectedResource = s_conteCategory1ResourcePairs[*resourceSlot].second;
                    break;
                }
            }
        }

        void* model = ((ConteModelSlot*)obj)->model;
        if (model == 0)
            return;
        void* animationSource = ((ConteModelSlot*)obj)->animationSource;
        if (animationSource == 0)
            return;
        unsigned int flags = ((ConteModelSlot*)obj)->materialName.w;
        int inlineMaterialName = !(flags >> 31);
        unsigned int materialNameLength = inlineMaterialName ? (((ConteModelSlot*)obj)->materialName.b[0] & 0x7f)
                                                             : ((ConteModelSlot*)obj)->materialNameLength;
        if (materialNameLength == 0)
            return;
        void* materialName = inlineMaterialName ? (void*)&((ConteModelSlot*)obj)->materialName.b[1]
                                                : ((ConteModelSlot*)obj)->materialNameData;
        float frameValue = (float)materialFrame;
        WPuppet_ApplyNamedMaterialLayerAnimation(model, animationSource, materialName, selectedResource, frameValue);
    }
}

// Append a duration/material-frame byte pair to the currently selected facial track.
// The receiver is const here: the original compiles this way and the non-const form scores lower.
void CEventFace::AppendKeyframe(unsigned char* value) const {
    __alloca(0);
    // Input byte 0 is ignored here; input order is material frame, THEN duration.
    // Stored FaceKeyframe order is duration, material frame. No pointer is retained.
    unsigned char materialFrame = value[1];
    unsigned char durationFrames = value[2];
    if (((const FaceAnimationState*)this)->activeTrack == kPrimaryFaceTrack) {
        PUSH_BACK(this, kPrimaryKeysOffset, durationFrames, materialFrame);
    } else {
        PUSH_BACK(this, kSecondaryKeysOffset, durationFrames, materialFrame);
    }
}

// Tick both facial tracks, apply material frames, and handle looping and held countdowns.
#define FACE_PAIRS0 (((FaceResourcePairRegion*)resourcePairStorage)->category0)
#define FACE_PAIRS1 (((FaceResourcePairRegion*)resourcePairStorage)->category1)
void CEventFace::UpdateTracks() {
    FaceAnimationState* face = (FaceAnimationState*)this;
    // One linked data block is addressed from its destructor-registration prefix.
    // Offsets +0x10/+0x50 start category 0/1 arrays of eight pointer pairs (8 bytes
    // each); +0/+4 within model pair selects the primary/secondary animation resource.
    // Do not change the relocation base to the neighboring table symbols.
    // ConteModelSlot.materialName is MW small-string storage: high bit clear means
    // length in byte 0 (low 7 bits), text at byte 1; otherwise length/data are external.
    unsigned char* resourcePairStorage = s_bpPlayerDtorRegistration;
    // Per-track phases: latch key -> resolve model -> select resource -> apply frame ->
    // advance/loop/finish -> decrement unless held. Missing resources still advance;
    // zero-duration keys do not advance. The two tracks differ at end-of-track and
    // in their default resource word (primary 0, secondary 1); do not merge those cases.
    // Primary phase: latch duration/frame, resolve the borrowed puppet/resource,
    // apply the frame, then advance/loop/finish. Missing resources still advance.
    if ((*(int*)&face->channel0Count) != 0 && !face->tracks[0].finished) {
        if (!face->tracks[0].remainingFrames) {
            FaceKeyframe* keyframe = (FaceKeyframe*)face->channel0Keys;
            keyframe += face->tracks[0].keyframeIndex;
            unsigned char durationFrames = keyframe->durationFrames;
            if (durationFrames) {
                face->tracks[0].remainingFrames = durationFrames;
                unsigned char materialFrame = keyframe->materialFrame;
                if (!((int&)GetConteModelSlotsSlot())) {
                    if (!s_courseSlotTableInitialized) {
                        CourseSlotTable_InitEmpty(s_conteModelSlots);
                        s_courseSlotTableInitialized = 1;
                    }
                    ((int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
                }
                void* entry = SlotTable_GetEntry((void*)((int&)GetConteModelSlotsSlot()), face->modelSlot);
                if (entry) {
                    int selectedResourceWord = 0;
                    if (face->modelSlot < kFirstExtraModelSlot) {
                        if (!((int&)GetConteModelSlotsSlot())) {
                            if (!s_courseSlotTableInitialized) {
                                CourseSlotTable_InitEmpty(s_conteModelSlots);
                                s_courseSlotTableInitialized = 1;
                            }
                            ((int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
                        }
                        void* manager = (void*)((int&)GetConteModelSlotsSlot());
                        void* resourceSlot = SlotTable_GetEntry(manager, face->modelSlot);
                        if (resourceSlot) {
                            unsigned char resourceCategory =
                                (unsigned char)CourseSlotTable_IsExtraSlotRange(manager, face->modelSlot);
                            switch (resourceCategory) {
                            case 0:
                                selectedResourceWord = (int)FACE_PAIRS0[((ConteModelSlot*)resourceSlot)->kind].first;
                                break;
                            case 1:
                                selectedResourceWord = (int)FACE_PAIRS1[((ConteModelSlot*)resourceSlot)->kind].first;
                                break;
                            }
                        }
                    }
                    void* model = ((ConteModelSlot*)entry)->model;
                    void* animationSource = ((ConteModelSlot*)entry)->animationSource;
                    int inlineMaterialName = ((*(int*)&((ConteModelSlot*)entry)->materialName.w) >= 0);
                    unsigned int materialNameLength = inlineMaterialName
                                                          ? (((ConteModelSlot*)entry)->materialName.b[0] & 0x7F)
                                                          : ((ConteModelSlot*)entry)->materialNameLength;
                    if (model && animationSource && materialNameLength) {
                        void* materialName = inlineMaterialName ? (void*)&((ConteModelSlot*)entry)->materialName.b[1]
                                                                : ((ConteModelSlot*)entry)->materialNameData;
                        ((void (*)(void*, void*, float, void*, void*))WPuppet_ApplyNamedMaterialLayerAnimation)(
                            model, animationSource, (float)(int)materialFrame, materialName,
                            (void*)selectedResourceWord);
                    }
                }
                if ((unsigned int)face->tracks[0].keyframeIndex < (unsigned int)((*(int*)&face->channel0Count) - 1))
                    face->tracks[0].keyframeIndex++;
                else if (face->tracks[0].loop)
                    face->tracks[0].keyframeIndex = 0;
                else
                    face->tracks[0].finished = 1;
            }
        }
        if (face->tracks[0].remainingFrames && !(*(int*)&face->frameHold[0]))
            face->tracks[0].remainingFrames--;
    }
    // Secondary phase follows the same rules but uses pair.second. On model non-looping
    // final key, the BP player model slot restarts this channel; other slots finish.
    if ((*(int*)&face->channel1Count) && !face->tracks[1].finished) {
        if (!face->tracks[1].remainingFrames) {
            FaceKeyframe* keyframe = (FaceKeyframe*)face->channel1Keys;
            keyframe += face->tracks[1].keyframeIndex;
            unsigned char durationFrames = keyframe->durationFrames;
            if (durationFrames) {
                face->tracks[1].remainingFrames = durationFrames;
                unsigned char materialFrame = keyframe->materialFrame;
                if (!((int&)GetConteModelSlotsSlot())) {
                    if (!s_courseSlotTableInitialized) {
                        CourseSlotTable_InitEmpty(s_conteModelSlots);
                        s_courseSlotTableInitialized = 1;
                    }
                    ((int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
                }
                void* entry = SlotTable_GetEntry((void*)((int&)GetConteModelSlotsSlot()), face->modelSlot);
                if (entry) {
                    int selectedResourceWord = 1;
                    if (face->modelSlot < kFirstExtraModelSlot) {
                        if (!((int&)GetConteModelSlotsSlot())) {
                            if (!s_courseSlotTableInitialized) {
                                CourseSlotTable_InitEmpty(s_conteModelSlots);
                                s_courseSlotTableInitialized = 1;
                            }
                            ((int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
                        }
                        void* manager = (void*)((int&)GetConteModelSlotsSlot());
                        void* resourceSlot = SlotTable_GetEntry(manager, face->modelSlot);
                        if (resourceSlot) {
                            unsigned char resourceCategory =
                                (unsigned char)CourseSlotTable_IsExtraSlotRange(manager, face->modelSlot);
                            switch (resourceCategory) {
                            case 0:
                                selectedResourceWord = (int)FACE_PAIRS0[((ConteModelSlot*)resourceSlot)->kind].second;
                                break;
                            case 1:
                                selectedResourceWord = (int)FACE_PAIRS1[((ConteModelSlot*)resourceSlot)->kind].second;
                                break;
                            case 5:
                                break;
                            }
                        }
                    }
                    void* model = ((ConteModelSlot*)entry)->model;
                    void* animationSource = ((ConteModelSlot*)entry)->animationSource;
                    int inlineMaterialName = ((*(int*)&((ConteModelSlot*)entry)->materialName.w) >= 0);
                    unsigned int materialNameLength = inlineMaterialName
                                                          ? (((ConteModelSlot*)entry)->materialName.b[0] & 0x7F)
                                                          : ((ConteModelSlot*)entry)->materialNameLength;
                    if (model && animationSource && materialNameLength) {
                        void* materialName = inlineMaterialName ? (void*)&((ConteModelSlot*)entry)->materialName.b[1]
                                                                : ((ConteModelSlot*)entry)->materialNameData;
                        ((void (*)(void*, void*, float, void*, void*))WPuppet_ApplyNamedMaterialLayerAnimation)(
                            model, animationSource, (float)(int)materialFrame, materialName,
                            (void*)selectedResourceWord);
                    }
                }
                if ((unsigned int)face->tracks[1].keyframeIndex < (unsigned int)((*(int*)&face->channel1Count) - 1))
                    face->tracks[1].keyframeIndex++;
                else if (face->tracks[1].loop)
                    face->tracks[1].keyframeIndex = 0;
                else {
                    if (!((int&)Sing_805DC670())) {
                        if (!s_bpPlayerInitialized) {
                            ((CBpPlayer*)s_bpPlayer)->__ct();
                            __register_global_object(s_bpPlayer, (void*)__dt__9CBpPlayerFv, resourcePairStorage);
                            s_bpPlayerInitialized = 1;
                        }
                        ((int&)Sing_805DC670()) = (int)s_bpPlayer;
                    }
                    if (face->modelSlot == *(int*)((char*)((int&)Sing_805DC670()) + 0x28))
                        face->tracks[1].keyframeIndex = 0;
                    else
                        face->tracks[1].finished = 1;
                }
            }
        }
        if (face->tracks[1].remainingFrames && !(*(int*)&face->frameHold[1]))
            face->tracks[1].remainingFrames--;
    }
}

// Release a byte-pair vector's storage and optionally free its wrapper.
void* dtor_802418E4(void* self, int doDelete) {
    if (self != 0) {
        register void* allocation;
        *((int*)self + 1) = 0;
        if (self != 0) {
            allocation = *(void**)self;
            if (allocation != 0) {
                *((int*)self + 1) = 0;
                appFreeUni__FPv(allocation);
            }
        }
        if (doDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Initialize the facial-animation unit's mutable two-pi constant.
void CEventFace_InitializeUnitConstants() {
    s_twoPi_EventFace = kFloat_6_2831855;
}

// Initialize a stopped menu process that starts automatically after initialization.
void MPBase1_Ctor(void* self) {
    AsMPBase1(self)->vtable = (void*)s_mpBase1Vtable;
    AsMPBase1(self)->running = 0;
    AsMPBase1(self)->initialized = 0;
    AsMPBase1(self)->autoStart = 1;
    AsMPBase1(self)->unk_10 = 0;
    AsMPBase1(self)->complete = 0;
}

// Start the menu once and clear its previous completion result.
void MPBase1_Start(MenuProcessInterface* self) {
    self->complete = 0;
    if (self->running == 0) {
        self->start();
        self->running = 1;
    }
}

// Default menu start hook; subclasses supply behavior.
void IMenuBase_vfunc_3() {}

// Initialize, optionally start, and process the menu until it reports completion.
// Returns 0 while waiting; running process results are forwarded as an int, and
// any nonnull result marks completion. Later calls return 1. dt is forwarded unchanged.
// Process result pointers are notifications only here; this wrapper never frees them.
int MPBase1_Process(MenuProcessInterface* self, float dt) {
    // Already-complete calls skip every virtual hook.
    if (self->complete)
        return 1;
    if (self->initialized && self->running) {
        void* result = self->process(dt);
        if (result) {
            self->complete = 1;
            self->running = 0;
        }
        self->onProcessResult(result);
        return (int)result;
    }
    // Waiting phase: try initialization again until it succeeds; autoStart gates m3.
    self->onWaiting();
    if (self->initialized == 0) {
        if (self->initialize()) {
            int one = 1;
            self->initialized = one;
            if (self->autoStart) {
                self->complete = 0;
                if (self->running == 0) {
                    self->start();
                    self->running = one;
                }
            }
        }
    }
    return 0;
}

// Default menu process-result hook.
void IMenuBase_vfunc_6() {}

// Default hook while waiting for initialization or start.
void IMenuBase_vfunc_7() {}

// Default menu initialization succeeds immediately.
int IMenuBase_PollLoadComplete_Default() {
    return 1;
}

// Choose whether successful initialization starts the menu automatically.
void MPBase1_SetAutoStart(void* self, int value) {
    AsMPBase1(self)->autoStart = value;
}

// Invoke virtual slot m4 only after initialization (draw/release naming is unresolved).
void MPBase1_DrawIfInitialized(void* self) {
    if (AsMPBase1(self)->initialized != 0) {
        // Legacy release name denotes slot 4 only. This wrapper transfers no ownership;
        // call sites occur in display paths, but the slot contract still needs confirmation.
        ((MenuProcessInterface*)self)->release();
    }
}

// Default menu availability/status hook returns false.
int IMenuBase_vfunc_1() {
    return 0;
}

// Return the menu completion flag.
unsigned char MPBase1_IsComplete(void* self) {
    return AsMPBase1(self)->complete;
}

// Clear initialized, running, and complete state for a fresh menu execution.
int MPBase1_ResetExecutionState(void* self) {
    AsMPBase1(self)->complete = 0;
    AsMPBase1(self)->running = 0;
    AsMPBase1(self)->initialized = 0;
    return 1;
}

// Initialize the base-menu unit's mutable two-pi constant.
void MPBase1_InitializeUnitConstants() {
    s_twoPi_MenuProcess = kFloat_6_2831855;
}

// Start the shared overlay's fifteen-frame menu fade-out.
void IFadeMenuBase_StartMenuFadeOut() {
    if (!((int&)Sing_805DC608())) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayDtorRegistration_8023F8DC);
            s_fadeOverlayInitialized = 1;
        }
        ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
    }
    FadeState_StartFadeOut((void*)((int&)Sing_805DC608()), kMenuFadeOutFrames);
}

// Fade back in on completion, otherwise request the pending overlay transition.
void IFadeMenuBase_FadeInOrRequestTransition(void*, int condition) {
    if (condition) {
        if (!((int&)Sing_805DC608())) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                         s_fadeOverlayDtorRegistration_8023F8DC);
                s_fadeOverlayInitialized = 1;
            }
            ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
        }
        FadeState_StartFadeIn((void*)((int&)Sing_805DC608()), 0);
    } else {
        if (!((int&)Sing_805DC608())) {
            if (!s_fadeOverlayInitialized) {
                FadeOverlay_Init(s_fadeOverlay);
                __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor,
                                         s_fadeOverlayDtorRegistration_8023F8DC);
                s_fadeOverlayInitialized = 1;
            }
            ((int&)Sing_805DC608()) = (int)s_fadeOverlay;
        }
        FadeState_RequestFramebufferCapture((void*)((int&)Sing_805DC608()));
    }
}

// Initialize the fading-menu unit's mutable two-pi constant.
void IFadeMenuBase_InitializeUnitConstants() {
    s_twoPi_FadingMenuProcess = kFloat_6_2831855;
}

} // extern "C"

#undef INSERT_FREE
#undef THROW_TOO_LONG
#undef PUSH_BACK

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622AC", _ctors_u8023F8DC_0, CEventFace_InitializeUnitConstants);
STATIC_INIT_ENTRY(".CRT$XCU804622AC", _ctors_u8023F8DC_1, MPBase1_InitializeUnitConstants);
STATIC_INIT_ENTRY(".CRT$XCU804622AC", _ctors_u8023F8DC_2, IFadeMenuBase_InitializeUnitConstants);
#pragma pop
