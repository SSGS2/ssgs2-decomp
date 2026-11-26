// Camera-motion (VCG) scripts: the player and source classes that sample key groups over time,
// the parser that loads a script into groups, and the framebuffer fade overlay.

#include <decomp/static_init.h>
// Camera/model-preview VCG motion scripts and framebuffer fade ownership.
// Frame rate lookup: CVCGPlayer::AdvancePlayback adds kFloat_1_VCGPlayer (1 script
// tick/call). Its SAME constant is subtracted from duration at a nonlooping end;
// a future mod wanting a separate step must separate those two roles first.
// CameraScript_ParseGroupData scales duration AND all three timing phases by
// kFloat_0_8333333_CameraDuration when EventTiming enables the 5/6 policy.
// Durations/keys are asset values, not elapsed seconds. Preserve phase ratios,
// end-frame selection, loop restart, direction wrap and position/scalar sampling.
// FadeState_Update is a second per-call clock: opacityStep alpha units/call,
// endpoints 0/255. Framebuffer capture is serviced BEFORE opacity advances.
// Resolution/texture lookup: acquireResource/FadeOverlay_Init use current
// Engine_GetViewport width/height, one mip, kFramebufferTextureFormat=0x80000006
// (platform RGBA8). Change viewport dimensions and copy/draw consumers together.
// There is no local screen width, aspect ratio, scissor or presentation interval.
// Scalar definitions: the data section and the shared fade pool; binary layouts
// and ownership aliases are mapped below. Packed scripts use native Wii big-endian
// Unless specified, self/output pointers are required and borrowed. Destructors
// accept null; positive deleting flags release wrapper storage and return the
// original possibly freed address. Vector helpers can throw length/allocation
// exceptions; guards retain constructed-range cleanup and allocation provenance.
// Calls assume serialized scene/render access; no internal locking is added.
#include "sdk_protos.h"
#include "game/types.h"
#include "classes/CVCGSource.h"
#include "classes/CVCGPlayer.h"
#include "classes/CVCGFile.h"

#define AsCVCGPlayer(p) ((CVCGPlayer*)(p))

// Construct into caller-owned vector storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}
// Placement delete leaves allocation cleanup to the vector transaction guard.
inline void operator delete(void*, void*) {}

extern "C" {

static const unsigned int kMotionPointLimit = 0x6666666u;
static const unsigned int kMotionPointGrowthTier1 = 0x2222222u;
static const unsigned int kMotionPointGrowthTier2 = 0x4444444u;
static const unsigned int kMotionGroupLimit = 0x3c3c3c3u;
static const unsigned int kMotionGroupGrowthTier1 = 0x1414141u;
static const unsigned int kMotionGroupGrowthTier2 = 0x2828282u;
static const unsigned int kMotionVectorLimit = 0x15555555u;
static const unsigned int kMotionVectorGrowthTier1 = 0x071c71c7u;
static const unsigned int kMotionVectorGrowthTier2 = 0x0e38e38eu;
// Runtime/key layout constants: units are bytes, not point counts.
enum MotionStorageLayout {
    kMotionKeyBytes = 0x28,
    kMotionGroupBytes = 0x44,
    kSecondTrackOffset = 0x0C,
    kDirectionOffset = 0x18,
    kDirectionSecondComponentOffset = 0x1C,
    kScalarOffset = 0x24,
    kGroupKeysOffset = 0x38,
    kGroupKeyCountOffset = 0x3C,
    kVectorXYZBytes = 0x0C
};
static const int kLinearMotion = 0, kBezierMotion = 1, kCubicMotion = 2;
static const int kFadeIdle = 0, kFadeIncreasing = 1, kFadeOpaque = 2, kFadeDecreasing = 3;
static const unsigned int kFramebufferTextureFormat =
    0x80000006u; // Engine platform-bit | GX RGBA8 (6); paired with EFB capture.
enum CameraPlaybackLoopMode {
    kStopAtGroupEnd = 0,
    kRestartGroupAtEnd = 1
};
enum FadeTextureLayout {
    kFadeMipCount = 1,
    kFadeBaseMipLevel = 0
};

// Storage map: source/player +4 is the logical group count, +8/+C/+10 is
// {group allocation, constructed count, capacity}. Each group is 0x44 bytes:
// packed header +0, expanded GroupValues +0x10, key vector +0x38.
// Key vectors own 0x28-byte entries through s_vcgAllocatorCallbacks. Temporary
// guards own the constructed range until commit; copies deep-copy nested keys.
// Playback advances by one script time unit per AdvancePlayback call. Packed
// durations become floats and may be scaled by 5/6; seconds are not implied.
// loopMode 0 stops at duration-1, 1 restarts at zero; other modes keep advancing.

extern const float kFloat_6_2831855;
extern float s_vcgPlayerTwoPiCache;
extern float s_vcgSourceTwoPiCache;
extern const float kFloat_0_VCGPlayer;
extern const float
    kFloat_1_VCGPlayer; // Shared 1 script tick: advance AND duration-1 endpoint. Larger is faster and changes final pose.

extern char __vt__10CVCGPlayer[];
extern char s_vcgSourceVtable[];

struct GroupRecords;
struct GroupValues;
struct MotionItem;
struct SplineArg;
int GroupVector_SampleMotionAtTime(GroupRecords*, MotionItem*, int, float);
void __dt__8CVCGFileFv(void*, int);
void __dl__FPv(void*);
int GroupVector_GetValues(GroupRecords*, GroupValues*, int);
void CameraMotionGroupVector_AppendDefaultGroups(void*, unsigned int);
struct RemovalVector;
void CameraMotionGroupVector_RemoveBackGroups(RemovalVector*, int);
int EventTiming_IsDurationScalingEnabled();
int CameraMotionGroupVector_ComputeTrackLengths(GroupRecords*, int);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
char* strcpy(char*, const char*);
void __dt__Q23std12length_errorFv(void*);
extern const char vcgVectorExceptionStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern float
    kFloat_0_8333333_CameraDuration; // binary32 ~5/6: all phase durations, reducing calls at unchanged tick/call.

struct VTableSlot {
    void* (*allocate)(unsigned int);
    void (*destroy)(void*);
};
extern VTableSlot s_vcgAllocatorCallbacks;

extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct PointAllocator {
    volatile char value;
    // Initialize the allocator tag retained in the original exception scopes.
    PointAllocator() : value(0) {}
    // The tag has no storage ownership to release.
    ~PointAllocator() {}
};

struct ErrorAllocation {
    char* pointer;
    char* allocator;
    // Adopt exception text until its control block is constructed.
    ErrorAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free exception text when adoption by the control block did not complete.
    ~ErrorAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct ErrorControlBase {
    void* table;
    unsigned strong, weak;
    // Initialize a shared character deleter with one strong and one weak owner.
    ErrorControlBase() {
        strong = 1;
        weak = 1;
        table = s_sharedCharDeleterVtable;
    }
};
struct ErrorControl : ErrorControlBase {
    char* pointer;
    // Adopt the allocated character buffer after base ownership initialization.
    ErrorControl(char* p) {
        pointer = p;
    }
};
namespace std {
struct exception {
    void* table;
};
struct logic_error : exception {
    // Install the logic-error runtime type before the derived message is built.
    logic_error() {
        table = __vt__Q23std11logic_error;
    }
    ~logic_error();
};
struct length_error : logic_error {
    char* text;
    void* control;
    // Deep-copy the vector error message and construct its shared ownership block.
    length_error() {
        const char* message = vcgVectorExceptionStrings;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            ErrorAllocation allocation(p, &allocator.value);
            control = new ErrorControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }

    ~length_error();
};
}

// Throw a vector-length exception without the point allocator tag.
inline void VectorThrow() {
    throw std::length_error();
}
// Throw a vector-length exception within the point allocator's cleanup scope.
inline void PointVectorThrow() {
    PointAllocator allocator;
    throw std::length_error();
}
// Select the larger capacity increment, preserving the chosen reference.
inline const unsigned& VectorMax(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}
// Choose point-vector capacity with overflow checks and the original two growth tiers.
inline unsigned GrowthCapacity(unsigned cp, unsigned need) {
    PointAllocator allocator;
    if (need > kMotionPointLimit - cp)
        PointVectorThrow();
    if (cp < kMotionPointGrowthTier1)
        return cp + VectorMax((cp + 1) * 3 / 5, need);
    if (cp < kMotionPointGrowthTier2)
        return cp + VectorMax((cp + 1) / 2, need);
    return kMotionPointLimit;
}
// Allocate storage for 0x28-byte motion points through the VCG allocator.
inline void* AllocatePoints(unsigned n) {
    PointAllocator allocator;
    if (n > kMotionPointLimit)
        PointVectorThrow();
    return s_vcgAllocatorCallbacks.allocate(n * 0x28);
}

// Storage map (all aliases refer to the same allocations):
// Pt/SmallItem = bitwise key construction/copy; MotionItem = sampling fields.
// Group/RemovalGroup = nested ownership; ParsedGroup = decode target;
// GroupRecord/GroupValues = timing/track metrics used by evaluation.
// These layouts intentionally have different constructors and cleanup boundaries.
// Storage-only views deliberately retain word access. Pt::w[0..2] is track one,
// [3..5] track two, [6..8] direction, [9] scalar (same bytes as MotionItem).
// Copy/cleanup views below are aliases of that layout, not independent records.
// Float conversion of these words would change bitwise copy behavior.
struct Pt {
    unsigned int w[10];
    // Zero all ten words of a default motion key.
    Pt() {
        w[0] = w[1] = w[2] = w[3] = w[4] = 0;
        w[5] = w[6] = w[7] = w[8] = w[9] = 0;
    }
    // Copy the key's stored bits with the original exception boundary.
    Pt(const Pt& other) {
        try {
            *this = other;
        } catch (...) {
            throw;
        }
    }
};
// Float components are stored as unsigned words to preserve bitwise copies;
// vector helpers interpret those words as float, never as integer coordinates.
struct MotionVec {
    unsigned int x, y, z;
};
// Camera-key consumer schema (CEventCameraSC_CAM_SET_UpdateCameraAnimation in
// effect_fade_and_camera_script.cpp, CRoundDol3D in round_dol3d_spinning_dolfini_scene.cpp, camera work in camera_work_shot_cameras.cpp): +0 eye position,
// +0xC target position (world XYZ), +0x18 up vector, +0x24 field of view in radians.
// The first two positions feed forward=target-eye. Up feeds right=up cross forward,
// then correctedUp=forward cross right. Multi-key sampling reconstructs the up
// pair as (sin(angle),cos(angle),0), angle=atan2(up.x,up.y); this is camera roll
// in the stored XY plane, not yaw or an angle measured from gameplay world Z.
// Single-key sampling copies all stored up components verbatim, including Z.
struct MotionItem {
    MotionVec firstPositionTrack, secondPositionTrack, planarDirectionStorage;
    float value;
};
struct PointBuffer {
    Pt* data;
    unsigned size;
    unsigned cap;
    unsigned* owner;
    unsigned offset;
    // Begin an empty guarded replacement for the destination key allocation.
    PointBuffer(unsigned* p) : data(0), size(0), cap(0), owner(p), offset(0) {}
    // Release the guarded key allocation; keys contain no nested owned objects.
    ~PointBuffer() {
        if (data) {
            PointAllocator allocator;
            size -= ((char*)data + size) - (char*)data;
            s_vcgAllocatorCallbacks.destroy(data);
        }
    }
};
// File records use signed 16-bit timing/count values before their motion keys.
// Producer: the borrowed VCG file image loaded by CVCGFile/CEventCameraSC in
// effect_fade_and_camera_script.cpp; ParseGroupData copies file+groupOffsets[i] to group.prefix. No script
// exporter is recovered. Bytes +0..3 are preserved metadata, with no decoded
// consumer in this parser, GetGroupValues, track-length calculation or sampling;
// camera consumers receive only MotionItem eye/target/up/FOV, never these bytes.
// Their individual meanings/valid encodings are unknown; preserve all four for
// deep-copy compatibility. They are not interpolation or loop flags (those have
// separate fields). Editing timing/keys needs no interpretation of them.
struct PackedCameraGroup {
    unsigned char formatBytes[4]; // Copied verbatim; their individual meanings remain unresolved.
    short duration, keyCount, interpolation, accelerationTime, cruiseTime, decelerationTime;
};
struct PackedCameraScript {
    unsigned int signature, groupCount, groupOffsets[1];
};
typedef char PackedCameraGroupSize[sizeof(PackedCameraGroup) == 0x10 ? 1 : -1];
struct PlaybackState {
    int groupIndex;
    float time;
    int stopped, loopMode;
};
struct ParsedGroup {
    unsigned char b0[0x10];
    float duration;
    int keyCount;
    int interpolation;
    float accelerationTime;
    float cruiseTime;
    float decelerationTime;
    unsigned char pad[0x10];
    Pt* data;
    unsigned int size;
    unsigned int cap;
};
struct Manager {
    void* vtable;
    unsigned int groupCount;
    ParsedGroup* groups;
    unsigned int storedGroupCount;
};
// Convert a group index into a byte displacement in expanded group storage.
inline unsigned GroupOffset(int index) {
    return index * sizeof(ParsedGroup);
}
// Convert a key index into a byte displacement in the nested key allocation.
inline unsigned PointOffset(int index) {
    return index * sizeof(MotionItem);
}

// One 0x28 byte entry of the nested buffer.
struct SmallItem {
    unsigned words[10];
    // Copy one motion key as ten stored words.
    SmallItem(const SmallItem& other) {
        *this = other;
    }
};

// Nested buffer stored at +0x38 of a Group.
struct SmallBufferStorage {
    SmallItem* data;
    unsigned size;
    unsigned capacity;
    // Initialize an empty owned nested key allocation.
    SmallBufferStorage() : data(0), size(0), capacity(0) {}
    // Free nested keys through the same allocator that created them.
    ~SmallBufferStorage() {
        if (data) {
            unsigned n = size;
            PointAllocator allocator;
            size = n - (((char*)data + n) - (char*)data);
            s_vcgAllocatorCallbacks.destroy(data);
        }
    }
};
struct SmallBufferBase : SmallBufferStorage {
    // Delegate storage release to the base, retaining the original cleanup layer.
    ~SmallBufferBase() {}
};
struct SmallBufferImpl : SmallBufferBase {
    // Delegate storage release through the original intermediate cleanup layer.
    ~SmallBufferImpl() {}
};
struct SmallBuffer : SmallBufferImpl {
    // Construct an empty key buffer through the storage base.
    SmallBuffer() {}
    // Deep-copy keys into a separately owned allocation, updating constructed size.
    SmallBuffer(const SmallBuffer& other) {
        const SmallItem* src = other.data;
        const SmallItem* end = src + other.size;
        unsigned n = end - src;
        if (n != 0) {
            PointAllocator allocator;
            if (n > kMotionPointLimit)
                PointVectorThrow();
            data = (SmallItem*)s_vcgAllocatorCallbacks.allocate(n * 0x28);
            capacity = n;
            PointAllocator copyAllocator;
            SmallItem* dst = data + size;
            while (src != end) {
                new (dst) SmallItem(*src);
                ++src;
                ++dst;
                ++size;
            }
        }
    }
    // Let the storage base release the key allocation.
    ~SmallBuffer() {}
};

// The first 0x38 bytes of a Group.
// Header word map: h[0..7] is the original 16-byte packed header.
// w[0] duration(float bits), w[1] key count, w[2] interpolation,
// w[3..5] acceleration/cruise/deceleration(float bits),
// w[6..9] first length/speed then second length/speed(float bits).
// ParsedGroup and GroupRecord overlay these same 0x44 bytes; SmallBuffer follows.
struct GroupHeader {
    unsigned short h[8];
    unsigned w[10];
};

// Expanded group header word roles; integer access preserves float component bits.
enum ExpandedGroupWord {
    kGroupDurationWord,
    kGroupKeyCountWord,
    kGroupInterpolationWord,
    kGroupAccelerationWord,
    kGroupCruiseWord,
    kGroupDecelerationWord,
    kGroupFirstLengthWord,
    kGroupFirstSpeedWord,
    kGroupSecondLengthWord,
    kGroupSecondSpeedWord
};
struct CopyGroupHeader : GroupHeader {
    // Leave an unpopulated header untouched until parsing/copying fills it.
    CopyGroupHeader() {}
    // Copy packed halfwords and expanded words with their original access widths.
    CopyGroupHeader(const CopyGroupHeader& other) {
        unsigned short a = other.h[0], b = other.h[1];
        h[0] = a;
        h[1] = b;
        a = other.h[2];
        b = other.h[3];
        h[2] = a;
        h[3] = b;
        a = other.h[4];
        b = other.h[5];
        h[4] = a;
        h[5] = b;
        a = other.h[6];
        b = other.h[7];
        h[6] = a;
        h[7] = b;
        unsigned c = other.w[kGroupDurationWord], d = other.w[kGroupKeyCountWord];
        w[kGroupDurationWord] = c;
        w[kGroupKeyCountWord] = d;
        c = other.w[kGroupInterpolationWord];
        d = other.w[kGroupAccelerationWord];
        w[kGroupInterpolationWord] = c;
        w[kGroupAccelerationWord] = d;
        c = other.w[kGroupCruiseWord];
        d = other.w[kGroupDecelerationWord];
        w[kGroupCruiseWord] = c;
        w[kGroupDecelerationWord] = d;
        c = other.w[kGroupFirstLengthWord];
        d = other.w[kGroupFirstSpeedWord];
        w[kGroupFirstLengthWord] = c;
        w[kGroupFirstSpeedWord] = d;
        c = other.w[kGroupSecondLengthWord];
        d = other.w[kGroupSecondSpeedWord];
        w[kGroupSecondLengthWord] = c;
        w[kGroupSecondSpeedWord] = d;
    }
};

struct Group {
    CopyGroupHeader header;
    SmallBuffer buffer;
    // Construct empty nested storage; the header is populated later.
    Group() : buffer() {}
    // Copy the header and deep-copy its owned motion keys.
    Group(const Group& other) : header(other.header), buffer(other.buffer) {}
};

// The vector of Groups: data at +0, size at +4, capacity at +8.
struct GroupVector {
    Group* data;
    unsigned size;
    unsigned capacity;
};

// Temporary owner of a Group block; destroys its elements and frees the block.
struct GroupBuffer {
    Group* data;
    unsigned size;
    unsigned capacity;
    unsigned* owner;
    unsigned offset;
    // Begin a guarded replacement group allocation with an empty constructed range.
    GroupBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Destroy constructed groups in reverse order, then release their outer block.
    ~GroupBuffer() {
        if (data) {
            unsigned n = size;
            PointAllocator allocator;
            size -= n;
            Group* p = data + n;
            while (n != 0) {
                --p;
                p->~Group();
                --n;
            }
            s_vcgAllocatorCallbacks.destroy(data);
        }
    }
};

struct GroupValues {
    float duration;
    int keyCount, interpolation;
    float accelerationTime, cruiseTime, decelerationTime, firstTrackLength, firstTrackSpeed, secondTrackLength,
        secondTrackSpeed;
};
struct GroupRecord {
    char prefix[0x10];
    GroupValues values;
    SmallBuffer buffer;
};
// Consolidated runtime aliases (Wii pointers/words four bytes, alignment four):
// packed/file header 0x00..0x0F is retained in GroupRecord.prefix/ParsedGroup.b0;
// its short keyCount at +6 is rechecked by single-key sampling. Runtime +0x10 is
// GroupValues: duration(+10), keyCount(+14), interpolation(+18), acceleration(+1C),
// cruise(+20), deceleration(+24), first length/speed(+28/+2C), second(+30/+34).
// ParsedGroup names +10..24 directly and covers +28..37 with pad; it does NOT
// place the key buffer there. +38/+3C/+40 = owned key pointer/size/capacity in
// GroupRecord.buffer, ParsedGroup.data/size/cap, Group::buffer and StoreOwner.store.
// Whole record stride is 0x44; file header stride is 0x10 plus 0x28 per key.
// MotionContext and SplineArg are the same separate 0x10-byte stack descriptor:
// +0 segment index, +4 fraction, +8 borrowed GroupValues*/SplineCfg*, +C borrowed
// MotionItem*/points. SplineCfg.headerWord aliases FLOAT duration bits (not a
// packed header); cfg.count at +4 aliases keyCount, +18..24 aliases lengths/speeds.
// These casts change access vocabulary only. No construction or ownership moves.
// Fade ResetState/ResourceState below describe a DIFFERENT object, never a group.
struct GroupRecords {
    void* unknown;
    int size;
    GroupRecord* data;
};

// Copy a group's timing, interpolation mode, lengths and speeds to the caller.
// Requires index >= 0; returns 0 for index >= logical count, else writes out and returns 1.
inline int GetGroupValues(GroupRecords* v, GroupValues* out, int index) {
    if (index >= v->size)
        return 0;
    GroupValues* src = &v->data[index].values;
    out->duration = src->duration;
    out->keyCount = src->keyCount;
    out->interpolation = src->interpolation;
    out->accelerationTime = src->accelerationTime;
    out->cruiseTime = src->cruiseTime;
    out->decelerationTime = src->decelerationTime;
    out->firstTrackLength = src->firstTrackLength;
    out->firstTrackSpeed = src->firstTrackSpeed;
    out->secondTrackLength = src->secondTrackLength;
    out->secondTrackSpeed = src->secondTrackSpeed;
    return 1;
}

struct MotionContext {
    int index;
    float fraction;
    GroupValues* values;
    MotionItem* data;
};
void Vec3_Subtract(void*, const void*, const void*);
void Vec3_Scale(void*, const void*, float);
void Vec3_Add(void*, const void*, const void*);
void GroupVector_FindMotionSegmentAtTime(int, int*, float*, float*, void*, float);
void GroupVector_SampleBezierMotion(int, int, SplineArg*, float);
void GroupMotion_SampleCubicVectorPair(int, char*, SplineArg*, float);
void GroupVector_InterpolatePlanarDirection(int, char*, SplineArg*);
extern const float kFloat_0_GroupVector, kFloat_0_5_MotionCurve;

// Integrate acceleration, constant speed and deceleration into script time.
// The result has script-time units; multiplying by either track speed gives
// distance along that track. Callers provide valid nonnegative phase durations.
inline float IntegrateMotionScriptTime(GroupValues* values, float time) {
    if (time <= values->accelerationTime) {
        if (kFloat_0_GroupVector != values->accelerationTime) {
            time *= time;
            time /= values->accelerationTime;
            return time * kFloat_0_5_MotionCurve;
        } else
            return kFloat_0_GroupVector;
    } else if (time <= values->accelerationTime + values->cruiseTime) {
        return values->accelerationTime * kFloat_0_5_MotionCurve + (time - values->accelerationTime);
    } else {
        float remaining = values->duration - time;
        float decel = remaining * remaining / values->decelerationTime;
        return values->decelerationTime * kFloat_0_5_MotionCurve +
               (values->accelerationTime * kFloat_0_5_MotionCurve + values->cruiseTime) -
               decel * kFloat_0_5_MotionCurve;
    }
}

struct RemovalBase {
    SmallItem* data;
    unsigned size, capacity;
    // Clear logical key size and free the nested allocation during group removal.
    ~RemovalBase() {
        if (data) {
            size -= ((char*)data + size) - (char*)data;
            s_vcgAllocatorCallbacks.destroy(data);
        }
    }
};
struct RemovalStorage : RemovalBase {
    // Delegate nested allocation release through this ABI cleanup layer.
    ~RemovalStorage() {}
};
struct RemovalImpl : RemovalStorage {
    // Delegate nested allocation release through this ABI cleanup layer.
    ~RemovalImpl() {}
};
struct RemovalBuffer : RemovalImpl {
    // Delegate nested allocation release to the storage base.
    ~RemovalBuffer() {}
};
struct RemovalGroup {
    GroupHeader header;
    RemovalBuffer buffer;
};
struct RemovalVector {
    RemovalGroup* data;
    unsigned size, capacity;
};

struct ClearRecords {
    void* unknown;
    int size;
    RemovalGroup* data;
    unsigned used;
};

float Vec3_Length(const void*);
extern const float kCameraTrackSpeedNumeratorFactor;

extern const float kFloat_1_Spline;
extern float kFloat_neg3_1415927_AngleInterpolation, kFloat_6_2831855_AngleInterpolation,
    kFloat_3_1415927_AngleInterpolation;

extern const float kFloat_0_805E0218;
extern float s_motionFadeTwoPiCache;

extern void Vector_CopyComponents(void* dst, const void* src);

extern double atan2(double, double);
extern float Math_SinApprox(float);
extern float Math_CosApprox(float);

struct AllocTag {
    volatile char value;
    char pad[3];
    // Initialize the vector allocator tag used in guarded copy/throw scopes.
    AllocTag() : value(0) {}
};

struct VecStore {
    MotionVec* data; // 0x00
    u32 size;        // 0x04
    u32 cap;         // 0x08
    // Clear logical size and free the owned three-word-vector allocation.
    ~VecStore() {
        if (data != 0) {
            AllocTag alloc;
            size -= ((char*)data + size) - (char*)data;
            s_vcgAllocatorCallbacks.destroy(data);
        }
    }
};

struct VecGuard_u8023A378 {
    MotionVec* data;
    u32 size;
    u32 cap;
    u32* oldcap;
    u32 remaining;
    // Begin an empty replacement allocation tied to the old capacity word.
    VecGuard_u8023A378(u32* p) : data(0), size(0), cap(0), oldcap(p), remaining(0) {}
    // Free uncommitted storage, or old storage handed over during commit.
    ~VecGuard_u8023A378() {
        if (data) {
            AllocTag alloc;
            size -= ((char*)data + size) - (char*)data;
            s_vcgAllocatorCallbacks.destroy(data);
        }
    }
};

struct SplineCfg {
    u32 headerWord; // 0x00
    s32 count;      // 0x04
    char pad08[0x10];
    f32 firstTrackLength;  // 0x18
    f32 firstTrackSpeed;   // 0x1c
    f32 secondTrackLength; // 0x20
    f32 secondTrackSpeed;  // 0x24
};

struct SplineArg {
    s32 index;      // 0x00
    f32 value;      // 0x04
    SplineCfg* cfg; // 0x08
    char* points;   // 0x0c
};

struct ErrObj {
    u32 vptr;  // 0x00
    char* msg; // 0x04
    void* rep; // 0x08
};

struct Rep16 {
    u32 vptr;   // 0x00
    u32 refs;   // 0x04
    u32 cap;    // 0x08: second shared-control counter, not vector capacity (name is provisional)
    char* data; // 0x0c
    // Adopt exception text and initialize both shared-control counters to one.
    Rep16(char* p) : refs(1), cap(1) {
        vptr = (u32)s_sharedCharDeleterVtable;
        data = p;
    }
};

struct MsgGuard {
    char* data;
    volatile char* alloc;
    // Own text until shared-control construction succeeds.
    MsgGuard(char* p, volatile char* a) : data(p), alloc(a) {}
    // Free text if its ownership was not transferred.
    ~MsgGuard() {
        if (data)
            __dla__FPv(data);
    }
};

struct ErrBase : ErrObj {
    // Construct an owned exception message and a guarded shared-control block.
    ErrBase(const char* s) {
        vptr = (u32)__vt__Q23std11logic_error;
        AllocTag alloc;
        u32 len = strlen(s) + 1;
        char* p = (char*)__nwa__FUl(len * 2 - len);
        msg = p;
        {
            MsgGuard guard(p, &alloc.value);
            rep = new Rep16(p);
            guard.data = 0;
        }
        strcpy(msg, s);
    }
    // Forward destruction to the retained exception base runtime.
    ~ErrBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};

struct LengthErr : ErrBase {
    // Set the derived length-error type after constructing the message base.
    LengthErr(const char* s) : ErrBase(s) {
        vptr = (u32)__vt__Q23std12length_error;
    }
};

// Inlined length_error construction + throw (the unit's vector helper does this).
inline void ThrowMotionVectorLengthError() {
    AllocTag allocator;
    throw LengthErr("vector length error");
}

struct ResetState {
    u32 headerWord;
    f32 opacity;
    f32 opacityStep;
    u32 phase;
    unsigned char captureRequested;
};

struct StoreOwner {
    char pad00[0x38];
    VecStore store;
};

struct OwnerStore {
    StoreOwner* data;
    u32 size, cap, unknown, first;
};

#include "MSL/vector.h"
#include <decomp/cw_bitfields.h>
// Allocator whose storage goes back through the vcg allocator callbacks.
template <class T> class vcg_allocator {
public:
    // Return vector storage through the VCG allocator that created it.
    void deallocate(T* p, unsigned long) {
        s_vcgAllocatorCallbacks.destroy(p);
    }
};
struct VcgRecord {
    u32 words[3];
};
typedef std::vector<VcgRecord, vcg_allocator<VcgRecord> > VcgVector;
// Release the nested vector allocation owned by one motion group.
inline void destroyOwner(StoreOwner* p) {
    if (p) {
        ((VcgVector*)&p->store)->~VcgVector();
    }
}

struct SharedResource {
    CW_BITS2(u32 refs : 16, u32 flags : 16)
};

struct ResourceState {
    SharedResource* resource;
    f32 opacity;
    f32 opacityStep;
    u32 phase;
    unsigned char captureRequested, option11, option12, releaseOnCompletion;
};
// Fade alias map (0x14 bytes): ResetState.headerWord aliases the resource pointer
// at +0; opacity/step/phase at +4/+8/+C and captureRequested at +10 coincide.
// ResetState omits the trailing option11/option12/releaseOnCompletion bytes;
// resetting its named members must not be read as clearing all fade options.
// ResourceConfig/Info/Buffer are separate viewport/mip/lock call outputs, not
// aliases of the motion keys or ResourceState. Only resource owns a reference.
// Fade phases: 0=transparent/idle, 1=increase toward 255, 2=opaque,
// 3=decrease toward zero. opacityStep is alpha units per Update, not per second.
// resource owns one 16-bit shared reference. captureRequested is consumed once
// even when acquisition fails; releaseOnCompletion applies at the end of phase 3.

struct ResourceConfig {
    u32 unknown[2];
    u32 width, height;
    u32 tail[2];
};

struct ResourceInfo {
    u32 format;
    char pad04[0x18];
    u32 width, height;
};

struct ResourceBuffer {
    u32 unknown;
    void* data;
};

extern unsigned char s_fadeOverlayTextureReady, s_clearFramebufferTextureOnCreate;
void CubeResourceStorage_dtor(SharedResource*, s32);
void Engine_GetViewport(ResourceConfig*);
s32 Texture_Create2D(u32, u32, s32, s32, u32, s32, ResourceState*, s32);
s32 Texture_GetLevelDescription(SharedResource*, s32, ResourceInfo*);
s32 Texture_LockLevel(SharedResource*, s32, ResourceBuffer*, s32, s32);
u32 Texture_CalculateMipStorageSize(u32, u32, s32, u32);
s32 Texture_UnlockLevel(SharedResource*, s32);

extern unsigned char s_releaseFadeResourceOnCompletion;
extern const float kFloat_255_Fade;
void Engine_CopyFramebufferToTexture(s32, s32, SharedResource*, s32, s32, s32, s32, s32, s32, s32, s32, f32);

// Lazily create the fade framebuffer texture and optionally clear its pixels.
// Returns 0 on create/description/lock/unlock failure. A texture already stored
// in p returns 1 without retrying a previous failed clear; failures after create
// retain that reference. Only successful completion publishes the ready flag.
// Config words +8/+C are viewport width/height in pixels (Engine_GetViewport);
// ResourceInfo +1C/+20 are mip width/height, +0 the encoded texture format.
inline s32 acquireResource(ResourceState* p) {
    ResourceInfo info;
    ResourceConfig config;
    ResourceBuffer buffer;
    if (p->resource)
        return 1;
    Engine_GetViewport(&config);
    if (!Texture_Create2D(config.width, config.height, 1, 0, kFramebufferTextureFormat, 0, p, 0))
        return 0;
    if (s_clearFramebufferTextureOnCreate) {
        if (!Texture_GetLevelDescription(p->resource, 0, &info))
            return 0;
        if (!Texture_LockLevel(p->resource, 0, &buffer, 0, 0))
            return 0;
        Memory_Clear(buffer.data, Texture_CalculateMipStorageSize(info.width, info.height, 1, info.format));
        if (!Texture_UnlockLevel(p->resource, 0))
            return 0;
    }
    s_fadeOverlayTextureReady = 1;
    return 1;
}

// Optionally clear the new framebuffer texture, then publish its ready flag.
inline void FinishResourceLoad(ResourceState* p, ResourceInfo* info, ResourceBuffer* buffer) {
    if (s_clearFramebufferTextureOnCreate) {
        if (!Texture_GetLevelDescription(p->resource, 0, info))
            return;
        if (!Texture_LockLevel(p->resource, 0, buffer, 0, 0))
            return;
        Memory_Clear(buffer->data, Texture_CalculateMipStorageSize(info->width, info->height, 1, info->format));
        if (!Texture_UnlockLevel(p->resource, 0))
            return;
    }
    s_fadeOverlayTextureReady = 1;
}

// Clear and release the vector's storage, then optionally delete its wrapper.
void* GroupVector_dtor(void* self, int shouldDelete) {
    if (self) {
        (*(void**)&((GroupVector*)self)->size) = 0;
        if (self) {
            if ((void*)((GroupVector*)self)->data) {
                (*(void**)&((GroupVector*)self)->size) = 0;
                s_vcgAllocatorCallbacks.destroy((void*)((GroupVector*)self)->data);
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Initialize the VCG player module's two-pi cache.
void VCGPlayer_InitAngleCache() {
    s_vcgPlayerTwoPiCache = kFloat_6_2831855;
}

// Initialize a stopped camera-motion player at group zero and time zero.
CVCGPlayer::CVCGPlayer() {
    ((CVCGFile*)this)->__ct();
    float value = kFloat_0_VCGPlayer;
    *(void**)this = __vt__10CVCGPlayer;
    groupIndex = 0;
    playbackTime = value;
    stopped = 1;
}

// Destroy the camera-file base and optionally delete the player.
CVCGPlayer::~CVCGPlayer() {
    __dt__8CVCGFileFv(this, 0);
}

// Sample the active group at the current playback time into output.
// Returns the sampler's 0/1 result; the caller keeps ownership of output.
int CVCGPlayer::SampleCurrentMotion(void* output) {
    float time = playbackTime;
    return GroupVector_SampleMotionAtTime((GroupRecords*)this, (MotionItem*)output, groupIndex, time);
}

// Advance one time unit, stopping on the last frame or restarting a looping group.
// Required initialized player with a selected valid group; the values-query
// status is ignored. stopped!=0 leaves it unchanged; always returns 1 (not
// completion status). loopMode 0 selects duration-1 and stops, 1 resets to zero,
// all other values keep advancing. No leftover time is carried across a loop.
// No dt is accepted here. Preserve increment-before-boundary-test and the
// shared step/end-offset role when introducing a later frame-rate setting.
int CVCGPlayer::AdvancePlayback() {
    char groupValues[0x28];
    if (stopped == 0) {
        float value = playbackTime;
        playbackTime = value + kFloat_1_VCGPlayer;
        void* groupIndex = (*(void**)&this->groupIndex);
        float* durationOutput = (float*)groupValues;
        GroupVector_GetValues((GroupRecords*)this, (GroupValues*)durationOutput, (int)groupIndex);
        float duration = *durationOutput;
        if (duration <= playbackTime) {
            int loopMode = this->loopMode;
            if (loopMode == kStopAtGroupEnd) {
                float delta = kFloat_1_VCGPlayer;
                stopped = 1;
                float newValue = duration - delta;
                playbackTime = newValue;
            } else if (loopMode == kRestartGroupAtEnd) {
                playbackTime = kFloat_0_VCGPlayer;
            }
        }
    }
    return 1;
}

// Validate a group, select its loop mode, and restart playback at time zero.
// index is a nonnegative group number, not a pointer. Returns 0 with playback unchanged for index>=count;
// negative indices are not rejected and are invalid inputs. See loop-mode map
// above. Returns 1 after publication; no allocation or resource transfer occurs.
int CVCGPlayer::StartGroupPlayback(unsigned int index, int mode) {
    char groupValues[0x28];
    int result = GroupVector_GetValues((GroupRecords*)this, (GroupValues*)groupValues, (int)index);
    if (!result)
        return 0;
    float z = kFloat_0_VCGPlayer;
    groupIndex = (int)index;
    playbackTime = z;
    stopped = 0;
    loopMode = mode;
    return 1;
}

#pragma push
// Preserve the getter's load-all-then-store snapshot: lifetime optimization and
// propagation are disabled together so the four source temporaries stay explicit.
#pragma opt_lifetimes off
#pragma opt_propagation off
// Copy the selected group, playback time, stopped flag and loop mode to the caller.
// output is nullable PlaybackState[1] (16 bytes); null returns 0 unchanged,
// otherwise writes all four fields and returns 1. No pointer to internal state escapes.
int CVCGPlayer::GetPlaybackState(void* output) {
    if (output) {
        int loopMode, stopped, groupIndex;
        float playbackTime;
        groupIndex = this->groupIndex;
        playbackTime = this->playbackTime;
        stopped = this->stopped;
        loopMode = this->loopMode;
        ((PlaybackState*)output)->groupIndex = groupIndex;
        ((PlaybackState*)output)->time = playbackTime;
        ((PlaybackState*)output)->stopped = stopped;
        ((PlaybackState*)output)->loopMode = loopMode;
        return 1;
    }
    return 0;
}
#pragma pop

// Initialize the VCG source module's two-pi cache.
void VCGSource_InitAngleCache() {
    s_vcgSourceTwoPiCache = kFloat_6_2831855;
}

// Initialize an empty camera-motion group source.
CVCGSource::CVCGSource() {
    (*(int*)&entries) = 0;
    *(void**)this = s_vcgSourceVtable;
    entryCount = 0;
    entryCapacity = 0;
    groupCount = 0;
}

#pragma push
// Source-vector teardown policy: keep multi-register save/restore enabled;
// retain byte-offset multiplication (strength reduction off), distinct nested
// member guards/loads (propagation/CSE off), and their cleanup-scope lifetimes
// (lifetimes off). These are compiler-shape constraints, not extra null safety.
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
// Destroy every group's nested point storage and release the source vector.
CVCGSource::~CVCGSource() {
    // Phase 1: clear logical key counts before reverse destruction of constructed groups.
    {
        int offset;
        char* item;
        int i;
        item = s_vcgSourceVtable;
        i = 0;
        offset = 0;
        *(void**)this = item;
        while (i < groupCount) {
            char* items = (*(char**)&entries);
            ++i;
            item = items + offset;
            offset += kMotionGroupBytes;
            int n = *(int*)(item + kGroupKeyCountOffset);
            char* data = *(char**)(item + kGroupKeysOffset);
            *(int*)(item + kGroupKeyCountOffset) -= (data + n) - data;
        }
    }

    // Phase 2: destroy constructed groups in reverse order. Nested +0x38
    // guards model destructor layers; they are not pointer-validity checks.
    {
        VTableSlot* vt;
        char* cur;
        int used;
        int bytes;
        char* items;
        used = entryCount;
        vt = &s_vcgAllocatorCallbacks;
        items = (*(char**)&entries);
        bytes = used * kMotionGroupBytes;
        entryCount -= used;
        cur = items + bytes;
        while (used != 0) {
            if ((cur -= kMotionGroupBytes) != 0) {
                if (cur + kGroupKeysOffset != 0) {
                    if (cur + kGroupKeysOffset != 0) {
                        if (cur + kGroupKeysOffset != 0) {
                            if (cur + kGroupKeysOffset != 0) {
                                if (*(void**)(cur + kGroupKeysOffset) != 0) {
                                    int n = *(int*)(cur + kGroupKeyCountOffset);
                                    char* data = *(char**)(cur + kGroupKeysOffset);
                                    *(int*)(cur + kGroupKeyCountOffset) -= (data + n) - data;
                                    vt->destroy(*(void**)(cur + kGroupKeysOffset));
                                }
                            }
                        }
                    }
                }
            }
            --used;
        }
    }

    // Nested buffers are released; clear the logical count and release the
    // outer vector through its retained destructor layers.
    groupCount = 0;
    if ((void*)((char*)this + 8)) {
        if ((void*)((char*)this + 8)) {
            if ((void*)((char*)this + 8)) {
                if ((void*)((char*)this + 8)) {
                    int bytes;
                    char* items = (*(char**)&entries);
                    if (items != 0) {
                        char* cur;
                        int used;
                        VTableSlot* vt;
                        used = entryCount;
                        vt = &s_vcgAllocatorCallbacks;
                        bytes = used * kMotionGroupBytes;
                        entryCount -= used;
                        cur = items + bytes;
                        while (used != 0) {
                            if ((cur -= kMotionGroupBytes) != 0) {
                                if (cur + kGroupKeysOffset != 0) {
                                    if (cur + kGroupKeysOffset != 0) {
                                        if (cur + kGroupKeysOffset != 0) {
                                            if (cur + kGroupKeysOffset != 0) {
                                                if (*(void**)(cur + kGroupKeysOffset) != 0) {
                                                    int n = *(int*)(cur + kGroupKeyCountOffset);
                                                    char* data = *(char**)(cur + kGroupKeysOffset);
                                                    *(int*)(cur + kGroupKeyCountOffset) -= (data + n) - data;
                                                    vt->destroy(*(void**)(cur + kGroupKeysOffset));
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            --used;
                        }
                        s_vcgAllocatorCallbacks.destroy(entries);
                    }
                }
            }
        }
    }
}
#pragma pop

// Decode packed group headers and motion keys, resize storage, and compute both track lengths.
// Requires a complete aligned file image: offsets relative to file base,
// 16-byte group headers followed by keyCount contiguous 40-byte keys. Signature,
// version, bounds, signed counts and phase sums are NOT validated. Sampling
// requires one key or >=2 keys, duration>=0 and nonnegative timing phases whose
// sum agrees with duration; speeds divide by duration+cruiseTime. Do not supply
// zero denominators. Native big-endian decoding performs no byte swapping.
// Returns 1 after all groups; allocation/length failure throws and may leave
// earlier groups updated. The input image may be released after successful copy.
// Storage/representability requirements are SEPARATE from sampling admission:
// groupCount is unsigned on disk but the traversal casts it to signed int;
// require <= INT_MAX and <= kMotionGroupLimit (0x3C3C3C3 runtime 0x44-byte
// groups). The file must contain groupCount offset words, all offsets and each
// 0x10 + keyCount*0x28 extent inside the image, with no pointer arithmetic wrap.
// keyCount is SIGNED 16-bit: admit 0..32767 for copying, also <= the point-vector
// storage ceiling kMotionPointLimit=0x6666666. Negative values sign-extend then
// cast to unsigned requiredKeys (huge allocation request) while the signed copy
// loop runs zero times; they are not empty groups. Runtime buffer size/capacity
// count keys, not bytes. Zero keys can be parsed but cannot be sampled; one key
// uses direct-copy sampling, >=2 uses interpolation. Neither storage capacity
// nor representability establishes valid durations/phases/denominators above.
int CameraScript_ParseGroupData(Manager* self, PackedCameraScript* file) {
    // Trust the packed file's offsets/counts: no file length or signature is
    // checked here. The input stays borrowed; all motion keys are copied.
    PackedCameraGroup* record;
    unsigned* groupFileOffsets;
    unsigned groupByteOffset;
    int groupIndex;
    // Resize the outer group vector to match the file's logical group count.
    self->groupCount = file->groupCount;
    unsigned int oldGroupCount = self->storedGroupCount;
    unsigned int fileGroupCount = file->groupCount;
    if (fileGroupCount > oldGroupCount)
        CameraMotionGroupVector_AppendDefaultGroups((char*)self + 8, fileGroupCount - oldGroupCount);
    else if (fileGroupCount < oldGroupCount)
        CameraMotionGroupVector_RemoveBackGroups((RemovalVector*)((char*)self + 8), oldGroupCount - fileGroupCount);
    groupFileOffsets = file->groupOffsets;
    for (groupIndex = 0; groupIndex < (int)file->groupCount; ++groupIndex) {
        unsigned int fileByteOffset = groupFileOffsets[groupIndex];
        groupByteOffset = GroupOffset(groupIndex);
        record = (PackedCameraGroup*)((char*)file + fileByteOffset);
        // Preserve the packed header and expand timing fields to runtime floats.
        PackedCameraGroup* dst = (PackedCameraGroup*)((char*)self->groups + groupByteOffset);
        dst->formatBytes[0] = record->formatBytes[0];
        dst->formatBytes[1] = record->formatBytes[1];
        dst->formatBytes[2] = record->formatBytes[2];
        dst->formatBytes[3] = record->formatBytes[3];
        dst->duration = record->duration;
        dst->keyCount = record->keyCount;
        dst->interpolation = record->interpolation;
        dst->accelerationTime = record->accelerationTime;
        dst->cruiseTime = record->cruiseTime;
        dst->decelerationTime = record->decelerationTime;
        ((ParsedGroup*)((char*)self->groups + groupByteOffset))->duration = (float)record->duration;
        ((ParsedGroup*)((char*)self->groups + groupByteOffset))->keyCount = record->keyCount;
        ((ParsedGroup*)((char*)self->groups + groupByteOffset))->accelerationTime = (float)record->accelerationTime;
        ((ParsedGroup*)((char*)self->groups + groupByteOffset))->cruiseTime = (float)record->cruiseTime;
        ((ParsedGroup*)((char*)self->groups + groupByteOffset))->decelerationTime = (float)record->decelerationTime;
        if (EventTiming_IsDurationScalingEnabled() != 0) {
            float s = kFloat_0_8333333_CameraDuration;
            ((ParsedGroup*)((char*)self->groups + groupByteOffset))->duration *= s;
            ((ParsedGroup*)((char*)self->groups + groupByteOffset))->accelerationTime *= s;
            ((ParsedGroup*)((char*)self->groups + groupByteOffset))->cruiseTime *= s;
            ((ParsedGroup*)((char*)self->groups + groupByteOffset))->decelerationTime *= s;
        }
        switch (record->interpolation) {
        case 0:
            ((ParsedGroup*)((char*)self->groups + groupByteOffset))->interpolation = 0;
            break;
        case 1:
            ((ParsedGroup*)((char*)self->groups + groupByteOffset))->interpolation = 1;
            break;
        case 2:
            ((ParsedGroup*)((char*)self->groups + groupByteOffset))->interpolation = 2;
            break;
        default:
            ((ParsedGroup*)((char*)self->groups + groupByteOffset))->interpolation = 0;
            break;
        }
        // Resize this group's owned key buffer. During growth, temp describes
        // the constructed suffix plus the copied prefix until the allocation swap.
        ParsedGroup* g = (ParsedGroup*)((char*)self->groups + groupByteOffset);
        unsigned int requiredKeys = (unsigned int)record->keyCount;
        if (requiredKeys > g->size) {
            unsigned int additionalKeys = requiredKeys - g->size;
            unsigned int cap = g->cap;
            PointAllocator allocator;
            if (additionalKeys > cap || g->size > cap - additionalKeys) {
                GrowthCapacity(g->cap, g->size + additionalKeys - cap);
            }
            if (additionalKeys <= cap && g->size <= cap - additionalKeys) {
                PointAllocator allocator;
                Pt* p = g->data + g->size;
                unsigned int cnt = additionalKeys;
                while (cnt != 0) {
                    Pt tmp;
                    new (p) Pt(tmp);
                    ++p;
                    ++g->size;
                    --cnt;
                }
            } else {
                unsigned need = g->size + additionalKeys - g->cap;
                PointBuffer temp(&g->cap);
                unsigned newCapacity = GrowthCapacity(g->cap, need);
                temp.data = (Pt*)AllocatePoints(newCapacity);
                temp.cap = newCapacity;
                temp.offset = g->size;
                Pt* p = temp.data + g->size + temp.size;
                {
                    PointAllocator allocator;
                    while (additionalKeys != 0) {
                        Pt tmp;
                        new (p) Pt(tmp);
                        ++temp.size;
                        ++p;
                        --additionalKeys;
                    }
                }
                Pt* src = g->data + g->size;
                Pt* dst = temp.data + temp.offset;
                {
                    PointAllocator allocator;
                    while (src > g->data) {
                        --dst;
                        --src;
                        new (dst) Pt(*src);
                        --temp.offset;
                        ++temp.size;
                    }
                }
                g->size = 0;
                unsigned constructedKeys = temp.size;
                unsigned oldCapacity = g->cap;
                Pt* fresh = temp.data;
                g->cap = temp.cap;
                Pt* oldKeyData = g->data;
                g->data = fresh;
                temp.cap = oldCapacity;
                temp.data = oldKeyData;
                g->size = constructedKeys;
                temp.size = 0;
            }
        } else if (requiredKeys < g->size) {
            unsigned remove = g->size - requiredKeys;
            PointAllocator allocator;
            g->size -= remove;
        }
        // Copy packed keys after the header, then derive both distance/speed pairs.
        MotionItem* incoming = (MotionItem*)((char*)record + sizeof(PackedCameraGroup));
        for (int j = 0; j < (int)record->keyCount; ++j, ++incoming) {
            *(MotionItem*)((char*)((ParsedGroup*)((char*)self->groups + groupByteOffset))->data + PointOffset(j)) =
                *incoming;
        }
        CameraMotionGroupVector_ComputeTrackLengths((GroupRecords*)self, groupIndex);
    }
    return 1;
}

// Choose group-vector capacity with overflow protection.
inline unsigned GroupGrowth(unsigned cp, unsigned need) {
    PointAllocator allocator;
    if (need > kMotionGroupLimit - cp)
        PointVectorThrow();
    if (cp < kMotionGroupGrowthTier1)
        return cp + VectorMax((cp + 1) * 3 / 5, need);
    if (cp < kMotionGroupGrowthTier2)
        return cp + VectorMax((cp + 1) / 2, need);
    return kMotionGroupLimit;
}

#pragma push
// Outer growth policy: multi-register saves stay enabled; strength reduction
// stays off for group byte cursors; propagation stays off across reserve/copy/
// swap so old capacity and new capacity remain separate values. CSE returns to
// the unit default here (unlike teardown); lifetimes stay off to retain the
// replacement guard's constructed suffix/prefix and retired-buffer scopes.
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs reset
#pragma opt_lifetimes off
// Append empty groups, moving existing groups into a guarded allocation when necessary.
// vector is a required GroupVector, not a complete player. additionalGroups is
// a count, not bytes; invalid/overflow growth throws before publication. Guards
// build the suffix and deep-copy the prefix before swapping allocation ownership.
// Key allocations remain per-group; outer capacity is not a playback duration.
void CameraMotionGroupVector_AppendDefaultGroups(void* vector, unsigned int additionalGroups) {
    register GroupVector* v = (GroupVector*)vector;
    PointAllocator allocator;

    // 0x8023B114: Finish in-place insertion before entering the growth transaction.
    do {
        if (additionalGroups > v->capacity || v->size > v->capacity - additionalGroups) {
            unsigned needed = v->size + additionalGroups - v->capacity;
            unsigned cp = *(volatile unsigned*)&v->capacity;
            GroupGrowth(cp, needed);
            break;
        } else {
            Group* p = v->data + v->size;
            PointAllocator fillAllocator;
            while (additionalGroups != 0) {
                {
                    Group tmp;
                    new (p) Group(tmp);
                }
                ++p;
                ++v->size;
                --additionalGroups;
            }
        }
        return;
    } while (0);
    {
        unsigned needed = v->size + additionalGroups - v->capacity;

        GroupBuffer temp(&v->capacity);
        unsigned oldCapacityForGrowth = v->capacity;
        if (needed > kMotionGroupLimit - oldCapacityForGrowth)
            PointVectorThrow();
        unsigned capacity;
        if (oldCapacityForGrowth < kMotionGroupGrowthTier1) {
            capacity = oldCapacityForGrowth + VectorMax((3 * (oldCapacityForGrowth + 1)) / 5, needed);
        } else if (oldCapacityForGrowth < kMotionGroupGrowthTier2) {
            capacity = oldCapacityForGrowth + VectorMax((oldCapacityForGrowth + 1) / 2, needed);
        } else {
            capacity = kMotionGroupLimit;
        }
        PointAllocator blockAllocator;
        if (capacity > kMotionGroupLimit)
            PointVectorThrow();

        temp.data = (Group*)s_vcgAllocatorCallbacks.allocate(capacity * 0x44);
        temp.capacity = capacity;
        temp.offset = v->size;
        Group* p = temp.data + temp.offset + temp.size;
        PointAllocator fillAllocator;
        while (additionalGroups != 0) {
            {
                Group tmp;
                new (p) Group(tmp);
            }
            --additionalGroups;
            ++temp.size;
            ++p;
        }

        // New suffix is constructed; deep-copy the old prefix backwards.
        Group* begin = v->data;
        Group* src = begin + v->size;
        Group* dst = temp.data + temp.offset;
        PointAllocator moveAllocator;
        while (src > begin) {
            --src;
            --dst;
            new (dst) Group(*src);
            --temp.offset;
            ++temp.size;
        }

        // Commit only after construction succeeds; destroy the old nested keys
        // before the guard frees the old outer allocation.
        Group* oldData = v->data;
        Group* oldBegin = oldData + temp.offset;
        Group* oldEnd = oldBegin + v->size;
        unsigned oldSize = v->size;
        unsigned oldCapacity = v->capacity;
        Group* data = temp.data;
        unsigned size = temp.size;
        unsigned newCapacity = temp.capacity;
        temp.data = oldData;
        PointAllocator swapAllocator;
        v->capacity = newCapacity;
        temp.capacity = oldCapacity;
        v->data = data;
        v->size = size;
        temp.size = oldSize;
        {
            Group* p = oldEnd;
            while (p > oldBegin) {
                --p;
                p->~Group();
            }
        }
        temp.size = 0;
    }
}
#pragma pop

// Destroy the requested suffix of groups and reduce the vector size.
// Requires 0 <= count <= v->size; releases nested keys but retains outer capacity.
void CameraMotionGroupVector_RemoveBackGroups(RemovalVector* v, int count) {
    int remaining = count;
    RemovalGroup* p = v->data + v->size;
    v->size -= count;
    while (remaining != 0) {
        --p;
        p->~RemovalGroup();
        --remaining;
    }
}

// Sample two position tracks, planar direction and the scalar key value at the given time.
// Requires index >= 0, valid output storage and nonnegative script time. A
// single key copies directly; all other groups must contain at least two keys.
// Returns 0 for an unavailable group/out-of-duration time, otherwise 1. Inputs
// and output are borrowed; no storage ownership transfers.
// Camera consumers interpret the output as eye/target/up/FOV (schema above).
// All pre-work failure branches leave out unchanged. A single key is bit-copied;
// multiple keys publish positions, up and interpolated FOV without further checks.
// Keep output separate from source keys because interpolation writes before all
// source components have been consumed. Sampling duration itself is allowed.
// Direct runtime callers must supply interpolation 0=linear, 1=Bezier, 2=cubic
// whenever keyCount!=1, plus valid phase/length denominators (ParseGroupData
// contract). The file parser maps unknown packed modes to runtime linear; this
// sampler does not normalize or validate manually built runtime records. Invalid
// runtime mode skips all position writes and leaves context.index/fraction
// uninitialized, then still calls direction/FOV interpolation: undefined reads,
// possible invalid key accesses/partial output, not a clean return-0 failure.
// No default pose or unchanged-output guarantee applies to that invalid input.
int GroupVector_SampleMotionAtTime(GroupRecords* v, MotionItem* out, int index, float time) {
    GroupValues values;
    if (!GetGroupValues(v, &values, index))
        return 0;
    if (time > values.duration)
        return 0;
    if (values.keyCount == 1) {
        if (index >= v->size)
            return 0;
        GroupRecord* p = &v->data[index];
        if (*(short*)(p->prefix + 6) <= 0)
            return 0;
        *out = *(MotionItem*)p->buffer.data;
        return 1;
    }
    // Integrate timing once, then dispatch position interpolation by group mode.
    float integratedTime = IntegrateMotionScriptTime(&values, time);
    MotionContext context;
    GroupRecord* record = &v->data[index];
    context.values = &values;
    context.data = (MotionItem*)record->buffer.data;
    switch (*(volatile int*)&context.values->interpolation) {
    case kLinearMotion: {
        int segment;
        float secondResidualDistance, firstResidualDistance;
        float secondTrackDelta[3], firstTrackDelta[3];
        GroupVector_FindMotionSegmentAtTime((int)v, &segment, &firstResidualDistance, &secondResidualDistance, &context,
                                            integratedTime);
        context.index = segment;
        Vec3_Subtract(firstTrackDelta, &context.data[segment + 1].firstPositionTrack,
                      &context.data[segment].firstPositionTrack);
        Vec3_Scale(firstTrackDelta, firstTrackDelta, context.fraction);
        Vec3_Add(&out->firstPositionTrack, firstTrackDelta, &context.data[segment].firstPositionTrack);
        Vec3_Subtract(secondTrackDelta, &context.data[segment + 1].secondPositionTrack,
                      &context.data[segment].secondPositionTrack);
        Vec3_Scale(secondTrackDelta, secondTrackDelta, context.fraction);
        Vec3_Add(&out->secondPositionTrack, secondTrackDelta, &context.data[segment].secondPositionTrack);
        break;
    }
    case kBezierMotion:
        GroupVector_SampleBezierMotion((int)v, (int)out, (SplineArg*)&context, integratedTime);
        break;
    case kCubicMotion:
        GroupMotion_SampleCubicVectorPair((int)v, (char*)out, (SplineArg*)&context, integratedTime);
        break;
    }
    // All modes publish a segment/fraction used by direction and scalar interpolation.
    GroupVector_InterpolatePlanarDirection((int)v, (char*)out, (SplineArg*)&context);
    MotionItem* p = &context.data[context.index];
    out->value = context.fraction * (p[1].value - p[0].value) + p[0].value;
    return 1;
}

// Return the number of stored motion groups through the output pointer.
int GroupVector_GetCount(GroupVector* v, unsigned* out) {
    *out = v->size;
    return 1;
}

// Return one group's timing and track parameters when its index is valid.
int GroupVector_GetValues(GroupRecords* v, GroupValues* out, int index) {
    return GetGroupValues(v, out, index);
}

// Clear nested key buffers, destroy all groups and reset the logical group count.
int CameraMotionGroupVector_Clear(ClearRecords* v) {
    RemovalGroup* g;
    for (int i = 0; i < v->size; ++i) {
        g = &v->data[i];
        g->buffer.size -= ((char*)g->buffer.data + g->buffer.size) - (char*)g->buffer.data;
    }
    RemovalGroup* p;
    int remaining = v->used;
    p = v->data + remaining;
    v->used -= remaining;
    while (remaining != 0) {
        --p;
        p->~RemovalGroup();
        --remaining;
    }
    v->size = 0;
    return 1;
}

// Sum segment lengths for both tracks and derive their constant-speed scales.
// Require index>=0; index>=logical count returns 0 unchanged. Nonempty key
// storage must match keyCount; duration+cruiseTime must be nonzero. Returns 1
// after writing both length/speed pairs. Length is script-coordinate distance;
// speed is that distance per integrated tick. No camera/world transform occurs.
int CameraMotionGroupVector_ComputeTrackLengths(GroupRecords* v, int index) {
    if (index >= v->size)
        return 0;
    int offset = 0;
    GroupRecord* p = &v->data[index];
    int next;
    float sum[2] = {0, 0};
    float delta[3];
    for (index = 0; index < (int)p->values.keyCount - 1; ++index, offset += sizeof(SmallItem)) {
        next = (index + 1) * sizeof(SmallItem);
        Vec3_Subtract(delta, (char*)p->buffer.data + next, (char*)p->buffer.data + offset);
        sum[0] += Vec3_Length(delta);
        Vec3_Subtract(delta, (char*)p->buffer.data + next + 0xc, (char*)p->buffer.data + offset + 0xc);
        float length = Vec3_Length(delta);
        sum[1] += length;
    }
    p->values.firstTrackLength = sum[0];
    p->values.secondTrackLength = sum[1];
    p->values.firstTrackSpeed =
        (kCameraTrackSpeedNumeratorFactor * sum[0]) / (p->values.duration + p->values.cruiseTime);
    p->values.secondTrackSpeed =
        (kCameraTrackSpeedNumeratorFactor * sum[1]) / (p->values.duration + p->values.cruiseTime);
    return 1;
}

// Choose three-word-vector capacity while rejecting an overflowing request.
inline unsigned VecGrowth(unsigned cp, unsigned need) {
    if (need > kMotionVectorLimit - cp)
        PointVectorThrow();
    if (cp < kMotionVectorGrowthTier1)
        return cp + VectorMax((cp + 1) * 3 / 5, need);
    if (cp < kMotionVectorGrowthTier2)
        return cp + VectorMax((cp + 1) / 2, need);
    return kMotionVectorLimit;
}
// Bezier compiler policy (retain this source shape, not a gameplay setting):
// strength reduction off keeps keyIndex*0x28 byte cursors independent from the
// shrinking de Casteljau control count; propagation off keeps the two tracks'
// length/normalized-time temporaries and allocation-swap values explicit;
// CSE reset permits the unit default rather than inheriting teardown's CSE-off;
// lifetimes off retains allocation tags/guard scopes through reserve/copy/swap.
// No claim is made that each switch alone is necessary for byte identity;
// preserving the combined policy avoids changing EH scheduling or FP order.
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs reset
#pragma opt_lifetimes off
// Evaluate the two Bezier position tracks using temporary control-point vectors.
// Required: sample/cfg/points and writable MotionItem at outputAddress; cfg.count
// >=2, with count complete 0x28-byte keys. Each key's XYZ tracks are at +0/+0xC;
// +0x18 onward is direction/scalar data, sampled by the caller after this return.
// cfg is the GroupValues view documented above, not the serialized key header.
// Complete local state map (both tracks share this one temporary allocation):
//   controlPoints {data,size,cap}: owned XYZ[cap], resized once to count;
//   workCount: initially missing controls, later the de Casteljau pass 1..count-1;
//   keyByteOffset: keyIndex*0x28; controlIndex: surviving control in each pass;
//   trackLength/normalizedTime: current track's length and speed*time/length;
//   first/secondTrackDelta: (right-left)*normalizedTime, added to left IN ORDER;
//   segmentIndex/residuals: separate polyline lookup after both positions publish.
// Zero track length selects t=0; t is otherwise unclamped (extrapolation possible).
// Output ledger: write XYZ at +0, then XYZ at +0xC; segment lookup writes
// sample->value (the MotionContext fraction), then we publish sample->index.
// Output direction/scalar stay unchanged here. Scratch controls
// are overwritten from source keys before track two, never reused as its input.
// Allocation/length exceptions propagate before position writes; stack VecStore
// and VecGuard release their owned buffers. The source keys are always borrowed.
// Reserve/copy/swap scopes below expose the allocator's exception ownership:
// guard owns replacement until swap, then owns retired storage until destruction.
// The allocator must return usable storage or throw: null is not checked before
// placement construction. count==1 belongs to SampleMotionAtTime's direct-copy
// path; calling this helper with one key would publish segment index -1.
// Keep these scopes and arithmetic order: changing their shape changes EH/FPU code.
void GroupVector_SampleBezierMotion(s32 forwardedContext, s32 outputAddress, SplineArg* sample, f32 integratedTime) {
    // outputAddress is a 32-bit ABI pointer to a MotionItem; only its two
    // position triplets are written here. Coordinate space is inherited from the
    // script; this function applies no world/view transform. integratedTime is
    // script time; normalizedTime is dimensionless after speed/length scaling.
    // Phase 1: reserve a temporary XYZ vector for all control keys. Its destructor owns
    // storage on both normal and exceptional exits; the source keys stay borrowed.
    u32 capacityIncrease;
    s32 keyByteOffset;
    s32 workCount;
    u32 scratchCapacity;
    VecStore controlPoints;
    MotionVec firstTrackDelta;
    MotionVec secondTrackDelta;
    f32 normalizedTime;
    f32 trackLength;
    s32 controlIndex;
    f32 firstResidualDistance;
    f32 secondResidualDistance;
    s32 segmentIndex;
    s32 keyIndex;
    controlPoints.data = 0;
    controlPoints.size = 0;
    controlPoints.cap = 0;
    VecStore* scratchVector = &controlPoints;
    u32 existingControlCount = scratchVector->size;
    u32 controlKeyCount = (u32)sample->cfg->count;
    if (controlKeyCount > existingControlCount) {
        workCount = controlKeyCount - existingControlCount;
        AllocTag resizeAlloc;
        // 0x8023A378 unit: preserve construction and resize cleanup order.
        do {
            if (workCount > existingControlCount || existingControlCount > existingControlCount - workCount) {
                PointAllocator allocator;
                VecGrowth(controlPoints.cap, (controlPoints.size + workCount) - existingControlCount);
            } else {
                AllocTag insertAlloc;
                MotionVec* p = controlPoints.data + controlPoints.size;
                while (workCount != 0) {
                    MotionVec z = {0, 0, 0};
                    new (p) MotionVec(z);
                    --workCount;
                    ++p;
                    ++controlPoints.size;
                }
                break;
            }

            {
                VecGuard_u8023A378 guard(&controlPoints.cap);
                capacityIncrease = (controlPoints.size + workCount) - controlPoints.cap;
                scratchCapacity = VecGrowth(controlPoints.cap, capacityIncrease);
                AllocTag reserveAlloc;
                if (scratchCapacity > kMotionVectorLimit) {
                    PointVectorThrow();
                }

                {
                    MotionVec* replacementData = (MotionVec*)((void* (*)(u32))s_vcgAllocatorCallbacks.allocate)(
                        scratchCapacity * kVectorXYZBytes);
                    MotionVec* p;
                    MotionVec* src;
                    MotionVec* dst;
                    MotionVec* old;
                    u32 retiredCapacity;
                    u32 c;

                    guard.data = replacementData;
                    guard.cap = scratchCapacity;
                    guard.remaining = controlPoints.size;
                    p = (MotionVec*)((char*)replacementData + controlPoints.size * kVectorXYZBytes +
                                     guard.size * kVectorXYZBytes);
                    AllocTag constructAlloc;
                    while (workCount != 0) {
                        MotionVec z = {0, 0, 0};
                        new (p) MotionVec(z);
                        ++guard.size;
                        --workCount;
                        ++p;
                    }
                    src = (MotionVec*)((char*)controlPoints.data + controlPoints.size * kVectorXYZBytes);
                    dst = (MotionVec*)((char*)guard.data + guard.remaining * kVectorXYZBytes);
                    MotionVec* begin = controlPoints.data;
                    AllocTag copyAlloc;
                    while (src > begin) {
                        --dst;
                        --src;
                        new (dst) MotionVec(*src);
                        --guard.remaining;
                        ++guard.size;
                    }
                    old = controlPoints.data;
                    retiredCapacity = controlPoints.cap;
                    AllocTag swapAlloc;
                    controlPoints.cap = guard.cap;
                    guard.cap = retiredCapacity;
                    controlPoints.data = guard.data;
                    guard.data = old;
                    controlPoints.size = guard.size;
                    guard.size = 0;
                }
            }
        } while (0);
        ;
    } else if (controlKeyCount < existingControlCount) {
        AllocTag eraseAlloc;
        controlPoints.size -= existingControlCount - controlKeyCount;
    }
    // Phase 2: evaluate the first track with de Casteljau reduction: each pass replaces
    // adjacent controls by lerp(left,right,normalizedTime), leaving the result in element zero.
    normalizedTime = kFloat_0_GroupVector;
    trackLength = sample->cfg->firstTrackLength;
    if (normalizedTime != trackLength) {
        normalizedTime = sample->cfg->firstTrackSpeed * integratedTime / trackLength;
    }

    for (keyIndex = 0, keyByteOffset = 0; keyIndex < sample->cfg->count; keyByteOffset += kMotionKeyBytes, ++keyIndex) {
        Vector_CopyComponents(&controlPoints.data[keyIndex], sample->points + keyByteOffset);
    }

    for (workCount = 1; workCount <= sample->cfg->count - 1; ++workCount) {
        for (controlIndex = 0; controlIndex <= (sample->cfg->count - 1) - workCount; ++controlIndex) {
            Vec3_Subtract(&firstTrackDelta, &controlPoints.data[controlIndex + 1], &controlPoints.data[controlIndex]);
            Vec3_Scale(&firstTrackDelta, &firstTrackDelta, normalizedTime);
            Vec3_Add(&controlPoints.data[controlIndex], &controlPoints.data[controlIndex], &firstTrackDelta);
        }
    }

    Vector_CopyComponents((void*)outputAddress, controlPoints.data);

    // Phase 3: refill the same scratch vector from the second position in every key.
    trackLength = sample->cfg->secondTrackLength;
    normalizedTime = kFloat_0_GroupVector;
    if (normalizedTime != trackLength) {
        normalizedTime = sample->cfg->secondTrackSpeed * integratedTime / trackLength;
    }

    for (keyIndex = 0, keyByteOffset = 0; keyIndex < sample->cfg->count; keyByteOffset += kMotionKeyBytes, ++keyIndex) {
        Vector_CopyComponents(&controlPoints.data[keyIndex], sample->points + keyByteOffset + kSecondTrackOffset);
    }

    for (workCount = 1; workCount <= sample->cfg->count - 1; ++workCount) {
        for (controlIndex = 0; controlIndex <= (sample->cfg->count - 1) - workCount; ++controlIndex) {
            Vec3_Subtract(&secondTrackDelta, &controlPoints.data[controlIndex + 1], &controlPoints.data[controlIndex]);
            Vec3_Scale(&secondTrackDelta, &secondTrackDelta, normalizedTime);
            Vec3_Add(&controlPoints.data[controlIndex], &controlPoints.data[controlIndex], &secondTrackDelta);
        }
    }

    Vector_CopyComponents((void*)(outputAddress + kSecondTrackOffset), controlPoints.data);

    // Phase 4: position evaluation is global, but direction/scalar still need a local segment.
    GroupVector_FindMotionSegmentAtTime(forwardedContext, &segmentIndex, &firstResidualDistance,
                                        &secondResidualDistance, sample, integratedTime);
    sample->index = segmentIndex;
}

// Cubic/segment helpers retain explicit byte-stride and interpolation
// temporaries (strength reduction/propagation off). CSE keeps the unit default;
// lifetime optimization returns to default now that Bezier's allocation-guard
// scopes have ended. Keep the two tracks' arithmetic order and endpoint copies;
// these directives are code-generation policy, not permission to reassociate FP.
#pragma opt_strength_reduction off
#pragma opt_propagation off
#pragma opt_common_subs reset
#pragma opt_lifetimes reset
// Motion vectors stored through the vcg allocator callbacks.
typedef std::vector<MotionVec, vcg_allocator<MotionVec> > MotionVecVector;
// Destroy a motion vector; free the object too when deleting > 0.
MotionVecVector* dtor_8023CDA8(MotionVecVector* self, int deleting) {
    return std::__vector_deleting_dtor(self, deleting);
}

// Evaluate both cubic position tracks for the selected segment.
// sample requires >=2 initialized keys and valid GroupValues; output needs one
// MotionItem and must not overlap keys. Writes only eye/target and sample's
// segment/fraction; the caller supplies up/FOV afterwards. No allocation occurs.
void GroupMotion_SampleCubicVectorPair(s32 forwardedContext, char* output, SplineArg* sample, f32 integratedTime) {
    MotionVec tangent0[2];
    MotionVec tangent1[2];
    MotionVec delta0;
    MotionVec temp0;
    MotionVec delta1;
    MotionVec temp1;
    f32 firstResidualDistance, secondResidualDistance;
    s32 segmentIndex;
    GroupVector_FindMotionSegmentAtTime(forwardedContext, &segmentIndex, &firstResidualDistance,
                                        &secondResidualDistance, sample, integratedTime);
    sample->index = segmentIndex;
    // The last argument is integrated motion time. Segment lookup produces u
    // in [0,1] for Hermite weights: h0=2u^3-3u^2+1,
    // h2=u^3-2u^2+u, h3=u^3-u^2.
    f32 u = sample->value;
    f32 square = u * u;
    f32 h3 = square * u - square;
    f32 h2 = h3 - square;
    f32 h0 = kFloat_1_Spline + (h2 + h3);
    h2 += u;
    char* points = sample->points;
    s32 current = segmentIndex;
    Vec3_Subtract(&delta0, points + (current + 1) * kMotionKeyBytes, points + current * kMotionKeyBytes);
    if (kFloat_0_GroupVector != Vec3_Length(&delta0)) {
        s32 lowKeyIndex = segmentIndex - 1;
        s32 highKeyIndex = segmentIndex + 1;
        if (lowKeyIndex < 0)
            lowKeyIndex = 0;
        if (highKeyIndex >= sample->cfg->count)
            highKeyIndex = sample->cfg->count - 1;
        Vec3_Subtract(&tangent0[0], sample->points + highKeyIndex * kMotionKeyBytes,
                      sample->points + lowKeyIndex * kMotionKeyBytes);
        if (highKeyIndex - lowKeyIndex > 1)
            Vec3_Scale(&tangent0[0], &tangent0[0], (f32)(highKeyIndex - lowKeyIndex));
        lowKeyIndex = segmentIndex;
        highKeyIndex = segmentIndex + 2;
        if (lowKeyIndex < 0)
            lowKeyIndex = 0;
        if (highKeyIndex >= sample->cfg->count)
            highKeyIndex = sample->cfg->count - 1;
        Vec3_Subtract(&tangent0[1], sample->points + highKeyIndex * kMotionKeyBytes,
                      sample->points + lowKeyIndex * kMotionKeyBytes);
        if (highKeyIndex - lowKeyIndex > 1)
            Vec3_Scale(&tangent0[1], &tangent0[1], (f32)(highKeyIndex - lowKeyIndex));
        Vec3_Scale(output, sample->points + segmentIndex * kMotionKeyBytes, h0);
        Vec3_Scale(&temp0, sample->points + (segmentIndex + 1) * kMotionKeyBytes, kFloat_1_Spline - h0);
        Vec3_Add(output, output, &temp0);
        Vec3_Scale(&temp0, &tangent0[0], h2);
        Vec3_Add(output, output, &temp0);
        Vec3_Scale(&temp0, &tangent0[1], h3);
        Vec3_Add(output, output, &temp0);
    } else {
        Vector_CopyComponents(output, sample->points + segmentIndex * kMotionKeyBytes);
    }
    // Repeat for track two. Tangents retain the original multiplication by
    // (highKeyIndex-lowKeyIndex); replacing it by a conventional average would change this curve.
    points = sample->points;
    current = segmentIndex;
    Vec3_Subtract(&delta1, points + (current + 1) * kMotionKeyBytes + kSecondTrackOffset,
                  points + current * kMotionKeyBytes + kSecondTrackOffset);
    if (kFloat_0_GroupVector != Vec3_Length(&delta1)) {
        s32 highKeyIndex = segmentIndex + 1;
        s32 lowKeyIndex = segmentIndex - 1;
        if (lowKeyIndex < 0)
            lowKeyIndex = 0;
        if (highKeyIndex >= sample->cfg->count)
            highKeyIndex = sample->cfg->count - 1;
        Vec3_Subtract(&tangent1[0], sample->points + highKeyIndex * kMotionKeyBytes + kSecondTrackOffset,
                      sample->points + lowKeyIndex * kMotionKeyBytes + kSecondTrackOffset);
        if (highKeyIndex - lowKeyIndex > 1)
            Vec3_Scale(&tangent1[0], &tangent1[0], (f32)(highKeyIndex - lowKeyIndex));
        lowKeyIndex = segmentIndex;
        highKeyIndex = segmentIndex + 2;
        if (lowKeyIndex < 0)
            lowKeyIndex = 0;
        if (highKeyIndex >= sample->cfg->count)
            highKeyIndex = sample->cfg->count - 1;
        Vec3_Subtract(&tangent1[1], sample->points + highKeyIndex * kMotionKeyBytes + kSecondTrackOffset,
                      sample->points + lowKeyIndex * kMotionKeyBytes + kSecondTrackOffset);
        if (highKeyIndex - lowKeyIndex > 1)
            Vec3_Scale(&tangent1[1], &tangent1[1], (f32)(highKeyIndex - lowKeyIndex));
        Vec3_Scale(output + kSecondTrackOffset, sample->points + segmentIndex * kMotionKeyBytes + kSecondTrackOffset,
                   h0);
        Vec3_Scale(&temp1, sample->points + (segmentIndex + 1) * kMotionKeyBytes + kSecondTrackOffset,
                   kFloat_1_Spline - h0);
        Vec3_Add(output + kSecondTrackOffset, output + kSecondTrackOffset, &temp1);
        Vec3_Scale(&temp1, &tangent1[0], h2);
        Vec3_Add(output + kSecondTrackOffset, output + kSecondTrackOffset, &temp1);
        Vec3_Scale(&temp1, &tangent1[1], h3);
        Vec3_Add(output + kSecondTrackOffset, output + kSecondTrackOffset, &temp1);
    } else {
        Vector_CopyComponents(output + kSecondTrackOffset,
                              sample->points + segmentIndex * kMotionKeyBytes + kSecondTrackOffset);
    }
}

// Consume segment lengths and choose a segment and fraction using the faster track.
// integratedTime is integrated script time, not a world-space length. The outputs are
// residual distances in the selected segment; sample->value receives its fraction.
// Requires >=2 keys. Equal track speeds choose track two; exhaustion selects
// the final segment at fraction 1. A zero-length selected segment instead uses 0.
void GroupVector_FindMotionSegmentAtTime(s32 unused, s32* segmentIndex, f32* firstTrackDistance,
                                         f32* secondTrackDistance, void* sampleContext, f32 integratedTime) {
    SplineArg* p = (SplineArg*)sampleContext;
    MotionVec tmp;
    f32 lengths[2];
    *firstTrackDistance = p->cfg->firstTrackSpeed * integratedTime;
    *secondTrackDistance = p->cfg->secondTrackSpeed * integratedTime;
    *segmentIndex = -1;
    s32 offset;
    s32 i;
    i = 0;
    offset = 0;
    for (; i < p->cfg->count - 1; offset += kMotionKeyBytes, ++i) {
        char* point = (char*)(offset + (u32)p->points);
        Vec3_Subtract(&tmp, point + kMotionKeyBytes, point);
        lengths[0] = Vec3_Length(&tmp);
        point = (char*)(offset + (u32)p->points);
        Vec3_Subtract(&tmp, point + (kMotionKeyBytes + kSecondTrackOffset), point + kSecondTrackOffset);
        lengths[1] = Vec3_Length(&tmp);
        if (p->cfg->firstTrackSpeed > p->cfg->secondTrackSpeed) {
            if (lengths[0] >= *firstTrackDistance) {
                *segmentIndex = i;
                if (lengths[0] == kFloat_0_GroupVector)
                    p->value = kFloat_0_GroupVector;
                else
                    p->value = *firstTrackDistance / lengths[0];
                break;
            }
        } else {
            if (lengths[1] >= *secondTrackDistance) {
                *segmentIndex = i;
                if (lengths[1] == kFloat_0_GroupVector)
                    p->value = kFloat_0_GroupVector;
                else
                    p->value = *secondTrackDistance / lengths[1];
                break;
            }
        }
        *firstTrackDistance -= lengths[0];
        *secondTrackDistance -= lengths[1];
    }
    if (*segmentIndex == -1) {
        f32 one = kFloat_1_Spline;
        *segmentIndex = p->cfg->count - 2;
        p->value = one;
    }
}

// Interpolate the stored direction pair through the shortest arc and write (sin,cos,0).
// Angles are radians; camera consumers use the result as their up vector.
// Required sample.index and index+1 must be valid keys; output needs 0x28 bytes.
// Writes only +18/+1C/+20, leaving positions and FOV untouched. The ABI context
// argument is unused. Preserve the strict +/-pi tests and approximate sin/cos.
void GroupVector_InterpolatePlanarDirection(s32 unused, char* output, SplineArg* sample) {
    f32 angles[2];
    f32 y = *(f32*)(sample->points + sample->index * kMotionKeyBytes + kDirectionSecondComponentOffset);
    angles[0] = atan2(*(f32*)(sample->points + sample->index * kMotionKeyBytes + kDirectionOffset), y);
    y = *(f32*)(sample->points + sample->index * kMotionKeyBytes + (kMotionKeyBytes + kDirectionSecondComponentOffset));
    angles[1] =
        atan2(*(f32*)(sample->points + sample->index * kMotionKeyBytes + (kMotionKeyBytes + kDirectionOffset)), y);
    f32 delta = angles[1] - angles[0];
    if (delta < kFloat_neg3_1415927_AngleInterpolation)
        delta += kFloat_6_2831855_AngleInterpolation;
    if (delta > kFloat_3_1415927_AngleInterpolation)
        delta -= kFloat_6_2831855_AngleInterpolation;
    f32 angle = sample->value * delta + angles[0];
    if (angle < kFloat_neg3_1415927_AngleInterpolation)
        angle += kFloat_6_2831855_AngleInterpolation;
    if (angle > kFloat_3_1415927_AngleInterpolation)
        angle -= kFloat_6_2831855_AngleInterpolation;
    *(f32*)(output + kDirectionOffset) = Math_SinApprox(angle);
    f32 cosine = Math_CosApprox(angle);
    f32 zero = kFloat_0_GroupVector;
    *(f32*)(output + kDirectionSecondComponentOffset) = cosine;
    *(f32*)(output + 0x20) = zero;
}

// Release a group's nested vector and optionally delete the containing group.
StoreOwner* dtor_8023D548(StoreOwner* p, s32 deleting) {
    if (p) {
        ((VcgVector*)&p->store)->~VcgVector();
        if (deleting > 0)
            __dl__FPv(p);
    }
    return p;
}

// Release the vector allocation and optionally delete the wrapper.
VecStore* dtor_8023D5D8(VecStore* p, s32 deleting) {
    if (p) {
        if (p->data) {
            u32 count = p->size;
            p->size -= count;
            ((void (*)(void*))s_vcgAllocatorCallbacks.destroy)(p->data);
        }
        if (deleting > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// Reset vector size, free its allocation, and optionally delete the wrapper.
VecStore* dtor_8023D654(VecStore* p, s32 deleting) {
    if (p) {
        p->size = 0;
        if (p) {
            if (p->data) {
                p->size = 0;
                ((void (*)(void*))s_vcgAllocatorCallbacks.destroy)(p->data);
            }
        }
        if (deleting > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

#pragma push
#pragma use_lmw_stmw on
// Destroy groups in [first,first+size), release outer storage, and optionally delete self.
OwnerStore* OwnerStore_dtor(OwnerStore* p, s32 deleting) {
    if (p) {
        StoreOwner* end;
        StoreOwner* begin;
        begin = p->data + p->first;
        end = begin + p->size;
        while (end > begin) {
            --end;
            destroyOwner(end);
        }
        p->size = 0;
        if (p) {
            if (p->data) {
                u32 count = p->size;
                p->size = 0;
                StoreOwner* end = p->data + count;
                while (count != 0) {
                    --end;
                    destroyOwner(end);
                    --count;
                }
                ((void (*)(void*))s_vcgAllocatorCallbacks.destroy)(p->data);
            }
        }
        if (deleting > 0)
            __dl__FPv(p);
    }
    return p;
}
#pragma pop

// Reset vector size and release its allocation through the alternate cleanup entry.
VecStore* dtor_8023D824(VecStore* p, s32 deleting) {
    if (p) {
        p->size = 0;
        if (p) {
            if (p->data) {
                p->size = 0;
                ((void (*)(void*))s_vcgAllocatorCallbacks.destroy)(p->data);
            }
        }
        if (deleting > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

// Initialize the motion/fade helper module's two-pi cache.
void MotionFade_InitAngleCache() {
    s_motionFadeTwoPiCache = kFloat_6_2831855;
}

// Reset fade state and create a texture sized to the current viewport.
// Required fresh caller storage; this overwrites an old resource without releasing
// it, so call ResourceState_Dtor before reinitializing a live owner. Returns p
// even on creation/clear failure; readiness is published separately. Framebuffer
// ownership is one shared reference, released by ResourceState_Dtor or fade-out.
ResourceState* FadeOverlay_Init(ResourceState* p) {
    ResourceInfo info;
    ResourceConfig config;
    ResourceBuffer buffer;
    p->resource = 0;
    p->option11 = 0;
    p->option12 = 0;
    p->releaseOnCompletion = 0;
    p->opacity = kFloat_0_805E0218;
    p->opacityStep = kFloat_0_805E0218;
    p->phase = kFadeIdle;
    p->captureRequested = 0;
    Engine_GetViewport(&config);
    if (Texture_Create2D(config.width, config.height, 1, 0, kFramebufferTextureFormat, 0, p, 0)) {
        FinishResourceLoad(p, &info, &buffer);
    }
    return p;
}

// Release the shared fade texture and optionally delete its owner.
ResourceState* ResourceState_Dtor(ResourceState* p, s32 deleting) {
    if (p) {
        SharedResource* resource = p->resource;
        if (resource) {
            if (--resource->refs == 0)
                CubeResourceStorage_dtor(resource, 1);
            p->resource = 0;
        }
        s_fadeOverlayTextureReady = 0;
        if (deleting > 0)
            __dl__FPv(p);
    }
    return p;
}

// Reset the fade phase, opacity, opacity step and pending capture flag.
// Required ResetState alias; retains the resource, option bytes and global ready
// flag. This is a clock/state reset, not texture destruction or cancellation cleanup.
void FadeState_Reset(ResetState* p) {
    p->phase = kFadeIdle;
    p->opacity = kFloat_0_805E0218;
    p->opacityStep = kFloat_0_805E0218;
    p->captureRequested = 0;
}

// Capture a requested frame, advance opacity, and release the texture after a completed fade.
// Required ResourceState. FadeState_StartFadeIn/StartFadeOut (fade_overlay_and_shared_file.cpp) produce
// opacityStep=255.0f/(float)duration for positive integer update-call durations;
// duration zero snaps the endpoint and sets step zero. Requests in the same
// active direction are ignored, so they do not repair an invalid existing step.
// Intended ramp invariant: finite opacity in [0,255], finite opacityStep>=0;
// positive step is required for progress from an interior opacity. Zero holds
// an interior ramp forever; a negative step moves away from its endpoint;
// NaN defeats endpoint comparisons. No validation/absolute-value correction is
// done here. Increasing a valid step shortens fades; there is no delta-seconds
// multiplier. Capture requests are consumed even if
// allocation/clear fails. Completion clamps opacity and changes phase; optional
// release decrements exactly one shared reference. There is no catch-up loop.
void FadeState_Update(ResourceState* p) {
    // Service a pending framebuffer capture before changing this update's opacity.
    if (p->captureRequested) {
        if (acquireResource(p)) {
            Engine_CopyFramebufferToTexture(0, 0, p->resource, 0, 0, 0, 0, 0, 0, 0, 0, kFloat_0_805E0218);
        }
    }
    // Clamp at endpoints; a completed decreasing fade may release the shared texture.
    if ((s32)p->phase == kFadeDecreasing) {
        p->opacity -= p->opacityStep;
        if (p->opacity <= kFloat_0_805E0218) {
            p->opacity = kFloat_0_805E0218;
            p->phase = kFadeIdle;
            if (s_releaseFadeResourceOnCompletion || p->releaseOnCompletion) {
                SharedResource* resource = p->resource;
                if (resource) {
                    if (--resource->refs == 0)
                        CubeResourceStorage_dtor(resource, 1);
                    p->resource = 0;
                }
                s_fadeOverlayTextureReady = 0;
                p->releaseOnCompletion = 0;
            }
        }
    } else if ((s32)p->phase == kFadeIncreasing) {
        p->opacity += p->opacityStep;
        if (p->opacity >= kFloat_255_Fade) {
            p->opacity = kFloat_255_Fade;
            p->phase = kFadeOpaque;
        }
    }
    p->captureRequested = 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462280", _ctors_u8023A378_0, VCGPlayer_InitAngleCache);
STATIC_INIT_ENTRY(".CRT$XCU80462280", _ctors_u8023A378_1, VCGSource_InitAngleCache);
STATIC_INIT_ENTRY(".CRT$XCU80462280", _ctors_u8023A378_2, MotionFade_InitAngleCache);
#pragma pop
