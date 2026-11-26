// Entry view owned by golf_caddie_dialogue_entries.cpp. CGolfCaddie::CreateCaddieEntry
// allocates 0x48 bytes, initializes via GolfCaddieEntry_Init, and stores the result of
// ItemInventory_FindCaddieData at +0x10. The two state bytes have no established meaning here.
#ifndef CLASSES_GOLFCADDIEENTRY80128808_H
#define CLASSES_GOLFCADDIEENTRY80128808_H
#include "iff/IffStruct.h"

struct GolfCaddieEntryView {
    char pad_0[0x10];
    IFF_STRUCT::sCaddie* caddieRecord; // 0x10: CreateCaddieEntry stores the IFF record
    char pad_14[0x30];
    char ringEnabled;
    char caddieViewEnabled;
    char pad_46[2];
};

#define OFF(f, o) typedef char GolfCaddieEntry80128808_off_##f[((int)&((GolfCaddieEntryView*)0)->f == (o)) ? 1 : -1]
OFF(caddieRecord, 0x10);
OFF(ringEnabled, 0x44);
OFF(caddieViewEnabled, 0x45);
#undef OFF
typedef char GolfCaddieEntry80128808_size_check[(sizeof(GolfCaddieEntryView) == 0x48) ? 1 : -1];
#define AsGolfCaddieEntry80128808(p) ((GolfCaddieEntryView*)(p))

#endif
