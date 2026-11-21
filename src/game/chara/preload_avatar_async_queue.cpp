// Translation unit 0x8026D98C-0x8026E834 (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "iff/IffStruct.h"

struct PetPathStringPool {
    char petExtension[6], vectorLengthError[20], lengthErrorType[55];
    char storyExtension[7], menuExtension[6], swingExtension[7];
    char commendationExtension[14], animationPetExtension[6], geometryExtension[5];
};
enum RequestVectorGrowth {
    kThreeFifthsLimit = 0x15555555, kHalfLimit = 0x2AAAAAAA,
    kPointerBytes = sizeof(void*)
};

extern "C" {

typedef unsigned int u32;
typedef signed char s8;
typedef unsigned char u8;
// The argument points at the preload-request vector subobject, not the loader base.
struct PtrVec { void* vtable; void** requests; u32 size; u32 capacity; };
// Reference-counted owner for an exception message allocation.
struct AllocTag { void* vtable; int strongReferences; int weakReferences; char* text; };
// Request enqueue contract shared by all three entry points below:
// self is the vector subobject, character is a borrowed IFF record. Success (1)
// appends one owned async request; an empty texture filename is also success but
// appends nothing. A null factory result returns 0. Vector overflow throws.
// All copies move pointer slots, then clear the old slots; no request refcount
// increment occurs. The replacement guard frees storage only, not request objects.
// Local H/E/G definitions preserve compiler unwind identities. Operations use
// ExceptionMessageArrayGuard, PreloadVectorLengthError and RequestPointerArrayGuard.
// The final exchange swaps pointer-array ownership without releasing requests.
extern void* CPreloadTextureAsync_Create(s8*);
extern void String_ReserveCapacity(void*, u32);
extern void String_ReplaceRange(void*, u32, u32, const char*, const char*, char*);
extern void* CPreloadFileAsync_Create(const char*);
extern void* __nwa__FUl(u32);
extern void* appMallocDummy__FUl(u32);
extern void appFreeUni__FPv(void*);
extern void* memcpy(void*, const void*, u32);
extern void* memset(void*, int, u32);
extern void* __nw__FUl(u32);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv(void*);
extern char petPathStrings[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];

struct Str {
    union {
        unsigned int flag;
        unsigned char small[4];
    };
    unsigned int length;
    char* data;
};
struct AllocChar { char c __attribute__((aligned(4))); };

#pragma use_lmw_stmw on
// Append a geometry-file preload for the character's model, returning zero on creation failure.
int CPreloadAvatarAsync_QueueModelFiles(PtrVec* self, const IFF_STRUCT::sChar* characterRecord) {
    int pathLength;
    const IFF_STRUCT::sChar* character;
    const char* geometryExtension;
    // Guard an exception-message array until its shared owner takes responsibility.
    struct H {
        char* allocation; char* allocatorTag;
        // Own x until its shared message owner is installed; y is a borrowed allocator tag.
        H(char* x, char* y) : allocation(x), allocatorTag(y) {}
        // Free an exception-message array whose ownership was not transferred.
        ~H() { if (allocation) __dla__FPv(allocation); }
    };
    typedef H ExceptionMessageArrayGuard;
    // ABI-compatible std::length_error payload; construction order controls cleanup.
    struct E {
        void* vtable; char* message; AllocTag* messageOwner;
        // Adopt the message bytes in a shared owner while guarding allocation failure.
        void makeTag(char* ownedMessage, char* allocatorTag) {
            ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
            AllocTag* tag = (AllocTag*)__nw__FUl(sizeof(AllocTag));
            if (tag) { tag->strongReferences=1; tag->weakReferences=1; tag->vtable=s_sharedCharDeleterVtable; tag->text=ownedMessage; }
            messageOwner = tag;
            messageGuard.allocation = 0;
        }
    };
    // Phase 1: build model-name + geometry extension in a temporary small string.
    // High flag bit selects heap storage; otherwise byte 0 is length and text is at +1.
    Str path = {};
    typedef E PreloadVectorLengthError;
    PreloadVectorLengthError firstError;
    PreloadVectorLengthError lastError;
    PreloadVectorLengthError growthError;
    character = characterRecord;
    pathLength = strlen(character->model);
    String_ReserveCapacity(&path, pathLength);
    // Preserve the grouped addition used by the original string-range constructor.
    const char* end = (const char*)character + (pathLength + IFF_OFFSET(IFF_STRUCT::sChar, model));
    {
        AllocChar extra;
        AllocChar copy = extra;
        String_ReplaceRange(&path, 0, 0, character->model, end, (char*)&copy);
    }
    geometryExtension = petPathStrings + IFF_OFFSET(PetPathStringPool, geometryExtension);
    if ((path.flag >> 31) == 0) pathLength = path.small[0] & 0x7f;
    else pathLength = path.length;
    {
        AllocChar extra;
        AllocChar copy = extra;
        String_ReplaceRange(&path, pathLength, 0, geometryExtension, geometryExtension + strlen(geometryExtension), (char*)&copy);
    }
    const char* key;
    if ((path.flag >> 31) == 0) key = (const char*)&path + 1;
    else key = path.data;
    // Phase 2: the factory must capture the path before this temporary is released.
    void* request = CPreloadFileAsync_Create(key);
    if (!request) {
        if (path.flag >> 31) appFreeUni__FPv(path.data);
        return 0;
    }
    char firstMsg[1];
    char lastMsg[1];
    char growthMsg[1];
    const u32 maxRequests = 0x3fffffff; // Largest pointer vector allowed by the allocator.
    // Phase 3: append in spare storage, or reserve a replacement before committing.
    u32 requestCount = self->size;
    u32 capacity = self->capacity;
    if (requestCount < capacity) {
        self->size = requestCount + 1;
        self->requests[self->size - 1] = request;
    } else {
    if (maxRequests - self->capacity < 1) {
        firstMsg[0] = 0;
        char* allocatorTag = firstMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = firstError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        {
        ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
        AllocTag* messageOwner = (AllocTag*)__nw__FUl(sizeof(AllocTag));
        if (messageOwner) { messageOwner->strongReferences=1; messageOwner->weakReferences=1; messageOwner->vtable=s_sharedCharDeleterVtable; messageOwner->text=ownedMessage; }
        error.messageOwner = messageOwner;
        messageGuard.allocation = 0;
        }
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Growth transaction: the array guard owns pointer storage, size counts
    // constructed slots, remaining is the prefix insertion cursor, and
    // ownerCapacity borrows the destination capacity word (never freed).
    struct G {
        void** begin; u32 size; u32 capacity; u32* ownerCapacity; u32 remaining;
        // Release only backing storage; transferred request pointers are never deleted here.
        ~G() { if (begin) { size = 0; appFreeUni__FPv(begin); } }
    };
    typedef G RequestPointerArrayGuard;
    RequestPointerArrayGuard replacement;
    u32 threeFifthsIncrement;
    u32 halfIncrement;
    u32 oldSize;
    u32 oldCapacity;
    u32* capacityPtr;
    u32 zero;
    capacityPtr = &self->capacity;
    oldSize = self->size;
    oldCapacity = *capacityPtr;
    zero = 0;
    u32 requiredAdditionalSlots = oldSize + 1 - oldCapacity;
    replacement.begin = 0;
    u32 newCapacity = *capacityPtr;
    replacement.size = zero;
    replacement.capacity = zero;
    replacement.ownerCapacity = capacityPtr;
    replacement.remaining = zero;
    if (requiredAdditionalSlots > maxRequests - newCapacity) {
        growthMsg[0] = 0;
        char* allocatorTag = growthMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = growthError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        error.makeTag(ownedMessage, allocatorTag);
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Phase 3a: pick a representable capacity, throwing the length-error payload on overflow.
    if (newCapacity < kThreeFifthsLimit) {
        threeFifthsIncrement = ((newCapacity + 1) * 3) / 5;
        u32* growthIncrement = threeFifthsIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &threeFifthsIncrement;
        newCapacity = newCapacity + *growthIncrement;
    } else if (newCapacity < kHalfLimit) {
        halfIncrement = (newCapacity + 1) / 2;
        u32* growthIncrement = halfIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &halfIncrement;
        newCapacity = newCapacity + *growthIncrement;
    } else {
        newCapacity = maxRequests;
    }
    if (newCapacity > maxRequests) {
        lastMsg[0] = 0;
        char* allocatorTag = lastMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = lastError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        {
        ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
        AllocTag* messageOwner = (AllocTag*)__nw__FUl(sizeof(AllocTag));
        if (messageOwner) { messageOwner->strongReferences=1; messageOwner->weakReferences=1; messageOwner->vtable=s_sharedCharDeleterVtable; messageOwner->text=ownedMessage; }
        error.messageOwner = messageOwner;
        messageGuard.allocation = 0;
        }
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Phase 3b: append the new request, then move/clear existing pointer slots.
    void** newRequests = (void**)appMallocDummy__FUl(newCapacity * kPointerBytes);
    u32 previousRequestCount = self->size;
    replacement.remaining = previousRequestCount;
    replacement.begin = newRequests;
    replacement.capacity = newCapacity;
    (newRequests + previousRequestCount)[replacement.size++] = request;
    void** oldSlots = self->requests;
    int end = (int)(oldSlots + self->size);
    int movedCount = (end - (int)oldSlots) / kPointerBytes;
    replacement.remaining = previousRequestCount - movedCount;
    memcpy(newRequests + (previousRequestCount - movedCount), oldSlots, movedCount * kPointerBytes);
    memset(oldSlots, 0, movedCount * kPointerBytes);
    replacement.size = replacement.size + movedCount;
    self->size = 0;
    // Phase 3c: commit the replacement; its guard now owns only the cleared old array.
    u32 oldCapacityValue = self->capacity;
    self->capacity = replacement.capacity;
    replacement.capacity = oldCapacityValue;
    void** oldRequests = self->requests;
    self->requests = replacement.begin;
    replacement.begin = oldRequests;
    self->size = replacement.size;
    replacement.size = 0;
    }
    if (path.flag >> 31) appFreeUni__FPv(path.data);
    return 1;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Queue the character's face texture when its filename is nonempty.
int CPreloadAvatarAsync_QueueFaceTexture(PtrVec* requests, const IFF_STRUCT::sChar* character) {
    // Guard an exception-message array until its shared owner takes responsibility.
    struct H {
        char* allocation; char* allocatorTag;
        // Own x until its shared message owner is installed; y is a borrowed allocator tag.
        H(char* x, char* y) : allocation(x), allocatorTag(y) {}
        // Free an exception-message array whose ownership was not transferred.
        ~H() { if (allocation) __dla__FPv(allocation); }
    };
    typedef H ExceptionMessageArrayGuard;
    // ABI-compatible std::length_error payload; construction order controls cleanup.
    struct E {
        void* vtable; char* message; AllocTag* messageOwner;
        // Adopt the message bytes in a shared owner while guarding allocation failure.
        void makeTag(char* ownedMessage, char* allocatorTag) {
            ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
            AllocTag* tag = (AllocTag*)__nw__FUl(sizeof(AllocTag));
            if (tag) { tag->strongReferences=1; tag->weakReferences=1; tag->vtable=s_sharedCharDeleterVtable; tag->text=ownedMessage; }
            messageOwner = tag;
            messageGuard.allocation = 0;
        }
    };
    typedef E PreloadVectorLengthError;
    PreloadVectorLengthError firstError;
    PreloadVectorLengthError lastError;
    PreloadVectorLengthError growthError;
    char firstMsg[1];
    char lastMsg[1];
    char growthMsg[1];
    // Phase 1: empty face texture is optional success; otherwise create a texture request.
    s8 firstFilenameByte = (s8)character->faceTexture[0];
    PtrVec* self = requests;
    if (firstFilenameByte == 0) return 1;
    void* request = CPreloadTextureAsync_Create((s8*)&character->faceTexture[0]);
    if (!request) return 0;
    const u32 maxRequests = 0x3fffffff; // Largest pointer vector allowed by the allocator.
    u32 size = self->size;
    u32 capacity = self->capacity;
    // Phase 2: append directly if there is room; all following code is reserve machinery.
    if (size < capacity) {
        self->size = size + 1;
        self->requests[self->size - 1] = request;
    } else {
    if (maxRequests - self->capacity < 1) {
        firstMsg[0] = 0;
        char* allocatorTag = firstMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = firstError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        {
        ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
        AllocTag* messageOwner = (AllocTag*)__nw__FUl(sizeof(AllocTag));
        if (messageOwner) { messageOwner->strongReferences=1; messageOwner->weakReferences=1; messageOwner->vtable=s_sharedCharDeleterVtable; messageOwner->text=ownedMessage; }
        error.messageOwner = messageOwner;
        messageGuard.allocation = 0;
        }
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Growth transaction: the array guard owns replacement pointer storage, then old storage
    // after the final swap. It never destroys the request pointers in those slots.
    struct G {
        void** begin; u32 size; u32 capacity; u32* ownerCapacity; u32 remaining;
        // Release only backing storage; transferred request pointers are never deleted here.
        ~G() { if (begin) { size = 0; appFreeUni__FPv(begin); } }
    };
    typedef G RequestPointerArrayGuard;
    RequestPointerArrayGuard replacement;
    u32 threeFifthsIncrement;
    u32 halfIncrement;
    u32* capacityPtr = &self->capacity;
    u32 oldSize = self->size;
    u32 oldCapacity = *capacityPtr;
    u32 zero = 0;
    u32 requiredAdditionalSlots = oldSize + 1 - oldCapacity;
    replacement.begin = 0;
    u32 newCapacity = *capacityPtr;
    replacement.size = zero;
    replacement.capacity = zero;
    replacement.ownerCapacity = capacityPtr;
    replacement.remaining = zero;
    if (requiredAdditionalSlots > maxRequests - newCapacity) {
        growthMsg[0] = 0;
        char* allocatorTag = growthMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = growthError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        error.makeTag(ownedMessage, allocatorTag);
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Phase 3a: pick a representable capacity, throwing the length-error payload on overflow.
    if (newCapacity < kThreeFifthsLimit) {
        threeFifthsIncrement = ((newCapacity + 1) * 3) / 5;
        u32* growthIncrement = threeFifthsIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &threeFifthsIncrement;
        newCapacity = newCapacity + *growthIncrement;
    } else if (newCapacity < kHalfLimit) {
        halfIncrement = (newCapacity + 1) / 2;
        u32* growthIncrement = halfIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &halfIncrement;
        newCapacity = newCapacity + *growthIncrement;
    } else {
        newCapacity = maxRequests;
    }
    if (newCapacity > maxRequests) {
        lastMsg[0] = 0;
        char* allocatorTag = lastMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = lastError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        {
        ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
        AllocTag* messageOwner = (AllocTag*)__nw__FUl(sizeof(AllocTag));
        if (messageOwner) { messageOwner->strongReferences=1; messageOwner->weakReferences=1; messageOwner->vtable=s_sharedCharDeleterVtable; messageOwner->text=ownedMessage; }
        error.messageOwner = messageOwner;
        messageGuard.allocation = 0;
        }
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Phase 3b: append the new request, then move/clear existing pointer slots.
    void** newRequests = (void**)appMallocDummy__FUl(newCapacity * kPointerBytes);
    u32 previousRequestCount = self->size;
    replacement.remaining = previousRequestCount;
    replacement.begin = newRequests;
    replacement.capacity = newCapacity;
    (newRequests + previousRequestCount)[replacement.size++] = request;
    void** oldSlots = self->requests;
    int end = (int)(oldSlots + self->size);
    int movedCount = (end - (int)oldSlots) / kPointerBytes;
    replacement.remaining = previousRequestCount - movedCount;
    memcpy(newRequests + (previousRequestCount - movedCount), oldSlots, movedCount * kPointerBytes);
    memset(oldSlots, 0, movedCount * kPointerBytes);
    replacement.size = replacement.size + movedCount;
    self->size = 0;
    // Phase 3c: commit the replacement; its guard now owns only the cleared old array.
    u32 oldCapacityValue = self->capacity;
    self->capacity = replacement.capacity;
    replacement.capacity = oldCapacityValue;
    void** oldRequests = self->requests;
    self->requests = replacement.begin;
    replacement.begin = oldRequests;
    self->size = replacement.size;
    replacement.size = 0;
    }
    return 1;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Queue the character's base texture when its filename is nonempty.
int CPreloadAvatarAsync_QueueBaseTexture(PtrVec* requests, const IFF_STRUCT::sChar* character) {
    // Guard an exception-message array until its shared owner takes responsibility.
    struct H {
        char* allocation; char* allocatorTag;
        // Own x until its shared message owner is installed; y is a borrowed allocator tag.
        H(char* x, char* y) : allocation(x), allocatorTag(y) {}
        // Free an exception-message array whose ownership was not transferred.
        ~H() { if (allocation) __dla__FPv(allocation); }
    };
    typedef H ExceptionMessageArrayGuard;
    // ABI-compatible std::length_error payload; construction order controls cleanup.
    struct E {
        void* vtable; char* message; AllocTag* messageOwner;
        // Adopt the message bytes in a shared owner while guarding allocation failure.
        void makeTag(char* ownedMessage, char* allocatorTag) {
            ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
            AllocTag* tag = (AllocTag*)__nw__FUl(sizeof(AllocTag));
            if (tag) { tag->strongReferences=1; tag->weakReferences=1; tag->vtable=s_sharedCharDeleterVtable; tag->text=ownedMessage; }
            messageOwner = tag;
            messageGuard.allocation = 0;
        }
    };
    typedef E PreloadVectorLengthError;
    PreloadVectorLengthError firstError;
    PreloadVectorLengthError lastError;
    PreloadVectorLengthError growthError;
    char firstMsg[1];
    char lastMsg[1];
    char growthMsg[1];
    // Phase 1: empty base texture is optional success; otherwise create a texture request.
    s8 firstFilenameByte = (s8)character->baseTexture[0];
    PtrVec* self = requests;
    if (firstFilenameByte == 0) return 1;
    void* request = CPreloadTextureAsync_Create((s8*)&character->baseTexture[0]);
    if (!request) return 0;
    const u32 maxRequests = 0x3fffffff; // Largest pointer vector allowed by the allocator.
    u32 size = self->size;
    u32 capacity = self->capacity;
    // Phase 2: append directly if there is room; all following code is reserve machinery.
    if (size < capacity) {
        self->size = size + 1;
        self->requests[self->size - 1] = request;
    } else {
    if (maxRequests - self->capacity < 1) {
        firstMsg[0] = 0;
        char* allocatorTag = firstMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = firstError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        {
        ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
        AllocTag* messageOwner = (AllocTag*)__nw__FUl(sizeof(AllocTag));
        if (messageOwner) { messageOwner->strongReferences=1; messageOwner->weakReferences=1; messageOwner->vtable=s_sharedCharDeleterVtable; messageOwner->text=ownedMessage; }
        error.messageOwner = messageOwner;
        messageGuard.allocation = 0;
        }
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Growth transaction: the array guard owns replacement pointer storage, then old storage
    // after the final swap. It never destroys the request pointers in those slots.
    struct G {
        void** begin; u32 size; u32 capacity; u32* ownerCapacity; u32 remaining;
        // Release only backing storage; transferred request pointers are never deleted here.
        ~G() { if (begin) { size = 0; appFreeUni__FPv(begin); } }
    };
    typedef G RequestPointerArrayGuard;
    RequestPointerArrayGuard replacement;
    u32 threeFifthsIncrement;
    u32 halfIncrement;
    u32* capacityPtr = &self->capacity;
    u32 oldSize = self->size;
    u32 oldCapacity = *capacityPtr;
    u32 zero = 0;
    u32 requiredAdditionalSlots = oldSize + 1 - oldCapacity;
    replacement.begin = 0;
    u32 newCapacity = *capacityPtr;
    replacement.size = zero;
    replacement.capacity = zero;
    replacement.ownerCapacity = capacityPtr;
    replacement.remaining = zero;
    if (requiredAdditionalSlots > maxRequests - newCapacity) {
        growthMsg[0] = 0;
        char* allocatorTag = growthMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = growthError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        error.makeTag(ownedMessage, allocatorTag);
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Phase 3a: pick a representable capacity, throwing the length-error payload on overflow.
    if (newCapacity < kThreeFifthsLimit) {
        threeFifthsIncrement = ((newCapacity + 1) * 3) / 5;
        u32* growthIncrement = threeFifthsIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &threeFifthsIncrement;
        newCapacity = newCapacity + *growthIncrement;
    } else if (newCapacity < kHalfLimit) {
        halfIncrement = (newCapacity + 1) / 2;
        u32* growthIncrement = halfIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &halfIncrement;
        newCapacity = newCapacity + *growthIncrement;
    } else {
        newCapacity = maxRequests;
    }
    if (newCapacity > maxRequests) {
        lastMsg[0] = 0;
        char* allocatorTag = lastMsg;
        char* message = petPathStrings + IFF_OFFSET(PetPathStringPool, vectorLengthError);
        PreloadVectorLengthError& error = lastError;
        error.vtable = __vt__Q23std11logic_error;
        u32 messageBytes = (u32)strlen(message) + 1;
        char* ownedMessage = (char*)__nwa__FUl(messageBytes * 2 - messageBytes);
        error.message = ownedMessage;
        {
        ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
        AllocTag* messageOwner = (AllocTag*)__nw__FUl(sizeof(AllocTag));
        if (messageOwner) { messageOwner->strongReferences=1; messageOwner->weakReferences=1; messageOwner->vtable=s_sharedCharDeleterVtable; messageOwner->text=ownedMessage; }
        error.messageOwner = messageOwner;
        messageGuard.allocation = 0;
        }
        strcpy(error.message, message);
        error.vtable = __vt__Q23std12length_error;
        __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error, (void*)__dt__Q23std12length_errorFv);
    }

    // Phase 3b: append the new request, then move/clear existing pointer slots.
    void** newRequests = (void**)appMallocDummy__FUl(newCapacity * kPointerBytes);
    u32 previousRequestCount = self->size;
    replacement.remaining = previousRequestCount;
    replacement.begin = newRequests;
    replacement.capacity = newCapacity;
    (newRequests + previousRequestCount)[replacement.size++] = request;
    void** oldSlots = self->requests;
    int end = (int)(oldSlots + self->size);
    int movedCount = (end - (int)oldSlots) / kPointerBytes;
    replacement.remaining = previousRequestCount - movedCount;
    memcpy(newRequests + (previousRequestCount - movedCount), oldSlots, movedCount * kPointerBytes);
    memset(oldSlots, 0, movedCount * kPointerBytes);
    replacement.size = replacement.size + movedCount;
    self->size = 0;
    // Phase 3c: commit the replacement; its guard now owns only the cleared old array.
    u32 oldCapacityValue = self->capacity;
    self->capacity = replacement.capacity;
    replacement.capacity = oldCapacityValue;
    void** oldRequests = self->requests;
    self->requests = replacement.begin;
    replacement.begin = oldRequests;
    self->size = replacement.size;
    replacement.size = 0;
    }
    return 1;
}

#pragma use_lmw_stmw reset
} // extern "C"
