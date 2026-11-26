#ifndef CLASSES_CRVLPOINTINGDEVICEDRIVER_H
#define CLASSES_CRVLPOINTINGDEVICEDRIVER_H

#include "game/types.h"

// The data layout of CRvlPointingDeviceDriver has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
class CRvlPointingDeviceDriver {
public:
    int GetChannelStatus(int channel);
    int BuildStateReports(void* reportCursor, void* reportOptions, int channel);
};

#endif // CLASSES_CRVLPOINTINGDEVICEDRIVER_H
