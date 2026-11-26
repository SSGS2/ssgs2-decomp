// Stroke-play and match-play selection menus (CComSelectStroke and CComSelectMatch): course and
// rank textures, stroke-level and message resources, match descriptions, and round settings.
// The byte-vector length check used by the string helpers is here too.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "classes/MWStringStorage.h"
// ECB records are already endian-converted by their table loader.
struct MatchSelectionRecord {
    char reserved[5];
    signed char backNine;  // 0x05: zero selects the front-nine description
    signed char holeCount; // 0x06: 1, 3, 6, 9 or 18
};
struct StrokeLevelRecord {
    short recordId;
    unsigned char settings[9];
    unsigned char courseId;
    unsigned char firstHole; // output holes start at firstHole + 1
    unsigned char holeCount;
};
struct StrokeProgressFields {
    char reserved[0x214];
    signed char results[8][4]; // first row is skipped by the unlock scan
};
struct StrokeChatFontFields {
    char reserved[0x10];
    void* fixedFont;
};

// Byte offsets within the existing pooled strings; storage and encodings stay unchanged.
enum MatchDescriptionOffset {
    kMatchTextBackNine = 0x3a,       // MSG_COMMON_SELECT_BACK
    kMatchTextSeparator = 0x72,      // two spaces, UTF-16
    kMatchTextOneHole = 0x78,        // MSG_COMMON_HOLE_1
    kMatchTextThreeHoles = 0xa6,     // MSG_COMMON_HOLE_3
    kMatchTextSixHoles = 0xd4,       // MSG_COMMON_HOLE_6
    kMatchTextNineHoles = 0x102,     // MSG_COMMON_HOLE_9
    kMatchTextEighteenHoles = 0x130, // MSG_COMMON_HOLE_18
};

// Byte offsets within the existing pooled strings; storage and encodings stay unchanged.
enum StrokeTextOffset {
    kStrokeTextMessageFile = 0x1a,       // iff_msg.eky
    kStrokeTextVectorLengthError = 0x26, // vector length error
    kStrokeTextLengthErrorType = 0x3a,   // std::length_error type chain
};

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char comStrokeSelectionStrings[];
}
}

#include "sdk_protos.h"
#include "game/exception.h"
#include "classes/CComSelectMatch.h"
#include "classes/CComSelectStroke.h"
#include "classes/CSharedDoc.h"
#include "classes/CComSelectBase.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CBpTex.h"

#define AsCComSelectStroke(p) ((CComSelectStroke*)(p))

extern "C" {

extern float s_comSelectMenuFullTurnRadians;
extern const float kFloat_6_2831855;
extern void* s_comMatchDescriptionTags[];
extern void Text_ExpandTaggedText(void*, void*, void*, int);
extern void* g_sharedDoc;
extern void* ItemInventory_GetBallIdAt(void*, int);
extern void __ct__14CPopItemDialogFPvPv(void*, void*, int);
extern void* SharedFile_Create(void*, int);
extern void __dt__15SelectMatchBaseFv(void*, int);
extern void* Table_GetEntry(void*, void*);
extern void WString_ReplaceRange(void*, int, int, void*, void*, void*);
extern void Text_ExpandTaggedText(void*, void*, void*, int);
extern void __dl__FPv(void*);
extern void* g_chatMsg;
extern char __vt__16CComSelectStroke[];
extern void* s_comStrokeCourseTextureNames[];
extern void* s_comStrokeRankTextureNames[];
extern char comStrokeSelectionStrings[];
extern char comMatchSelectionText[];
extern char __vt__12CComMatchTbl[];
extern char __vt__15CComStrokeLevel[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern void* CSharedFilePtr_GetLoadedBuffer(void*);
extern void WFixedFont_CreateMsgEcbResource(void*, void**, void*, int);
extern void* SaveData_AcquireInitializedInstance();
extern void* SaveData_GetCurrentRecord(void*);
extern void* __nwa__FUl(unsigned int);
extern char* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void __dla__FPv(void*);
extern void __nw__FUl(unsigned int);
extern void __dt__Q23std12length_errorFv();
extern void* memcpy(void*, const void*, unsigned long);
extern void* memset(void*, int, unsigned long);
extern void __dt__Q33std6detail16char_array_guardFv(void*);
extern void OwnedVectorView_dtor(void*);

struct Interface8032464C {
    virtual void destroy(int);
    virtual void unused1();
    virtual void unused2();
    virtual void unused3();
    virtual int valid();
};

// Check all course and rank textures plus stroke-level and message files for readiness.
int CComSelectStroke::AreResourcesLoaded() {
    if (!((CComSelectBase*)this)->PollLoadComplete())
        return 0;
    struct Fields {
        char pad[0x54];
        void* first[14];
        void* second[8];
    };
    Fields* fields = (Fields*)this;
    for (unsigned int i = 0; i < 14; ++i) {
        void* object = fields->first[i];
        if (object && !((Interface8032464C*)object)->valid())
            return 0;
    }
    for (unsigned int i = 0; i < 8; ++i) {
        void* object = fields->second[i];
        if (object && !((Interface8032464C*)object)->valid())
            return 0;
    }
    void* object = strokeLevelFile;
    if (object && !CSharedFilePtr_GetLoadedBuffer(object))
        return 0;
    object = messageResourceFile;
    if (object && !CSharedFilePtr_GetLoadedBuffer(object))
        return 0;
    return 1;
}

// Recreate the stroke-selection message resource, then initialize the common selection UI.
void CComSelectStroke::InitMessageResourceAndMenu() {
    struct Self {
        char pad[0xB4];
        Interface8032464C* child;
    };
    Self* object = (Self*)this;
    if (object->child) {
        if (object->child)
            object->child->destroy(1);
        object->child = 0;
    }
    void* head = ((StrokeChatFontFields*)g_chatMsg)->fixedFont;
    void* parent = messageResourceFile;
    void* item = CSharedFilePtr_GetLoadedBuffer(parent);
    WFixedFont_CreateMsgEcbResource(head, (void**)&object->child, item, 1);
    ((CComSelectBase*)this)->StartMenuScripts();
}

// Return the first level whose saved result exceeds two, or eight when none does.
int CComSelectStroke::FindUnlockRow() {
    for (int row = 1; row < 8; ++row) {
        for (int col = 0; col < 4; ++col) {
            void* extraReceiver4 = (void*)SaveData_AcquireInitializedInstance();
            signed char* progress = (signed char*)SaveData_GetCurrentRecord(extraReceiver4) + col + row * 4;
            if (progress[(int)&((StrokeProgressFields*)0)->results] > 2)
                return row;
        }
    }
    return 8;
}

struct ScopedBuffer {
    char* data;
    char* allocator;
    // Free uncommitted array storage if shared-message construction throws.
    ~ScopedBuffer() {
        if (data)
            __dla__FPv(data);
    }
};

struct SharedCharDeleter {
    void* vtable;
    int count;
    int owned;
    char* data;
    // Adopt the exception message into a reference-counted deleter record.
    SharedCharDeleter(char* p) : vtable(s_sharedCharDeleterVtable), count(1), owned(1), data(p) {}
};

struct ErrorBase80324878 : std::exception {
    // Set the exception ABI vtable explicitly during staged construction.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    char* data;
    SharedCharDeleter* ref;
    // Initialize the logic_error base before filling message ownership.
    ErrorBase80324878() {
        SetTable(__vt__Q23std11logic_error);
    }
};

} // extern "C"

namespace std {
struct length_error : ErrorBase80324878 {
    ~length_error();
    // Copy the error message and transfer its allocation to a shared deleter.
    length_error(const char* message) {
        char* buffer;
        unsigned int n;
        const char* text;
        text = message;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        n = strlen(text) + 1;
        buffer = (char*)__nwa__FUl(n * 2 - n);
        data = buffer;
        {
            ScopedBuffer owner;
            owner.data = buffer;
            owner.allocator = &allocator.value;
            ref = new SharedCharDeleter(buffer);
            owner.data = 0;
        }
        strcpy(data, text);
        SetTable(__vt__Q23std12length_error);
    }
    DECOMP_CT_MEMBER(length_error)
};
}

extern "C" {
typedef std::length_error LengthError80324878;

// Construct and throw the byte-vector length_error through the Metrowerks exception ABI.
inline void lengthError80324878(const char* message) {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(LengthError80324878)];
        } storage;
        LengthError80324878* error = (LengthError80324878*)storage.bytes;
        error->__ct(message);
        exception_runtime::__throw(exception_runtime::comStrokeSelectionStrings + kStrokeTextLengthErrorType, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

struct ReplacementBuffer {
    char* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* allocator;
    unsigned int position;
    // Start an empty replacement-buffer owner with an allocator token.
    ReplacementBuffer(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), position(0) {}
    // Free whichever buffer remains after commit (the old one), or a failed replacement.
    ~ReplacementBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
    // Account for one pending byte, move the prefix, and clear old prefix bytes.
    void copy(char* first, char* last) {
        unsigned int count = (unsigned int)last - (unsigned int)first;
        ++size;
        position -= count;
        memcpy(data + position, first, count);
        memset(first, 0, count);
        size += count;
    }
};

// Borrow the larger capacity requirement without copying its reference identity.
inline const unsigned int& max80324878(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}

// Owned counted bytes, with no NUL-termination contract; used for numeric
// one-based hole numbers and course IDs. Growth keeps size <= capacity.
// Capacity policy: +max(needed,3*(cap+1)/5) below 0x55555555, then
// +max(needed,(cap+1)/2) below 0xAAAAAAAA, otherwise UINT_MAX.
struct StrokeSelectionByteVector {
    char* data;
    unsigned int size;
    unsigned int capacity;
    // Append the low byte of c; on a full vector transfer storage via Grow owner.
    void push_back(const unsigned int& c) {
        unsigned int currentSize = size;
        unsigned int currentCapacity = capacity;
        if (currentSize < currentCapacity) {
            ++size;
            data[size - 1] = c;
        } else {
            if (0xffffffffU - capacity < 1) {
                lengthError80324878(comStrokeSelectionStrings + kStrokeTextVectorLengthError);
            }
            unsigned int* allocator = &capacity;
            unsigned int extra = size + 1 - capacity;
            // Phase 1: guard a replacement buffer until capacity checks/allocation succeed.
            ReplacementBuffer grown(allocator);
            unsigned int cap = capacity;
            if (extra > 0xffffffffU - cap) {
                lengthError80324878(comStrokeSelectionStrings + kStrokeTextVectorLengthError);
            }
            unsigned int halfStep;
            unsigned int smallStep;
            if (cap < 0x55555555U) {
                smallStep = ((cap + 1) * 3) / 5;
                cap += max80324878(smallStep, extra);
            } else if (cap < 0xaaaaaaaaU) {
                halfStep = (cap + 1) / 2;
                cap += max80324878(halfStep, extra);
            } else
                cap = 0xffffffffU;
            grown.data = appMallocDummy__FUl(cap);
            grown.capacity = cap;
            unsigned int offset = size;
            grown.position = offset;
            memset(grown.data + offset + grown.size, (unsigned char)c, 1);
            // Phase 2: prefix copy clears the old bytes; swap commits new ownership.
            grown.copy(data, data + size);
            unsigned int newSize;
            unsigned int oldCapacity;
            char* replacement;
            char* previous;
            previous = data;
            replacement = grown.data;
            newSize = grown.size;
            oldCapacity = capacity;
            size = 0;
            grown.data = previous;
            capacity = grown.capacity;
            grown.capacity = oldCapacity;
            data = replacement;
            size = newSize;
            grown.size = 0;
        }
    }
};

// Output ABI: initial[4] at +0, holes vector at +4, courses vector at +0x10,
// recordId at +0x1C, data[12] at +0x1E, aligned sourceIndex at +0x2C.
// initial = {courseId, firstHole, 0, holeCount}; data = {settings[0..8],
// courseId, firstHole, holeCount}. Duplication serves two existing consumers.
// firstHole is zero-based; holes receives firstHole+1..firstHole+holeCount.
// Both vectors append to existing contents; the caller must clear for replacement.
struct RoundSettings {
    unsigned char initial[4];
    StrokeSelectionByteVector holes;
    StrokeSelectionByteVector courses;
    short recordId;
    unsigned char data[12];
    int sourceIndex;
};

struct CComSelectStrokeView {
    char pad0[0x1c];
    int index;
    char pad1[0x8c];
    void* source;
};

struct LookupGuard80325610 {
    void* object;
    void* vtable;
    // Borrowed table adapter; destruction does not release file-owned bytes.
    ~LookupGuard80325610() {}
};

#pragma push
#pragma use_lmw_stmw on
// Copy the selected computer stroke level and expand its course and hole-number vectors.
// Returns 0 for index=-1, otherwise 1 after copying/appending; a non-null loaded
// table and valid record index are required. out retains ownership of its vectors.
int CComSelectStroke::BuildRoundSettings(RoundSettings* out) {
    CComSelectStrokeView* self = (CComSelectStrokeView*)this;
    int holeOffset;
    unsigned char holeCount;
    unsigned char firstHole;
    unsigned char courseId;
    unsigned int holeNumber;
    unsigned int courseNumber;
    if (self->index == -1)
        return 0;
    void* levelTable = CSharedFilePtr_GetLoadedBuffer(self->source);
    LookupGuard80325610 guard;
    guard.object = 0;
    if (levelTable)
        guard.object = levelTable;
    guard.vtable = __vt__15CComStrokeLevel;
    StrokeLevelRecord* record = (StrokeLevelRecord*)((void* (*)(void*, void*, void*))Table_GetEntry)(
        guard.object, (void*)self->index, __vt__15CComStrokeLevel);
    // Phase 1: copy the scalar record/header without reinterpreting option bytes.
    out->recordId = record->recordId;
    out->data[0] = record->settings[0];
    out->data[1] = record->settings[1];
    out->data[2] = record->settings[2];
    out->data[3] = record->settings[3];
    out->data[4] = record->settings[4];
    out->data[5] = record->settings[5];
    out->data[6] = record->settings[6];
    out->data[7] = record->settings[7];
    out->data[8] = record->settings[8];
    courseId = record->courseId;
    out->data[9] = courseId;
    firstHole = record->firstHole;
    out->data[10] = firstHole;
    holeCount = record->holeCount;
    out->data[11] = holeCount;
    out->initial[0] = courseId;
    out->initial[2] = 0;
    out->initial[1] = firstHole;
    out->initial[3] = holeCount;
    // Phase 2: append paired hole/course bytes; signed holeCount controls the loop.
    for (holeOffset = 0; holeOffset < (signed char)out->data[11]; ++holeOffset) {
        holeNumber = out->data[10] + holeOffset + 1;
        out->holes.push_back(holeNumber);
        courseNumber = out->data[9];
        out->courses.push_back(courseNumber);
    }
    out->sourceIndex = self->index;
    return 1;
}
#pragma pop

struct CPopItemDialogStorage {
    char storage[0x30];
    // Initialize the ball-item dialog from inventory entry ten in mode two.
    CPopItemDialogStorage() {
        void* arg = ItemInventory_GetBallIdAt(((CSharedDoc*)g_sharedDoc)->itemInventory, 10); // original +0xA04
        __ct__14CPopItemDialogFPvPv(this, arg, 2);
    }
};

// Initialize the computer-selection angle constant to one full turn.
void CComSelectStroke_InitAngleConstant() {
    s_comSelectMenuFullTurnRadians = kFloat_6_2831855; // source group's cached full-turn angle
}

// Expand the indexed match description tag into the supplied text object.
void CComSelectMatch::ExpandMatchDescriptionText(int descriptionIndex, void* description) {
    if (description == 0)
        return;
    Text_ExpandTaggedText(messageDraw, description, s_comMatchDescriptionTags[descriptionIndex], 0);
}

// Create the computer-match ball-item dialog using inventory ball index ten.
void CComSelectMatch::CreateBallItemDialog(void** out) {
    void* object = new CPopItemDialogStorage;
    *out = object;
}

struct CBpTexStorage {
    char storage[0x28];
    // Initialize an owned texture wrapper before requesting its resource.
    CBpTexStorage() {
        ((CBpTex*)this)->__ct();
    }
};

struct Setup803243D4 {
    virtual void unused();
    virtual void setup(void*);
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_strength_reduction off
// Create course/rank texture objects and request stroke-level and message files.
CComSelectStroke::CComSelectStroke() {
    ((CComSelectBase*)this)->Init();
    *(void**)this = (void*)__vt__16CComSelectStroke;
    strokeLevelFile = 0;
    messageResourceFile = 0;
    messageDraw = 0;
    // Phase 1: construct the 14 course textures, then the eight rank textures.
    unsigned int offset = 0;
    void** arr = s_comStrokeCourseTextureNames;
    void* obj = 0;
    for (unsigned int i = 0; i < 14; ++i) {
        obj = new CBpTexStorage;
        *(void**)((char*)&courseTextures[0] + offset) = obj; // original +0x54
        void* value = *arr;
        ((Setup803243D4*)obj)->setup(value);
        offset += 4;
        ++arr;
    }
    arr = s_comStrokeRankTextureNames;
    unsigned int i = 0;
    offset = 0;
    for (; i < 8; ++i) {
        obj = new CBpTexStorage;
        *(void**)((char*)&rankTextures[0] + offset) = obj; // original +0x8C
        void* value = *arr;
        ((Setup803243D4*)obj)->setup(value);
        offset += 4;
        ++arr;
    }
    // Phase 2: retain asynchronous file handles for readiness polling and destruction.
    strokeLevelFile = SharedFile_Create(comStrokeSelectionStrings, 0);
    messageResourceFile = SharedFile_Create(comStrokeSelectionStrings + kStrokeTextMessageFile, 0);
}
#pragma pop

struct Destroyable {
    virtual void destroy(int);
};

#pragma push
#pragma use_lmw_stmw on
// Release course/rank textures and message resources before destroying the selection base.
CComSelectStroke::~CComSelectStroke() {
    *(void**)this = (void*)__vt__16CComSelectStroke;
    void* cur = this;
    void* zero = 0;
    // cur is a sliding address base, not a new object: field +0x54 +4*i
    // addresses the 14 course texture slots. Reset it for the eight +0x8C rank slots.
    cur = this;
    for (unsigned int i = 0; i < 14; ++i) {
        void* obj = AsCComSelectStroke(cur)->courseTextures[0];
        if (obj) {
            if (obj)
                ((Destroyable*)obj)->destroy(1);
            AsCComSelectStroke(cur)->courseTextures[0] = zero;
        }
        cur = (char*)cur + 4;
    }
    cur = this;
    for (unsigned int k = 0; k < 8; ++k) {
        void* obj = AsCComSelectStroke(cur)->rankTextures[0];
        if (obj) {
            if (obj)
                ((Destroyable*)obj)->destroy(1);
            AsCComSelectStroke(cur)->rankTextures[0] = zero;
        }
        cur = (char*)cur + 4;
    }
    void* p = strokeLevelFile;
    if (p) {
        CSharedFileHandle::Destroy(p, 1);
        strokeLevelFile = 0;
    }
    p = messageResourceFile;
    if (p) {
        CSharedFileHandle::Destroy(p, 1);
        messageResourceFile = 0;
    }
    p = messageDraw;
    if (p) {
        if (p)
            ((Destroyable*)p)->destroy(1);
        messageDraw = 0;
    }
    __dt__15SelectMatchBaseFv(this, 0);
}
#pragma pop

struct Local8032413C {
    void* p;
    void* q;
    // Borrowed match-table adapter; the shared-file handle owns its data.
    ~Local8032413C() {}
};

#pragma push
#pragma use_lmw_stmw on
// Describe front/back nine when applicable, append a separator, then append the match hole count.
void CComSelectMatch::DescribeMatchSelection(void* matchIndex, void* description) {
    if (description == 0)
        return;
    void* matchTable = CSharedFilePtr_GetLoadedBuffer(matchTableFile);
    Local8032413C lookup;
    lookup.p = 0;
    if (matchTable)
        lookup.p = matchTable;
    lookup.q = (void*)__vt__12CComMatchTbl;
    volatile char a;
    char b;
    void* record = Table_GetEntry(lookup.p, matchIndex);
    signed char holeCount = ((MatchSelectionRecord*)record)->holeCount;
    if (holeCount != 0x12) {
        record = Table_GetEntry(lookup.p, matchIndex);
        signed char backNine = ((MatchSelectionRecord*)record)->backNine;
        if (backNine == 0) {
            Text_ExpandTaggedText(messageDraw, description, (void*)comMatchSelectionText, 0);
        } else {
            Text_ExpandTaggedText(messageDraw, description, (void*)(comMatchSelectionText + kMatchTextBackNine), 0);
        }
        char* separator = comMatchSelectionText + kMatchTextSeparator;
        unsigned int stringTag = *(unsigned int*)description;
        unsigned int textLength;
        textLength =
            (!(stringTag >> 31)) ? *(unsigned char*)description & 0x7f : ((MWStringStorage*)description)->length;
        b = a;
        int separatorLength = wcslen((const wchar_t*)separator);
        int separatorBytes = separatorLength << 1;
        WString_ReplaceRange(description, textLength, 0, (void*)separator, (void*)(separator + separatorBytes),
                             (void*)&b);
    }
    // second part
    record = Table_GetEntry(lookup.p, matchIndex);
    signed char selectedHoleCount = (signed char)(unsigned char)((MatchSelectionRecord*)record)->holeCount;
    int selectedHoles = selectedHoleCount;
    switch (selectedHoles) {
    case 1:
        Text_ExpandTaggedText(messageDraw, description, (void*)(comMatchSelectionText + kMatchTextOneHole), 0);
        break;
    case 3:
        Text_ExpandTaggedText(messageDraw, description, (void*)(comMatchSelectionText + kMatchTextThreeHoles), 0);
        break;
    case 6:
        Text_ExpandTaggedText(messageDraw, description, (void*)(comMatchSelectionText + kMatchTextSixHoles), 0);
        break;
    case 9:
        Text_ExpandTaggedText(messageDraw, description, (void*)(comMatchSelectionText + kMatchTextNineHoles), 0);
        break;
    case 18:
        Text_ExpandTaggedText(messageDraw, description, (void*)(comMatchSelectionText + kMatchTextEighteenHoles), 0);
        break;
    default:
        break;
    }
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462604", _ctors_u8032413C_0, CComSelectStroke_InitAngleConstant);
#pragma pop

// Verify the partial views against the offsets used by this unit.
typedef char MatchSelectionRecord_backNine_offset[((int)&((MatchSelectionRecord*)0)->backNine == 0x5) ? 1 : -1];
typedef char MatchSelectionRecord_holeCount_offset[((int)&((MatchSelectionRecord*)0)->holeCount == 0x6) ? 1 : -1];
typedef char StrokeLevelRecord_courseId_offset[((int)&((StrokeLevelRecord*)0)->courseId == 0xb) ? 1 : -1];
typedef char StrokeLevelRecord_holeCount_offset[((int)&((StrokeLevelRecord*)0)->holeCount == 0xd) ? 1 : -1];
typedef char StrokeProgressFields_results_offset[((int)&((StrokeProgressFields*)0)->results == 0x214) ? 1 : -1];
