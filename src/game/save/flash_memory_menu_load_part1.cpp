#include <decomp/static_init.h>
// Compare borrowed entry positions without changing their ownership.
template <class T> inline bool same_pos(const T& a, const T& b) { return a == b; }
// Translation unit 0x8027CAA8-0x8027D564 (11 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CFlashMemoryMenuLoad.h"
#include "classes/CFlashMemoryMenu.h"

// Fields inherited from the file-I/O menu but absent from its current shared view.
struct FlashMenuIoView {
    char base[0x28];
    void* saveForm;
    char padding2C[0x44 - 0x2C];
    int result;
};
enum { kSaveSlotBlockBytes = 0x20A0, kLoadMenuEntryCount = 2 };

#define AsCFlashMemoryMenu(p) ((CFlashMemoryMenu*)(p))

#define AsCFlashMemoryMenuLoad(p) ((CFlashMemoryMenuLoad*)(p))
extern "C" {

extern float kFloat_6_2831855;
extern float s_flashMemoryLoadFullTurn;
// Initialize this menu unit's angle constant to one turn.
void InitFlashMemoryLoadAngleConstant() { s_flashMemoryLoadFullTurn = kFloat_6_2831855; }

// Report whether the load flow was aborted.
unsigned char CFlashMemoryMenuLoad_vfunc_1(void* self) { return AsCFlashMemoryMenuLoad(self)->aborted; }

// Clear the file-menu result before beginning an operation.
int CFlashMemoryMenu_ClearResult(void* self) { ((FlashMenuIoView*)self)->result = 0; return 1; }

extern void* SaveData_AcquireInitializedInstance();
extern int SaveData_SetSharedRecordMode(void*, int, unsigned char);
extern int CFileIoAccess_QueryResourcePredicate8(void*);
extern void* CFileIoMenu_FindValidSaveRecord(void*);
extern void SaveData_InitSlotRecordDefaults(void*);
extern void Memory_Copy(void*, const void*, unsigned int);
extern void SaveData_CopyRecordBlock(void*, const void*);
// The save file stores the four slot records in the Wii (big-endian) byte order; see
// save_data_records_and_defaults.cpp for the field map.
extern void* SaveSlotArray_CopyCtor(void*, void*);
extern int CFileIoAccess_GetTransferredBytes(void*);
extern void* CFileIoAccess_GetBuffer(void*);
extern int SaveForm_VerifyAllRecordCopies(void*, void*, void*, int);
extern void* __nw__FUl(unsigned int);
extern void __dl__FPv(void*);
extern void CFlashMemoryMenu_Ctor(void*);
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_fadeOverlayDtorRegistration_FileIoMenu[];
extern void FadeOverlay_Init(void*);
extern void ResourceState_Dtor(void*, int);
extern void __register_global_object(void*, void (*)(void*, int), void*);
extern void FadeState_StartFadeIn(void*, int);
extern char __vt__20CFlashMemoryMenuLoad[];
extern signed char s_movieControllerInitialized;
extern char s_movieController[];
extern char s_movieControllerDtorRegistration_Movie[];
extern void MovieController_Ctor(void*);
extern void MovieController_Dtor(void*, int);
extern void MovieController_ReleasePlaybackReference(void*);
extern void appFreeUni__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void CFlashMemoryMenu_dtor(void*, int);
extern int CFlashMemoryMenu_vfunc_8(void*);
extern void FadeState_SetAlternateDrawEnabled(void*, int);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nwa__FUl(unsigned int);
extern unsigned int strlen(const char*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char flashLoadVectorExceptionStrings[];
extern void strcpy(char*, const char*);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void __dla__FPv(void*);
extern void __throw(const char*, void*, void (*)(void*)) __attribute__((noreturn));
extern void __dt__Q23std12length_errorFv(void*);
// The original flow after a save-less first start: Manage Data > slot 1 > name > Create User Data, then it writes
// the new save (pangya.dat + banner.bin).
// A new account gets the screen defaults (Scout, Righty, bit 6 = Beginner set = club set 14) and arms the
// play-time counter through SaveData_HasSlotSelectionChanged.
// Forward the selected shared-record mode to the save-data singleton.
void CFlashMemoryMenu_SetSharedRecordMode(void* unused, int recordMode, unsigned char modeFlags) {
    void* saveData = SaveData_AcquireInitializedInstance();
    SaveData_SetSharedRecordMode(saveData, recordMode, modeFlags);
}

// Forward the attached resource's slot-8 predicate; save/load callers use it for busy/retry handling.
int CFlashMemoryMenu_QueryResourcePredicate8(void* self) {
    int predicateSet = 0;
    void* fileIo = AsCFlashMemoryMenu(self)->fileIo;
    if (fileIo) {
        if (CFileIoAccess_QueryResourcePredicate8(fileIo) != 0) predicateSet = 1;
    }
    return predicateSet;
}

// Copy a validated slot block into the active save-data singleton.
int CFlashMemoryMenu_LoadValidSaveRecord(void* self) {
    if (AsCFlashMemoryMenu(self)->fileIo == 0) return 0;
    void* validRecord = CFileIoMenu_FindValidSaveRecord(self);
    if (validRecord == 0) return 0;
    char recordBlock[kSaveSlotBlockBytes];
    SaveData_InitSlotRecordDefaults(recordBlock);
    Memory_Copy(recordBlock, validRecord, kSaveSlotBlockBytes);
// The save file holds the Wii (big-endian) byte order.
    void* saveData = SaveData_AcquireInitializedInstance();
    SaveData_CopyRecordBlock(saveData, recordBlock);
    return 1;
}

// Start the shared fade overlay when the caller supplies a nonzero flag.
void CFlashMemoryMenuLoad_StartFadeIn(void* self, int flag) {
    if (flag == 0) return;
    if (Sing_805DC608() == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_FileIoMenu);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = (void*)s_fadeOverlay;
    }
    FadeState_StartFadeIn(Sing_805DC608(), 0);
}



class FlashMenuSlotDispatch { public: 
virtual void v08(); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual void v28(); virtual void v2C(); virtual void v30(); virtual void v34(); virtual void v38(); virtual void v3C(); virtual void v40(); virtual void v44(); virtual void v48(); virtual void* getSaveSlotBlock();
};
// Verify the attached file buffer against a copy of the menu's current slot block.
int CFlashMemoryMenu_VerifyWrittenSlotBlock(void* self) {
    char recordBlock[kSaveSlotBlockBytes];
    SaveSlotArray_CopyCtor(recordBlock, ((FlashMenuSlotDispatch*)self)->getSaveSlotBlock());
    if (AsCFlashMemoryMenu(self)->fileIo == 0 || ((FlashMenuIoView*)self)->saveForm == 0) return 0;
    int transferredBytes = CFileIoAccess_GetTransferredBytes(AsCFlashMemoryMenu(self)->fileIo);
    void* fileBuffer = CFileIoAccess_GetBuffer(AsCFlashMemoryMenu(self)->fileIo);
    return SaveForm_VerifyAllRecordCopies(((FlashMenuIoView*)self)->saveForm, fileBuffer, recordBlock, transferredBytes);
}

class Z5 { public: 
virtual void v08(int); virtual void v0C(); virtual void v10(); virtual void v14(); virtual void v18(); virtual void v1C(); virtual void v20(); virtual void v24(); virtual int v28();
};
// Invoke the load menu initialization slot; zero means creation must be rolled back.
static inline int InitializeFlashLoadMenu(void* menu) { return ((Z5*)menu)->v28(); }
// Invoke the deleting slot after initialization fails; storage ownership ends here.
static inline void DestroyFailedFlashLoadMenu(void* menu) { ((Z5*)menu)->v08(1); }

// Construct the base and initialize all owned submenu/entry slots empty.
inline CFlashMemoryMenuLoad::CFlashMemoryMenuLoad(int skipSelection) {
    CFlashMemoryMenu_Ctor(this);
    vtbl = (void*)__vt__20CFlashMemoryMenuLoad;
    (*(int*)&AsCFlashMemoryMenuLoad(this)->state) = 0;
    (*(int*)&AsCFlashMemoryMenuLoad(this)->createAccountMenu) = 0;
    (*(int*)&AsCFlashMemoryMenuLoad(this)->nameEntryMenu) = 0;
    AsCFlashMemoryMenuLoad(this)->aborted = 0;
    AsCFlashMemoryMenuLoad(this)->createdNewSave = 0;
    AsCFlashMemoryMenuLoad(this)->skipAccountSelection = (unsigned char)skipSelection;
    AsCFlashMemoryMenuLoad(this)->deletingAccount = 0;
    AsCFlashMemoryMenuLoad(this)->storageCheckFailed = 0;
    AsCFlashMemoryMenuLoad(this)->invalidSaveData = 0;
    (*(int*)&AsCFlashMemoryMenuLoad(this)->entries) = 0;
    (*(int*)&AsCFlashMemoryMenuLoad(this)->entryCount) = 0;
    (*(int*)&AsCFlashMemoryMenuLoad(this)->entryCapacity) = 0;
    (*(int*)&AsCFlashMemoryMenuLoad(this)->loadDataMenu) = 0;
}

// Allocate the load menu, set its selection policy, and run initialization.
void* CFlashMemoryMenuLoad_Create(int skipAccountSelection) {
    CFlashMemoryMenuLoad* menu = new CFlashMemoryMenuLoad(skipAccountSelection);
    if (menu == 0) return 0;
    int initialized = InitializeFlashLoadMenu(menu);
    if (initialized == 0) {
        if (menu != 0) {
            DestroyFailedFlashLoadMenu(menu);
        }
        return 0;
    }
    return menu;
}

class CFMML_Del { public: 
virtual void del(int);
};
// Remove a range from the logical entry count after its objects have been destroyed.
static inline void cfmml_erase(unsigned int& size, unsigned int first, unsigned int last) {
    size -= last - first;
}
// Clear the entry count and free its array after all pointed-to menus are destroyed.
static inline void ReleaseFlashLoadEntryStorage(void* self) {
    // Keep integer address checks: pointer-member tests change CodeWarrior's register allocation.
    if ((int)(char*)self + (int)&((CFlashMemoryMenuLoad*)0)->entries) {
        if ((int)(char*)self + (int)&((CFlashMemoryMenuLoad*)0)->entries) {
            if ((int)(char*)self + (int)&((CFlashMemoryMenuLoad*)0)->entries) {
                if ((int)(char*)self + (int)&((CFlashMemoryMenuLoad*)0)->entries) {
                    if ((int)(char*)self + (int)&((CFlashMemoryMenuLoad*)0)->entries) {
                        if (AsCFlashMemoryMenuLoad(self)->entries != 0) {
                            cfmml_erase(AsCFlashMemoryMenuLoad(self)->entryCount, 0, AsCFlashMemoryMenuLoad(self)->entryCount);
                            appFreeUni__FPv(AsCFlashMemoryMenuLoad(self)->entries);
                        }
                    }
                }
            }
        }
    }
}

// Release the movie playback reference and destroy owned submenus and entry storage.
void* CFlashMemoryMenuLoad_dtor(void* self, int deleteFlag) {
    if (self != 0) {
        int movieController = ((int&)Sing_805DC950());
        *(void**)self = __vt__20CFlashMemoryMenuLoad;
        if (movieController == 0) {
            if (s_movieControllerInitialized == 0) {
                MovieController_Ctor(s_movieController);
                __register_global_object(s_movieController, (void(*)(void*, int))MovieController_Dtor, s_movieControllerDtorRegistration_Movie);
                s_movieControllerInitialized = 1;
            }
            ((int&)Sing_805DC950()) = (int)s_movieController;
        }
        MovieController_ReleasePlaybackReference((void*)((int&)Sing_805DC950()));
        void** ownedMenu = (void**)&AsCFlashMemoryMenuLoad(self)->createAccountMenu;
        if (*ownedMenu) {
            if (*ownedMenu) ((CFMML_Del*)*ownedMenu)->del(1);
            *ownedMenu = 0;
        }
        ownedMenu = (void**)&AsCFlashMemoryMenuLoad(self)->nameEntryMenu;
        if (*ownedMenu) {
            if (*ownedMenu) ((CFMML_Del*)*ownedMenu)->del(1);
            *ownedMenu = 0;
        }
        void** item = (*(void***)&AsCFlashMemoryMenuLoad(self)->entries);
        unsigned int oldEntryCount = AsCFlashMemoryMenuLoad(self)->entryCount;
        while (!same_pos((*(void***)&AsCFlashMemoryMenuLoad(self)->entries) + AsCFlashMemoryMenuLoad(self)->entryCount, item)) {
            void* value = *item;
            if (value) {
                if (value) ((CFMML_Del*)value)->del(1);
                *item = 0;
            }
            ++item;
        }
        ReleaseFlashLoadEntryStorage(self);
        CFlashMemoryMenu_dtor(self, 0);
        if (deleteFlag > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"

extern "C" void* __nw__FUl(unsigned int);

struct V8Allocation {
    char* pointer; char* allocator;
    // Guard an allocated exception-message array until shared ownership succeeds.
    V8Allocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Free the message array only while this guard retains it.
    ~V8Allocation() { if (pointer) __dla__FPv(pointer); }
};
struct V8ControlBase {
    void* table; unsigned int strong, weak;
    // Start the message control block with one strong and one weak reference.
    V8ControlBase() { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; }
};
struct V8Control : V8ControlBase {
    char* pointer;
    // Attach the already allocated message array to its control block.
    V8Control(char* p) { pointer = p; }
};
struct V8ErrorBase {
    void* table;
    // Initialize the length exception through its logic-error base vtable.
    V8ErrorBase() { table = __vt__Q23std11logic_error; }
    // Destroy the exception base without deleting this subobject.
    ~V8ErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : V8ErrorBase {
    char* text; void* control;
    // Copy the diagnostic and throw with the original exception metadata.
    length_error(const char* message) {
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        unsigned int n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            V8Allocation allocation(p, &allocator.value);
            control = new V8Control(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
        __throw(flashLoadVectorExceptionStrings + 0x14, this, __dt__Q23std12length_errorFv);
    }
};
}
// Raise the vector length exception using the original exception metadata.
static inline void v8_throw() {
    const char* msg = flashLoadVectorExceptionStrings;
    std::length_error error(msg);
}
// Reject vector growth beyond the 32-bit pointer-array allocation limit.
static inline void v8_check_len(unsigned int size, unsigned int count, unsigned int oldCap, unsigned int cap) {
    if (size + count - oldCap > 0x3fffffff - cap) v8_throw();
}
// Fill newly added entry slots with the supplied pointer-sized value.
static inline void v8_filln(unsigned int* p, unsigned int n, unsigned int value) {
    for (unsigned int i = n; i != 0; --i) *p++ = value;
}


// Private storage views: V8Vec owns the live pointer slots; V8Buf owns staged
// growth. V8Allocation/V8Control/V8ErrorBase belong only to the length-error path.
// Their existing names are retained because destructors participate in EH relocations.
struct V8Vec { unsigned int* data; unsigned int size; unsigned int cap; };
struct V8Buf {
    unsigned int* p; unsigned int constructedCount; unsigned int cap; unsigned int* alloc; unsigned int insertionOffset;
    // Stage an empty replacement allocation for entry-vector growth.
    V8Buf(unsigned int* a) { alloc = a; p = 0; constructedCount = 0; cap = 0; insertionOffset = 0; }
    // Free uncommitted storage, or the old allocation after a successful swap.
    ~V8Buf() { constructedCount = 0; if (p) { constructedCount = 0; appFreeUni__FPv(p); } }
};
// Select the larger growth request without changing reference lifetimes.
static inline const unsigned int& v8_max(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }
// Choose the original 60%, then 50%, pointer-vector growth policy.
static inline unsigned int v8_capacity(unsigned int cp, unsigned int needed) {
    if (cp < 0x15555555) {
        unsigned int candidate = ((cp + 1) * 3) / 5;
        return cp + v8_max(candidate, needed);
    } else if (cp < 0x2AAAAAAA) {
        unsigned int candidate = (cp + 1) / 2;
        return cp + v8_max(candidate, needed);
    }
    return 0x3fffffff;
}
// Transfer old entries into replacement storage and clear the old pointer slots.
static inline void v8_move(V8Buf& buf, unsigned int* data, unsigned int* last, int& n) {
    n = ((char*)last - (char*)data) / 4;
    int nbytes = n * 4;
    buf.insertionOffset -= n;
    memcpy(buf.p + buf.insertionOffset, data, nbytes);
    memset(data, 0, nbytes);
    buf.constructedCount += n;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes on
#pragma opt_strength_reduction off
typedef V8Vec FlashLoadEntryVector;
typedef V8Buf FlashLoadEntryGrowthOwner;

// Initialize the base menu, enable the fade overlay, and allocate two entry slots.
extern "C" int CFlashMemoryMenuLoad_InitializeMenuAndEntries(void* param) {
    int movedEntryCount;
    unsigned int oldCapacity;
    unsigned int newCapacity;
    unsigned int entriesToAppend;
    void* self;
    FlashLoadEntryVector* entries;
    unsigned int emptyEntry;
    self = param;
    // Phase 1: initialize base behavior and publish/enable the fade overlay.
    if (CFlashMemoryMenu_vfunc_8(self) == 0) return 0;
    if (Sing_805DC608() == 0) {
        if (s_fadeOverlayInitialized == 0) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_FileIoMenu);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_SetAlternateDrawEnabled(Sing_805DC608(), 1);
    // Phase 2: grow or trim the pointer array to the two menu entry slots.
    entries = (FlashLoadEntryVector*)(((char*)&AsCFlashMemoryMenuLoad(self)->entries));
    unsigned int oldSize = entries->size;
    emptyEntry = 0;
    if (oldSize < kLoadMenuEntryCount) {
        // Original 0x8027CAA8 unit: share completion after either resize path.
        do {
            unsigned int oldCap = entries->cap;
            entriesToAppend = kLoadMenuEntryCount - oldSize;
            if (entriesToAppend > oldCap || oldSize > oldCap - entriesToAppend) {
                oldCapacity = entries->cap;
                v8_check_len(entries->size, entriesToAppend, oldCap, oldCapacity);
                v8_capacity(oldCapacity, entries->size + entriesToAppend - entries->cap);
            } else {
                v8_filln(entries->data + oldSize, entriesToAppend, emptyEntry);
                entries->size += entriesToAppend;
                break;
            }
            unsigned int needed = entries->size + entriesToAppend - entries->cap;
            FlashLoadEntryGrowthOwner replacement(&entries->cap);
            newCapacity = entries->cap;
            if (needed > 0x3fffffff - newCapacity) v8_throw();
            unsigned int secondCandidate;
            unsigned int firstCandidate;
            if (newCapacity < 0x15555555) {
                firstCandidate = ((newCapacity + 1) * 3) / 5;
                newCapacity += v8_max(firstCandidate, needed);
            } else if (newCapacity < 0x2AAAAAAA) {
                secondCandidate = (newCapacity + 1) / 2;
                newCapacity += v8_max(secondCandidate, needed);
            } else {
                newCapacity = 0x3fffffff;
            }
            if (newCapacity > 0x3fffffff) v8_throw();
            replacement.p = (unsigned int*)appMallocDummy__FUl(newCapacity * 4);
            replacement.cap = newCapacity;
            replacement.insertionOffset = entries->size;
            v8_filln(replacement.p + replacement.insertionOffset + replacement.constructedCount, entriesToAppend, emptyEntry);
            replacement.constructedCount += entriesToAppend;
            v8_move(replacement, entries->data, entries->data + entries->size, movedEntryCount);
            // Phase 3: publish replacement storage; its guard takes the old allocation.
            unsigned int* previousData = entries->data;
            unsigned int constructedEntryCount = replacement.constructedCount;
            unsigned int previousCapacity = entries->cap;
            unsigned int* replacementData = replacement.p;
            entries->size = 0;
            replacement.p = previousData;
            entries->cap = replacement.cap;
            replacement.cap = previousCapacity;
            entries->data = replacementData;
            entries->size = constructedEntryCount;
        } while (0);
    } else if (oldSize > kLoadMenuEntryCount) {
        entries->size -= oldSize - kLoadMenuEntryCount;
    }
    return 1;
}
#pragma pop

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623C8", _ctors_u8027CAA8_0, InitFlashMemoryLoadAngleConstant);
#pragma pop

