// PartyItemModifiers: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_PARTYITEMMODIFIERS_H
#define CLASSES_PARTYITEMMODIFIERS_H

class PartyItemModifiers {
public:
    float clubDistanceBonusRate; // 0x0: CPartyItemFireBantage_ApplyOwnerModifiers, CPartyItemFireBantage_ApplyModifiersIfActive
    float impactZoneWidthBonus; // 0x4: CPartyItemFireBantage_ApplyOwnerModifiers, CPartyItemFireBantage_ApplyModifiersIfActive, party_CPartyItemSilentMisanga_vfunc_3, party_CPartyItemSilentMisanga_vfunc_4
    float pangyaZoneWidthBonus; // 0x8: CPartyItemLuckyBracelet_ApplyOwnerModifiers, CPartyItemLuckyBracelet_ApplyModifiersIfActive, party_CPartyItemSilentMisanga_vfunc_3, party_CPartyItemSilentMisanga_vfunc_4
    float gaugeSpeedMultiplier; // 0xc: party_CPartyItemSilentMisanga_vfunc_3, party_CPartyItemSilentMisanga_vfunc_4
    float positiveShotOutcomeBonusRate; // 0x10: CPartyItemLuckyBracelet_ApplyOwnerModifiers, CPartyItemLuckyBracelet_ApplyModifiersIfActive
    float launchVerticalVelocityBonus; // 0x14: CPartyItemFeatherBadge_ApplyModifiersToPlayer, CPartyItemFeatherBadge_ApplyModifiersIfActive
    float windSpeedOverride; // 0x18: CPartyItemHurricaneBadge_ApplyWindOverrides
    float windSpeed; // 0x1c: CPartyItemFujinUchiwa_UpdateOpponentWind
    float windHeading; // 0x20: CPartyItemFujinUchiwa_UpdateOpponentWind
    float spinRangeY; // 0x24: CPartyItemFeatherBadge_ApplyModifiersToPlayer, CPartyItemFeatherBadge_ApplyModifiersIfActive
    float spinRangeX; // 0x28: CPartyItemFeatherBadge_ApplyModifiersToPlayer, CPartyItemFeatherBadge_ApplyModifiersIfActive
    float impactZoneWidthDivisor; // 0x2c: CPartyItemPululuJarashi_UpdateTickleStrength
    float ballScaleBonus; // 0x30: party_CPartyItemTitanWristband_vfunc_3, party_CPartyItemTitanWristband_vfunc_4
    float unk_34; // 0x34: party_CPartyItemTitanWristband_vfunc_3
    int timebombTicks; // 0x38: party_CPartyItemTimebombBadge_UpdateTimebomb
    int timebombPeriod; // 0x3c: party_CPartyItemTimebombBadge_UpdateTimebomb
    int oneMeterWindEnabled; // 0x40: CPartyItemHurricaneBadge_ApplyWindOverrides, party_CPartyItemSilentMisanga_vfunc_3
    int windSpeedOverrideEnabled; // 0x44: CPartyItemHurricaneBadge_ApplyWindOverrides
    int spinRangeOverrideEnabled; // 0x48: CPartyItemFeatherBadge_ApplyModifiersToPlayer
    char pad_04C[0x4];
    int timebombTriggered; // 0x50: party_CPartyItemTimebombBadge_UpdateTimebomb
    int tickleActive; // 0x54: CPartyItemPululuJarashi_UpdateTickleStrength
};

#define OFF(f, o) typedef char PartyItemModifiers_off_##f[((int)&((PartyItemModifiers*)0)->f == (o)) ? 1 : -1]
OFF(clubDistanceBonusRate, 0x0);
OFF(impactZoneWidthBonus, 0x4);
OFF(pangyaZoneWidthBonus, 0x8);
OFF(gaugeSpeedMultiplier, 0xc);
OFF(positiveShotOutcomeBonusRate, 0x10);
OFF(launchVerticalVelocityBonus, 0x14);
OFF(windSpeedOverride, 0x18);
OFF(windSpeed, 0x1c);
OFF(windHeading, 0x20);
OFF(spinRangeY, 0x24);
OFF(spinRangeX, 0x28);
OFF(impactZoneWidthDivisor, 0x2c);
OFF(ballScaleBonus, 0x30);
OFF(unk_34, 0x34);
OFF(timebombTicks, 0x38);
OFF(timebombPeriod, 0x3c);
OFF(oneMeterWindEnabled, 0x40);
OFF(windSpeedOverrideEnabled, 0x44);
OFF(spinRangeOverrideEnabled, 0x48);
OFF(timebombTriggered, 0x50);
OFF(tickleActive, 0x54);
#undef OFF

#endif
