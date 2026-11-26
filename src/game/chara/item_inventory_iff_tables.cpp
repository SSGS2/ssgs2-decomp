// Item inventory tables read from IFF data: course, hairstyle and auxiliary-part tables.
// Each table is a vector of entries keyed by type ID, with growth and length-error handling.

typedef unsigned long new_size_t;
#include "iff/IffStruct.h"
#include "game/exception.h"
#include <decomp/cw_bitfields.h>

extern "C" {

// Input contract for all three loaders: a borrowed, complete IFF table image
// already in native Wii byte order. First u16 is record count; header is 8 bytes.
// No length/endian/version validation occurs here. Entries copy the serialized
// record and cache common.typeId in a leading word for later lookup.
// self owns a contiguous appMallocDummy allocation; size/capacity count entries.
// Growth stages new slots and copies old entries backwards before swapping
// ownership. A temporary then frees the old allocation; shrinking retains capacity.
// Initial fill temporaries may contain unspecified bytes, overwritten by the
// record-loading phase. These are not semantic defaults to copy into new mods.
enum InventoryTableLayout {
    kIffHeaderBytes = 8,
    kSyntheticCourseCount = 3,
    kCourseRecordBytes = 0x104,
    kCourseEntryBytes = 0x108,
    kHairRecordBytes = 0x60,
    kHairEntryBytes = 0x64,
    kAuxRecordBytes = 0x70,
    kAuxEntryBytes = 0x74,
    kSyntheticCourseFD = 0x280000FD,
    kSyntheticCourseFE = 0x280000FE,
    kSyntheticCourse7F = 0x2800007F
};

struct Record100 {
    unsigned int key;
    unsigned char data[0x60];
};

struct Container {
    unsigned char pad[0xA4];
    Record100* data;
    unsigned int size;
    unsigned int capacity;
};

void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void* appMallocDummy__FUl(unsigned int);
void appFreeUni__FPv(void*);
unsigned int __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);

// Placement new borrows an already allocated entry slot; it allocates nothing.
extern "C++" {
inline void* operator new(new_size_t, void* p) {
    return p;
}
}
typedef IffRecordStorage<IFF_STRUCT::sCourse> CourseRecordStorage;
struct CourseLoadEntry {
    unsigned int typeIdKey;
    IffRecordStorage<IFF_STRUCT::sCourse> record;
    // Copy a course payload, leaving the cached key zero until assignment.
    CourseLoadEntry(const CourseRecordStorage& v) : typeIdKey(0) {
        *(CourseRecordStorage*)&record = v;
    }
    // Copy both cached key and payload during storage growth.
    CourseLoadEntry(const CourseLoadEntry& v) : typeIdKey(v.typeIdKey) {
        *(CourseRecordStorage*)&record = *(const CourseRecordStorage*)&v.record;
    }
};
struct CourseTableLoader {
    unsigned char pad[0x8C];
    CourseLoadEntry* data;
    unsigned int size;
    unsigned int capacity;
};
extern const char iffSchemaAndCourseStrings[];
extern const float kFloat_1_1_PangRate;
char* strcpy(char*, const char*);
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
unsigned int strlen(const char*);
void* __nw__FUl(unsigned long);
void __dla__FPv(void*, int);
void __throw(void*, void*, void (*)()) __attribute__((noreturn));
void __dt__Q23std12length_errorFv();
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct TargetHolder {
    char* p;
    char* flag;
    // Hold exception text until its shared owner has been allocated.
    TargetHolder(char* b, char* f) {
        p = b;
        flag = f;
    }
    // Free exception text when ownership was not handed off.
    ~TargetHolder() {
        if (p)
            __dla__FPv(p, -1);
    }
};
struct TargetRef {
    void* vt;
    int c1;
    int c2;
    char* s;
};
struct TargetErrBase {
    void* vt;
    char* buf;
    TargetRef* ref;
    // Allocate/copy exception text and construct its shared deleter state.
    TargetErrBase(const char* msg) {
        char done __attribute__((aligned(4))) = 0;
        vt = __vt__Q23std11logic_error;
        unsigned n = strlen(msg) + 1;
        char* buffer = (char*)(unsigned int)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            TargetHolder h(buffer, &done);
            TargetRef* r = (TargetRef*)__nw__FUl(16);
            if (r) {
                r->c1 = 1;
                r->c2 = 1;
                r->vt = s_sharedCharDeleterVtable;
                r->s = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
    // Run the target standard exception base destructor.
    ~TargetErrBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
struct TargetErr : TargetErrBase {
    // Set the length-error vtable and throw the completed exception object.
    TargetErr(const char* msg) : TargetErrBase(msg) {
        vt = __vt__Q23std12length_error;
        __throw((void*)(iffSchemaAndCourseStrings + 0x2f9), this, __dt__Q23std12length_errorFv);
    }
    // Derived cleanup is empty; TargetErrBase owns base destruction.
    ~TargetErr() {}
};
struct TargetLength : TargetErrBase {
    // Build a length-error object without throwing it from construction.
    TargetLength(const char* msg) : TargetErrBase(msg) {
        vt = __vt__Q23std12length_error;
    }
    // Leave owned text/base cleanup to the base destructor.
    ~TargetLength() {}
};
// Construct and throw the course vector's length-error object.
inline void targetCapacityError(const char* message) {
    TargetErr error(message);
}
// Return the larger growth bound by reference, as required by the target library.
inline const unsigned int& max108(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
struct Flags108 {
    CW_BITS2(unsigned char hi : 4, unsigned char lo : 4)
};
// Placement-construct a course slot while preserving its unwind boundary.
inline void construct108(CourseLoadEntry* dst, const CourseRecordStorage& value) {
    try {
        new (dst) CourseLoadEntry(value);
    } catch (...) {
        throw;
    }
}
// Copy-construct an old course entry into replacement storage.
inline void copyConstruct108(CourseLoadEntry* dst, const CourseLoadEntry& value) {
    try {
        new (dst) CourseLoadEntry(value);
    } catch (...) {
        throw;
    }
}
// Store a course record and synchronize its cached type-ID key.
inline void assign108(CourseTableLoader* self, int index, const CourseRecordStorage& src) {
    CourseRecordStorage staged = src;
    CourseLoadEntry* dst = self->data + index;
    dst->typeIdKey = src.value.common.typeId;
    *(CourseRecordStorage*)&dst->record = staged;
}
struct TargetGrow {
    CourseLoadEntry* data;
    unsigned int size, capacity;
    unsigned int* capacityRef;
    unsigned int oldsize;
    // Start an empty owner for a replacement course allocation.
    TargetGrow(unsigned int* c) : data(0), size(0), capacity(0), capacityRef(c), oldsize(0) {}
    // Free the allocation retained after growth/swap or exceptional exit.
    ~TargetGrow() {
        if (data) {
            size = size - size;
            appFreeUni__FPv(data);
        }
    }
};

#pragma push
#pragma inline_depth(8)
// Replace the course table and append the three built-in course records.
void ItemInventory_ReadCourseTable(CourseTableLoader* self, const void* source) {
    unsigned char hdr[8];
    memcpy(hdr, source, kIffHeaderBytes);
    const unsigned char* input = (const unsigned char*)source + kIffHeaderBytes;
    // Phase 1: resize for serialized records plus the three synthetic entries.
    if (*(unsigned short*)hdr + kSyntheticCourseCount > self->size) {
        unsigned int need = *(unsigned short*)hdr + kSyntheticCourseCount - self->size;
        // 0x8018D65C: finish the in-place path before the growth block.
        do {
            if (need > self->capacity || self->size > self->capacity - need) {
                if (self->size + need - self->capacity > 0xf83e0f - self->capacity)
                    targetCapacityError(iffSchemaAndCourseStrings + 0x2e5);
            } else {
                CourseLoadEntry* dst = self->data + self->size;
                CourseRecordStorage value0;
                CourseRecordStorage value = value0;
                while (need) {
                    construct108(dst, value);
                    --need;
                    ++dst;
                    ++self->size;
                }
                break;
            }
            {
                TargetGrow grow(&self->capacity);
                unsigned int needed = self->size + need - self->capacity;
                unsigned int oldcap = self->capacity;
                if (needed > 0xf83e0f - oldcap)
                    targetCapacityError(iffSchemaAndCourseStrings + 0x2e5);
                unsigned int cap;
                if (oldcap < 0x52bf5a)
                    cap = oldcap + max108(((oldcap + 1) * 3) / 5, needed);
                else if (oldcap < 0xa57eb4)
                    cap = oldcap + max108((oldcap + 1) / 2, needed);
                else
                    cap = 0xf83e0f;
                if (cap > 0xf83e0f)
                    targetCapacityError(iffSchemaAndCourseStrings + 0x2e5);
                CourseLoadEntry* p = (CourseLoadEntry*)appMallocDummy__FUl(cap * kCourseEntryBytes);
                unsigned int sz = self->size;
                grow.data = p;
                grow.capacity = cap;
                grow.oldsize = sz;
                CourseLoadEntry* dst = p + sz + grow.size;
                CourseRecordStorage value0;
                CourseRecordStorage value = value0;
                while (need) {
                    construct108(dst, value);
                    --need;
                    ++dst;
                    ++grow.size;
                }
                CourseLoadEntry* begin = self->data;
                CourseLoadEntry* o = begin + self->size;
                CourseLoadEntry* d = grow.data + grow.oldsize;
                while (o > begin) {
                    --o;
                    --d;
                    copyConstruct108(d, *o);
                    --grow.oldsize;
                    ++grow.size;
                }
                CourseLoadEntry* old = self->data;
                self->size = 0;
                unsigned int oldcap2 = self->capacity;
                grow.data = old;
                self->capacity = grow.capacity;
                grow.capacity = oldcap2;
                self->data = p;
                self->size = grow.size;
                grow.size = 0;
            }
        } while (0);
    } else if (*(unsigned short*)hdr + kSyntheticCourseCount < self->size) {
        self->size = *(unsigned short*)hdr + kSyntheticCourseCount;
    }
    // Phase 2: overwrite all serialized slots and their cached lookup keys.
    CourseRecordStorage rec;
    int index = 0;
    for (int i = 0; i < *(unsigned short*)hdr; ++i) {
        memcpy(&rec, input, kCourseRecordBytes);
        input += kCourseRecordBytes;
        assign108(self, index, rec);
        ++index;
    }
    // Phase 3: append FD, FE and 7F in that order, resetting the record each time.
    memset(&rec, 0, kCourseRecordBytes);
    rec.value.common.typeId = kSyntheticCourseFD;
    ((Flags108*)(&rec.value.courseFlags))->hi = 1;
    strcpy(rec.value.common.name, iffSchemaAndCourseStrings + 0x330);
    strcpy(rec.value.mapName, iffSchemaAndCourseStrings + 0x339);
    rec.value.pangRate = kFloat_1_1_PangRate;
    strcpy(rec.value.propertyFile, iffSchemaAndCourseStrings + 0x33e);
    assign108(self, index, rec);
    memset(&rec, 0, kCourseRecordBytes);
    rec.value.common.typeId = kSyntheticCourseFE;
    ((Flags108*)(&rec.value.courseFlags))->hi = 1;
    strcpy(rec.value.common.name, iffSchemaAndCourseStrings + 0x350);
    strcpy(rec.value.mapName, iffSchemaAndCourseStrings + 0x355);
    rec.value.pangRate = kFloat_1_1_PangRate;
    strcpy(rec.value.propertyFile, iffSchemaAndCourseStrings + 0x35a);
    assign108(self, index + 1, rec);
    memset(&rec, 0, kCourseRecordBytes);
    rec.value.common.typeId = kSyntheticCourse7F;
    ((Flags108*)(&rec.value.courseFlags))->hi = 1;
    strcpy(rec.value.common.name, iffSchemaAndCourseStrings + 0x36c);
    strcpy(rec.value.mapName, iffSchemaAndCourseStrings + 0x373);
    rec.value.pangRate = kFloat_1_1_PangRate;
    strcpy(rec.value.propertyFile, iffSchemaAndCourseStrings + 0x35a);
    strcpy(rec.value.common.icon, iffSchemaAndCourseStrings + 0x37a);
    assign108(self, index + 2, rec);
}
#pragma pop

typedef IffRecordStorage<IFF_STRUCT::sHairStyle> HairStyleRecordStorage;
struct HairStyleLoadEntry {
    unsigned int key;
    IffRecordStorage<IFF_STRUCT::sHairStyle> record;
    // Leave staging storage uninitialized; the loader overwrites its payload.
    HairStyleLoadEntry() {}
    // Copy a hairstyle payload and initialize its cached key to zero.
    HairStyleLoadEntry(const HairStyleRecordStorage& v) : key(0) {
        *(HairStyleRecordStorage*)&record = v;
    }
    // Copy a complete hairstyle entry while replacing the owning allocation.
    HairStyleLoadEntry(const HairStyleLoadEntry& v) : key(v.key) {
        *(HairStyleRecordStorage*)&record = *(const HairStyleRecordStorage*)&v.record;
    }
};
struct Grow100 {
    HairStyleLoadEntry* data;
    unsigned int size, capacity;
    unsigned int* capacityRef;
    unsigned int oldsize;
    // Start an empty temporary owner for hairstyle storage growth.
    Grow100(unsigned int* c) : data(0), size(0), capacity(0), capacityRef(c), oldsize(0) {}
    // Free whichever hairstyle allocation remains owned after the swap.
    ~Grow100() {
        if (data) {
            size = size - size;
            appFreeUni__FPv(data);
        }
    }
};
struct HairStyleTableLoader {
    unsigned char pad[0xA4];
    HairStyleLoadEntry* data;
    unsigned int size;
    unsigned int capacity;
};
// Placement-construct one hairstyle payload with an initially zero key.
inline void construct100(HairStyleLoadEntry* dst, const HairStyleRecordStorage& value) {
    try {
        new (dst) HairStyleLoadEntry(value);
    } catch (...) {
        throw;
    }
}
// Preserve an old hairstyle entry when moving to a larger allocation.
inline void copyConstruct100(HairStyleLoadEntry* dst, const HairStyleLoadEntry& value) {
    try {
        new (dst) HairStyleLoadEntry(value);
    } catch (...) {
        throw;
    }
}

struct Err100 {
    void* vt;
    char* buf;
    TargetRef* ref;
    // Allocate shared exception text and throw a hairstyle length error.
    Err100(const char* msg) {
        char done __attribute__((aligned(4))) = 0;
        vt = __vt__Q23std11logic_error;
        unsigned n = strlen(msg) + 1;
        char* buffer = (char*)(unsigned int)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            TargetHolder h(buffer, &done);
            TargetRef* r = (TargetRef*)__nw__FUl(16);
            if (r) {
                r->c1 = 1;
                r->c2 = 1;
                r->vt = s_sharedCharDeleterVtable;
                r->s = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
        vt = __vt__Q23std12length_error;
        __throw((void*)(iffSchemaAndCourseStrings + 0x2f9), this, __dt__Q23std12length_errorFv);
    }
};
// Throw the hairstyle specialization's length error.
inline void lengthError100(const char* m) {
    Err100 e(m);
}
#pragma push
#pragma use_lmw_stmw on
// Replace hairstyle entries with the IFF records, caching each record's type ID.
void ItemInventory_ReadHairStyleTable(HairStyleTableLoader* self, const void* source) {
    HairStyleLoadEntry tailSrc;
    HairStyleLoadEntry tailDst;
    HairStyleLoadEntry fillSrc;
    HairStyleLoadEntry growSrc;
    HairStyleLoadEntry growDst;
    HairStyleLoadEntry fillDst;
    unsigned char hdr[8];
    memcpy(hdr, source, kIffHeaderBytes);
    const unsigned char* input = (const unsigned char*)source + kIffHeaderBytes;
    // Phase 1: resize logical slots, staging ownership if reallocation is needed.
    if (*(unsigned short*)hdr > self->size) {
        unsigned int need = *(unsigned short*)hdr - self->size;
        // Check capacity for an in-place fill; otherwise continue to the growth path.
        do {
            if (need > self->capacity || self->size > self->capacity - need) {
                do {
                    unsigned int cap0 = self->capacity;
                    if (self->size + need - cap0 > 0x28F5C28 - cap0) {
                        const char* m = iffSchemaAndCourseStrings + 0x2e5;
                        TargetErr error(m);
                    }
                    if (cap0 < 0xDA740D)
                        break;
                    if (cap0 < 0x1B4E81A)
                        break;
                    break;
                } while (0);
            } else {
                {
                    HairStyleLoadEntry* dst = self->data + self->size;
                    *(HairStyleRecordStorage*)((char*)&fillDst + 4) = *(HairStyleRecordStorage*)((char*)&fillSrc + 4);
                    while (need) {
                        construct100(dst, *(const HairStyleRecordStorage*)((char*)&fillDst + 4));
                        --need;
                        ++dst;
                        ++self->size;
                    }
                }
                break;
            }

            {
                Grow100 grow(&self->capacity);
                unsigned int needed = self->size + need - self->capacity;
                unsigned int oldcap = self->capacity;
                if (needed > 0x28F5C28 - oldcap) {
                    const char* m = iffSchemaAndCourseStrings + 0x2e5;
                    TargetErr error(m);
                }
                unsigned int cap;
                if (oldcap < 0xDA740D)
                    cap = oldcap + max108(((oldcap + 1) * 3) / 5, needed);
                else if (oldcap < 0x1B4E81A)
                    cap = oldcap + max108((oldcap + 1) / 2, needed);
                else
                    cap = 0x28F5C28;
                if (cap > 0x28F5C28) {
                    const char* m = iffSchemaAndCourseStrings + 0x2e5;
                    TargetErr error(m);
                }
                HairStyleLoadEntry* p = (HairStyleLoadEntry*)appMallocDummy__FUl(cap * kHairEntryBytes);
                unsigned int sz = self->size;
                grow.data = p;
                grow.capacity = cap;
                grow.oldsize = sz;
                HairStyleLoadEntry* dst = p + sz + grow.size;
                *(HairStyleRecordStorage*)((char*)&growDst + 4) = *(HairStyleRecordStorage*)((char*)&growSrc + 4);
                while (need) {
                    construct100(dst, *(const HairStyleRecordStorage*)((char*)&growDst + 4));
                    --need;
                    ++dst;
                    ++grow.size;
                }
                HairStyleLoadEntry* begin = self->data;
                HairStyleLoadEntry* o = begin + self->size;
                HairStyleLoadEntry* d = grow.data + grow.oldsize;
                while (o > begin) {
                    --o;
                    --d;
                    copyConstruct100(d, *o);
                    --grow.oldsize;
                    ++grow.size;
                }
                HairStyleLoadEntry* old = self->data;
                self->size = 0;
                unsigned int oldcap2 = self->capacity;
                grow.data = old;
                self->capacity = grow.capacity;
                grow.capacity = oldcap2;
                self->data = p;
                self->size = grow.size;
                grow.size = 0;
            }
        } while (0);
        ;
    } else if (*(unsigned short*)hdr < self->size) {
        self->size -= self->size - *(unsigned short*)hdr;
    }
    // Phase 2: replace every live record; fill temporaries no longer matter.
    int index = 0;
    for (int i = 0; i < *(unsigned short*)hdr; ++i) {
        memcpy(&tailSrc.record, input, kHairRecordBytes);
        input += kHairRecordBytes;
        *(HairStyleRecordStorage*)&tailDst.record = *(HairStyleRecordStorage*)&tailSrc.record;
        HairStyleLoadEntry* d = self->data + index;
        d->key = tailSrc.record.value.common.typeId;
        *(HairStyleRecordStorage*)&d->record = *(HairStyleRecordStorage*)&tailDst.record;
        ++index;
    }
}
#pragma pop

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
void* __nw__FUl(unsigned long);
void __dla__FPv(void*, int);
void __throw(void*, void*, void (*)()) __attribute__((noreturn));
void __dt__Q23std12length_errorFv();
extern "C" void __dt__Q23std9exceptionFv(void*, int);

struct Holder {
    char* p;
    char* flag;
    // Hold auxiliary-table exception text until shared-owner construction succeeds.
    Holder(char* b, char* f) {
        p = b;
        flag = f;
    }
    // Free exception text unless ownership was transferred.
    ~Holder() {
        if (p)
            __dla__FPv(p, -1);
    }
};
struct Ref {
    void* vt;
    int c1;
    int c2;
    char* s;
};
struct ErrBase : std::exception {
    // Install the target exception vtable without changing its storage layout.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }
    char* buf;
    Ref* ref;
    // Construct the auxiliary-table error's shared message representation.
    ErrBase(const char* msg0) {
        char done __attribute__((aligned(4))) = 0;
        SetTable(__vt__Q23std11logic_error);
        const char* msg = msg0;
        unsigned n = strlen(msg) + 1;
        char* buffer = (char*)(unsigned int)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            Holder h(buffer, &done);
            Ref* r = (Ref*)__nw__FUl(16);
            if (r) {
                r->c1 = 1;
                r->c2 = 1;
                r->vt = s_sharedCharDeleterVtable;
                r->s = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
};
struct Err2 : ErrBase {
    // Finish the auxiliary length-error type and throw it.
    Err2(const char* msg) : ErrBase(msg) {
        SetTable(__vt__Q23std12length_error);
        __throw((void*)(iffSchemaAndCourseStrings + 0x2f9), this, __dt__Q23std12length_errorFv);
    }
    // Retain the target's empty derived destructor.
    ~Err2() {}
};
// Construct and throw the auxiliary vector's capacity error.
inline void capacityError(const char* message) {
    Err2 e(message);
}

typedef IffRecordStorage<IFF_STRUCT::sAuxPart> AuxPartRecordStorage;
struct AuxPartLoadEntry {
    unsigned int key;
    IffRecordStorage<IFF_STRUCT::sAuxPart> record;
    // Leave staging storage unspecified until the loader copies a record.
    AuxPartLoadEntry() {}
    // Copy an auxiliary payload and initialize its cached key to zero.
    AuxPartLoadEntry(const AuxPartRecordStorage& v) : key(0) {
        *(AuxPartRecordStorage*)&record = v;
    }
    // Copy key and payload when moving auxiliary entries to a larger allocation.
    AuxPartLoadEntry(const AuxPartLoadEntry& v) : key(v.key) {
        *(AuxPartRecordStorage*)&record = *(const AuxPartRecordStorage*)&v.record;
    }
};
struct Grow74 {
    AuxPartLoadEntry* data;
    unsigned int size, capacity;
    unsigned int* capacityRef;
    unsigned int oldsize;
    // Start an empty temporary owner for auxiliary-part storage growth.
    Grow74(unsigned int* c) : data(0), size(0), capacity(0), capacityRef(c), oldsize(0) {}
    // Free the allocation still owned when this growth scope ends.
    ~Grow74() {
        if (data) {
            size = size - size;
            appFreeUni__FPv(data);
        }
    }
};
struct AuxPartTableLoader {
    unsigned char pad[0xB0];
    AuxPartLoadEntry* data;
    unsigned int size;
    unsigned int capacity;
};
// Placement-construct one auxiliary-part payload with an initially zero key.
inline void construct74(AuxPartLoadEntry* dst, const AuxPartRecordStorage& value) {
    try {
        new (dst) AuxPartLoadEntry(value);
    } catch (...) {
        throw;
    }
}
// Copy an old auxiliary-part entry into replacement storage.
inline void copyConstruct74(AuxPartLoadEntry* dst, const AuxPartLoadEntry& value) {
    try {
        new (dst) AuxPartLoadEntry(value);
    } catch (...) {
        throw;
    }
}

struct Err74 {
    void* vt;
    char* buf;
    TargetRef* ref;
    // Build shared message storage and throw this specialization's length error.
    Err74(const char* msg) {
        char done __attribute__((aligned(4))) = 0;
        vt = __vt__Q23std11logic_error;
        unsigned n = strlen(msg) + 1;
        char* buffer = (char*)(unsigned int)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            TargetHolder h(buffer, &done);
            TargetRef* r = (TargetRef*)__nw__FUl(16);
            if (r) {
                r->c1 = 1;
                r->c2 = 1;
                r->vt = s_sharedCharDeleterVtable;
                r->s = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
        vt = __vt__Q23std12length_error;
        __throw((void*)(iffSchemaAndCourseStrings + 0x2f9), this, __dt__Q23std12length_errorFv);
    }
};
// Construct TargetErr, whose constructor throws the length error.
inline void lengthError74(const char* m) {
    TargetErr e(m);
}
#pragma push
#pragma use_lmw_stmw on
// Replace auxiliary-part entries with IFF records and synchronized type-ID keys.
void ItemInventory_ReadAuxPartTable(AuxPartTableLoader* self, const void* source) {
    AuxPartLoadEntry tailSrc;
    AuxPartLoadEntry tailDst;
    AuxPartLoadEntry fillSrc;
    AuxPartLoadEntry growSrc;
    AuxPartLoadEntry growDst;
    AuxPartLoadEntry fillDst;
    unsigned char hdr[8];
    memcpy(hdr, source, kIffHeaderBytes);
    const unsigned char* input = (const unsigned char*)source + kIffHeaderBytes;
    // Phase 1: resize logical slots; a growth guard owns any replacement buffer.
    if (*(unsigned short*)hdr > self->size) {
        unsigned int need = *(unsigned short*)hdr - self->size;
        // Check capacity for an in-place fill; otherwise continue to the growth path.
        do {
            if (need > self->capacity || self->size > self->capacity - need) {
                do {
                    unsigned int cap0 = self->capacity;
                    if (self->size + need - cap0 > 0x234F72C - cap0) {
                        const char* m = iffSchemaAndCourseStrings + 0x2e5;
                        TargetErr error(m);
                    }
                    if (cap0 < 0xBC5264)
                        break;
                    if (cap0 < 0x178A4C8)
                        break;
                    break;
                } while (0);
            } else {
                {
                    AuxPartLoadEntry* dst = self->data + self->size;
                    *(AuxPartRecordStorage*)((char*)&fillDst + 4) = *(AuxPartRecordStorage*)((char*)&fillSrc + 4);
                    while (need) {
                        construct74(dst, *(const AuxPartRecordStorage*)((char*)&fillDst + 4));
                        --need;
                        ++dst;
                        ++self->size;
                    }
                }
                break;
            }

            {
                Grow74 grow(&self->capacity);
                unsigned int needed = self->size + need - self->capacity;
                unsigned int oldcap = self->capacity;
                if (needed > 0x234F72C - oldcap) {
                    const char* m = iffSchemaAndCourseStrings + 0x2e5;
                    TargetErr error(m);
                }
                unsigned int cap;
                if (oldcap < 0xBC5264)
                    cap = oldcap + max108(((oldcap + 1) * 3) / 5, needed);
                else if (oldcap < 0x178A4C8)
                    cap = oldcap + max108((oldcap + 1) / 2, needed);
                else
                    cap = 0x234F72C;
                if (cap > 0x234F72C) {
                    const char* m = iffSchemaAndCourseStrings + 0x2e5;
                    TargetErr error(m);
                }
                AuxPartLoadEntry* p = (AuxPartLoadEntry*)appMallocDummy__FUl(cap * kAuxEntryBytes);
                unsigned int sz = self->size;
                grow.data = p;
                grow.capacity = cap;
                grow.oldsize = sz;
                AuxPartLoadEntry* dst = p + sz + grow.size;
                *(AuxPartRecordStorage*)((char*)&growDst + 4) = *(AuxPartRecordStorage*)((char*)&growSrc + 4);
                while (need) {
                    construct74(dst, *(AuxPartRecordStorage*)((char*)&growDst + 4));
                    --need;
                    ++dst;
                    ++grow.size;
                }
                AuxPartLoadEntry* begin = self->data;
                AuxPartLoadEntry* o = begin + self->size;
                AuxPartLoadEntry* d = grow.data + grow.oldsize;
                while (o > begin) {
                    --o;
                    --d;
                    copyConstruct74(d, *o);
                    --grow.oldsize;
                    ++grow.size;
                }
                AuxPartLoadEntry* old = self->data;
                self->size = 0;
                unsigned int oldcap2 = self->capacity;
                grow.data = old;
                self->capacity = grow.capacity;
                grow.capacity = oldcap2;
                self->data = p;
                self->size = grow.size;
                grow.size = 0;
            }
        } while (0);
        ;
    } else if (*(unsigned short*)hdr < self->size) {
        self->size -= self->size - *(unsigned short*)hdr;
    }
    // Phase 2: overwrite each slot with a serialized auxiliary-part record.
    int index = 0;
    for (int i = 0; i < *(unsigned short*)hdr; ++i) {
        memcpy(&tailSrc.record, input, kAuxRecordBytes);
        input += kAuxRecordBytes;
        *(AuxPartRecordStorage*)&tailDst.record = *(AuxPartRecordStorage*)&tailSrc.record;
        AuxPartLoadEntry* d = self->data + index;
        d->key = tailSrc.record.value.common.typeId;
        *(AuxPartRecordStorage*)&d->record = *(AuxPartRecordStorage*)&tailDst.record;
        ++index;
    }
}
#pragma pop

} // extern "C"
