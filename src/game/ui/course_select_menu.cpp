// The course select menu (CCourseSelect): course cards and pages, round options (hole count,
// hole order, weather), course unlock states, and the one-hole selector.
//
// Applying a mini-game course loads its hole table and writes the round settings.

extern "C" char s_courseSelectBpControllerDtorRegistration[];
extern "C" char s_courseSelectBpHandleDtorRegistration[];
extern "C" char s_inputManagerDtorRegistration_CourseSelect[];
extern "C" {
extern unsigned char kPartyCourseTableCourseIds[];
}
#include "classes/SavePlayerRecord.h" // CCourseSelect::DrawCourseCards: save record course bits at +0x258
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))

#include "game/singletons.h"
#include "classes/CCourseSelect.h"
#include "classes/CPackTextureResource.h"
#include "classes/CHoleNoSelect.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CMiniGameHole.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpCtrlHandle.h"
#include "classes/CFontNumDraw.h"
#include "classes/CMsgDraw.h"
extern "C" char bgmCollectionStrings[];

#define AsCCourseSelect(p) ((CCourseSelect*)(p))
enum CourseMenuConstants {
    kFullRoundHoles = 18,
    kCourseCardCount = 19,
    kUnlockCourseCount = 14,
    kCoursesPerPage = 5,
    kCourseTextureBytes = 0x28,
    kSelectedHoleText = 0x3F,
    kConfirmElement = 0x2C,
    kDescriptionAnchorElement = 0x2D,
    kOptionHighlightOffset = 0x16,
    kPlayerFlagChannel = 5,
    kCourseButtonFirst = 3,
    kCourseButtonLast = 11,
    kOrdinaryPageButtonLast = 7,
    kLastPageButtonFirst = 8,
    kHoleOptionFirst = 12,
    kHoleOptionLast = 16,
    kOrderOptionFirst = 17,
    kOrderOptionLast = 19,
    kWeatherOptionFirst = 20,
    kWeatherOptionLast = 21,
    kLayoutNormal = 0,
    kLayoutSelected = 2,
    kLayoutUnavailable = 3
};
enum RoundSelectionMode {
    kDartsCourseMode = 1,
    kDriveCourseMode = 2,
    kPartyCourseTable = 3
};
// Front/back order depends on holeCount; both codes produce holes 1..18 for a full round.
enum RoundHoleOrder {
    kFrontOrder = 0,
    kBackOrder = 1,
    kShuffledHoles = 3
};
enum RoundWeather {
    kClearWeather = 0,
    kRainyWeather = 2
};
// Layout ID map (highlight/control pairs differ by kOptionHighlightOffset):
// 00..02 page tabs; 03..07 ordinary-page cards; 08..0B final-page cards;
// 0C single-hole selector; 0D..10 hole-count presets in kCourseSelectionHoleCounts;
// 11..13 order presets in kCourseSelectionHoleOrders; 14 clear / 15 rain;
// 19..21 card rectangles; 2C confirm; 2D description anchor; 2E unresolved element.
// Option states: 0 normal, 2 selected, 3 unavailable. Setting an option state
// also clears player channel 5; visual state and input flags are separate.
enum CourseMenuElement {
    kPageTabFirst = 0,
    kPageTabLast = 2,
    kSingleHoleOption = 0x0C,
    kHolePreset1 = 0x0D,
    kHolePreset2 = 0x0E,
    kHolePreset3 = 0x0F,
    kHolePreset4 = 0x10,
    kOrderPreset0 = 0x11,
    kOrderPreset1 = 0x12,
    kOrderPreset2 = 0x13,
    kClearWeatherOption = 0x14,
    kRainWeatherOption = 0x15,
    kCardRectFirst = 0x19,
    kFinalPageCardRectFirst = 0x1E,
    kCardRectLast = 0x21,
    kUnresolvedElement2E = 0x2E,
    kLastOrdinaryCoursePage = 2,
    kFinalCoursePage = 3,
    kCardStripPrimitive = 0x98,
    kCardVertexBytes = 0x18
};
// Group reset and activation use this same inclusive ID contract:
// group       controls             paired visuals          availability rule
// page        00..02               16..18                  page selection
// course      03..07 OR 08..0B      card itself             page 1/2 OR page 3
// holes       0C..10               22..26                  preset restrictions
// order       11..13               27..29                  one hole: all disabled;
//                                                         18 holes: back disabled
// weather     14..15               2A..2B                  weatherOptionEnabled
// confirm     2C                   none                    delayed after selection
// Card rectangles 19..21 are geometry anchors, NOT selectable card IDs. Page tabs
// are zero-based IDs, whereas coursePage is 1..3. Avoid mixing those two domains.
enum CourseOptionVisual {
    kFrontOrderHighlight = kOrderPreset0 + kOptionHighlightOffset,
    kBackOrderHighlight = kOrderPreset1 + kOptionHighlightOffset,
    kShuffleOrderHighlight = kOrderPreset2 + kOptionHighlightOffset
};
enum RoundPayloadWord {
    kVectorDataWord = 0,
    kVectorCountWord = 1,
    kVectorCapacityWord = 2
};
enum RoundPayloadByte {
    kRoundCourseByte = 0,
    kRoundOrderByte = 1,
    kRoundWeatherByte = 2,
    kRoundHoleCountByte = 3
};
enum CourseTextOffset {
    kVectorLengthErrorType = 0x14,
    kCourseMenuScript = 0x99,
    kCourseNumberFont = 0xA7,
    kBitsetRangeErrorType = 0xB7
};

namespace std {
class out_of_range {
public:
    explicit out_of_range(const char*);
    virtual ~out_of_range();
};
}

// ABI view of CCourseSelect: signed selections and volatile vector counts retain the original accesses.
struct CourseSelectStateView {
    char loadedTable[0x18];
    signed char selectedCourseIndex;
    signed char holeOrder;
    unsigned char weatherMode; // Round settings byte 2; 0 clear, 2 rain (rain doubles earned Pang).
    unsigned char holeCount;
    char* holeNumbers;
    volatile int holeNumberCount;
    int holeNumberCapacity;
    char* holeCourseIds;
    volatile int holeCourseCount;
    volatile int holeCourseCapacity;
    int selectionMode;
    void* layout;
    void* scriptObject;
    int bpEntryId;
    void* courseTextures;
    void* lockedCourseTexture;
    int coursePage;
    int selectionDelayFrames;
    unsigned char selectionComplete;
    char padding55[0x03];
    void* messageFile;
    void* fixedFont;
    void* messageTable;
    void* messageRenderer;
    void* textTable;
    void* holeNoSelect;
    unsigned char holeSelectorActive;
    char padding71[0x03];
    void* miniGameHole;
    void* texturePack;
    unsigned char weatherOptionEnabled;
    unsigned char exitRequested;
};

struct CMiniGameHoleView {
    char loadedTable[0x04];
    void* vtable;
    void* sharedFileHandle;
};

struct Cleanup_8031 {
    virtual void cleanup(int) = 0;
};

struct Cleanup_8031_B {
    int pad;
    virtual void cleanup(int) = 0;
};

extern "C" {
void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u8031B3C0 {
    const unsigned int* words;
    bool active;
    // Arm heap-string cleanup while the enclosing construction can unwind.
    explicit StringUnwindCleanup_u8031B3C0(const void* string) : words((const unsigned int*)string), active(true) {}
    // Transfer cleanup responsibility to the completed string owner.
    void release() {
        active = false;
    }
    // Free only an armed heap-backed string, using its stored data pointer.
    ~StringUnwindCleanup_u8031B3C0() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};

void appFreeUni__FPv(void*);

// The raw vector's explicit normal cleanup releases this unwind guard first.
struct ActivatedUnwindCleanup {
    volatile unsigned int* words;
    bool active;
    // Guard a raw three-word activated-element vector until explicit cleanup.
    explicit ActivatedUnwindCleanup(volatile void* vector) : words((volatile unsigned int*)vector), active(true) {}
    // Disarm the guard before the normal-path free.
    void release() {
        active = false;
    }
    // Clear the count and free the vector allocation if construction unwinds.
    ~ActivatedUnwindCleanup() {
        if (active && words[0]) {
            words[1] = 0;
            appFreeUni__FPv((void*)words[0]);
        }
    }
};

void appFreeUni__FPv(void*);
void* appMallocDummy__FUl(unsigned int);
void* memmove(void*, const void*, unsigned int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
int __dt__Q23std12length_errorFv(void*);
int __throw(void*, void*, void*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char courseSelectStrings[];
}

struct ErrorControl_8031 {
    void* vtable;
    int refs, owners;
    char* text;
    // Adopt allocated exception text with its initial reference counts.
    ErrorControl_8031(char* p) {
        refs = 1;
        owners = 1;
        vtable = s_sharedCharDeleterVtable;
        text = p;
    }
};

struct Error_8031 {
    void* vtable;
    char* text;
    ErrorControl_8031* control;
};

struct ErrorGuard_8031 {
    char* pointer;
    char* allocator;
    // Guard exception text until a shared control block adopts it.
    ErrorGuard_8031(char* p, char* a) : pointer(p), allocator(a) {}
    // Delete only unadopted exception text.
    ~ErrorGuard_8031() {
        if (pointer)
            delete[] pointer;
    }
    // Return the allocation and relinquish this guard's ownership.
    char* release() {
        char* p = pointer;
        pointer = 0;
        return p;
    }
};

// Allocate the narrow exception-message buffer using the original allocator expression.
static inline char* allocateError_8031(unsigned int n) {
    return (char*)__nwa__FUl(n * 2 - n);
}

struct CountVec_8031 {
    char* begin;
    volatile int size;
    unsigned int capacity;
    // Select the larger byte-vector growth request by reference.
    static const unsigned int& maximum(const unsigned int& a, const unsigned int& b) {
        return a < b ? b : a;
    }
    // Copy a borrowed byte range, allowing overlap.
    static void copy(char* first, char* last, char* result) {
        memmove(result, first, last - first);
    }
    // Copy another vector's contents while retaining this vector's allocation ownership.
    CountVec_8031& operator=(const CountVec_8031& other) {
        if (this != &other) {
            assign(other.begin, other.begin + other.size);
        }
        return *this;
    }
    // Replace contents from [first,last), growing owned storage if necessary.
    void assign(char* first, char* last) {
        // Reserve: validate/grow before copying. Reallocation frees the old
        // buffer first, so an aliased source must not require that freed storage.
        unsigned int n = last - first;
        Error_8031 error;
        Error_8031* errorPtr = &error;
        if (n > capacity) {
            unsigned int extra = n - capacity;
            unsigned int oldCapacity = capacity;
            if (extra > (unsigned int)-1 - oldCapacity) {
                error.vtable = __vt__Q23std11logic_error;
                const char* message = courseSelectStrings;
                char allocator = 0;
                char* text;
                error.text = allocateError_8031(strlen(message) + 1);
                {
                    text = error.text;
                    ErrorGuard_8031 guard(text, &allocator);
                    error.control = new ErrorControl_8031(text);
                    guard.release();
                }
                strcpy(error.text, message);
                error.vtable = __vt__Q23std12length_error;
                __throw(courseSelectStrings + kVectorLengthErrorType, errorPtr, (void*)__dt__Q23std12length_errorFv);
            }
            unsigned int newCapacity;
            if (oldCapacity < 0x55555555) {
                newCapacity = oldCapacity + maximum(((oldCapacity + 1) * 3) / 5, extra);
            } else if (oldCapacity < 0xaaaaaaaa) {
                newCapacity = oldCapacity + maximum((oldCapacity + 1) / 2, extra);
            } else {
                newCapacity = (unsigned int)-1;
            }
            size = size - size;
            if (begin) {
                appFreeUni__FPv(begin);
                begin = 0;
                capacity = 0;
            }
            begin = (char*)appMallocDummy__FUl(newCapacity);
            capacity = newCapacity;
        }
        // Commit contents/size after the allocation is ready.
        copy(first, last, begin);
        size = n;
    }
};

// Byte-vector assignment preserves the original growth policy and overflow checks.
struct Vec_8031 {
    char* begin;
    char* end;
    char* capacity;
    // Copy a pointer-triple byte vector, retaining the original overflow/growth policy.
    Vec_8031& operator=(const Vec_8031& other) {
        if (this == &other)
            return *this;
        unsigned int size = other.end - other.begin;
        unsigned int oldCapacity = capacity - begin;
        if (size > oldCapacity) {
            unsigned int extra = size - oldCapacity;
            if (extra > (unsigned int)-1 - oldCapacity) {
                throw std::out_of_range("vector<T> too long");
            }
            unsigned int newCapacity;
            if (oldCapacity < 0x55555555) {
                unsigned int growth = ((oldCapacity + 1) * 3) / 5;
                newCapacity = oldCapacity + ((growth < extra) ? extra : growth);
            } else if (oldCapacity < 0xaaaaaaa) {
                unsigned int growth = (oldCapacity + 1) / 2;
                newCapacity = oldCapacity + ((growth < extra) ? extra : growth);
            } else {
                newCapacity = (unsigned int)-1;
            }
            end = begin;
            if (begin) {
                appFreeUni__FPv(begin);
                begin = 0;
                capacity = 0;
            }
            begin = (char*)appMallocDummy__FUl(newCapacity);
            capacity = begin + newCapacity;
        }
        end = begin + size;
        memmove(begin, other.begin, size);
        return *this;
    }
};

extern "C" {

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Copy the confirmed course, hole order/count, and parallel hole/course lists to the caller.
int CCourseSelect::GetSelectionResult(char* out) {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    // out must contain four setting bytes followed by two initialized owning
    // CountVec_8031 objects at +4/+0x10. Deep-copy lists; caller owns those buffers.
    // Return 0 leaves out untouched until selectionComplete becomes nonzero.
    if (!self->selectionComplete)
        return 0;
    out[0] = self->selectedCourseIndex;
    out[1] = self->holeOrder;
    out[2] = self->weatherMode;
    out[3] = self->holeCount;
    if ((char*)(out + 4) != (char*)&self->holeNumbers) {
        ((CountVec_8031*)(out + 4))
            ->assign(((CountVec_8031*)((char*)&self->holeNumbers))->begin,
                     ((CountVec_8031*)((char*)&self->holeNumbers))->begin +
                         ((CountVec_8031*)((char*)&self->holeNumbers))->size);
    }
    if ((char*)(out + 0x10) != (char*)&self->holeCourseIds) {
        ((CountVec_8031*)(out + 0x10))
            ->assign(((CountVec_8031*)((char*)&self->holeCourseIds))->begin,
                     ((CountVec_8031*)((char*)&self->holeCourseIds))->begin +
                         ((CountVec_8031*)((char*)&self->holeCourseIds))->size);
    }
    return 1;
}
#pragma opt_strength_reduction reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

void Layout_Update(void*);
void __dl__FPv(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void Text_DrawGlyphs(void*);
void UITextTable_Draw(void*);
void* dtor_8031F364(void*, int);
void* __dt__13CMiniGameHoleFv(CMiniGameHoleView*, int);
void CourseSelection_ReleaseAndFree(void*, int);
void* CSharedFilePtr_GetLoadedBuffer(void*);
void* SharedFile_Create(char*, int);
int Layout_PollLoadComplete(void*);
void CBpCtrl_dtor(void*);
extern char __vt__13CMiniGameHole[];
extern char kDartsHoleTableFilename[];
extern char kDriveHoleTableFilename[];
extern char s_bpCtrl[];
extern const float kFloat_1_CourseCardOpacity;
extern signed char s_bpCtrlInitialized;
extern char s_courseSelectBpControllerDtorRegistration[]; // Course-select BP controller destructor-chain registration.
extern char s_inputManagerDtorRegistration_CourseSelect[];
extern void* g_sharedDoc;
void appFreeUni__FPv(void*);
void* __register_global_object(void*, void (*)(void*), void*);
void Layout_Destroy(void*, int);
void __destroy_new_array(void*, void (*)(void*));
extern "C" void __dt__6CBpTexFv(void*);
void UITextTable_ClearEntries(void*);
void PointerVector_InitReserved(void*);
void PointerVector_DestroyStorage(void*);
void PointerArray_DeleteValue(void*, void*);
void PairVector_InitReserved(void*);
void MessageDrawPairList_Dtor(void*);
void PointerPairList_RemoveByKey(void*, void*);
void CBpCtrlHandle_Dtor(void*);
extern signed char s_pointerVectorInitialized;
extern signed char s_pointerPairListInitialized;
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char __vt__13CCourseSelect[];
extern char s_fontNumDrawRegistry[];
extern char s_messageDrawPairList[];

struct Resource_8031BC38 {
    void* field00;
    char padding04[0x04];
    void* sharedFileHandle;
    int field0C;
};

struct Virtual_8031BC38 {
    virtual int f0();
    virtual int f1();
    virtual int f2();
    virtual int f3();
    virtual int ready();
};

struct Base_8031F1A0 {
    void* loadedTable;
    // Initialize the non-owning view of a loaded mini-game table.
    Base_8031F1A0() : loadedTable(0) {}
    // Run non-deleting cleanup of the table base.
    ~Base_8031F1A0() {
        dtor_8031F364(this, -1);
    }
};

struct Asset_8031F1A0 : Base_8031F1A0 {
    void* vtable;
    void* handle;
    volatile int loaded;
    // Initialize an empty owning shared-file handle and its poll flag.
    Asset_8031F1A0() : vtable(__vt__13CMiniGameHole), handle(0), loaded(0) {}
    // Release the shared handle; loadedTable is borrowed from that handle.
    ~Asset_8031F1A0() {
        vtable = __vt__13CMiniGameHole;
        if (handle) {
            CSharedFileHandle::Destroy(handle, 1);
            handle = 0;
        }
    }
};

struct RoundSettingsBytes {
    volatile char bytes[4];
    // Settings bytes have no resources; retain the original cleanup scope.
    ~RoundSettingsBytes() {}
};

struct Frame_8031F1A0 {
    Asset_8031F1A0 asset;
    // args/out1/out2 together form the ApplyRoundSettings input, starting at args:
    // four bytes (course, order, weather, count), then two 3-word vectors
    // (data, count, capacity): hole numbers first, parallel course IDs second.
    // Keep this adjacency: passing &args deliberately exposes the following words.
    // The frame owns both generated vector buffers and its shared-file handle;
    // ApplyRoundSettings consumes their contents before the buffers are freed.
    RoundSettingsBytes args;
    volatile int out1[3];
    volatile int out2[3];
};
// ABI checks protect the intentionally split payload without adding a new
// constructor/destructor scope. Offsets are relative to the complete frame:
// +10 settings bytes, +14 hole vector, +20 parallel course-ID vector; total 2C.
// out1/out2 elements are 32-bit words; the first word stores pointer bits.
typedef char RoundPayload_header_offset[((int)&((Frame_8031F1A0*)0)->args == 0x10) ? 1 : -1];
typedef char RoundPayload_holes_adjacent[((int)&((Frame_8031F1A0*)0)->out1 == 0x14) ? 1 : -1];
typedef char RoundPayload_courses_adjacent[((int)&((Frame_8031F1A0*)0)->out2 == 0x20) ? 1 : -1];
typedef char RoundPayload_frame_size[(sizeof(Frame_8031F1A0) == 0x2C) ? 1 : -1];

// Remember the selected nine-hole party-course table in the shared document.
void SharedDoc_SetPartyCourseTableIndex(void* sharedDocument, unsigned int tableIndex) {
    AsCSharedDoc(sharedDocument)->partyCourseTableIndex = tableIndex;
}

// Enable or disable the weather controls for course selection.
int CCourseSelect::SetWeatherSelectionEnabled(unsigned char value) {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    self->weatherOptionEnabled = value;
    return 1;
}

// Poll the course texture pack and update the course and hole-selector layouts.
void CCourseSelect::UpdateLayoutAndTexture() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    ((CPackTextureResource*)(self->texturePack))->PollLoad();
    Layout_Update(self->layout);
    ((CHoleNoSelect*)(self->holeNoSelect))->UpdateLayout();
}

// Delete the mini-game table base when called as a deleting destructor.
void* dtor_8031F364(void* self, int flag) {
    if (self && flag > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Draw course cards, course information, and the active hole-number selector.
void CCourseSelect::Draw() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    BpObject_TryInvokeCurrentCallback(self->scriptObject, 0);
    BpObject_Draw(self->scriptObject);
    Layout_DrawWithOpacity(self->layout, kFloat_1_CourseCardOpacity);
    ((CCourseSelect*)self)->DrawCourseCards();
    Text_DrawGlyphs(self->messageRenderer);
    UITextTable_Draw(self->textTable);
    if (self->holeSelectorActive) {
        ((CHoleNoSelect*)(self->holeNoSelect))->Draw();
    }
}

// Release a mini-game hole table's shared file handle, then optionally delete its wrapper.
CMiniGameHole::~CMiniGameHole() {
    CMiniGameHoleView* self = (CMiniGameHoleView*)this;
    self->vtable = (void*)__vt__13CMiniGameHole;
    if (self->sharedFileHandle) {
        CSharedFileHandle::Destroy(self->sharedFileHandle, 1);
        self->sharedFileHandle = 0;
    }
}

// Release course-menu layouts, textures, text registrations, selectors, and selected-hole lists.
// Named member rather than a real destructor: the original takes the delete flag and returns the object,
// and a real destructor scores lower.
void* CCourseSelect::Destroy(int flag) {
    CourseSelectStateView& self = *(CourseSelectStateView*)this;
    // Phase 1: destroy layout/BP entry and release card textures.
    char* names = s_inputManagerDtorRegistration_CourseSelect;
    if (this) {
        *(void**)this = __vt__13CCourseSelect;
        if (self.layout) {
            Layout_Destroy(self.layout, 1);
            self.layout = 0;
        }
        if (self.bpEntryId != -1) {
            if (!Sing_805DC5F0()) {
                if (!s_bpCtrlInitialized) {
                    ((CBpCtrl*)s_bpCtrl)->__ct();
                    __register_global_object(s_bpCtrl, CBpCtrl_dtor, (names + 0x0c));
                    s_bpCtrlInitialized = 1;
                }
                Sing_805DC5F0() = s_bpCtrl;
            }
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(self.bpEntryId);
            self.bpEntryId = -1;
        }
        if (self.courseTextures) {
            __destroy_new_array(self.courseTextures, __dt__6CBpTexFv);
            self.courseTextures = 0;
        }
        if (self.lockedCourseTexture) {
            if (self.lockedCourseTexture) {
                ((Cleanup_8031*)self.lockedCourseTexture)->cleanup(1);
            }
            self.lockedCourseTexture = 0;
        }
        // Phase 2: unregister text objects before destroying their owned renderers.
        UITextTable_ClearEntries(self.textTable);
        if (self.textTable) {
            if (!Sing_805DC830()) {
                if (!s_pointerVectorInitialized) {
                    PointerVector_InitReserved(s_fontNumDrawRegistry);
                    __register_global_object(s_fontNumDrawRegistry, PointerVector_DestroyStorage, (names + 0x18));
                    s_pointerVectorInitialized = 1;
                }
                Sing_805DC830() = s_fontNumDrawRegistry;
            }
            PointerArray_DeleteValue(Sing_805DC830(), self.textTable);
        }
        if (!Sing_805DC650()) {
            if (!s_pointerPairListInitialized) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor, (names + 0x30));
                s_pointerPairListInitialized = 1;
            }
            Sing_805DC650() = s_messageDrawPairList;
        }
        PointerPairList_RemoveByKey(Sing_805DC650(), (void*)self.messageRenderer);
        if (self.messageTable) {
            if (self.messageTable) {
                ((Cleanup_8031*)self.messageTable)->cleanup(1);
            }
            self.messageTable = 0;
        }
        if (self.messageRenderer) {
            if (self.messageRenderer) {
                ((Cleanup_8031*)self.messageRenderer)->cleanup(1);
            }
            self.messageRenderer = 0;
        }
        if (self.messageFile) {
            CSharedFileHandle::Destroy(self.messageFile, 1);
            self.messageFile = 0;
        }
        // Phase 3: release selectors/shared assets, then BP handle and list storage.
        if (self.holeNoSelect) {
            if (self.holeNoSelect) {
                ((Cleanup_8031*)self.holeNoSelect)->cleanup(1);
            }
            self.holeNoSelect = 0;
        }
        if (self.miniGameHole) {
            if (self.miniGameHole) {
                ((Cleanup_8031_B*)self.miniGameHole)->cleanup(1);
            }
            self.miniGameHole = 0;
        }
        if (self.texturePack) {
            if (self.texturePack) {
                ((Cleanup_8031*)self.texturePack)->cleanup(1);
            }
            self.texturePack = 0;
        }
        if (!Sing_805DC5F8()) {
            if (!s_bpCtrlHandleInitialized) {
                s_bpCtrlHandle[0] = -1;
                s_bpCtrlHandle[1] = -1;
                __register_global_object(s_bpCtrlHandle, CBpCtrlHandle_Dtor, (names + 0x24));
                s_bpCtrlHandleInitialized = 1;
            }
            Sing_805DC5F8() = s_bpCtrlHandle;
        }
        ((CBpCtrlHandle*)Sing_805DC5F8())->ReleaseEntry();
        if ((char*)&self.selectedCourseIndex) {
            if ((char*)&self.holeCourseIds) {
                if ((char*)&self.holeCourseIds) {
                    if ((char*)&self.holeCourseIds) {
                        if ((char*)&self.holeCourseIds) {
                            if ((char*)&self.holeCourseIds) {
                                if (self.holeCourseIds) {
                                    int c2 = self.holeCourseCount;
                                    c2 -= c2;
                                    self.holeCourseCount = c2;
                                    appFreeUni__FPv(self.holeCourseIds);
                                }
                            }
                        }
                    }
                }
            }
            if ((char*)&self.holeNumbers) {
                if ((char*)&self.holeNumbers) {
                    if ((char*)&self.holeNumbers) {
                        if ((char*)&self.holeNumbers) {
                            if ((char*)&self.holeNumbers) {
                                if (self.holeNumbers) {
                                    int c1 = self.holeNumberCount;
                                    c1 -= c1;
                                    self.holeNumberCount = c1;
                                    appFreeUni__FPv(self.holeNumbers);
                                }
                            }
                        }
                    }
                }
            }
        }
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}

void appFreeUni__FPv(void*);
void* MessageTable_FindTextByName(void*, void*);
void Text_ExpandTaggedText(void*, void*, void*, int);
void Text_SetContent(void*, void*);
void* ItemInventory_GetCourseIdAt(void*, int);
void* ItemInventory_FindCourseData(void*, void*);
void PartyCourseTable_BuildHoleAndCourseLists(void*, void*, int, unsigned char, signed char, int);
void MiniGameHoleTable_BuildHoleAndCourseLists(void*, void*, void*, signed char, int, signed char);
void CourseSelection_ApplyRoundSettings(void*);
void PartyRound_ChooseHoleVariants();
void* UIGroup_GetOrCreateById(void*, void*);
void UIGroup_SetEnabled(void*, int);
void CourseSelection_ReleaseAndFree(void*, int);
extern int kCourseSelectionCourseIdOrder[];
extern int kCourseSelectionPageGroupIds[];
extern void* kPartyCourseNameMessageIds[];
extern void* g_sharedDoc;
extern unsigned char kPartyCoursePreset0Id;
extern unsigned char kPartyCoursePreset1Id;
extern unsigned char kPartyCoursePreset2Id;
extern unsigned char kPartyCoursePreset3Id;
extern unsigned char kPartyCoursePreset4Id;
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_courseSelectFadeOverlayDtorRegistration[]; // Course-select fade-overlay destructor-chain registration.
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*);
int FadeState_GetState(void*);
void* __register_global_object(void*, void (*)(void*), void*);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_SetElementEnabled(void*, int, int);

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
struct TempByteVector {
    char* begin;
    unsigned int size;
    unsigned int capacity;
    // Initialize an empty owning temporary byte vector.
    TempByteVector() {
        begin = 0;
        size = 0;
        capacity = 0;
    }
    // Free the generated hole/course vector when round application finishes.
    ~TempByteVector() {
        if (begin) {
            size -= (begin + size) - begin;
            appFreeUni__FPv(begin);
        }
    }
};
struct PartyRoundPayload {
    unsigned char data[4];
    TempByteVector x;
    TempByteVector y;
    // Generated vectors clean themselves up after these four settings bytes.
    ~PartyRoundPayload() {}
};

struct Local8031E7B8 {
    int a, b, c;
};

#pragma use_lmw_stmw on
#pragma opt_common_subs on
#pragma opt_propagation on
#pragma opt_lifetimes on
// Build a hole/course list for the selected party mode, then apply the round settings.
void CourseSelection_ApplyPartyCourseTable(int tableIndex, int holeOrder, unsigned char holeCount,
                                           signed char specifiedHole) {
    // tableIndex must be 0..4. local.data and its following two vectors form the
    // same contiguous round payload as Frame_8031F1A0; buffers live through apply.
    int holeNumber = specifiedHole;
    unsigned char tableCourseIds[5];
    int shuffleHoles = (holeOrder == 3) ? 1 : 0;
    tableCourseIds[0] = kPartyCoursePreset0Id;
    tableCourseIds[1] = kPartyCoursePreset1Id;
    tableCourseIds[2] = kPartyCoursePreset2Id;
    tableCourseIds[3] = kPartyCoursePreset3Id;
    tableCourseIds[4] = kPartyCoursePreset4Id;
    PartyRoundPayload local;
    local.data[3] = holeCount;
    local.data[2] = 0;
    local.data[1] = holeOrder;
    local.data[0] = tableCourseIds[tableIndex];
    local.data[2] = 0;
    PartyCourseTable_BuildHoleAndCourseLists(&local.x, &local.y, tableIndex, local.data[3], holeNumber, shuffleHoles);
    CourseSelection_ApplyRoundSettings(local.data);
}
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

#pragma push
#pragma use_lmw_stmw on
// Show only course-page groups up to the page containing the current course.
int CCourseSelect::UpdateVisiblePageGroups() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    int* courseIds = kCourseSelectionCourseIdOrder;
    int courseListIndex = 0;
    for (int courseIndex = 0; courseIndex < kCourseCardCount; courseIndex++) {
        if ((int)(signed char)self->selectedCourseIndex == *courseIds) {
            courseListIndex = courseIndex;
            break;
        }
        courseIds++;
    }
    int selectionMode = self->selectionMode;
    int* pageGroupIds;
    int lastVisiblePage = 0;
    if (selectionMode == kPartyCourseTable) {
        int courseId = (signed char)self->selectedCourseIndex;
        switch (courseId) {
        case 0x0e:
            lastVisiblePage = 1;
            break;
        case 0x0f:
        case 0x10:
            lastVisiblePage = 2;
            break;
        case 0x11:
        case 0x12:
            lastVisiblePage = 3;
            break;
        default:
            return 0;
        }
    } else {
        if (courseListIndex >= 0 && courseListIndex < 5)
            lastVisiblePage = 1;
        else if (courseListIndex >= 5 && courseListIndex < 10)
            lastVisiblePage = 2;
        else if (courseListIndex >= 10 && courseListIndex < 14)
            lastVisiblePage = 3;
    }
    pageGroupIds = kCourseSelectionPageGroupIds;
    int pageEnabled = 1;
    for (int pageIndex = 0; pageIndex < 3; pageIndex++) {
        if (lastVisiblePage < pageIndex + 1)
            pageEnabled = 0;
        void* pageGroup = UIGroup_GetOrCreateById(self->scriptObject, (void*)*pageGroupIds);
        UIGroup_SetEnabled(pageGroup, pageEnabled);
        pageGroupIds++;
    }
    return 1;
}
#pragma pop

// Resolve the selected course's message text and replace the displayed description.
int CCourseSelect::UpdateCourseText() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    ((CMsgDraw*)(self->messageRenderer))->ResetTextState();
    char c = (signed char)(*(char*)&AsCCourseSelect(self)->selectedCourseIndex);
    if (0 > c || (unsigned int)c >= 0x13u)
        return 0;

    Local8031E7B8 local;
    local.a = 0;
    local.b = 0;
    local.c = 0;
    StringUnwindCleanup_u8031B3C0 stringCleanup0(&local);

    if (((*(int*)&AsCCourseSelect(self)->selectionMode)) == 3) {

        void* r4 =
            kPartyCourseNameMessageIds[((signed char)(*(char*)&AsCCourseSelect(self)->selectedCourseIndex)) - 0x0e];
        void* r3ret = MessageTable_FindTextByName(AsCCourseSelect(self)->text, r4);
        Text_ExpandTaggedText(AsCCourseSelect(self)->text, &local, r3ret, 0);
    } else {
        void* r31 = AsCSharedDoc(g_sharedDoc)->itemInventory;
        void* r3ret2 =
            ItemInventory_GetCourseIdAt(r31, (signed char)(*(char*)&AsCCourseSelect(self)->selectedCourseIndex));
        void* r4 = r3ret2;
        r3ret2 = ItemInventory_FindCourseData(r31, r4);
        if (!r3ret2) {
            if (((unsigned int)local.a) >> 31) {
                stringCleanup0.release();
                appFreeUni__FPv((void*)local.c);
            }
            return 0;
        }
        void* r3ret3 = MessageTable_FindTextByName(AsCCourseSelect(self)->text, r3ret2);
        Text_ExpandTaggedText(AsCCourseSelect(self)->text, &local, r3ret3, 0);
    }
    __typeof__(self->messageRenderer) savedField64 = self->messageRenderer;
    ((CMsgDraw*)(self->messageRenderer))->ClearFormattingAndRevealAll();
    void* r4;
    if (((unsigned int)local.a) >> 31)
        r4 = (void*)local.c;
    else
        r4 = (void*)((char*)&local + 2);
    Text_SetContent(savedField64, r4);
    if (((unsigned int)local.a) >> 31) {
        stringCleanup0.release();
        appFreeUni__FPv((void*)local.c);
    }
    return 1;
}

// Reset available peers of an activated course, order, hole-count, or weather option.
int CCourseSelect::ResetOtherButtonsInGroup(int elementId) {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    // Only available peers of the activated element are reset;
    // unavailable controls keep their disabled visuals. Always returns 1.
    int i = 0;
    int end = 0;
    if (elementId >= kPageTabFirst && elementId <= kPageTabLast) {
        i = kPageTabFirst;
        end = kPageTabLast;
    } else if (elementId >= kCourseButtonFirst && elementId <= kCourseButtonLast) {
        if (self->coursePage <= kLastOrdinaryCoursePage) {
            i = kCourseButtonFirst;
            end = kOrdinaryPageButtonLast;
        } else {
            i = kLastPageButtonFirst;
            end = kCourseButtonLast;
        }
    } else if (elementId >= kHoleOptionFirst && elementId <= kHoleOptionLast) {
        i = kHoleOptionFirst;
        end = kHoleOptionLast;
    } else if (elementId >= kOrderOptionFirst && elementId <= kOrderOptionLast) {
        i = kOrderOptionFirst;
        end = kOrderOptionLast;
    } else if (elementId >= kWeatherOptionFirst && elementId <= kWeatherOptionLast) {
        i = kWeatherOptionFirst;
        end = kWeatherOptionLast;
    }
    // An invalid element ID leaves start=0,end=0.
    for (; i <= end; i++) {
        if (elementId == i)
            continue;
        if (!IsButtonAvailable(i))
            continue;
        Layout_SetElementValueState(self->layout, i, kLayoutNormal);
        Layout_SetElementPlayerFlag(self->layout, i, 1, kPlayerFlagChannel);
        if (i < kCourseButtonFirst || i > kCourseButtonLast) {
            if (i != kConfirmElement) {
                Layout_SetElementValueState(self->layout, i + kOptionHighlightOffset, kLayoutNormal);
                Layout_SetElementPlayerFlag(self->layout, i + kOptionHighlightOffset, 1, kPlayerFlagChannel);
            }
        }
    }
    return 1;
}

// Switch course pages and restore the selected course's button highlight.
int CCourseSelect::SetCoursePage(int pageIndex) {
    // Phase 1: translate page-tab ID (0..2) to coursePage (1..3) and locate the
    // selected save course ID in the display-order list. Returns 0 for other IDs.
    int selectedCourseListIndex;
    int buttonElement;
    int buttonsVisible;
    int courseElement;
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    if (pageIndex < 0 || pageIndex > 2)
        return 0;
    self->coursePage = pageIndex + 1;
    selectedCourseListIndex = 0;
    for (int i = 0; i < kCourseCardCount; i++) {
        if ((int)self->selectedCourseIndex == kCourseSelectionCourseIdOrder[i]) {
            selectedCourseListIndex = i;
            break;
        }
    }
    // Phase 2: clear available card states, swap the two card-control ranges'
    // visibility and refresh unlock visuals. Selected card restoration comes last.
    courseElement = kCourseButtonFirst;
    do {
        // Check the button against selection mode, course unlocks, page and round-option restrictions.
        if (((CCourseSelect*)self)->IsButtonAvailable(courseElement)) {
            Layout_SetElementValueState(self->layout, courseElement, 0);
            Layout_SetElementPlayerFlag(self->layout, courseElement, 1, kPlayerFlagChannel);
            if (courseElement < 3 || courseElement > 0xb) {
                if (courseElement != kConfirmElement) {
                    Layout_SetElementValueState(self->layout, courseElement + kOptionHighlightOffset, 0);
                    Layout_SetElementPlayerFlag(self->layout, courseElement + kOptionHighlightOffset, 1,
                                                kPlayerFlagChannel);
                }
            }
        }
        courseElement++;
    } while (courseElement <= 0xb);
    buttonsVisible = 1;
    if (self->coursePage == 3)
        buttonsVisible = 0;
    buttonElement = 3;
    do {
        if (buttonElement == 8)
            buttonsVisible ^= 1;
        Layout_SetElementPlayerFlagsAndState(self->layout, buttonElement, buttonsVisible);
        buttonElement++;
    } while (buttonElement <= 0xb);
    ((CCourseSelect*)self)->UpdateCourseUnlockStates();
    // Phase 3: highlight the selected card only when it belongs to this page.
    switch (self->coursePage) {
    case 1: {
        if (selectedCourseListIndex < 0 || selectedCourseListIndex >= 5)
            break;
        Layout_SetElementValueState(self->layout, selectedCourseListIndex + 3, 2);
        if (selectedCourseListIndex + 3 != 0xc)
            Layout_SetElementPlayerFlag(self->layout, selectedCourseListIndex + 3, 0, kPlayerFlagChannel);
        if ((selectedCourseListIndex + 3 < 3 || selectedCourseListIndex + 3 > 0xb) &&
            selectedCourseListIndex + 3 != kConfirmElement) {
            Layout_SetElementValueState(self->layout, selectedCourseListIndex + 0x19, 2);
            Layout_SetElementPlayerFlag(self->layout, selectedCourseListIndex + 0x19, 0, kPlayerFlagChannel);
        }
        break;
    }
    case 2: {
        if (selectedCourseListIndex < 5 || selectedCourseListIndex >= 0xa)
            break;
        int courseButton = 0;
        int pageSlot = selectedCourseListIndex % 5;
        courseButton = pageSlot + 3;
        Layout_SetElementValueState(self->layout, courseButton, 2);
        if (courseButton != 0xc)
            Layout_SetElementPlayerFlag(self->layout, courseButton, 0, kPlayerFlagChannel);
        if ((courseButton < 3 || courseButton > 0xb) && courseButton != kConfirmElement) {
            int courseHighlight = pageSlot + 0x19;
            Layout_SetElementValueState(self->layout, courseHighlight, 2);
            Layout_SetElementPlayerFlag(self->layout, courseHighlight, 0, kPlayerFlagChannel);
        }
        break;
    }
    case 3: {
        if (selectedCourseListIndex < 0xa || (unsigned int)selectedCourseListIndex >= 0x13u)
            break;
        int courseButton = 0;
        int pageSlot = selectedCourseListIndex % 5;
        courseButton = pageSlot + 8;
        Layout_SetElementValueState(self->layout, courseButton, 2);
        if (courseButton != 0xc)
            Layout_SetElementPlayerFlag(self->layout, courseButton, 0, kPlayerFlagChannel);
        if ((courseButton < 3 || courseButton > 0xb) && courseButton != kConfirmElement) {
            int courseHighlight = pageSlot + 0x1e;
            Layout_SetElementValueState(self->layout, courseHighlight, 2);
            Layout_SetElementPlayerFlag(self->layout, courseHighlight, 0, kPlayerFlagChannel);
        }
        break;
    }
    }
    return 1;
}

// Wait for fades and selection delays, then process the course menu or hole-number selector.
int CCourseSelect::UpdateSelection() {
    int menuComplete = 0;
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            char* p = s_fadeOverlay;
            FadeOverlay_Init(p);
            __register_global_object(p, ResourceState_Dtor, s_courseSelectFadeOverlayDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    if (FadeState_GetState(Sing_805DC608()))
        return 0;
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    if (self->exitRequested)
        return 1;
    if (self->holeSelectorActive) {
        ((CCourseSelect*)self)->UpdateHoleSelector();
        return 0;
    }
    Layout_Update(self->layout);
    int delayState = self->selectionDelayFrames;
    if (delayState == -1) {
        delayState = 0;
    } else if (delayState > 0) {
        self->selectionDelayFrames = delayState - 1;
        delayState = 1;
    } else {
        if (((CCourseSelect*)self)->IsButtonAvailable(kConfirmElement)) {
            Layout_SetElementValueState(self->layout, kConfirmElement, 0);
            Layout_SetElementPlayerFlag(self->layout, kConfirmElement, 1, kPlayerFlagChannel);
        }
        self->selectionDelayFrames = -1;
        delayState = 1;
    }
    if (delayState == 0) {
        if (self->selectionComplete) {
            menuComplete = 1;
        } else {
            if (((CCourseSelect*)self)->HandleActivatedElements())
                menuComplete = 1;
        }
    }
    return menuComplete;
}

// Load a mini-game hole table, build its hole/course lists, and apply round settings.
void CourseSelection_ApplyMiniGameCourse(int courseIndex, int holeCount, signed char specifiedHole) {
    // Preconditions: sharedDoc partyGameMode is darts (1) or drive (2); otherwise
    // no table is selected here. specifiedHole is forwarded unchanged to the builder.
    // Phase 1: request the table and synchronously poll its loaded buffer.
    // This is a busy wait: no frame pump, sleep, timeout or failure exit exists;
    // the shared-file loader must make progress independently. Preserve both queries.
    Frame_8031F1A0 frame;
    if ((*(int*)&((CSharedDoc*)g_sharedDoc)->partyGameMode) == 1)
        frame.asset.handle = SharedFile_Create(kDartsHoleTableFilename, 0);
    if ((*(int*)&((CSharedDoc*)g_sharedDoc)->partyGameMode) == 2)
        frame.asset.handle = SharedFile_Create(kDriveHoleTableFilename, 0);
    {
        int done;
        Asset_8031F1A0* local = &frame.asset;

        do {
            if (!(!(local->loaded))) {
                done = 1;
            } else {
                int load = 1;
                if (local->handle) {
                    if (!CSharedFilePtr_GetLoadedBuffer(local->handle)) {
                        done = 0;
                        continue;
                    }
                    local->loadedTable = CSharedFilePtr_GetLoadedBuffer(local->handle);
                }
                local->loaded = load;
                done = 1;
            }
        } while (!done);
    }
    // Phase 2: build paired hole/course arrays, then apply the contiguous round payload.
    frame.out1[kVectorDataWord] = 0;
    frame.out1[kVectorCountWord] = 0;
    frame.out1[kVectorCapacityWord] = 0;
    frame.out2[kVectorDataWord] = 0;
    frame.out2[kVectorCountWord] = 0;
    frame.out2[kVectorCapacityWord] = 0;
    frame.args.bytes[kRoundCourseByte] = courseIndex;
    frame.args.bytes[kRoundOrderByte] = kFrontOrder;
    frame.args.bytes[kRoundWeatherByte] = kClearWeather;
    frame.args.bytes[kRoundHoleCountByte] = holeCount;
    unsigned char narrowedCourseIndex = courseIndex;
    MiniGameHoleTable_BuildHoleAndCourseLists(&frame.asset, (void*)frame.out1, (void*)frame.out2,
                                              (signed char)narrowedCourseIndex, holeCount, specifiedHole);
    CourseSelection_ApplyRoundSettings((void*)&frame.args);
    PartyRound_ChooseHoleVariants();
    // Phase 3: release generated arrays; the asset's destructor releases its handle.
    void* ptr2 = *(void**)&frame.out2[kVectorDataWord];
    if (ptr2) {
        int courseCount = frame.out2[kVectorCountWord];
        frame.out2[kVectorCountWord] = courseCount - courseCount;
        appFreeUni__FPv(ptr2);
    }
    void* ptr1 = *(void**)&frame.out1[kVectorDataWord];
    if (ptr1) {
        int holeCountToRelease = frame.out1[kVectorCountWord];
        frame.out1[kVectorCountWord] = holeCountToRelease - holeCountToRelease;
        appFreeUni__FPv(ptr1);
    }
}

struct Activated_8031DA14 {
    int* items;
    volatile int count;
    int capacity;
};

struct ActivatedElementList {
    int* items;
    volatile unsigned int count;
    int capacity;
    // Release the activated-element list owned by this temporary collection.
    ~ActivatedElementList() {
        if (items) {
            unsigned int n = count;
            count = n - n;
            appFreeUni__FPv(items);
        }
    }
};

struct Virtual_8031BE78 {
    virtual void destroy(int) = 0;
};

void InputManager_Init(void*);
void InputManager_Dtor(void*);
int Input_IsActionPressed(void*, int, int);
int Layout_CollectActivated(void*, Activated_8031DA14*, int);
void Layout_ReadElementRect(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
void BpObject_StartScriptByName(void*, void*, int);
// Set an option's visual state and disable its input flag on the menu's player channel.
static inline void SetCourseOptionState(CourseSelectStateView* self, int element, int state) {
    Layout_SetElementValueState(self->layout, element, state);
    Layout_SetElementPlayerFlag(self->layout, element, 0, kPlayerFlagChannel);
}
// Initialize available course and round-option buttons for the current selection mode.
int CCourseSelect::InitializeButtonStates() {
    // Phase 1: reset card rectangles and choose the ordinary/final card group.
    int j;
    int visible;
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    int i = kCardRectFirst;
    do {
        Layout_SetElementPlayerFlagsAndState(self->layout, i, 0);
        ++i;
    } while (i <= kCardRectLast);
    visible = 1;
    if (self->coursePage == kFinalCoursePage)
        visible = 0;
    j = kCourseButtonFirst;
    do {
        if (j == kLastPageButtonFirst)
            visible ^= 1;
        Layout_SetElementPlayerFlagsAndState(self->layout, j, visible);
        ++j;
    } while (j <= kCourseButtonLast);
    ((CCourseSelect*)self)->UpdateCourseUnlockStates();
    Layout_SetElementEnabled(self->layout, kDescriptionAnchorElement, 0);
    Layout_SetElementEnabled(self->layout, kUnresolvedElement2E, 0);
    Layout_SetElementEnabled(self->layout, kSingleHoleOption + kOptionHighlightOffset, 0);

    // Phase 2: apply mode restrictions to controls AND their paired highlights.
    // Party locks page tabs; darts and drive have distinct preset restrictions.
    int type = self->selectionMode;
    if (type == kPartyCourseTable) {
        for (int i = kPageTabFirst; i <= kPageTabLast; ++i) {
            Layout_SetElementPlayerFlagsAndState(self->layout, i, 0);
            Layout_SetElementPlayerFlagsAndState(self->layout, i + kOptionHighlightOffset, 0);
        }
        SetCourseOptionState(self, kHolePreset4, kLayoutUnavailable);
        SetCourseOptionState(self, kHolePreset4 + kOptionHighlightOffset, kLayoutUnavailable);
        SetCourseOptionState(self, kOrderPreset1, kLayoutUnavailable);
        SetCourseOptionState(self, kOrderPreset1 + kOptionHighlightOffset, kLayoutUnavailable);
        SetCourseOptionState(self, kCourseButtonFirst, kLayoutSelected);
        SetCourseOptionState(self, kHolePreset3, kLayoutSelected);
        SetCourseOptionState(self, kHolePreset3 + kOptionHighlightOffset, kLayoutSelected);
        SetCourseOptionState(self, kOrderPreset0, kLayoutSelected);
        SetCourseOptionState(self, kOrderPreset0 + kOptionHighlightOffset, kLayoutSelected);
    } else {
        if (type == kDartsCourseMode) {
            SetCourseOptionState(self, kSingleHoleOption, kLayoutUnavailable);
            SetCourseOptionState(self, kSingleHoleOption + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset4, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset4 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset0, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset0 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset1, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset1 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset2, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset2 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset3, kLayoutSelected);
            SetCourseOptionState(self, kHolePreset3 + kOptionHighlightOffset, kLayoutSelected);
        } else if (type == kDriveCourseMode) {
            SetCourseOptionState(self, kSingleHoleOption, kLayoutUnavailable);
            SetCourseOptionState(self, kSingleHoleOption + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset1, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset1 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset2, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset2 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset3, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset3 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset4, kLayoutUnavailable);
            SetCourseOptionState(self, kHolePreset4 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset0, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset0 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset1, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset1 + kOptionHighlightOffset, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset2, kLayoutUnavailable);
            SetCourseOptionState(self, kOrderPreset2 + kOptionHighlightOffset, kLayoutUnavailable);
        } else {
            SetCourseOptionState(self, kHolePreset3, kLayoutSelected);
            SetCourseOptionState(self, kHolePreset3 + kOptionHighlightOffset, kLayoutSelected);
            SetCourseOptionState(self, kOrderPreset0, kLayoutSelected);
            SetCourseOptionState(self, kOrderPreset0 + kOptionHighlightOffset, kLayoutSelected);
        }
        SetCourseOptionState(self, kPageTabFirst, kLayoutSelected);
        SetCourseOptionState(self, kPageTabFirst + kOptionHighlightOffset, kLayoutSelected);
        SetCourseOptionState(self, kCourseButtonFirst, kLayoutSelected);
    }

    // Phase 3: weather is independently gated, after mode defaults are established.
    if (!self->weatherOptionEnabled) {
        Layout_SetElementValueState(self->layout, kClearWeatherOption, kLayoutUnavailable);
        Layout_SetElementPlayerFlag(self->layout, kClearWeatherOption, 0, kPlayerFlagChannel);
        Layout_SetElementValueState(self->layout, kClearWeatherOption + kOptionHighlightOffset, kLayoutUnavailable);
        Layout_SetElementPlayerFlag(self->layout, kClearWeatherOption + kOptionHighlightOffset, 0, kPlayerFlagChannel);
        Layout_SetElementValueState(self->layout, kRainWeatherOption, kLayoutUnavailable);
        Layout_SetElementPlayerFlag(self->layout, kRainWeatherOption, 0, kPlayerFlagChannel);
        Layout_SetElementValueState(self->layout, kRainWeatherOption + kOptionHighlightOffset, kLayoutUnavailable);
        Layout_SetElementPlayerFlag(self->layout, kRainWeatherOption + kOptionHighlightOffset, 0, kPlayerFlagChannel);
    } else {
        Layout_SetElementValueState(self->layout, kClearWeatherOption, kLayoutSelected);
        Layout_SetElementPlayerFlag(self->layout, kClearWeatherOption, 0, kPlayerFlagChannel);
        Layout_SetElementValueState(self->layout, kClearWeatherOption + kOptionHighlightOffset, kLayoutSelected);
        Layout_SetElementPlayerFlag(self->layout, kClearWeatherOption + kOptionHighlightOffset, 0, kPlayerFlagChannel);
    }
    return 1;
}
void WFixedFont_CreateMsgEcbResource(void*, void*, void*, int);
void WFixedFont_CreateAndRegisterMsgDraw(void*, void*, int);
void Layout_GetElementPosition(void*, int, void*);
void Text_SetPosition(void*, void*);
void IFadeMenuBase_StartMenuFadeOut(void*);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_CourseSelect[];
extern char courseSelectStrings[];
extern int kCourseSelectionControlGroupIds[];
extern char kCourseSelectionNumericTextEntries[];
extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_courseSelectBpHandleDtorRegistration[]; // Course-select BP handle destructor-chain registration.
void CBpCtrlHandle_Dtor(void*);
extern signed char s_saveDataInitialized;
extern char s_saveData[];
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
void* __nwa__FUl(unsigned int);
int __throw(void*, void*, void*);
int __dt__Q23std12length_errorFv(void*);
void* memset(void*, int, unsigned int);
void* memcpy(void*, const void*, unsigned int);
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12out_of_range[];
extern char courseSelectStrings[];
extern char s_saveData[];
void Layout_GetElementPlayerFlags(void*, int, void*, int);
int GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, int, void*, int);
int __throw(void*, void*, void*);
void __dt__Q23std12out_of_rangeFv(void*);
void __dla__FPv(void*, int);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementPlayerFlag(void*, int, int, int);
void NumericTextTable_ClearEntryText(void*, int);
void UITextTable_SetInteger(void*, int, int);
extern int kCourseSelectionHoleCounts[];

struct State_8031CA64 {
    virtual int f0();
    virtual int f1();
    virtual int f2();
    virtual int bind();
    virtual int isReady();
    virtual int f5();
    virtual int f6();
};
struct Entry_8031CA64 {
    State_8031CA64* state;
    char padding04[0x24];
};

// Construct and throw the vector length exception without changing its unwind cleanup.
static inline void throwLength_8031() {
    struct Error {
        void* vtable;
        char* text;
        ErrorControl_8031* control;
    } error;
    Error* e = &error;
    error.vtable = __vt__Q23std11logic_error;
    const char* message = courseSelectStrings;
    char allocator[4];
    allocator[0] = 0;
    e->text = allocateError_8031(strlen(message) + 1);
    {
        char* text = e->text;
        ErrorGuard_8031 guard(text, allocator);
        e->control = new ErrorControl_8031(text);
        guard.release();
    }
    strcpy(e->text, message);
    e->vtable = __vt__Q23std12length_error;
    __throw(courseSelectStrings + kVectorLengthErrorType, e, (void*)__dt__Q23std12length_errorFv);
}

struct PushBuf_8031 {
    char* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* allocator;
    unsigned int pos;
    // Guard a new byte-vector allocation until it swaps with the old one.
    PushBuf_8031(unsigned int* a) : data(0), size(0), capacity(0), allocator(a), pos(0) {}
    // Free uncommitted storage, or the old allocation received during commit.
    ~PushBuf_8031() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};

struct PushStr_8031 {
    char* begin;
    unsigned int size;
    unsigned int capacity;
    // Select the larger bounded byte-vector growth request by reference.
    static const unsigned int& maximum(const unsigned int& a, const unsigned int& b) {
        return a < b ? b : a;
    }
    // Clear logical contents while retaining the owned allocation.
    void clear() {
        size -= (begin + size) - begin;
    }
    // Append one signed hole/course byte with its original reference ABI.
    void push_back(const signed char& c) {
        push_value<const signed char&>(c);
    }
    // Append in place or reserve/copy/swap, retaining temporary cleanup on unwind.
    template <class T> void push_value(T c) {
        if (size < capacity) {
            size = size + 1;
            begin[size - 1] = c;
            return;
        }
        if (1 > (unsigned int)-1 - capacity)
            throwLength_8031();
        PushBuf_8031 buf(&capacity);
        unsigned int extra = size + 1 - capacity;
        unsigned int oldCapacity = capacity;
        if (extra > (unsigned int)-1 - oldCapacity)
            throwLength_8031();
        unsigned int newCapacity;
        unsigned int half;
        unsigned int growth;
        if (oldCapacity < 0x55555555) {
            growth = ((oldCapacity + 1) * 3) / 5;
            newCapacity = oldCapacity + maximum(growth, extra);
        } else if (oldCapacity < 0xaaaaaaaa) {
            half = (oldCapacity + 1) / 2;
            newCapacity = oldCapacity + maximum(half, extra);
        } else {
            newCapacity = (unsigned int)-1;
        }
        buf.data = (char*)appMallocDummy__FUl(newCapacity);
        buf.capacity = newCapacity;
        buf.pos = size;
        memset(buf.data + buf.pos + buf.size, (unsigned char)c, 1);
        char* first = begin;
        char* last = begin + size;
        buf.size += 1;
        unsigned int n = last - first;
        buf.pos -= n;
        memcpy(buf.data + buf.pos, first, n);
        memset(first, 0, n);
        buf.size += n;
        size = 0;
        char* d = buf.data;
        buf.data = begin;
        unsigned int cap = capacity;
        capacity = buf.capacity;
        buf.capacity = cap;
        begin = d;
        size = buf.size;
        buf.size = 0;
    }
};

#pragma push
#pragma use_lmw_stmw on
// Commit a one-hole choice or restore its previous option group after cancellation.
int CCourseSelect::UpdateHoleSelector() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    int index = -1;
    if (((CHoleNoSelect*)(self->holeNoSelect))->GetSelectionResult(&index)) {
        ((CHoleNoSelect*)(self->holeNoSelect))->SetEnabled(0);
        self->holeSelectorActive = 0;
        if (index != -1) {
            PushStr_8031* first = (PushStr_8031*)((char*)&self->holeNumbers);
            first->clear();
            first->push_value(index);
            PushStr_8031* second = (PushStr_8031*)((char*)&self->holeCourseIds);
            second->clear();
            second->push_value<const signed char&>(self->selectedCourseIndex);
            self->holeCount = 1;
            UITextTable_SetInteger(self->textTable, kSelectedHoleText, (signed char)*(char*)self->holeNumbers);
        } else {
            int category = 0;
            int value = self->holeCount;
            int* ids = kCourseSelectionHoleCounts;
            do {
                category = 0;
                if (value == *ids++)
                    break;
                category = 1;
                if (value == *ids++)
                    break;
                category = 2;
                if (value == *ids++)
                    break;
                category = 3;
                if (value == *ids++)
                    break;
                category = 4;
                if (value == *ids++)
                    break;
                category = 5;
            } while (0);
            Layout_SetElementValueState(self->layout, category + 0xc, 2);
            if (category)
                Layout_SetElementPlayerFlag(self->layout, category + 0xc, 0, kPlayerFlagChannel);
            if ((category + 0xc < 3 || category + 0xc > 0xb) && category + 0xc != kConfirmElement) {
                Layout_SetElementValueState(self->layout, category + 0x22, 2);
                Layout_SetElementPlayerFlag(self->layout, category + 0x22, 0, kPlayerFlagChannel);
            }
            ((CCourseSelect*)self)->ResetOtherButtonsInGroup(category + 0xc);
        }
        ((CCourseSelect*)self)->RefreshOrderOptionButtons();
    }
    ((CHoleNoSelect*)(self->holeNoSelect))->Update();
    return 1;
}
#pragma pop
#undef APPEND_8031

extern char bitsetTestRangeErrorText[];

// Throw an out-of-range exception for an invalid course-unlock bit index.
static inline void throwBitset_8031() {
    struct Error {
        void* vtable;
        char* text;
        ErrorControl_8031* control;
    } error;
    error.vtable = __vt__Q23std11logic_error;
    char allocator = 0;
    const char* message = bitsetTestRangeErrorText;
    error.text = allocateError_8031(strlen(message) + 1);
    {
        char* text = error.text;
        ErrorGuard_8031 guard(text, &allocator);
        error.control = new ErrorControl_8031(text);
        guard.release();
    }
    strcpy(error.text, message);
    error.vtable = __vt__Q23std12out_of_range;
    __throw(courseSelectStrings + kBitsetRangeErrorType, &error, (void*)__dt__Q23std12out_of_rangeFv);
}

// Four vertices, six words each: TL,TR,BL,BR. x/y are screen-space floats;
// z/color/UV interpretation of the remaining words is not established here.
// Keep the word-copy view: the template contains raw bits, not 24 float values.
enum CourseCardVertexWord {
    kTopLeftX = 0,
    kTopLeftY = 1,
    kTopRightX = 6,
    kTopRightY = 7,
    kBottomLeftX = 12,
    kBottomLeftY = 13,
    kBottomRightX = 18,
    kBottomRightY = 19,
    kCardQuadWordCount = 24
};
struct CourseCardQuadWords {
    unsigned int w[kCardQuadWordCount];
};

// Throw the course-unlock bitset exception with the original addressable temporary.
static inline void throwBitsetB_8031() {
    struct Error {
        void* vtable;
        char* text;
        ErrorControl_8031* control;
    } error;
    Error* pe = &error;
    error.vtable = __vt__Q23std11logic_error;
    char allocator = 0;
    error.text = allocateError_8031(strlen(bitsetTestRangeErrorText) + 1);
    {
        char* text = error.text;
        ErrorGuard_8031 guard(text, &allocator);
        error.control = new ErrorControl_8031(text);
        guard.release();
    }
    strcpy(error.text, bitsetTestRangeErrorText);
    error.vtable = __vt__Q23std12out_of_range;
    __throw(courseSelectStrings + kBitsetRangeErrorType, pe, (void*)__dt__Q23std12out_of_rangeFv);
}

// Draw unlocked course cards or the lock texture, with selection-dependent rectangles.
int CCourseSelect::DrawCourseCards() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    // Per-card phases: resolve saved unlock -> bind ready texture -> read feedback
    // rectangle -> emit a screen-space strip. Resource pointers are borrowed.
    int firstTextureIndex =
        self->selectionMode == kPartyCourseTable ? kUnlockCourseCount : (self->coursePage - 1) * kCoursesPerPage;
    int coursePage = self->coursePage;
    // Signed bit idiom: five cards on pages 1/2, four on page 3; keep its shape.
    int count = 5 + (~((coursePage - 3) | (3 - coursePage)) >> 31);
    int* displayCourseId = kCourseSelectionCourseIdOrder + firstTextureIndex;
    char* templateStorage = (char*)((char*)&bgmCollectionStrings + 260);
    CourseCardQuadWords* vertexTemplate = (CourseCardQuadWords*)(((char*)(templateStorage + 0x568)));
    for (int i = 0; i < count; displayCourseId++, ++i) {
        // Phase A: choose the displayed resource. Texture index is a position in
        // the menu list; *displayCourseId is the save-bit index. They can differ.
        // Party cards bypass saved bits, but still execute singleton setup.
        if (!GetSaveDataSlot()) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            GetSaveDataSlot() = s_saveData;
        }
        void* bits = SaveData_GetCurrentRecord(GetSaveDataSlot());
        int courseUnlocked;
        if (self->selectionMode == kPartyCourseTable) {
            courseUnlocked = 1;
        } else if (firstTextureIndex + i >= kUnlockCourseCount) {
            courseUnlocked = 0;
        } else {
            unsigned int bit = (unsigned int)*displayCourseId;
            if (bit >= kUnlockCourseCount)
                throwBitset_8031();
            courseUnlocked = (AsSavePlayerRecord(bits)->unlockedCourses & (1u << bit)) != 0;
        }
        // Phase B: bind the unlocked or lock texture. A missing/not-ready texture
        // skips this card; readiness here says nothing about input availability.
        int textureReady;
        if (!courseUnlocked) {
            State_8031CA64* state = (State_8031CA64*)self->lockedCourseTexture;
            if (state && state->isReady()) {
                ((State_8031CA64*)self->lockedCourseTexture)->bind();
                textureReady = 1;
            } else
                textureReady = 0;
        } else {
            if (!self->courseTextures)
                textureReady = 0;
            else {
                int off = (firstTextureIndex + i) * kCourseTextureBytes;
                if (!((State_8031CA64*)((char*)self->courseTextures + off))->isReady())
                    textureReady = 0;
                else {
                    ((State_8031CA64*)((char*)self->courseTextures + off))->bind();
                    textureReady = 1;
                }
            }
        }
        if (!textureReady)
            continue;
        // Phase C: resolve the card's geometry anchor and its input feedback.
        // Rectangles are screen pixels (left, top, right, bottom). An available
        // card with no player feedback flag is translated by (+1,+1), not scaled.
        int screenRect[4];
        int state = 0;
        if (self->coursePage != kFinalCoursePage) {
            Layout_ReadElementRect(self->layout, i + kCardRectFirst, screenRect);
            Layout_GetElementPlayerFlags(self->layout, i + kCourseButtonFirst, &state, kPlayerFlagChannel);
        } else {
            Layout_ReadElementRect(self->layout, i + kFinalPageCardRectFirst, screenRect);
            Layout_GetElementPlayerFlags(self->layout, i + kLastPageButtonFirst, &state, kPlayerFlagChannel);
        }
        int pageSnapshot = self->coursePage;
        int offset = kCourseButtonFirst;
        if (pageSnapshot == kFinalCoursePage)
            offset = kLastPageButtonFirst;
        int courseElement = i + offset;
        if (!state) {
            int valid;
            // This second lookup mirrors button availability, including page/ID
            // restrictions. Keep it separate from Phase A's resource decision.
            do {
                if (courseElement < kCourseButtonFirst || courseElement > kCourseButtonLast)
                    valid = 0;
                else {
                    int base = (pageSnapshot - 1) * kCoursesPerPage;
                    if (self->selectionMode == kPartyCourseTable)
                        valid = 1;
                    else {
                        int idx;
                        if (pageSnapshot == kFinalCoursePage) {
                            if (courseElement < kLastPageButtonFirst) {
                                valid = 0;
                                break;
                            }
                            idx = courseElement - kLastPageButtonFirst;
                        } else {
                            if (courseElement > kOrdinaryPageButtonLast) {
                                valid = 0;
                                break;
                            }
                            idx = courseElement - kCourseButtonFirst;
                        }
                        if (!GetSaveDataSlot()) {
                            if (!s_saveDataInitialized) {
                                SaveData_Ctor(s_saveData);
                                s_saveDataInitialized = 1;
                            }
                            GetSaveDataSlot() = s_saveData;
                        }
                        void* data = SaveData_GetCurrentRecord(GetSaveDataSlot());
                        if (self->selectionMode == kPartyCourseTable)
                            valid = 1;
                        else {
                            int k = base + idx;
                            if (k >= kUnlockCourseCount)
                                valid = 0;
                            else {
                                unsigned int bitValue = (unsigned int)kCourseSelectionCourseIdOrder[k];
                                if (bitValue >= kUnlockCourseCount)
                                    throwBitset_8031();
                                valid = (AsSavePlayerRecord(data)->unlockedCourses & (1u << bitValue)) != 0;
                            }
                        }
                    }
                }
            } while (0);

            if (valid) {
                ++screenRect[0];
                ++screenRect[2];
                ++screenRect[1];
                ++screenRect[3];
            }
        }
        // Phase D: copy the template and replace positions, then submit once.
        // Four 24-byte vertices in TL,TR,BL,BR strip order. Each vertex is six
        // raw words: only x/y are established here; the other four words come
        // unchanged from the template. Do not infer their types from float[].
        float vertices[kCardQuadWordCount];
        *(CourseCardQuadWords*)vertices = *vertexTemplate;
        vertices[kTopLeftX] = (float)screenRect[0];
        vertices[kTopLeftY] = (float)screenRect[1];
        vertices[kTopRightX] = (float)screenRect[2];
        vertices[kTopRightY] = (float)screenRect[1];
        vertices[kBottomLeftX] = (float)screenRect[0];
        vertices[kBottomLeftY] = (float)screenRect[3];
        vertices[kBottomRightX] = (float)screenRect[2];
        vertices[kBottomRightY] = (float)screenRect[3];
        int primitiveCount = GX_GetPrimitiveCount(4, kCardStripPrimitive);
        DrawPrimitiveUP(kCardStripPrimitive, primitiveCount, vertices, kCardVertexBytes);
    }
    return 1;
}

// Map a course-card element to a course index and refresh its description.
int CCourseSelect::SelectCourseByElementId(int elementId) {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    if (elementId < kCourseButtonFirst || elementId > kCourseButtonLast)
        return 0;
    int index;
    if ((*(int*)&((CCourseSelect*)self)->selectionMode) == 3) {
        index = elementId + 0xb;
    } else {
        int pageIndex = self->coursePage;
        if (pageIndex == 3) {
            pageIndex -= 1;
            index = elementId + pageIndex * kCoursesPerPage - 8;
        } else {
            pageIndex -= 1;
            index = elementId + pageIndex * kCoursesPerPage - 3;
        }
    }
    if ((unsigned int)(elementId - 3) >= 0x13)
        return 0;
    self->selectedCourseIndex = (signed char)kCourseSelectionCourseIdOrder[index];
    PushStr_8031* str = (PushStr_8031*)((char*)&self->holeCourseIds);
    str->clear();
    if (self->holeCount == 1) {
        str->clear();
        str->push_back(self->selectedCourseIndex);
    }
    ((CCourseSelect*)self)->UpdateCourseText();
    ((CCourseSelect*)self)->UpdateVisiblePageGroups();
    return 1;
}
void NumericTextTable_ClearEntryText(void*, int);
extern int kCourseSelectionHoleCounts[];
extern const int kCourseSelectionHoleOrders[];

// Handle element 44 confirmation, starting its feedback delay; return 1 for cancellation.
int CCourseSelect::HandleElement44Activation() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    Activated_8031DA14 activated;
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor,
                                     s_inputManagerDtorRegistration_CourseSelect);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    if (Input_IsActionPressed(GetInputManagerSlot(), 6, -1))
        return 1;

    activated.items = 0;
    activated.count = 0;
    activated.capacity = 0;
    ActivatedUnwindCleanup activatedCleanup(&activated);

    if (Layout_CollectActivated(self->layout, &activated, 0)) {
        if (activated.items[0] == kConfirmElement) {
            int type = (*(int*)&((CCourseSelect*)self)->selectionMode);
            if (3 == type) {
                int selected = 0;

                void* first = (char*)&self->holeNumbers;
                void* second = (char*)&self->holeCourseIds;
                unsigned char holeCount = self->holeCount;
                int idOffset = (self->holeOrder) - 3;
                __typeof__(self->selectedCourseIndex) selectedCourseIndex = self->selectedCourseIndex;

                __typeof__(!idOffset) savedIdOffset = !idOffset;
                char partyTableIndex = selectedCourseIndex;
                selected = (1 == holeCount) ? *(signed char*)self->holeNumbers : selected;
                int shuffleHoles = savedIdOffset;
                ((void (*)(void*, void*, int, unsigned char, signed char,
                           int))PartyCourseTable_BuildHoleAndCourseLists)(
                    first, second, partyTableIndex - 0xe, holeCount, (signed char)selected, shuffleHoles);
            } else if (type == 1 || 2 == type) {
                void* first = (char*)&self->holeNumbers;
                void* resource = self->miniGameHole;

                void* second = (char*)&self->holeCourseIds;
                unsigned char holeCount = self->holeCount;
                MiniGameHoleTable_BuildHoleAndCourseLists(resource, first, second,
                                                          (signed char)(self->selectedCourseIndex), holeCount, 0);
            }
            Layout_SetElementValueState(self->layout, kConfirmElement, 2);
            Layout_SetElementPlayerFlag(self->layout, kConfirmElement, 0, kPlayerFlagChannel);
            self->selectionDelayFrames = 3;
            self->selectionComplete = 1;
            if (activated.items) {
                unsigned int count = activated.count;
                activated.count = count - count;
                activatedCleanup.release();
                appFreeUni__FPv(activated.items);
            }
            return 0;
        }
    }
    if (activated.items) {
        int count = activated.count;
        activated.count = count - count;
        activatedCleanup.release();
        appFreeUni__FPv(activated.items);
    }
    return 0;
}

// Initialize menu scripts, course page, text placement, and round-option highlights.
void CCourseSelect::StartMenuFadeOut() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    if (self->texturePack) {
        if (self->texturePack)
            ((Virtual_8031BE78*)self->texturePack)->destroy(1);
        self->texturePack = 0;
    }
    BpObject_StartScriptByName(self->scriptObject, courseSelectStrings + kCourseMenuScript, 1);
    ((CCourseSelect*)self)->InitializeButtonStates();
    int i = 0;
    int* p = kCourseSelectionControlGroupIds;
    for (; (unsigned int)i < 0x23u; ++i, ++p) {
        void* found = UIGroup_GetOrCreateById(self->scriptObject, (void*)*p);
        if (found)
            UIGroup_SetEnabled(found, 0);
    }
    i = 0;
    char buffer[16];
    for (; i <= 0x15; ++i) {
        if (i < 3 || i > 0xb) {
            Layout_ReadElementRect(self->layout, i, buffer);
            Layout_WriteElementRect(self->layout, i + kOptionHighlightOffset, buffer);
        }
    }
    ((CFontNumDraw*)(self->textTable))
        ->RegisterEntries(kCourseSelectionNumericTextEntries, 1, self->scriptObject,
                          courseSelectStrings + kCourseNumberFont);
    void* handle = self->messageFile;
    void* loaded = handle ? CSharedFilePtr_GetLoadedBuffer(handle) : 0;
    WFixedFont_CreateMsgEcbResource(self->fixedFont, &AsCCourseSelect(self)->text, loaded, 1);
    WFixedFont_CreateAndRegisterMsgDraw(self->fixedFont, &AsCCourseSelect(self)->messageRenderer, 1);
    int value;
    Layout_GetElementPosition(self->layout, kDescriptionAnchorElement, &value);
    Text_SetPosition(self->messageRenderer, &value);
    ((CCourseSelect*)self)->UpdateVisiblePageGroups();
    ((CCourseSelect*)self)->UpdateCourseText();
    ((CHoleNoSelect*)(self->holeNoSelect))->InitLayout((*(int*)&AsCCourseSelect(self)->selectionMode) == 3);
    ((CHoleNoSelect*)(self->holeNoSelect))->SelectHole(1);
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, CBpCtrlHandle_Dtor, s_courseSelectBpHandleDtorRegistration);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = s_bpCtrlHandle;
    }
    ((CBpCtrlHandle*)Sing_805DC5F8())->SetScriptIndex(0xe);
    IFadeMenuBase_StartMenuFadeOut(self);
}

// Wait until the menu layout, scripts, text, and mini-game resources are loaded.
int CCourseSelect::AreResourcesLoaded() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    if (!((CPackTextureResource*)(self->texturePack))->IsLoadFinished())
        return 0;
    unsigned int type = AsCCourseSelect(self)->selectionMode;
    if (type == 1 || type == 2) {
        Resource_8031BC38* resource = (Resource_8031BC38*)self->miniGameHole;
        int ready;
        if (resource->field0C) {
            ready = 1;
        } else {
            // 0x8031BC38: Stop resource initialization while its file buffer is unavailable.
            do {
                if (resource->sharedFileHandle) {
                    if (!CSharedFilePtr_GetLoadedBuffer(resource->sharedFileHandle)) {
                        ready = 0;
                        break;
                    }
                    resource->field00 = (void*)CSharedFilePtr_GetLoadedBuffer(resource->sharedFileHandle);
                }
                resource->field0C = 1;
                ready = 1;
            } while (0);

            ;
        }
        if (!ready)
            return 0;
    }
    int offset;
    unsigned int i;
    i = 0;
    offset = 0;
    for (; i < kCourseCardCount; i++, offset += kCourseTextureBytes) {
        Virtual_8031BC38* entry = (Virtual_8031BC38*)((char*)self->courseTextures + offset);
        if (!entry->ready())
            return 0;
    }
    Virtual_8031BC38* other = (Virtual_8031BC38*)self->lockedCourseTexture;
    if (!other->ready())
        return 0;
    if (!Layout_PollLoadComplete(self->layout))
        return 0;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_courseSelectBpControllerDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    if (!((CBpCtrl*)Sing_805DC5F0())->PollAllEntriesLoaded())
        return 0;
    void* handle = self->messageFile;
    if (!handle || !CSharedFilePtr_GetLoadedBuffer(handle))
        return 0;
    return ((CHoleNoSelect*)(self->holeNoSelect))->IsLoaded();
}

// Apply game-mode, hole-count and saved course-unlock restrictions to a menu button.
int CCourseSelect::IsButtonAvailable(int elementId) {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    int ready;
    int count;
    int index;
    int bit;
    if (self->selectionMode == kPartyCourseTable && (elementId == 0x10 || elementId == 0x12))
        return 0;
    if (self->selectionMode == kDartsCourseMode) {
        for (int i = 0x10; i <= 0x15; ++i)
            if (i == elementId)
                return 0;
        if (elementId == 0xc)
            return 0;
    }
    if (self->selectionMode == kDriveCourseMode) {
        for (int i = 0xc; i <= 0x15; ++i)
            if (i == elementId)
                return 0;
    }
    if (self->holeCount == kFullRoundHoles && elementId == 0x12)
        return 0;
    if (elementId >= kCourseButtonFirst && elementId <= kCourseButtonLast) {
        // 0x8031EBBC: Stop at the first course-unlock failure or unconditional success.
        do {
            if (elementId < kCourseButtonFirst || elementId > kCourseButtonLast) {
                ready = 0;
            } else {
                count = self->coursePage;
                int group = (count - 1) * kCoursesPerPage;
                if (self->selectionMode == kPartyCourseTable) {
                    ready = 1;
                    break;
                }
                int offset;
                if (count == 3) {
                    if (elementId < kLastPageButtonFirst) {
                        ready = 0;
                        break;
                    }
                    offset = elementId - 8;
                } else {
                    if (elementId > 7) {
                        ready = 0;
                        break;
                    }
                    offset = elementId - 3;
                }
                if (!GetSaveDataSlot()) {
                    if (!s_saveDataInitialized) {
                        SaveData_Ctor(s_saveData);
                        s_saveDataInitialized = 1;
                    }
                    GetSaveDataSlot() = s_saveData;
                }
                void* bits = SaveData_GetCurrentRecord(GetSaveDataSlot());
                if (self->selectionMode == kPartyCourseTable) {
                    ready = 1;
                    break;
                }
                index = group + offset;
                if (index >= 0xe) {
                    ready = 0;
                    break;
                }
                bit = kCourseSelectionCourseIdOrder[index];
                if ((unsigned int)bit >= 0xe)
                    throwBitsetB_8031();
                ready = (AsSavePlayerRecord(bits)->unlockedCourses & (1u << bit)) != 0;
            }
        } while (0);

        if (!ready)
            return 0;
    }
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Set the current page's course buttons to unlocked or unavailable states.
int CCourseSelect::UpdateCourseUnlockStates() {
    CourseSelectStateView* self = (CourseSelectStateView*)this;
    int n = self->coursePage;
    int firstCourseIndex = (n - 1) * kCoursesPerPage;
    if ((*(int*)&((CCourseSelect*)self)->selectionMode) == 3)
        return 1;
    int count;
    int first;
    if (n == 3) {
        count = 4;
        first = 8;
    } else {
        count = 5;
        first = 3;
    }
    int* p = kCourseSelectionCourseIdOrder + firstCourseIndex;
    for (int i = 0; i < count; i++) {
        if (!GetSaveDataSlot()) {
            if (!s_saveDataInitialized) {
                SaveData_Ctor(s_saveData);
                s_saveDataInitialized = 1;
            }
            GetSaveDataSlot() = s_saveData;
        }
        void* bits = SaveData_GetCurrentRecord(GetSaveDataSlot());
        int test;
        if ((*(int*)&((CCourseSelect*)self)->selectionMode) == 3) {
            test = 1;
        } else {
            int index = firstCourseIndex + i;
            if (index >= kUnlockCourseCount)
                test = 0;
            else {
                unsigned int bit = (unsigned int)*p;
                if (bit >= kUnlockCourseCount)
                    throwBitsetB_8031();
                test = (AsSavePlayerRecord(bits)->unlockedCourses & (1u << bit)) != 0;
            }
        }
        if (test) {
            Layout_SetElementPlayerFlag(self->layout, first + i, 1, kPlayerFlagChannel);
        } else {
            int item = first + i;
            Layout_SetElementPlayerFlag(self->layout, item, 0, kPlayerFlagChannel);
            Layout_SetElementValueState(self->layout, item, kLayoutNormal);
        }
        p++;
    }
    return 1;
}
#pragma pop

// Apply a hole-count option, opening the one-hole selector when needed.
static inline int CourseSelect_HandleInputGroup0(CourseSelectStateView* self, int mode) {
    int recognized;
    if (mode < kHoleOptionFirst || mode > kHoleOptionLast)
        recognized = 0;
    else if ((unsigned int)(mode - kHoleOptionFirst) >= 5)
        recognized = 0;
    else {
        if (mode == kHoleOptionFirst) {
            self->holeSelectorActive = 1;
            ((CHoleNoSelect*)(self->holeNoSelect))->SetEnabled(1);
        } else {
            int value = (&kCourseSelectionHoleCounts[mode])[-kHoleOptionFirst];
            int a = self->holeNumberCount;
            int b = self->holeCourseCount;
            self->holeNumberCount = a - a;
            void* r68 = self->textTable;
            self->holeCount = (unsigned char)value;
            self->holeCourseCount = b - b;
            NumericTextTable_ClearEntryText(r68, kSelectedHoleText);
        }
        recognized = 1;
    }
    return recognized;
}
// Apply front, back, or shuffled hole order; holeCount determines the selected range.
static inline int CourseSelect_HandleInputGroup1(CourseSelectStateView* self, int mode) {
    int recognized;
    if (mode < kOrderOptionFirst || mode > kOrderOptionLast)
        recognized = 0;
    else if ((unsigned int)(mode - kOrderOptionFirst) >= 3)
        recognized = 0;
    else {
        self->holeOrder = (signed char)(&kCourseSelectionHoleOrders[mode])[-kOrderOptionFirst];
        recognized = 1;
    }
    return recognized;
}
// Apply clear or rainy weather for the round.
static inline int CourseSelect_HandleInputGroup2(CourseSelectStateView* self, int mode) {
    int recognized;
    if (mode < kWeatherOptionFirst || mode > kWeatherOptionLast)
        recognized = 0;
    else {
        int value = kRainyWeather;
        if (mode == kWeatherOptionFirst)
            value = kClearWeather;
        self->weatherMode = value;
        recognized = 1;
    }
    return recognized;
}

// Collect activated controls, dispatch their selection group, and handle cancellation.
int CCourseSelect::HandleActivatedElements() {
    if (HandleElement44Activation()) {
        exitRequested = 1;
        return 0;
    }

    ActivatedElementList activated;
    activated.items = 0;
    activated.count = 0;
    activated.capacity = 0;
    if (!Layout_CollectActivated(layout, (Activated_8031DA14*)&activated, 0)) {
        return 0;
    }

    int index = 0;
    unsigned int count = activated.count;
    for (; (unsigned int)index < count; ++index) {
        int item = activated.items[index];
        if (item >= 0 && item <= 0x15)
            break;
    }
    // 0x8031E374: process the first button group that recognizes this element.
    if (SetCoursePage(activated.items[index]) || SelectCourseByElementId(activated.items[index]) ||
        CourseSelect_HandleInputGroup0((CourseSelectStateView*)this, activated.items[index]) ||
        CourseSelect_HandleInputGroup1((CourseSelectStateView*)this, activated.items[index]) ||
        CourseSelect_HandleInputGroup2((CourseSelectStateView*)this, activated.items[index])) {
        RefreshOrderOptionButtons();
        int mode = activated.items[index];
        Layout_SetElementValueState(layout, mode, kLayoutSelected);
        if (mode != kSingleHoleOption)
            Layout_SetElementPlayerFlag(layout, mode, 0, kPlayerFlagChannel);
        if ((mode < kCourseButtonFirst || mode > kCourseButtonLast) && mode != kConfirmElement) {
            Layout_SetElementValueState(layout, mode + kOptionHighlightOffset, kLayoutSelected);
            Layout_SetElementPlayerFlag(layout, mode + kOptionHighlightOffset, 0, kPlayerFlagChannel);
        }
        ResetOtherButtonsInGroup(mode);

        return 0;
    }
    return 0;
}

// Refresh order-option availability and highlights after a hole-count change.
int CCourseSelect::RefreshOrderOptionButtons() {
    // Phase 1: choose the valid order controls for this hole count. Each state
    // change is mirrored to its paired visual; player flags control interactivity.
    if (holeCount == 1) {
        for (int i = kOrderOptionFirst; i <= kOrderOptionLast; i++) {
            Layout_SetElementValueState(layout, i, kLayoutUnavailable);
            Layout_SetElementPlayerFlag(layout, i, 0, kPlayerFlagChannel);
            if (i < kCourseButtonFirst || i > kCourseButtonLast) {
                Layout_SetElementValueState(layout, i + kOptionHighlightOffset, kLayoutUnavailable);
                Layout_SetElementPlayerFlag(layout, i + kOptionHighlightOffset, 0, kPlayerFlagChannel);
            }
        }
    } else if (holeCount == kFullRoundHoles) {
        if ((unsigned char)holeOrder <= kBackOrder) {
            holeOrder = kFrontOrder;
            Layout_SetElementValueState(layout, kOrderPreset0, kLayoutSelected);
            Layout_SetElementPlayerFlag(layout, kOrderPreset0, 0, kPlayerFlagChannel);
            Layout_SetElementValueState(layout, kFrontOrderHighlight, kLayoutSelected);
            Layout_SetElementPlayerFlag(layout, kFrontOrderHighlight, 0, kPlayerFlagChannel);
            ResetOtherButtonsInGroup(kOrderPreset0);
        } else if (holeOrder == kShuffledHoles) {
            holeOrder = kShuffledHoles;
            Layout_SetElementValueState(layout, kOrderPreset2, kLayoutSelected);
            Layout_SetElementPlayerFlag(layout, kOrderPreset2, 0, kPlayerFlagChannel);
            Layout_SetElementValueState(layout, kShuffleOrderHighlight, kLayoutSelected);
            Layout_SetElementPlayerFlag(layout, kShuffleOrderHighlight, 0, kPlayerFlagChannel);
            ResetOtherButtonsInGroup(kOrderPreset2);
        }
        Layout_SetElementValueState(layout, kOrderPreset1, kLayoutUnavailable);
        Layout_SetElementPlayerFlag(layout, kOrderPreset1, 0, kPlayerFlagChannel);
        Layout_SetElementValueState(layout, kBackOrderHighlight, kLayoutUnavailable);
        Layout_SetElementPlayerFlag(layout, kBackOrderHighlight, 0, kPlayerFlagChannel);
    } else {
        // Phase 2: partial rounds retain their selected order except in the two
        // mini-game modes. Unknown order values retain index=-1 as before.
        unsigned int type = selectionMode;
        unsigned int typeMinusOne = type - 1;
        if (typeMinusOne <= 1)
            return 0;
        int index = -1;
        switch (holeOrder) {
        case kFrontOrder:
            index = 0;
            break;
        case kBackOrder:
            index = 1;
            break;
        case kShuffledHoles:
            index = 2;
            break;
        }
        Layout_SetElementValueState(layout, index + kOrderOptionFirst, kLayoutSelected);
        if (index + kOrderOptionFirst != kSingleHoleOption)
            Layout_SetElementPlayerFlag(layout, index + kOrderOptionFirst, 0, kPlayerFlagChannel);
        if (index + kOrderOptionFirst < kCourseButtonFirst || index + kOrderOptionFirst > kCourseButtonLast) {
            if (index + kOrderOptionFirst != kConfirmElement) {
                Layout_SetElementValueState(layout, index + kFrontOrderHighlight, kLayoutSelected);
                Layout_SetElementPlayerFlag(layout, index + kFrontOrderHighlight, 0, kPlayerFlagChannel);
            }
        }
        ResetOtherButtonsInGroup(index + kOrderOptionFirst);
    }
    return 1;
}

} // extern "C"
