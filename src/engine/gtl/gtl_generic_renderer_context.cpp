// Generic renderer context: its destructor, which releases the four resource handles the context holds
// and frees the context when asked.

extern "C" {

void gtl_GenericResource_Free(void* res);
void __dl__FPv(void* p);

} // extern "C"

// A resource handle and its use count. The game's release goes through several inlined wrapper layers, each
// testing the handle's address for null (5 tests here, 4 for the context's first handle): the repeated tests
// below reproduce them. Clearing the count is a vector erase(begin(), end()): count -= count.
struct GenericResourceRef {
    void* res;
    int count;
    void Release() {
        if (this == 0) {
            return;
        }
        ReleaseInner();
    }
    void ReleaseInner() {
        if (this == 0) {
            return;
        }
        if (this == 0) {
            return;
        }
        if (this == 0) {
            return;
        }
        if (this == 0) {
            return;
        }
        if (res == 0) {
            return;
        }
        int n = count;
        count -= n;
        gtl_GenericResource_Free(res);
    }
};

struct GenericRendererContext {
    char unk0[0x184];
    GenericResourceRef ref184;
    char reserved18c[8];
    GenericResourceRef ref194;
    char reserved19c[4];
    GenericResourceRef ref1a0;
    char reserved1a8[4];
    GenericResourceRef ref1ac;
};

extern "C" {

// without propagation the count is reloaded and subtracted from itself (subf r0, r0, r0), as in the game
#pragma opt_propagation off
void* gtl_GenericRenderer_ContextDtor(GenericRendererContext* self, int flags) {
    if (self != 0) {
        self->ref1ac.Release();
        self->ref1a0.Release();
        self->ref194.Release();
        self->ref184.ReleaseInner();
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

} // extern "C"
