// Party-slot pointer-vector growth, replacement ownership and per-player accessory storage.
// Private implementation fragment, included once at its original declaration position.
// Keep declaration order and emitted destructor names unchanged.

typedef unsigned long A522CSize;
struct A522CTemp {
    int* data;
    volatile A522CSize size;
    A522CSize capacity;
    A522CSize* owner;
    volatile A522CSize offset;
    // Clear pointer count and free the old or partially built vector buffer.
    ~A522CTemp() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
struct A522CVec {
    int* data;
    A522CSize size;
    A522CSize capacity;
};
// Return the larger item-slot vector growth candidate by reference.
inline const A522CSize& A522CMax(const A522CSize& a, const A522CSize& b) { return a < b ? b : a; }

struct A522CLengthError : ErrorBase_A59E0 {
    // Construct and throw the item-vector length-error diagnostic.
    A522CLengthError(const char* p) : ErrorBase_A59E0(p) {
        table = (void*)__vt__Q23std12length_error;
        __throw(partyBalloonItemStrings + kPartyLengthErrorType, this, (void*)__dt__Q23std12length_errorFv);
    }
    // Release length-error text through its existing runtime helper.
    ~A522CLengthError() { __dt__Q33std6detail16char_array_guardFv(this, 0); }
};
// Recommend pointer-vector capacity with the original piecewise growth policy.
inline A522CSize A522CGrow(A522CSize capacity, const A522CSize& needed) {
    if (capacity < 0x15555555u)
        return capacity + A522CMax((3 * (capacity + 1)) / 5, needed);
    if (capacity < 0x2aaaaaaau)
        return capacity + A522CMax((capacity + 1) / 2, needed);
    return 0x3fffffffu;
}
// Validate requested item-slot capacity before applying the growth policy.
inline A522CSize A522CRecommend(A522CVec* v, A522CSize n) {
    A522CSize size = *(volatile A522CSize*)&v->size;
    A522CSize capacity = *(volatile A522CSize*)&v->capacity;
    if (size + n > 0x3fffffffu - capacity) {
        const char* text = partyBalloonItemStrings + kPartyVectorLengthError;
        A522CLengthError error(text);
    }
    return A522CGrow(capacity, n);
}

struct A522CItem {
    char data[0x4c];
    // Initialize one owned accessory-state object for player i.
    A522CItem(int i) { CAccState_Ctor(this, i); }
};
