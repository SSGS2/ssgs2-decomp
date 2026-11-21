// PartyRibbonState: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_PARTYRIBBONSTATE_H
#define CLASSES_PARTYRIBBONSTATE_H

class PartyRibbonState {
public:
    char pad_000[0x18];
    void* pointPositions; // 0x18: party_CPartyItemRakugakiCrayon_dtor
    void* baseVertexTriplets; // 0x1c: party_CPartyItemRakugakiCrayon_dtor
    void* fadedVertexTriplets; // 0x20: party_CPartyItemRakugakiCrayon_dtor
    void* pointHeadings; // 0x24: party_CPartyItemRakugakiCrayon_dtor
    char pad_028[0x20];
    int bufferCount; // 0x48: party_CPartyItemRakugakiCrayon_dtor
};

#define OFF(f, o) typedef char PartyRibbonState_off_##f[((int)&((PartyRibbonState*)0)->f == (o)) ? 1 : -1]
OFF(pointPositions, 0x18);
OFF(baseVertexTriplets, 0x1c);
OFF(fadedVertexTriplets, 0x20);
OFF(pointHeadings, 0x24);
OFF(bufferCount, 0x48);
#undef OFF

#endif
