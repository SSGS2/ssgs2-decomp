// SoundChannelGroup. Two playback records with a leading selected-channel index in sound_manager.cpp.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_SOUNDCHANNELGROUP_H
#define CLASSES_SOUNDCHANNELGROUP_H

#include "classes/SoundPlaybackState.h"

class SoundChannelGroup {
public:
    int selected; // 0x0: SoundChannelGroup_StopWithFade selects a channel with stride 0x64
    SoundPlaybackState channels
        [2]; // 0x4: SoundChannelGroup_StopWithFade, SoundManager_PausePrimaryStreams loop over two playback records
};

#define OFF(f, o) typedef char SoundChannelGroup_off_##f[((int)&((SoundChannelGroup*)0)->f == (o)) ? 1 : -1]
OFF(selected, 0x0);
OFF(channels, 0x4);
#undef OFF
#define AsSoundChannelGroup(p) ((SoundChannelGroup*)(p))

#endif
