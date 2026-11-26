// CPartyItemTimebombBadge: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_CPARTYITEMTIMEBOMBBADGE_H
#define CLASSES_CPARTYITEMTIMEBOMBBADGE_H

class CPartyItemTimebombBadge {
public:
    CPartyItemTimebombBadge(int size);

    void* vtbl; // 0x0
    char pad_004[0xc];
    int elapsedTicks; // 0x10: party_CPartyItemTimebombBadge_vfunc_3, party_CPartyItemTimebombBadge_UpdateTimebomb
    int period;       // 0x14: party_CPartyItemTimebombBadge_UpdateTimebomb
};

#define OFF(f, o) typedef char CPartyItemTimebombBadge_off_##f[((int)&((CPartyItemTimebombBadge*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(elapsedTicks, 0x10);
OFF(period, 0x14);
#undef OFF

#endif
