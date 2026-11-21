#include <decomp/ct_member.h>
#include <decomp/offset_of.h>
extern "C" char s_renderContextRegistrations_805041E8[];
#include "MSL/throw.h"
#include "MSL/error_message.h"
#include "MSL/vector.h"
// Containers whose deleting destructors this unit keeps out of line under plain names. The element
// types are placeholders: the destructor depends only on whether elements are scalars or records.
typedef std::vector<void*, app::app_allocator<void*> > MSLPointerVector;
#include "classes/CEnemyRenderRecord.h"
#include "classes/CEnemyRenderTable.h"
#define AsCEnemyRenderTable(p) ((CEnemyRenderTable*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char charaSelectStrings[];
}
}

#include "game/vector.h"
extern "C" void __construct_array(void*, void*, void*, unsigned int, unsigned int);
// Translation unit 0x80241CEC-0x80243434 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"
#include "game/exception.h"
#include <decomp/cw_bitfields.h>
extern "C" {

enum CharaSelectConstants {
    kPlayerSlots = 4,
    kInvalidEntryId = -1,
    kLayoutType = 5,
    kLayoutDescriptorCount = 0x52,
    kInactiveLayoutGameMode = 6,
    kHideCaddieGameMode = 7,
    kEnemySelectionCategory = 2,
    kRandomEnemyRecord = 0x8000
};
// Mode numbers 6/7 come from CSharedDoc.gameMode. This unit establishes only
// their effects (6 leaves the layout inactive; 7 removes the caddie). The actual
// game/dialog names are unresolved; these local constants do not name game modes.
// Views of the shared context: four saves, four ID vectors, then enemy records.
struct PlayerSaveStorage { char bytes[0x828]; };
struct SlotEntryVector { int* data; unsigned int count; unsigned int capacity; };
struct CharaRenderContext {
    PlayerSaveStorage saves[kPlayerSlots];
    SlotEntryVector slotEntries[kPlayerSlots];
    CEnemyRenderRecord enemyRecords[kPlayerSlots];
    int enemyRecordCount;
};
typedef char RenderContext_entries_offset[((int)DECOMP_OFFSETOF(CharaRenderContext, slotEntries) == 0x20A0) ? 1 : -1];
typedef char RenderContext_records_offset[((int)DECOMP_OFFSETOF(CharaRenderContext, enemyRecords) == 0x20D0) ? 1 : -1];


// Byte address order is intentional in the slot updater (CodeWarrior scheduling).
enum RenderContextOffsets {
    kSlotEntryStride = sizeof(SlotEntryVector),
    kEnemyRecordStride = sizeof(CEnemyRenderRecord),
    kPlayerSaveStride = sizeof(PlayerSaveStorage),
    kSlotEntryCountOffset = (int)DECOMP_OFFSETOF(CharaRenderContext, slotEntries[0].count),
    kEnemyRecordsOffset = (int)DECOMP_OFFSETOF(CharaRenderContext, enemyRecords[0]),
    kEnemyFlagsOffset = (int)DECOMP_OFFSETOF(CharaRenderContext, enemyRecords[0].flags),
    kEnemyCaddieInventoryIndexOffset = (int)DECOMP_OFFSETOF(CharaRenderContext, enemyRecords[0].caddieInventoryIndex),
    kEnemyPlayerSlotOffset = (int)DECOMP_OFFSETOF(CharaRenderContext, enemyRecords[0].pad_18)
};
enum CharaSelectStringOffset {
    kLayoutName = 0x11,
    kPrimaryScriptName = 0x1F,
    kSecondaryScriptName = 0x2D,
    kVectorLengthError = 0x44,
    kLengthErrorType = 0x58
};

void RenderContext_ResetPlayerRecords(void*);
void* CharacterPreviewTable_dtor(void*, int);
void appFreeUni__FPv(void*);

struct RenderContextConstructionCleanup_u80241CEC {
    char* base;
    bool active;
    // Arm cleanup after construction of the context's records and vectors.
    explicit RenderContextConstructionCleanup_u80241CEC(void* object)
        : base((char*)object), active(true) {}
    // Roll back owned subobjects if the context reset throws.
    ~RenderContextConstructionCleanup_u80241CEC() {
        if (active) {
            CharacterPreviewTable_dtor(base + 0x20D0, -1);
            for (int index = 3; index >= 0; --index) {
                unsigned int* vector = (unsigned int*)(base + 0x20A0 + index * 12);
                if (vector[0]) {
                    vector[1] = 0;
                    appFreeUni__FPv((void*)vector[0]);
                }
            }
        }
    }
};

// Reset the newly constructed context, unwinding its owned records and vectors if reset throws.
static void InitializeNewRenderContext(void* base) {
    RenderContextConstructionCleanup_u80241CEC cleanup(base);
    RenderContext_ResetPlayerRecords(base);
    cleanup.active = false;
}


static char s_inputManagerDtorRegistration_CharaSelect[12];
extern char s_inputManagerDtorRegistration_CharaSelect[];
static char s_bpCtrlDtorRegistration_805041C4[12];
extern char s_bpCtrlDtorRegistration_805041C4[];
#pragma force_active on
static char s_charaSelectUnusedDtorRegistration[12];
#pragma force_active off
static char s_bpCtrlHandleDtorRegistration_80241CEC[12];
extern char __vt__12CCharaSelect[], s_gameInputManager[], s_bpCtrl[];
extern int s_resourceTableStorage;
extern CSharedDoc* g_sharedDoc;
extern signed char s_inputManagerInitialized, s_bpCtrlInitialized, s_resourceTableInitialized;
void InputManager_Init(void*); void InputManager_Dtor(); void InputManager_SetSelectedPlayer(void*, int);
void __ct__7CBpCtrlFv(void*); void CBpCtrl_dtor();

extern int s_resourceTableStorage;
extern signed char s_resourceTableInitialized;
extern CSharedDoc* g_sharedDoc;
extern int s_charaSelectInvalidEntryId;
// The remaining data label is the 82-entry character-selection layout descriptor table.
extern char charaSelectStrings[], charaSelectLayoutDescriptors[], __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
void MPBase1_Ctor(void* self); int ResourceTable_FindEntryByName(void*, const char*, int*);
void* __nw__FUl(unsigned int); void* __nwa__FUl(unsigned int);
void CPackTextureResource_Init(void*, const char*, int, int);
void Layout_Init(void*, int); void Layout_InitStandard(void*, int, const char*, int, const char*);
void Layout_SetActive(void*, int); void Allocation_Dtor(); void CBpCtrl_CreateEntry(void*, const char*, int*, int);
void* appMallocDummy__FUl(unsigned int); void appFreeUni__FPv(void*);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);

struct CharaSelectConstructionView {
    void* vtable; char baseStorage[0x14]; int playerState0, playerState1, playerState2, playerState3, selectionConfirmed;
    void* layout; void* packTextureResource; void* playerTransitions; int primaryBpEntryId, secondaryBpEntryId, inputReady;
    void* selectionResource; int* playerEntryIds; unsigned int playerEntryCount, playerEntryCapacity; int loadDataMenu, selectedSlot, slotStorage;
};

extern char charaSelectStrings[];

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
extern void __dla__FPv(void*);
struct LayoutVectorAllocation : std::detail::__message_buffer_guard {
    LayoutVectorAllocation(char* p, char* a) : std::detail::__message_buffer_guard(p, a) {}
    ~LayoutVectorAllocation() { dispose(); }
};
typedef std::detail::__message_control LayoutVectorControl;
struct LayoutVectorErrorBase_u80241CEC : std::exception {
    // Install the requested exception vtable without changing the object layout.
    void SetTable(const void* vt) { *(const void**)this = vt; }

    // Initialize the logic-error base used by vector length exceptions.
    LayoutVectorErrorBase_u80241CEC() { SetTable(__vt__Q23std11logic_error); }
};
namespace std {
struct length_error : LayoutVectorErrorBase_u80241CEC {
    std::__error_message<LayoutVectorAllocation, LayoutVectorControl> message_;
    // Copy the vector diagnostic and install its length-error vtable.
    length_error() {
        const char* message = charaSelectStrings + kVectorLengthError;
        message_.assign(message);
        SetTable(__vt__Q23std12length_error);
    }

    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
extern void __dt__Q23std12length_errorFv();
// Construct and throw the vector length exception using aligned stack storage.
inline void LayoutVectorThrow() { {
    std::__throw_exception<std::length_error>( exception_runtime::charaSelectStrings + kLengthErrorType, (void*)exception_runtime::__dt__Q23std12length_errorFv);
} }
struct LayoutVectorBuffer {
    int* data; volatile unsigned size; unsigned capacity; unsigned* owner; unsigned offset;
    // Initialize an empty replacement buffer tied to the destination capacity field.
    LayoutVectorBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free whichever vector allocation the replacement buffer currently owns.
    ~LayoutVectorBuffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Return the larger growth bound by reference.
inline const unsigned& LayoutVectorMax(const unsigned& a, const unsigned& b) { return a < b ? b : a; }
// Compute bounded 3/5 or 1/2 vector growth, throwing on overflow.
inline unsigned LayoutVectorGrowth(const unsigned& initialCapacity, unsigned needed) {
    unsigned capacity = initialCapacity;
    if (needed > 0x3FFFFFFFu - capacity) LayoutVectorThrow();
    if (capacity < 0x15555555u)
        return capacity + LayoutVectorMax((3 * (capacity + 1)) / 5, needed);
    if (capacity < 0x2AAAAAAAu)
        return capacity + LayoutVectorMax((capacity + 1) / 2, needed);
    return 0x3FFFFFFFu;
}
struct CharaPackResource {
    char bytes[0x20];
    // Construct the texture pack from a borrowed texture-name table.
    CharaPackResource(const char* name, int textureNamesAddress, int textureCount) { CPackTextureResource_Init(this, name, textureNamesAddress, textureCount); }
};
struct CharaLayoutResource {
    char bytes[0x5C];
    // Initialize the owned character-selection layout wrapper.
    CharaLayoutResource(int type) { Layout_Init(this, type); }
};

#pragma push
#pragma use_lmw_stmw on
// Create character selection resources, BP scripts and four initially invalid player entry IDs.
CharaSelectConstructionView* CCharaSelect_Ctor(CharaSelectConstructionView* self) {
    // Phase 1: initialize ownership fields, acquire input, and select player zero.
    char* destructorRegistrations = s_inputManagerDtorRegistration_CharaSelect;
    MPBase1_Ctor(self);
    int inputManager = ((int&)Sing_805DC5C8());
    self->selectionConfirmed = 0;
    self->vtable = __vt__12CCharaSelect;
    self->layout = 0; self->packTextureResource = 0;
    self->selectionResource = 0; self->playerEntryIds = 0; self->playerEntryCount = 0; self->playerEntryCapacity = 0;
    self->loadDataMenu = 0; self->selectedSlot = 0; self->slotStorage = 0;
    if (inputManager == 0) {
        if (s_inputManagerInitialized == 0) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, destructorRegistrations + 0);
            s_inputManagerInitialized = 1;
        }
        ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
    }
    InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), 0);
    // Phase 2: load the texture-name table, then create the owned pack and layout.
    int resourceTable = ((int&)Sing_805DC620());
    int textureNamesAddress = 0;
    if (resourceTable == 0) {
        if (s_resourceTableInitialized == 0) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor, (destructorRegistrations + 0x3C));
            s_resourceTableInitialized = 1;
        }
        ((int&)Sing_805DC620()) = (int)&s_resourceTableStorage;
    }
    int textureCount = ResourceTable_FindEntryByName((void*)((int&)Sing_805DC620()), charaSelectStrings, &textureNamesAddress);
    self->packTextureResource = new CharaPackResource(charaSelectStrings, textureNamesAddress, textureCount);
    self->playerState0 = 0;
    self->playerState1 = 0;
    self->playerState2 = 0;
    self->playerState3 = 0;
    self->layout = new CharaLayoutResource(kLayoutType);
    Layout_InitStandard(self->layout, 0, charaSelectLayoutDescriptors, kLayoutDescriptorCount, charaSelectStrings + kLayoutName);
    if (AsCSharedDoc(g_sharedDoc)->gameMode != kInactiveLayoutGameMode) Layout_SetActive(self->layout, 1);
    int invalid = -1;
    // Phase 3: create two BP entries and allocate the per-player transition block.
    // Singleton checks are repeated at each use; keep their acquisition order.
    int cleared = 0;
    int bpController = ((int&)Sing_805DC5F0());
    self->primaryBpEntryId = invalid;
    self->secondaryBpEntryId = invalid;
    self->inputReady = cleared;
    if (bpController == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (destructorRegistrations + 0x0C));
            s_bpCtrlInitialized = 1;
        }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), charaSelectStrings + kPrimaryScriptName, &self->primaryBpEntryId, 0);
    if (((int&)Sing_805DC5F0()) == 0) {
        if (s_bpCtrlInitialized == 0) { __ct__7CBpCtrlFv(s_bpCtrl); __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (destructorRegistrations + 0x0C)); s_bpCtrlInitialized = 1; }
        ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
    }
    CBpCtrl_CreateEntry((void*)((int&)Sing_805DC5F0()), charaSelectStrings + kSecondaryScriptName, &self->secondaryBpEntryId, 0);
    self->playerTransitions = __nwa__FUl(0x30);
    unsigned int size = self->playerEntryCount;
    // Phase 4: resize the ID vector to exactly four invalid entries.
    // In-place append and shrinking need no allocation. The growth path below
    // builds a guarded replacement, copies the old prefix, swaps ownership, and
    // lets the temporary free the previous allocation. Count <= capacity at exit.
    // Original 0x80241CEC: all resize paths join before returning the object.
    do {
        if (size < 4) {
            unsigned int add = 4 - size;
            if (add > self->playerEntryCapacity || size > self->playerEntryCapacity - add) {
                LayoutVectorGrowth(self->playerEntryCapacity, ((volatile CharaSelectConstructionView*)self)->playerEntryCount + add - self->playerEntryCapacity);
            } else {
                int* appendPosition = self->playerEntryIds + size;
                for (unsigned int i = add; i != 0; --i) *appendPosition++ = s_charaSelectInvalidEntryId;
                self->playerEntryCount += add;
                break;
            }
            {
                LayoutVectorBuffer temp(&self->playerEntryCapacity);
                unsigned int needed = self->playerEntryCount + add - self->playerEntryCapacity;
                unsigned int capacity = self->playerEntryCapacity;
                if (needed > 0x3FFFFFFFu - capacity) {
                    std::__throw_exception<std::length_error>( exception_runtime::charaSelectStrings + kLengthErrorType, (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                unsigned secondGrowth;
                unsigned firstGrowth;
                if (capacity < 0x15555555u) {
                    firstGrowth = (3 * (capacity + 1)) / 5;
                    capacity += LayoutVectorMax(firstGrowth, needed);
                } else if (capacity < 0x2AAAAAAAu) {
                    secondGrowth = (capacity + 1) / 2;
                    capacity += LayoutVectorMax(secondGrowth, needed);
                } else capacity = 0x3FFFFFFF;
                if (capacity > 0x3FFFFFFFu) {
                    std::__throw_exception<std::length_error>( exception_runtime::charaSelectStrings + kLengthErrorType, (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
                int* data = (int*)appMallocDummy__FUl(capacity * 4);
                // Reserve replacement storage; construct the appended suffix first.
                unsigned int oldSize = self->playerEntryCount;
                temp.data = data;
                temp.capacity = capacity;
                int* newEntries = data + oldSize + temp.size;
                temp.offset = oldSize;
                for (unsigned int i = add; i != 0; --i) *newEntries++ = s_charaSelectInvalidEntryId;
                unsigned int count;
                int* oldData = self->playerEntryIds;
                int* oldEnd = oldData + self->playerEntryCount;
                temp.size += add;
                count = ((char*)oldEnd - (char*)oldData) / 4;
                // Move the old prefix before the suffix; clear the moved words.
                temp.offset -= count;
                unsigned int bytes = count * 4;
                memcpy(temp.data + temp.offset, oldData, bytes);
                memset(oldData, 0, bytes);
                unsigned int newSize = temp.size + count;
                int* previousData = self->playerEntryIds;
                int* newData = temp.data;
                temp.size = newSize;
                unsigned int oldCapacity = self->playerEntryCapacity;
                // Commit: the object owns newData; temp now owns previousData.
                unsigned int newCapacity = temp.capacity;
                self->playerEntryCount = 0;
                temp.data = previousData;
                self->playerEntryCapacity = newCapacity;
                temp.capacity = oldCapacity;
                self->playerEntryIds = newData;
                self->playerEntryCount = newSize;
                temp.size = 0;
            }
        } else if (size > 4) {
            self->playerEntryCount = self->playerEntryCount - (size - 4);
        }
        } while (0);
    return self;
}
#pragma pop

extern char __vt__12CCharaSelect[];
extern char s_inputManagerDtorRegistration_CharaSelect[];
extern signed char s_bpCtrlInitialized;
extern signed char s_inputManagerInitialized;
extern int s_bpCtrlHandle[2];
extern signed char s_bpCtrlHandleInitialized;
extern char s_bpCtrl[];
extern char s_gameInputManager[];
void CBpCtrlHandle_Dtor();
void Layout_Destroy(void*, int);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor();
void CBpCtrl_DeleteEntryById(void*, int);
void InputManager_Init(void*);
void InputManager_Dtor();
void InputManager_SetSelectedPlayer(void*, int);
void CBpCtrlHandle_ReleaseEntry(void*);
void __dla__FPv(void*);
void __dl__FPv(void*);
void appFreeUni__FPv(void*);

struct CharaSelectDestructionView {
    void* vtable;
    char baseStorage[0x28];
    void* layout;
    void* packTextureResource;
    void* playerTransitions;
    int primaryBpEntryId;
    int secondaryBpEntryId;
    char inputReady[4];
    void* selectionResource;
    int playerEntryIds;
    unsigned int playerEntryCount;
    char entryVectorTail[0x0C];
    void* slotStorage;
};
struct SelectionOwnedObject {
    virtual void destroy(int);
};

// Release selection UI resources and BP entries, then free the per-player entry vector.
CharaSelectDestructionView* CCharaSelect_dtor(CharaSelectDestructionView* self, int deleting) {
    if (self != 0) {
        self->vtable = __vt__12CCharaSelect;
        if (self->layout != 0) {
            Layout_Destroy(self->layout, 1);
            self->layout = 0;
        }
        if (self->primaryBpEntryId != -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (s_bpCtrlInitialized == 0) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_805041C4);
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->primaryBpEntryId);
            self->primaryBpEntryId = -1;
        }
        if (self->secondaryBpEntryId != -1) {
            if (((int&)Sing_805DC5F0()) == 0) {
                if (s_bpCtrlInitialized == 0) {
                    __ct__7CBpCtrlFv(s_bpCtrl);
                    __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_805041C4);
                    s_bpCtrlInitialized = 1;
                }
                ((int&)Sing_805DC5F0()) = (int)s_bpCtrl;
            }
            CBpCtrl_DeleteEntryById((void*)((int&)Sing_805DC5F0()), self->secondaryBpEntryId);
            self->secondaryBpEntryId = -1;
        }
        if (self->playerTransitions != 0) {
            __dla__FPv(self->playerTransitions);
            self->playerTransitions = 0;
        }
        if (self->selectionResource != 0) {
            if (self->selectionResource != 0) {
                ((SelectionOwnedObject*)self->selectionResource)->destroy(1);
            }
            self->selectionResource = 0;
        }
        if (((int&)Sing_805DC5C8()) == 0) {
            if (s_inputManagerInitialized == 0) {
                InputManager_Init(s_gameInputManager);
           
           
           
                __register_global_object(s_gameInputManager, (void*)InputManager_Dtor, s_inputManagerDtorRegistration_CharaSelect);
                s_inputManagerInitialized = 1;
            }
            ((int&)Sing_805DC5C8()) = (int)s_gameInputManager;
        }
        InputManager_SetSelectedPlayer((void*)((int&)Sing_805DC5C8()), 0);
        if (self->packTextureResource != 0) {
            if (self->packTextureResource != 0) {
                ((SelectionOwnedObject*)self->packTextureResource)->destroy(1);
            }
            self->packTextureResource = 0;
        }
        if (((int&)Sing_805DC5F8()) == 0) {
            if (s_bpCtrlHandleInitialized == 0) {
                s_bpCtrlHandle[0] = -1;
                s_bpCtrlHandle[1] = -1;
                __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_80241CEC);
                s_bpCtrlHandleInitialized = 1;
            }
            ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
        }
        CBpCtrlHandle_ReleaseEntry((void*)((int&)Sing_805DC5F8()));
        if (self->slotStorage != 0) {
            __dl__FPv(self->slotStorage);
            self->slotStorage = 0;
        }
        if (&self->playerEntryIds != 0) {
            if (&self->playerEntryIds != 0) {
                if (&self->playerEntryIds != 0) {
                    if (&self->playerEntryIds != 0) {
                        if (&self->playerEntryIds != 0) {
                            if (self->playerEntryIds != 0) {
                                self->playerEntryCount = self->playerEntryCount - (int)self->playerEntryCount;
                                appFreeUni__FPv((void*)self->playerEntryIds);
                            }
                        }
                    }
                }
            }
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

extern signed char s_bpCtrlHandleInitialized;
extern int s_bpCtrlHandle[2];
extern char s_bpCtrlHandleDtorRegistration_80241CEC[];
void CBpCtrlHandle_Dtor();

// Lazily initialize the pair of BP entry handles and register its destructor.
int* CBpCtrlHandle_GetInstance() {
    if (((int&)Sing_805DC5F8()) == 0) {
        if (s_bpCtrlHandleInitialized == 0) {
            s_bpCtrlHandle[0] = -1;
            s_bpCtrlHandle[1] = -1;
            __register_global_object(s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_80241CEC);
            s_bpCtrlHandleInitialized = 1;
        }
        ((int&)Sing_805DC5F8()) = (int)s_bpCtrlHandle;
    }
    return (int*)((int&)Sing_805DC5F8());
}

void appFreeUni__FPv(void*);
void __dl__FPv(void*);
struct Dtor80242548 { void* ptr; unsigned int count; };
// Free the pointer-vector allocation, optionally deleting its owning header.
MSLPointerVector* CharaSelectEntryVector_dtor(MSLPointerVector* self, int deleting) {
    return ::std::__vector_deleting_dtor(self, deleting);
}

extern signed char s_renderContextInitialized;
extern char s_renderContext[];
extern char s_renderContextRegistrations_805041E8[];
void SaveData_InitPlayerRecord(void*);


void PartyItemRecord_InitDefaults(void*);
void PlayerRenderRecord_dtor(void*);
void RenderContext_ResetPlayerRecords(void*);
void __dt__13RenderContextFv(void*);
extern CSharedDoc* g_sharedDoc;
extern signed char s_pangyaEnemyDataInitialized;
extern char s_pangyaEnemyData[];
extern char s_pangyaEnemyDataDtorRegistration[];
void CPangyaEnemyData_Init(void*);
void CPangyaEnemyData_dtor();
void EnemyData_SelectRandomMatchingRecord(void*, void*, int, int, int);
void CCharacterPreviewRecord_FillFromSaveData(void*, int, void*);
char* RenderContext_GetInstance();

// Ensure the shared render context exists; retained inline to preserve initialization at each use.
static inline char* base_8024290C() {
    if (((int&)Sing_805DC648()) == 0) {
        if (s_renderContextInitialized == 0) {
            char* end;
            char* base = s_renderContext;
            end = base + 0x20A0;
            char* p = base;
            do { SaveData_InitPlayerRecord(p); p += 0x828; } while ((unsigned)p < (unsigned)end);
            ((IntVectorArray*)(end))->__ct();
            end = base + 0x20D0;
            __construct_array(end, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
            AsCEnemyRenderTable(end)->count = 0;
            InitializeNewRenderContext(base);
            __register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextRegistrations_805041E8);
            s_renderContextInitialized = 1;
        }
        ((int&)Sing_805DC648()) = (int)s_renderContext;
    }
    return (char*)((int&)Sing_805DC648());
}

// Lazily initialize the enemy-data singleton and register its destructor.
static inline void EnsureEnemyData() {
    if (((int&)Sing_805DC678()) == 0) {
        if (s_pangyaEnemyDataInitialized == 0) {
            CPangyaEnemyData_Init(s_pangyaEnemyData);
            __register_global_object(s_pangyaEnemyData, (void*)CPangyaEnemyData_dtor, s_pangyaEnemyDataDtorRegistration);
            s_pangyaEnemyDataInitialized = 1;
        }
        ((int&)Sing_805DC678()) = (int)s_pangyaEnemyData;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Populate a render slot from saved player data or enemy data, applying mode-specific caddie hiding.
// The first argument is unused. playerSlot (0..3) selects the nonempty entry-list
// gate and enemy selection filter; renderSlot (0..3) selects the destination record
// AND the save block used in the player-data branch. They need not be equal.
// recordId is forwarded as the saved character/enemy selector; its range belongs
// to those providers. useEnemyData is a pointer-encoded boolean: only nullness is
// tested and the address is never read. randomizeEnemy forces provider selection;
// otherwise an equal cached ID is retained and marked with bit 0x8000.
// Returns nothing; an empty playerSlot leaves all destination state unchanged.
// All singleton/save storage is borrowed; no input pointer is retained or freed.
void CharaSelect_UpdateSlotRenderRecord(void*, int playerSlot, int recordId, void* useEnemyData, int renderSlot, int randomizeEnemy) {
    if (*(int*)(base_8024290C() + kSlotEntryCountOffset + playerSlot * kSlotEntryStride) == 0) return;
    if (useEnemyData != 0) {
        if (randomizeEnemy != 0) {
            void* obj;
            EnsureEnemyData();
            obj = (void*)((int&)Sing_805DC678());
            EnemyData_SelectRandomMatchingRecord(obj, base_8024290C() + renderSlot * kEnemyRecordStride + kEnemyRecordsOffset, kEnemySelectionCategory, recordId, playerSlot);
        } else {
            int* item = (int*)(base_8024290C() + renderSlot * kEnemyRecordStride + kEnemyRecordsOffset);
            if (recordId != item[0]) {
                void* obj;
                EnsureEnemyData();
                obj = (void*)((int&)Sing_805DC678());
                EnemyData_SelectRandomMatchingRecord(obj, base_8024290C() + renderSlot * kEnemyRecordStride + kEnemyRecordsOffset, kEnemySelectionCategory, recordId, playerSlot);
            } else {
                *(unsigned short*)(base_8024290C() + renderSlot * kEnemyRecordStride + kEnemyFlagsOffset) |= kRandomEnemyRecord;
            }
        }
        if (AsCSharedDoc(g_sharedDoc)->gameMode == kHideCaddieGameMode) {
            *(int*)(base_8024290C() + renderSlot * kEnemyRecordStride + kEnemyCaddieInventoryIndexOffset) = -1;
        }
    } else {
        *(int*)(base_8024290C() + renderSlot * kEnemyRecordStride + kEnemyPlayerSlotOffset) = playerSlot;
        struct Flags { CW_BITS2( unsigned short high : 1, unsigned short low : 15) };
        ((Flags*)(base_8024290C() + renderSlot * kEnemyRecordStride + kEnemyFlagsOffset))->high = 0;
        char* base = base_8024290C();
        char* save = base_8024290C();
        CCharacterPreviewRecord_FillFromSaveData(base + renderSlot * kEnemyRecordStride + kEnemyRecordsOffset, recordId, save + renderSlot * kPlayerSaveStride);
        if (AsCSharedDoc(g_sharedDoc)->gameMode == kHideCaddieGameMode) {
            *(int*)(base_8024290C() + renderSlot * kEnemyRecordStride + kEnemyCaddieInventoryIndexOffset) = -1;
        }
    }
}

#pragma pop

// Construct four save records, entry vectors and enemy render records on first use.
char* RenderContext_GetInstance() {
    if (((int&)Sing_805DC648()) == 0) {
        if (s_renderContextInitialized == 0) {
            char* end;
            char* base = s_renderContext;
            end = base + 0x20A0;
            char* p = base;
            do { SaveData_InitPlayerRecord(p); p += 0x828; } while ((unsigned)p < (unsigned)end);
            ((IntVectorArray*)(end))->__ct();
            end = base + 0x20D0;
            __construct_array(end, (void*)PartyItemRecord_InitDefaults, (void*)PlayerRenderRecord_dtor, 0xA4, 4);
            AsCEnemyRenderTable(end)->count = 0;
            InitializeNewRenderContext(base);
__register_global_object(s_renderContext, (void*)__dt__13RenderContextFv, s_renderContextRegistrations_805041E8);
            s_renderContextInitialized = 1;
        }
        ((int&)Sing_805DC648()) = (int)s_renderContext;
    }
    return (char*)((int&)Sing_805DC648());
}

} // extern "C"
