#ifndef DECOMP_OFFSET_OF_H
#define DECOMP_OFFSET_OF_H

// DECOMP_OFFSETOF(T, member) is the field-offset idiom `&((T*)0)->member`. On the Wii it expands to exactly that
// own offset operator, which yields the same byte offset.
#define DECOMP_OFFSETOF(T, member) &((T*)0)->member

#endif
