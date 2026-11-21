#include <decomp/static_init.h>
#include "ppc_double_words.h"
#include <decomp/ct_member.h>
#include "classes/CEventFxPositionLayout.h"
#define AsCEventFxPosition(p) ((CEventFxPositionLayout*)(p))
#include "classes/CEventModelMoveLayout.h"
#include "classes/CEventModelRotateLayout.h"
#include "classes/CEventFadeLayout.h"
#define AsModelMove(p) ((CEventModelMoveLayout*)(p))
#define AsModelRotate(p) ((CEventModelRotateLayout*)(p))
#define AsEventFade(p) ((CEventFadeLayout*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char headTrackingStrings[];
}
}

extern "C" void appFreeUni__FPv(void*);

// Own a completed string only while its enclosing operation can unwind.
struct EH5StringCleanup {
    const unsigned int* words;
    bool active;
    // Arm release of a completed heap-backed string if the next operation throws.
    explicit EH5StringCleanup(const void* p) : words((const unsigned int*)p), active(true) {}
    // Transfer responsibility to the normal-path owner.
    void release() { active = false; }
    // Free only strings whose high ownership bit denotes heap storage.
    ~EH5StringCleanup() {
        if (active && (words[0] & 0x80000000u)) appFreeUni__FPv((void*)words[2]);
    }
};

#include "game/singletons.h"
#include "game/eventconte.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "game/types.h"
#include "classes/CEventFx.h"
#include "classes/CEventModelSC.h"
#include "classes/CEventModelSC_MDL_MOT.h"
#include "classes/CEventEffectSC.h"
#include <decomp/cw_bitfields.h>

#define AsCEventEffectSC(p) ((CEventEffectSC*)(p))

#define AsCEventModelSC_MDL_MOT(p) ((CEventModelSC_MDL_MOT*)(p))

#define AsCEventModelSC(p) ((CEventModelSC*)(p))

#define AsCEventFx(p) ((CEventFx*)(p))
// Construct records in already allocated vector storage without allocating again.
inline void* operator new(size_t, void* p) { return p; }
// Placement construction failure does not own the caller's backing storage.
inline void operator delete(void*, void*) {}

// ---- merged from event_script_model_commands.cpp ----
namespace ns0 {

// Translation unit 0x8021F0F4-0x80220730 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

struct FacingModelView {
    char padding00[0x14];
    unsigned int rotationDirty;
    char padding18[0x14];
    float headingDegrees;
};

struct FacingEventState {
    char padding00[0x04];
    unsigned int duration;
    unsigned char mode;
    char padding09[3];
    FacingModelView* first;
    FacingModelView* second;
    float firstStartDegrees;
    float secondStartDegrees;
    float firstTurnDegrees;
    float secondTurnDegrees;
};

struct NamedEventStringState {
    unsigned char active;
    char padding01[3];
    unsigned int ptr;
    union { unsigned int flags; unsigned char bytes[4]; };
    unsigned int count;
    void* object;
};

struct Fn8021FAB4Input { char padding00[4]; unsigned int count; unsigned char mode; unsigned char first; unsigned char second; };
typedef Fn8021FAB4Input HeadResetCommand;
struct HeadResetEventState {
    void* vtable; unsigned int count; unsigned char mode; char padding09[3];
    void* first; void* second; float firstStart; float secondStart;
    float firstDelta; float secondDelta; unsigned int firstHeadBone; unsigned int secondHeadBone;
};
struct HeadResetUpdateState { char padding00[4]; unsigned int count; unsigned char mode; char padding09[3]; void* first; void* second; float firstStart; float secondStart; float firstDelta; float secondDelta; unsigned int firstHeadBone; unsigned int secondHeadBone; };

extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern int EventTiming_IsDurationScalingEnabled(void*);
extern void CourseSlotTable_InitEmpty(void*);
extern void* SlotTable_GetEntry(void*, unsigned int);
extern void* WPuppet_FindOrCacheBone(void*, const char*);
extern void WPuppet_SetBoneControllerVector(void*, void*, float*);
extern char __vt__28CEventModelSC_MDL_HEAD_RESET[];
extern char __vt__22CEventModelSC_MDL_HEAD[];
extern signed char s_courseSlotTableInitialized;
extern char s_conteModelSlots[];
extern const char headTrackingStrings[];
extern const float kEventDurationScaleNumerator_ModelScript;
extern const float kEventDurationScaleDenominator_ModelScript;
extern const float kFloat_0_017453292_EventModel;
extern const float kRadiansToDegrees_EventModel, kFloat_0_EventMotion, kFloat_360_eventModel, kFloat_180_EventMotion, kFloat_neg180_EventModel;
extern const float kHeadTrackingBodyTurnCutoffDegrees;
extern float fabsf(float);
extern char __vt__22CEventModelSC_MDL_FACE[];
extern double atan2(double, double) __attribute__((nothrow));

struct FacingModelPositionView {
    void* vtable; void* puppet; char padding08[0x14];
    float x;
    float y;
    float z;
    char padding28[4];
    float angle;
    char padding30[0x10];
    float headYawDegrees;
};
// Position is world-space; angle is the body's world heading in degrees.
// headYawDegrees is relative to that heading. Horizontal target bearing uses
// atan2(source.x-target.x, target.z-source.z), so its sign must be preserved.
// All command count/duration words are elapsed-frame limits, not actor counts.
struct Fn8021F0F4Input { char padding00[4]; unsigned int count; unsigned char mode, first, second; };
typedef Fn8021F0F4Input FacingCommand;
struct FacingSetupState {
    void* vtable; unsigned int count; unsigned char mode; char padding09[3];
    FacingModelPositionView* first; FacingModelPositionView* second;
    float firstAngle, secondAngle, deltaFirst, deltaSecond;
};

struct ModelPositionVector { float x, y, z; };

// Compute the horizontal heading from one model to the other in degrees.
static inline float HeadingDegreesBetweenModels(FacingModelPositionView* p, FacingModelPositionView* q) {
    ModelPositionVector qv = *(ModelPositionVector*)&q->x;
    ModelPositionVector pv = *(ModelPositionVector*)&p->x;
    float dz = qv.z - pv.z;
    float dx = pv.x - qv.x;
    return kRadiansToDegrees_EventModel * (float)atan2(dx, dz);
}

// Wrap one turn delta to the inclusive -180 to +180 degree range.
static inline float WrapShortestTurnDegrees(float d) {
    if (kFloat_180_EventMotion < d) d -= kFloat_360_eventModel;
    if (d < kFloat_neg180_EventModel) d += kFloat_360_eventModel;
    return d;
}

// Apply the event timing ratio to facing durations of at least two frames.
static inline unsigned int ScaleFacingDuration(void* self, unsigned int n) {
    if (EventTiming_IsDurationScalingEnabled(self)) {
        if (n < 2) return n;
        return Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_ModelScript * (float)n / kEventDurationScaleDenominator_ModelScript);
    }
    return n;
}

} }
struct CEventModelSC_MDL_FACE : CEventConte {
    unsigned int count; unsigned char mode; char padding09[3];
    ns0::FacingModelPositionView* first; ns0::FacingModelPositionView* second;
    float firstAngle, secondAngle, deltaFirst, deltaSecond;
    CEventModelSC_MDL_FACE(ns0::FacingCommand* input);
};
// Capture initial body headings and shortest turns toward the other actor.
CEventModelSC_MDL_FACE::CEventModelSC_MDL_FACE(ns0::FacingCommand* input) {
    using namespace ns0;
    CEventModelSC_MDL_FACE* self = this;
    self->vtable = __vt__22CEventModelSC_MDL_FACE;
    unsigned int count = input->count;
    count = ScaleFacingDuration(self, count);
    self->count = count;
    unsigned char mode = input->mode;
    self->mode = mode;
    if (((int&)Sing_805DC668()) == 0) {
        if (s_courseSlotTableInitialized == 0) {
            CourseSlotTable_InitEmpty(&s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        ((int&)Sing_805DC668()) = (int)&s_conteModelSlots;
    }
    void* manager = (void*)((int&)Sing_805DC668());
    self->first = (FacingModelPositionView*)SlotTable_GetEntry(manager, input->first);
    self->second = (FacingModelPositionView*)SlotTable_GetEntry(manager, input->second);
    self->firstAngle = self->first->angle;
    self->secondAngle = self->second->angle;
    float angle = HeadingDegreesBetweenModels(self->first, self->second);
    if (angle < kFloat_0_EventMotion) angle += kFloat_360_eventModel;
    if (kFloat_360_eventModel <= angle) angle -= kFloat_360_eventModel;
    self->deltaFirst = angle;
    angle = HeadingDegreesBetweenModels(self->second, self->first);
    if (angle < kFloat_0_EventMotion) angle += kFloat_360_eventModel;
    if (kFloat_360_eventModel <= angle) angle -= kFloat_360_eventModel;
    self->deltaSecond = angle;
    self->deltaFirst = WrapShortestTurnDegrees(self->deltaFirst - self->firstAngle);
    self->deltaSecond = WrapShortestTurnDegrees(self->deltaSecond - self->secondAngle);
}
namespace ns0 { extern "C" {

struct Fn8021F4A4Input { char padding00[4]; unsigned int count; unsigned char mode, first, second; };
typedef Fn8021F4A4Input HeadTrackingCommand;
struct HeadTrackingSetupState {
    void* vtable; unsigned int count; unsigned char mode; char padding09[3];
    FacingModelPositionView* first; FacingModelPositionView* second;
    float firstAngle, secondAngle, deltaFirst, deltaSecond;
    unsigned int firstHeadBone, secondHeadBone;
};
// Setup/update views share offsets. first/second borrow actor slots; head-bone
// handles are borrowed from their puppets. On entry firstAngle/secondAngle hold
// body headings. A successful head-bone lookup changes each start to relative
// head yaw and recomputes its delta from body heading + existing head yaw.
// Without a bone those same fields retain world-space body start/delta values.
// Head tracking is attempted only within 90 degrees of the body's forward heading.

// Apply the event timing ratio to head-tracking durations of at least two frames.
static inline unsigned int ScaleHeadTrackingDuration(void* self, unsigned int n) {
    if (EventTiming_IsDurationScalingEnabled(self)) {
        if (n < 2) return n;
        return Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_ModelScript * (float)n / kEventDurationScaleDenominator_ModelScript);
    }
    return n;
}

// Lazily initialize and return the story-scene model-slot table.
static inline void* GetConteModelSlots() {
    if (((int&)Sing_805DC668()) == 0) {
        if (s_courseSlotTableInitialized == 0) {
            CourseSlotTable_InitEmpty(&s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        ((int&)Sing_805DC668()) = (int)&s_conteModelSlots;
    }
    return (void*)((int&)Sing_805DC668());
}

// Subtract two headings and select their shortest signed turn.
static inline float ShortestTurnDegreesBetween(float a, float b) {
    float d = a - b;
    if (kFloat_180_EventMotion < d) d -= kFloat_360_eventModel;
    if (d < kFloat_neg180_EventModel) d += kFloat_360_eventModel;
    return d;
}

} }
struct CEventModelSC_MDL_HEAD : CEventConte {
    char data[40];
    CEventModelSC_MDL_HEAD(ns0::HeadTrackingCommand* input);
};
// Find head controllers for nearby targets, with body-turn fallback.
CEventModelSC_MDL_HEAD::CEventModelSC_MDL_HEAD(ns0::HeadTrackingCommand* input) {
    // Phase 1: scale duration and borrow the two actor records from the slot table.
    using namespace ns0;
    HeadTrackingSetupState* self = (HeadTrackingSetupState*)this; self->vtable = &__vt__22CEventModelSC_MDL_HEAD;
    unsigned int count = input->count;
    count = ScaleHeadTrackingDuration(self, count);
    self->count = count;
    int current = ((int&)Sing_805DC668());
    unsigned char mode = input->mode;
    self->mode = mode;
    if (current == 0) {
        if (s_courseSlotTableInitialized == 0) {
            CourseSlotTable_InitEmpty(&s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        ((int&)Sing_805DC668()) = (int)&s_conteModelSlots;
    }
    void* manager = (void*)((int&)Sing_805DC668());
    self->first = (FacingModelPositionView*)SlotTable_GetEntry(manager, input->first);
    self->second = (FacingModelPositionView*)SlotTable_GetEntry(manager, input->second);
    // Phase 2: capture world-space target bearings and shortest body-turn deltas.
    float headingFirst;
    float headingSecond;
    headingFirst = HeadingDegreesBetweenModels(self->first, self->second);
    if (headingFirst < kFloat_0_EventMotion) headingFirst += kFloat_360_eventModel;
    if (kFloat_360_eventModel <= headingFirst) headingFirst -= kFloat_360_eventModel;
    headingSecond = HeadingDegreesBetweenModels(self->second, self->first);
    if (headingSecond < kFloat_0_EventMotion) headingSecond += kFloat_360_eventModel;
    if (kFloat_360_eventModel <= headingSecond) headingSecond -= kFloat_360_eventModel;
    float delta;
    self->firstAngle = self->first->angle;
    self->secondAngle = self->second->angle;
    self->deltaFirst = ShortestTurnDegreesBetween(headingFirst, self->firstAngle);
    self->deltaSecond = WrapShortestTurnDegrees(headingSecond - self->secondAngle);
    // Phase 3: substitute a head-local yaw animation when its bone is available.
    // Each actor can independently use head tracking or whole-body fallback.
    self->secondHeadBone = 0;
    self->firstHeadBone = 0;
    if ((float)__fabs((double)self->deltaFirst) <= kHeadTrackingBodyTurnCutoffDegrees) {
        void* puppet = ((FacingModelPositionView*)self->first)->puppet;
        if (puppet) {
            unsigned int firstHeadBone = (unsigned int)WPuppet_FindOrCacheBone(puppet, headTrackingStrings);
            self->firstHeadBone = firstHeadBone;
            if (firstHeadBone) {
                self->firstAngle += self->first->headYawDegrees;
                self->deltaFirst = ShortestTurnDegreesBetween(headingFirst, self->firstAngle);
                self->firstAngle = self->first->headYawDegrees;
            }
        }
    }
    if ((float)__fabs((double)self->deltaSecond) <= kHeadTrackingBodyTurnCutoffDegrees) {
        void* puppet = ((FacingModelPositionView*)self->second)->puppet;
        if (puppet) {
            unsigned int secondHeadBone = (unsigned int)WPuppet_FindOrCacheBone(puppet, headTrackingStrings);
            self->secondHeadBone = secondHeadBone;
            if (secondHeadBone) {
                self->secondAngle += self->second->headYawDegrees;
                self->deltaSecond = ShortestTurnDegreesBetween(headingSecond, self->secondAngle);
                self->secondAngle = self->second->headYawDegrees;
            }
        }
    }
}
namespace ns0 { extern "C" {

extern void CourseSlotTable_InitEmpty(void*);
extern void* __nwa__FUl(unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern char __vt__21CEventModelSC_MDL_MOT[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern const char headTrackingStrings[];
extern char __vt__Q23std12length_error[];
extern signed char s_courseSlotTableInitialized;
extern char s_conteModelSlots[];
extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct Alloc { char c __attribute__((aligned(4))); };

struct Str;
extern void String_ReserveCapacity(Str*, unsigned int);
extern Str* String_ReplaceRange(Str*, unsigned int, unsigned int, const char*, const char*, const Alloc&);

struct Str {
    union {
        unsigned int flag;
        struct { CW_BITS2( unsigned int heapbit:1, unsigned int capacity:31) };
        struct { CW_BITS2( unsigned char byteheap:1, unsigned char smallsize:7) };
    };
    unsigned int size;
    char* ptr;

    // Initialize an empty inline string; the high flag bit selects heap storage.
    Str() {
        flag = 0; size = 0; ptr = 0;
    }
    // Clear contents in place without releasing any reserved heap capacity.
    void clearEmpty() {
        if (!(flag >> 31)) { ((char*)this)[1] = 0; smallsize = 0; }
        else { *ptr = 0; size = 0; }
    }
    // Copy inline bytes or allocate/copy heap contents; never share the character buffer.
    Str(const Str& s) {
        unsigned int f = s.flag;
        if (!(f >> 31)) { unsigned int n = s.size; flag = f; size = n; ptr = s.ptr; }
        else {
            flag = 0; size = 0; ptr = 0;
            String_ReserveCapacity(this, s.size);
            Alloc a; Alloc copy = a;
            String_ReplaceRange(this, 0, 0, s.ptr, s.ptr + s.size, copy);
        }
    }

    // Select logical length from the active inline/heap representation.
    unsigned int length() const { return !heapbit ? smallsize : size; }
    // Replace the full string from a borrowed NUL-terminated source.
    void assign(const char* s) {
        unsigned int n = length();
        Alloc a; Alloc copy = a;
        unsigned int len = strlen(s);
        String_ReplaceRange(this, 0, n, s, s + len, copy);
    }
};

struct ModelMotionCommand {
    char padding00[1];
    unsigned char key;
    unsigned char mode;
    char padding03;
    unsigned int value;
    char string[0x14];
};
struct Record {
    unsigned char mode;
    char pad01[3];
    unsigned int value;
    Str text;

    // Initialize a blank queued motion record.
    Record() : mode(0), value(0), text() { text.clearEmpty(); }
    // Copy motion metadata and independently owned text.
    Record(const Record& o) : mode(o.mode), value(o.value), text(o.text) {}
    // Clear metadata/text and free its heap character storage if present.
    ~Record() {
        mode = 0;
        value = 0;
        text.clearEmpty();
        if ((char*)&text != 0 && (text.flag >> 31) != 0) appFreeUni__FPv(text.ptr);
    }
};
struct ModelMotionQueue {
    char pad00[0x18];
    unsigned int ready;
    char pad1C[0x18];
    Record* records;
    unsigned int size;
    unsigned int capacity;
};
// Select the larger vector-growth request by reference.
static inline const unsigned int& maxu(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }
// Destroy the constructed record range in reverse order for exception cleanup.
static inline void destroyRecords(Record* first, Record* last) {
    while (last > first) { --last; last->~Record(); }
}

struct RecordTemp {
    Record* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* allocator;
    unsigned int offset;
    // Release temporary storage; the existing size reset precedes its cleanup loop.
    ~RecordTemp() {
        size = 0;
        if (data) {
            Record* p = data + size;
            for (unsigned int i = size; i != 0; --i) {
                --p;
                p->~Record();
            }
            appFreeUni__FPv(data);
        }
    }
};

namespace std {
struct exception {
    void* vtable;
    char* message;
    void* owner;
    // Copy exception text and transfer its allocation to a shared control block.
    exception(const char* const& text) {
        vtable = __vt__Q23std11logic_error;
        Alloc allocator0 = {0};
        Alloc allocator = {0};
        unsigned int length = strlen(text) + 1;
        message = (char*)__nwa__FUl(length * 2 - length);
        {
            struct Buffer {
                char* data;
                void* allocator;
                // Delete the message buffer only until its control block takes ownership.
                ~Buffer() { if (data) __dla__FPv(data); }
            };
            Buffer buffer;
            char* allocation = message;
            buffer.data = allocation;
            buffer.allocator = &allocator;
            struct Shared {
                void* vtable;
                int strong, weak;
                char* data;
                // Adopt the allocated exception text with initial strong/weak counts.
                Shared(char* allocation) {
                    strong = 1; weak = 1;
                    vtable = s_sharedCharDeleterVtable;
                    data = allocation;
                }
            };
            Shared* control = new Shared(allocation);
            owner = control;
            buffer.data = 0;
        }
        strcpy(message, text);
    }
    // Delegate exception ownership cleanup to the runtime destructor.
    ~exception() { __dt__Q23std9exceptionFv(this, 0); }
};
struct logic_error : public exception {
    // Construct a logic error with the copied base exception message.
    logic_error(const char* text) : exception(text) {}
};
class length_error : public logic_error {
public:
    // Select the length-error runtime type after base message construction.
    length_error(const char* text) : logic_error(text) { vtable = __vt__Q23std12length_error; }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
// Actor selection used by face/head commands; both actors turn when mode is zero.
enum { kTurnBothActors = 0, kTurnFirstActor = 1, kTurnSecondActor = 2 };
struct HeadTrackingStringPool {
    char headBone[11]; char vectorLengthError[20]; char lengthErrorType[55];
};
#define HEAD_STRING_OFFSET(member) ((int)&((HeadTrackingStringPool*)0)->member)

// Throw the head-tracking vector's length error with its verified pool strings.
static inline void throwLength() { {
    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
    std::length_error* error = (std::length_error*)storage.bytes;
    error->__ct(headTrackingStrings + HEAD_STRING_OFFSET(vectorLengthError));
    exception_runtime::__throw(exception_runtime::headTrackingStrings + HEAD_STRING_OFFSET(lengthErrorType), error,
        (void*)exception_runtime::__dt__Q23std12length_errorFv);
} }
#undef LENGTH_ERROR1
#define LENGTH_ERROR1() throwLength()
#undef LENGTH_ERROR2
#define LENGTH_ERROR2() throwLength()
#undef LENGTH_ERROR3
#define LENGTH_ERROR3() throwLength()

#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Append a named motion command to the model queue and mark it ready for processing.
void* CEventModelSC_MDL_MOT_Ctor(void* self, ModelMotionCommand* input) {
    // Phase 1: borrow the actor queue and copy the command text into a local record.
    int current = ((int&)Sing_805DC668());
    char* str = input->string;
    *(void**)self = __vt__21CEventModelSC_MDL_MOT;
    if (current == 0) {
        if (!s_courseSlotTableInitialized) {
            CourseSlotTable_InitEmpty(&s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        ((int&)Sing_805DC668()) = (int)&s_conteModelSlots;
    }
    AsCEventModelSC_MDL_MOT(self)->modelSlot = SlotTable_GetEntry((void*)((int&)Sing_805DC668()), input->key);

    Record rec;
    rec.mode = input->mode;
    rec.value = input->value;
    rec.text.assign(str);

    ModelMotionQueue* manager = (*(ModelMotionQueue**)&AsCEventModelSC_MDL_MOT(self)->modelSlot);
    Alloc capacityAllocator = {0};
    // Phase 2: append in place or grow under RecordTemp's allocation guard.
    if (manager->size < manager->capacity) {
        Record* dst = manager->records + manager->size;
        new (dst) Record(rec);
        ++manager->size;
    } else {
        Alloc growthAllocator = {0};
        Alloc limitAllocator = {0};
        if (0x0CCCCCCCU - manager->capacity < 1U) LENGTH_ERROR1();
        unsigned int needed = manager->size + 1 - manager->capacity;
        RecordTemp temp;
        temp.data = 0;
        unsigned int newCapacity;
        unsigned int capacity = manager->capacity;
        temp.size = 0;
        temp.capacity = 0;
        temp.allocator = &manager->capacity;
        temp.offset = 0;
        if (needed > 0x0CCCCCCCU - capacity) LENGTH_ERROR2();
        if (capacity < 0x04444444U) {
            unsigned int growth = 3 * (capacity + 1) / 5;
            newCapacity = capacity + maxu(growth, needed);
        } else if (capacity < 0x08888888U) {
            unsigned int growth = (capacity + 1) >> 1;
            newCapacity = capacity + maxu(growth, needed);
        } else {
            newCapacity = 0x0CCCCCCCU;
        }
        Alloc allocationAllocator = {0};
        if (newCapacity > 0x0CCCCCCCU) LENGTH_ERROR3();
        Record* allocation = (Record*)appMallocDummy__FUl(newCapacity * 20);
        unsigned int oldSize = manager->size;
        temp.data = allocation;
        temp.capacity = newCapacity;
        Record* dst = allocation + oldSize + temp.size;
        temp.offset = oldSize;
        Alloc insertAllocator = {0};
        unsigned int count = 1;
        while (count != 0) {
            new (dst) Record(rec);
            --count;
            ++dst;
            ++temp.size;
        }
        // Copy the existing prefix in reverse before committing the new storage.
        Record* begin = manager->records;
        Alloc copyAllocator = {0};
        Record* end = begin + manager->size;
        dst = temp.data + temp.offset;
        while (end > begin) {
            --dst; --end;
            new (dst) Record(*end);
            --temp.offset;
            ++temp.size;
        }
        // Commit allocation/size, then destroy old records and release old storage.
        unsigned int oldCapacity = manager->capacity;
        manager->capacity = temp.capacity;
        temp.capacity = oldCapacity;
        Record* oldData = manager->records;
        manager->records = temp.data;
        temp.data = oldData;
        unsigned int oldSize2 = manager->size;
        Alloc exchangeAllocator = {0};
        manager->size = temp.size;
        temp.size = oldSize2;
        destroyRecords(temp.data + temp.offset, temp.data + temp.offset + oldSize2);
    }
    // Phase 3: publish readiness only after the record is fully appended.
    ((*(ModelMotionQueue**)&AsCEventModelSC_MDL_MOT(self)->modelSlot))->ready = 1;
    return self;
}

#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset
#undef LENGTH_ERROR

// Wait until the model motion queue has cleared its ready flag.
unsigned int CEventModelSC_MDL_MOT_IsMotionQueueIdle(void* self) {
    void* ptr = AsCEventModelSC_MDL_MOT(self)->modelSlot;
    return ((ModelMotionQueue*)ptr)->ready == 0;
}

#pragma opt_lifetimes off
// Clear a named-event record and release its heap string before optional deletion.
void* NamedEventStringState_dtor(NamedEventStringState* self, int deleteFlag) {
    if (self != 0) {
        unsigned int flags = self->flags;
        self->active = 0;
        unsigned int wasSet = flags >> 31;
        self->ptr = 0;
        if (wasSet == 0) {
            self->bytes[1] = 0;
            int v = self->bytes[0];
            v = v & 0xFFFFFF80;
            self->bytes[0] = (unsigned char)v;
        } else {
            *(unsigned char*)self->object = 0;
            self->count = 0;
        }
        if ((char*)self + 8 != 0 && (self->flags >> 31) != 0)
            appFreeUni__FPv(self->object);
        if (deleteFlag > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_lifetimes reset

extern const float kFloat_1_ValueAnimation;
extern const float kFloat_0_EventMotion;
extern const float kFloat_360_eventModel;
extern void WPuppet_SetBoneControllerVector(void*, void*, float*);

struct HeadTrackingModelView {
    char padding00[4];
    void* node;
    char padding08[0xC];
    unsigned int rotationDirty;
    char padding18[0x14];
    float headingDegrees;
    char padding30[0x10];
    float headYawDegrees;
};

struct HeadBoneControllerView {
    char padding00[0x68];
    void* data;
};

struct HeadTrackingUpdateState {
    void* vtable;
    unsigned int duration;
    unsigned char mode;
    char padding09[3];
    HeadTrackingModelView* first;
    HeadTrackingModelView* second;
    float firstStart;
    float secondStart;
    float firstDelta;
    float secondDelta;
    HeadBoneControllerView* firstHeadBone;
    HeadBoneControllerView* secondHeadBone;
};
// firstStart/Delta and secondStart/Delta use degrees: head-local if the matching
// bone exists, otherwise world body heading. The bone controller consumes radians
// on its local X axis with the opposite sign: X = -headYawDegrees * pi/180,
// Y = Z = 0. This is the asset's controller convention, not a world X rotation.

// Interpolate head-bone yaw, falling back to body turning when no head controller exists.
int CEventModelSC_MDL_HEAD_UpdateHeadTurn(HeadTrackingUpdateState* self, unsigned int frame) {
    // frame is elapsed frames. Zero duration applies the endpoint immediately.
    // Return 1 at/after duration; interpolation is intentionally NOT clamped, so
    // callers wanting the exact endpoint must supply duration as their last frame.
    // Phase 1: compute the common interpolation fraction.
    float rate;
    if (self->duration != 0) {
        rate = (float)frame / (float)self->duration;
    } else {
        rate = kFloat_1_ValueAnimation;
    }
    // Phase 2: apply each selected actor in its setup-selected coordinate space.
    if (!self->mode || kTurnFirstActor == self->mode) {
        if (self->firstHeadBone) { float scale = kFloat_0_017453292_EventModel;
            float rotation[3] = {0, 0, 0};

            __typeof__(self->firstDelta) savedFirstDelta = self->firstDelta; self->first->headYawDegrees = self->firstStart + savedFirstDelta * rate;
            rotation[0] = scale * -self->first->headYawDegrees;
            WPuppet_SetBoneControllerVector(self->first->node, self->firstHeadBone->data, rotation);
            } else if (self->first->node) {
            float angle = self->firstStart + self->firstDelta * rate;
            if (angle < kFloat_0_EventMotion) angle += kFloat_360_eventModel;
            if (kFloat_360_eventModel <= angle) angle -= kFloat_360_eventModel;
            self->first->headingDegrees = angle;
            self->first->rotationDirty = 1;
        }
    }
    if (!self->mode || kTurnSecondActor == self->mode) {
        if (self->secondHeadBone) { float scale = kFloat_0_017453292_EventModel;
            float rotation[3] = {0, 0, 0};

            __typeof__(self->secondDelta) savedSecondDelta = self->secondDelta; self->second->headYawDegrees = self->secondStart + savedSecondDelta * rate;
            rotation[0] = scale * -self->second->headYawDegrees;
            WPuppet_SetBoneControllerVector(self->second->node, self->secondHeadBone->data, rotation);
        } else if (self->second->node) {
            float angle = self->secondStart + self->secondDelta * rate;
            if (angle < kFloat_0_EventMotion) angle += kFloat_360_eventModel;
            if (kFloat_360_eventModel <= angle) angle -= kFloat_360_eventModel;
            self->second->headingDegrees = angle;
            self->second->rotationDirty = 1;
        }
    }
    return frame >= self->duration;
}

// Interpolate selected actors toward one another and mark their body rotations dirty.
int CEventModelSC_MDL_FACE_UpdateActorHeadings(FacingEventState* self, unsigned int frame) {
    float rate;
    if (!self->duration) {
        rate = kFloat_1_ValueAnimation;
    } else {
        rate = (float)frame / (float)self->duration;
    }
    if (!self->mode || self->mode == kTurnFirstActor) {
        float angle = self->firstStartDegrees + self->firstTurnDegrees * rate;
        if (angle < kFloat_0_EventMotion)
            angle += kFloat_360_eventModel;
        if (kFloat_360_eventModel <= angle)
            angle -= kFloat_360_eventModel;
        self->first->headingDegrees = angle;
        self->first->rotationDirty = 1;
    }
    if (0 == self->mode || kTurnSecondActor == self->mode) {
        float angle = self->secondStartDegrees + self->secondTurnDegrees * rate;
        if (angle < kFloat_0_EventMotion)
            angle += kFloat_360_eventModel;
        if (kFloat_360_eventModel <= angle)
            angle -= kFloat_360_eventModel;
        self->second->headingDegrees = angle;
        self->second->rotationDirty = 1;
    }
    if (frame < self->duration)
        return 0;
    return 1;
}

} }
struct CEventModelSC_MDL_HEAD_RESET : CEventConte {
    char data[40];
    CEventModelSC_MDL_HEAD_RESET(ns0::HeadResetCommand* input);
};
// Capture current head yaw and prepare deltas back to neutral.
CEventModelSC_MDL_HEAD_RESET::CEventModelSC_MDL_HEAD_RESET(ns0::HeadResetCommand* input) {
    using namespace ns0;
    HeadResetEventState* self = (HeadResetEventState*)this; self->vtable = &__vt__28CEventModelSC_MDL_HEAD_RESET;
    unsigned int count = input->count;
    count = ScaleHeadTrackingDuration(self, count);
    self->count = count;
    self->mode = input->mode;
    if (((int&)Sing_805DC668()) == 0) {
        if (s_courseSlotTableInitialized == 0) { CourseSlotTable_InitEmpty(&s_conteModelSlots); s_courseSlotTableInitialized = 1; }
        ((int&)Sing_805DC668()) = (int)&s_conteModelSlots;
    }
    void* manager = (void*)((int&)Sing_805DC668());
    self->first = SlotTable_GetEntry(manager, input->first);
    self->second = SlotTable_GetEntry(manager, input->second);
    self->firstStart = ((FacingModelPositionView*)self->first)->headYawDegrees;
    self->firstDelta = -self->firstStart;
    self->secondStart = ((FacingModelPositionView*)self->second)->headYawDegrees;
    self->secondDelta = -self->secondStart;
    self->secondHeadBone = 0; self->firstHeadBone = 0;
    void* puppet = ((FacingModelPositionView*)self->first)->puppet;
    if (puppet) self->firstHeadBone = (unsigned int)WPuppet_FindOrCacheBone(puppet, headTrackingStrings);
    puppet = ((FacingModelPositionView*)self->second)->puppet;
    if (puppet) self->secondHeadBone = (unsigned int)WPuppet_FindOrCacheBone(puppet, headTrackingStrings);
}
namespace ns0 { extern "C" {

// Return selected head controllers to neutral yaw over the event duration.
int CEventModelSC_MDL_HEAD_RESET_ResetHeadYaw(HeadResetUpdateState* self, unsigned int frame) {
    // Same elapsed-frame/no-clamping and negative local-X radians contract as HEAD.
    // Missing controllers are left unchanged; reset does not turn the whole body.
    float rate = self->count ? (float)frame / (float)self->count : kFloat_1_ValueAnimation;
    if (!self->mode || self->mode == kTurnFirstActor) {
        if (0 != self->firstHeadBone) {

            float rotation[3] = {0, 0, 0};
            float delta = self->firstDelta;

            __typeof__(self->firstStart) savedFirstStart = self->firstStart; float k = kFloat_0_017453292_EventModel;
            ((FacingModelPositionView*)self->first)->headYawDegrees = (delta * rate + (savedFirstStart)); rotation[0] = k * -((FacingModelPositionView*)self->first)->headYawDegrees;
            WPuppet_SetBoneControllerVector(((FacingModelPositionView*)self->first)->puppet, ((HeadBoneControllerView*)self->firstHeadBone)->data, rotation);
        }
    }
    if (!self->mode || self->mode == kTurnSecondActor) {
        if (self->secondHeadBone) { float k = kFloat_0_017453292_EventModel;

            float rotation[3] = {0, 0, 0};
             float delta = self->secondDelta;

            ((FacingModelPositionView*)self->second)->headYawDegrees = (delta * rate + (self->secondStart));
            rotation[0] = k * -((FacingModelPositionView*)self->second)->headYawDegrees;
            WPuppet_SetBoneControllerVector(((FacingModelPositionView*)self->second)->puppet, ((HeadBoneControllerView*)self->secondHeadBone)->data, rotation);
        }
    }
    return frame >= self->count;
}

} // extern "C"

} // namespace ns0

// ---- merged from event_script_model_commands.cpp ----
namespace ns1 {
// Translation unit 0x80220730-0x80221980 (18 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern void __dl__FPv(void*);
extern const float kFloat_6_2831855;
extern float s_twoPi_EventEffects;
extern const float kFloat_1_ValueAnimation;

extern void appFreeUni__FPv(void*);
extern void CFx_RemoveSequence(void*, void*, int);
extern void* CFx_RemoveSpray(void*, void*);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void __ml__FRC7WVectorf(void*, void*, float);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void __ami__7WVectorFRC7WVector(void*, void*);
extern void* SlotTable_GetEntry(void*, unsigned char);
extern void CourseSlotTable_InitEmpty(void*);
extern void ScriptByteMap_FreeSubtree(void*, void*);

extern void* g_fx;
extern char __vt__8CEventFx[];
extern char __vt__12CEventEffect[];
extern char __vt__23CEventEffectSC_EFF_FADE[];
extern char s_sprayEffectExtension[];
extern char s_sceneLighting[];
extern char s_effectFade[];
extern char s_effectFadeDestructorRegistration[];
extern signed char s_effectFadeInitialized;
extern signed char s_courseSlotTableInitialized;
extern char __vt__21CEventModelSC_MDL_ROT[];
extern char __vt__22CEventModelSC_MDL_MOVE[];
extern char s_conteModelSlots[];
extern int s_eventFadeColor0Rgb;
extern int s_eventFadeColor1Rgb;
extern const double kUnsignedToDoubleBias_EventFade;
extern const float kEventDurationScaleNumerator_ModelScript;
extern const float kEventDurationScaleDenominator_ModelScript;
extern const float kEventFadeDurationScaleNumerator;
extern const float kEventFadeDurationScaleDenominator;
extern const float kEventFadeImmediateProgress;
extern const float kEventFadeAlphaScale;
extern const float kEventFxDefaultPositionComponent;
extern const float kFloat_0_EventMotion;
extern const float kFloat_360_eventModel;

extern void CEffectFade_Ctor(void*);
extern void CEffectFade_dtor();
extern void __register_global_object(void*, void*, void*);
extern void FadeQuad_SetRgb(void*, int);
extern void CEffectFade_SetAlpha(void*, int);
extern int EventTiming_IsDurationScalingEnabled();
extern void String_ReplaceRange(void*, int, unsigned int, const char*, const char*, char*);
extern int FxAttachment_Init(void*);
extern void* CFx_CreateAttachedEffect(void*, void*, void*, int, int, int);
extern int strrchr(void*, int);
extern int strcmp(void*, void*);
extern void FxSpray_NoOpLifecycleHook(void*, void*);
extern void CFxSequence_InvokeSprayLifecycleHooks(void*, void*);
extern unsigned int strlen(const char*);

typedef Vec3f V3;
// Shared slot representation used by model move/rotation commands.
struct ConteTransformModel {
    void* vtable; void* puppet; char padding08[4]; unsigned int moveMode;
    unsigned int positionDirty; unsigned int rotationDirty; char padding18[4];
    V3 position; V3 eulerDegrees;
};
struct ModelMoveCommand {
    unsigned char commandHeader[4]; unsigned int duration; unsigned char modelSlot;
    unsigned char relative; char padding0A[2]; unsigned int xBits, yBits, zBits;
    unsigned char moveMode;
};
#define AsTransformModel(value) ((ConteTransformModel*)(value))
#define AsMoveCommand(value) ((ModelMoveCommand*)(value))

struct Input209 { unsigned char unk[4]; unsigned int count; unsigned char mode; unsigned char flag; short y; };
struct Obj209 { void* vtable; unsigned int count; void* texture; V3 a; V3 b; };
struct ItemDC { unsigned char valid; unsigned char pad[3]; unsigned int value; unsigned int flags; unsigned int fieldC; unsigned char* owned; };
struct ItemArr { ItemDC* base; unsigned int count; char unk8[8]; unsigned int start; };

// Apply the event timing ratio to movement durations of at least two frames.
static inline unsigned int ScaleMoveDuration(unsigned int n) {
    if (EventTiming_IsDurationScalingEnabled()) {
        if (n < 2) return n;
        return Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_ModelScript * (float)n / kEventDurationScaleDenominator_ModelScript);
    }
    return n;
}
#pragma push
#pragma opt_common_subs off
} }
struct CEventModelSC_MDL_MOVE : CEventConte {
    char data[32];
    CEventModelSC_MDL_MOVE(void* arg);
};
// Decode the little-endian destination vector and prepare absolute or relative movement.
CEventModelSC_MDL_MOVE::CEventModelSC_MDL_MOVE(void* arg) {
    using namespace ns1;
    void* self = (void*)this; void* manager=Sing_805DC668();
    *(void**)self = __vt__22CEventModelSC_MDL_MOVE;
    if (!manager) {
        if (!s_courseSlotTableInitialized) { CourseSlotTable_InitEmpty(s_conteModelSlots); s_courseSlotTableInitialized = 1; }
        Sing_805DC668() = s_conteModelSlots;
    }
    AsModelMove(self)->object = SlotTable_GetEntry(Sing_805DC668(), AsMoveCommand(arg)->modelSlot);
    unsigned int count = AsMoveCommand(arg)->duration;
    count = ScaleMoveDuration(count);
    AsModelMove(self)->count=count;
    void* tex=AsModelMove(self)->object;
    AsTransformModel(tex)->moveMode=AsMoveCommand(arg)->moveMode;
    union Swap { unsigned int u; unsigned char b[4]; float f; } x,y,z;
    x.u=AsMoveCommand(arg)->xBits;
    y.u=AsMoveCommand(arg)->yBits;
    z.u=AsMoveCommand(arg)->zBits;
    unsigned char x0,y0,z0,x1,y1,z1;
    x0=x.b[0];
    x.b[0]=x.b[3]; x.b[3]=x0;
    x1=x.b[1]; x.b[1]=x.b[2]; x.b[2]=x1;
    y0=y.b[0];
    y.b[0]=y.b[3]; y.b[3]=y0;
    y1=y.b[1]; y.b[1]=y.b[2]; y.b[2]=y1;
    z0=z.b[0];
    z.b[0]=z.b[3]; z.b[3]=z0;
    z1=z.b[1]; z.b[1]=z.b[2]; z.b[2]=z1;
    float fx=x.f,fy=y.f,fz=z.f;
    *(WVector*)&AsModelMove(self)->startPosition = *(WVector*)&AsTransformModel(AsModelMove(self)->object)->position;
    float v[3]={fx,fy,fz};
    *(WVector*)&AsModelMove(self)->deltaPosition = *(WVector*)v;
    if(!AsMoveCommand(arg)->relative) *(WVector*)&AsModelMove(self)->deltaPosition -= *(WVector*)&AsModelMove(self)->startPosition;
}
namespace ns1 { extern "C" {
#pragma pop

#pragma push
#pragma opt_common_subs off
// Create an attached spray or sequence effect, retaining its name and position inputs.
void* CEventFx_Ctor(void* self, const char* name, void* attachment, void* positionOffset, void* basePosition, void* bypassResourceCache) {
    volatile unsigned int flagSrc;
    unsigned int allocatorFlags;
    float zero[3];
    struct Arr3 { float v[3]; } local;
    struct Stor { char a[8]; void* texture; char b[0xc]; void* pos; } storage;
    *(unsigned int*)self=0;
    AsCEventFx(self)->nameLength=0;
    (*(unsigned int*)&AsCEventFx(self)->nameData)=0;
    EH5StringCleanup eh5Cleanup0(self);
    AsCEventFx(self)->eventVtable=__vt__8CEventFx;
    unsigned int inlineNameLength=*(unsigned char*)self & 0x7f;
    *(unsigned char*)&allocatorFlags=*(volatile unsigned char*)&flagSrc;
    unsigned int len=strlen(name);
    String_ReplaceRange(self,0,inlineNameLength,name,name+len,(char*)&allocatorFlags);
    AsCEventFx(self)->texture=attachment;
    float z=kEventFxDefaultPositionComponent;
    AsCEventFx(self)->positionOffset=positionOffset;
    zero[0]=z; zero[1]=z; zero[2]=z;
    __as__7WVectorFRC7WVector(AsCEventFxPosition(self)->basePosition,zero);
    if(basePosition) __as__7WVectorFRC7WVector(AsCEventFxPosition(self)->basePosition,basePosition);
    __as__7WVectorFRC7WVector(AsCEventFxPosition(self)->position,AsCEventFxPosition(self)->basePosition);
    (*(void**)&AsCEventFx(self)->bypassResourceCache)=bypassResourceCache;
    (*(unsigned int*)&AsCEventFx(self)->spray)=0;
    (*(unsigned int*)&AsCEventFx(self)->sequence)=0;
    if(AsCEventFx(self)->spray) {
        CFx_RemoveSpray(g_fx,AsCEventFx(self)->spray);
        (*(unsigned int*)&AsCEventFx(self)->spray)=0;
    }
    if(AsCEventFx(self)->sequence) {
        CFx_RemoveSequence(g_fx,AsCEventFx(self)->sequence,0);
        (*(unsigned int*)&AsCEventFx(self)->sequence)=0;
    }
    FxAttachment_Init(&storage);
    storage.texture=AsCEventFx(self)->texture;
    storage.pos=AsCEventFxPosition(self)->position;
    void* effectName=((*(unsigned int*)self >> 31) == 0) ? (char*)self+1 : AsCEventFx(self)->nameData;
    unsigned int cacheBypassFlag=AsCEventFx(self)->bypassResourceCache;
    unsigned int nonzero=(cacheBypassFlag != 0);
    void* effect=CFx_CreateAttachedEffect(g_fx,&storage,effectName,0,0,(int)nonzero);
    if(effect) {
        void* storedEffectName=((*(unsigned int*)self >> 31) == 0) ? (char*)self+1 : AsCEventFx(self)->nameData;
        if(strcmp((void*)strrchr(storedEffectName,0x2e),s_sprayEffectExtension)==0) {
            AsCEventFx(self)->spray=effect;
            FxSpray_NoOpLifecycleHook(effect,s_sceneLighting);
        } else {
            AsCEventFx(self)->sequence=effect;
            CFxSequence_InvokeSprayLifecycleHooks(effect,s_sceneLighting);
        }
    }
    if(AsCEventFx(self)->spray && AsCEventFx(self)->sequence) {
        void* v=AsCEventFx(self)->positionOffset;
        if(v) {
            __pl__FRC7WVectorRC7WVector(&local,AsCEventFxPosition(self)->basePosition,v);
            __as__7WVectorFRC7WVector(AsCEventFxPosition(self)->position,&local);
        }
    }
    eh5Cleanup0.release(); return self;
}
#pragma pop

// Apply the event timing ratio to rotation durations of at least two frames.
static inline unsigned int ScaleRotationDuration(unsigned int n) {
    if (EventTiming_IsDurationScalingEnabled()) {
        if (n < 2) return n;
        return (unsigned int)Runtime_ConvertDoubleToUnsigned(kEventDurationScaleNumerator_ModelScript * (float)n / kEventDurationScaleDenominator_ModelScript);
    }
    return n;
}

} }
struct CEventModelSC_MDL_ROT : CEventConte {
    char data[32];
    CEventModelSC_MDL_ROT(ns1::Input209* arg);
};
// Capture Euler angles and prepare the command rotation delta.
CEventModelSC_MDL_ROT::CEventModelSC_MDL_ROT(ns1::Input209* arg) { unsigned int current = ((int&)Sing_805DC668());
    using namespace ns1;
    Obj209* self = (Obj209*)this;
    self->vtable = (void*)__vt__21CEventModelSC_MDL_ROT;
    if (current == 0) {
        if (!s_courseSlotTableInitialized) {
            CourseSlotTable_InitEmpty(s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        ((int&)Sing_805DC668()) = (int)s_conteModelSlots;
    }
    self->texture = SlotTable_GetEntry((void*)((int&)Sing_805DC668()), arg->mode);
    int count = arg->count;
    count = ScaleRotationDuration(count);
    float temp[3];
      self->count=count;
     *(WVector*)(&self->a) = *(WVector*)&AsTransformModel(self->texture)->eulerDegrees;
    __typeof__(arg->y) savedY = arg->y; __typeof__(self->a.z) savedZ = self->a.z;
    temp[0] = self->a.x;
    temp[1] = ((float)savedY);
    temp[2] = savedZ;
    *(WVector*)(&self->b) = *(WVector*)temp;
    if (!arg->flag) *(WVector*)(&self->b) -= *(WVector*)(&self->a);
}
namespace ns1 { extern "C" {

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Destroy active motion records in reverse order and release their backing allocation.
void* dtor_80220DC0(ItemArr* self, int flag) {
    unsigned int zero;
    ItemDC* low;
    ItemDC* p;
    unsigned int zero2;
    ItemDC* q;
    unsigned int n;
    if (self) {
        zero = 0;
        low = self->base + self->start;
        p = low + self->count;
        while (p > low) {
            p -= 1;
            if (p) {
                p->valid = zero;
                p->value = zero;
                if ((p->flags >> 31) == 0) {
                    *((unsigned char*)p + 9) = zero;
                    { int __b = *((unsigned char*)p + 8); __b = __b & ~0x7F; *((unsigned char*)p + 8) = (unsigned char)__b; }
                } else {
                    *p->owned = zero;
                    p->fieldC = zero;
                }
                if (&p->flags && (p->flags >> 31)) appFreeUni__FPv(p->owned);
            }
        }
        zero2 = 0;
        self->count = zero2;
        if (self) {
            if (self->base) {
                q = self->base + zero2;
                n = zero2;
                self->count = n;
                while (n != 0) {
                    q -= 1;
                    if (q) {
                        q->valid = zero;
                        q->value = zero;
                        if ((q->flags >> 31) == 0) {
                            *((unsigned char*)q + 9) = zero;
                            { int __b = *((unsigned char*)q + 8); __b = __b & ~0x7F; *((unsigned char*)q + 8) = (unsigned char)__b; }
                        } else {
                            *q->owned = zero;
                            q->fieldC = zero;
                        }
                        if (&q->flags && (q->flags >> 31)) appFreeUni__FPv(q->owned);
                    }
                    n -= 1;
                }
                appFreeUni__FPv(self->base);
            }
            if (flag > 0) __dl__FPv(self);
        }
    }
    return self;
}
#pragma pop

// Delete this command only for a positive deleting-destructor flag; return its original address.
void* CEventModelSC_MDL_FACE_dtor(void* p, int flag) {
    if (p && flag > 0) __dl__FPv(p);
    return p;
}

// Delete this command only for a positive deleting-destructor flag; return its original address.
void* CEventModelSC_MDL_HEAD_dtor(void* p, int flag) {
    if (p && flag > 0) __dl__FPv(p);
    return p;
}

// Delete this command only for a positive deleting-destructor flag; return its original address.
void* CEventModelSC_MDL_HEAD_RESET_dtor(void* p, int flag) {
    if (p && flag > 0) __dl__FPv(p);
    return p;
}

// Delete this command only for a positive deleting-destructor flag; return its original address.
void* CEventModelSC_MDL_MOT_dtor(void* p, int flag) {
    if (p && flag > 0) __dl__FPv(p);
    return p;
}

// Delete this command only for a positive deleting-destructor flag; return its original address.
void* CEventModelSC_MDL_MOVE_dtor(void* p, int flag) {
    if (p && flag > 0) __dl__FPv(p);
    return p;
}

// Delete this command only for a positive deleting-destructor flag; return its original address.
void* CEventModelSC_MDL_ROT_dtor(void* p, int flag) {
    if (p && flag > 0) __dl__FPv(p);
    return p;
}

// Initialize the event-effect angle constant to one full turn in radians.
void EventEffects_InitAngleConstant() {
    s_twoPi_EventEffects = kFloat_6_2831855;
}

#pragma push
#pragma opt_lifetimes off
// Interpolate a model position, mark it dirty and finish at the requested frame count.
int CEventModelSC_MDL_MOVE_UpdatePosition(void* self, unsigned int elapsedFrames) {

    unsigned int durationFrames = AsModelMove(self)->count;
    float progress;
    if (durationFrames != 0) {
        progress = (float)elapsedFrames;
        float duration = (float)durationFrames;
        progress = progress / duration;
    } else {
        progress = kFloat_1_ValueAnimation;
    }
    WVector position;
    WVector scaledDelta;
    __ml__FRC7WVectorf(&scaledDelta, AsModelMove(self)->deltaPosition, progress);
    __pl__FRC7WVectorRC7WVector(&position, AsModelMove(self)->startPosition, &scaledDelta);
    __as__7WVectorFRC7WVector(&AsTransformModel(AsModelMove(self)->object)->position, &position);
    (*(int*)&AsTransformModel(AsModelMove(self)->object)->positionDirty) = 1;
    if (elapsedFrames < AsModelMove(self)->count) {
        return 0;
    }
    return 1;
}
#pragma pop

// Remove the attached spray and sequence, then release the effect-name string.
void* CEventFx_dtor(void* p, int flag) {
    void* self = p;
    if (self) {
        AsCEventFx(self)->eventVtable = (void*)__vt__8CEventFx;
        void* v = AsCEventFx(self)->spray;
        if (v) {
            CFx_RemoveSpray(g_fx, v);
            AsCEventFx(self)->spray = 0;
        }
        v = AsCEventFx(self)->sequence;
        if (v) {
            CFx_RemoveSequence(g_fx, v, 0);
            AsCEventFx(self)->sequence = 0;
        }
        if (self) {
            unsigned int w = *(unsigned int*)self;
            if (w >> 31) {
                appFreeUni__FPv(AsCEventFx(self)->nameData);
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
struct EventEffectTreeStorage { unsigned int count; void* root; void* first; void* vtable; };
// Tree header: count +0, root +4, begin/first +8, object vtable +C.
// The empty begin pointer equals &root; null root denotes an empty owned node tree.
// Keep the disabled optimization pragmas and expanded cleanup shape for matching.
// Initialize the effect tree and its empty sentinel; the expanded cleanup path is unreachable.
void* CEventEffect_Ctor(void* self) {
    // Phase 1: establish the empty header before the expanded clear operation.
    void* effectTree;
    void* emptySentinel;
    void* nullRoot;
    void* effectVtable = 0;
    nullRoot = 0;
    effectVtable = (void*)__vt__12CEventEffect;
    emptySentinel = (char*)self + 4;
    effectTree = self;
    ((EventEffectTreeStorage*)effectTree)->vtable = effectVtable;
    ((EventEffectTreeStorage*)effectTree)->count = (unsigned int)nullRoot;
    ((EventEffectTreeStorage*)effectTree)->root = nullRoot;
    ((EventEffectTreeStorage*)effectTree)->first = emptySentinel;
    // Phase 2: expanded subtree clear. root was just stored as zero, so none of
    // these frees run during construction; there is only one tree header here.
    {
    void* rootNode = *(void**)((char*)effectTree + 4);
    if (rootNode) {
        void* leftSubtree = *(void**)rootNode;
        if (leftSubtree) {
            void* leftBranch = *(void**)leftSubtree;
            if (leftBranch) {
                void* leftSubbranch = *(void**)leftBranch;
                if (leftSubbranch) {
                    void* leftRemainder = *(void**)leftSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(effectTree, leftRemainder);
                    void* rightRemainder = *(void**)((char*)leftSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(effectTree, rightRemainder);
                    appFreeUni__FPv(leftSubbranch);
                }
                void* rightSubbranch = *(void**)((char*)leftBranch + 4);
                if (rightSubbranch) {
                    void* leftRemainder = *(void**)rightSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(effectTree, leftRemainder);
                    void* rightRemainder = *(void**)((char*)rightSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(effectTree, rightRemainder);
                    appFreeUni__FPv(rightSubbranch);
                }
                appFreeUni__FPv(leftBranch);
            }
            void* rightBranch = *(void**)((char*)leftSubtree + 4);
            if (rightBranch) {
                void* leftSubbranch = *(void**)rightBranch;
                if (leftSubbranch) {
                    void* leftRemainder = *(void**)leftSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(effectTree, leftRemainder);
                    void* rightRemainder = *(void**)((char*)leftSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(effectTree, rightRemainder);
                    appFreeUni__FPv(leftSubbranch);
                }
                void* rightSubbranch = *(void**)((char*)rightBranch + 4);
                if (rightSubbranch) {
                    void* leftRemainder = *(void**)rightSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(effectTree, leftRemainder);
                    void* rightRemainder = *(void**)((char*)rightSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(effectTree, rightRemainder);
                    appFreeUni__FPv(rightSubbranch);
                }
                appFreeUni__FPv(rightBranch);
            }
            appFreeUni__FPv(leftSubtree);
        }
        void* rightSubtree = *(void**)((char*)rootNode + 4);
        if (rightSubtree) {
            void* leftBranch = *(void**)rightSubtree;
            if (leftBranch) {
                void* leftSubbranch = *(void**)leftBranch;
                if (leftSubbranch) {
                    void* leftRemainder = *(void**)leftSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(effectTree, leftRemainder);
                    void* rightRemainder = *(void**)((char*)leftSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(effectTree, rightRemainder);
                    appFreeUni__FPv(leftSubbranch);
                }
                void* rightSubbranch = *(void**)((char*)leftBranch + 4);
                if (rightSubbranch) {
                    void* leftRemainder = *(void**)rightSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(effectTree, leftRemainder);
                    void* rightRemainder = *(void**)((char*)rightSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(effectTree, rightRemainder);
                    appFreeUni__FPv(rightSubbranch);
                }
                appFreeUni__FPv(leftBranch);
            }
            void* rightBranch = *(void**)((char*)rightSubtree + 4);
            if (rightBranch) {
                void* leftSubbranch = *(void**)rightBranch;
                if (leftSubbranch) {
                    void* leftRemainder = *(void**)leftSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(effectTree, leftRemainder);
                    void* rightRemainder = *(void**)((char*)leftSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(effectTree, rightRemainder);
                    appFreeUni__FPv(leftSubbranch);
                }
                void* rightSubbranch = *(void**)((char*)rightBranch + 4);
                if (rightSubbranch) {
                    void* leftRemainder = *(void**)rightSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(effectTree, leftRemainder);
                    void* rightRemainder = *(void**)((char*)rightSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(effectTree, rightRemainder);
                    appFreeUni__FPv(rightSubbranch);
                }
                appFreeUni__FPv(rightBranch);
            }
            appFreeUni__FPv(rightSubtree);
        }
        appFreeUni__FPv(rootNode);
    }
    }
    /* no separate appFreeUni(0) here: the original's only free of the root is the one above (li r3,0 at
       0x80221688 + bl 0x8022168C = free(rootNode), rootNode known null); the whole clear block 0x80221474-0x802216A0 is skipped by the b at
       0x80221470 (to 0x802216A4), so the original never calls appFreeUni on the live path; ours called appFreeUni(0) */
    {
        // Phase 3: leave count/root zero and begin pointing to the root sentinel.
        void* zero = 0;
        void* sentinel = (char*)effectTree + 4;
        *(void**)effectTree = zero;
        *(void**)((char*)effectTree + 4) = zero;
        *(void**)((char*)effectTree + 8) = sentinel;
    }
    {
        void* result = effectTree;
        return result;
    }
}
#pragma pop

} }
struct CEventEffectSC_EFF_FADE : CEventConte {
    char data[8];
    CEventEffectSC_EFF_FADE(void* arg);
};
// Select the fade color/direction and scale its requested duration.
CEventEffectSC_EFF_FADE::CEventEffectSC_EFF_FADE(void* arg) {
    using namespace ns1;
    void* self = (void*)this; volatile void* g = Sing_805DC660();
    *(void**)self = (void*)__vt__23CEventEffectSC_EFF_FADE;
    int arr[2];
    arr[0] = s_eventFadeColor0Rgb;
    arr[1] = s_eventFadeColor1Rgb;
    if (g == 0) {
        if (s_effectFadeInitialized == 0) {
            CEffectFade_Ctor(s_effectFade);
            ns1::__register_global_object(s_effectFade, (void*)CEffectFade_dtor, s_effectFadeDestructorRegistration);
            s_effectFadeInitialized = 1;
        }
        Sing_805DC660() = (void*)s_effectFade;
    }
    unsigned char idx = *((unsigned char*)arg + 2);
    FadeQuad_SetRgb(Sing_805DC660(), arr[idx]);
    AsEventFade(self)->direction = *((unsigned char*)arg + 3);
    unsigned int v = *((unsigned int*)arg + 1);
    if (EventTiming_IsDurationScalingEnabled() != 0) {
        if (v >= 2) {
            union { PpcDoubleWords w; double d; } conv;
            conv.w.hi = 0x43300000;
            conv.w.lo = v;
            float f1 = kEventFadeDurationScaleNumerator * (float)(conv.d - kUnsignedToDoubleBias_EventFade);
            f1 = f1 / kEventFadeDurationScaleDenominator;
            v = (unsigned int)Runtime_ConvertDoubleToUnsigned(f1);
        }
    }
    AsEventFade(self)->count = v;
}
namespace ns1 { extern "C" {

// Convert elapsed frames to fade alpha, reversing the ramp for direction zero.
int CEventEffectSC_EFF_FADE_UpdateFadeAlpha(void* self, unsigned int elapsedFrames) {
    unsigned int durationFrames = AsEventFade(self)->count;
    float progress;
    if (durationFrames != 0) {
        progress = (float)elapsedFrames / (float)durationFrames;
    } else {
        progress = kEventFadeImmediateProgress;
    }
    unsigned char direction = AsCEventEffectSC(self)->fadeDirection;
    int alpha = (int)(kEventFadeAlphaScale * progress);
    if (direction == 0) {
        alpha = (255 - alpha) & 0xFF;
    }
    // global init
    if (Sing_805DC660() == 0) {
        if (s_effectFadeInitialized == 0) {
            CEffectFade_Ctor(s_effectFade);
            __register_global_object(s_effectFade, (void*)CEffectFade_dtor, s_effectFadeDestructorRegistration);
            s_effectFadeInitialized = 1;
        }
        Sing_805DC660() = (void*)s_effectFade;
    }
    CEffectFade_SetAlpha(Sing_805DC660(), (unsigned int)alpha & 0xFF);
    if (elapsedFrames < AsEventFade(self)->count) {
        return 0;
    }
    return 1;
}

// Interpolate all three Euler angles, wrap them to one turn and invalidate model rotation.
int CEventModelSC_MDL_ROT_UpdateRotation(void* self, unsigned int elapsedFrames) {
    unsigned int durationFrames = AsModelRotate(self)->count;
    float fullTurn;
    float xDegrees;
    float deltaX;
    float progress;
    if (durationFrames != 0) {
        progress = (float)elapsedFrames / (float)durationFrames;
    } else {
        progress = kFloat_1_ValueAnimation;
    }
    deltaX = AsModelRotate(self)->deltaX; // 0x18
    xDegrees = AsModelRotate(self)->startX; // 0x0C
    xDegrees = xDegrees + deltaX * progress;
    if (xDegrees < kFloat_0_EventMotion) xDegrees += kFloat_360_eventModel;
    fullTurn = kFloat_360_eventModel;
    if (fullTurn <= xDegrees) xDegrees -= fullTurn;
    AsTransformModel(AsModelRotate(self)->object)->eulerDegrees.x = xDegrees;
    float y;
    float dy;
    dy = AsModelRotate(self)->deltaY;
    y = AsModelRotate(self)->startY;
    y = y + dy * progress;
    if (y < kFloat_0_EventMotion) y += kFloat_360_eventModel;
    fullTurn = kFloat_360_eventModel;
    if (fullTurn <= y) y -= fullTurn;
    AsTransformModel(AsModelRotate(self)->object)->eulerDegrees.y = y;
    float z;
    float dz;
    dz = AsModelRotate(self)->deltaZ;
    z = AsModelRotate(self)->startZ;
    z = z + dz * progress;
    if (z < kFloat_0_EventMotion) z += kFloat_360_eventModel;
    fullTurn = kFloat_360_eventModel;
    if (fullTurn <= z) z -= fullTurn;
    AsTransformModel(AsModelRotate(self)->object)->eulerDegrees.z = z; // 0x30
    AsTransformModel(AsModelRotate(self)->object)->rotationDirty = 1; // 0x14
    if (elapsedFrames < AsModelRotate(self)->count) {
        return 0;
    }
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Release effect-tree node allocations before optionally deleting the wrapper.
void* ScriptByteMap_dtor(void* ptr, int flag) {
    // Phase 1: walk the first three link levels explicitly, delegating any deeper
    // subtree to ScriptByteMap_FreeSubtree; free parents after both children.
    // Node links are +0 left / +4 right. Payload destruction is not shown here.
    char* self = (char*)ptr;
    if (self) { if (self) if (self) if (self) if (self) if (self) {
    void* rootNode = *(void**)(self + 4);
    if (rootNode) {
        void* leftSubtree = *(void**)rootNode;
        if (leftSubtree) {
            void* leftBranch = *(void**)leftSubtree;
            if (leftBranch) {
                void* leftSubbranch = *(void**)leftBranch;
                if (leftSubbranch) {
                    void* leftRemainder = *(void**)leftSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(self, leftRemainder);
                    void* rightRemainder = *(void**)((char*)leftSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(self, rightRemainder);
                    appFreeUni__FPv(leftSubbranch);
                }
                void* rightSubbranch = *(void**)((char*)leftBranch + 4);
                if (rightSubbranch) {
                    void* leftRemainder = *(void**)rightSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(self, leftRemainder);
                    void* rightRemainder = *(void**)((char*)rightSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(self, rightRemainder);
                    appFreeUni__FPv(rightSubbranch);
                }
                appFreeUni__FPv(leftBranch);
            }
            void* rightBranch = *(void**)((char*)leftSubtree + 4);
            if (rightBranch) {
                void* leftSubbranch = *(void**)rightBranch;
                if (leftSubbranch) {
                    void* leftRemainder = *(void**)leftSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(self, leftRemainder);
                    void* rightRemainder = *(void**)((char*)leftSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(self, rightRemainder);
                    appFreeUni__FPv(leftSubbranch);
                }
                void* rightSubbranch = *(void**)((char*)rightBranch + 4);
                if (rightSubbranch) {
                    void* leftRemainder = *(void**)rightSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(self, leftRemainder);
                    void* rightRemainder = *(void**)((char*)rightSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(self, rightRemainder);
                    appFreeUni__FPv(rightSubbranch);
                }
                appFreeUni__FPv(rightBranch);
            }
            appFreeUni__FPv(leftSubtree);
        }
        void* rightSubtree = *(void**)((char*)rootNode + 4);
        if (rightSubtree) {
            void* leftBranch = *(void**)rightSubtree;
            if (leftBranch) {
                void* leftSubbranch = *(void**)leftBranch;
                if (leftSubbranch) {
                    void* leftRemainder = *(void**)leftSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(self, leftRemainder);
                    void* rightRemainder = *(void**)((char*)leftSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(self, rightRemainder);
                    appFreeUni__FPv(leftSubbranch);
                }
                void* rightSubbranch = *(void**)((char*)leftBranch + 4);
                if (rightSubbranch) {
                    void* leftRemainder = *(void**)rightSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(self, leftRemainder);
                    void* rightRemainder = *(void**)((char*)rightSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(self, rightRemainder);
                    appFreeUni__FPv(rightSubbranch);
                }
                appFreeUni__FPv(leftBranch);
            }
            void* rightBranch = *(void**)((char*)rightSubtree + 4);
            if (rightBranch) {
                void* leftSubbranch = *(void**)rightBranch;
                if (leftSubbranch) {
                    void* leftRemainder = *(void**)leftSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(self, leftRemainder);
                    void* rightRemainder = *(void**)((char*)leftSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(self, rightRemainder);
                    appFreeUni__FPv(leftSubbranch);
                }
                void* rightSubbranch = *(void**)((char*)rightBranch + 4);
                if (rightSubbranch) {
                    void* leftRemainder = *(void**)rightSubbranch;
                    if (leftRemainder) ScriptByteMap_FreeSubtree(self, leftRemainder);
                    void* rightRemainder = *(void**)((char*)rightSubbranch + 4);
                    if (rightRemainder) ScriptByteMap_FreeSubtree(self, rightRemainder);
                    appFreeUni__FPv(rightSubbranch);
                }
                appFreeUni__FPv(rightBranch);
            }
            appFreeUni__FPv(rightSubtree);
        }
        appFreeUni__FPv(rootNode);
    }
    }
    // Phase 2: only a positive deleting-destructor flag frees the wrapper itself.
    if (flag > 0) __dl__FPv(ptr);
    }
    return ptr;
}
#pragma pop

} // extern "C"

} // namespace ns1

extern "C" { void EventEffects_InitAngleConstant(); }
/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462218", _ctors_u8021F0F4_0, EventEffects_InitAngleConstant);
#pragma pop
