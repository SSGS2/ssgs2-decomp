#include <decomp/ct_member.h>
// Local standard-exception representations and diagnostic-buffer ownership.
// Private implementation fragment, included once at its original declaration position.
// Keep declaration order and emitted destructor names unchanged.

struct BitAllocation {
    char* data;
    char* allocator;
    // Release the guarded character array unless ownership transferred.
    ~BitAllocation() { if (data) delete[] data; }
};
struct BitControl {
    void* vtable;
    int refs, owners;
    char* data;
    // Initialize shared ownership of the diagnostic character array.
    BitControl(char* p) { refs = 1; owners = 1; vtable = s_sharedCharDeleterVtable; data = p; }
};
namespace std {
struct exception {
    void* vtable;
};
struct logic_error: exception {
    char* data;
    BitControl* control;
    // Allocate shared diagnostic text for the standard logic-error base.
    logic_error(const char* message) {
        vtable = __vt__Q23std11logic_error;
        char allocator[4];
        allocator[0] = 0;
        unsigned int n = strlen(message) + 1;
        data = (char*)__nwa__FUl((n << 1) - n);
        {
            char* buffer = data;
            BitAllocation allocation = { buffer, allocator };
            control = new BitControl(buffer);
            allocation.data = 0;
        }
        strcpy(data, message);
    }
    ~logic_error();
};
struct out_of_range: logic_error {
    // Set the range-error identity on the copied diagnostic.
    out_of_range(const char* message): logic_error(message) { vtable = __vt__Q23std12out_of_range; }
    ~out_of_range();
};
struct LenBase: exception {
    // Destroy the standard exception base.
    ~LenBase() { __dt__Q23std9exceptionFv(this); }
};
struct length_error: LenBase {
    char* data;
    BitControl* control;
    // Copy length-error text with the supplied allocator tag.
    length_error(const char* message, char* allocator) {
        vtable = __vt__Q23std11logic_error;
        allocator[0] = 0;
        unsigned int n = strlen(message) + 1;
        data = (char*)__nwa__FUl((n << 1) - n);
        {
            char* buffer = data;
            BitAllocation allocation = { buffer, allocator };
            control = new BitControl(buffer);
            allocation.data = 0;
        }
        strcpy(data, message);
        vtable = __vt__Q23std12length_error;
    }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}

struct ErrorAlloc_A59E0 { char value; char pad[3];
    // Initialize the allocator tag for copied range-error text.
    ErrorAlloc_A59E0() : value(0) {} };
struct BufferGuard_A59E0 {
    char* data; void* allocator;
    // Guard a diagnostic buffer until shared ownership is installed.
    BufferGuard_A59E0(char* p, void* a) : data(p), allocator(a) {}
    // Transfer the guarded buffer and disarm automatic release.
    char* release() { char* p = data; data = 0; return p; }
    // Delete the buffer only if the guard retains ownership.
    ~BufferGuard_A59E0() { if (data) __dla__FPv(data); }
};
struct ExceptionCount_A59E0 {
    void* table; int a, b; char* data;
    // Initialize the exception text control block and its reference counts.
    ExceptionCount_A59E0(char* p) : a(1), b(1) { table = (void*)s_sharedCharDeleterVtable; data = p; }
};
struct ErrorBase_A59E0 {
    void* table; char* data; ExceptionCount_A59E0* owner;
    // Copy and share error text with cleanup on allocation failure.
    ErrorBase_A59E0(const char* text) {
        table = (void*)__vt__Q23std11logic_error;
        ErrorAlloc_A59E0 allocator;
        unsigned len = strlen(text) + 1;
        data = (char*)__nwa__FUl((len << 1) - len);
        {
            char* p = data;
            BufferGuard_A59E0 guard(p, &allocator);
            owner = new ExceptionCount_A59E0(p);
            guard.release();
        }
        strcpy(data, text);
    }
    // Destroy the standard exception base without freeing this allocation.
    ~ErrorBase_A59E0() { __dt__Q23std9exceptionFv(this, 0); }
};
