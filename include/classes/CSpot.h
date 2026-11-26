// CSpot: the aiming and landing marker actor on the golf course.
//
// It shows where the shot is expected to land, the distance to it, and the other
// players' markers. This header covers the whole object; loading, animation and
// teardown are in spot_marker_actor.cpp and drawing and messages are in
// spot_marker_draw_and_messages.cpp.
//
// The class is declared without virtual functions on purpose: its vtable is
// still taken from the original game data, so `vtbl` is spelled out as a member.
#ifndef CLASSES_CSPOT_H
#define CLASSES_CSPOT_H

#include "game/types.h"
#include "game/wvector.h"

// One of the nine marker textures. The loader and ready test are virtual calls
// made through the texture object's own vtable.
struct SpotTextureBinding {
    char storage[0x28];
};

// A message to the marker: a header, a type, one 32-bit value that the type
// gives a meaning to, and a second pointer some types use for a world position.
struct SpotMessage {
    unsigned int header; // 0x00
    int type;            // 0x04
    union {
        int integer;
        void* pointer;
        float* scalar;
    } value;             // 0x08
    void* secondPointer; // 0x0C
};

class CSpot {
public:
    enum {
        kTextureCount = 9
    };

    CSpot();

    // Actor callbacks.
    void OnPreLoadInit();
    void OnDestroy();
    void OnLoad();
    void OnInit();
    void OnProcess(float deltaSeconds);

    // Drawing.
    void DrawMarkersAndLabels();
    void DrawFallingMarker();
    void DrawTexturedMarker();
    void DrawPlayerMarker(int playerIndex);

    // Messages.
    void HandleMsg(const SpotMessage* message);

    void* vtbl; // 0x00: original vtable pointer
    char pad_004[4];
    int unk_08;                                        // 0x08: the constructor sets 1
    char pad_00C[0xC];                                 // 0x0C
    void* taskVtbl;                                    // 0x18: second interface; the preload queue is given its address
    void* distanceText;                                // 0x1C
    void* playerNumberText;                            // 0x20
    float position[3];                                 // 0x24: where the marker is drawn
    float previousPosition[3];                         // 0x30
    float transitionReferencePosition[3];              // 0x3C
    float heading;                                     // 0x48: radians about the Y axis
    float positionBlend;                               // 0x4C: 1 when the marker sits at `position`
    WVector unk_50;                                    // 0x50: zeroed by OnInit
    WVector initialRandomOffset;                       // 0x5C: random horizontal offset chosen by OnInit
    float targetPosition[3];                           // 0x68: ground point under the aim
    float targetHeightOffset;                          // 0x74
    int groundHitIndex;                                // 0x78: GetGroundPoint output
    float rotationPhase;                               // 0x7C: radians, advances while enabled
    unsigned char enabled;                             // 0x80
    char pad_081[3];                                   // 0x81
    float arrowWidth;                                  // 0x84
    float endpointSize;                                // 0x88
    void* clubRangeInfo;                               // 0x8C: owned CClubRangeInfo
    unsigned int markerColor;                          // 0x90: packed color
    float labelWidthOffset;                            // 0x94
    float labelHeightOffset;                           // 0x98
    void* indicatorBitmap;                             // 0x9C
    void* fallingBitmap;                               // 0xA0
    void* slopeBitmap;                                 // 0xA4
    void* targetHolePositionBitmap;                    // 0xA8
    float pulsePhase;                                  // 0xAC: 0 to 1
    float arrowProgress;                               // 0xB0
    float arrowBlend;                                  // 0xB4: 0 to 1, bounces
    float arrowBlendDirection;                         // 0xB8: +1 or -1
    void* validTargetOverlay;                          // 0xBC
    void* invalidTargetOverlay;                        // 0xC0
    void* arrowOverlay;                                // 0xC4
    void* playerMarkerOverlay;                         // 0xC8
    void* auxiliaryMarker;                             // 0xCC: destroyed with OverlayPlayer_Destroy
    SpotTextureBinding textureBindings[kTextureCount]; // 0xD0
    float markerHeading;                               // 0x238
    void* scoreTargetBitmap;                           // 0x23C
    void* markerScreenLink;                            // 0x240: points at the marker's screen record
    void* ballScreenLink;                              // 0x244: points at the ball's screen record
    void* fourthScreenLink;                            // 0x248: points at the fourth screen record
    void* thirdScreenLink;                             // 0x24C: points at the third screen record
    int screenRecordCount;                             // 0x250
    float markerScreen[2];         // 0x254: projected marker x and y; first of four 0x38-byte records
    char pad_25C[0x30];            // 0x25C
    float ballScreen[2];           // 0x28C: projected ball x and y; the second record
    char pad_294[0x30];            // 0x294
    char thirdScreenRecord[0x38];  // 0x2C4
    char fourthScreenRecord[0x38]; // 0x2FC
    unsigned char targetDirty;     // 0x334: the landing point must be recomputed
    char pad_335[7];               // 0x335
    void* markerVertices;          // 0x33C: heap array, allocated with a 0x10-byte header
    bool playerMarkersVisible;     // 0x340
    char pad_341[3];               // 0x341
    int messageValueAtLoad;        // 0x344

    void DrawScoreTarget();
    static void DrawProjectedClippedQuad(void* obj, void* worldPosition, unsigned int color);
    static void* GetRTTI();
    static void* vfunc_1Thunk18(void* arg);

    ~CSpot();
};

#define CSPOT_OFFSET(field, offset) typedef char CSpot_offset_##field[((int)&((CSpot*)0)->field == (offset)) ? 1 : -1]
CSPOT_OFFSET(taskVtbl, 0x18);
CSPOT_OFFSET(position, 0x24);
CSPOT_OFFSET(previousPosition, 0x30);
CSPOT_OFFSET(transitionReferencePosition, 0x3C);
CSPOT_OFFSET(heading, 0x48);
CSPOT_OFFSET(unk_50, 0x50);
CSPOT_OFFSET(initialRandomOffset, 0x5C);
CSPOT_OFFSET(targetPosition, 0x68);
CSPOT_OFFSET(groundHitIndex, 0x78);
CSPOT_OFFSET(enabled, 0x80);
CSPOT_OFFSET(arrowWidth, 0x84);
CSPOT_OFFSET(clubRangeInfo, 0x8C);
CSPOT_OFFSET(labelHeightOffset, 0x98);
CSPOT_OFFSET(pulsePhase, 0xAC);
CSPOT_OFFSET(validTargetOverlay, 0xBC);
CSPOT_OFFSET(textureBindings, 0xD0);
CSPOT_OFFSET(markerHeading, 0x238);
CSPOT_OFFSET(markerScreen, 0x254);
CSPOT_OFFSET(ballScreen, 0x28C);
CSPOT_OFFSET(thirdScreenRecord, 0x2C4);
CSPOT_OFFSET(fourthScreenRecord, 0x2FC);
CSPOT_OFFSET(targetDirty, 0x334);
CSPOT_OFFSET(markerVertices, 0x33C);
CSPOT_OFFSET(playerMarkersVisible, 0x340);
CSPOT_OFFSET(messageValueAtLoad, 0x344);
#undef CSPOT_OFFSET
typedef char CSpot_size[(sizeof(CSpot) == 0x348) ? 1 : -1];

#endif // CLASSES_CSPOT_H
