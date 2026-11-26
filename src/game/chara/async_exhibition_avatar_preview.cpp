// The asynchronous character preview: the base preview that owns an exhibition,
// the avatar preview with its equipment descriptor and deferred model updates,
// and the shared loading animation and resident resources the previews use.
#include <decomp/static_init.h>
extern "C" char s_sharedFileListDtorRegistration_8026B004[];
extern "C" char s_startupSharedResourceHandles[];
extern "C" char s_taskSingletonRegistrations_80509C30[];
struct PreviewRectangle {
    int left, top, right, bottom;
};
enum {
    kAttachmentSlotCount = 5,
    kLoadingScriptNameOffset = 0x10,
    kCommon2DArchiveNameOffset = 0x0F
};
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
#include "classes/CPreloadAvatarAsync.h"
#include "classes/CPackTextureResource.h"
#include "classes/CPackedFile.h"
#include "classes/CExhibition.h"
#include "classes/CBpCtrl.h"

#define AsCAsyncExhibitionAvatar(p) ((CAsyncExhibitionAvatar*)(p))

#define AsCAsyncExhibition(p) ((CAsyncExhibition*)(p))
extern "C" void appFreeUni__FPv(void*);
namespace app {
template <class T> class app_allocator {};
}
namespace std {
template <class T> class char_traits {};
template <class T, class Traits, class Alloc> class LocalBasicString_u80269E68 {
public:
    unsigned int words[3];
    // Release heap-backed temporary string storage; inline bytes need no disposal.
    ~LocalBasicString_u80269E68() {
        if (words[0] >> 31)
            appFreeUni__FPv((void*)words[2]);
    }
};
}

enum {
    kAvatarDescriptorOffset = (int)&((CAsyncExhibitionAvatar*)0)->avatarDescriptor,
    kViewportRectangleOffset = (int)&((CAsyncExhibition*)0)->viewportLeft
};

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
extern "C" void* __dt__16CAsyncExhibitionFv(void*, int);
extern "C" void __dl__FPv(void*);

extern "C" int ItemSelection_Equals(void*, void*);
extern "C" void ItemSelection_SetStateIfSameCharacter(void*, void*, void*);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void ItemSelection_Init(void*);
extern "C" char __vt__22CAsyncExhibitionAvatar[];

extern "C" const float kFloat_6_2831855;
extern float s_asyncExhibitionAngleConstant;
extern float s_asyncAvatarExhibitionAngleConstant;
extern "C" const float kFloat_3_AvatarPreviewParameterStorage;
extern "C" void CBpCtrl_dtor(void*, int);
extern "C" void* __register_global_object(void*, void*, void*);
extern "C" void* IdMap_Lookup(void*, int);
extern "C" void BpObject_TryInvokeCurrentCallback(void*, int);
extern "C" void ResolvedItemSelection_Clear(void*);
extern "C" void Memory_Clear(void*, int);
extern "C" void Memory_Copy(void*, void*, int);
extern "C" void __ct__11CExhibitionFPif(void*, void*, float);
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
extern "C" void* ResourceTable_FindEntryByName(int, const char*, int*);
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
int CAsyncExhibition::ToggleAttachment(unsigned int index, void* value) {
    if (index >= kAttachmentSlotCount)
        return 0;
    void* manager = exhibition;
    if (manager == 0)
        return 0;
    int slot = (int)index;
    if (slot >= kAttachmentSlotCount)
        slot = 0;
    if (value == attachments[slot])
        value = 0;
    int result = ((CExhibition*)manager)->SetAttachment((int)index, value);
    if (result != 0) {
        if ((int)index >= kAttachmentSlotCount)
            index = 0;
        attachments[index] = value;
    }
    return result;
}

// Apply a texture variant and update the resolved selection only after success.
int CAsyncExhibition::ApplyTextureVariant(void* source, void* arg) {
    void* manager = exhibition;
    if (manager == 0)
        return 0;
    // Snapshot the raw selection before applying a variant; see the copy contract above.
    struct Pair {
        unsigned int a, b;
    };
    struct Body {
        Pair preview[12];
    };
    struct Copy {
        unsigned int head;
        unsigned char bytes[4];
        Body body;
    } copy;
    copy.head = *(unsigned int*)source;
    copy.bytes[0] = ((unsigned char*)source)[4];
    copy.bytes[1] = ((unsigned char*)source)[5];
    copy.bytes[2] = ((unsigned char*)source)[6];
    copy.bytes[3] = ((unsigned char*)source)[7];
    copy.body = *(Body*)((char*)source + 8);
    int result = ((CExhibition*)manager)->ApplyTextureVariant(source, (int)arg);
    if (result != 0)
        ItemSelection_SetStateIfSameCharacter(displayDescriptor, &copy, arg);
    return result;
}

struct CAsyncExhibitionView {
    CAsyncExhibitionView();
    ~CAsyncExhibitionView();
    char pad[0x124];
};

#pragma inline_depth(0)
// Construct the base preview and select the avatar interface before member initialization.
CAsyncExhibitionView::CAsyncExhibitionView() {
    ((CAsyncExhibition*)this)->__ct();
    *(void**)this = __vt__22CAsyncExhibitionAvatar;
}
// Destroy base-owned preview resources without deleting this enclosing allocation.
CAsyncExhibitionView::~CAsyncExhibitionView() {
    __dt__16CAsyncExhibitionFv(this, 0);
}
#pragma inline_depth(255)

struct CAsyncExhibitionAvatarView : CAsyncExhibitionView {
    struct Member {
        Member();
        char pad[0x68];
    };
    Member member;
    int pendingUpdateFlags;
    unsigned char avatarLoadPending;
    unsigned char modelUpdatePending;
    int preloadAvatar;
    CAsyncExhibitionAvatarView();
};

// Initialize the requested equipment descriptor subobject.
CAsyncExhibitionAvatarView::Member::Member() {
    ItemSelection_Init(this);
}

// Start with no deferred requests and no avatar-parts preloader.
CAsyncExhibitionAvatarView::CAsyncExhibitionAvatarView() {
    pendingUpdateFlags = 0;
    avatarLoadPending = 0;
    modelUpdatePending = 0;
    preloadAvatar = 0;
}

struct ObjA75CVt {
    virtual void destroy(int);
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual int update(void*, int, float);
};

// Allocate the avatar preview; destroy it if virtual initialization fails.
CAsyncExhibitionAvatar* CAsyncExhibitionAvatar::Create(void* viewport, int mode, float scale) {
    CAsyncExhibitionAvatarView* object = new CAsyncExhibitionAvatarView();
    if (object == 0)
        return 0;
    ObjA75CVt* x = (ObjA75CVt*)object;
    int upd = x->update(viewport, mode, scale);
    if (upd == 0) {
        if (object != 0) {
            x->destroy(1);
        }
        return 0;
    }
    return (CAsyncExhibitionAvatar*)object;
}
struct ObjA9C4Vt {
    virtual void v0();
    virtual void v1();
    virtual int fn();
};

// Store a changed equipment selection and attachments, deferring installation until preloading is ready.
// source borrows a 0x68-byte ItemSelection; value borrows five attachment pointers
// or is null to clear them. 0 means unchanged OR immediate install failed;
// 1 can mean deferred acceptance, not that a visible model has been installed.
int CAsyncExhibitionAvatar::SetEquipment(void* source, void* value) {
    if (ItemSelection_Equals(avatarDescriptor, source))
        return 0;
    modelUpdatePending = 0;
    pendingUpdateFlags = 0;
    // Same raw requested-selection shape; no resolved display data or attachments inside it.
    struct Pair {
        unsigned int a, b;
    };
    struct Body {
        Pair preview[12];
    };
    struct Copy {
        unsigned int head;
        unsigned char bytes[4];
        Body body;
    };
    Copy* copy = (Copy*)((char*)this + kAvatarDescriptorOffset);
    copy->head = *(unsigned int*)source;
    copy->bytes[0] = ((unsigned char*)source)[4];
    copy->bytes[1] = ((unsigned char*)source)[5];
    copy->bytes[2] = ((unsigned char*)source)[6];
    copy->bytes[3] = ((unsigned char*)source)[7];
    copy->body = *(Body*)((char*)source + 8);
    if (value)
        Memory_Copy(((CAsyncExhibition*)this)->attachments, value, sizeof(((CAsyncExhibition*)this)->attachments));
    else
        Memory_Clear(((CAsyncExhibition*)this)->attachments, sizeof(((CAsyncExhibition*)this)->attachments));
    void* obj = resourcePreloader;
    if (obj && !((ObjA9C4Vt*)obj)->fn()) {
        avatarLoadPending = 1;
        return 1;
    }
    return CAsyncExhibition::TryInstallPreloadedObject(this);
}

struct ObjVtSlot {
    virtual void v0();
    virtual void v1();
    virtual int fn();
};

#pragma inline_depth(0)
// Process the preview and complete deferred avatar/parts updates when their preloaders finish.
// A true return means the base preview was processed. It does not imply that
// either pending request reached a visible/install-complete state this frame.
int CAsyncExhibitionAvatar::UpdateDeferredRequests(float delta) {
    // Phase 1: base update also controls visibility and the loading indicator.
    if (!((CAsyncExhibition*)this)->UpdateLoadingStateAndIndicator(delta))
        return 0;
    // Phase 2: a ready resource preloader consumes the deferred install attempt.
    // The pending byte is cleared regardless of TryInstall's return value.
    if (avatarLoadPending) {
        void* obj = resourcePreloader;
        if (obj && ((ObjVtSlot*)obj)->fn()) {
            CAsyncExhibition::TryInstallPreloadedObject(this);
            avatarLoadPending = 0;
        }
    }
    // Phase 3: try the parts reload, then queue the merged update flags. Only a
    // successful queue clears pendingUpdateFlags. modelUpdatePending is cleared
    // once the outer preloader is ready even if parts/mgr/queue are not ready;
    // it is NOT a retry-until-installed flag. Preserve these distinct commits.
    if (modelUpdatePending) {
        void* obj = resourcePreloader;
        if (obj && ((ObjVtSlot*)obj)->fn()) {
            if (!preloadAvatar || (((ObjVtSlot*)preloadAvatar)->fn() &&
                                   ((CPreloadAvatarAsync*)preloadAvatar)->ReloadParts(avatarDescriptor, 1))) {
                void* mgr = ((CAsyncExhibition*)this)->GetExhibition();
                if (mgr && ((CExhibition*)mgr)
                               ->QueueModelUpdate(avatarDescriptor, AsCAsyncExhibition(this)->displayDescriptor,
                                                  pendingUpdateFlags))
                    pendingUpdateFlags = 0;
            }
            modelUpdatePending = 0;
        }
    }
    return 1;
}
#pragma inline_depth(255)

// Initialize viewport interaction, resolved selection, attachments and loading-indicator state.
CAsyncExhibition::CAsyncExhibition() {
    cameraDistance = kFloat_0_AsyncExhibition;
    *(void**)this = __vt__16CAsyncExhibition;
    previewMode = 0;
    (*(int*)&preloadObject) = 0;
    preloaderExternallyOwned = 0;
    ResolvedItemSelection_Clear(displayDescriptor);
    Memory_Clear(attachments, sizeof(attachments));
    float h;
    float w;
    float z;
    z = kFloat_0_AsyncExhibition;
    w = kFloat_325_AsyncExhibitionWidth;
    h = kFloat_230_AsyncExhibitionHeightStorage;
    dragStartDepth = z;
    pointerDragActive = 0;
    (*(int*)&exhibition) = 0;
    bpEntryId = -1;
    loadingIndicatorShown = 0;
    loadingIndicatorX = w;
    loadingIndicatorY = h;
}

// Create the shared character-loading animation once, using -1 as the absent-entry sentinel.
void CharacterLoading_CreateAnimationEntry() {
    if (s_characterLoadingAnimationEntryId == -1) {
        if (Sing_805DC5F0() == 0) {
            if (s_bpCtrlInitialized == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        ((CBpCtrl*)Sing_805DC5F0())
            ->CreateEntry(asyncExhibitionLoadingScriptName + kLoadingScriptNameOffset,
                          &s_characterLoadingAnimationEntryId, 0);
    }
}

// Add the supplied Euler-angle offset to the active preview rotation.
int CAsyncExhibition::OffsetRotation(Vec3f* offset) {
    void* obj = exhibition;
    if (obj == 0)
        return 0;
    float* pos = (float*)((CExhibition*)obj)->GetRotation();
    Vec3f adjusted = *(Vec3f*)pos;
    adjusted.x += offset->x;
    adjusted.y += offset->y;
    adjusted.z += offset->z;
    ((CExhibition*)obj)->SetRotation(&adjusted.x);
    return 1;
}

struct ObjACF8 {
    char pad[0x20];
    // Initialize temporary texture-resource storage using its resolved table entry.
    ObjACF8(const char* path, int local, int n) {
        ((CPackTextureResource*)this)->Init((const char*)path, (void**)local, (unsigned int)n);
    }
};

struct ObjDel1 {
    virtual void destroy(int);
};

// Lazily construct and register the resident shared-file list singleton (Sing_805DC680), then return it.
static void* getSharedFileList(char* registrations) {
    if (((int&)Sing_805DC680()) == 0) {
        if (!s_sharedFileListInitialized) {
            SharedFilePointerVector_InitReserved1024(s_sharedFileListStorage);
            __register_global_object(s_sharedFileListStorage, (void*)SharedFilePointerVector_Destroy,
                                     (registrations + kRegistrationSharedFileList));
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
    void* x = CPackedFile::Create(startupArchiveAndWarningStrings);
    g_residentEffectArchive = (int)x;
    int local = 0;
    if (((int&)Sing_805DC620()) == 0) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object((void*)&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     registrations + kRegistrationResourceTable);
            s_resourceTableInitialized = 1;
        }
        ((int&)Sing_805DC620()) = (int)&s_resourceTableStorage;
    }
    int n = (int)ResourceTable_FindEntryByName(((int&)Sing_805DC620()),
                                               startupArchiveAndWarningStrings + kCommon2DArchiveNameOffset, &local);
    ObjACF8* object = new ObjACF8(startupArchiveAndWarningStrings + kCommon2DArchiveNameOffset, local, n);
    // Phase 2: synchronously poll both required resources before effect preloading.
    while (!((CPackedFile*)g_residentEffectArchive)->IsLoadFinished())
        ((CPackedFile*)g_residentEffectArchive)->PollLoad();
    while (!((CPackTextureResource*)object)->IsLoadFinished())
        ((CPackTextureResource*)object)->PollLoad();
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
        for (unsigned int i = 0; i < kSharedFileNameCount; ++i)
            SharedFileList_CreateAndAppend(getSharedFileList(registrations), (void*)table[i], 0);
    }
    {
        int* table = (int*)sharedGolfUiArchiveNames;
        for (unsigned int i = 0; i < kSharedGolfUiArchiveCount; ++i)
            SharedFileList_CreateAndAppend(getSharedFileList(registrations), (void*)table[i], 0);
    }
    for (unsigned int i = 0; i < 1; ++i) {
        std::LocalBasicString_u80269E68<char, std::char_traits<char>, app::app_allocator<char> > args;
        args.words[0] = 0;
        args.words[1] = 0;
        args.words[2] = 0;
        NString_AssignCString(&args, sharedLabelResourceNames);
        SharedFileList_CreateAndAppend(
            getSharedFileList(registrations),
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
        if (object)
            ((ObjDel1*)object)->destroy(1);
    }
}
#pragma pop

// Report whether a deferred avatar-install attempt remains scheduled.
unsigned char CAsyncExhibitionAvatar::GetDeferredInstallAttemptPending() {
    return avatarLoadPending;
}

// Return the callback that installs resolved item selections in the exhibition.
void* CAsyncExhibition::vfunc_9() {
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
void* CAsyncExhibition::SetViewport(void* rectangle, float amount) {
    void* x = exhibition;
    if (x != 0) {
        return (void*)((CExhibition*)x)->SetViewport((int*)rectangle, amount);
    }
    return 0;
}

// Forward camera distance limits when an exhibition exists.
int CAsyncExhibition::SetCameraDistanceLimits(float minimumDistance, float maximumDistance) {
    void* x = exhibition;
    if (x != 0) {
        return ((CExhibition*)x)->SetCameraDistanceLimits(minimumDistance, maximumDistance);
    }
    return 0;
}

// Forward camera rotation limits when an exhibition exists.
int CAsyncExhibition::SetRotationLimits(float minimumRotation, float maximumRotation) {
    void* x = exhibition;
    if (x != 0) {
        return ((CExhibition*)x)->SetRotationLimits(minimumRotation, maximumRotation);
    }
    return 0;
}

// Set preview rotation and report failure when no exhibition exists.
int CAsyncExhibition::SetRotation(void* vec) {
    void* x = exhibition;
    if (x == 0) {
        return 0;
    }
    ((CExhibition*)x)->SetRotation((float*)vec);
    return 1;
}

// Set vertical viewport limits and report failure when no exhibition exists.
int CAsyncExhibition::SetViewportYLimits(unsigned int minimumYBits, unsigned int maximumYBits) {
    void* x = exhibition;
    if (x == 0) {
        return 0;
    }
    ((CExhibition*)x)->SetViewportYLimits(minimumYBits, maximumYBits);
    return 1;
}

// Create the equipment preloader for the stored avatar descriptor.
int CAsyncExhibitionAvatar::CreateAvatarPreloader(void* preview) {
    void* self = preview;
    CPreloadAvatarAsync* preloader =
        CPreloadAvatarAsync::Create(AsCAsyncExhibitionAvatar(self)->avatarDescriptor, 0, 0, 1);
    AsCAsyncExhibitionAvatar(self)->preloadAvatar = preloader;
    return (int)preloader;
}

#pragma inline_depth(0)
// Configure the exhibition from this avatar descriptor and its attachments.
int CAsyncExhibitionAvatar::ConfigureAvatarPreview(void* preview) {
    void* self = preview;
    void* x = ((CAsyncExhibition*)preview)->GetExhibition();
    ((CExhibition*)x)
        ->ConfigureAvatarPreview(AsCAsyncExhibitionAvatar(self)->avatarDescriptor,
                                 AsCAsyncExhibition(self)->attachments, 0, 0, kFloat_3_AvatarPreviewParameterStorage);
    return 1;
}
#pragma inline_depth(255)

// Destroy the avatar preview. The compiler adds the null guard and frees the
// allocation when the deleting destructor is used.
CAsyncExhibitionAvatar::~CAsyncExhibitionAvatar() {
    __dt__16CAsyncExhibitionFv(this, 0);
}

struct ObjAC88 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void fn(void*, int, int, int);
};

// Restart the existing avatar preloader with the current equipment descriptor.
int CAsyncExhibitionAvatar::RestartAvatarPreloader(void* preview) {
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
    ((CExhibition*)x)->CopyResolvedAvatarSelection(AsCAsyncExhibition(obj)->displayDescriptor);
    ((CExhibition*)x)->GetCallbackResult(0, 0);
    return 1;
}

struct ObjA338 {
    virtual void v0();
    virtual void v1();
    virtual int fn();
};

// Play a named animation only when resource preloading is complete and an exhibition exists.
int CAsyncExhibition::PlayFixedNameAnimation(void* preview, void* q) {
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
    ((CExhibition*)y)->PlayAnimationByName(arg, 0, kFloat_0_AsyncExhibition);
    return 1;
}

// Invoke the shared loading-animation entry callback.
void CharacterLoading_InvokeAnimationCallback() {
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            char* base = s_bpCtrl;
            ((CBpCtrl*)base)->__ct();
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
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById(s_characterLoadingAnimationEntryId);
    s_characterLoadingAnimationEntryId = -1;
}

struct CExhibitionStorage {
    char data[0x654];
    // Construct owned exhibition storage with its camera distance and viewport.
    CExhibitionStorage(float f, void* src) {
        __ct__11CExhibitionFPif(this, src, f);
    }
};

#pragma inline_depth(0)
// Copy the requested equipment and merge update flags; defer model changes until preloaders are ready.
// flags is an opaque engine update mask: this unit ORs requests together and
// passes the result through; individual bit meanings are not established here.
// 1 means the request was recorded, including when no queue call succeeds.
// A successful QueueModelUpdate means QUEUED, not yet resolved/installed;
// displayDescriptor is committed later by InstallResolvedSelectionCallback.
int CAsyncExhibitionAvatar::RequestModelUpdate(void* preview, void* source, int flags) {
    // Phase 1: snapshot requested equipment, then merge flags without clearing old requests.
    char* self = (char*)preview;
    // Same raw selection shape as SetEquipment; keep the local copy type for code generation.
    struct Pair {
        unsigned int a, b;
    };
    struct Body {
        Pair preview[12];
    };
    struct Copy {
        unsigned int head;
        unsigned char bytes[4];
        Body body;
    };
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
            if (!((ObjVtSlot*)AsCAsyncExhibitionAvatar(self)->preloadAvatar)->fn())
                break;
            if (((CPreloadAvatarAsync*)(AsCAsyncExhibitionAvatar(self)->preloadAvatar))
                    ->ReloadParts(self + kAvatarDescriptorOffset, 1) == 0)
                break;
        }
        {
            void* mgr = ((CAsyncExhibition*)preview)->GetExhibition();
            if (mgr) {
                if (((CExhibition*)mgr)
                        ->QueueModelUpdate(self + kAvatarDescriptorOffset, AsCAsyncExhibition(self)->displayDescriptor,
                                           AsCAsyncExhibitionAvatar(self)->pendingUpdateFlags)) {
                    AsCAsyncExhibitionAvatar(self)->pendingUpdateFlags = 0;
                }
            }
        }
    } while (0);
    return 1;
}
#pragma inline_depth(255)

// Return the exhibition object owned by this asynchronous preview.
void* CAsyncExhibition::GetExhibition() {
    return exhibition;
}

// Replace the exhibition, initialize viewport/camera state and create its loading animation entry.
int CAsyncExhibition::ReplaceExhibition(void* preview, void* viewportRectangle, float cameraDistance,
                                        unsigned char previewMode) {
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
    CExhibitionStorage* obj =
        new CExhibitionStorage(AsCAsyncExhibition(self)->cameraDistance, self + kViewportRectangleOffset);
    void* ptr = obj;
    AsCAsyncExhibition(self)->exhibition = ptr;
    int ok = 0;
    if (ptr == 0) {
        ok = 0;
    } else {
        ((CExhibition*)ptr)->ClearControlFlag(AsCAsyncExhibition(self)->previewMode);
        ok = 1;
    }
    if (!ok) {
        return 0;
    }
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_asyncExhibitionBpCtrlDtorRegistration);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    // Phase 3: create the per-preview loading entry only after exhibition creation succeeds.
    ((CBpCtrl*)Sing_805DC5F0())
        ->CreateEntry(asyncExhibitionLoadingScriptName + kLoadingScriptNameOffset,
                      ((char*)&AsCAsyncExhibition(self)->bpEntryId), 1);
    return 1;
}

struct ObjA3C4Child {
    virtual void v0();
    virtual void v1();
    virtual int ready();
};
struct ObjA3C4Self {
    virtual void v0();
    virtual void v1();
    virtual void v4();
    virtual void v5();
    virtual void v6();
    virtual int ready();
    virtual void process();
    virtual void* create();
    virtual void v10();
    virtual int getValue();
};

#pragma inline_depth(0)
// Install or refresh a ready preloader, attach its callback and hide the preview until loading completes.
// Return 0 if an existing preloader, derived readiness gate or exhibition is
// absent/unready; return 1 after binding (not a guarantee of visible completion).
int CAsyncExhibition::TryInstallPreloadedObject(void* preview) {
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
    if (!exhibition)
        return 0;
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
                ((CExhibition*)exhibition)->ClearModelPreloader();
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
    ((CExhibition*)exhibition)->SetModelPreloader((int)AsCAsyncExhibition(self)->preloadObject);
    int callbackValue = ((ObjA3C4Self*)self)->getValue();
    ((CExhibition*)exhibition)->GetCallbackResult(callbackValue, self);
    ((CExhibition*)exhibition)->SetVisible(0);
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
