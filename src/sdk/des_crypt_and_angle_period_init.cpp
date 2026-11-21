#include <decomp/static_init.h>
// Translation unit 0x8028CB84-0x8028DFB0 (4 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern const float kFloat_6_2831855;
extern float s_auxiliaryFirstAnglePeriod;
extern float s_auxiliarySecondAnglePeriod;
extern unsigned int desSpTrans[];

// Initialize the first angle-period cache to two pi; unrelated to the DES helpers in this unit.
void AuxiliaryAngles_InitFirstPeriod() {
    s_auxiliaryFirstAnglePeriod = kFloat_6_2831855;
}

// Initialize the second angle-period cache to two pi.
void AuxiliaryAngles_InitSecondPeriod() {
    s_auxiliarySecondAnglePeriod = kFloat_6_2831855;
}

// DES words are 32-bit unsigned long on the Wii ABI.
typedef unsigned long DES_LONG;
// Exchange masked bit groups between two halves; mask/shift define the DES permutation.
// Permutations keep one scratch lifetime across exchanges. Replacing these
// macros with reference-parameter inline functions changes the matched object.
#define PERM_OP(a,b,t,n,m) ((t)=((((a)>>(n))^(b))&(m)),(b)^=(t),(a)^=((t)<<(n)))

// Rotate one 32-bit DES word right; callers use nonzero shifts strictly below 32.
// The inline-function form also changes register allocation in this unrolled round body.
#define RotateRight(word,shift) (((word)>>(shift))+((word)<<(32-(shift))))

// Apply DES's initial bit permutation in five masked exchanges.
#define IP(l,r) { \
    register DES_LONG tt; \
    PERM_OP(r,l,tt, 4,0x0f0f0f0f); \
    PERM_OP(l,r,tt,16,0x0000ffff); \
    PERM_OP(r,l,tt, 2,0x33333333); \
    PERM_OP(l,r,tt, 8,0x00ff00ff); \
    PERM_OP(r,l,tt, 1,0x55555555); \
}

// Undo the initial bit permutation using the exchanges in reverse order.
#define FP(l,r) { \
    register DES_LONG tt; \
    PERM_OP(l,r,tt, 1,0x55555555); \
    PERM_OP(r,l,tt, 8,0x00ff00ff); \
    PERM_OP(l,r,tt, 2,0x33333333); \
    PERM_OP(r,l,tt,16,0x0000ffff); \
    PERM_OP(l,r,tt, 4,0x0f0f0f0f); \
}

// Indices are signed byte offsets: keeping this representation preserves the DES table loads.
static inline unsigned int Substitution(const unsigned int* row, int byteOffset) {
    return *(const unsigned int*)((const char*)row + byteOffset);
}
enum DesTableLayout { kTableRowWords = 64, kSubstitutionByteMask = 0xFC };
// Keep the round expansion local: an inline-function form changes register allocation.
// One Feistel round: XOR the right half with TWO pre-expanded key words, then
// XOR eight combined S-box/permutation lookups into the left half. sp0/2/4/6
// use u's bytes, sp1/3/5/7 use t rotated right four bits. The 0xFC mask selects
// a six-bit index scaled by four bytes; it is not a 256-entry table index.
#define D_ENCRYPT(LL,R,S) { \
    int u1,u2,u3; \
    u=(R)^schedule[S]; t=(R)^schedule[(S)+1]; \
    u2=(int)u>>8; \
    u1=(int)u&kSubstitutionByteMask; \
    u2&=kSubstitutionByteMask; \
    t=RotateRight(t,4); \
    u>>=16; \
    LL^=Substitution(sp0,u1); \
    LL^=Substitution(sp2,u2); \
    u3=(int)(u>>8); \
    u1=(int)u&kSubstitutionByteMask; \
    u3&=kSubstitutionByteMask; \
    LL^=Substitution(sp4,u1); \
    LL^=Substitution(sp6,u3); \
    u2=(int)t>>8; \
    u1=(int)t&kSubstitutionByteMask; \
    u2&=kSubstitutionByteMask; \
    t>>=16; \
    LL^=Substitution(sp1,u1); \
    LL^=Substitution(sp3,u2); \
    u3=(int)t>>8; \
    u1=(int)t&kSubstitutionByteMask; \
    u3&=kSubstitutionByteMask; \
    LL^=Substitution(sp5,u1); \
    LL^=Substitution(sp7,u3); \
}

#pragma use_lmw_stmw on
// Apply 16 DES rounds to one block; nonzero direction uses forward keys, zero uses reverse keys.
// data is a writable pair of 32-bit words. keyAddress is a borrowed Wii pointer
// to 32 pre-expanded key words (two per round), NOT a raw eight-byte key.
// This is one in-place block: no mode, padding, IV, or key expansion is performed.
void DES_CryptBlock(unsigned int* data, int keyAddress, int direction) {
    DES_LONG left, right, t, u;
    DES_LONG *schedule;
    const unsigned int *sp0;
    const unsigned int *sp2, *sp4, *sp6, *sp1, *sp3, *sp5, *sp7;

    sp0 = desSpTrans;

    schedule = (DES_LONG *)keyAddress;

    right = data[0];
    left = data[1];

    // Permute the input halves, then rotate into the combined lookup-table representation.
    IP(right,left);

    right = RotateRight(right,29);
    left = RotateRight(left,29);

    if (direction) {
        sp2 = desSpTrans + 2 * kTableRowWords;
        sp4 = desSpTrans + 4 * kTableRowWords;
        sp6 = desSpTrans + 6 * kTableRowWords;
        sp1 = desSpTrans + 1 * kTableRowWords;
        sp3 = desSpTrans + 3 * kTableRowWords;
        sp5 = desSpTrans + 5 * kTableRowWords;
        sp7 = desSpTrans + 7 * kTableRowWords;
        // Forward schedule: alternating updates incorporate sixteen round-key pairs (words 0..31).
        D_ENCRYPT(left,right,0);
        D_ENCRYPT(right,left,2);
        D_ENCRYPT(left,right,4);
        D_ENCRYPT(right,left,6);
        D_ENCRYPT(left,right,8);
        D_ENCRYPT(right,left,10);
        D_ENCRYPT(left,right,12);
        D_ENCRYPT(right,left,14);
        D_ENCRYPT(left,right,16);
        D_ENCRYPT(right,left,18);
        D_ENCRYPT(left,right,20);
        D_ENCRYPT(right,left,22);
        D_ENCRYPT(left,right,24);
        D_ENCRYPT(right,left,26);
        D_ENCRYPT(left,right,28);
        D_ENCRYPT(right,left,30);
    } else {
        sp2 = desSpTrans + 2 * kTableRowWords;
        sp4 = desSpTrans + 4 * kTableRowWords;
        sp6 = desSpTrans + 6 * kTableRowWords;
        sp1 = desSpTrans + 1 * kTableRowWords;
        sp3 = desSpTrans + 3 * kTableRowWords;
        sp5 = desSpTrans + 5 * kTableRowWords;
        sp7 = desSpTrans + 7 * kTableRowWords;
        // Reverse schedule decrypts with the same lookup operation and reversed key pairs.
        D_ENCRYPT(left,right,30);
        D_ENCRYPT(right,left,28);
        D_ENCRYPT(left,right,26);
        D_ENCRYPT(right,left,24);
        D_ENCRYPT(left,right,22);
        D_ENCRYPT(right,left,20);
        D_ENCRYPT(left,right,18);
        D_ENCRYPT(right,left,16);
        D_ENCRYPT(left,right,14);
        D_ENCRYPT(right,left,12);
        D_ENCRYPT(left,right,10);
        D_ENCRYPT(right,left,8);
        D_ENCRYPT(left,right,6);
        D_ENCRYPT(right,left,4);
        D_ENCRYPT(left,right,2);
        D_ENCRYPT(right,left,0);
    }

    // Leave the lookup representation, undo the bit permutation and store swapped halves.
    left = RotateRight(left,3);
    right = RotateRight(right,3);

    FP(right,left);

    data[0] = left;
    data[1] = right;
    left=right=t=u=0;
}

#pragma use_lmw_stmw reset
// Read and write one eight-byte DES block in little-endian word order, then clear temporary block words.
// src/dst each cover eight bytes; they may overlap because all input is loaded
// before output begins. keyAddress/direction have the word routine's contract.
void DES_CryptBlockBytes(unsigned char* src, unsigned char* dst, int keyAddress, int direction) {
    // Assemble two little-endian words explicitly on this big-endian CPU. Preserve
    // the volatile byte loads/store: simplifying them can change generated code.
    unsigned int blockWords[2];
    volatile unsigned char* sourceBytes = (volatile unsigned char*)src;
    unsigned int highByte0;
    unsigned int secondByte0 = sourceBytes[1];
    unsigned int firstWord = sourceBytes[0];
    highByte0 = sourceBytes[3];
    firstWord = (firstWord & ~0x0000FF00u) | (secondByte0 << 8);
    unsigned int thirdByte0 = sourceBytes[2];
    highByte0 = highByte0 << 24;
    unsigned int shiftedThirdByte0 = thirdByte0 << 16;
    firstWord = firstWord | shiftedThirdByte0;
    firstWord = firstWord | highByte0;
    *(volatile unsigned int*)&blockWords[0] = firstWord;

    unsigned int highByte1 = sourceBytes[7];
    unsigned int thirdByte1;
    unsigned int secondByte1 = sourceBytes[5];
    unsigned int secondWord = sourceBytes[4];
    highByte1 = highByte1 << 24;
    thirdByte1 = sourceBytes[6];
    secondWord = (secondWord & ~0x0000FF00u) | (secondByte1 << 8);
    unsigned int shiftedThirdByte1 = thirdByte1 << 16;
    secondWord = secondWord | shiftedThirdByte1;
    secondWord = secondWord | highByte1;
    blockWords[1] = secondWord;

    // Transform only the local words; neither input nor schedule is retained.
    DES_CryptBlock(blockWords, keyAddress, direction);
    unsigned int outputWord = blockWords[0];
    dst[0] = (unsigned char)outputWord;
    dst[1] = (unsigned char)(outputWord >> 8);
    dst[2] = (unsigned char)(outputWord >> 16);
    dst[3] = (unsigned char)(outputWord >> 24);
    outputWord = blockWords[1];
    dst[4] = (unsigned char)outputWord;
    dst[5] = (unsigned char)(outputWord >> 8);
    dst[6] = (unsigned char)(outputWord >> 16);
    dst[7] = (unsigned char)(outputWord >> 24);
    // Preserve the original temporary clearing; this is not a wipe of caller buffers.
    blockWords[1] = 0;
    blockWords[0] = 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462424", _ctors_u8028CB84_0, AuxiliaryAngles_InitFirstPeriod);
STATIC_INIT_ENTRY(".CRT$XCU80462424", _ctors_u8028CB84_1, AuxiliaryAngles_InitSecondPeriod);
#pragma pop
