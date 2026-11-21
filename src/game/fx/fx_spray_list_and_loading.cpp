#define GAME_NSTRING_TEXT_CONSTRUCTOR
#include "game/string.h"
#include "classes/CFxSpray.h"

#define AsCFxSpray(p) ((CFxSpray*)(p))

// Translation unit 0x80110A50-0x80111E8C (14 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

#pragma dont_inline on

// Read the list's first-node pointer into its begin iterator.
void FxSprayList_GetBeginIterator(unsigned int* out, const char* src) {
    *out = *(const unsigned int*)(src + 8);
}

// Return the payload address of the iterator's current list node.
char* FxSprayListIterator_GetValueAddress(char** src) {
    return *src + 8;
}

// Construct the list's end iterator from its embedded sentinel.
void FxSprayList_GetEndIterator(char** out, char* src) {
    *out = src + 4;
}

// Construct the configuration-list end iterator from its embedded sentinel.
void FxSprayConfigList_GetEndIterator(char** out, char* src) {
    *out = src + 4;
}

// Advance a particle-list iterator to its next node.
void FxSprayListIterator_Advance(char** self) {
    *self = *(char**)(*self + 4);
}

// Test whether two node iterators are equal.
int FxSprayIterator_Equals(unsigned int* left, unsigned int* right) {
    return *left == *right;
}

// Test whether two node iterators differ.
int FxSprayIterator_NotEquals(unsigned int* left, unsigned int* right) {
    return *left != *right;
}

int memcmp(const char*, const char*, unsigned int);

// Non-owning views of the name tree and the particle-list sentinels.
struct SprayNameNode { void* left; void* right; void* parent; NString name; };
struct SprayCacheRecord { NString name; void* object; };
struct SprayListView { unsigned int count; char* previous; char* next; };
struct SprayTextureBufferView { char pad_000[0x36C]; char textureBuffer[12]; };
struct SprayOwnerView { char pad_000[0x140]; void* resourceManager; };
struct SprayScriptStringPool { char trimCharacters[6]; char scanLine[3]; };
static const int kSprayScanLineOffset = (int)&((SprayScriptStringPool*)0)->scanLine;
enum SprayScriptLimits { kSprayScriptLineBytes = 0x400, kSprayScriptOpenFlags = 0x2000 };

enum SprayOwnerOffsets { kSprayOwnerNameCache = 0x128, kSprayOwnerResourceManager = 0x140 };

// Every list header is {u32 count, node* previous, node* next}; its sentinel
// aliases &previous and has no payload. Nodes are {previous,next,payload...}.
// ClearNodes frees nodes only. Particle payload objects must be destroyed first;
// parameter-track payloads are inline values and need no separate release.
// All addresses passed to the cleanup routines must refer to initialized rings.

struct StdMapTreeHead {
    char unk0[4];
    char* root;
};

#pragma dont_inline off
// Break equal-prefix byte comparisons by length to obtain a three-way string order.
static inline int fn_cmp3(int r, unsigned int a, unsigned int b) {
    if (r != 0) return r;
    if (a < b) return -1;
    if (a == b) return 0;
    return 1;
}

// Borrow narrow-string bytes and length; bit 31 selects heap storage.
static inline void ReadSprayString(const NString* text, const char*& bytes, unsigned int& length) {
    if ((text->flags >> 31) == 0) {
        length = *(const unsigned char*)text & 0x7f;
        bytes = (const char*)text + 1;
    } else {
        bytes = text->data;
        length = text->length;
    }
}

// Read a string length without touching its borrowed character buffer.
static inline unsigned int SprayStringLength(const NString* text) {
    if ((text->flags >> 31) == 0) return *(const unsigned char*)text & 0x7f;
    return text->length;
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma optimization_level 2
// Find a spray-name tree entry with small-string lexicographic comparison, or return end.
void FxSprayCache_FindNameIterator(void** resultIn, void* treeIn, const void* keyIn) {
    void** result;
    const void* key;
    StdMapTreeHead* head;
    result = resultIn;
    key = keyIn;
    head = (StdMapTreeHead*)treeIn;
    char* parent = (char*)&head->root;
    char* node = head->root;
    // Phase 1: lower_bound(key); parent is the smallest visited name >= key.
    // PPC strings: flags bit31 selects heap storage; inline byte0&0x7f is length.
    // The duplicated length slots preserve the compiler's reference-to-min temporaries.
    while (node != 0) {
        unsigned int valueMinSlot, valueLengthBSlot, keyLengthSlot, valueLengthASlot;
        unsigned int keyLength;
        const char* keyData;
        ReadSprayString((const NString*)key, keyData, keyLength);
        keyLengthSlot = keyLength;

        unsigned int valueLengthA;
        valueLengthA = SprayStringLength(&((SprayNameNode*)node)->name);
        valueLengthASlot = valueLengthA;

        const char* valueData;
        unsigned int valueLength;
        ReadSprayString(&((SprayNameNode*)node)->name, valueData, valueLength);
        valueLengthBSlot = valueLength;

        const unsigned int* minPtr = &valueLengthBSlot;
        if (valueLengthA < valueLength)
            minPtr = &valueLengthASlot;
        valueMinSlot = *minPtr;
        const unsigned int* cmpLenPtr = &valueMinSlot;
        if (keyLength < valueMinSlot)
            cmpLenPtr = &keyLengthSlot;
        int cmp = memcmp(valueData, keyData, *cmpLenPtr);
        if (cmp == 0) {
            if (valueMinSlot < keyLength) cmp = -1;
            else if (valueMinSlot != keyLength) cmp = 1;
        }
        if (cmp >= 0) {
            parent = node;
            node = *(char**)node;
        } else {
            node = *(char**)(node + 4);
        }
    }

    // Phase 2: reject a strictly greater lower bound by comparing in reverse.
    // An absent key or an empty tree returns the sentinel, never a null iterator.
    // Original 0x80111C58: keep the shared sentinel assignment after the lookup.
    do {
        if (parent != (char*)&head->root) {
            const char* valueData;
            unsigned int valueLength;
            ReadSprayString(&((SprayNameNode*)parent)->name, valueData, valueLength);
            unsigned int keyLength2Slot;
            unsigned int valueLengthSlot = valueLength;

            unsigned int keyLength2;
            keyLength2 = SprayStringLength((const NString*)key);
            keyLength2Slot = keyLength2;

            const char* keyData2;
            unsigned int keyLength3;
            ReadSprayString((const NString*)key, keyData2, keyLength3);
            unsigned int keyLength3Slot = keyLength3;

            const unsigned int* minPtr = &keyLength3Slot;
            if (keyLength2 < keyLength3)
                minPtr = &keyLength2Slot;
            unsigned int keyMinSlot = *minPtr;
            const unsigned int* cmpLenPtr = &keyMinSlot;
            if (valueLength < keyMinSlot)
                cmpLenPtr = &valueLengthSlot;
            int cmp = memcmp(keyData2, valueData, *cmpLenPtr);
            if (cmp == 0) {
                if (keyMinSlot < valueLength) cmp = -1;
                else if (keyMinSlot != valueLength) cmp = 1;
            }
            if (cmp >= 0) break;
        }
        parent = (char*)&head->root;
    } while (0);
    *result = parent;
}
#pragma opt_loop_invariants reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
#pragma optimization_level reset
#pragma dont_inline on

void MWString_ConstructCString(void*, const char*);
void FxSprayConfig_Assign(void*, void*);
void FxSpray_AcquireReferencedObject(void*, void*);
void ResourceHolder_SetResourceAddress(void*, void*);
void* Resource_OpenResolvedPath(void*, const char*, int, int, int, int);
int Stream_ScanFormat(void*, const char*, ...);
void Resource_Release(void*);
void ResourceHandle_dtor(void*, int);

void* CFxSpray_Ctor(void*, void*);
struct LoadObject { char data[0x378]; LoadObject(void*); };
#pragma dont_inline off
// Construct a full spray object owned/associated with the supplied effect owner.
LoadObject::LoadObject(void* parent) { CFxSpray_Ctor(this, parent); }
#pragma dont_inline on
void* FxSprayNameIterator_GetRecord(void*);
void* FxSprayNameTree_GetOrInsertValue(void*, const char*);
int CFxSpray_ParseScriptLine(void*, const char*);
char* String_TrimCharacterSet(void*, const char*);
extern const char fxSprayScriptStrings[];

#pragma dont_inline off
#pragma use_lmw_stmw on
typedef NString LoadKey;
struct LoadStream {
    void* stream;
    void* file;
    // Bind holder cleanup to this local file slot; explicit releases clear the slot.
    LoadStream(void* p) : file(p) { ResourceHolder_SetResourceAddress(&stream, &file); }
    // Destroy the holder on normal return or parse failure.
    ~LoadStream() { ResourceHandle_dtor(&stream, -1); }
};
// Trim script syntax before applying one configuration command to the spray.
static inline int ApplySprayScriptLine(void* self, char* line) {
    return CFxSpray_ParseScriptLine(self, String_TrimCharacterSet(line, fxSprayScriptStrings));
}

// Release the open resource and clear the slot also watched by the stream holder.
static inline void ReleaseSprayScriptFile(void*& file) {
    if (file) { Resource_Release(file); file = 0; }
}

// Load and parse a spray script; useCachedConfig copies a cache hit unless replacement is requested.
// Return 0 on open/parse failure, 1 on completion. Mode policy:
// cache miss OR replace: parse into self, create a configuration object, attach it;
// publish in owner cache only when replace==0. Cache hit + useCachedConfig: copy
// cached configuration. Cache hit without that flag: parse directly, without publishing.
// Resources returned by OpenResolvedPath are released here; cached objects are borrowed.
// Parse failure can leave self partially modified; this is not a transactional loader.
int CFxSpray_LoadByName(void* self, const char* name, int useCachedConfig, int replace) {
    // Phase 1: look up the owner cache; end denotes a cache miss.
    char found[4];
    {
        LoadKey key(name);
        FxSprayCache_FindNameIterator((void**)found, (char*)AsCFxSpray(self)->effectOwner + kSprayOwnerNameCache, &key);
    }
    char previous[4]; FxSprayConfigList_GetEndIterator((char**)previous, (char*)AsCFxSpray(self)->effectOwner + kSprayOwnerNameCache);
    int equal = FxSprayIterator_Equals((unsigned int*)found, (unsigned int*)previous);
    // Phase 2: miss/replacement builds an attached configuration after parsing.
    if (equal || replace) {
        LoadStream input(Resource_OpenResolvedPath(*(void**)((char*)AsCFxSpray(self)->effectOwner + kSprayOwnerResourceManager), name, kSprayScriptOpenFlags, 0, replace, 0));
        void*& file = input.file;
        if (!file) return 0;
        char scriptLine[kSprayScriptLineBytes];
        while (Stream_ScanFormat(file, fxSprayScriptStrings + kSprayScanLineOffset, scriptLine)) {
            if (!ApplySprayScriptLine(self, scriptLine)) {
                ReleaseSprayScriptFile(file);
                return 0;
            }
        }
        ReleaseSprayScriptFile(file);
        void* object = new LoadObject(AsCFxSpray(self)->effectOwner);
        FxSprayConfig_Assign((char*)&AsCFxSpray(object)->particlesFollowEmitter, (char*)&AsCFxSpray(self)->particlesFollowEmitter);
        FxSpray_AcquireReferencedObject(self, object);
        if (!replace) {
            LoadKey temp(name);
            void** slot = (void**)FxSprayNameTree_GetOrInsertValue((char*)AsCFxSpray(self)->effectOwner + kSprayOwnerNameCache, (const char*)&temp);
            *slot = object;
        }
    // A cache hit either copies configuration or parses privately without publishing.
    } else if (useCachedConfig) {
        void* node = FxSprayNameIterator_GetRecord(found);
        void* object = ((SprayCacheRecord*)node)->object;
        FxSprayConfig_Assign((char*)&AsCFxSpray(self)->particlesFollowEmitter, (char*)&AsCFxSpray(object)->particlesFollowEmitter);
        FxSpray_AcquireReferencedObject(self, self);
    } else {
        LoadStream input(Resource_OpenResolvedPath(*(void**)((char*)AsCFxSpray(self)->effectOwner + kSprayOwnerResourceManager), name, kSprayScriptOpenFlags, 0, replace, 0));
        void*& file = input.file;
        if (!file) return 0;
        char scriptLine[kSprayScriptLineBytes];
        while (Stream_ScanFormat(file, fxSprayScriptStrings + kSprayScanLineOffset, scriptLine)) {
            if (!ApplySprayScriptLine(self, scriptLine)) {
                ReleaseSprayScriptFile(file);
                return 0;
            }
        }
        ReleaseSprayScriptFile(file);
    }
    return 1;
}
#pragma use_lmw_stmw reset
#pragma dont_inline on

void __dt__13GeneTexBufferFv(void*, int);
// Destroy the sixteen owned configuration-track lists in reverse member order.
void* FxSprayConfig_dtor(void*, int);
void ParticleList_dtor(void*, int);
void ResourceManager_ReleaseId(void*, void*);
// Detach and free particle-list nodes while preserving the list sentinel.
void FxSprayList_ClearNodes(void*);
void __dl__FPv(void*);
extern unsigned char __vt__8CFxSpray[];
struct FxVObj {
    virtual void v0(int);
};

#pragma opt_common_subs off
#pragma opt_loop_invariants off
// Destroy live particles, release the script resource and texture buffer, then free track lists.
void* CFxSpray_dtor(void* self, int shouldFree) {
    if (self) {
        *(void**)self = __vt__8CFxSpray;
        char* it;
        FxSprayList_GetBeginIterator((unsigned int*)&it, (char*)&AsCFxSpray(self)->particleCount);
        // Original 0x801111A0: release each pointed-to object before advancing.
        char* end;
        while ((FxSprayList_GetEndIterator(&end, (char*)&AsCFxSpray(self)->particleCount),
            FxSprayIterator_NotEquals((unsigned int*)&it, (unsigned int*)&end))) {
            {
                char* p = FxSprayListIterator_GetValueAddress(&it);
                void* obj = *(void**)p;
                if (obj) {
                    ((FxVObj*)obj)->v0(1);
                }
            }
            FxSprayListIterator_Advance(&it);
        }
        FxSprayList_ClearNodes((char*)&AsCFxSpray(self)->particleCount);
        void* resourceId = AsCFxSpray(self)->resourceId;
        if (resourceId) {
            void* a = AsCFxSpray(self)->effectOwner;
            void* b = ((SprayOwnerView*)a)->resourceManager;
            if (b) {
                ResourceManager_ReleaseId(b, resourceId);
                AsCFxSpray(self)->resourceId = 0;
            }
        }
        __dt__13GeneTexBufferFv(((SprayTextureBufferView*)self)->textureBuffer, -1);
        FxSprayConfig_dtor((char*)&AsCFxSpray(self)->particlesFollowEmitter, -1);
        ParticleList_dtor((char*)&AsCFxSpray(self)->particleCount, -1);
        if (shouldFree > 0) __dl__FPv(self);
    }
    return self;
}
#pragma opt_loop_invariants reset
#pragma opt_common_subs reset

void appFreeUni__FPv(void*);
void* __as__7WVectorFRC7WVector(void*, const void*);
void Matrix_SetIdentityAffine(void*);
void Vec3_Zero(void*);

extern const float kFloat_0_FxSpray;
extern const float kFloat_1_FxSpray;
extern const float kFloat_neg1000_FxSpray;
extern const float kFloat_50_FxSpray;

#pragma dont_inline off
// Detach the whole initialized ring, free each node, and preserve its sentinel.
// Payloads must be inline values or already destroyed; no payload destructor runs.
static inline void ClearSprayTrackNodes(char*& sentinel, char*& node, unsigned int& count) {
    if (node != sentinel) {
        char* head = *(char**)sentinel;
        *(char**)(*(char**)node + 4) = *(char**)(head + 4);
        *(char**)(*(char**)(head + 4)) = *(char**)node;
        while (node != sentinel) {
            char* cur = node;
            node = *(char**)(node + 4);
            appFreeUni__FPv(cur);
            --count;
        }
    }
}

// Spatial roles follow the offset-checked SpraySpatialView in fx_spray_particles.cpp.
// The volume pair is min/max for box emission, center/radii for spherical
// emission. Matrix direction and world-unit scale are not established here.
// The 268..28c vectors and opaque word at +8 remain unresolved.
enum SprayResetStorageOffset {
    kResetOpaqueWord = 8, kResetShakeOffset = 0x94, kResetShakeCenter = 0xa0,
    kResetPosition = 0xb8, kResetVelocity = 0xd0,
    kResetParentRotation = 0xe0, kResetModelRotation = 0x110, kResetVelocityRotation = 0x140,
    kResetVolumeMinimumOrCenter = 0x1a8, kResetVolumeMaximumOrRadii = 0x1b4, kResetAcceleration = 0x1e4,
    kResetMinimumVelocity = 0x22c, kResetMaximumVelocity = 0x238, kResetWindVector = 0x24c,
    kResetVector268 = 0x268, kResetVector274 = 0x274,
    kResetVector280 = 0x280, kResetVector28C = 0x28c,
    kParticleRingSentinel = (int)&((CFxSpray*)0)->particleCount + 4,
    kAngularSpeedRingSentinel = (int)&((CFxSpray*)0)->angularSpeedTrackCount + 4,
    kVelocityRotationRingSentinel = (int)&((CFxSpray*)0)->velocityRotationTrackCount + 4,
    kParticleOrbitRingSentinel = (int)&((CFxSpray*)0)->particleOrbitTrackCount + 4,
    kEmitterOrbitRingSentinel = (int)&((CFxSpray*)0)->emitterOrbitTrackCount + 4,
    kSpawnRingSentinel = (int)&((CFxSpray*)0)->spawnTrackCount + 4,
    kGravityPointRingSentinel = (int)&((CFxSpray*)0)->gravityPointTrackCount + 4,
    kColorRingSentinel = (int)&((CFxSpray*)0)->colorTrackCount + 4,
    kSizeRingSentinel = (int)&((CFxSpray*)0)->sizeTrackCount + 4,
    kSpriteFrameRingSentinel = (int)&((CFxSpray*)0)->spriteFrameTrackCount + 4
};

// Reset-only defaults: lifetime's tick scale and blend encoding remain unresolved.
enum SprayResetDefaults { kDefaultSprayLifetime = 1000, kDefaultSprayBlend = 0x00800000,
                          kWhiteSprayLighting = 0x00ffffff };

// Restore emitter defaults, clear live particles and reset selected parameter tracks.
// Requires constructed list sentinels. Particle objects are destroyed before their
// nodes; parameter tracks own only node allocations. Some track lists are left
// untouched by this reset: do not infer that this clears every configuration list.
// The old resourceId is zeroed without release here; this is existing reset behavior.
void CFxSpray_Reset(void* self) {
    float zeroPosition[3];
    char* p = (char*)self;
    float zeroVelocity[3];
    float zeroEmissionVolume[3];
    float zeroAcceleration[3];
    float zeroVelocityRange[3];
    float zeroVector268And274[3];

    // Phase 1: playback flags, elapsed time, transforms and spatial state.
    AsCFxSpray(p)->unk_4 = 1;
    AsCFxSpray(p)->drawEnabled = 1;
    *(unsigned int*)(kResetOpaqueWord + p) = 0;
    AsCFxSpray(p)->parentSequence = 0;
    AsCFxSpray(p)->elapsedTime = kFloat_0_FxSpray;
    zeroPosition[0] = kFloat_0_FxSpray; zeroPosition[1] = kFloat_0_FxSpray; zeroPosition[2] = kFloat_0_FxSpray;
    __as__7WVectorFRC7WVector(p + kResetPosition, zeroPosition);
    zeroVelocity[0] = kFloat_0_FxSpray; zeroVelocity[1] = kFloat_0_FxSpray; zeroVelocity[2] = kFloat_0_FxSpray;
    __as__7WVectorFRC7WVector(p + kResetVelocity, zeroVelocity);
    Matrix_SetIdentityAffine(p + kResetParentRotation);
    Matrix_SetIdentityAffine(p + kResetModelRotation);
    Matrix_SetIdentityAffine(p + kResetVelocityRotation);
    Vec3_Zero(p + kResetShakeOffset);
    Vec3_Zero(p + kResetShakeCenter);
    AsCFxSpray(p)->flashOpacity = 0;
    AsCFxSpray(p)->positionIsWorldSpace = 0;
    AsCFxSpray(p)->viewCullingEnabled = 0;
    // Phase 2: destroy particle payload objects, then detach/free their list nodes.
    {
        char* sentinel = p + kParticleRingSentinel;
        char* node = AsCFxSpray(p)->particleListHead;
        while (node != sentinel) {
            void* obj = *(void**)(node + 8);
            if (obj) { ((FxVObj*)obj)->v0(1); }
            node = *(char**)((node + 4));
        }
    }
    {
        char* sentinel;
        char* node;
        node = AsCFxSpray(p)->particleListHead;
        sentinel = p + kParticleRingSentinel;
        if (node != sentinel) {
            char* head = *(char**)sentinel;
            *(char**)(*(char**)node + 4) = *(char**)(head + 4);
            *(char**)(*(char**)(head + 4)) = *(char**)node;
            while (node != sentinel) {
                char* cur = node;
                node = *(char**)(node + 4);
                appFreeUni__FPv(cur);
                --AsCFxSpray(p)->particleCount;
            }
        }
    }
    // Phase 3: emission volume, lifetime, collision and appearance defaults.
    AsCFxSpray(p)->sphericalEmission = 0;
    zeroEmissionVolume[0] = kFloat_0_FxSpray; zeroEmissionVolume[1] = kFloat_0_FxSpray; zeroEmissionVolume[2] = kFloat_0_FxSpray;
    {
        void* t = __as__7WVectorFRC7WVector(p + kResetVolumeMaximumOrRadii, zeroEmissionVolume);
        __as__7WVectorFRC7WVector(p + kResetVolumeMinimumOrCenter, t);
    }
    AsCFxSpray(p)->regenerateAtSamePosition = 0;
    AsCFxSpray(p)->cameraOffset = kFloat_0_FxSpray;
    AsCFxSpray(p)->billboardEnabled = 1;
    AsCFxSpray(p)->spriteResource = 0;
    AsCFxSpray(p)->petResourceName = 0;
    AsCFxSpray(p)->petLightingEnabled = 0;
    AsCFxSpray(p)->particlesFollowEmitter = 0;
    AsCFxSpray(p)->attachedToParent = 0;
    AsCFxSpray(p)->parentRotationAxisMask = 0;
    AsCFxSpray(p)->maxLifetime = kDefaultSprayLifetime;
    AsCFxSpray(p)->minLifetime = kDefaultSprayLifetime;
    AsCFxSpray(p)->opacityMultiplier = kFloat_1_FxSpray;
    AsCFxSpray(p)->volumeCoreScaleY = kFloat_1_FxSpray;
    AsCFxSpray(p)->volumeCoreScaleX = kFloat_1_FxSpray;
    AsCFxSpray(p)->friction = kFloat_0_FxSpray;
    AsCFxSpray(p)->groundFriction = kFloat_0_FxSpray;
    zeroAcceleration[0] = kFloat_0_FxSpray; zeroAcceleration[1] = kFloat_0_FxSpray; zeroAcceleration[2] = kFloat_0_FxSpray;
    __as__7WVectorFRC7WVector(p + kResetAcceleration, zeroAcceleration);
    AsCFxSpray(p)->groundHeightOffset = kFloat_neg1000_FxSpray;
    AsCFxSpray(p)->groundRestitution = kFloat_0_FxSpray;
    AsCFxSpray(p)->groundCollisionRadius = kFloat_0_FxSpray;
    AsCFxSpray(p)->spriteBlendMode = kDefaultSprayBlend;
    AsCFxSpray(p)->trailBlendMode = kDefaultSprayBlend;
    AsCFxSpray(p)->colorModulationEnabled = 0;
    AsCFxSpray(p)->lightingColor = kWhiteSprayLighting;
    zeroVelocityRange[0] = kFloat_0_FxSpray; zeroVelocityRange[1] = kFloat_0_FxSpray; zeroVelocityRange[2] = kFloat_0_FxSpray;
    {
        void* t = __as__7WVectorFRC7WVector(p + kResetMaximumVelocity, zeroVelocityRange);
        __as__7WVectorFRC7WVector(p + kResetMinimumVelocity, t);
    }
    AsCFxSpray(p)->maxWindScale = kFloat_0_FxSpray;
    AsCFxSpray(p)->minWindScale = kFloat_0_FxSpray;
    Vec3_Zero(p + kResetWindVector);
    AsCFxSpray(p)->externalOrientationAxisMask = 0;
    zeroVector268And274[0] = kFloat_0_FxSpray; zeroVector268And274[1] = kFloat_0_FxSpray; zeroVector268And274[2] = kFloat_0_FxSpray;
    {
        void* t = __as__7WVectorFRC7WVector(p + kResetVector274, zeroVector268And274);
        __as__7WVectorFRC7WVector(p + kResetVector268, t);
    }
    float zeroVector280And28C[3]; zeroVector280And28C[0] = kFloat_0_FxSpray; zeroVector280And28C[1] = kFloat_0_FxSpray; zeroVector280And28C[2] = kFloat_0_FxSpray;
    {
        void* t = __as__7WVectorFRC7WVector(p + kResetVector28C, zeroVector280And28C);
        __as__7WVectorFRC7WVector(p + kResetVector280, t);
    }
    AsCFxSpray(p)->maxInitialFrame = 0;
    AsCFxSpray(p)->minInitialFrame = 0;
    AsCFxSpray(p)->maxInitialSize = kFloat_1_FxSpray;
    AsCFxSpray(p)->minInitialSize = kFloat_1_FxSpray;
    AsCFxSpray(p)->spriteAspectRatio = kFloat_1_FxSpray;
    AsCFxSpray(p)->trailEndAlpha = 0;
    AsCFxSpray(p)->trailEndRed = 0xff;
    AsCFxSpray(p)->trailEndGreen = 0xff;
    AsCFxSpray(p)->trailEndBlue = 0xff;
    AsCFxSpray(p)->trailSegmentCount = 0;
    AsCFxSpray(p)->trailSampleInterval = kFloat_0_FxSpray;
    (*(unsigned int*)&AsCFxSpray(p)->resourceId) = 0;
    AsCFxSpray(p)->trailWidthScale = kFloat_50_FxSpray;
    AsCFxSpray(p)->trailMonolithEnabled = 0;
    AsCFxSpray(p)->trailCameraSideEnabled = 0;
    // Phase 4: clear nine selected tracks. ClearSprayTrackNodes detaches the
    // ring and frees nodes; the self-linked sentinel and count zero remain.
    // Angular speed.
    {
        char* sentinel = p + kAngularSpeedRingSentinel;
        char* node = AsCFxSpray(p)->angularSpeedTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->angularSpeedTrackCount);
    }
    // Velocity rotation.
    {
        char* sentinel = p + kVelocityRotationRingSentinel;
        char* node = AsCFxSpray(p)->velocityRotationTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->velocityRotationTrackCount);
    }
    // Particle orbit.
    {
        char* sentinel = p + kParticleOrbitRingSentinel;
        char* node = AsCFxSpray(p)->particleOrbitTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->particleOrbitTrackCount);
    }
    // Emitter orbit.
    {
        char* sentinel = p + kEmitterOrbitRingSentinel;
        char* node = AsCFxSpray(p)->emitterOrbitTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->emitterOrbitTrackCount);
    }
    // Spawn.
    {
        char* sentinel = p + kSpawnRingSentinel;
        char* node = AsCFxSpray(p)->spawnTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->spawnTrackCount);
    }
    // Gravity point.
    {
        char* sentinel = p + kGravityPointRingSentinel;
        char* node = AsCFxSpray(p)->gravityPointTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->gravityPointTrackCount);
    }
    // Color.
    {
        char* sentinel = p + kColorRingSentinel;
        char* node = AsCFxSpray(p)->colorTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->colorTrackCount);
    }
    // Size.
    {
        char* sentinel = p + kSizeRingSentinel;
        char* node = AsCFxSpray(p)->sizeTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->sizeTrackCount);
    }
    // Sprite frame.
    {
        char* sentinel = p + kSpriteFrameRingSentinel;
        char* node = AsCFxSpray(p)->spriteFrameTrackHead;
        ClearSprayTrackNodes(sentinel, node, AsCFxSpray(p)->spriteFrameTrackCount);
    }
    // Phase 5: restore distance and circular-emission defaults after track cleanup.
    AsCFxSpray(p)->circularEmission = 0;
    AsCFxSpray(p)->maxDrawDistance = kFloat_0_FxSpray;
    AsCFxSpray(p)->minDrawDistance = kFloat_0_FxSpray;
    AsCFxSpray(p)->emissionAngle = kFloat_0_FxSpray;
}

#pragma dont_inline on

// Detach and free inline-value list nodes; pointed-to payloads must already be released.
void FxSprayList_ClearNodes(void* self) {
    char* sentinel = (char*)&((SprayListView*)self)->previous;
    char* node = ((SprayListView*)self)->next;
    // Original 0x80111300: detach and free a nonempty list.
    if (node != sentinel) {
        {
            char* link = *(char**)sentinel;
            *(char**)(*(char**)node + 4) = *(char**)(link + 4);
            *(char**)(*(char**)(link + 4)) = *(char**)node;
        }
        while (node != sentinel) {
            char* current = node;
            node = *(char**)(node + 4);
            appFreeUni__FPv(current);
            --((SprayListView*)self)->count;
        }
    }
}

void __dl__FPv(void*);

// Clear an owned list and optionally delete its storage.
void* dtor_80110A50(void* self, int shouldFree) {
    if (self != 0) {
        char* sentinel = (char*)&((SprayListView*)self)->previous;
        char* node = ((SprayListView*)self)->next;
        // Original 0x80110A50: the empty list only needs optional deallocation.
        if (node != sentinel) {
            {
                char* link = *(char**)sentinel;
                *(char**)(*(char**)node + 4) = *(char**)(link + 4);
                *(char**)(*(char**)(link + 4)) = *(char**)node;
            }
            while (node != sentinel) {
                char* current = node;
                node = *(char**)(node + 4);
                appFreeUni__FPv(current);
                --((SprayListView*)self)->count;
            }
        }
        if (shouldFree > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma dont_inline off
struct CleanupList {
    unsigned int count;
    char* links[2];
    // Release this initialized ring's nodes; the embedded sentinel is never freed.
    ~CleanupList() {
        char* node;
        char* sentinel;
        sentinel = (char*)&links[0];
        node = links[1];
        if (node != sentinel) {
            char* link = *(char**)sentinel;
            *(char**)(*(char**)node + 4) = *(char**)(link + 4);
            *(char**)(*(char**)(link + 4)) = *(char**)node;
            while (node != sentinel) {
                char* cur = node;
                node = *(char**)(node + 4);
                appFreeUni__FPv(cur);
                --count;
            }
        }
    }
};
// Preserve each destructor layer's exception-cleanup boundary, with no additional work.
struct CleanupBase : CleanupList { ~CleanupBase() {} };
// Forward destruction to the list base.
struct CleanupMember : CleanupBase { ~CleanupMember() {} };
// Forward destruction through the original outer wrapper.
struct CleanupOuter : CleanupMember { ~CleanupOuter() {} };
// Destroy one configuration list selected by its count-word offset from the config base.
static inline void destroyConfigTrack(char* config, int countOffset) {
    ((CleanupOuter*)(config + countOffset))->~CleanupOuter();
}
// Configuration-relative count offsets: config begins at full spray +170.
// Reserved names identify full-object offsets with unresolved quantities. Reset
// leaves these seven lists untouched, including the initial-speed percentage ring.
enum SprayConfigTrackOffset {
    kConfigReserved354Track = 0x1e4,
    kConfigReserved348Track = 0x1d8,
    kConfigReserved33CTrack = 0x1cc,
    kConfigReserved328Track = 0x1b8,
    kConfigReserved31CTrack = 0x1ac,
    kConfigSpriteFrameTrack = 0x1a0,
    kConfigSizeTrack = 0x194,
    kConfigColorTrack = 0x17c,
    kConfigGravityPointTrack = 0x170,
    kConfigSpawnTrack = 0x164,
    kConfigReserved2C8Track = 0x158,
    kConfigEmitterOrbitTrack = 0x14c,
    kConfigParticleOrbitTrack = 0x140,
    kConfigVelocityRotationTrack = 0x134,
    kConfigAngularSpeedTrack = 0x128,
    // Full spray +25C corresponds to config +EC (config starts at +170).
    // Same ring as constructor emissionTrack and update speedTrackTail/Head.
    // fx_spray_particles.cpp CFxSpray_EmitScheduledParticles samples it at the emitter frame;
    // CFxSpray_InitializeParticle multiplies initial velocity by that value / 100.
    // Thus values are INITIAL SPEED PERCENTAGES, not emission counts or a per-tick speed curve.
    kConfigInitialSpeedPercentTrack = 0xec
};
// Destroy all sixteen configuration lists in reverse member order; optionally free config.
void* FxSprayConfig_dtor(void* self, int shouldFree) {
    if (self) {
        char* p = (char*)self;
        destroyConfigTrack(p, kConfigReserved354Track);
        destroyConfigTrack(p, kConfigReserved348Track);
        destroyConfigTrack(p, kConfigReserved33CTrack);
        destroyConfigTrack(p, kConfigReserved328Track);
        destroyConfigTrack(p, kConfigReserved31CTrack);
        destroyConfigTrack(p, kConfigSpriteFrameTrack);
        destroyConfigTrack(p, kConfigSizeTrack);
        destroyConfigTrack(p, kConfigColorTrack);
        destroyConfigTrack(p, kConfigGravityPointTrack);
        destroyConfigTrack(p, kConfigSpawnTrack);
        destroyConfigTrack(p, kConfigReserved2C8Track);
        destroyConfigTrack(p, kConfigEmitterOrbitTrack);
        destroyConfigTrack(p, kConfigParticleOrbitTrack);
        destroyConfigTrack(p, kConfigVelocityRotationTrack);
        destroyConfigTrack(p, kConfigAngularSpeedTrack);
        destroyConfigTrack(p, kConfigInitialSpeedPercentTrack);
        if (shouldFree > 0) __dl__FPv(self);
    }
    return self;
}
#pragma dont_inline on

} // extern "C"
