#ifndef CLASSES_CBUBBLE_H
#define CLASSES_CBUBBLE_H

#include "game/types.h"

// The data layout of CBubble has not been recovered yet. Until it is, its member functions reach the
// object's data through file-local layouts or raw offsets.
struct Event;
class CBubble {
public:
    void UpdateTextFadeOut(float dt);
    void SetCaddieMessage(wchar_t* input);
    void DrawAlignedBalloon(void* camera, float x, float y, int mode);
    int IsActiveForCurrentPlayer();

    void OnLoad();
    void OnProcess(float deltaTime);
    void HandleMsg(const Event* event);
    void OnDestroy();
    void OnDisplay();
    static void* GetRTTI();

    ~CBubble();
};

#endif // CLASSES_CBUBBLE_H
