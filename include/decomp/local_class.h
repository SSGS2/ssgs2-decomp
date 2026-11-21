#ifndef DECOMP_LOCAL_CLASS_H
#define DECOMP_LOCAL_CLASS_H

// Function-local view classes declare virtual members only to get a vtable slot call through a raw object pointer
// keeps the slot order and needs no body; on the Wii it expands to nothing, so those declarations are unchanged.
#define DECOMP_LOCAL_VIRTUAL

#endif
