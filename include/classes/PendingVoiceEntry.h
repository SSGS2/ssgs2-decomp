// Pending voice entry owned by CVoiceManager_OnProcess (0x802E6030 unit).
// The 0x10-byte vector entry contains a short string followed by its delay.
#ifndef CLASSES_PENDINGVOICE802E6030_H
#define CLASSES_PENDINGVOICE802E6030_H

struct PendingVoiceEntry {
    char soundName[0xC]; // 0x0: passed to String_ReplaceRange when compacting entries
    float delaySeconds; // 0xC: OnProcess subtracts dt and plays the name when negative
};

#define OFF(f, o) typedef char PendingVoice802E6030_off_##f[((int)&((PendingVoiceEntry*)0)->f == (o)) ? 1 : -1]
OFF(soundName, 0x0);
OFF(delaySeconds, 0xC);
#undef OFF
typedef char PendingVoice802E6030_size[(sizeof(PendingVoiceEntry) == 0x10) ? 1 : -1];
#define AsPendingVoice802E6030(p) ((PendingVoiceEntry*)(p))

#endif
