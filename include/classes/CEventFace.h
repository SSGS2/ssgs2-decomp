#ifndef CLASSES_CEVENTFACE_H
#define CLASSES_CEVENTFACE_H

#include "game/types.h"

// The data layout of CEventFace has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CEventFace {
public:
    void SelectModelAndTrack(unsigned char* input);
    void ApplyLastFrameAndClearTrack(unsigned char index);
    void UpdateTracks();
    void SetTrackFrameHold(unsigned char index, void* value);
    void AppendKeyframe(unsigned char* value) const;

    CEventFace();
    ~CEventFace();
};

#endif // CLASSES_CEVENTFACE_H
