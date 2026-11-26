// RandomTable: layout view; rain_effect_and_random_table.cpp: MT19937_NextU32/MT19937_Seed use a 624-word table and saved seed
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_RANDOMTABLE_H
#define CLASSES_RANDOMTABLE_H

struct RandomTable {
    char pad_000[0x10];
    unsigned int* data; // 0x10
    int nextWordIndex;  // 0x14
    unsigned int seed;  // 0x18
};

#define OFF(f, o) typedef char RandomTable_off_##f[((int)&((RandomTable*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x10);
OFF(nextWordIndex, 0x14);
OFF(seed, 0x18);
#undef OFF
#define AsRandomTable(p) ((RandomTable*)(p))

#endif
