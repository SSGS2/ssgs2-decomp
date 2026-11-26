// CPartyItemSilentMisanga: the party item that widens the shot-gauge and impact
// zones and slows the gauge while the owner is in motion-swing control mode.
#ifndef CLASSES_CPARTYITEMSILENTMISANGA_H
#define CLASSES_CPARTYITEMSILENTMISANGA_H

class CPartyItemSilentMisanga {
public:
    CPartyItemSilentMisanga(void* arg);

    void* vtbl;                 // 0x00
    char pad_004[0x8];          // 0x04
    int ownerIndex;             // 0x0C
    int active;                 // 0x10
    int oneMeterWindEnabled;    // 0x14
    float pangyaZoneWidthBonus; // 0x18
    float gaugeSpeedMultiplier; // 0x1C
    float impactZoneWidthBonus; // 0x20
};

#define CPARTYITEMSILENTMISANGA_OFF(f, o) \
    typedef char CPartyItemSilentMisanga_off_##f[((int)&((CPartyItemSilentMisanga*)0)->f == (o)) ? 1 : -1]
CPARTYITEMSILENTMISANGA_OFF(vtbl, 0x00);
CPARTYITEMSILENTMISANGA_OFF(ownerIndex, 0x0C);
CPARTYITEMSILENTMISANGA_OFF(active, 0x10);
CPARTYITEMSILENTMISANGA_OFF(oneMeterWindEnabled, 0x14);
CPARTYITEMSILENTMISANGA_OFF(pangyaZoneWidthBonus, 0x18);
CPARTYITEMSILENTMISANGA_OFF(gaugeSpeedMultiplier, 0x1C);
CPARTYITEMSILENTMISANGA_OFF(impactZoneWidthBonus, 0x20);
#undef CPARTYITEMSILENTMISANGA_OFF
typedef char CPartyItemSilentMisanga_size[(sizeof(CPartyItemSilentMisanga) == 0x24) ? 1 : -1];

#endif
