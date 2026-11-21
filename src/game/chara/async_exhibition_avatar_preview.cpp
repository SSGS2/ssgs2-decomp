#include <decomp/static_init.h>
extern "C" char s_sharedFileListDtorRegistration_8026B004[];
extern "C" char s_startupSharedResourceHandles[];
extern "C" char s_taskSingletonRegistrations_80509C30[];
struct PreviewRectangle { int left, top, right, bottom; };
enum { kAttachmentSlotCount = 5, kLoadingScriptNameOffset = 0x10, kCommon2DArchiveNameOffset = 0x0F };
// Byte offsets into the startup global-registration block (s_taskSingletonRegistrations_80509C30): each is the
// slot handed to __register_global_object for one lazily built singleton, or the resident texture handle array.
enum {
    kRegistrationResourceTable = 0x18,
    kRegistrationSharedFileList = 0x3C,
    kResidentTextureHandles = 0x48
};
// Counts of entries in the tables Game_LoadResidentResources walks.
enum {
    kResidentTextureCount = 3,
    kSharedFileNameCount = 2,
    kSharedGolfUiArchiveCount = 3
};
#include "game/singletons.h"
#include "game/types.h"
#include "classes/CAsyncExhibition.h"
#include "classes/CAsyncExhibitionAvatar.h"

#define AsCAsyncExhibitionAvatar(p) ((CAsyncExhibitionAvatar*)(p))

#define AsCAsyncExhibition(p) ((CAsyncExhibition*)(p))
extern "C" void appFreeUni__FPv(void*);
namespace app { template<class T> class app_allocator {}; }
namespace std {
template<class T> class char_traits {};
template<class T, class Traits, class Alloc> class LocalBasicString_u80269E68 {
public:
    unsigned int words[3];
    // Release heap-backed temporary string storage; inline bytes need no disposal.
    ~LocalBasicString_u80269E68() { if (words[0] >> 31) appFreeUni__FPv((void*)words[2]); }
};
}
// Translation unit 0x80269E68-0x8026B004 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

enum {
    kAvatarDescriptorOffset = (int)&((CAsyncExhibitionAvatar*)0)->avatarDescriptor, kViewportRectangleOffset = (int)&((CAsyncExhibition*)0)->viewportLeft
}
;

extern "C" {

// Raw 0x68-byte ItemSelection copy contract, used by all three copy sites.
// +0..3 and +4..7 are copied separately; +8..0x67 contains twelve 8-byte
// records. The individual word/byte meanings and record-to-item mapping are
// unresolved here: Pair is a copy granularity, NOT a pair of preview objects.
// avatarDescriptor is the REQUESTED selection consumed by the preloaders;
// displayDescriptor is a DIFFERENT, 0xC8-byte resolved selection updated by
// the exhibition callback. attachments is a separate array of five pointers.
// These copies do not acquire references or transfer pointed-to resources.
// Local Pair/Body/Copy declarations deliberately repeat this representation:
// merging their types changed generated copy code under inline_depth(0).

extern "C" int CAsyncExhibition_InstallResolvedSelectionCallback(void*, void*);
extern "C" void* CAsyncExhibition_GetExhibition(void*);
extern "C" void* CAsyncExhibition_dtor(void*, int);
extern "C" void __dl__FPv(void*);
extern "C" void CExhibition_CopyResolvedAvatarSelection(void*, void*);

extern "C" void* CExhibition_SetViewport(void*, void*, float);
extern "C" int CExhibition_SetCameraDistanceLimits(void*, float, float);
extern "C" int CExhibition_SetRotationLimits(void*, float, float);
extern "C" int CExhibition_SetRotation(void*, float*);
extern "C" int CExhibition_ApplyTextureVariant(void*, void*, void*);
extern "C" int ItemSelection_Equals(void*, void*);
extern "C" int CExhibition_SetAttachment(void*, int, void*);
extern "C" void* CAsyncExhibition_Ctor(void*);
extern "C" void ItemSelection_SetStateIfSameCharacter(void*, void*, void*);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void ItemSelection_Init(void*);
extern "C" char __vt__22CAsyncExhibitionAvatar[];

extern "C" const float kFloat_6_2831855;
extern float s_asyncExhibitionAngleConstant;
extern float s_asyncAvatarExhibitionAngleConstant;
extern "C" const float kFloat_3_AvatarPreviewParameterStorage;
extern "C" void CExhibition_SetViewportYLimits(void*, unsigned int, unsigned int);
extern "C" int CPreloadAvatarAsync_Create(void*, int, int, int);
extern "C" void CExhibition_ConfigureAvatarPreview(void*, void*, void*, int, int, float);
extern "C" void __ct__7CBpCtrlFv(void*);
extern "C" void CBpCtrl_dtor(void*, int);
extern "C" void* __register_global_object(void*, void*, void*);
extern "C" void CBpCtrl_CreateEntry(void*, void*, void*, int);
extern "C" void* IdMap_Lookup(void*, int);
extern "C" void BpObject_TryInvokeCurrentCallback(void*, int);
extern "C" void CBpCtrl_DeleteEntryById(void*, int);
extern "C" int CExhibition_PlayAnimationByName(void*, void*, int, float);
extern "C" void ResolvedItemSelection_Clear(void*);
extern "C" void Memory_Clear(void*, int);
extern "C" void Memory_Copy(void*, void*, int);
extern "C" int CAsyncExhibition_TryInstallPreloadedObject(void*);
extern "C" int CAsyncExhibition_UpdateLoadingStateAndIndicator(void*, float);
extern "C" int CPreloadAvatarAsync_ReloadParts(void*, void*, int);
extern "C" int CExhibition_QueueModelUpdate(void*, void*, void*, int);
extern "C" void CExhibition_Ctor(void*, void*, float);
extern "C" int CExhibition_ClearControlFlag(void*, int);
extern "C" void CExhibition_ClearModelPreloader(void*);
extern "C" void CExhibition_SetModelPreloader(void*, void*);
extern "C" void CExhibition_SetVisible(void*, int);
extern "C" void CExhibition_GetCallbackResult(void*, int, void*);
extern "C" void* CExhibition_GetRotation(void*);
extern "C" signed char s_bpCtrlInitialized;
extern "C" char s_bpCtrl[];
extern "C" char s_asyncExhibitionBpCtrlDtorRegistration[];
extern int s_characterLoadingAnimationEntryId;
extern "C" const float kFloat_0_AsyncExhibition;
extern "C" float kFloat_325_AsyncExhibitionWidth;
extern "C" float kFloat_230_AsyncExhibitionHeightStorage;
extern "C" char __vt__16CAsyncExhibition[];
extern "C" char asyncExhibitionLoadingScriptName[];
extern "C" signed char s_resourceTableInitialized;
extern "C" int s_resourceTableStorage;
extern "C" int g_residentEffectArchive;
extern "C" int g_sharedResourcesReady;
extern "C" signed char s_sharedFileListInitialized;
extern "C" int g_sharedFileForMenuData;
extern "C" int sharedFileResourceNames;
extern "C" int sharedLabelResourceNames;
extern "C" char startupArchiveAndWarningStrings[];
extern "C" char* s_residentTextureResourceNames[3]; // three resident texture resource names
extern "C" char sharedGolfUiArchiveNames[];
extern "C" char s_menuSharedFilePath[];
extern "C" char s_sharedFileListStorage[];
extern "C" char s_taskSingletonRegistrations_80509C30[];
extern "C" void* CPackedFile_Create(void*);
extern "C" void* ResourceTable_FindEntryByName(int, const char*, int*);
extern "C" void* CPackTextureResource_Init(void*, const char*, int, int);
extern "C" int CPackedFile_PollLoad(void*);
extern "C" int CPackedFile_IsLoadFinished(void*);
extern "C" void CPackTextureResource_PollLoad(void*);
extern "C" int CPackTextureResource_IsLoadFinished(void*);
extern "C" void GolfRule_PreloadEffectSequences();
extern "C" int CreateSharedTextureResource__FPCci(const char*, int);
extern "C" void SharedFilePointerVector_InitReserved1024(const char*);
extern "C" void NString_AssignCString(void*, int);
extern "C" int SharedFile_Create(const char*, int);
extern "C" int CSharedFilePtr_GetLoadedBuffer(int);
extern "C" void appFreeUni__FPv(void*);
extern "C" int Allocation_Dtor(void*);
extern "C" int SharedFilePointerVector_Destroy(void*);
extern "C" void SharedFileList_CreateAndAppend(void*, void*, int);

// Toggle one of five preview attachments; commit the selection only if the model accepts it.
int CAsyncExhibition_ToggleAttachment(void* preview, unsigned int index, void* value) {
    if (index >= kAttachmentSlotCount) return 0;
    void* manager = AsCAsyncExhibition(preview)->exhibition;
    if (manager == 0) return 0;
    int slot = (int)index;
    if (slot >= kAttachmentSlotCount) slot = 0;
    if (value == AsCAsyncExhibition(preview)->attachments[slot]) value = 0;
    int result = CExhibition_SetAttachment(manager, (int)index, value);
    if (result != 0) {
        if ((int)index >= kAttachmentSlotCount) index = 0;
        AsCAsyncExhibition(preview)->attachments[index] = value;
    }
    return result;
}

// Apply a texture variant and update the resolved selection only after success.
int CAsyncExhibition_ApplyTextureVariant(void* preview, void* source, void* arg) {
    void* manager = AsCAsyncExhibition(preview)->exhibition;
    if (manager == 0) return 0;
    // Snapshot the raw selection before applying a variant; see the copy contract above.
    struct Pair { unsigned int a, b; };
    struct Body { Pair preview[12]; };
    struct Copy { unsigned int head; unsigned char bytes[4]; Body body; } copy;
    copy.head = *(unsigned int*)source;
    copy.bytes[0] = ((unsigned char*)source)[4];
    copy.bytes[1] = ((unsigned char*)source)[5];
    copy.bytes[2] = ((unsigned char*)source)[6];
    copy.bytes[3] = ((unsigned char*)source)[7];
    copy.body = *(Body*)((char*)source + 8);
    int result = CExhibition_ApplyTextureVariant(manager, source, arg);
    if (result != 0) ItemSelection_SetStateIfSameCharacter(AsCAsyncExhibition(preview)->displayDescriptor, &copy, arg);
    return result;
}

struct ObjA75CBase {
    ObjA75CBase();
    ~ObjA75CBase();
    char pad[0x124];
};

#pragma inline_depth(0)
// Construct the base preview and select the avatar interface before member initialization.
ObjA75CBase::ObjA75CBase() { CAsyncExhibition_Ctor(this); *(void**)this = __vt__22CAsyncExhibitionAvatar; }
// Destroy base-owned preview resources without deleting this enclosing allocation.
ObjA75CBase::~ObjA75CBase() { CAsyncExhibition_dtor(this, 0); }
#pragma inline_depth(255)

struct ObjA75C : ObjA75CBase {
    struct Member { Member(); char pad[0x68]; };
    Member member;
    int pendingUpdateFlags;
    unsigned char avatarLoadPending;
    unsigned char modelUpdatePending;
    int preloadAvatar;
    ObjA75C();
};

// Initialize the requested equipment descriptor subobject.
ObjA75C::Member::Member() { ItemSelection_Init(this); }

// Start with no deferred requests and no avatar-parts preloader.
ObjA75C::ObjA75C() {
    pendingUpdateFlags = 0;
    avatarLoadPending = 0;
    modelUpdatePending = 0;
    preloadAvatar = 0;
}

struct ObjA75CVt {
    virtual void destroy(int);
    virtual void v1(); virtual void v2(); virtual void v3();
    virtual int update(void*, void*, float);
};

// Allocate the avatar preview; destroy it if virtual initialization fails.
void* CAsyncExhibitionAvatar_Create(void* preview, void* arg, float value) {
    ObjA75C* object = new ObjA75C();
    if (object == 0) return 0;
    ObjA75CVt* x = (ObjA75CVt*)object;
    int upd = x->update(preview, arg, value);
    if (upd == 0) {
        if (object != 0) {
            x->destroy(1);
        }
        return 0;
    }
    return object;
}
struct ObjA9C4Vt { 
virtual void v0(); virtual void v1(); virtual int fn();
};

// Store a changed equipment selection and attachments, deferring installation until preloading is ready.
// source borrows a 0x68-byte ItemSelection; value borrows five attachment pointers
// or is null to clear them. 0 means unchanged OR immediate install failed;
// 1 can mean deferred acceptance, not that a visible model has been installed.
int CAsyncExhibitionAvatar_SetEquipment(void* preview, void* source, void* value) {
    if (ItemSelection_Equals(AsCAsyncExhibitionAvatar(preview)->avatarDescriptor, source)) return 0;
    char* self = (char*)preview;
    AsCAsyncExhibitionAvatar(self)->modelUpdatePending = 0;
    AsCAsyncExhibitionAvatar(self)->pendingUpdateFlags = 0;
    // Same raw requested-selection shape; no resolved display data or attachments inside it.
    struct Pair { unsigned int a, b; };
    struct Body { Pair preview[12]; };
    struct Copy { unsigned int head; unsigned char bytes[4]; Body body; };
    Copy* copy = (Copy*)(self + kAvatarDescriptorOffset);
    copy->head = *(unsigned int*)source;
    copy->bytes[0] = ((unsigned char*)source)[4];
    copy->bytes[1] = ((unsigned char*)source)[5];
    copy->bytes[2] = ((unsigned char*)source)[6];
    copy->bytes[3] = ((unsigned char*)source)[7];
    copy->body = *(Body*)((char*)source + 8);
    if (value) Memory_Copy(AsCAsyncExhibition(self)->attachments, value, sizeof(AsCAsyncExhibition(self)->attachments));
    else Memory_Clear(AsCAsyncExhibition(self)->attachments, sizeof(AsCAsyncExhibition(preview)->attachments));
    void* obj = AsCAsyncExhibitionAvatar(self)->resourcePreloader;
    if (obj && !((ObjA9C4Vt*)obj)->fn()) {
        AsCAsyncExhibitionAvatar(self)->avatarLoadPending = 1;
        return 1;
    }
    return CAsyncExhibition_TryInstallPreloadedObject(preview);
}

struct ObjVtSlot { 
virtual void v0(); virtual void v1(); virtual int fn();
};

#pragma inline_depth(0)
// Process the preview and complete deferred avatar/parts updates when their preloaders finish.
// A true return means the base preview was processed. It does not imply that
// either pending request reached a visible/install-complete state this frame.
int CAsyncExhibitionAvatar_UpdateDeferredRequests(void* preview, float delta) {
    // Phase 1: base update also controls visibility and the loading indicator.
    if (!CAsyncExhibition_UpdateLoadingStateAndIndicator(preview, delta)) return 0;
    // Phase 2: a ready resource preloader consumes the deferred install attempt.
    // The pending byte is cleared regardless of TryInstall's return value.
    if (AsCAsyncExhibitionAvatar(preview)->avatarLoadPending) {
        void* obj = AsCAsyncExhibitionAvatar(preview)->resourcePreloader;
        if (obj && ((ObjVtSlot*)obj)->fn()) {
            CAsyncExhibition_TryInstallPreloadedObject(preview);
            AsCAsyncExhibitionAvatar(preview)->avatarLoadPending = 0;
        }
    }
    // Phase 3: try the parts reload, then queue the merged update flags. Only a
    // successful queue clears pendingUpdateFlags. modelUpdatePending is cleared
    // once the outer preloader is ready even if parts/mgr/queue are not ready;
    // it is NOT a retry-until-installed flag. Preserve these distinct commits.
    if (AsCAsyncExhibitionAvatar(preview)->modelUpdatePending) {
        void* obj = AsCAsyncExhibitionAvatar(preview)->resourcePreloader;
        if (obj && ((ObjVtSlot*)obj)->fn()) {
            if (!AsCAsyncExhibitionAvatar(preview)->preloadAvatar
                || (((ObjVtSlot*)AsCAsyncExhibitionAvatar(preview)->preloadAvatar)->fn()
                    && CPreloadAvatarAsync_ReloadParts(AsCAsyncExhibitionAvatar(preview)->preloadAvatar, AsCAsyncExhibitionAvatar(preview)->avatarDescriptor, 1))) {
                void* mgr = CAsyncExhibition_GetExhibition(preview);
                if (mgr && CExhibition_QueueModelUpdate(mgr, AsCAsyncExhibitionAvatar(preview)->avatarDescriptor, AsCAsyncExhibition(preview)->displayDescriptor, AsCAsyncExhibitionAvatar(preview)->pendingUpdateFlags))
                    AsCAsyncExhibitionAvatar(preview)->pendingUpdateFlags = 0;
            }
            AsCAsyncExhibitionAvatar(preview)->modelUpdatePending = 0;
        }
    }
    return 1;
}
#pragma inline_depth(255)

// Initialize viewport interaction, resolved selection, attachments and loading-indicator state.
void* CAsyncExhibition_Ctor(void* preview) {
    AsCAsyncExhibition(preview)->cameraDistance = kFloat_0_AsyncExhibition;
    *(void**)preview = __vt__16CAsyncExhibition;
    AsCAsyncExhibition(preview)->previewMode = 0;
    (*(int*)&AsCAsyncExhibition(preview)->preloadObject) = 0;
    AsCAsyncExhibition(preview)->preloaderExternallyOwned = 0;
    ResolvedItemSelection_Clear(AsCAsyncExhibition(preview)->displayDescriptor);
    Memory_Clear(AsCAsyncExhibition(preview)->attachments, sizeof(AsCAsyncExhibition(preview)->attachments));
    float h;
    float w;
    float z;
    z = kFloat_0_AsyncExhibition;
    w = kFloat_325_AsyncExhibitionWidth;
    h = kFloat_230_AsyncExhibitionHeightStorage;
    AsCAsyncExhibition(preview)->dragStartDepth = z;
    AsCAsyncExhibition(preview)->pointerDragActive = 0;
    (*(int*)&AsCAsyncExhibition(preview)->exhibition) = 0;
    AsCAsyncExhibition(preview)->bpEntryId = -1;
    AsCAsyncExhibition(preview)->loadingIndicatorShown = 0;
    AsCAsyncExhibition(preview)->loadingIndicatorX = w;
    AsCAsyncExhibition(preview)->loadingIndicatorY = h;
    return preview;
}

// Create the shared character-loading animation once, using -1 as the absent-entry sentinel.
void CharacterLoading_CreateAnimationEntry() {
    if (s_characterLoadingAnimationEntryId == -1) {
            if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
            }
            CBpCtrl_CreateEntry(Sing_805DC5F0(), asyncExhibitionLoadingScriptName + kLoadingScriptNameOffset, &s_characterLoadingAnimationEntryId, 0);
    }
}


// Add the supplied Euler-angle offset to the active preview rotation.
int CAsyncExhibition_OffsetRotation(void* preview, Vec3f* offset) {
    void* obj = AsCAsyncExhibition(preview)->exhibition;
    if (obj == 0) return 0;
    float* pos = (float*)CExhibition_GetRotation(obj);
    unsigned int* up = (unsigned int*)pos;
    Vec3f adjusted = *(Vec3f*)pos;
    adjusted.x += offset->x;
    adjusted.y += offset->y;
    adjusted.z += offset->z;
    CExhibition_SetRotation(obj, &adjusted.x);
    return 1;
}

struct ObjACF8 {
    char pad[0x20];
    // Initialize temporary texture-resource storage using its resolved table entry.
    ObjACF8(const char* path, int local, int n) { CPackTextureResource_Init(this, path, local, n); }
};

struct ObjDel1 { 
virtual void destroy(int);
};

// Lazily construct and register the resident shared-file list singleton (Sing_805DC680), then return it.
static void* getSharedFileList(char* registrations) {
    if (((int&)Sing_805DC680()) == 0) {
        if (!s_sharedFileListInitialized) {
            SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
            __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy, (registrations + kRegistrationSharedFileList));
            s_sharedFileListInitialized = 1;
        }
        ((int&)Sing_805DC680()) = (int)(long)s_sharedFileListStorage;
    }
    return (void*)((int&)Sing_805DC680());
}

#pragma push
#pragma use_lmw_stmw on
// Load resident effects, common UI textures and shared files; wait for required archives to finish.
void Game_LoadResidentResources() {
    // Phase 1: start the effect archive and resolve the common-2D texture archive.
    register char* registrations = s_taskSingletonRegistrations_80509C30;
    void* x = CPackedFile_Create(startupArchiveAndWarningStrings);
    g_residentEffectArchive = (int)x;
    int local = 0;
    if (((int&)Sing_805DC620()) == 0) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object((void*)&s_resourceTableStorage, (void*)Allocation_Dtor, registrations + kRegistrationResourceTable);
            s_resourceTableInitialized = 1;
        }
        ((int&)Sing_805DC620()) = (int)&s_resourceTableStorage;
    }
    int n = (int)ResourceTable_FindEntryByName(((int&)Sing_805DC620()), startupArchiveAndWarningStrings + kCommon2DArchiveNameOffset, &local);
    ObjACF8* object = new ObjACF8(startupArchiveAndWarningStrings + kCommon2DArchiveNameOffset, local, n);
    // Phase 2: synchronously poll both required resources before effect preloading.
    while (!CPackedFile_IsLoadFinished((void*)g_residentEffectArchive)) CPackedFile_PollLoad((void*)g_residentEffectArchive);
    while (!CPackTextureResource_IsLoadFinished(object)) CPackTextureResource_PollLoad(object);
    GolfRule_PreloadEffectSequences();
    // Phase 3: populate resident textures and the shared-file list; the shared
    // resources-ready global is cleared here, not set by this routine.
    g_sharedResourcesReady = 0;
    {
        char** names = s_residentTextureResourceNames;
        int* out = (int*)((registrations + kResidentTextureHandles));
        for (unsigned int i = 0; i < kResidentTextureCount; ++i) {
            *out = CreateSharedTextureResource__FPCci(*names, 0);
            ++names;
            ++out;
        }
    }
    {
        int* table = &sharedFileResourceNames;
        for (unsigned int i = 0; i < kSharedFileNameCount; ++i) SharedFileList_CreateAndAppend(getSharedFileList(registrations), (void*)table[i], 0);
    }
    {
        int* table = (int*)sharedGolfUiArchiveNames;
        for (unsigned int i = 0; i < kSharedGolfUiArchiveCount; ++i) SharedFileList_CreateAndAppend(getSharedFileList(registrations), (void*)table[i], 0);
    }
    for (unsigned int i = 0; i < 1; ++i) {
    std::LocalBasicString_u80269E68<char, std::char_traits<char>, app::app_allocator<char> > args;
    args.words[0] = 0;
    args.words[1] = 0;
    args.words[2] = 0;
    NString_AssignCString(&args, sharedLabelResourceNames);
    SharedFileList_CreateAndAppend(getSharedFileList(registrations),
                (args.words[0] >> 31) == 0 ? (void*)((unsigned int)&args + 1) : (void*)args.words[2], 0);
    }
    // Phase 4: wait for menu data to expose its loaded buffer, then dispose only
    // the temporary common-2D wrapper. Resident globals/list retain their resources.
    g_sharedFileForMenuData = SharedFile_Create(s_menuSharedFilePath, 0);
    bool done;
    do {
        done = CSharedFilePtr_GetLoadedBuffer(g_sharedFileForMenuData) != 0;
    } while (!done);
    if (object) {
        if (object) ((ObjDel1*)object)->destroy(1);
    }
}
#pragma pop

// Report whether a deferred avatar-install attempt remains scheduled.
unsigned char CAsyncExhibitionAvatar_GetDeferredInstallAttemptPending(void* preview) {
    return AsCAsyncExhibitionAvatar(preview)->avatarLoadPending;
}

// Return the callback that installs resolved item selections in the exhibition.
void* CAsyncExhibition_vfunc_9() {
    return (void*)CAsyncExhibition_InstallResolvedSelectionCallback;
}

// Cache 2*pi for the base preview at startup; no consumer is established here.
void CAsyncExhibition_InitAngleConstant() {
    s_asyncExhibitionAngleConstant = kFloat_6_2831855;
}

// Cache 2*pi for the avatar preview at startup; no consumer is established here.
void CAsyncExhibitionAvatar_InitAngleConstant() {
    s_asyncAvatarExhibitionAngleConstant = kFloat_6_2831855;
}

// Forward viewport changes when an exhibition exists; otherwise return null.
void* CAsyncExhibition_SetViewport(void* preview, void* rectangle, float amount) {
    void* x = AsCAsyncExhibition(preview)->exhibition;
    if (x != 0) {
        return CExhibition_SetViewport(x, rectangle, amount);
    }
    return 0;
}

// Forward camera distance limits when an exhibition exists.
int CAsyncExhibition_SetCameraDistanceLimits(void* preview, float minimumDistance, float maximumDistance) {
    void* x = AsCAsyncExhibition(preview)->exhibition;
    if (x != 0) {
        return CExhibition_SetCameraDistanceLimits(x, minimumDistance, maximumDistance);
    }
    return 0;
}

// Forward camera rotation limits when an exhibition exists.
int CAsyncExhibition_SetRotationLimits(void* preview, float minimumRotation, float maximumRotation) {
    void* x = AsCAsyncExhibition(preview)->exhibition;
    if (x != 0) {
        return CExhibition_SetRotationLimits(x, minimumRotation, maximumRotation);
    }
    return 0;
}

// Set preview rotation and report failure when no exhibition exists.
int CAsyncExhibition_SetRotation(void* preview, void* vec) {
    void* x = AsCAsyncExhibition(preview)->exhibition;
    if (x == 0) {
        return 0;
    }
    CExhibition_SetRotation(x, (float*)vec);
    return 1;
}

// Set vertical viewport limits and report failure when no exhibition exists.
int CAsyncExhibition_SetViewportYLimits(void* preview, unsigned int minimumYBits, unsigned int maximumYBits) {
    void* x = AsCAsyncExhibition(preview)->exhibition;
    if (x == 0) {
        return 0;
    }
    CExhibition_SetViewportYLimits(x, minimumYBits, maximumYBits);
    return 1;
}

// Create the equipment preloader for the stored avatar descriptor.
int CAsyncExhibitionAvatar_CreateAvatarPreloader(void* preview) {
    void* self = preview;
    int ret = CPreloadAvatarAsync_Create(AsCAsyncExhibitionAvatar(self)->avatarDescriptor, 0, 0, 1);
    (*(int*)&AsCAsyncExhibitionAvatar(self)->preloadAvatar) = ret;
    return ret;
}

#pragma inline_depth(0)
// Configure the exhibition from this avatar descriptor and its attachments.
int CAsyncExhibitionAvatar_ConfigureAvatarPreview(void* preview) {
    void* self = preview;
    void* x = CAsyncExhibition_GetExhibition(preview);
    CExhibition_ConfigureAvatarPreview(x, AsCAsyncExhibitionAvatar(self)->avatarDescriptor, AsCAsyncExhibition(self)->attachments, 0, 0, kFloat_3_AvatarPreviewParameterStorage);
    return 1;
}
#pragma inline_depth(255)

// Destroy the avatar preview and optionally free its allocation.
void* CAsyncExhibitionAvatar_dtor(void* preview, int dealloc) {
    void* self = preview;
    int shouldDelete = dealloc;
    if (self != 0) {
        CAsyncExhibition_dtor(self, 0);
        if (shouldDelete > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

struct ObjAC88 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void fn(void*, int, int, int);
};

// Restart the existing avatar preloader with the current equipment descriptor.
int CAsyncExhibitionAvatar_RestartAvatarPreloader(void* preview) {
    char* self = (char*)preview;
    ObjAC88* x = (ObjAC88*)AsCAsyncExhibitionAvatar(self)->preloadAvatar;
    if (x == 0) {
        return 0;
    }
    x->fn(AsCAsyncExhibitionAvatar(self)->avatarDescriptor, 0, 0, 1);
    return 1;
}

// Install the resolved selection in the exhibition and clear its model-update callback.
int CAsyncExhibition_InstallResolvedSelectionCallback(void* a, void* b) {
    void* obj = b;
    if (obj == 0) {
        return 0;
    }
    void* x = AsCAsyncExhibition(obj)->exhibition;
    if (x == 0) {
        return 0;
    }
    CExhibition_CopyResolvedAvatarSelection(x, AsCAsyncExhibition(obj)->displayDescriptor);
    CExhibition_GetCallbackResult(x, 0, 0);
    return 1;
}

struct ObjA338 {
    virtual void v0();
    virtual void v1();
    virtual int fn();
};

// Play a named animation only when resource preloading is complete and an exhibition exists.
int CAsyncExhibition_PlayFixedNameAnimation(void* preview, void* q) {
    void* self = preview;
    void* arg = q;
    void* x = AsCAsyncExhibition(self)->preloadObject;
    if (x != 0) {
        ObjA338* o = (ObjA338*)x;
        int r = o->fn();
        if (r == 0) {
            return 0;
        }
    }
    void* y = AsCAsyncExhibition(self)->exhibition;
    if (y == 0) {
        return 0;
    }
    CExhibition_PlayAnimationByName(y, arg, 0, kFloat_0_AsyncExhibition);
    return 1;
}

// Invoke the shared loading-animation entry callback.
void CharacterLoading_InvokeAnimationCallback() {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            char* base = s_bpCtrl;
            __ct__7CBpCtrlFv(base);
            __register_global_object(base, (void*)CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* o = IdMap_Lookup(Sing_805DC5F0(), s_characterLoadingAnimationEntryId);
    BpObject_TryInvokeCurrentCallback(o, 0);
}

// Delete the shared character-loading animation entry and reset its id to -1.
void CharacterLoading_DeleteAnimationEntry() {
    if (s_characterLoadingAnimationEntryId == -1) {
        return;
    }
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CBpCtrl_DeleteEntryById(Sing_805DC5F0(), s_characterLoadingAnimationEntryId);
    s_characterLoadingAnimationEntryId = -1;
}

struct Obj8026A1E0 {
    char data[0x654];
    // Construct owned exhibition storage with its camera distance and viewport.
    Obj8026A1E0(float f, void* src) { CExhibition_Ctor(this, src, f); }
};

#pragma inline_depth(0)
// Copy the requested equipment and merge update flags; defer model changes until preloaders are ready.
// flags is an opaque engine update mask: this unit ORs requests together and
// passes the result through; individual bit meanings are not established here.
// 1 means the request was recorded, including when no queue call succeeds.
// A successful QueueModelUpdate means QUEUED, not yet resolved/installed;
// displayDescriptor is committed later by InstallResolvedSelectionCallback.
int CAsyncExhibitionAvatar_RequestModelUpdate(void* preview, void* source, int flags) {
    // Phase 1: snapshot requested equipment, then merge flags without clearing old requests.
    char* self = (char*)preview;
    // Same raw selection shape as SetEquipment; keep the local copy type for code generation.
    struct Pair { unsigned int a, b; };
    struct Body { Pair preview[12]; };
    struct Copy { unsigned int head; unsigned char bytes[4]; Body body; };
    Copy* copy = (Copy*)(self + kAvatarDescriptorOffset);
    copy->head = *(unsigned int*)source;
    copy->bytes[0] = ((unsigned char*)source)[4];
    copy->bytes[1] = ((unsigned char*)source)[5];
    copy->bytes[2] = ((unsigned char*)source)[6];
    copy->bytes[3] = ((unsigned char*)source)[7];
    copy->body = *(Body*)((char*)source + 8);
    AsCAsyncExhibitionAvatar(self)->pendingUpdateFlags |= flags;
    // Phase 2: only an unready outer resource preloader sets modelUpdatePending.
    void* obj = AsCAsyncExhibitionAvatar(self)->resourcePreloader;
    if (obj && !((ObjVtSlot*)obj)->fn()) {
        AsCAsyncExhibitionAvatar(self)->modelUpdatePending = 1;
        return 1;
    }
    // Phase 3: try parts readiness/reload, then queue. Failed gates leave the
    // merged flags intact but do not set modelUpdatePending on this path.
    do {
        if (AsCAsyncExhibitionAvatar(self)->preloadAvatar) {
            if (!((ObjVtSlot*)AsCAsyncExhibitionAvatar(self)->preloadAvatar)->fn()) break;
            if (CPreloadAvatarAsync_ReloadParts(AsCAsyncExhibitionAvatar(self)->preloadAvatar, self + kAvatarDescriptorOffset, 1) == 0) break;
        }
        {
            void* mgr = CAsyncExhibition_GetExhibition(preview);
            if (mgr) {
                if (CExhibition_QueueModelUpdate(mgr, self + kAvatarDescriptorOffset, AsCAsyncExhibition(self)->displayDescriptor, AsCAsyncExhibitionAvatar(self)->pendingUpdateFlags)) {
                    AsCAsyncExhibitionAvatar(self)->pendingUpdateFlags = 0;
                }
            }
        }
    } while (0);
    return 1;
}
#pragma inline_depth(255)

// Return the exhibition object owned by this asynchronous preview.
void* CAsyncExhibition_GetExhibition(void* preview) {
    return AsCAsyncExhibition(preview)->exhibition;
}

// Replace the exhibition, initialize viewport/camera state and create its loading animation entry.
int CAsyncExhibition_ReplaceExhibition(void* preview, void* viewportRectangle, float cameraDistance, unsigned char previewMode) {
    // Phase 1: store the rectangle/camera mode and destroy the previous owned exhibition.
    char* self = (char*)preview;
    char* src = (char*)viewportRectangle;
    float zero = kFloat_0_AsyncExhibition;
    void* previousExhibition = AsCAsyncExhibition(self)->exhibition;
    AsCAsyncExhibition(self)->viewportLeft = ((PreviewRectangle*)src)->left;
    AsCAsyncExhibition(self)->viewportTop = ((PreviewRectangle*)src)->top;
    AsCAsyncExhibition(self)->viewportRight = ((PreviewRectangle*)src)->right;
    AsCAsyncExhibition(self)->viewportBottom = ((PreviewRectangle*)src)->bottom;
    AsCAsyncExhibition(self)->cameraDistance = cameraDistance;
    AsCAsyncExhibition(self)->previewMode = previewMode;
    AsCAsyncExhibition(self)->previousPointerX = zero;
    AsCAsyncExhibition(self)->previousPointerY = zero;
    if (previousExhibition != 0) {
        if (previousExhibition != 0) {
            ((ObjDel1*)previousExhibition)->destroy(1);
        }
        AsCAsyncExhibition(self)->exhibition = 0;
    }
    // Phase 2: install the new exhibition before clearing its control flag.
    Obj8026A1E0* obj = new Obj8026A1E0(AsCAsyncExhibition(self)->cameraDistance, self + kViewportRectangleOffset);
    void* ptr = obj;
    AsCAsyncExhibition(self)->exhibition = ptr;
    int ok = 0;
    if (ptr == 0) {
        ok = 0;
    } else {
        CExhibition_ClearControlFlag(ptr, AsCAsyncExhibition(self)->previewMode);
        ok = 1;
    }
    if (!ok) {
        return 0;
    }
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    // Phase 3: create the per-preview loading entry only after exhibition creation succeeds.
    CBpCtrl_CreateEntry(Sing_805DC5F0(), asyncExhibitionLoadingScriptName + kLoadingScriptNameOffset, ((char*)&AsCAsyncExhibition(self)->bpEntryId), 1);
    return 1;
}

struct ObjA3C4Child {
    virtual void v0(); virtual void v1();
    virtual int ready();
};
struct ObjA3C4Self {
    virtual void v0(); virtual void v1();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual int ready();
    virtual void process(); virtual void* create(); virtual void v10(); virtual int getValue();
};

#pragma inline_depth(0)
// Install or refresh a ready preloader, attach its callback and hide the preview until loading completes.
// Return 0 if an existing preloader, derived readiness gate or exhibition is
// absent/unready; return 1 after binding (not a guarantee of visible completion).
int CAsyncExhibition_TryInstallPreloadedObject(void* preview) {
    // Phase 1: gate on existing resource readiness and the derived avatar hook.
    void* exhibition = 0;
    char* self = (char*)preview;
    if (AsCAsyncExhibition(self)->preloadObject) {
        void* resourcePreloader = AsCAsyncExhibition(self)->preloadObject;
        if (!((ObjA3C4Child*)resourcePreloader)->ready()) {
            return 0;
        }
    }
    if (!((ObjA3C4Self*)self)->ready()) {
        return 0;
    }
    exhibition = AsCAsyncExhibition(self)->exhibition;
    if (!exhibition) return 0;
    // Phase 2: external ownership refreshes in place; internal ownership detaches,
    // deletes and replaces the old preloader before rebinding it to the exhibition.
    unsigned char reusePreloader = AsCAsyncExhibition(self)->preloaderExternallyOwned;
    if (reusePreloader) {
        ((ObjA3C4Self*)self)->process();
    } else {
        // Internally owned: drop the old preloader first. (The repeated !reusePreloader / o != 0 tests are
        // compiler-expansion residue kept for matching; they are always true here.)
        if (!reusePreloader) {
        if (exhibition != 0) {
            CExhibition_ClearModelPreloader(exhibition);
        }
        void* o = AsCAsyncExhibition(self)->preloadObject;
        if (o != 0) {
            if (o != 0) {
                ((ObjDel1*)o)->destroy(1);
            }
            AsCAsyncExhibition(self)->preloadObject = 0;
        }
        }
        void* res = ((ObjA3C4Self*)self)->create();
        AsCAsyncExhibition(self)->preloadObject = res;
    }
    // Phase 3: bind callback context (borrowed self), hide stale preview content,
    // and reset the loading-indicator latch for the next base update.
    CExhibition_SetModelPreloader(exhibition, AsCAsyncExhibition(self)->preloadObject);
    int callbackValue = ((ObjA3C4Self*)self)->getValue();
    CExhibition_GetCallbackResult(exhibition, callbackValue, self);
    CExhibition_SetVisible(exhibition, 0);
    AsCAsyncExhibition(self)->loadingIndicatorShown = 0;
    return 1;
}
#pragma inline_depth(255)

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046233C", _ctors_u80269E68_0, CAsyncExhibition_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046233C", _ctors_u80269E68_1, CAsyncExhibitionAvatar_InitAngleConstant);
#pragma pop
