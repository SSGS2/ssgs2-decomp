// The stroke-point selector and the voice callout queue.
//
// The selector lets the player drag a strike point away from the ball preview and
// draws the marker, direction and special-shot buttons, the power gauge and the
// help message. The same file holds the part of CVoiceManager that queues voice
// names and formats them.
#include <decomp/static_init.h>
#include "MSL/error_message.h"
#include "classes/CStrokePoint2d.h"
#include "classes/CVoiceManager.h"
#include "classes/PendingVoiceEntry.h"
#include "sdk_protos.h"

enum {
    kVoiceNameFormat = 0x4B,     // "%s%d"
    kVoiceManagerTypeName = 0xE0 // "CVoiceManager"
};

enum StrokeDirection {
    kStrokeDirectionLeft = 4,
    kStrokeDirectionRight = 8
};

enum StrokeElement {
    kStrokeMarker = 0x4C,
    kStrokeMessage = 0x4A,
    kStrokeMessageBackdrop = 0x4B
};

enum {
    kBaseVoiceNumberCount = 13,
    kVoiceNameBufferBytes = 64,
    kStrokePlayerFlagSelector = 5
};

// Layout element IDs of the action buttons and the power gauge.
enum StrokeActionElement {
    kTomahawkButton = 0x2D,
    kTomahawkCompanion = 0x2E,
    kSpikeButton = 0x2F,
    kSpikeCompanion = 0x30,
    kCobraButton = 0x31,
    kCobraCompanion = 0x32,
    kPowerSegmentTwo = 0x3A,
    kPowerSegmentThree = 0x3B,
    kPowerRemainder = 0x3C,
    kSpinLeftButton = 0x3D,
    kSpinLeftCompanion = 0x3E,
    kSpinBottomButton = 0x3F,
    kSpinBottomCompanion = 0x40,
    kSpinRightButton = 0x41,
    kSpinRightCompanion = 0x42,
    kSpinTopButton = 0x43,
    kSpinTopCompanion = 0x44,
    kConfirmButton = 0x49
};

void* operator new(size_t);
// Construct in caller-provided storage without allocating.
inline void* operator new(size_t, void* p) {
    return p;
}

extern "C" {

extern char s_voiceManagerTypeInfo;
extern const char strokePointSpecialShotStrings[];
extern const float kFloat_6_2831855;
extern float s_voiceManagerRuntimeTwoPi; // writable copy of two pi, set during registration
extern char s_actorTypeInfo;
extern char voiceManagerStrings[];
extern char s_voiceManagerActorFactoryKey[];
extern char s_voiceManagerTaskFactoryDescriptor[];
extern char __vt__14CStrokePoint2d[];
extern const float kFloat_1_805E0B38;
extern const float kFloat_0_StrokePointControls;
extern const float kFloat_0_805E0A78;
extern const float kFloat_33_StrokePointGauge;
extern void* g_soundManager;
extern const int voiceBaseCharacterNameFormats[];
extern const int voiceAlternateCharacterNameFormats[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std11logic_error[];
extern char __vt__Q23std12length_error[];

void Layout_Init(void*, int);
void __dt__18NativeBase80201D18Fv(void*, int);
void Layout_LoadExtended(void*, int, const char*);
void gtl_Vec2_Copy(void*, void*);
void* IntPair_Set(void*, void*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void CVoiceManager_Create();
void Layout_DrawWithOpacity(void*, float);
void ScreenBallPreview_DrawStrokePoint(void*);
void Layout_DrawElementWithOpacity(void*, int, float);
void MenuMessage_DrawText(void*);
void MenuMessage_Dtor(void*, int);
void Layout_Destroy(void*, int);
int Layout_Update(void*);
void SoundManager_PlayVoiceByName(void*, void*);
void String_ReplaceRange(void*, int, void*, int, void*, void*, void*, void*);
void Layout_SetElementEnabled(void*, int, int);
void Layout_GetElementPosition(void*, int, void*);
void Layout_ReadElementRect(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementPosition(void*, int, void*);
void Layout_GetElementScale(void*, int, void*);
void MenuMessage_Ctor(void*);
void* __nw__FUl(unsigned int);
void MenuMessage_SetPosition(void*, void*);
void String_ReserveCapacity(void*, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* __nwa__FUl(unsigned int);
void __dla__FPv(void*);
void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q23std12length_errorFv(void*, int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);

} // extern "C"

// Float constants linked from the original data.
#define kZero kFloat_0_StrokePointControls
#define kFullOpacity kFloat_1_805E0B38
#define kGaugeSegmentStep kFloat_33_StrokePointGauge
#define kZeroDelay kFloat_0_805E0A78
#define kTwoPi kFloat_6_2831855

// A pending-voice range: the storage, the live count from the first index, the
// capacity and the allocator word.
struct VoiceRangeStorage {
    char* data;
    int count;
    int capacity;
    void* allocator;
    int firstIndex;
};

// Shared error text for the voice-name vector's length check.
typedef ::std::detail::__message_control SharedErrorRef;

// The allocator tag the string and vector calls pass around; it holds no state.
struct VoiceNameAllocator {
    char byte;
};

typedef void RethrowFn(const char*, void*, void*) __attribute__((noreturn));

// Eight voice-name format indices, one per character.
struct VoiceFormatTable {
    int v[8];
};

// A 12-byte small string: the high flag bit selects heap ownership (+4 length,
// +8 pointer), otherwise the bytes start at +1. Copies duplicate heap text and
// destruction frees only heap text.
struct VoiceNameString {
    unsigned int flag;
    unsigned int len;
    char* ptr;

    // Copy formatted text into an owning small string.
    VoiceNameString(const char* str, const VoiceNameAllocator& alloc) {
        unsigned int zero = 0;
        flag = zero;
        len = zero;
        ptr = (char*)zero;
        unsigned int n = strlen(str);
        String_ReserveCapacity(this, n);
        VoiceNameAllocator copy = alloc;
        ((void (*)(void*, int, int, void*, void*, void*))String_ReplaceRange)(this, 0, 0, (void*)str, (void*)(str + n),
                                                                              &copy);
    }

    // Copy inline bytes or deep-copy heap-backed text.
    VoiceNameString(const VoiceNameString& other) {
        try {
            unsigned int f = other.flag;
            if (!(f >> 31)) {
                unsigned int n = other.len;
                flag = f;
                len = n;
                ptr = other.ptr;
            } else {
                flag = 0;
                len = 0;
                ptr = 0;
                String_ReserveCapacity(this, other.len);
                VoiceNameAllocator alloc;
                VoiceNameAllocator copy = alloc;
                ((void (*)(void*, int, int, void*, void*, void*))String_ReplaceRange)(this, 0, 0, other.ptr,
                                                                                      other.ptr + other.len, &copy);
            }
        } catch (...) {
            ((RethrowFn*)__throw)(0, 0, 0);
        }
    }

    // Free text only when the string owns a heap allocation.
    ~VoiceNameString() {
        if (flag >> 31)
            appFreeUni__FPv(ptr);
    }
};

struct VoiceNameVector {
    VoiceNameString* data;
    unsigned int size, capacity;
    void push(const VoiceNameString&, const VoiceNameAllocator&);
    // Append one copied name with the default allocator tag.
    void push(const VoiceNameString& local) {
        push(local, VoiceNameAllocator());
    }
    unsigned int nextCapacity(const unsigned int&) const;
    // Return the 32-bit allocation limit for 12-byte string elements.
    unsigned int maxSize() const {
        VoiceNameAllocator alloc = VoiceNameAllocator();
        return 0x15555555;
    }
};

// Growth guard owns a new allocation and the constructed range [offset, offset+size).
// append constructs the new suffix first; prepend transfers old values and clears
// their sources; install swaps the buffers. The guard then frees the old storage
// without freeing the transferred strings, and unwinding destroys its live range.
struct VoiceNameStorage {
    VoiceNameString* data;
    unsigned int size, capacity;
    unsigned int* allocator;
    unsigned int offset;
    char reservedTail[4];

    // Initialize an empty growth guard linked to the vector's capacity word.
    VoiceNameStorage(unsigned int* a) {
        data = 0;
        size = 0;
        capacity = 0;
        allocator = a;
        offset = 0;
    }

    // Construct the new suffix and track its live range for unwind cleanup.
    void append(unsigned int pos, int count, const VoiceNameString& local) {
        offset = pos;
        VoiceNameString* item = size + (data + pos);
        VoiceNameAllocator alloc = VoiceNameAllocator();
        while (count != 0) {
            new (item) VoiceNameString(local);
            --count;
            ++item;
            ++size;
        }
    }

    // Transfer old string values before the suffix and clear their previous owners.
    int prepend(VoiceNameString* last, VoiceNameString* first) {
        int n = ((char*)last - (char*)first) / 12;
        VoiceNameAllocator alloc = VoiceNameAllocator();
        offset -= n;
        memcpy(data + offset, first, n * 12);
        memset(first, 0, n * 12);
        return n;
    }

    // Publish replacement storage and retain the old buffer for guard cleanup.
    void install(VoiceNameVector* vec, int n) {
        unsigned int oldsize;
        VoiceNameString* olddata;
        unsigned int oldcap;
        unsigned int newsize;
        unsigned int newcap;
        VoiceNameString* newdata;
        newsize = size + n;
        oldsize = vec->size;
        size = newsize;
        olddata = vec->data;
        newdata = data;
        oldcap = vec->capacity;
        newcap = capacity;
        VoiceNameString* start = olddata + offset;
        VoiceNameString* end = start + oldsize;
        data = olddata;
        VoiceNameAllocator alloc = VoiceNameAllocator();
        vec->capacity = newcap;
        capacity = oldcap;
        vec->data = newdata;
        vec->size = newsize;
        size = oldsize;
        while (end > start) {
            --end;
            if (end != 0 && (end->flag >> 31))
                appFreeUni__FPv(end->ptr);
        }
        size = 0;
    }

    // Destroy live heap strings in reverse order while retaining the allocation.
    void clear() {
        VoiceNameString* p;
        unsigned int n = size;
        p = data + n;
        VoiceNameAllocator alloc = VoiceNameAllocator();
        size = size - n;
        while (n != 0) {
            --p;
            if (p != 0 && (p->flag >> 31))
                appFreeUni__FPv(p->ptr);
            --n;
        }
    }

    // Destroy any live strings and free the guarded allocation.
    ~VoiceNameStorage() {
        if (data != 0) {
            clear();
            appFreeUni__FPv(data);
        }
    }
};

struct ErrorTextGuard {
    char* text;
    VoiceNameAllocator* allocator;

    // Free the text unless ownership has transferred to its reference block.
    ~ErrorTextGuard() {
        if (text)
            __dla__FPv(text);
    }
};

struct SharedErrorText {
    char* text;
    SharedErrorRef* ref;

    // Allocate and share a NUL-terminated copy of the diagnostic text.
    SharedErrorText(const char* message) {
        VoiceNameAllocator alloc = VoiceNameAllocator();
        unsigned int length = strlen(message) + 1;
        text = (char*)__nwa__FUl(length * 2 - length);
        {
            ErrorTextGuard guard;
            guard.text = text;
            guard.allocator = &alloc;
            char* p = text;
            ref = new SharedErrorRef(p);
            guard.text = 0;
        }
        strcpy(text, message);
    }
};

namespace std {
struct exception {
    void* vtable;
    // Initialize the standard logic-error base identity.
    exception() : vtable(__vt__Q23std11logic_error) {}
    ~exception();
};
struct logic_error : exception {
    SharedErrorText text;
    // Construct copied text for the standard logic-error object.
    logic_error(const char*& message) : text(message) {}

    ~logic_error();
};
struct length_error : logic_error {
    // Set the length-error identity on the constructed logic-error base.
    length_error(const char* message) : logic_error(message) {
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
};
} // namespace std

extern "C" {

// Throw the voice-name vector length error.
inline void ThrowVoiceNameLengthError() {
    throw std::length_error(voiceManagerStrings);
}

// Free the heap text of one voice-name string, then the string itself. This is
// the deleting destructor of the 12-byte string the pending queue stores.
void* dtor_802E6030(VoiceNameString* self, int flags) {
    if (self != 0) {
        if (self != 0 && (self->flag >> 31) != 0)
            appFreeUni__FPv(self->ptr);
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

} // extern "C"

// Count down each queued voice delay and play the names that expire.
//
// A queue entry is a 12-byte string followed by its delay; a negative updated
// delay expires, while zero waits for a later frame. Playback borrows the name
// for the call only. Removing an entry shifts the following strings left and
// destroys the last one, then retries the same index.
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
void CVoiceManager::OnProcess(float elapsedSeconds) {
    char* pendingEntry;
    CVoiceManager* self;
    pendingEntry = (*(char**)&pendingVoices);
    float zero = kZeroDelay;
    self = this;
    while (pendingEntry != (char*)self->pendingVoices + self->pendingVoiceCount * 0x10) {
        float remainingDelaySeconds = AsPendingVoice802E6030(pendingEntry)->delaySeconds - elapsedSeconds;
        AsPendingVoice802E6030(pendingEntry)->delaySeconds = remainingDelaySeconds;
        if (remainingDelaySeconds < zero) {
            unsigned int flag = *(unsigned int*)pendingEntry >> 31;
            void* voiceName;
            if (flag == 0) {
                voiceName = pendingEntry + 1;
            } else {
                voiceName = *(void**)(pendingEntry + 8);
            }
            SoundManager_PlayVoiceByName(g_soundManager, voiceName);
            char* end = (char*)self->pendingVoices + self->pendingVoiceCount * 0x10;
            // Shift every following entry left; the queue keeps its capacity and
            // the strings keep their owners.
            if (((int)(end - pendingEntry) / 16) - 1 != 0) {
                char* sourceEntry;
                char* destinationEntry;
                destinationEntry = pendingEntry;
                sourceEntry = pendingEntry + 0x10;
                while (sourceEntry < end) {
                    unsigned int sourceStringFlags;
                    unsigned int destinationIsHeap;
                    destinationIsHeap = *(unsigned int*)destinationEntry >> 31;
                    if (!destinationIsHeap && !((sourceStringFlags = *(unsigned int*)sourceEntry) >> 31)) {
                        unsigned int second = *(unsigned int*)(sourceEntry + 4);
                        *(unsigned int*)destinationEntry = sourceStringFlags;
                        *(unsigned int*)(destinationEntry + 4) = second;
                        *(unsigned int*)(destinationEntry + 8) = *(unsigned int*)(sourceEntry + 8);
                    } else {
                        unsigned int destinationLengthAbiWord =
                            destinationIsHeap ? *(unsigned int*)(destinationEntry + 4)
                                              : (unsigned int)(*(unsigned char*)destinationEntry & 0x7f);
                        char* sourceText;
                        int sourceLength;
                        if (!(*(unsigned int*)sourceEntry >> 31)) {
                            sourceText = sourceEntry + 1;
                            sourceLength = *(unsigned char*)sourceEntry & 0x7f;
                        } else {
                            sourceText = *(char**)(sourceEntry + 8);
                            sourceLength = *(int*)(sourceEntry + 4);
                        }
                        struct Alloc {
                            char c;
                        } alloc;
                        Alloc copy = alloc;
                        ((void (*)(void*, int, void*, void*, void*, void*))String_ReplaceRange)(
                            destinationEntry, 0, (void*)destinationLengthAbiWord, sourceText, sourceText + sourceLength,
                            &copy);
                    }
                    AsPendingVoice802E6030(destinationEntry)->delaySeconds =
                        AsPendingVoice802E6030(sourceEntry)->delaySeconds;
                    destinationEntry += 0x10;
                    sourceEntry += 0x10;
                }
            }
            // Destroy the duplicate last name after reducing the logical count.
            int count = self->pendingVoiceCount - 1;
            char* base = (char*)self->pendingVoices;
            self->pendingVoiceCount = count;
            char* last = base + count * 0x10;
            if (last != 0 && last != 0 && (*(unsigned int*)last >> 31))
                appFreeUni__FPv(*(void**)(last + 8));
        } else
            pendingEntry += 0x10;
    }
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset

void* CVoiceManager::GetRTTI() {
    return &s_voiceManagerTypeInfo;
}

// Free the queued voice names and their storage, then destroy the actor base.
// The interface vtable calls this with a nonzero flags word to free the object
// too, so the delete is written out instead of being left to a destructor.
void* CVoiceManager::Destroy(int flags) {
    if (this != 0) {
        void** items = (*(void***)&pendingVoices);
        if (items != 0) {
            char* p = 0;
            int count = pendingVoiceCount;
            int zero = count;
            zero = zero - count;
            pendingVoiceCount = zero;
            p = (char*)items + count * 16;
            while (count != 0) {
                p = p - 0x10;
                if (p != 0 && p != 0) {
                    if ((*(unsigned int*)p >> 31) != 0)
                        appFreeUni__FPv(*(void**)(p + 8));
                }
                count = count - 1;
            }
            appFreeUni__FPv(pendingVoices);
        }
        __dt__18NativeBase80201D18Fv(this, 0);
        if (flags > 0)
            __dl__FPv(this);
    }
    return this;
}

// Destroy a pending-voice range and release its vector storage. The repeated
// null tests are what the original's element loop compiles from.
extern "C" void* VoiceRangeStorage_dtor(VoiceRangeStorage* self, int flags) {
    if (self != 0) {
        char* base = self->data;
        int c2 = self->firstIndex;
        int c1 = self->count;
        char* end;
        char* start = base + c2 * 0x10;
        end = start + c1 * 0x10;
        while (end > start) {
            end -= 0x10;
            if (end != 0 && end != 0) {
                if ((*(unsigned int*)end >> 31) != 0)
                    appFreeUni__FPv(*(void**)(end + 8));
            }
        }
        self->count = 0;
        if (self != 0) {
            int count = self->count;
            char* base2 = self->data;
            if (base2 != 0) {
                self->count = count;
                while (count != 0) {
                    base2 -= 0x10;
                    if (base2 != 0 && base2 != 0) {
                        if ((*(unsigned int*)base2 >> 31) != 0)
                            appFreeUni__FPv(*(void**)(base2 + 8));
                    }
                    count = count - 1;
                }
                appFreeUni__FPv(self->data);
            }
        }
        if (flags > 0)
            __dl__FPv(self);
    }
    return self;
}

// Register the voice-manager type information, actor factory and task factory.
void CVoiceManager::RegisterFactory() {
    float f = kTwoPi;
    s_voiceManagerRuntimeTwoPi = f;
    char* p = voiceManagerStrings;
    IntPair_Set(&s_voiceManagerTypeInfo, p + kVoiceManagerTypeName, &s_actorTypeInfo);
    StringMap_SetByName(ActorFactoryRegistry_GetInstance(), CVoiceManager_Create, s_voiceManagerActorFactoryKey);
    TaskRegistry_AppendFactory(TaskRegistry_GetInstance(), s_voiceManagerTaskFactoryDescriptor);
}

// A 12-byte message object, constructed in place when the layout is initialised.
struct StrokePointMessage {
    char data[0x0C];
    StrokePointMessage() {
        MenuMessage_Ctor(this);
    }
};

// The selector's interface vtable is at 0xE0. A derived class's vptr follows its
// base, so the 0xE0-byte base places these hooks over that table; the unused
// slots keep the offsets of the four hooks this file calls.
struct StrokePointVirtualBase {
    char pad[224];
};
class StrokePointUpdateHooks : public StrokePointVirtualBase {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual void v2C();
    virtual void v30();
    virtual void v34();
    virtual void v38();
    virtual void v3C();
    virtual void v40();
    virtual void v44();
    virtual void v48();
    virtual void v4C();
    virtual void v50();
    virtual void v54();
    virtual void v58();
    virtual void v5C();
    virtual void v60();
    virtual void v64();
    virtual void v68();
    virtual void v6C();
    virtual void v70();
    virtual int AdvanceDialogAnimation();
    virtual int ConsumeActivatedControls();
    virtual int RefreshDirtyControls();
    virtual void v80();
    virtual void v84();
    virtual void UpdateHoveredHelpText();
};

// Set the selector to its empty state: no strike point, no selection, no message.
CStrokePoint2d* CStrokePoint2d::Init() {
    Layout_Init(this, 5);
    float zero = kZero;
    int izero = 0;
    markerOriginX = zero;
    interfaceVtable = __vt__14CStrokePoint2d;
    markerOriginY = zero;
    strikeX = zero;
    strikeY = zero;
    savedStrikeX = zero;
    savedStrikeY = zero;
    horizontalThreshold = zero;
    verticalThreshold = zero;
    inputLock = izero;
    powerModeEnabled = izero;
    directionButtonsEnabled = izero;
    extendedSpecialShotsEnabled = izero;
    powerGauge = izero;
    directionSelection = izero;
    savedDirectionSelection = izero;
    confirmRequested = izero;
    powerMode = izero;
    savedPowerMode = izero;
    specialShotSelection = izero;
    savedSpecialShotSelection = izero;
    animationFramesRemaining = izero;
    dialogState = izero;
    opacity = zero;
    visible = izero;
    resourcesReady = izero;
    controlsDirty = izero;
    specialShotOpacity = izero;
    powerClickFramesRemaining = izero;
    powerClickFeedback = izero;
    gaugeOriginX = zero;
    gaugeSegmentWidth = zero;
    message = 0;
    selectedIndex = -1;
    return this;
}

// Release the help message and layout before optional deletion.
CStrokePoint2d::~CStrokePoint2d() {
    interfaceVtable = __vt__14CStrokePoint2d;
    if (message != 0) {
        MenuMessage_Dtor(message, 1);
        message = 0;
    }
    message = 0;
    Layout_Destroy(this, 0);
}

int CStrokePoint2d::LoadSpecialShotLayout(int mode) {
    Layout_LoadExtended(this, mode, strokePointSpecialShotStrings);
    return 1;
}

int CStrokePoint2d::ClearShotSelections() {
    directionSelection = 0;
    confirmRequested = 0;
    powerMode = 0;
    specialShotSelection = 0;
    return 1;
}

// Initialize the selector rectangles, marker position and the lazily created
// help message.
//
// The action buttons and their companion layers are given the same rectangle.
// The gauge segments two and three sit 33 units right of the previous one; the
// remainder element supplies the gauge origin and the segment width.
int CStrokePoint2d::InitLayout() {
    void* object = 0;
    // Hide transient elements and capture the marker origin.
    Layout_SetElementEnabled(this, kStrokeMarker, 0);
    Layout_SetElementEnabled(this, kStrokeMessage, 0);
    Layout_SetElementEnabled(this, kStrokeMessageBackdrop, 0);
    Layout_GetElementPosition(this, kStrokeMarker, &markerOriginX);
    gtl_Vec2_Copy(&strikeX, &markerOriginX);
    struct PairValue {
        float x, y, z, w;
    } value1; // the rectangle calls move four words
    // Align each companion with the rectangle of its action button.
    Layout_ReadElementRect(this, kSpinLeftButton, &value1);
    Layout_WriteElementRect(this, kSpinLeftCompanion, &value1);
    Layout_ReadElementRect(this, kSpinRightButton, &value1);
    Layout_WriteElementRect(this, kSpinRightCompanion, &value1);
    Layout_ReadElementRect(this, kSpinTopButton, &value1);
    Layout_WriteElementRect(this, kSpinTopCompanion, &value1);
    Layout_ReadElementRect(this, kSpinBottomButton, &value1);
    Layout_WriteElementRect(this, kSpinBottomCompanion, &value1);
    Layout_ReadElementRect(this, kTomahawkButton, &value1);
    Layout_WriteElementRect(this, kTomahawkCompanion, &value1);
    Layout_ReadElementRect(this, kSpikeButton, &value1);
    Layout_WriteElementRect(this, kSpikeCompanion, &value1);
    Layout_ReadElementRect(this, kCobraButton, &value1);
    Layout_WriteElementRect(this, kCobraCompanion, &value1);
    // Enable confirmation and position the gauge segments.
    Layout_SetElementPlayerFlag(this, kConfirmButton, 1, kStrokePlayerFlagSelector);
    float value2[2];
    Layout_GetElementPosition(this, kPowerSegmentTwo, value2);
    value2[0] += kGaugeSegmentStep;
    Layout_SetElementPosition(this, kPowerSegmentTwo, value2);
    value2[0] += kGaugeSegmentStep;
    Layout_SetElementPosition(this, kPowerSegmentThree, value2);
    Layout_GetElementPosition(this, kPowerRemainder, value2);
    gaugeOriginX = value2[0];
    Layout_GetElementScale(this, kPowerRemainder, value2);
    gaugeSegmentWidth = value2[0];
    // Allocate the owned help message only once.
    if (message == 0) {
        object = (void*)new StrokePointMessage();
        message = object;
        Layout_GetElementPosition(this, kStrokeMessage, value2);
        MenuMessage_SetPosition(message, value2);
    }
    resourcesReady = 1;
    controlsDirty = 1;
    return 1;
}

int CStrokePoint2d::SetPointerSelectionDisabled(int value) {
    inputLock = value;
    return 1;
}

int CStrokePoint2d::SetPowerModeEnabled(int value) {
    powerModeEnabled = value;
    return 1;
}

int CStrokePoint2d::SetDirectionButtonsEnabled(int value) {
    directionButtonsEnabled = value;
    return 1;
}

// Set the gauge fill (33 units per segment; 99 fills all three segments).
int CStrokePoint2d::SetPowerGaugeFill(int value) {
    powerGauge = value;
    return 1;
}

// Allow Cobra and Spike when a power mode is selected; Tomahawk needs only power.
int CStrokePoint2d::SetExtendedSpecialShotsAllowed(int value) {
    extendedSpecialShotsEnabled = value;
    return 1;
}

// Draw the selector, the fully visible ball preview, the marker and the message.
int CStrokePoint2d::Draw(void* ballPreview) {
    Layout_DrawWithOpacity(this, opacity);
    if (kFullOpacity == opacity)
        ScreenBallPreview_DrawStrokePoint(ballPreview);
    Layout_DrawElementWithOpacity(this, kStrokeMarker, opacity);
    if (message != 0)
        MenuMessage_DrawText(message);
    return 1;
}

// Run the dialog animation, consume the player's activations, update the help
// text and refresh the controls. The hooks' results are combined bitwise with
// the layout update's first bit; every hook still runs after a zero result.
int CStrokePoint2d::UpdateDialog() {
    int result = Layout_Update(this) & 1;
    result &= ((StrokePointUpdateHooks*)this)->AdvanceDialogAnimation();
    result &= ((StrokePointUpdateHooks*)this)->ConsumeActivatedControls();
    if (message != 0)
        ((StrokePointUpdateHooks*)this)->UpdateHoveredHelpText();
    result &= ((StrokePointUpdateHooks*)this)->RefreshDirtyControls();
    return result;
}

int CStrokePoint2d::IsVisible() {
    return visible;
}

// Set the direction thresholds and clear a left or right selection that no
// longer reaches its threshold.
int CStrokePoint2d::SetDirectionThresholds(float x, float y) {
    int state = directionSelection;
    horizontalThreshold = x;
    verticalThreshold = y;
    if (state == kStrokeDirectionLeft) {
        if (strikeX > -(float)(double)x)
            directionSelection = 0;
    } else if (state == kStrokeDirectionRight) {
        if (strikeX < (float)(double)x)
            directionSelection = 0;
    }
    return 1;
}

// Move the strike point, and request a refresh only when it actually moves.
int CStrokePoint2d::SetPointPosition(float x, float y) {
    if (strikeX == x && strikeY == y)
        return 1;
    strikeX = x;
    strikeY = y;
    controlsDirty = 1;
    return 1;
}

int CStrokePoint2d::IsConfirmRequested() {
    return confirmRequested;
}

int CStrokePoint2d::GetDirectionSelection() {
    return directionSelection;
}

int CStrokePoint2d::GetSpecialShotSelection() {
    return specialShotSelection;
}

int CStrokePoint2d::GetPowerMode() {
    return powerMode;
}

int CStrokePoint2d::ClearDirectionSelection() {
    directionSelection = 0;
    return 1;
}

int CStrokePoint2d::ClearSpecialShotSelection() {
    specialShotSelection = 0;
    return 1;
}

int CStrokePoint2d::ClearPowerMode() {
    powerMode = 0;
    return 1;
}

// Restore the saved strike point, direction, power mode and special shot, then
// request a refresh.
int CStrokePoint2d::RestoreSavedSelection() {
    gtl_Vec2_Copy(&strikeX, &savedStrikeX);
    directionSelection = savedDirectionSelection;
    specialShotSelection = savedSpecialShotSelection;
    powerMode = savedPowerMode;
    controlsDirty = 1;
    return 1;
}

#pragma push
#pragma always_inline on
#pragma inline_max_size(10000)
#pragma inline_max_total_size(100000)
// Recommend capacity for 12-byte names with overflow checks.
inline unsigned int VoiceNameVector::nextCapacity(const unsigned int& extra) const {
    unsigned int result = capacity;
    if (extra > 0x15555555U - result) {
        ThrowVoiceNameLengthError();
    }
    if (result < 0x071C71C7U) {
        unsigned int grow = (3 * (result + 1)) / 5;
        result += *(grow < extra ? &extra : &grow);
    } else if (result < 0x0E38E38EU) {
        unsigned int grow = (result + 1) / 2;
        result += *(grow < extra ? &extra : &grow);
    } else {
        result = 0x15555555;
    }
    return result;
}

// Copy a name into the vector, growing the storage with unwind cleanup when full.
inline void VoiceNameVector::push(const VoiceNameString& local, const VoiceNameAllocator&) {
    VoiceNameVector* vec = this;
    if (vec->size < vec->capacity) {
        VoiceNameString* item = vec->data + vec->size;
        new (item) VoiceNameString(local);
        vec->size = vec->size + 1;
    } else {
        VoiceNameAllocator growAlloc = VoiceNameAllocator();
        if (vec->maxSize() - vec->capacity < 1U) {
            ThrowVoiceNameLengthError();
        }
        unsigned int extra = vec->size + 1 - vec->capacity;
        VoiceNameStorage storage(&vec->capacity);
        unsigned int capacity = vec->nextCapacity(extra);
        VoiceNameAllocator reserveAlloc = VoiceNameAllocator();
        if (capacity > 0x15555555U) {
            ThrowVoiceNameLengthError();
        }
        storage.data = (VoiceNameString*)appMallocDummy__FUl(capacity * 12);
        storage.capacity = capacity;
        // Build the suffix before moving the old values, so a failed copy leaves
        // the vector intact.
        storage.append(vec->size, 1, local);
        int n = storage.prepend(vec->data + vec->size, vec->data);
        storage.install(vec, n);
    }
}
#pragma always_inline reset
#pragma use_lmw_stmw on
// Append the formatted name of a character voice, choosing the base or the
// alternate series by the voice number. Voice zero and a missing alternate index
// append nothing.
void CVoiceManager::AppendFormattedVoiceName(void* outputNames, int voiceNumber, void* characterIndex,
                                             void* alternateCharacterIndex) {
    if (voiceNumber == 0)
        return;
    VoiceFormatTable baseNameFormats = *(VoiceFormatTable*)voiceBaseCharacterNameFormats;
    VoiceFormatTable alternateNameFormats = *(VoiceFormatTable*)voiceAlternateCharacterNameFormats;
    int name, number;
    if (voiceNumber <= kBaseVoiceNumberCount) {
        name = baseNameFormats.v[(int)characterIndex];
        number = voiceNumber;
    } else {
        if (alternateCharacterIndex == 0)
            return;
        if ((int)alternateCharacterIndex == -1)
            return;
        name = alternateNameFormats.v[(int)alternateCharacterIndex];
        number = voiceNumber - kBaseVoiceNumberCount;
    }
    char voiceName[kVoiceNameBufferBytes];
    const char* strings = voiceManagerStrings;
    sprintf(voiceName, strings + kVoiceNameFormat, name, number);
    VoiceNameAllocator alloc;
    VoiceNameString local(voiceName, alloc);
    VoiceNameVector* vec = (VoiceNameVector*)outputNames;
    vec->push(local);
}
#pragma pop

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462578", _ctors_u802E6030_0, CVoiceManager::RegisterFactory);
#pragma pop
