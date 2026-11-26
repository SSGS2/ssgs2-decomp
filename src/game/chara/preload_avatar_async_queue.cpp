// Queue an avatar's model file, face texture and base texture as child loaders.
//
// Each entry point creates the request and appends it to the avatar loader's
// list of child preloaders, growing the list when it is full. An empty texture
// name is accepted without queuing anything.
#include "sdk_protos.h"
#include "iff/IffStruct.h"
#include "classes/CPreloadAvatarAsync.h"
#include "classes/CPreloadFileAsync.h"
#include "classes/CPreloadTextureAsync.h"

// The packed pool of resource suffixes and exception texts this file reads.
struct PetPathStringPool {
    char petExtension[6], vectorLengthError[20], lengthErrorType[55];
    char storyExtension[7], menuExtension[6], swingExtension[7];
    char commendationExtension[14], animationPetExtension[6], geometryExtension[5];
};

// Pointer-vector growth limits: three fifths, then one half of the element
// limit, and the limit itself past that.
enum RequestVectorGrowth {
    kThreeFifthsLimit = 0x15555555,
    kHalfLimit = 0x2AAAAAAA,
    kPointerBytes = sizeof(void*)
};

extern "C" {

typedef unsigned int u32;
typedef signed char s8;

// Reference-counted owner for an exception message allocation.
struct AllocTag {
    void* vtable;
    int strongReferences;
    int weakReferences;
    char* text;
};

extern void String_ReserveCapacity(void*, u32);
extern void String_ReplaceRange(void*, u32, u32, const char*, const char*, char*);
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

} // extern "C"

// A small string: the high flag bit selects heap storage, otherwise byte 0
// holds the length and the text follows it.
struct Str {
    union {
        unsigned int flag;
        unsigned char small[4];
    };
    unsigned int length;
    char* data;
};

// Stands in for the allocator tag passed with a string range; only its address
// is used.
struct AllocChar {
    char c __attribute__((aligned(4)));
};

#pragma use_lmw_stmw on
// Append a load of the character's model file, named with the geometry suffix.
int CPreloadAvatarAsync::QueueModelFiles(const IFF_STRUCT::sChar* characterRecord) {
    int pathLength;
    const IFF_STRUCT::sChar* character;
    const char* geometryExtension;
    // Guard an exception-message array until its shared owner takes responsibility.
    struct H {
        char* allocation;
        char* allocatorTag;
        // Own x until its shared message owner is installed; y is a borrowed allocator tag.
        H(char* x, char* y) : allocation(x), allocatorTag(y) {}
        // Free an exception-message array whose ownership was not transferred.
        ~H() {
            if (allocation)
                __dla__FPv(allocation);
        }
    };
    typedef H ExceptionMessageArrayGuard;
    // ABI-compatible std::length_error payload; construction order controls cleanup.
    struct E {
        void* vtable;
        char* message;
        AllocTag* messageOwner;
        // Adopt the message bytes in a shared owner while guarding allocation failure.
        void makeTag(char* ownedMessage, char* allocatorTag) {
            ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
            AllocTag* tag = (AllocTag*)__nw__FUl(sizeof(AllocTag));
            if (tag) {
                tag->strongReferences = 1;
                tag->weakReferences = 1;
                tag->vtable = s_sharedCharDeleterVtable;
                tag->text = ownedMessage;
            }
            messageOwner = tag;
            messageGuard.allocation = 0;
        }
    };
    // Build the model path: the model name followed by the geometry suffix.
    Str path = {};
    typedef E PreloadVectorLengthError;
    PreloadVectorLengthError firstError;
    PreloadVectorLengthError lastError;
    PreloadVectorLengthError growthError;
    character = characterRecord;
    pathLength = strlen(character->model);
    String_ReserveCapacity(&path, pathLength);
    // The two additions stay grouped so the compiler folds them into one address.
    const char* end = (const char*)character + (pathLength + IFF_OFFSET(IFF_STRUCT::sChar, model));
    {
        AllocChar extra;
        AllocChar copy = extra;
        String_ReplaceRange(&path, 0, 0, character->model, end, (char*)&copy);
    }
    geometryExtension = petPathStrings + IFF_OFFSET(PetPathStringPool, geometryExtension);
    if ((path.flag >> 31) == 0)
        pathLength = path.small[0] & 0x7f;
    else
        pathLength = path.length;
    {
        AllocChar extra;
        AllocChar copy = extra;
        String_ReplaceRange(&path, pathLength, 0, geometryExtension, geometryExtension + strlen(geometryExtension),
                            (char*)&copy);
    }
    const char* key;
    if ((path.flag >> 31) == 0)
        key = (const char*)&path + 1;
    else
        key = path.data;
    // The factory reads the path, so the string stays alive until it returns.
    void* request = CPreloadFileAsync::Create((void*)key);
    if (!request) {
        if (path.flag >> 31)
            appFreeUni__FPv(path.data);
        return 0;
    }
    char firstMsg[1];
    char lastMsg[1];
    char growthMsg[1];
    const u32 maxRequests = 0x3fffffff; // Largest pointer vector allowed by the allocator.
    // Append in spare storage, or reserve a replacement array first.
    u32 count = requestCount;
    u32 capacity = requestCapacity;
    if (count < capacity) {
        requestCount = count + 1;
        requests[requestCount - 1] = request;
    } else {
        if (maxRequests - requestCapacity < 1) {
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
                if (messageOwner) {
                    messageOwner->strongReferences = 1;
                    messageOwner->weakReferences = 1;
                    messageOwner->vtable = s_sharedCharDeleterVtable;
                    messageOwner->text = ownedMessage;
                }
                error.messageOwner = messageOwner;
                messageGuard.allocation = 0;
            }
            strcpy(error.message, message);
            error.vtable = __vt__Q23std12length_error;
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Growth transaction: the array guard owns pointer storage, size counts
        // constructed slots, remaining is the prefix insertion cursor, and
        // ownerCapacity borrows the destination capacity word (never freed).
        struct G {
            void** begin;
            u32 size;
            u32 capacity;
            u32* ownerCapacity;
            u32 remaining;
            // Release only backing storage; transferred request pointers are never deleted here.
            ~G() {
                if (begin) {
                    size = 0;
                    appFreeUni__FPv(begin);
                }
            }
        };
        typedef G RequestPointerArrayGuard;
        RequestPointerArrayGuard replacement;
        u32 threeFifthsIncrement;
        u32 halfIncrement;
        u32 oldSize;
        u32 oldCapacity;
        u32* capacityPtr;
        u32 zero;
        capacityPtr = &requestCapacity;
        oldSize = requestCount;
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
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Pick a capacity the allocator can represent; going past the limit throws.
        if (newCapacity < kThreeFifthsLimit) {
            threeFifthsIncrement = ((newCapacity + 1) * 3) / 5;
            u32* growthIncrement =
                threeFifthsIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &threeFifthsIncrement;
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
                if (messageOwner) {
                    messageOwner->strongReferences = 1;
                    messageOwner->weakReferences = 1;
                    messageOwner->vtable = s_sharedCharDeleterVtable;
                    messageOwner->text = ownedMessage;
                }
                error.messageOwner = messageOwner;
                messageGuard.allocation = 0;
            }
            strcpy(error.message, message);
            error.vtable = __vt__Q23std12length_error;
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Append the request, move the existing slots, and clear the old array.
        void** newRequests = (void**)appMallocDummy__FUl(newCapacity * kPointerBytes);
        u32 previousRequestCount = requestCount;
        replacement.remaining = previousRequestCount;
        replacement.begin = newRequests;
        replacement.capacity = newCapacity;
        (newRequests + previousRequestCount)[replacement.size++] = request;
        void** oldSlots = requests;
        int end = (int)(oldSlots + requestCount);
        int movedCount = (end - (int)oldSlots) / kPointerBytes;
        replacement.remaining = previousRequestCount - movedCount;
        memcpy(newRequests + (previousRequestCount - movedCount), oldSlots, movedCount * kPointerBytes);
        memset(oldSlots, 0, movedCount * kPointerBytes);
        replacement.size = replacement.size + movedCount;
        requestCount = 0;
        // Swap the new array in; the guard now owns only the cleared old array.
        u32 oldCapacityValue = requestCapacity;
        requestCapacity = replacement.capacity;
        replacement.capacity = oldCapacityValue;
        void** oldRequests = requests;
        requests = replacement.begin;
        replacement.begin = oldRequests;
        requestCount = replacement.size;
        replacement.size = 0;
    }
    if (path.flag >> 31)
        appFreeUni__FPv(path.data);
    return 1;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Queue the character's face texture when its filename is nonempty.
int CPreloadAvatarAsync::QueueFaceTexture(const IFF_STRUCT::sChar* character) {
    // Guard an exception-message array until its shared owner takes responsibility.
    struct H {
        char* allocation;
        char* allocatorTag;
        // Own x until its shared message owner is installed; y is a borrowed allocator tag.
        H(char* x, char* y) : allocation(x), allocatorTag(y) {}
        // Free an exception-message array whose ownership was not transferred.
        ~H() {
            if (allocation)
                __dla__FPv(allocation);
        }
    };
    typedef H ExceptionMessageArrayGuard;
    // ABI-compatible std::length_error payload; construction order controls cleanup.
    struct E {
        void* vtable;
        char* message;
        AllocTag* messageOwner;
        // Adopt the message bytes in a shared owner while guarding allocation failure.
        void makeTag(char* ownedMessage, char* allocatorTag) {
            ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
            AllocTag* tag = (AllocTag*)__nw__FUl(sizeof(AllocTag));
            if (tag) {
                tag->strongReferences = 1;
                tag->weakReferences = 1;
                tag->vtable = s_sharedCharDeleterVtable;
                tag->text = ownedMessage;
            }
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
    // An empty face texture name is accepted without queuing anything.
    s8 firstFilenameByte = (s8)character->faceTexture[0];
    if (firstFilenameByte == 0)
        return 1;
    void* request = CPreloadTextureAsync::Create((s8*)&character->faceTexture[0]);
    if (!request)
        return 0;
    const u32 maxRequests = 0x3fffffff; // Largest pointer vector allowed by the allocator.
    u32 size = requestCount;
    u32 capacity = requestCapacity;
    // Append in spare storage, or reserve a replacement array first.
    if (size < capacity) {
        requestCount = size + 1;
        requests[requestCount - 1] = request;
    } else {
        if (maxRequests - requestCapacity < 1) {
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
                if (messageOwner) {
                    messageOwner->strongReferences = 1;
                    messageOwner->weakReferences = 1;
                    messageOwner->vtable = s_sharedCharDeleterVtable;
                    messageOwner->text = ownedMessage;
                }
                error.messageOwner = messageOwner;
                messageGuard.allocation = 0;
            }
            strcpy(error.message, message);
            error.vtable = __vt__Q23std12length_error;
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Growth transaction: the array guard owns replacement pointer storage, then old storage
        // after the final swap. It never destroys the request pointers in those slots.
        struct G {
            void** begin;
            u32 size;
            u32 capacity;
            u32* ownerCapacity;
            u32 remaining;
            // Release only backing storage; transferred request pointers are never deleted here.
            ~G() {
                if (begin) {
                    size = 0;
                    appFreeUni__FPv(begin);
                }
            }
        };
        typedef G RequestPointerArrayGuard;
        RequestPointerArrayGuard replacement;
        u32 threeFifthsIncrement;
        u32 halfIncrement;
        u32* capacityPtr = &requestCapacity;
        u32 oldSize = requestCount;
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
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Pick a capacity the allocator can represent; going past the limit throws.
        if (newCapacity < kThreeFifthsLimit) {
            threeFifthsIncrement = ((newCapacity + 1) * 3) / 5;
            u32* growthIncrement =
                threeFifthsIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &threeFifthsIncrement;
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
                if (messageOwner) {
                    messageOwner->strongReferences = 1;
                    messageOwner->weakReferences = 1;
                    messageOwner->vtable = s_sharedCharDeleterVtable;
                    messageOwner->text = ownedMessage;
                }
                error.messageOwner = messageOwner;
                messageGuard.allocation = 0;
            }
            strcpy(error.message, message);
            error.vtable = __vt__Q23std12length_error;
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Append the request, move the existing slots, and clear the old array.
        void** newRequests = (void**)appMallocDummy__FUl(newCapacity * kPointerBytes);
        u32 previousRequestCount = requestCount;
        replacement.remaining = previousRequestCount;
        replacement.begin = newRequests;
        replacement.capacity = newCapacity;
        (newRequests + previousRequestCount)[replacement.size++] = request;
        void** oldSlots = requests;
        int end = (int)(oldSlots + requestCount);
        int movedCount = (end - (int)oldSlots) / kPointerBytes;
        replacement.remaining = previousRequestCount - movedCount;
        memcpy(newRequests + (previousRequestCount - movedCount), oldSlots, movedCount * kPointerBytes);
        memset(oldSlots, 0, movedCount * kPointerBytes);
        replacement.size = replacement.size + movedCount;
        requestCount = 0;
        // Swap the new array in; the guard now owns only the cleared old array.
        u32 oldCapacityValue = requestCapacity;
        requestCapacity = replacement.capacity;
        replacement.capacity = oldCapacityValue;
        void** oldRequests = requests;
        requests = replacement.begin;
        replacement.begin = oldRequests;
        requestCount = replacement.size;
        replacement.size = 0;
    }
    return 1;
}
#pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Queue the character's base texture when its filename is nonempty.
int CPreloadAvatarAsync::QueueBaseTexture(const IFF_STRUCT::sChar* character) {
    // Guard an exception-message array until its shared owner takes responsibility.
    struct H {
        char* allocation;
        char* allocatorTag;
        // Own x until its shared message owner is installed; y is a borrowed allocator tag.
        H(char* x, char* y) : allocation(x), allocatorTag(y) {}
        // Free an exception-message array whose ownership was not transferred.
        ~H() {
            if (allocation)
                __dla__FPv(allocation);
        }
    };
    typedef H ExceptionMessageArrayGuard;
    // ABI-compatible std::length_error payload; construction order controls cleanup.
    struct E {
        void* vtable;
        char* message;
        AllocTag* messageOwner;
        // Adopt the message bytes in a shared owner while guarding allocation failure.
        void makeTag(char* ownedMessage, char* allocatorTag) {
            ExceptionMessageArrayGuard messageGuard(ownedMessage, allocatorTag);
            AllocTag* tag = (AllocTag*)__nw__FUl(sizeof(AllocTag));
            if (tag) {
                tag->strongReferences = 1;
                tag->weakReferences = 1;
                tag->vtable = s_sharedCharDeleterVtable;
                tag->text = ownedMessage;
            }
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
    // An empty base texture name is accepted without queuing anything.
    s8 firstFilenameByte = (s8)character->baseTexture[0];
    if (firstFilenameByte == 0)
        return 1;
    void* request = CPreloadTextureAsync::Create((s8*)&character->baseTexture[0]);
    if (!request)
        return 0;
    const u32 maxRequests = 0x3fffffff; // Largest pointer vector allowed by the allocator.
    u32 size = requestCount;
    u32 capacity = requestCapacity;
    // Append in spare storage, or reserve a replacement array first.
    if (size < capacity) {
        requestCount = size + 1;
        requests[requestCount - 1] = request;
    } else {
        if (maxRequests - requestCapacity < 1) {
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
                if (messageOwner) {
                    messageOwner->strongReferences = 1;
                    messageOwner->weakReferences = 1;
                    messageOwner->vtable = s_sharedCharDeleterVtable;
                    messageOwner->text = ownedMessage;
                }
                error.messageOwner = messageOwner;
                messageGuard.allocation = 0;
            }
            strcpy(error.message, message);
            error.vtable = __vt__Q23std12length_error;
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Growth transaction: the array guard owns replacement pointer storage, then old storage
        // after the final swap. It never destroys the request pointers in those slots.
        struct G {
            void** begin;
            u32 size;
            u32 capacity;
            u32* ownerCapacity;
            u32 remaining;
            // Release only backing storage; transferred request pointers are never deleted here.
            ~G() {
                if (begin) {
                    size = 0;
                    appFreeUni__FPv(begin);
                }
            }
        };
        typedef G RequestPointerArrayGuard;
        RequestPointerArrayGuard replacement;
        u32 threeFifthsIncrement;
        u32 halfIncrement;
        u32* capacityPtr = &requestCapacity;
        u32 oldSize = requestCount;
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
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Pick a capacity the allocator can represent; going past the limit throws.
        if (newCapacity < kThreeFifthsLimit) {
            threeFifthsIncrement = ((newCapacity + 1) * 3) / 5;
            u32* growthIncrement =
                threeFifthsIncrement < requiredAdditionalSlots ? &requiredAdditionalSlots : &threeFifthsIncrement;
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
                if (messageOwner) {
                    messageOwner->strongReferences = 1;
                    messageOwner->weakReferences = 1;
                    messageOwner->vtable = s_sharedCharDeleterVtable;
                    messageOwner->text = ownedMessage;
                }
                error.messageOwner = messageOwner;
                messageGuard.allocation = 0;
            }
            strcpy(error.message, message);
            error.vtable = __vt__Q23std12length_error;
            __throw(petPathStrings + IFF_OFFSET(PetPathStringPool, lengthErrorType), &error,
                    (void*)__dt__Q23std12length_errorFv);
        }

        // Append the request, move the existing slots, and clear the old array.
        void** newRequests = (void**)appMallocDummy__FUl(newCapacity * kPointerBytes);
        u32 previousRequestCount = requestCount;
        replacement.remaining = previousRequestCount;
        replacement.begin = newRequests;
        replacement.capacity = newCapacity;
        (newRequests + previousRequestCount)[replacement.size++] = request;
        void** oldSlots = requests;
        int end = (int)(oldSlots + requestCount);
        int movedCount = (end - (int)oldSlots) / kPointerBytes;
        replacement.remaining = previousRequestCount - movedCount;
        memcpy(newRequests + (previousRequestCount - movedCount), oldSlots, movedCount * kPointerBytes);
        memset(oldSlots, 0, movedCount * kPointerBytes);
        replacement.size = replacement.size + movedCount;
        requestCount = 0;
        // Swap the new array in; the guard now owns only the cleared old array.
        u32 oldCapacityValue = requestCapacity;
        requestCapacity = replacement.capacity;
        replacement.capacity = oldCapacityValue;
        void** oldRequests = requests;
        requests = replacement.begin;
        replacement.begin = oldRequests;
        requestCount = replacement.size;
        replacement.size = 0;
    }
    return 1;
}

#pragma use_lmw_stmw reset
