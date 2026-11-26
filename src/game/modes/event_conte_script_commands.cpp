// Story (Conte) event script commands: the handlers and skip helpers for CEventConteProc's
// command set, including the if/else/goto flow, the camera set command and the timing waits.
// Also holds the string, vector and queue helpers these handlers use.

#include "MSL/error_message.h"
#include "ppc_double_words.h"
#include "game/string.h"
#include "classes/EventScript80091824.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>
#include "classes/CEventConteProc.h"

extern "C" {

typedef EventScript80091824 State_80091E;

// Conditional-state map (shared member names are retained):
// counter = current nesting depth (IF enters 1..15; ENDIF pops one).
// flags = suppressed-branch mask: any bit skips ordinary command side effects.
// extraFlags = branch-already-taken mask: ELSEIF/ELSE use it to avoid a second arm.
// Each depth uses 1u<<counter in both masks. ENDIF clears that depth in both.
// IF resets the two result bytes; mode 0 is AND, nonzero is OR. Results are
// scratch for the current test list, not a per-depth stack of accumulated tests.
// Conditional handlers mask flags with 0xFFFE, allowing bit 0 alone; the origin
// of that special bit is not established here. A blocked enclosing depth still
// prevents THEN from recording a taken branch, even if its local tests passed.
// Command handlers return 0 while waiting (cursor unchanged), 1 when complete.

// The first two bytes of the shared header's large scratch region accumulate IF results.
// AND mode requires all tested conditions to pass; OR mode requires any passing condition.
struct ScriptConditionResults {
    unsigned char combineMode, nestingDepth;
    unsigned short suppressedBranches, takenBranches;
    unsigned char testedCount, passedCount;
};
typedef char ScriptConditionResults_size[(sizeof(ScriptConditionResults) == 8) ? 1 : -1];

// The shared class leaves the embedded label in padding; expose its 12-byte string representation here.
struct ScriptLabelView {
    char prefix[0x408];
    union {
        struct {
            unsigned int flags, length;
            char* data;
        } heap;
        struct {
            unsigned char tag;
            char text[11];
        } small;
    } label;
};
enum ScriptLimits {
    kMaxConditionDepth = 15,
    kInactiveOuterConditions = 0xfffe,
    kMaxQueuedSources = 0x3fffffff
};
// Keep this field-derived byte view: direct member access changes register allocation in camera enqueue.
enum {
    kQueuedCapacityOffset = (int)&((State_80091E*)0)->queuedCapacity
};

extern int EventTiming_IsDurationScalingEnabled();
extern void CourseSlotTable_InitEmpty(const char*);
extern volatile signed char s_courseSlotTableInitialized;
extern const char s_conteModelSlots[];
extern const double kEventWaitUnsignedConversionBias;
extern const float kFloat_5_BpScript;
extern const float kFloat_6_EventConteWait;
extern const float kFloat_0_5_EventScriptSoundMinimumGain;
extern const float kFloat_200_EventScriptSoundRange;
extern const float kFloat_1_TourWindow;
extern volatile signed char s_inputManagerInitialized;
extern volatile int g_soundManager;
extern const char s_storyWindowStaticStorage[];
struct StoryResourceStringPool {
    char confirmSound[19];        // 0
    char vectorLengthError[20];   // 19
    char lengthErrorTypeName[55]; // 39
    char empty[1];                // 94
    char vcgExtension[5];         // 95
    char facialData[11];          // 100
    char treeLengthError[26];     // 111
    char reserveLengthError[35];  // 137
    char outOfRange[27];          // 172
    char outOfRangeTypeName[55];  // 199
    char stringLengthError[27];   // 254
};
extern const StoryResourceStringPool bpPlayerResourceStrings;
extern void InputManager_Init(void*);
extern void InputManager_Dtor();
extern int Input_IsActionPressed(void*, int, int);
extern void Sound_PlayByName(int, const char*, int, int, int, int, float, float, float);
extern int String_ReplaceRange(void*, int, int, const char*, const char*, char*);
extern char s_gameInputManager[];
extern void* conteCommandNames[];
extern char __vt__15CEventConteProc[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern void __ct__22CEventCameraSC_CAM_SETFPCc(void*, const char*);
extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);

// Stop the script only when no enclosing condition suppresses this command.
int CEventConteProc::ProcessExitCommand() {
    State_80091E* self = (State_80091E*)this;
    unsigned short f = self->flags;
    int cursor = self->cursor;
    if ((f & kInactiveOuterConditions) != 0) {
        self->cursor = cursor + 4;
        return 1;
    }
    self->cursor = 0;
    return 1;
}

// Skip a four-byte command; preserve volatile reads and writes of the script cursor.
int CEventConteProc::SkipCommand4() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    if (flags != 0) {
        self->cursor = cursor + 4;
        return 1;
    }
    int* cp = (int*)&self->cursor; // 0x418; retain the non-volatile view
    *cp = cursor + 4;
    return 1;
}

// Skip an eight-byte command, including its one-word payload.
int CEventConteProc::SkipCommand8() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    int* cp = (int*)&self->cursor; // 0x418; retain the non-volatile view
    if (flags != 0) {
        self->cursor = cursor + 8;
        return 1;
    }
    *cp = cursor + 8;
    return 1;
}

// Skip a twelve-byte command, including its two-word payload.
int CEventConteProc::SkipCommand12() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    int* cp = (int*)&self->cursor; // 0x418; retain the non-volatile view
    if (flags != 0) {
        self->cursor = cursor + 12;
        return 1;
    }
    *cp = cursor + 12;
    return 1;
}

// Skip a sixteen-byte command with no active behavior in this interpreter.
int CEventConteProc::SkipCommand16() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    int* cp = (int*)&self->cursor; // 0x418; retain the non-volatile view
    if (flags != 0) {
        self->cursor = cursor + 16;
        return 1;
    }
    *cp = cursor + 16;
    return 1;
}

// Skip a twelve-byte command with no active behavior in this interpreter.
int CEventConteProc::SkipCommand12_C() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    int* cp = (int*)&self->cursor; // 0x418; retain the non-volatile view
    if (flags != 0) {
        self->cursor = cursor + 12;
        return 1;
    }
    *cp = cursor + 12;
    return 1;
}

// Enter a conditional block and clear its branch-taken and suppression bits.
int CEventConteProc::ProcessIfCommand() {
    State_80091E* self = (State_80091E*)this;
    unsigned char nestingDepth = self->counter;
    char* cursor = *(char**)&self->cursor;
    unsigned char combineMode = cursor[1];
    if (nestingDepth < kMaxConditionDepth) {
        unsigned char enteredDepth = nestingDepth + 1;
        unsigned int depthBit = 1 << enteredDepth;
        unsigned short suppressedBranches = self->flags;
        unsigned short takenBranches = self->extraFlags;
        self->counter = enteredDepth;
        self->flags = suppressedBranches & ~depthBit;
        self->extraFlags = takenBranches & ~depthBit;
        self->conditionCombineMode = combineMode;
        ((ScriptConditionResults*)self)->passedCount = 0;
        ((ScriptConditionResults*)self)->testedCount = 0;
    }
    *(char**)&self->cursor = cursor + 4;
    return 1;
}

// Start an alternate condition unless an earlier branch at this depth already ran.
int CEventConteProc::ProcessElseIfCommand() {
    State_80091E* self = (State_80091E*)this;
    unsigned char nestingDepth = self->counter;
    unsigned int depthBit = 1u << nestingDepth;
    unsigned short takenBranches = self->extraFlags;
    int cursor = self->cursor;
    if (takenBranches & depthBit) {
        unsigned short suppressedBranches = self->flags;
        self->flags = suppressedBranches | depthBit;
    } else {
        unsigned short suppressedBranches = self->flags;
        self->flags = suppressedBranches & ~depthBit;
        self->conditionCombineMode = *(unsigned char*)(cursor + 1);
        ((ScriptConditionResults*)self)->passedCount = 0;
        ((ScriptConditionResults*)self)->testedCount = 0;
    }
    self->cursor = cursor + 4;
    return 1;
}

// Enter the final alternate branch and mark this conditional as handled.
int CEventConteProc::ProcessElseCommand() {
    State_80091E* self = (State_80091E*)this;
    unsigned int depthBit = 1u << self->counter;
    unsigned short takenBranches = self->extraFlags;
    int cursor = self->cursor;
    if (takenBranches & depthBit) {
        unsigned short suppressedBranches = self->flags;
        self->flags = suppressedBranches | depthBit;
    } else {
        unsigned short suppressedBranches = self->flags;
        self->flags = suppressedBranches & ~depthBit;
    }
    unsigned char currentDepth = self->counter;
    unsigned short takenBranchesAfterTest = self->extraFlags;
    int newCursor = cursor + 4;
    unsigned int currentDepthBit = 1u << currentDepth;
    self->cursor = newCursor;
    self->extraFlags = takenBranchesAfterTest | currentDepthBit;
    return 1;
}

// Leave one conditional level and clear the bits belonging to that level.
int CEventConteProc::ProcessEndIfCommand() {
    State_80091E* self = (State_80091E*)this;
    unsigned char nestingDepth = self->counter;
    unsigned short suppressedBranches = self->flags;
    int cursor = self->cursor;
    self->flags = suppressedBranches & ~(1u << nestingDepth);
    if (nestingDepth != 0) {
        unsigned short takenBranches = self->extraFlags;
        self->counter = nestingDepth - 1;
        self->extraFlags = takenBranches & ~(1 << nestingDepth);
    }
    self->cursor = cursor + 4;
    return 1;
}

// Skip an eight-byte command with no active behavior in this interpreter.
int CEventConteProc::SkipCommand8_B() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    int* cp = (int*)&self->cursor; // 0x418; retain the non-volatile view
    if (flags != 0) {
        self->cursor = cursor + 8;
        return 1;
    }
    *cp = cursor + 8;
    return 1;
}

// Jump to a little-endian byte offset relative to the script base and pop one condition.
int CEventConteProc::ProcessGotoCommand() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    if (flags & kInactiveOuterConditions) {
        self->cursor = cursor + 4;
        return 1;
    }
    unsigned short length = *(unsigned short*)(cursor + 2);
    volatile unsigned short swapped = length;
    {
        unsigned char* bytes = (unsigned char*)&swapped;
        unsigned char tmp = bytes[0];
        bytes[0] = bytes[1];
        bytes[1] = tmp;
    }
    unsigned char counter = self->counter;
    if (counter != 0) {
        unsigned short oldFlags = self->extraFlags;
        int one = 1;
        int mask = one << counter;
        self->extraFlags = oldFlags & ~mask;
        self->counter = counter - 1;
    }
    self->cursor = self->lengthBase + swapped;
    return 1;
}

// Return bytes to skip a NUL-terminated string plus padding to the next word.
static unsigned int advance_string(char* string) {
    unsigned int length = strlen(string);
    return (length + 4) & ~3;
}

// Skip a command containing three strings, each padded to a four-byte boundary.
int CEventConteProc::SkipThreeStrings() {
    State_80091E* self = (State_80091E*)this;
    char* cursor = (char*)self->cursor + 4;
    cursor += advance_string(cursor);
    cursor += advance_string(cursor);
    unsigned int size = advance_string(cursor);
    self->cursor = (int)(cursor + size);
    return 1;
}

// Skip a command containing two strings, each padded to a four-byte boundary.
int CEventConteProc::SkipTwoStrings() {
    State_80091E* self = (State_80091E*)this;
    char* cursor = (char*)self->cursor + 4;
    cursor += advance_string(cursor);
    unsigned int size = advance_string(cursor);
    self->cursor = (int)(cursor + size);
    return 1;
}

// Skip a command containing one string padded to a four-byte boundary.
int CEventConteProc::SkipOneString() {
    State_80091E* self = (State_80091E*)this;
    char* string = (char*)self->cursor + 4;
    unsigned int length = strlen(string);
    self->cursor = (int)(string + ((length + 4) & ~3));
    return 1;
}

// Skip an eight-byte command unconditionally.
int CEventConteProc::Advance8() {
    State_80091E* self = (State_80091E*)this;
    self->cursor += 8;
    return 1;
}

// Skip a second eight-byte command unconditionally.
int CEventConteProc::Advance8_B() {
    State_80091E* self = (State_80091E*)this;
    self->cursor += 8;
    return 1;
}

// Skip a four-byte command unconditionally.
int CEventConteProc::Advance4() {
    State_80091E* self = (State_80091E*)this;
    self->cursor += 4;
    return 1;
}

// Wait one update, then an action press, then one more update before advancing.
int EventCommand_WaitForActionPress(State_80091E* self) {
    unsigned short flags = self->flags;
    char* cursor = (char*)self->cursor;
    if (flags != 0) {
        self->cursor = (int)(cursor + 4);
        return 1;
    }
    int count = self->count;
    if (count == 0) {
        self->count = count + 1;
        return 0;
    }
    if ((unsigned int)count == 1) {
        if (((volatile int&)GetInputManagerSlot()) == 0) {
            if (!s_inputManagerInitialized) {
                InputManager_Init(s_gameInputManager);
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                         (void*)s_storyWindowStaticStorage);
                s_inputManagerInitialized = 1;
            }
            ((volatile int&)GetInputManagerSlot()) = (int)s_gameInputManager;
        }
        if (Input_IsActionPressed((void*)((volatile int&)GetInputManagerSlot()), *(unsigned char*)(cursor + 1), -1)) {
            Sound_PlayByName(g_soundManager, bpPlayerResourceStrings.confirmSound, 0, 0, 0, 0,
                             kFloat_0_5_EventScriptSoundMinimumGain, kFloat_200_EventScriptSoundRange,
                             kFloat_1_TourWindow);
            self->count++;
        }
        return 0;
    }
    self->cursor = (int)(cursor + 4);
    return 1;
}

struct Source_80091F68 {
    virtual ~Source_80091F68();
    virtual int update(unsigned int);
    unsigned char data[0x10];
};
struct CEventCameraSC_CAM_SETStorage {
    unsigned char data[0x14];
    // Build the camera source from the borrowed encoded command.
    CEventCameraSC_CAM_SETStorage(const char* base) {
        __ct__22CEventCameraSC_CAM_SETFPCc(this, base);
    }
};

struct Allocator_80091F68 {
    char value;
};
struct String_80091F68 {
    union {
        unsigned int words[3];
        struct {
            CW_BITS2(unsigned int isLong : 1, unsigned int unused : 31) unsigned int length, heapDataWord;
        } flags;
    };
    // Leave storage untouched; TextNode explicitly calls init before use.
    String_80091F68() {}
    // Set an empty inline string representation.
    void init() {
        words[0] = 0;
        words[1] = 0;
        words[2] = 0;
    }
    // Release heap string storage, if the tag selects it.
    ~String_80091F68() {
        ((NString*)(this))->~NString();
    }
    // Read the byte length from the active small/heap representation.
    unsigned int size() const {
        if (flags.isLong == 0)
            return *(const unsigned char*)this & 0x7f;
        else
            return words[1];
    }
    // Replace this string with an owned copy of the borrowed NUL-terminated text.
    void assign(const char* text, const Allocator_80091F68& allocator) {
        unsigned int old = size();
        Allocator_80091F68 copy(allocator);
        String_ReplaceRange(this, 0, old, text, text + strlen(text), (char*)&copy);
    }
};

struct TextNode_80091F68 {
    void* reserved;
    Source_80091F68* source;
    String_80091F68 name;
    String_80091F68 kind;
    void* vtable;
    // Wrap the source for asynchronous ownership, copying its label and command name.
    TextNode_80091F68(const char* text, const char* type, Source_80091F68* src) {
        vtable = __vt__15CEventConteProc;
        name.init();
        kind.init();
        reserved = 0;
        source = src;
        Allocator_80091F68 nameAllocator;
        name.assign(text, nameAllocator);
        Allocator_80091F68 kindAllocator;
        kind.assign(type, kindAllocator);
    }
};

// Shared RTTI: std::tr1::detail::shared_ptr_deleter<char, std::detail::default_delete<char[]> >.
// Use the common 16-byte ABI view in MSL/error_message.h; keep this local alias for callers.
typedef ::std::detail::__message_control SharedCharDeleterView;

struct ArrayOwner {
    void* data;
    void* allocator;
    // Guard array storage until its exception control block takes ownership.
    ArrayOwner(void* p, void* a) : data(p), allocator(a) {}
    // Free the array on an exceptional exit before ownership transfer.
    ~ArrayOwner() {
        if (data)
            __dla__FPv(data);
    }
};

// Throw the standard vector length exception used by the asynchronous camera queue.
static inline void length_error_80091F68() {
    const char* text = bpPlayerResourceStrings.vectorLengthError;
    struct Error {
        void* vtable;
        char* text;
        SharedCharDeleterView* ref;
    } error;
    Allocator_80091F68 allocator;
    allocator.value = 0;
    error.vtable = __vt__Q23std11logic_error;
    unsigned int length = strlen(text) + 1;
    error.text = (char*)__nwa__FUl((length * 2) - length);
    {
        char* data = error.text;
        ArrayOwner owner(data, &allocator);
        error.ref = new SharedCharDeleterView(data);
        owner.data = 0;
    }
    strcpy(error.text, text);
    error.vtable = __vt__Q23std12length_error;
    __throw(bpPlayerResourceStrings.lengthErrorTypeName, &error, (void*)__dt__Q23std12length_errorFv);
}

struct QueueGrowthBuffer {
    void** data;
    unsigned int count, capacity;
    unsigned int* oldCapacity;
    unsigned int position;
    // Start an empty temporary pointer allocation for queue growth.
    QueueGrowthBuffer(unsigned int* cap) : data(0), count(0), capacity(0), oldCapacity(cap), position(0) {}
    // Free backing storage only; source pointers have been transferred, not cloned.
    ~QueueGrowthBuffer() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
};

#pragma push
#pragma use_lmw_stmw on
// Run a camera command synchronously, or transfer it to the named asynchronous source queue.
int CEventConteProc::ProcessCameraSetCommand() {
    State_80091E* self = (State_80091E*)this;
    char* base = (char*)self->cursor;
    char* text = base + 4;
    unsigned int n = strlen(text);
    char* next = text + ((n + 4) & ~3u);
    if (self->flags != 0) {
        self->cursor = (int)(next + 4);
        return 1;
    }
    if (*(int*)&self->count == 0) {
        CEventCameraSC_CAM_SETStorage* mem = new CEventCameraSC_CAM_SETStorage(base);
        self->activeSource = mem;
    }
    if ((unsigned char)base[5] == 0) {
        Source_80091F68* p = *(Source_80091F68**)&self->activeSource;
        if (!p->update(*(unsigned int*)&self->count)) {
            if (*(unsigned int*)&self->count < 0xffffffffu)
                ++*(unsigned int*)&self->count;
            return 0;
        }
        p = *(Source_80091F68**)&self->activeSource;
        if (p) {
            delete p;
            self->activeSource = 0;
        }
    } else {
        TextNode_80091F68* node = new TextNode_80091F68(
            (((ScriptLabelView*)self)->label.heap.flags >> 31) == 0 ? ((ScriptLabelView*)self)->label.small.text
                                                                    : ((ScriptLabelView*)self)->label.heap.data,
            (const char*)conteCommandNames[(unsigned char)base[0]], *(Source_80091F68**)&self->activeSource);
        void*** first = &self->queuedItems;    // 0x434
        unsigned int& end = self->queuedCount; // 0x438
        // Append asynchronous camera sources, growing the pointer array when it is full.
        if (end < *(unsigned int*)((char*)self + kQueuedCapacityOffset)) {
            ++end;
            (*first)[end - 1] = node;
        } else {
            if ((unsigned int)kMaxQueuedSources - *(unsigned int*)((char*)self + kQueuedCapacityOffset) < 1)
                length_error_80091F68();
            unsigned int additional = end + 1 - *(unsigned int*)((char*)self + kQueuedCapacityOffset);
            QueueGrowthBuffer buffer((unsigned int*)((char*)self + kQueuedCapacityOffset));
            unsigned int oldcap = *(unsigned int*)((char*)self + kQueuedCapacityOffset);
            if (additional > (unsigned int)kMaxQueuedSources - oldcap)
                length_error_80091F68();
            unsigned int newcap;
            if (oldcap < 0x15555555u) {
                unsigned int growth = (oldcap + 1) * 3 / 5;
                newcap = oldcap + *(growth < additional ? &additional : &growth);
            } else if (oldcap < 0x2aaaaaaau) {
                unsigned int growth = (oldcap + 1) / 2;
                newcap = oldcap + *(growth < additional ? &additional : &growth);
            } else
                newcap = 0x3fffffff;
            if (newcap > (unsigned int)kMaxQueuedSources)
                length_error_80091F68();
            buffer.data = (void**)appMallocDummy__FUl(newcap * 4);
            unsigned int oldsize = end;
            buffer.position = oldsize;
            (buffer.data + oldsize)[buffer.count++] = node;
            void** old = *first;
            buffer.capacity = newcap;
            unsigned int moved = ((int)(old + end) - (int)old) / 4;
            buffer.position = oldsize - moved;
            memcpy(buffer.data + buffer.position, old, moved * 4);
            memset(old, 0, moved * 4);
            buffer.count += moved;
            end = 0;
            void** storage = buffer.data;
            buffer.data = *first;
            unsigned int capacity = *(unsigned int*)((char*)self + kQueuedCapacityOffset);
            *(unsigned int*)((char*)self + kQueuedCapacityOffset) = buffer.capacity;
            buffer.capacity = capacity;
            *first = storage;
            end = buffer.count;
            buffer.count = 0;
        }
        self->activeSource = 0;
    }
    self->cursor = (int)(next + 4);
    return 1;
}

#pragma pop
#pragma push
#pragma use_lmw_stmw on
// Read the padded script name, append .vcg, and advance past the trailing word.
int EventScript_ReadVcgFilename(State_80091E* self, void* vcgFilename) {
    char s[3][8];
    char* cursor = *(char**)&self->cursor + 4;
    unsigned int length = strlen(cursor);
    char* end = cursor + ((length + 4) & ~3);
    char* strings = (char*)bpPlayerResourceStrings.empty;
    unsigned int value;
    if (((unsigned int)*(int*)vcgFilename >> 31) == 0)
        value = *(unsigned char*)vcgFilename & 0x7f;
    else
        value = ((NString*)vcgFilename)->length;
    s[2][0] = s[2][4];
    char* temp = &s[2][0];
    String_ReplaceRange(vcgFilename, 0, value, strings, strings + strlen(strings), temp);
    value = (((unsigned int)*(int*)vcgFilename >> 31) == 0) ? *(unsigned char*)vcgFilename & 0x7f
                                                            : ((NString*)vcgFilename)->length;
    s[1][0] = s[1][4];
    char* temp2 = &s[1][0];
    String_ReplaceRange(vcgFilename, value, 0, cursor, cursor + strlen(cursor), temp2);
    strings = (char*)bpPlayerResourceStrings.vcgExtension;
    if (((unsigned int)*(int*)vcgFilename >> 31) == 0)
        value = *(unsigned char*)vcgFilename & 0x7f;
    else
        value = ((NString*)vcgFilename)->length;
    s[0][0] = s[0][4];
    char* temp3 = &s[0][0];
    String_ReplaceRange(vcgFilename, value, 0, strings, strings + strlen(strings), temp3);
    *(char**)&self->cursor = end + 4;
    return 1;
}
#pragma pop

// Combine accumulated condition results and either enable or suppress this branch.
// AND with no tests passes (0==0); OR with no tests fails (passedCount==0).
// flags&0xFFFE short-circuits without touching either mask. On success record
// this depth in extraFlags (so later arms skip); on failure set flags at this
// depth (so ordinary commands skip). Neither path clears bits; IF/ELSEIF do that.
int CEventConteProc::ProcessThenCommand() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    if (flags & kInactiveOuterConditions) {
        self->cursor = cursor + 4;
        return 1;
    }
    int enabled;
    if (self->conditionCombineMode == 0)
        enabled = ((ScriptConditionResults*)self)->testedCount == ((ScriptConditionResults*)self)->passedCount;
    else
        enabled = ((ScriptConditionResults*)self)->passedCount != 0;
    if (enabled) {
        unsigned char counter = self->counter;
        unsigned short old = self->extraFlags;
        self->extraFlags = old | (1 << counter);
    } else {
        unsigned char counter = self->counter;
        unsigned short old = self->flags;
        self->flags = old | (1 << counter);
    }
    self->cursor = cursor + 4;
    return 1;
}

// Wait for the encoded update count, scaling the limit when the timing mode requires it.
// Payload +4 is an unsigned count of interpreter updates, not milliseconds.
// Timing scaling changes limits >=2 to trunc(float(limit)*5/6); 0/1 stay exact.
// count is compared before incrementing: limit N returns 0 N times, then 1.
// The 0x43300000 high word constructs 2^52+limit on big-endian PPC; subtracting
// 2^52 converts the unsigned word to a double before the original float rounding.
int CEventConteProc::ProcessWaitCommand() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    char* cursor = *(char**)&self->cursor;
    if (flags != 0) {
        *(char**)&self->cursor = cursor + 8;
        return 1;
    }
    unsigned int limit = *(unsigned int*)(cursor + 4);
    int enabled = EventTiming_IsDurationScalingEnabled();
    // Original 0x80091824: only convert limits of at least two.
    if (enabled && limit >= 2) {
        union {
            double d;
            unsigned int words[2];
        } converted;
        converted.words[PPC_DW_HI] = 0x43300000;
        converted.words[PPC_DW_LO] = limit;
        float delta = (float)(converted.d - kEventWaitUnsignedConversionBias);
        float scaled = kFloat_5_BpScript * delta;
        float value = scaled / kFloat_6_EventConteWait;
        limit = (unsigned int)Runtime_ConvertDoubleToUnsigned((double)value);
    }
    unsigned int count = self->count;
    if (count < limit) {
        self->count = count + 1;
        return 0;
    }
    *(char**)&self->cursor = cursor + 8;
    return 1;
}

// Request a conte model-slot change on the first update and wait for its completion.
int CEventConteProc::WaitForModelSlotChange() {
    State_80091E* self = (State_80091E*)this;
    unsigned short flags = self->flags;
    int cursor = self->cursor;
    if (flags != 0) {
        self->cursor = cursor + 4;
        return 1;
    }
    if (((volatile int&)GetConteModelSlotsSlot()) == 0) {
        if (!s_courseSlotTableInitialized) {
            CourseSlotTable_InitEmpty(s_conteModelSlots);
            s_courseSlotTableInitialized = 1;
        }
        ((volatile int&)GetConteModelSlotsSlot()) = (int)s_conteModelSlots;
    }
    unsigned int count = self->count;
    int* shared = (int*)((volatile int&)GetConteModelSlotsSlot());
    if (count == 0) {
        *(unsigned short*)shared = *(unsigned short*)(cursor + 2);
        shared[1] = 1;
    }
    if (shared[1] != 0) {
        unsigned int c = self->count;
        if (c < 0xffffffffu)
            self->count = c + 1;
        return 0;
    }
    self->cursor = cursor + 4;
    return 1;
}

} // extern "C"
