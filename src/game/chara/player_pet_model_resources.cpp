// Player and pet model resources: puppet layers, opacity and lighting, the equipment part lists
// and their reloads, and the registry of missing part resources.

#include <decomp/local_class.h>
#include <decomp/ct_member.h>
#include "iff/IffStruct.h"
#include "classes/WPuppet.h"
#include "classes/CSharedDoc.h"
#include "classes/ResourceModel801B.h"
#include "classes/ItemSelection801B7688.h"
struct PetModelStringPool {
    char baseModelFormat[8];
    char animationModelFormat[8];
    char animationExtension[6];
    char menuExtension[11];
    char swingExtension[12];
    char storyExtension[12];
    char commendationExtension[19];
    char faceVariantFormat[6];
    char stringOutOfRange[27];
    char outOfRangeType[55];
    char partModelFormat[8];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char spineBone[12];
    char groupFilenameFormat[9];
    char replacementMessage[17];
    char reserveLengthError[35];
    char stringLengthError[27];
};
typedef char PetStrings_spine_offset[((int)&((PetModelStringPool*)0)->spineBone == 0xF7) ? 1 : -1];
enum ModelReleaseMask {
    kReleaseBasePuppet = 1,
    kReleaseAnimationPuppets = 2,
    kReleasePartResources = 4,
    kReleaseAllResources = 7
};
enum ModelSuffixOffset {
    kMenuSuffix = (int)&((PetModelStringPool*)0)->menuExtension,
    kSwingSuffix = (int)&((PetModelStringPool*)0)->swingExtension,
    kStorySuffix = (int)&((PetModelStringPool*)0)->storyExtension,
    kCommendationSuffix = (int)&((PetModelStringPool*)0)->commendationExtension
};
enum ModelAnimationContext {
    kMenuAnimation = 0,
    kSwingAnimation = 2,
    kStoryAnimation = 3,
    kCommendationAnimation = 4
};
enum {
    kMaximumPartSlots = 24,
    kTexturesPerPart = 3,
    kTextureResourceNameLength = 40,
    kMaximumResourceVectorEntries = 0x3FFFFFFFu,
    kConcatAllocatorByteOffset = 0x14,
    kAppendAllocatorByteOffset = 0xC,
    kPartModelFormatOffset = (int)&((PetModelStringPool*)0)->partModelFormat
};
struct AvatarSelectionView {
    char modelState[0x1BC];
    ItemSelection801B7688 selection;
};
struct CharacterSelectionHeader {
    unsigned int characterId;
    unsigned char primaryCount, totalCount, textureVariant, bitmapVariant;
};
struct ResourcePairNode {
    char resourceHeader[0x14];
    void* pairedResource;
};
struct ModelMissingResourceRegistry {
    char registryHeader[0x4C];
    void* missingResources;
};
struct NarrowStringView {
    unsigned int flags;
    unsigned int length;
    char* data;
};

namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char petModelResourceStrings[];
}
}

#include "game/string.h"
#include "game/exception.h"
#include "classes/CString.h"

extern "C" {

extern CSharedDoc* g_sharedDoc;
extern void* ItemInventory_FindCharacterData(void*, void*);
extern void* WPuppet_ReplaceChildAndLayerAnimation(void*, void*, void*, void*, void*);
extern void* Object_GetChild(void*);

extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);
struct InstanceLock_u801B4F20 {
    void* lock;
    // Lock the supplied mutex for this scope.
    InstanceLock_u801B4F20(void* l) : lock(l) {
        OSLockMutex(lock);
    }
    // Unlock the mutex acquired for this scope.
    ~InstanceLock_u801B4F20() {
        OSUnlockMutex(lock);
    }
};

extern void* Pointer_GetFirstPointer(void*);
extern void* ItemSelection_ResolvePartData(void*, void*);
extern void* SharedDoc_GetItemInventory();
extern void* ItemInventory_FindCharacterData(void*, void*);
extern int sprintf(char*, const char*, ...);
extern void* MWString_ConstructCString(void*, void*);
extern int String_FindLastOfCStr(void*, const char*, int);
extern void* String_ConstructSubstring(void*, void*, int, int);
extern void String_AssignWithInlineFastPath(void*, void*);
extern void* MWString_GetCString(void*);
extern void AvatarModel_ReloadPartResources(void*, unsigned int);
extern void WPuppet_RefreshMatrixBuffers(void*);
extern void WPuppet_RebuildBoneController(void*);
extern void WPuppet_InitializeTextureCoordinateAnimation(void*);
extern void* ResourceManager_LoadBitmap(void*, const char*, int, int, char*);
extern void* ResourceManager_FindByNormalizedBasename(void*, const char*);
extern void WPuppet_ApplyNamedResourcePairRecursive(void*, void*, void*, int);
extern void AvatarModel_RefreshCoveredParts(void*);
extern void AvatarModel_ApplyTextureVariant(void*, void*, int);
extern void CharacterModel_ApplyBitmapVariant(void*, int, int);
extern void* g_resourceManager;
extern void* Pointer_GetFirstPointer(void*);
extern char* strcpy(char*, const char*);

struct PuppetHandleView {
    char headerStorage[8];
    void* puppet;
};

struct PuppetOwnerView {
    char headerStorage[8];
    void* puppet;
};

extern void* WPuppet_SetOpacityRecursive(void*, unsigned int);
extern void WPuppet_SetBoneVisibilityRecursive(void*, int, int, const char*);
extern void AvatarModel_ApplyPartHideMasks(void*, int);
extern int String_CompareCString(void*, void*);
extern int WPuppet_FindAnimationRangeByName(void*, void*);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, void*, void*, char*);
extern unsigned int strlen(const char*);

struct PartModelListView {
    char headerStorage[0x30];
    int count;
    char selectionState[4];
    void** entries;
};

struct AnimationPuppetListView {
    char headerStorage[0x1c];
    void** entries;
    int count;
    char listState[4];
    void* selected;
};

struct ModelResourcePresenceView {
    char headerStorage[8];
    int puppet;
    char objectState[0x14];
    int resourceCount;
    char listState[0x14];
    int fields[3][8];
};

extern void* WPuppet_FindNamedBoneEntry(void*, void*);
extern void Matrix_ExpandAffineTo4x4(void*, void*);
extern void* ModelRenderParams_GetShared();
extern void ModelRenderParams_Copy(void*, void*);
extern void Mtx44_TransformDirection(void*, void*, void*);
extern void Mtx44_SetRotationY(void*, float);
extern void Vec3_Normalize(void*, void*);
extern void WPuppet_SetRenderParams(void*, void*);
extern void WPuppet_SetRenderParamsEnabled(void*, int);
extern float kModelLightingNegativeQuarterTurnY;
extern char petModelResourceStrings[];
extern void* g_resourceManager;
extern void ResourceManager_ReleaseObject(void*, void*, int);
extern void ResourceManager_ReleaseId(void*, void*);
extern void* memset(void*, int, unsigned int);

struct PlayerModelResourcesView {
    char headerStorage[4];
    int characterKey;
    void* puppet;
    void* auxiliaryTexture0;
    void* auxiliaryTexture1;
    void* auxiliaryTexture2;
    void* faceTexture;
    void** resourceList;
    int resourceListCount;
    char listState[8];
    int primaryPartCount;
    int partCount;
    char selectionState[4];
    void* ptr38;
    char partModelStorage[0x5c];
    void* nested[0x48];
};

#pragma use_lmw_stmw on
// Release selected resources; clear base/part handles and the animation-list count.
// Released animation-list slots remain outside the active count and are not nulled.
void PlayerModel_ReleaseResourcesByMask(PlayerModelResourcesView* p, unsigned int flags) {
    if (flags & kReleaseBasePuppet) {
        p->characterKey = 0;
        if (g_resourceManager != 0 && 0 != p->puppet) {
            ResourceManager_ReleaseObject(g_resourceManager, p->puppet, 0);
            p->puppet = 0;
        }
    }
    if (flags & kReleaseAnimationPuppets) {
        void** savedList = p->resourceList;
        void** entry = savedList;
        while (entry != p->resourceList + p->resourceListCount) {
            if (*entry != 0)
                ResourceManager_ReleaseObject(g_resourceManager, *entry, 0);
            ++entry;
        }
        int savedListCount = p->resourceListCount;
        p->resourceListCount = p->resourceListCount - savedListCount;
    }
    if (kReleasePartResources & flags) {
        void** nested = ((ResourceModel801B*)p)->partTextures;
        char* record = (char*)p;
        int i = 0;
        while (p->partCount > i) {
            if (g_resourceManager) {
                if (((ResourceModel801B*)record)->partModels[0] != 0) {
                    ResourceManager_ReleaseObject(g_resourceManager, ((ResourceModel801B*)record)->partModels[0], 0);
                    ((ResourceModel801B*)record)->partModels[0] = 0;
                }
            }
            int j = 0;
            void** np = nested;
            while (3 > j) {
                if (0 != g_resourceManager) {
                    if (*np != 0) {
                        ResourceManager_ReleaseId(g_resourceManager, *np);
                        *np = 0;
                    }
                }
                j += 1;
                np = (np + 1);
            }
            record = record + 4;
            nested += 3;
            ++i;
        }
        if (p->partCount) {
            memset(((ResourceModel801B*)p)->partModels, 0, 0x60);
            memset(((ResourceModel801B*)p)->partTextures, 0, 0x120);
            p->partCount = 0;
            p->primaryPartCount = 0;
        }
        if (g_resourceManager != 0) {
            if (0 != p->faceTexture) {
                ResourceManager_ReleaseId(g_resourceManager, p->faceTexture);
                p->faceTexture = 0;
            }
        }
        if (0 != g_resourceManager && 0 != p->auxiliaryTexture0) {
            ResourceManager_ReleaseId(g_resourceManager, p->auxiliaryTexture0);
            p->auxiliaryTexture0 = 0;
        }
        if (g_resourceManager != 0) {
            if (0 != p->auxiliaryTexture2) {
                ResourceManager_ReleaseId(g_resourceManager, p->auxiliaryTexture2);
                p->auxiliaryTexture2 = 0;
            }
        }
        if (g_resourceManager) {
            if (0 != p->auxiliaryTexture1) {
                ResourceManager_ReleaseId(g_resourceManager, p->auxiliaryTexture1);
                p->auxiliaryTexture1 = 0;
            }
        }
        p->characterKey = 0;
    }
}

// Check whether any loaded part puppet has the requested resource name.
int ResourceModel_HasPartModelNamed(PartModelListView* model, void* resourceName) {
    char* partCursor = (char*)model;
    int i = 0;
    while (i < model->count) {
        void* entry = ((ResourceModel801B*)partCursor)->partModels[0];
        if (entry != 0 && String_CompareCString(((WPuppet*)entry)->resourceName, resourceName) == 0)
            return 1;
        partCursor += 4;
        ++i;
    }
    return 0;
}

#pragma use_lmw_stmw on
// Find an animation across loaded puppets and optionally select/return its owner.
int ObjectList_FindAnimationRangeByName(AnimationPuppetListView* models, void* animationName, int setSelected,
                                        void** matchedPuppet) {
    void** entryPtr = models->entries;
    while (entryPtr != models->entries + models->count) {
        int result = WPuppet_FindAnimationRangeByName(*entryPtr, animationName);
        if (result) {
            if (matchedPuppet != 0)
                *matchedPuppet = *entryPtr;
            if (setSelected != 0)
                models->selected = *entryPtr;
            return result;
        }
        ++entryPtr;
    }
    return 0;
}

#pragma opt_propagation off
// Require a base puppet, a nonempty resource list, and at least one loaded part model.
int ResourceModel801B_HasModelAndParts(ModelResourcePresenceView* p) {
    char* base = (char*)p;
    int any = 0;
    int unused = 0;
    int i = 0;
    int ctr = 3;
    (void)i;
    (void)ctr;
    for (int k = 0; k < 3; ++k) {
        if ((*(int*)&((ResourceModel801B*)base)->partModels[0]) != 0) {
            any = 1;
            break;
        }
        if ((*(int*)&((ResourceModel801B*)base)->partModels[1]) != 0) {
            any = 1;
            break;
        }
        if ((*(int*)&((ResourceModel801B*)base)->partModels[2]) != 0) {
            any = 1;
            break;
        }
        if ((*(int*)&((ResourceModel801B*)base)->partModels[3]) != 0) {
            any = 1;
            break;
        }
        if ((*(int*)&((ResourceModel801B*)base)->partModels[4]) != 0) {
            any = 1;
            break;
        }
        if ((*(int*)&((ResourceModel801B*)base)->partModels[5]) != 0) {
            any = 1;
            break;
        }
        if ((*(int*)&((ResourceModel801B*)base)->partModels[6]) != 0) {
            any = 1;
            break;
        }
        if ((*(int*)&((ResourceModel801B*)base)->partModels[7]) != 0) {
            any = 1;
            break;
        }
        base += 0x20;
        unused += 7;
    }
    int result = 0;
    if (any) {
        if (((ResourceModel801B*)p)->entryCount != 0) {
            if ((*(int*)&((ResourceModel801B*)p)->model) != 0) {
                result = 1;
            }
        }
    }
    return result + (unused - unused);
}

#pragma use_lmw_stmw off
// Rotate preview lighting into the spine-bone frame, then apply the supplied light parameters.
void PlayerModel_ApplyBoneRelativeLighting(void* model, void* lightSource, void* lightColor, void* ambientColor) {
    // Keep byte storage so the original aggregate-copy loop is retained.
    struct Blk {
        char bytes[0x68];
    };
    struct LightingDirectionView {
        char materialState[0x40];
        float lightDirection[3];
    };
    enum {
        kLightDirectionOffset = (int)&((LightingDirectionView*)0)->lightDirection
    };
    struct BoneTransform {
        char boneState[0x78];
        float worldMatrix[12];
    };
    struct NamedBone {
        BoneTransform* bone;
        void* metadata;
    };
    char quarterTurnY[0x40];
    char boneWorldMatrix[0x40];
    Blk sharedLighting;
    Blk boneLighting;
    void* resource = WPuppet_FindNamedBoneEntry(((ResourceModel801B*)model)->model,
                                                ((PetModelStringPool*)petModelResourceStrings)->spineBone);
    Matrix_ExpandAffineTo4x4(((NamedBone*)resource)->bone->worldMatrix, boneWorldMatrix);
    ModelRenderParams_Copy(&sharedLighting, ModelRenderParams_GetShared());
    boneLighting = sharedLighting;
    Mtx44_TransformDirection(boneLighting.bytes + kLightDirectionOffset, boneLighting.bytes + kLightDirectionOffset,
                             boneWorldMatrix);
    Mtx44_SetRotationY(quarterTurnY, kModelLightingNegativeQuarterTurnY);
    Mtx44_TransformDirection(boneLighting.bytes + kLightDirectionOffset, boneLighting.bytes + kLightDirectionOffset,
                             quarterTurnY);
    Vec3_Normalize(boneLighting.bytes + kLightDirectionOffset, boneLighting.bytes + kLightDirectionOffset);
    WPuppet_SetRenderParams(((ResourceModel801B*)model)->model, boneLighting.bytes);
    WPuppet_SetRenderParamsEnabled(((ResourceModel801B*)model)->model, 1);
    struct V6 {
        virtual void v0() DECOMP_LOCAL_VIRTUAL;
        virtual void v1() DECOMP_LOCAL_VIRTUAL;
        virtual void v2() DECOMP_LOCAL_VIRTUAL;
        virtual void v3(void*, void*, void*) DECOMP_LOCAL_VIRTUAL;
        virtual void v4(void*, void*, void*) DECOMP_LOCAL_VIRTUAL;
    };
    ((*(V6**)&((ResourceModel801B*)model)->model))->v3(lightSource, lightColor, ambientColor);
}

#pragma use_lmw_stmw on
// Construct a narrow string from a string prefix followed by a C-string suffix.
void String_ConstructConcatCStr(void** p, void* stringObj, const char* text) {
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    unsigned int textLen = strlen(text);
    unsigned int header = *(unsigned int*)stringObj;
    unsigned char len;
    unsigned int prefixLen;
    char* begin;
    if (((header >> 31) & 1) == 0) {
        len = *(unsigned char*)stringObj;
        begin = (char*)stringObj + 1;
        prefixLen = len & 0x7f;
    } else {
        begin = ((NarrowStringView*)stringObj)->data;
        prefixLen = ((NarrowStringView*)stringObj)->length;
    }
    // The allocator is stateless; its copy belongs in local storage.
    struct AllocatorToken {};
    AllocatorToken sourceA;
    AllocatorToken prefixAllocator = sourceA;
    String_ReserveCapacity(p, prefixLen + textLen);
    String_ReplaceRange(p, 0, 0, begin, begin + prefixLen, (char*)&prefixAllocator);
    AllocatorToken sourceB;
    AllocatorToken textAllocator = sourceB;
    String_ReplaceRange(p, (int)prefixLen, 0, (void*)text, (void*)(text + textLen), (char*)&textAllocator);
}

// Apply opacity recursively to the owned puppet hierarchy.
void* PuppetOwner_SetOpacityRecursive(PuppetOwnerView* p, unsigned int opacity) {
    return WPuppet_SetOpacityRecursive(p->puppet, opacity);
}

// Apply a named puppet layer mode and refresh part hiding when disabling that layer.
void PlayerModel_ConfigureNamedPuppetLayer(PuppetOwnerView* p, int mode, int flag, const char* name) {
    if (p->puppet != 0) {
        WPuppet_SetBoneVisibilityRecursive(p->puppet, mode, flag, name);
        if (flag == 0)
            AvatarModel_ApplyPartHideMasks(p, 1);
    }
}

// Toggle the default puppet layer and refresh part hiding when disabling it.
void PlayerModel_SetDefaultPuppetLayerEnabled(PuppetOwnerView* p, int flag) {
    if (p->puppet != 0) {
        WPuppet_SetBoneVisibilityRecursive(p->puppet, 1, flag, 0);
        if (flag == 0)
            AvatarModel_ApplyPartHideMasks(p, 1);
    }
}

// Return the player model's base puppet.
void* PlayerModel_GetPuppet(PuppetHandleView* p) {
    return p->puppet;
}

// Default player-model readiness hook returns true.
int PlayerModel_DefaultReady() {
    return 1;
}

// Return the embedded animation-puppet resource vector.
void* PlayerModel_GetAnimationResourceVector(void* p) {
    return &((AnimationPuppetListView*)p)->entries;
}

extern "C" void* Model_SetMatrixRecursive(void*, void*, void*);
extern "C" void* WPuppet_UpdateBoneWorldMatrices(void*);
extern "C" void* ResourceManager_LoadByExtension(void*, void*, int, int, int);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void* appMallocDummy__FUl(unsigned int);
extern "C" void appFreeUni__FPv(void*);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void __dl__FPv(void*);
extern "C" void __dla__FPv(void*, int);
extern "C" void __throw(void*, void*, void*);
extern "C" void __dt__Q23std12length_errorFv(void*);
extern "C" char* strcpy(char*, const char*);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void* memset(void*, int, unsigned int);
extern "C" char __vt__Q23std11logic_error[];
extern "C" char s_sharedCharDeleterVtable[];
extern "C" char __vt__Q23std12length_error[];

struct PuppetResourceListView {
    char headerStorage[0x1c];
    void** entries;
    unsigned int count;
    unsigned int capacity;
    void* selected;
};

#pragma opt_propagation off
#pragma dont_inline off
struct ErrorBase_u801B4F20 : std::exception {
    // Write the exception vtable pointer without changing its payload.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    char* text;
    void* control;
};
struct BufferGuard_u801B4F20 {
    char* data;
    char* flag;
    // Preserve the cleanup call: it passes the GUARD address and -1,
    // rather than data. A guard-aware callee would need data at +0 and flag
    // at +4 (Wii); the two-argument declaration alone does not prove that ABI.
    // __dla__FPv is the array-delete symbol, so the target/guard contract needs
    // investigation before changing this call or claiming it frees the text.
    // Successful control adoption clears data and disarms this cleanup.
    ~BufferGuard_u801B4F20() {
        if (data)
            __dla__FPv(this, -1);
    }
};
struct Control {
    void* vtable;
    unsigned int refs, weak;
    char* text;
    // Create the shared character-buffer control block with initial reference counts.
    Control(char* p) : refs(1), weak(1) {
        vtable = s_sharedCharDeleterVtable;
        text = p;
    }
};
namespace std {
struct length_error : ErrorBase_u801B4F20 {
    // Construct the owned length-error message and select its exception type.
    length_error(const char* str) {
        *(void**)this = __vt__Q23std11logic_error;
        struct Flag {
            char value;
            char pad[3];
        };
        Flag flag;
        flag.value = 0;
        unsigned int n = strlen(str) + 1;
        char* allocated = (char*)__nwa__FUl(2 * n - n);
        text = allocated;
        {
            BufferGuard_u801B4F20 guard;
            guard.data = allocated;
            guard.flag = &flag.value;
            control = new Control(allocated);
            guard.data = 0;
        }
        strcpy(text, str);
        *(void**)this = __vt__Q23std12length_error;
    }

    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
#pragma opt_propagation off
// Temporary vector owner: data is freed on unwind or after the final swap.
// transfer copies pointer slots into the prefix before the new tail, then zeros the
// old slots; resource pointees survive. offset counts prefix slots not transferred yet.
struct Storage {
    void** data;
    unsigned int count, capacity;
    unsigned int* owner;
    unsigned int offset;
    // Create an empty temporary slot buffer associated with the owner capacity.
    Storage(unsigned int* p) : data(0), count(0), capacity(0), owner(p), offset(0) {}
    // Free the slot allocation retained by this temporary.
    ~Storage() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
    // Copy the old pointer range into the new prefix, then zero the source slots.
    void transfer(void** first, void** last) {
        int n = last - first;
        unsigned int bytes = n * 4;
        offset -= n;
        memcpy(data + offset, first, bytes);
        memset(first, 0, bytes);
        *(volatile unsigned int*)&count = count + n;
    }
};
// Select the larger model-resource vector growth request by reference.
inline const unsigned int& maximum(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Throw the model-resource vector's length error using the game's exception ABI.
inline void capacityError(const char* message) {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(std::length_error)];
        } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(message);
        exception_runtime::__throw(((PetModelStringPool*)exception_runtime::petModelResourceStrings)->lengthErrorType,
                                   error, (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
#pragma use_lmw_stmw on

// Load a named puppet once, grow its resource vector if needed, and select it.
// value is a borrowed NUL-terminated resource name. Existing matches return without
// changing selected. Failed loads also leave selected untouched. Successful new loads
// join the owning resource list and become selected; Storage owns only the slot buffer.
void PuppetResourceList_LoadIfMissing(PuppetResourceListView* p, void* value) {
    // Phase 1: exact name lookup, then load only if absent.
    void** entry = p->entries;
    while (entry != p->entries + p->count) {
        if (String_CompareCString(((WPuppet*)*entry)->resourceName, value) == 0)
            return;
        ++entry;
    }
    void* resource = ResourceManager_LoadByExtension(g_resourceManager, value, 0, 1, 0);
    if (resource == 0)
        return;
    if (resource == 0)
        return;
    // Phase 2: append in spare capacity or reserve a replacement pointer vector.
    unsigned int currentCount = p->count;
    unsigned int currentCapacity = p->capacity;
    if (currentCount < currentCapacity) {
        unsigned int index = ++p->count;
        p->entries[index - 1] = resource;
    } else {

        if (kMaximumResourceVectorEntries - p->capacity < 1) {
            const char* message = ((PetModelStringPool*)petModelResourceStrings)->vectorLengthError;
            capacityError(message);
        }
        Storage storage(&p->capacity);
        unsigned int need;
        need = 1 + p->count - p->capacity;
        unsigned int cap = *storage.owner;
        if (need > kMaximumResourceVectorEntries - cap) {
            const char* message = ((PetModelStringPool*)petModelResourceStrings)->vectorLengthError;
            capacityError(message);
        }
        if (cap < 0x15555555U) {

            cap += maximum((3 * ((1 + cap)) / 5), need);
        } else if (!(!(cap < 0x2aaaaaaaU))) {
            unsigned int growth2 = ((1 + cap)) / 2;
            cap += maximum(growth2, need);
        } else {
            cap = kMaximumResourceVectorEntries;
        }
        if (cap > kMaximumResourceVectorEntries) {
            const char* message = ((PetModelStringPool*)petModelResourceStrings)->vectorLengthError;
            capacityError(message);
        }
        unsigned int savedCap = cap;
        unsigned int savedValue = 4 * savedCap;
        void** allocated = (void**)appMallocDummy__FUl(savedValue);
        *(volatile unsigned int*)&storage.offset = p->count;
        storage.data = allocated;
        (storage.offset + allocated)[storage.count] = resource;
        unsigned int savedCap2 = cap;
        storage.capacity = savedCap2;
        unsigned int savedCount = p->count;
        storage.count = storage.count + 1;
        void** savedEntries = p->entries;
        void** savedValue2 = savedCount + savedEntries;
        // Phase 3: transfer the existing prefix; old slots become null.
        storage.transfer(p->entries, savedValue2);
        *(volatile unsigned int*)&p->count = 0;

        // Phase 4: publish the replacement. storage now owns the old slot allocation.
        void** savedEntries2 = p->entries;
        unsigned int savedCapacity = p->capacity;
        void** old = savedEntries2;
        unsigned int oldCap = savedCapacity;
        p->capacity = storage.capacity;
        unsigned int savedOldCap = oldCap;
        __typeof__(storage.data) newEntries = storage.data;
        __typeof__(storage.data) retainedEntries = newEntries;
        __typeof__(retainedEntries) entriesToInstall = storage.data;
        storage.capacity = savedOldCap;
        p->entries = entriesToInstall;
        storage.data = old;
        p->count = storage.count;
        storage.count = 0;
    }
    p->selected = resource;
}

// Return the currently selected animation puppet.
void* ObjectList_GetSelected(void* p) {
    return ((ResourceModel801B*)p)->selected;
}

// Apply a matrix recursively to the model's puppet hierarchy.
void* ResourceModel_SetMatrixRecursive(void* p, void* matrix, void* updateMode) {
    return Model_SetMatrixRecursive(((ResourceModel801B*)p)->model, matrix, updateMode);
}

// Refresh world matrices for the model's puppet bones.
void* ResourceModel_UpdateBoneWorldMatrices(void* p) {
    return WPuppet_UpdateBoneWorldMatrices(((ResourceModel801B*)p)->model);
}

#pragma opt_common_subs off
struct F12CString {
    unsigned char data[12];
};

struct F12CStringCleanup {
    F12CString* object;
    // Destroy the temporary narrow string on normal exit or unwind.
    ~F12CStringCleanup() {
        ((NString*)object)->~NString();
    }
};

// Replace the base .apet suffix with a context-specific animation resource suffix.
static inline void f12cAppend(F12CString& str, int offset) {
    F12CString temp, joined;
    String_ConstructSubstring(
        &joined, &str, 0,
        String_FindLastOfCStr(&str, ((PetModelStringPool*)petModelResourceStrings)->animationExtension, -1) - 4);
    F12CStringCleanup joinedCleanup = {&joined};
    String_ConstructConcatCStr((void**)&temp, &joined, petModelResourceStrings + offset);
    F12CStringCleanup tempCleanup = {&temp};
    String_AssignWithInlineFastPath(&str, &temp);
}

struct F12CSystem {
    virtual void pad0();
    virtual void pad1();
    virtual void pad2();
    virtual void pad3();
    virtual void pad4();
    virtual void pad5();
    virtual void pad6();
    virtual void query(int, int, void*);
};

// Resolve character data, load its context animation and face texture, and refresh equipped parts.
// selection is borrowed ItemSelection data. parameters points to 0x14 readable
// bytes copied into model-owned storage (or null to zero it). The face filename
// extension is a separate borrowed pointer; it has no parameter-block meaning.
// The 0x14-byte payload schema is unresolved: keep it opaque and preserve its size.
// Fourth and sixth arguments are unused here. faceVariant is an integer passed in a
// pointer-typed ABI slot for sprintf; useStoryAnimation is a nonnull boolean.
// animationContext chooses the suffix enum above; unknown values keep the base path.
// Returns 1 after setup and setting unk_1B8, whose consumer is unresolved; returns 0
// on selection/character/base-model failure. Failure may leave partially changed state.
// This model retains loaded handles; the old face handle is released before replacement.
int AvatarModel_LoadCharacterResources(void* self, void* selection, void* parameters, void* unusedArgument4,
                                       int animationContext, void* unusedArgument6, void* faceVariant,
                                       void* useStoryAnimation) {
    char baseModelPath[0x80];
    char animationModelPath[0x80];
    char faceBitmapVariant[0x100];
    void* characterData;
    int sameCharacter;
    int resourceRoot;
    typedef void (*VCall)(void*, int, int, void*);
    // Phase 1: resolve selection/character identity and release resources if it changed.
    void* resourceSystem = Pointer_GetFirstPointer(g_resourceManager);
    ((F12CSystem*)resourceSystem)->query(0x1f, 2, &resourceRoot);
    if (!ItemSelection_ResolvePartData(&((AvatarSelectionView*)self)->selection, selection))
        return 0;
    sameCharacter = (((CharacterSelectionHeader*)selection)->characterId == ((ResourceModel801B*)self)->key);
    if (!sameCharacter)
        PlayerModel_ReleaseResourcesByMask((PlayerModelResourcesView*)self, kReleaseAllResources);
    unsigned int totalPartCount, primaryPartCount, characterId;
    characterId = ((CharacterSelectionHeader*)selection)->characterId;
    primaryPartCount = ((ItemSelection801B7688*)selection)->primaryCount;
    totalPartCount = ((ItemSelection801B7688*)selection)->totalCount;
    ((ResourceModel801B*)self)->key = characterId;
    ((ResourceModel801B*)self)->primaryPartCount = primaryPartCount;
    ((ResourceModel801B*)self)->resourceCount = totalPartCount;
    characterData = SharedDoc_GetItemInventory();
    characterData = ItemInventory_FindCharacterData(characterData, (*(void**)&((ResourceModel801B*)self)->key));
    if (!characterData)
        return 0;
    sprintf(baseModelPath, ((PetModelStringPool*)petModelResourceStrings)->baseModelFormat,
            ((IFF_STRUCT::sChar*)characterData)->model);
    if (sameCharacter == 0) {
        void* found = ResourceManager_LoadByExtension(g_resourceManager, baseModelPath, 0, 0, 0);
        ((ResourceModel801B*)self)->model = found;
        if (!found)
            return 0;
    }
    // Phase 2: select/load animation, rebuild equipped parts and puppet controllers.
    sprintf(animationModelPath, ((PetModelStringPool*)petModelResourceStrings)->animationModelFormat,
            ((IFF_STRUCT::sChar*)characterData)->model);
    F12CString path;
    MWString_ConstructCString(&path, animationModelPath);
    F12CStringCleanup pathCleanup = {&path};
    switch (animationContext) {
    case kMenuAnimation:
        f12cAppend(path, kMenuSuffix);
        break;
    case kSwingAnimation:
        f12cAppend(path, kSwingSuffix);
        break;
    case kStoryAnimation:
        f12cAppend(path, kStorySuffix);
        break;
    case kCommendationAnimation:
        f12cAppend(path, kCommendationSuffix);
        break;
    }
    if (useStoryAnimation) {
        F12CString opt;
        MWString_ConstructCString(&opt, animationModelPath);
        F12CStringCleanup optCleanup = {&opt};
        f12cAppend(opt, kStorySuffix);
        PuppetResourceList_LoadIfMissing((PuppetResourceListView*)self, MWString_GetCString(&opt));
    } else
        PuppetResourceList_LoadIfMissing((PuppetResourceListView*)self, MWString_GetCString(&path));
    AvatarModel_ReloadPartResources(self, -1);
    if (((ResourceModel801B*)self)->model) {
        WPuppet_RefreshMatrixBuffers(((ResourceModel801B*)self)->model);
        WPuppet_RebuildBoneController(((ResourceModel801B*)self)->model);
        WPuppet_InitializeTextureCoordinateAnimation(((ResourceModel801B*)self)->model);
    }
    // Phase 3: copy the optional opaque parameter block into model-owned storage.
    if (parameters)
        memcpy(((ResourceModel801B*)self)->copiedParameters, parameters, 0x14);
    else
        memset(((ResourceModel801B*)self)->copiedParameters, 0, 0x14);
    // Phase 4: construct the face variant, replace its retained bitmap and bind pairs.
    strcpy(faceBitmapVariant, ((IFF_STRUCT::sChar*)characterData)->faceTexture);
    const char* faceExtension = CString::FindLastCharForwarder(((IFF_STRUCT::sChar*)characterData)->faceTexture, '.');
    char* dot2 = CString::FindLastCharForwarder(faceBitmapVariant, '.');
    sprintf(dot2, ((PetModelStringPool*)petModelResourceStrings)->faceVariantFormat, faceVariant, faceExtension);
    if (g_resourceManager && ((ResourceModel801B*)self)->resource) {
        ResourceManager_ReleaseId(g_resourceManager, ((ResourceModel801B*)self)->resource);
        ((ResourceModel801B*)self)->resource = 0;
    }
    ((ResourceModel801B*)self)->resource = ResourceManager_LoadBitmap(
        g_resourceManager, ((IFF_STRUCT::sChar*)characterData)->faceTexture, 0, 0, faceBitmapVariant);
    void* node =
        ResourceManager_FindByNormalizedBasename(g_resourceManager, ((IFF_STRUCT::sChar*)characterData)->faceTexture);
    if (node)
        WPuppet_ApplyNamedResourcePairRecursive(((ResourceModel801B*)self)->model, ((ResourceModel801B*)self)->resource,
                                                ((ResourcePairNode*)node)->pairedResource, 0);
    // Phase 5: apply coverage/texture/bitmap variants and publish setup completion.
    AvatarModel_RefreshCoveredParts(self);
    AvatarModel_ApplyTextureVariant(self, selection, ((ItemSelection801B7688*)selection)->textureVariant);
    CharacterModel_ApplyBitmapVariant(self, 0, ((ItemSelection801B7688*)selection)->bitmapVariant);
    // Mark the end of resource/face/part variant setup; the flag's consumer remains unresolved.
    ((ResourceModel801B*)self)->unk_1B8 = 1;
    return 1;
}

// The optional parameter payload is copied verbatim; its internal schema is unresolved.
struct AvatarCharacterParameterBlock {
    unsigned char bytes[0x14];
};

// Load a character with typed switches; selection/parameters are borrowed and loaded handles stay with model.
static inline int LoadAvatarCharacterForContext(void* model, ItemSelection801B7688* selection,
                                                const AvatarCharacterParameterBlock* parameters,
                                                ModelAnimationContext animationContext, int faceVariant,
                                                bool useStoryAnimation) {
    return AvatarModel_LoadCharacterResources(model, selection, (void*)parameters, 0, animationContext, 0,
                                              (void*)faceVariant, (void*)(int)useStoryAnimation);
}

struct F56Item {
    char headerStorage[4];
    void* resource;
};
struct F56Node {
    // key/key2 alias the same owned copied path at insertion; flag is initialized to zero only here.
    char* key;          // 0x00
    char* key2;         // 0x04
    unsigned char flag; // 0x08
    char pad[3];
    F56Node* prev;       // 0x0c
    F56Node* next;       // 0x10
    F56Node* bucketNext; // 0x14
};
struct F56Map {
    char headerStorage[0x60];
    F56Node* resourceList; // 0x60
    char listState[4];
    F56Node* chunk;    // 0x68
    F56Node* freeHead; // 0x6c
    int capacity;      // 0x70
    unsigned int mask; // 0x74
    char bucketState[4];
    F56Node** buckets; // 0x7c
    char lock[0x18];   // 0x80
};
extern void* appMallocUni__FUl(unsigned int);
extern void OSLockMutex(void*);
extern void OSUnlockMutex(void*);
extern void* g_taskManager;

// Reload view: key carries the character IFF ID in a pointer-shaped ABI word;
// items borrow resolved IFF part records, containers/partTextures retain loaded handles.
struct F56Self {
    void* vtable;
    void* characterTypeIdWord;
    char resourceState[0x28];
    int count;
    int coveredPartSlot;
    void* containers[24];             // 0x38
    void* partTextures[72];           // 0x98
    unsigned char usesBaseTexture;    // 0x1b8
    unsigned char coveredPartEnabled; // 0x1b9
    char selectionStorage[7];
    unsigned char resourceCount; // 0x1c1
    char selectionPadding[2];
    F56Item items[0]; // 0x1c4
};

// Descriptive aliases preserve the legacy layout/type identities used by this unit.
typedef F56Item EquipmentResourceEntry;
typedef F56Node MissingPartResourceNode;
typedef F56Map MissingPartResourceMap;
typedef F56Self EquipmentReloadView;

// Keep this four-store splice inline even under the unit's late optimization pragmas.
#pragma push
#pragma always_inline on
// Splice a detached registry node immediately before a nonempty circular-list head.
static inline void LinkEquipmentRegistryNode(MissingPartResourceNode* node, MissingPartResourceNode* head) {
    node->next = head;
    node->prev = head->prev;
    head->prev = node;
    node->prev->next = node;
}

#pragma pop

#pragma push
#pragma always_inline on
// Grow the registry pool if empty and detach one node under the recursive instance mutex.
static inline void AcquireEquipmentRegistryNode(MissingPartResourceMap*& map, MissingPartResourceNode*& node) {
    InstanceLock_u801B4F20 lk2(map->lock);
    // Registry phase 1: grow its node pool while holding the nested instance lock.
    if (!map->freeHead) {
        MissingPartResourceNode* pool = (MissingPartResourceNode*)appMallocUni__FUl(map->capacity * 0x18);
        void* lockA = map->lock;
        MissingPartResourceNode* chunk = map->chunk;
        MissingPartResourceNode* node = pool;
        OSLockMutex(lockA);
        if (chunk == 0) {
            node->prev = node;
            node->next = node;
            OSUnlockMutex(lockA);
            chunk = node;
        } else {
            LinkEquipmentRegistryNode(node, chunk);
            OSUnlockMutex(lockA);
        }
        map->chunk = chunk;
        int k = 1;
        if (map->capacity > 1) {
            void* lockB = map->lock;
            MissingPartResourceNode* p = pool + 1;
            while (k < map->capacity) {
                MissingPartResourceNode* head = map->freeHead;
                OSLockMutex(lockB);
                if (head == 0) {
                    p->prev = p;
                    p->next = p;
                    OSUnlockMutex(lockB);
                    head = p;
                } else {
                    LinkEquipmentRegistryNode(p, head);
                    OSUnlockMutex(lockB);
                }
                map->freeHead = head;
                k++;
                p++;
            }
        }
    }
    void* lock = map->lock;
    node = map->freeHead;
    OSLockMutex(lock);
    node->next->prev = node->prev;
    node->prev->next = node->next;
    MissingPartResourceNode* next = node->next;
    if (node == next)
        next = 0;
    OSUnlockMutex(lock);
    map->freeHead = next;
}
#pragma pop

// Reload masked equipment slots, bind texture pairs, and preserve the existing missing-path branch.
void AvatarModel_ReloadPartResources(void* object, unsigned int slotMask) {
    // Phase 1: borrow character data and acquire a temporary base-texture handle.
    void* baseTexture = 0;
    EquipmentReloadView* self = (EquipmentReloadView*)object;
    self->usesBaseTexture = 0;
    IFF_STRUCT::sChar* data =
        (IFF_STRUCT::sChar*)ItemInventory_FindCharacterData(g_sharedDoc->itemInventory, self->characterTypeIdWord);
    if (data) {
        if (*(signed char*)(data->baseTexture))
            baseTexture = ResourceManager_LoadBitmap(g_resourceManager, data->baseTexture, 0, 0, 0);
        char* textureSlotsCursor = (char*)self;
        char* partModelsCursor = (char*)self;
        // original ~0x801B5758-0x801B5770: a slot not in slotMask still advances both cursors (the loop increment);
        // with the increments at the end of the body, 'continue' left them behind and later slots used the wrong models
        // Phase 2: visit selected slots; both cursor strides advance even on continue.
        for (int partSlot = 0; self->count > partSlot; partSlot++, partModelsCursor += 4, textureSlotsCursor += 0xc) {
            if (!(slotMask & (0xFFFFFFFFu << partSlot)))
                break;
            if (!(slotMask & (1u << partSlot)))
                continue;
            void* partTextures[kTexturesPerPart];
            memset(partTextures, 0, sizeof(partTextures));
            void* replacementTextures[kTexturesPerPart];
            memset(replacementTextures, 0, sizeof(replacementTextures));
            EquipmentResourceEntry* entry;
            unsigned char selectionCount = self->resourceCount;
            if ((unsigned char)partSlot >= selectionCount)
                entry = 0;
            else
                entry = &self->items[(unsigned char)partSlot];
            if (entry != 0) {
                EquipmentResourceEntry* entry2;
                if ((unsigned char)partSlot >= selectionCount)
                    entry2 = 0;
                else
                    entry2 = &self->items[(unsigned char)partSlot];
                void* partData = entry2->resource;
                if (partData) {
                    char partModelPath[0x28];

                    if (((((IFF_STRUCT::sPart*)partData)->common.typeId >> 11) & 3) == 2) {
                        self->coveredPartSlot = partSlot;
                        self->coveredPartEnabled = 1;
                    } else if ((((((IFF_STRUCT::sPart*)partData)->common.typeId >> 11) & 3) != 2) &&
                               ((IFF_STRUCT::sPart*)partData)->hideMask == 2) {
                        self->coveredPartSlot = 1;
                        self->coveredPartEnabled = 0;
                    } // original 0x801B57F4-0x801B581C: the second test is kind != 2 (always true here), then hideMask == 2

                    sprintf(partModelPath, petModelResourceStrings + kPartModelFormatOffset,
                            ((IFF_STRUCT::sPart*)partData)->model);
                    char* p = (char*)partData;
                    char* replacementTextureName = ((IFF_STRUCT::sPart*)partData)->secondTextures[0];
                    char* textureName = ((IFF_STRUCT::sPart*)partData)->textures[0];
                    // Acquire paired source/replacement textures for each layer.
                    for (int textureLayer = 0; textureLayer < kTexturesPerPart; textureLayer++,
                             p += kTextureResourceNameLength, textureName += kTextureResourceNameLength,
                             replacementTextureName += kTextureResourceNameLength) {
                        if (p[IFF_OFFSET(IFF_STRUCT::sPart, textures)]) {
                            if (p[IFF_OFFSET(IFF_STRUCT::sPart, secondTextures)]) {
                                partTextures[textureLayer] =
                                    ResourceManager_LoadBitmap(g_resourceManager, textureName, 0, 0, 0);
                                replacementTextures[textureLayer] =
                                    ResourceManager_LoadBitmap(g_resourceManager, replacementTextureName, 0, 0, 0);
                            }
                        }
                    }
                    // Reuse a matching child model, otherwise replace it before releasing the old handle.
                    void* currentPartModel = ((ResourceModel801B*)partModelsCursor)->partModels[0];
                    if (currentPartModel &&
                        String_CompareCString(((WPuppet*)currentPartModel)->resourceName, partModelPath) == 0) {
                        for (int textureLayer = 0; kTexturesPerPart > textureLayer; textureLayer++)
                            WPuppet_ApplyNamedResourcePairRecursive(
                                ((ResourceModel801B*)self)->model, partTextures[textureLayer],
                                replacementTextures[textureLayer],
                                (int)((ResourceModel801B*)partModelsCursor)->partModels[0]);
                    } else {
                        void* found = ResourceManager_LoadByExtension(g_resourceManager, partModelPath, 0, 1, 0);
                        if (found) {
                            WPuppet_ReplaceChildAndLayerAnimation(((ResourceModel801B*)self)->model,
                                                                  ((ResourceModel801B*)partModelsCursor)->partModels[0],
                                                                  found, partTextures[0], replacementTextures[0]);
                            for (int textureLayer = 0; kTexturesPerPart > textureLayer; textureLayer++)
                                if (partTextures[textureLayer] && replacementTextures[textureLayer])
                                    WPuppet_ApplyNamedResourcePairRecursive(
                                        ((ResourceModel801B*)self)->model, partTextures[textureLayer],
                                        replacementTextures[textureLayer], (int)found);
                            // original ~0x801B5928: a model that failed to load (found == 0) skips everything below, so the oldPartModel part
                            // model stays; the release, the store and the missing-resource registration run only when found != 0
                            void* oldPartModel = ((ResourceModel801B*)partModelsCursor)->partModels[0];
                            if (oldPartModel)
                                ResourceManager_ReleaseObject(g_resourceManager, oldPartModel, 0);
                            ((ResourceModel801B*)partModelsCursor)->partModels[0] = found;
                            // Preserved quirk: this is nested under if(found), so
                            // registration is unreachable here. Do not relocate it on a
                            // readability edit: that would change load-failure behavior.
                            if (found == 0) {
                                void* mgr = g_taskManager ? Object_GetChild(g_taskManager) : 0;
                                MissingPartResourceMap* map =
                                    mgr ? (*(MissingPartResourceMap**)&((ModelMissingResourceRegistry*)mgr)
                                                ->missingResources)
                                        : 0;
                                if (map) {
                                    int n = strlen(partModelPath) + 1;
                                    char* copy = (char*)__nwa__FUl(2 * n - n);
                                    strcpy(copy, partModelPath);
                                    InstanceLock_u801B4F20 lk1(map->lock);
                                    MissingPartResourceNode* node;
                                    AcquireEquipmentRegistryNode(map, node);
                                    // Registry phase 2: attach the owned path to hash buckets and the live list.
                                    node->key = copy;
                                    node->flag = 0;
                                    node->key2 = copy;
                                    if (copy) {
                                        void* lockHash = map->lock;
                                        OSLockMutex(lockHash);
                                        unsigned int len = strlen(copy);
                                        unsigned int h = ((unsigned char)copy[0] & 0x2a) |
                                                         ((unsigned char)copy[len ? len - 1 : 0] & 0x15);
                                        h = (h & 0x3f) | (len << 6);
                                        h = map->mask & h;
                                        OSUnlockMutex(lockHash);
                                        void* lockBuckets = map->lock;
                                        OSLockMutex(lockBuckets);
                                        node->bucketNext = map->buckets[h];
                                        map->buckets[h] = node;
                                        OSUnlockMutex(lockBuckets);
                                    }
                                    void* lockList = map->lock;
                                    MissingPartResourceNode* head = map->resourceList;
                                    OSLockMutex(lockList);
                                    if (!head) {
                                        node->prev = node;
                                        node->next = node;
                                        OSUnlockMutex(lockList);
                                    } else {
                                        LinkEquipmentRegistryNode(node, head);
                                        OSUnlockMutex(lockList);
                                        node = head;
                                    }
                                    map->resourceList = node;
                                }
                            }
                        }
                    }
                    for (int textureLayer = 0; textureLayer < kTexturesPerPart; textureLayer++) {
                        if ((int)replacementTextures[textureLayer] == (int)baseTexture &&
                            (int)replacementTextures[textureLayer] == (int)partTextures[textureLayer])
                            self->usesBaseTexture = 1;
                        if (g_resourceManager) {
                            if (replacementTextures[textureLayer]) {
                                ResourceManager_ReleaseId(g_resourceManager, replacementTextures[textureLayer]);
                                replacementTextures[textureLayer] = 0;
                            }
                        }
                    }
                } else {
                    WPuppet_ReplaceChildAndLayerAnimation(((ResourceModel801B*)self)->model,
                                                          ((ResourceModel801B*)partModelsCursor)->partModels[0], 0, 0,
                                                          0);
                    if (g_resourceManager) {
                        if (((ResourceModel801B*)partModelsCursor)->partModels[0] != 0) {
                            ResourceManager_ReleaseObject(g_resourceManager,
                                                          ((ResourceModel801B*)partModelsCursor)->partModels[0], 0);
                            ((ResourceModel801B*)partModelsCursor)->partModels[0] = 0;
                        }
                    }
                }
                // Phase 3: transfer source texture handles into this slot after releasing prior ones.
                char* slot = textureSlotsCursor;
                void** src = partTextures;
                for (int textureLayer = 0; textureLayer < kTexturesPerPart; textureLayer++) {
                    if (g_resourceManager) {
                        if (((ResourceModel801B*)slot)->partTextures[0]) {
                            ResourceManager_ReleaseId(g_resourceManager, ((ResourceModel801B*)slot)->partTextures[0]);
                            ((ResourceModel801B*)slot)->partTextures[0] = 0;
                        }
                    }
                    ((ResourceModel801B*)slot)->partTextures[0] = *src;
                    src++;
                    slot += 4;
                }
            }
        }
        // Release the temporary base texture after every selected slot has been processed.
        if (g_resourceManager && baseTexture)
            ResourceManager_ReleaseId(g_resourceManager, baseTexture);
    }
}

} // extern "C"
