// CVoiceManager: the queue of spoken callouts.
//
// The manager holds a list of voice names, each with a delay in seconds; the
// message handler fills the list and OnProcess plays the entries whose delay has
// run out. This file has only the queue and the name formatting; the handler and
// the rest of the class are in other files.
#ifndef CLASSES_CVOICEMANAGER_H
#define CLASSES_CVOICEMANAGER_H

#include "game/types.h"

class CVoiceManager {
public:
    // The class's type descriptor.
    static void* GetRTTI();
    // Register the class with the actor factory and the task registry.
    static void RegisterFactory();

    // Free the queue and the actor base; a nonzero flags word frees the object too.
    void* Destroy(int flags);

    void OnProcess(float elapsedSeconds);
    // Append the name of a character voice. The voice number selects one of the
    // two format tables; the two indices are the inventory's selector words.
    void AppendFormattedVoiceName(void* outputNames, int voiceNumber, void* characterIndex,
                                  void* alternateCharacterIndex);

    void* vtbl;            // 0x00  the class's vtable
    char pad_004[0x14];    // 0x04
    void* pendingVoices;   // 0x18  array of PendingVoiceEntry
    int pendingVoiceCount; // 0x1C

    void HandleMsg(void* arg);
};

#define CVOICEMANAGER_OFF(f, o) typedef char CVoiceManager_off_##f[((int)&((CVoiceManager*)0)->f == (o)) ? 1 : -1]
CVOICEMANAGER_OFF(pendingVoices, 0x18);
CVOICEMANAGER_OFF(pendingVoiceCount, 0x1C);
#undef CVOICEMANAGER_OFF

#endif // CLASSES_CVOICEMANAGER_H
