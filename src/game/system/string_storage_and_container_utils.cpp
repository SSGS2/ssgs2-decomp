// Translation unit 0x80065B4C-0x80065D10 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/MWStringStorage.h"

extern "C" {

char* strchr(const char*, int); // strchr
char* strrchr(const char*, int); // strrchr
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

typedef MWStringStorage StringLike;
// Each 16-byte entry contains a string representation followed by an opaque value.
// flags bit 31 selects an owned heap string at +8; otherwise bytes at +1 are inline.
// Container owns the entry array and only the heap strings, never Entry::value.
struct Entry { unsigned int flags; unsigned int length; void* data; unsigned int value; };
struct Container { Entry* data; int count; };
struct RenderableResourceLink { char baseState[0x10]; void* resourceManager; };
enum { kHeapStringFlagShift = 31 };

// Select inline or heap string storage and return the requested byte position.
// Borrowed result; offset is unchecked, in bytes (including a possible terminator).
// It is invalidated when the string is modified or destroyed.
char* String_GetCharPtrAt(StringLike* value, int offset) {
    char* base;
    if ((value->flags >> kHeapStringFlagShift) == 0)
        base = (char*)value + 1;
    else
        base = value->data;
    return base + offset;
}

// Destroy string/value entries in reverse order, then release their vector storage.
// self may be null; deleting > 0 also frees the wrapper. The returned address may be dangling.
Container* dtor_80065BA0(Container* self, int deleting) {
    if (self != 0) {
        Entry* data = self->data;
        if (data != 0) {
            Entry* entry;
            int count = self->count;
            entry = data + count;
            // Publish an empty container before walking its former live entries.
            self->count = self->count - count;
            while (count != 0) {
                entry = entry - 1;
                if (entry != 0 && entry != 0) {
                    if (entry->flags >> kHeapStringFlagShift)
                        appFreeUni__FPv(entry->data);
                }
                count = count - 1;
            }
            appFreeUni__FPv(self->data);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Destroy the second string/value vector specialization with the same ownership rules.
// count must describe a valid nonnegative range; no element bounds are checked.
Container* dtor_80065C58(Container* self, int deleting) {
    if (self != 0) {
        Entry* data = self->data;
        if (data != 0) {
            Entry* entry;
            int count = self->count;
            entry = data + count;
            self->count = self->count - count;
            while (count != 0) {
                entry = entry - 1;
                if (entry != 0 && entry != 0) {
                    if (entry->flags >> kHeapStringFlagShift)
                        appFreeUni__FPv(entry->data);
                }
                count = count - 1;
            }
            appFreeUni__FPv(self->data);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Return the pointer stored at the start of the wrapper.
void* Pointer_GetFirstPointer(void** value) {
    return *value;
}

// The base font has no resource to return.
void* WFont_MakeClone() {
    return 0;
}

// Clear three raw 32-bit words; this helper alone does not establish a vector or float type.
void Storage_ClearThreeWords(unsigned int* value) {
    value[0] = 0;
    value[1] = 0;
    value[2] = 0;
}

// 0x80065B54: forward to strchr; the caller tests the returned pointer at 0x80060F3C.
char* CString_FindCharForwarder(const char* text, int c) {
    return strchr(text, c);
}

// 0x80065B58: forward to strrchr; the caller keeps the pointer at 0x80060F70-0x80060F74.
char* CString_FindLastCharForwarder(const char* text, int c) {
    return strrchr(text, c);
}

// Attach the resource manager used by this renderable.
// Store a borrowed pointer without retaining or releasing either manager.
void Renderable_SetResourceManager(void* self, void* value) {
    ((RenderableResourceLink*)self)->resourceManager = value;
}

} // extern "C"
