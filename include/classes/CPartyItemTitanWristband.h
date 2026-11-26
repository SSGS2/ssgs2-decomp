// CPartyItemTitanWristband: the party item that grows the ball while it is in
// the first few or the last few steps of its flight.
#ifndef CLASSES_CPARTYITEMTITANWRISTBAND_H
#define CLASSES_CPARTYITEMTITANWRISTBAND_H

class CPartyItemTitanWristband {
public:
    CPartyItemTitanWristband(void* arg);

    void* vtbl;                         // 0x00
    char pad_004[0x8];                  // 0x04
    int ownerIndex;                     // 0x0C
    float baseValue;                    // 0x10  ball collision radius before the ramp
    float maximumRampModifier;          // 0x14
    float collisionRadiusIncreaseRatio; // 0x18
    int rampLength;                     // 0x1C  steps at each end of the flight
};

#define CPARTYITEMTITANWRISTBAND_OFF(f, o) \
    typedef char CPartyItemTitanWristband_off_##f[((int)&((CPartyItemTitanWristband*)0)->f == (o)) ? 1 : -1]
CPARTYITEMTITANWRISTBAND_OFF(vtbl, 0x00);
CPARTYITEMTITANWRISTBAND_OFF(ownerIndex, 0x0C);
CPARTYITEMTITANWRISTBAND_OFF(baseValue, 0x10);
CPARTYITEMTITANWRISTBAND_OFF(maximumRampModifier, 0x14);
CPARTYITEMTITANWRISTBAND_OFF(collisionRadiusIncreaseRatio, 0x18);
CPARTYITEMTITANWRISTBAND_OFF(rampLength, 0x1C);
#undef CPARTYITEMTITANWRISTBAND_OFF
typedef char CPartyItemTitanWristband_size[(sizeof(CPartyItemTitanWristband) == 0x20) ? 1 : -1];

#endif
