// PartyItemRecord: explicit layout view; no inheritance, virtual methods, or RTTI.
// Record initializer PartyItemRecord_InitDefaults lives in party_item_select.cpp; unknown meanings retained.
#ifndef CLASSES_PARTYITEMRECORD_H
#define CLASSES_PARTYITEMRECORD_H

#include "game/types.h"

class PartyItemRecord {
public:
    int characterInventoryIndex; // 0x0: PartyItemRecord_InitDefaults: initialized to -1
    int hairStyleInventoryIndex; // 0x4: PartyItemRecord_InitDefaults: initialized record field
    int unk_8;                   // 0x8: PartyItemRecord_InitDefaults: initialized record field
    int characterIndex;          // 0xC: PartyItemRecord_InitDefaults: initialized record field
    int clubSetInventoryIndex;   // 0x10: PartyItemRecord_InitDefaults: initialized record field
    int ballInventoryIndex;      // 0x14: PartyItemRecord_InitDefaults: initialized record field
    int playerIndex;             // 0x18: PartyItemRecord_InitDefaults: initialized record field
    unsigned char levelIndex;    // 0x1C: PartyItemRecord_InitDefaults: initialized record field
    signed char unk_1D;          // 0x1D: PartyItemRecord_InitDefaults: initialized record field
    unsigned short flags;        // 0x1E: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex0;   // 0x20: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex1;   // 0x22: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex2;   // 0x24: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex3;   // 0x26: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex4;   // 0x28: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex5;   // 0x2A: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex6;   // 0x2C: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex7;   // 0x2E: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex8;   // 0x30: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex9;   // 0x32: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex10;  // 0x34: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex11;  // 0x36: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex12;  // 0x38: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex13;  // 0x3A: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex14;  // 0x3C: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex15;  // 0x3E: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex16;  // 0x40: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex17;  // 0x42: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex18;  // 0x44: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex19;  // 0x46: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex20;  // 0x48: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex21;  // 0x4A: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex22;  // 0x4C: PartyItemRecord_InitDefaults: initialized record field
    short partInventoryIndex23;  // 0x4E: PartyItemRecord_InitDefaults: initialized record field
    int selection;  // 0x50: CPartyItemSelect_UpdateSelectionAndDetail: choice stored here; -1 means incomplete
    int nameFlags;  // 0x54: PartyItemRecord_InitDefaults: initialized record field
    int nameLength; // 0x58: PartyItemRecord_InitDefaults: initialized record field
    int nameData;   // 0x5C: PartyItemRecord_InitDefaults: initialized record field
    unsigned char equipmentPowerBonus;    // 0x60: PartyItemRecord_InitDefaults: initialized record field
    unsigned char equipmentControlBonus;  // 0x61: PartyItemRecord_InitDefaults: initialized record field
    unsigned char equipmentAccuracyBonus; // 0x62: PartyItemRecord_InitDefaults: initialized record field
    unsigned char equipmentSpinBonus;     // 0x63: PartyItemRecord_InitDefaults: initialized record field
    unsigned char equipmentCurveBonus;    // 0x64: PartyItemRecord_InitDefaults: initialized record field
    char pad_065[0x3];
    int unk_68;                     // 0x68: PartyItemRecord_InitDefaults: initialized record field
    int unk_6C;                     // 0x6C: PartyItemRecord_InitDefaults: initialized record field
    int unk_70;                     // 0x70: PartyItemRecord_InitDefaults: initialized record field
    int unk_74;                     // 0x74: PartyItemRecord_InitDefaults: initialized record field
    int unk_78;                     // 0x78: PartyItemRecord_InitDefaults: initialized record field
    int unk_7C;                     // 0x7C: PartyItemRecord_InitDefaults: initialized record field
    int unk_80;                     // 0x80: PartyItemRecord_InitDefaults: initialized record field
    int unk_84;                     // 0x84: PartyItemRecord_InitDefaults: initialized record field
    int unk_88;                     // 0x88: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_8C;           // 0x8C: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_8D;           // 0x8D: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_8E;           // 0x8E: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_8F;           // 0x8F: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_90;           // 0x90: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_91;           // 0x91: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_92;           // 0x92: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_93;           // 0x93: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_94;           // 0x94: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_95;           // 0x95: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_96;           // 0x96: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_97;           // 0x97: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_98;           // 0x98: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_99;           // 0x99: PartyItemRecord_InitDefaults: initialized record field
    unsigned char unk_9A;           // 0x9A: PartyItemRecord_InitDefaults: initialized record field
    unsigned char fallbackPower;    // 0x9B: PartyItemRecord_InitDefaults: initialized record field
    unsigned char fallbackControl;  // 0x9C: PartyItemRecord_InitDefaults: initialized record field
    unsigned char fallbackAccuracy; // 0x9D: PartyItemRecord_InitDefaults: initialized record field
    unsigned char fallbackSpin;     // 0x9E: PartyItemRecord_InitDefaults: initialized record field
    unsigned char fallbackCurve;    // 0x9F: PartyItemRecord_InitDefaults: initialized record field
    int renderObject;               // 0xA0: PartyItemRecord_InitDefaults: initialized record field
};

#define OFF(f, o) typedef char PartyItemRecord_off_##f[((int)&((PartyItemRecord*)0)->f == (o)) ? 1 : -1]
OFF(characterInventoryIndex, 0x0);
OFF(hairStyleInventoryIndex, 0x4);
OFF(unk_8, 0x8);
OFF(characterIndex, 0xC);
OFF(clubSetInventoryIndex, 0x10);
OFF(ballInventoryIndex, 0x14);
OFF(playerIndex, 0x18);
OFF(levelIndex, 0x1C);
OFF(unk_1D, 0x1D);
OFF(flags, 0x1E);
OFF(partInventoryIndex0, 0x20);
OFF(partInventoryIndex1, 0x22);
OFF(partInventoryIndex2, 0x24);
OFF(partInventoryIndex3, 0x26);
OFF(partInventoryIndex4, 0x28);
OFF(partInventoryIndex5, 0x2A);
OFF(partInventoryIndex6, 0x2C);
OFF(partInventoryIndex7, 0x2E);
OFF(partInventoryIndex8, 0x30);
OFF(partInventoryIndex9, 0x32);
OFF(partInventoryIndex10, 0x34);
OFF(partInventoryIndex11, 0x36);
OFF(partInventoryIndex12, 0x38);
OFF(partInventoryIndex13, 0x3A);
OFF(partInventoryIndex14, 0x3C);
OFF(partInventoryIndex15, 0x3E);
OFF(partInventoryIndex16, 0x40);
OFF(partInventoryIndex17, 0x42);
OFF(partInventoryIndex18, 0x44);
OFF(partInventoryIndex19, 0x46);
OFF(partInventoryIndex20, 0x48);
OFF(partInventoryIndex21, 0x4A);
OFF(partInventoryIndex22, 0x4C);
OFF(partInventoryIndex23, 0x4E);
OFF(selection, 0x50);
OFF(nameFlags, 0x54);
OFF(nameLength, 0x58);
OFF(nameData, 0x5C);
OFF(equipmentPowerBonus, 0x60);
OFF(equipmentControlBonus, 0x61);
OFF(equipmentAccuracyBonus, 0x62);
OFF(equipmentSpinBonus, 0x63);
OFF(equipmentCurveBonus, 0x64);
OFF(unk_68, 0x68);
OFF(unk_6C, 0x6C);
OFF(unk_70, 0x70);
OFF(unk_74, 0x74);
OFF(unk_78, 0x78);
OFF(unk_7C, 0x7C);
OFF(unk_80, 0x80);
OFF(unk_84, 0x84);
OFF(unk_88, 0x88);
OFF(unk_8C, 0x8C);
OFF(unk_8D, 0x8D);
OFF(unk_8E, 0x8E);
OFF(unk_8F, 0x8F);
OFF(unk_90, 0x90);
OFF(unk_91, 0x91);
OFF(unk_92, 0x92);
OFF(unk_93, 0x93);
OFF(unk_94, 0x94);
OFF(unk_95, 0x95);
OFF(unk_96, 0x96);
OFF(unk_97, 0x97);
OFF(unk_98, 0x98);
OFF(unk_99, 0x99);
OFF(unk_9A, 0x9A);
OFF(fallbackPower, 0x9B);
OFF(fallbackControl, 0x9C);
OFF(fallbackAccuracy, 0x9D);
OFF(fallbackSpin, 0x9E);
OFF(fallbackCurve, 0x9F);
OFF(renderObject, 0xA0);
#undef OFF

#endif
