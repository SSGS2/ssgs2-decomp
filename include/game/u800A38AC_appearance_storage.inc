// Appearance-effect pointer-vector allocation, growth guards and exception text.
// Private implementation fragment, included once at its original declaration position.
// Keep declaration order and emitted destructor names unchanged.

// ---- Exception text and allocator support (no party-item selection behavior). ----
// Keep specializations in place: their destructor emission order is compiler-sensitive.
struct A4B20Allocation {
    char* pointer; char* allocator;
    // Guard allocated exception text until ownership transfers.
    A4B20Allocation(char* p, char* a) : pointer(p), allocator(a) {}
    // Delete exception text if the guard still owns it.
    ~A4B20Allocation() { if (pointer) __dla__FPv(pointer); }
};
struct A4B20ControlBase {
    void* table; unsigned int strong, weak;
    // Initialize strong and weak reference counts for exception text.
    A4B20ControlBase() { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; }
};
struct A4B20Control : A4B20ControlBase {
    char* pointer;
    // Attach the allocated text to its reference-counted control block.
    A4B20Control(char* p) { pointer = p; }
};
struct A4B20ErrorBase {
    void* table;
    // Destroy the standard exception base with its existing deletion mode.
    ~A4B20ErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
struct A4B20LengthError : A4B20ErrorBase {
    char* text; void* control;
    // Build and throw the vector length-error diagnostic.
    A4B20LengthError() {
        const char* message = partyBalloonItemStrings + kPartyVectorLengthError;
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        table = __vt__Q23std11logic_error;
        unsigned int n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            A4B20Allocation allocation(p, &allocator.value);
            control = new A4B20Control(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
        __throw(partyBalloonItemStrings + kPartyLengthErrorType, this, (void*)__dt__Q23std12length_errorFv);
    }
    // Release the length-error object through its runtime destructor.
    ~A4B20LengthError() { dtor_80046988(this, -1); }
};

struct A4B20Buffer {
    int* data; unsigned int size; unsigned int capacity;
    unsigned int* owner; volatile unsigned int offset;
    // Initialize empty pointer storage for vector growth and unwind cleanup.
    A4B20Buffer(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Clear the pointer count and free only the guarded buffer.
    ~A4B20Buffer() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
// Return the larger pointer-vector growth candidate by reference.
inline const unsigned int& A4B20Max(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }
// Construct and throw the appearance-effect vector length error.
inline void A4B20Throw() { A4B20LengthError error; }
