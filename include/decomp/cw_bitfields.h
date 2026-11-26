//
// CodeWarrior (PowerPC, big-endian) allocates the fields of a storage unit from its most significant bit, in
// CodeWarrior value layout, e.g. "isLong : 1; rest : 31" means the tag is bit 31 of the 32-bit word.
//
// CW_BITSn(f1, ..., fn): n fields of one storage unit that they fill completely, written in the original order,
//   each as a declaration without its semicolon:   CW_BITS2(unsigned int refs : 16, unsigned int flags : 16)
// CW_BITSPn(T, pad, f1, ..., fn): the same when the fields do not fill the unit; `pad` unused low bits of type T
//   (CodeWarrior leaves them at the bottom of the unit).
// On the Wii build both expand to the plain declarations in the original order (the preprocessed Wii source is
// Values are bit positions in the value of the unit, so memory that holds Wii words must be byte-swapped to
#ifndef DECOMP_CW_BITFIELDS_H
#define DECOMP_CW_BITFIELDS_H

#define CW_BITS1(a) a;
#define CW_BITSP1(T, pad, a) a;
#define CW_BITS2(a, b) \
    a;                 \
    b;
#define CW_BITSP2(T, pad, a, b) \
    a;                          \
    b;
#define CW_BITS3(a, b, c) \
    a;                    \
    b;                    \
    c;
#define CW_BITSP3(T, pad, a, b, c) \
    a;                             \
    b;                             \
    c;
#define CW_BITS4(a, b, c, d) \
    a;                       \
    b;                       \
    c;                       \
    d;
#define CW_BITSP4(T, pad, a, b, c, d) \
    a;                                \
    b;                                \
    c;                                \
    d;
#define CW_BITS5(a, b, c, d, e) \
    a;                          \
    b;                          \
    c;                          \
    d;                          \
    e;
#define CW_BITSP5(T, pad, a, b, c, d, e) \
    a;                                   \
    b;                                   \
    c;                                   \
    d;                                   \
    e;
#define CW_BITS6(a, b, c, d, e, f) \
    a;                             \
    b;                             \
    c;                             \
    d;                             \
    e;                             \
    f;
#define CW_BITSP6(T, pad, a, b, c, d, e, f) \
    a;                                      \
    b;                                      \
    c;                                      \
    d;                                      \
    e;                                      \
    f;
#define CW_BITS7(a, b, c, d, e, f, g) \
    a;                                \
    b;                                \
    c;                                \
    d;                                \
    e;                                \
    f;                                \
    g;
#define CW_BITSP7(T, pad, a, b, c, d, e, f, g) \
    a;                                         \
    b;                                         \
    c;                                         \
    d;                                         \
    e;                                         \
    f;                                         \
    g;
#define CW_BITS8(a, b, c, d, e, f, g, h) \
    a;                                   \
    b;                                   \
    c;                                   \
    d;                                   \
    e;                                   \
    f;                                   \
    g;                                   \
    h;
#define CW_BITSP8(T, pad, a, b, c, d, e, f, g, h) \
    a;                                            \
    b;                                            \
    c;                                            \
    d;                                            \
    e;                                            \
    f;                                            \
    g;                                            \
    h;
#define CW_BITS9(a, b, c, d, e, f, g, h, i) \
    a;                                      \
    b;                                      \
    c;                                      \
    d;                                      \
    e;                                      \
    f;                                      \
    g;                                      \
    h;                                      \
    i;
#define CW_BITSP9(T, pad, a, b, c, d, e, f, g, h, i) \
    a;                                               \
    b;                                               \
    c;                                               \
    d;                                               \
    e;                                               \
    f;                                               \
    g;                                               \
    h;                                               \
    i;
#define CW_BITS10(a, b, c, d, e, f, g, h, i, j) \
    a;                                          \
    b;                                          \
    c;                                          \
    d;                                          \
    e;                                          \
    f;                                          \
    g;                                          \
    h;                                          \
    i;                                          \
    j;
#define CW_BITSP10(T, pad, a, b, c, d, e, f, g, h, i, j) \
    a;                                                   \
    b;                                                   \
    c;                                                   \
    d;                                                   \
    e;                                                   \
    f;                                                   \
    g;                                                   \
    h;                                                   \
    i;                                                   \
    j;
#define CW_BITS11(a, b, c, d, e, f, g, h, i, j, k) \
    a;                                             \
    b;                                             \
    c;                                             \
    d;                                             \
    e;                                             \
    f;                                             \
    g;                                             \
    h;                                             \
    i;                                             \
    j;                                             \
    k;
#define CW_BITSP11(T, pad, a, b, c, d, e, f, g, h, i, j, k) \
    a;                                                      \
    b;                                                      \
    c;                                                      \
    d;                                                      \
    e;                                                      \
    f;                                                      \
    g;                                                      \
    h;                                                      \
    i;                                                      \
    j;                                                      \
    k;
#define CW_BITS12(a, b, c, d, e, f, g, h, i, j, k, l) \
    a;                                                \
    b;                                                \
    c;                                                \
    d;                                                \
    e;                                                \
    f;                                                \
    g;                                                \
    h;                                                \
    i;                                                \
    j;                                                \
    k;                                                \
    l;
#define CW_BITSP12(T, pad, a, b, c, d, e, f, g, h, i, j, k, l) \
    a;                                                         \
    b;                                                         \
    c;                                                         \
    d;                                                         \
    e;                                                         \
    f;                                                         \
    g;                                                         \
    h;                                                         \
    i;                                                         \
    j;                                                         \
    k;                                                         \
    l;
#define CW_BITS13(a, b, c, d, e, f, g, h, i, j, k, l, m) \
    a;                                                   \
    b;                                                   \
    c;                                                   \
    d;                                                   \
    e;                                                   \
    f;                                                   \
    g;                                                   \
    h;                                                   \
    i;                                                   \
    j;                                                   \
    k;                                                   \
    l;                                                   \
    m;
#define CW_BITSP13(T, pad, a, b, c, d, e, f, g, h, i, j, k, l, m) \
    a;                                                            \
    b;                                                            \
    c;                                                            \
    d;                                                            \
    e;                                                            \
    f;                                                            \
    g;                                                            \
    h;                                                            \
    i;                                                            \
    j;                                                            \
    k;                                                            \
    l;                                                            \
    m;
#define CW_BITS14(a, b, c, d, e, f, g, h, i, j, k, l, m, n) \
    a;                                                      \
    b;                                                      \
    c;                                                      \
    d;                                                      \
    e;                                                      \
    f;                                                      \
    g;                                                      \
    h;                                                      \
    i;                                                      \
    j;                                                      \
    k;                                                      \
    l;                                                      \
    m;                                                      \
    n;
#define CW_BITSP14(T, pad, a, b, c, d, e, f, g, h, i, j, k, l, m, n) \
    a;                                                               \
    b;                                                               \
    c;                                                               \
    d;                                                               \
    e;                                                               \
    f;                                                               \
    g;                                                               \
    h;                                                               \
    i;                                                               \
    j;                                                               \
    k;                                                               \
    l;                                                               \
    m;                                                               \
    n;
#define CW_BITS15(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
    a;                                                         \
    b;                                                         \
    c;                                                         \
    d;                                                         \
    e;                                                         \
    f;                                                         \
    g;                                                         \
    h;                                                         \
    i;                                                         \
    j;                                                         \
    k;                                                         \
    l;                                                         \
    m;                                                         \
    n;                                                         \
    o;
#define CW_BITSP15(T, pad, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o) \
    a;                                                                  \
    b;                                                                  \
    c;                                                                  \
    d;                                                                  \
    e;                                                                  \
    f;                                                                  \
    g;                                                                  \
    h;                                                                  \
    i;                                                                  \
    j;                                                                  \
    k;                                                                  \
    l;                                                                  \
    m;                                                                  \
    n;                                                                  \
    o;
#define CW_BITS16(a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
    a;                                                            \
    b;                                                            \
    c;                                                            \
    d;                                                            \
    e;                                                            \
    f;                                                            \
    g;                                                            \
    h;                                                            \
    i;                                                            \
    j;                                                            \
    k;                                                            \
    l;                                                            \
    m;                                                            \
    n;                                                            \
    o;                                                            \
    p;
#define CW_BITSP16(T, pad, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p) \
    a;                                                                     \
    b;                                                                     \
    c;                                                                     \
    d;                                                                     \
    e;                                                                     \
    f;                                                                     \
    g;                                                                     \
    h;                                                                     \
    i;                                                                     \
    j;                                                                     \
    k;                                                                     \
    l;                                                                     \
    m;                                                                     \
    n;                                                                     \
    o;                                                                     \
    p;
#endif
