#ifndef DECOMP_STATIC_INIT_H
#define DECOMP_STATIC_INIT_H

// Static-initialiser table entries (CodeWarrior .ctors) and data-section placement.
// Wii/CodeWarrior: the original spelling, so the generated object is unchanged.
// hex == address order) and the CRT runs the .CRT$XCx range before main, so the tables run in the Wii order:
// units by address, entries inside one unit in declaration order (the Wii _N order).
// 'extern' gives the const tables external linkage so the compiler keeps them (internal-linkage constants were
// dropped from the object); the names embed the unit address and are unique.
// Section names must be literals for __declspec(allocate), hence the literal as first argument.
#define STATIC_INIT_ENTRY(sec, name, init) __declspec(section ".ctors") void (*const name)() = init
#define STATIC_INIT_ENTRY_NC(sec, name, init) __declspec(section ".ctors") void (*name)() = init
#define STATIC_INIT_ENTRY_STATIC(sec, name, init) static __declspec(section ".ctors") void (*name)() = init
#define DATA_SECTION(x) __declspec(section x)

#endif
