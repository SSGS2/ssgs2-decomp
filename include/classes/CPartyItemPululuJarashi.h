// CPartyItemPululuJarashi: the tickle party item. Gestures build its strength
// up to one; the strength decays every update and widens the impact zone of the
// opponent the item is aimed at.
#ifndef CLASSES_CPARTYITEMPULULUJARASHI_H
#define CLASSES_CPARTYITEMPULULUJARASHI_H

class CPartyItemPululuJarashi {
public:
    CPartyItemPululuJarashi(void* arg);
    ~CPartyItemPululuJarashi();

    int UpdateTickleStrength(void* unused, void* destination, int index);

    void* vtbl;        // 0x00
    char pad_004[0x8]; // 0x04
    int ownerIndex;    // 0x0C
    float strength;    // 0x10  0 to 1
    int activeTicks;   // 0x14  ticks the gesture stays active
};

#define CPARTYITEMPULULUJARASHI_OFF(f, o) \
    typedef char CPartyItemPululuJarashi_off_##f[((int)&((CPartyItemPululuJarashi*)0)->f == (o)) ? 1 : -1]
CPARTYITEMPULULUJARASHI_OFF(vtbl, 0x00);
CPARTYITEMPULULUJARASHI_OFF(ownerIndex, 0x0C);
CPARTYITEMPULULUJARASHI_OFF(strength, 0x10);
CPARTYITEMPULULUJARASHI_OFF(activeTicks, 0x14);
#undef CPARTYITEMPULULUJARASHI_OFF
typedef char CPartyItemPululuJarashi_size[(sizeof(CPartyItemPululuJarashi) == 0x18) ? 1 : -1];

#endif
