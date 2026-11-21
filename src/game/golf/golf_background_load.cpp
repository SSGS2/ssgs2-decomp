typedef unsigned long new_size_t;
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
struct MSLRecord { unsigned int words[1]; };
typedef std::vector<MSLRecord, app::app_allocator<MSLRecord> > MSLRecordVector;
#include "classes/CSharedDoc.h"
// View borrowed shared-document storage without changing its ownership.
static inline CSharedDoc* AsCSharedDoc(void* p) { return (CSharedDoc*)p; }
#include "classes/CGolfDoc.h"
#include "iff/IffStruct.h"
// CGolfBg_OnLoad (0x80123CCC): shared document +0x8BC/+0xAD8;
// golf document pointers +0x8/+0xC/+0x10 and hole records +0x2BC.
// Translation unit 0x80123CCC-0x8012484C (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

// Construct in caller-owned storage; this placement form never allocates.
inline void* operator new(new_size_t, void* p) { return p; }

extern "C" {

void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void CShadowManager_ReplaceSelection(void*, const char*);
void CFx_SetUpdateEnabled(void*, int);
void* __nw__FUl(unsigned int);
void IActor_DispatchMessage(void*, void*, const char*, int, void**, void**, int, int);
void* SharedStringState_GetSecondString(void*);
int String_FormatTemporary(const char*, ...);
void strrchr(void*, int);
int CHashPath_FindPath(void*); // Called at 0x80123E54 with r3 = the formatted resource path.
int CGolfBg_BuildWeatherResourcePath(void*, void*);
int CPackedFile_Create(void*);
int CPackedFile_PollLoad(void*);
int CourseModel_LoadFromFile(void*, int);
void GolfParameters_LoadKeyedTextRecords(void*, int);
void CPvsFxBox_Init(void*);
void CPvsFxBox_LoadSkyResources(void*, int, float);
void CPvsFxBox_LoadVariantBitmap(void*, int);
int SoundName_BuildCourseSePrefix(void*, void*);
void SoundManager_AcquireSoundBank(void*, void*);
void String_ReserveCapacity(void*, void*);
void String_ReplaceRange(void*, int, int, void*, void*, void*);
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void CGolfBg_InitializeCurrentHole(void*);
void CupPreview_PollResources(void*);
int GolfCup_GetCaptureTextureReference(void*);
void ShadowManager_SetProjectionDrawArgument(void*, int);
void CupPreview_SetPosition(void*, void*);
void ShadowManager_SetCaptureQuadAlphaPreset(void*, int, int);

extern void* g_shadowManager;
extern void* g_fx;
extern void* g_sharedDoc;
extern unsigned char g_courseResourceListsEnabled;
extern unsigned char g_courseLoadFailed;
extern void* g_golfCupPreview;
extern float kHoleSetupZero;
extern void* g_soundManager;
extern char golfBackgroundStrings[];
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
void __dt__Q23std12length_errorFv(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dla__FPv(void*);
void __throw(const char*, void*, void*) __attribute__((noreturn));
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);

struct Struct_801247CC {
    void* member;       // Owned raw buffer, released by appFreeUni; payload type unresolved.
    volatile int count; // Reset before freeing the buffer; no element destructors run here.
};

// This background actor has no additional initialization work.
void CGolfBg_OnInit() {}

// Copy position components as words to retain the original vector-copy instructions.
struct Vec3 { int xBits, yBits, zBits; };
struct Entry { unsigned int flag; int begin, end; };
struct Obj { unsigned char pad[0x3B6]; unsigned char currentHoleIndex, resourceVariant; };


struct Allocator {
    char dummy;
    // Initialize the library allocator's ABI placeholder; it carries no allocation here.
    Allocator() : dummy(0) {}
};
struct CourseEffectBoxStorage {
    char storage[0x9CC]; // CPvsFxBox storage.
    // Initialize the course effect box in its allocated storage.
    CourseEffectBoxStorage() { CPvsFxBox_Init(this); }
};
// 12-byte string representation: flag bit 31 selects owned heap storage. Otherwise
// text starts at byte +1 of this object; size/data then overlap inline characters.
// Vector relocation below transfers this representation as bytes and zeros the old
// records to prevent their destructors from freeing transferred heap pointers.
struct String {
    unsigned int flag;
    unsigned int size;
    char* data;
    // Start with an empty inline string.
    String() : flag(0), size(0), data(0) {}
    // Copy inline storage directly; deep-copy heap storage with exception cleanup.
    String(const String& s) {
      try {
        unsigned int bits = s.flag;
        if (!(bits >> 31)) { unsigned int length = s.size; flag = bits; size = length; data = s.data; }
        else {
            flag = 0; size = 0; data = 0;
                String_ReserveCapacity(this, (void*)s.size);
                struct Dummy { char value; } dummy, copy = dummy;
                String_ReplaceRange(this, 0, 0, s.data, s.data + s.size, &copy);
        }
      } catch (...) { __throw(0, 0, 0); }
    }
    // Release heap text only; inline storage belongs to the containing record.
    ~String() { if (flag >> 31) appFreeUni__FPv(data); }
    // Borrow text until this string changes or is destroyed; never free this result.
    char* c_str() { return !(flag >> 31) ? (char*)this + 1 : data; }
};
// Owns data[0..size) and its buffer; capacity counts String records, not bytes.
struct Vector { String* data; unsigned int size, capacity; };
// Local views fill gaps in shared layouts without changing any shared header.
struct BackgroundResources { char actorStorage[0x34]; Vector soundBankNames; };
struct HoleCaptureSettings { char prefix[0x36C]; unsigned int preset[18]; };
struct CourseCaptureSettings {
    char prefix[0x83D];
    unsigned char alphaByHole[18];
    char betweenAlphaAndPreset[0x9C6 - 0x84F];
    unsigned char currentPreset;
    char beforeCourseName[0x9E4 - 0x9C7];
    char courseName[32];
};
struct HoleRecord { char prefix[12]; Vec3 cupPosition; char suffix[8]; };
struct BackgroundStringPool {
    char unusedBeforeFormats[8];
    char nightCourseFormat[16];
    char courseFormat[13];
    char unusedBeforeVectorError[0x67 - 0x25];
    char vectorLengthError[20];
    char unusedBeforeSelection[0xB2 - 0x7B];
    char shadowSelection[3];
    char storyActor[11];
    char storyCourseFormat[8];
    char waveParametersFormat[12];
    char courseNameFormat[3];
};
// Borrow the fixed-offset string pool; array sizes preserve adjacent string starts.
static inline BackgroundStringPool& GetBackgroundText() { return *(BackgroundStringPool*)golfBackgroundStrings; }
#define BackgroundText GetBackgroundText()
enum BackgroundTextOffset { kCourseNameFormatOffset = IFF_OFFSET(BackgroundStringPool, courseNameFormat) };
// floor(UINT_MAX / sizeof(String)): largest element count representable in byte size.
static const unsigned int kMaximumStringVectorSize = 0x15555555;
enum CourseLoadMode { kStoryMode = 6, kNightPartyMode = 2 };
enum BackgroundLoadConstants {
    kNoCaptureAlpha = 0xFD, // Per-hole byte sentinel: disable capture-quad alpha.
    kAimPathObjectBytes = 0x18,
    kLastModeWithPerHolePreset = 1, // The numeric game-mode meanings 0/1 are unresolved here.
    kStoryQueryMessage = 4,
    kPathNameSeparator = '_',
    kStringGrowthThreeFifthsLimit = 0x071C71C7,
    kStringGrowthHalfLimit = 0x0E38E38E
};

struct BufferGuard_u80123CCC {
    char* pointer;
    char* allocator;
    // Guard a newly allocated exception-message array until its deleter takes ownership.
    BufferGuard_u80123CCC(char* p, char* a) : pointer(p), allocator(a) {}
    // Release only an untransferred array; allocator is an ABI placeholder.
    ~BufferGuard_u80123CCC() { if (pointer) __dla__FPv(pointer); }
};
} // extern "C"
namespace std {
struct exception {};
struct logic_error : exception {
    void* vtable;
    char* text;
    void* ref;
    // Own a copied message through a shared deleter, guarded until ownership transfers.
    logic_error(const char* const& s) {
        vtable = __vt__Q23std11logic_error;
        Allocator allocator;
        // Allocate the NUL-terminated message, keeping it guarded during setup.
        unsigned int messageBytes = strlen(s) + 1;
        char* buffer = (char*)__nwa__FUl((messageBytes << 1) - messageBytes);
        text = buffer;
        {
        BufferGuard_u80123CCC guard(text, (char*)&allocator);
        // Install the shared deleter control block before copying the message bytes.
        // The two count words retain their original initial value; exact roles unresolved.
        struct Ref { void* vt; int a, b; char* ptr; };
        Ref* deleter = (Ref*)__nw__FUl(16);
        if (deleter) { deleter->a = 1; deleter->b = 1; deleter->vt = s_sharedCharDeleterVtable; deleter->ptr = buffer; }
        ref = deleter;
        guard.pointer = 0;
        }
        strcpy(text, s);
    }
    ~logic_error();
};
struct length_error : logic_error {
    // Reuse the owned logic-error message and install the derived exception vtable.
    length_error(const char* const& s) : logic_error(s) { vtable = __vt__Q23std12length_error; }
    ~length_error();
};
} // namespace std
extern "C" {
// Throw the library vector-capacity exception using the original message pool.
inline void length_error() {
    throw std::length_error(BackgroundText.vectorLengthError);
}
struct TempVector_u80123CCC {
    String* data;
    volatile unsigned int count;
    unsigned int capacity;
    unsigned int* allocator;
    unsigned int offset; // Insertion origin; becomes zero after prefix relocation below.
    // Start an empty growth buffer; a points to the destination's capacity word.
    // The historical allocator field records that address but is never read here.
    TempVector_u80123CCC(unsigned int* a) : data(0), count(0), capacity(0), allocator(a), offset(0) {}
    // Destroy only successfully constructed strings, then release the temporary buffer.
    ~TempVector_u80123CCC() {
        if (data) {
            unsigned int n = count;
            count = n - n;
            String* p = data + n;
            while (n) { --p; p->~String(); --n; }
            appFreeUni__FPv(data);
        }
    }
};
// Select a growth amount without copying either candidate.
// The returned reference borrows one argument; both arguments must outlive its use.
inline const unsigned int& max_ref(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }
struct Found {
    virtual ~Found();
};

// Load the selected hole model, effects and sound bank, then configure cup-preview shadows.
// self is the background actor. Shared/golf documents, selected course and current
// hole index must already be valid (hole tables have 18 entries). Resource paths
// returned through String_FormatTemporary's int ABI slot are borrowed strings.
// This is synchronous: an optional packed resource list is polled without yielding.
// Model failure sets g_courseLoadFailed and exits with effects still disabled; the
// optional list is only deleted on the success path. Preserve that cleanup behavior.
void CGolfBg_OnLoad(void* self) {
    // Phase 1: disable effect updates and install a fresh aim-path object on the golf doc.
    CShadowManager_ReplaceSelection(g_shadowManager, BackgroundText.shadowSelection);
    if (g_fx) CFx_SetUpdateEnabled(g_fx, 0);
    void* aimPath = __nw__FUl(kAimPathObjectBytes);
    if (aimPath) { ((void**)aimPath)[0] = 0; ((void**)aimPath)[1] = 0; ((void**)aimPath)[2] = 0; }
    // These aliases intentionally re-read the shared global at each use.
    #define manager AsCSharedDoc(g_sharedDoc)
    ((CGolfDoc*)((void*)manager->golfDoc))->aimPath = aimPath;
    #define root AsCSharedDoc(g_sharedDoc)
    if (root->gameMode <= kLastModeWithPerHolePreset) {
        Obj* golfDoc = (Obj*)((void*)manager->golfDoc);
        ((CourseCaptureSettings*)manager)->currentPreset = ((HoleCaptureSettings*)golfDoc)->preset[golfDoc->currentHoleIndex];
    }
    // Phase 2: choose story override, existing night variant, or ordinary course path.
    // Dispatch must initialize replyData whenever reply is non-null. The unusual
    // !replyData branch below is existing behavior; its intent is unresolved.
    void* reply = 0;
// The original leaves the slot stale; it is only read when reply is set.
    void* replyData;
    if (root->gameMode == kStoryMode) IActor_DispatchMessage(self, self, BackgroundText.storyActor, kStoryQueryMessage, &reply, &replyData, 0, 0);
    int courseModelPath = 0;
    if (reply && SharedStringState_GetSecondString(replyData)) {
        int storyCourseString = (int)(long)SharedStringState_GetSecondString(replyData);
        courseModelPath = String_FormatTemporary(BackgroundText.storyCourseFormat, storyCourseString);
        if (!replyData) strrchr(SharedStringState_GetSecondString(replyData), kPathNameSeparator);
    } else {
        if ((*(int*)&AsCSharedDoc(manager)->partyGameMode) == kNightPartyMode) {
            void* selectedCourse = AsCSharedDoc(manager)->selectedCourseData;
            courseModelPath = String_FormatTemporary(BackgroundText.nightCourseFormat, selectedCourse ? ((IFF_STRUCT::sCourse*)selectedCourse)->mapName : 0,
                ((Obj*)manager->golfDoc)->resourceVariant);
            if (!CHashPath_FindPath((void*)courseModelPath)) courseModelPath = 0;
        }
    }
    if (!courseModelPath) {
        void* selectedCourse = AsCSharedDoc(manager)->selectedCourseData;
        courseModelPath = String_FormatTemporary(BackgroundText.courseFormat, selectedCourse ? ((IFF_STRUCT::sCourse*)selectedCourse)->mapName : 0,
            ((Obj*)manager->golfDoc)->resourceVariant);
    }
    // Phase 3: keep the optional weather resource list alive while loading the model.
    void* resourceListFile = 0;
    if (g_courseResourceListsEnabled) {
        String weatherListPath;
        if (CGolfBg_BuildWeatherResourcePath(self, &weatherListPath)) {
            void* pathText = weatherListPath.c_str();
            resourceListFile = (void*)(long)CPackedFile_Create(pathText);
            if (resourceListFile) while (!CPackedFile_PollLoad(resourceListFile)) {}
        }
    }
    void* courseModel = ((CGolfDoc*)((void*)manager->golfDoc))->courseData;
    // 0x80123CCC unit: loading failure exits before scene setup.
    if (!CourseModel_LoadFromFile(courseModel, courseModelPath)) { g_courseLoadFailed = 1; return; }
    // Phase 4: model is ready; load keyed wave settings and create sky/bitmap effects.
    {
        void* selectedCourse = AsCSharedDoc(manager)->selectedCourseData;
        char* courseMapName = selectedCourse ? ((IFF_STRUCT::sCourse*)selectedCourse)->mapName : 0;
        void* golfDoc = ((void*)manager->golfDoc);
        // Populate the keyed beach-wave parameter map used by CBeach's named-wave lookup.
        GolfParameters_LoadKeyedTextRecords(&((CGolfDoc*)golfDoc)->beachParameterTreeCount, String_FormatTemporary(BackgroundText.waveParametersFormat, courseMapName));
    }
    void* effectBox = new CourseEffectBoxStorage;
    ((CGolfDoc*)((void*)manager->golfDoc))->courseEffectBox = effectBox;
    {
        void* selectedCourse = AsCSharedDoc(manager)->selectedCourseData;
        char* courseMapName = selectedCourse ? ((IFF_STRUCT::sCourse*)selectedCourse)->mapName : 0;
        void* golfDoc = ((void*)manager->golfDoc);
        int courseNameText = String_FormatTemporary(golfBackgroundStrings + kCourseNameFormatOffset, courseMapName);
        CPvsFxBox_LoadSkyResources(((CGolfDoc*)golfDoc)->courseEffectBox, courseNameText, kHoleSetupZero);
    }
    {
        void* selectedCourse = AsCSharedDoc(manager)->selectedCourseData;
        char* courseMapName = selectedCourse ? ((IFF_STRUCT::sCourse*)selectedCourse)->mapName : 0;
        void* golfDoc = ((void*)manager->golfDoc);
        int courseNameText = String_FormatTemporary(golfBackgroundStrings + kCourseNameFormatOffset, courseMapName);
        CPvsFxBox_LoadVariantBitmap(((CGolfDoc*)golfDoc)->courseEffectBox, courseNameText);
    }
    {
        // Phase 5: acquire the course bank and retain a copied name on this actor.
        String soundBankName;
        if (SoundName_BuildCourseSePrefix(((CourseCaptureSettings*)manager)->courseName, &soundBankName)) {
            SoundManager_AcquireSoundBank(g_soundManager, soundBankName.c_str());
            Vector* v = &((BackgroundResources*)self)->soundBankNames;
            if (v->size < v->capacity) {
                new (v->data + v->size) String(soundBankName);
                ++v->size;
            } else {
                // Reserve: compute overflow-checked growth in String records.
                if (kMaximumStringVectorSize - v->capacity < 1) length_error();
                unsigned int needed = v->size + 1 - v->capacity;
                TempVector_u80123CCC temp(&v->capacity);
                unsigned int oldCapacity = v->capacity;
                unsigned int capacity;
                if (needed > kMaximumStringVectorSize - oldCapacity) length_error();
                if (oldCapacity < kStringGrowthThreeFifthsLimit) {
                    unsigned int growth = (oldCapacity + 1) * 3 / 5;
                    capacity = oldCapacity + max_ref(growth, needed);
                } else if (oldCapacity < kStringGrowthHalfLimit) {
                    unsigned int growth = (oldCapacity + 1) / 2;
                    capacity = oldCapacity + max_ref(growth, needed);
                } else capacity = kMaximumStringVectorSize;
                if (capacity > kMaximumStringVectorSize) length_error();
                temp.data = (String*)appMallocDummy__FUl(capacity * sizeof(String));
                temp.capacity = capacity;
                temp.offset = v->size;
                String* dst = temp.data + temp.offset + temp.count;
                // Construct the appended element first; temp owns it if copying throws.
                for (unsigned int n = 1; n != 0; --n, ++dst) {
                    new (dst) String(soundBankName);
                    ++temp.count;
                }
                String* old = v->data;
                // Transfer existing representations into the prefix, without deep copies.
                // Zeroing old records transfers heap-text ownership to the new buffer.
                int moved = ((char*)(old + v->size) - (char*)old) / 12;
                temp.offset -= moved;
                memcpy(temp.data + temp.offset, old, moved * 12);
                memset(old, 0, moved * 12);
                temp.count += moved;
                String* oldBegin = v->data + temp.offset;
                String* oldEnd = oldBegin + v->size;
                unsigned int oldSize = v->size;
                unsigned int oldCap = v->capacity;
                String* fresh = temp.data;
                // Commit the swap below: v will own the new range, temp the old buffer.
                temp.data = v->data;
                v->capacity = temp.capacity;
                temp.capacity = oldCap;
                v->data = fresh;
                v->size = temp.count;
                temp.count = oldSize;
                // Destroy the zeroed old records, then let temp release their buffer.
                while (oldEnd > oldBegin) { --oldEnd; oldEnd->~String(); }
                temp.count = 0;
            }
        }
    }
    // Phase 6: finish hole setup, resume effects, then configure the cup preview.
    CGolfBg_InitializeCurrentHole(self);
    if (g_fx) CFx_SetUpdateEnabled(g_fx, 1);
    void* cupPreview = g_golfCupPreview;
    CupPreview_PollResources(cupPreview);
    void* shadowManager = g_shadowManager;
    ShadowManager_SetProjectionDrawArgument(shadowManager, GolfCup_GetCaptureTextureReference(cupPreview));
    {
        char* golfDoc = (char*)((void*)manager->golfDoc);
        Vec3 cupPosition = ((HoleRecord*)((CGolfDoc*)golfDoc)->holeRecords)[((CGolfDoc*)golfDoc)->currentHoleIndex].cupPosition;
        // Cup position is copied from the current hole record; no coordinate transform here.
        CupPreview_SetPosition(cupPreview, &cupPosition);
    }
    {
        // Field-derived byte indexing preserves the original add operand order.
        unsigned char captureAlpha = ((unsigned char*)manager)[IFF_OFFSET(CourseCaptureSettings, alphaByHole) + ((CGolfDoc*)manager->golfDoc)->currentHoleIndex];
        if (captureAlpha == kNoCaptureAlpha) ShadowManager_SetCaptureQuadAlphaPreset(g_shadowManager, 0, 0);
        else ShadowManager_SetCaptureQuadAlphaPreset(g_shadowManager, captureAlpha, ((unsigned char*)manager)[IFF_OFFSET(CourseCaptureSettings, currentPreset)]);
    }
    // The list's virtual destructor releases the temporary packed-file owner.
    if (resourceListFile) delete (Found*)resourceListFile;
    return;
}

// Release the container's raw buffer and optionally delete its owner.
// Accepts null; deleting > 0 frees self. Returns the original address, possibly freed.
// A non-null member is freed after resetting count; member is left unchanged.
MSLRecordVector* dtor_801247CC(MSLRecordVector* self, int deleting) {
    return ::std::__vector_deleting_dtor(self, deleting);
}

} // extern "C"
