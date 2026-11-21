#ifndef DECOMP_CT_MEMBER_H
#define DECOMP_CT_MEMBER_H

// CodeWarrior accepts an explicit constructor call on raw storage: `storage->__ct(args)` runs the class's constructor
// the unit's or the standard library's own placement operator new); on the Wii build it expands to nothing.
#define DECOMP_CT_MEMBER(C)

#endif
