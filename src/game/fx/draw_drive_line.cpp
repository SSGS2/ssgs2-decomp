// Drive line effect: the on/off marker pattern along the predicted shot line for each course,
// and the CDrawDriveLine object that holds the prediction.

#include <decomp/static_init.h>
#include "game/draw_drive_line_match.h"

static const int kMarkerCount = 18;
static const int kLastThresholdRow = 13;
static const int kPercentRange = 100;
static const int kInactiveMarker = 0;
static const int kLeadInMarker = 1;
static const int kActiveMarker = 2;
// These are child protocol values, not difficulty levels; the meaning of state 4
// is unresolved. Keep invoke's returned byte rather than assuming it accepted 4.
enum {
    kPredictionChildBytes = 12,
    kPredictionRequestedState = 4
};

extern "C" {
extern float kFloat_6_2831855;
extern float s_driveLineFullTurnRadians;
extern signed char s_driveLineMarkerThresholdsByCourseIndex[];
extern char swingTaleStrings[];
extern void* g_drawDriveLine;
extern void* g_resourceManager;
extern char s_drawDriveLineSingletonVtable[];
extern char __vt__14CDrawDriveLine[];
extern char __vt__10CGeneTex3D[];
int rand();
void BaseObject_ctor(void*);
void* __nw__FUl(unsigned int);
void* ResourceManager_LoadBitmap(void*, void*, int, int, int);

// Initialize the drive-line full-turn angle used by the effect.
void DrawDriveLine_InitializeAngleConstant(void) {
    s_driveLineFullTurnRadians = kFloat_6_2831855;
}

// Clear all eighteen generated drive-line markers.
// markers must address kMarkerCount writable integers; storage stays with caller.
void DriveLineMarkers_Clear(int* markers) {
    markers[0] = 0;
    markers[1] = 0;
    markers[2] = 0;
    markers[3] = 0;
    markers[4] = 0;
    markers[5] = 0;
    markers[6] = 0;
    markers[7] = 0;
    markers[8] = 0;
    markers[9] = 0;
    markers[10] = 0;
    markers[11] = 0;
    markers[12] = 0;
    markers[13] = 0;
    markers[14] = 0;
    markers[15] = 0;
    markers[16] = 0;
    markers[17] = 0;
}

#pragma push
#pragma use_lmw_stmw on
// Roll one threshold per marker, then add a lead-in before each newly active marker.
// courseIndex is clamped to [0,13]. Both callers pass SharedDoc_GetCourseId();
// The shared table stores one marker-threshold row per course index.
// Each signed table byte is compared with rand()%100;
// thresholds outside [0,100] therefore mean never/always, without an extra clamp.
// Output codes: 0 inactive, 1 immediately before an active marker, 2 active.
void DriveLineMarkers_Generate(int* markers, int courseIndex) {
    // Phase 1: choose the course row, then generate independent active markers.
    int tableRow;
    if (courseIndex < 0)
        tableRow = 0;
    else if (courseIndex > kLastThresholdRow)
        tableRow = kLastThresholdRow;
    else
        tableRow = courseIndex;

    int* markerCursor = markers;
    int i;
    int randomValue;
    int hundreds;
    int percentile;
    signed char threshold;
    signed char* thresholds = &s_driveLineMarkerThresholdsByCourseIndex[tableRow * kMarkerCount];
    int activeMarker = kActiveMarker;
    int inactiveMarker = kInactiveMarker;
    for (i = 0; i < kMarkerCount; i++) {
        randomValue = rand();
        hundreds = randomValue / kPercentRange;
        percentile = randomValue - hundreds * kPercentRange;
        threshold = thresholds[i];
        if (percentile < threshold) {
            *markerCursor = activeMarker;
        } else {
            *markerCursor = inactiveMarker;
        }
        markerCursor++;
    }

    // Phase 2: mark inactive predecessors; the final marker has no successor.
    int* adjacentMarkers = markers;
    for (int i = 0; i < kMarkerCount - 1; i++) {
        if (adjacentMarkers[0] == kInactiveMarker && adjacentMarkers[1] == kActiveMarker) {
            adjacentMarkers[0] = kLeadInMarker;
        }
        adjacentMarkers++;
    }
}
#pragma pop

} // extern "C"

// Create the prediction child, advance it to state 4, and acquire its bitmap.
// The child is owned by this object; resource is a ResourceManager handle released
// by the destructor. Existing code requires child allocation to succeed: the
// state test dereferences predictionChild even when __nw__FUl returned null.
CDrawDriveLine::CDrawDriveLine() {
    // Phase 1: initialize marker storage and prediction flags.
    vtable = __vt__14CDrawDriveLine;
    storage.data = 0;
    storage.count = 0;
    storage.unknown08 = 0;
    predictionEnabled = 1;
    predictionDirty = 0;
    // Phase 2: construct the child and request its initial protocol state.
    DriveLineChild* predictionChild = (DriveLineChild*)__nw__FUl(kPredictionChildBytes);
    if (predictionChild) {
        predictionChild->state = 0;
        *(void**)((char*)predictionChild + 4) = __vt__10CGeneTex3D;
        predictionChild->unknown08 = 0;
    }
    child = predictionChild;
    if (predictionChild->state != kPredictionRequestedState) {
        unsigned char resultingState = predictionChild->invoke(kPredictionRequestedState);
        predictionChild->state = resultingState;
    }
    // Phase 3: acquire the bitmap used when rendering the drive-line effect.
    resource = ResourceManager_LoadBitmap(g_resourceManager, swingTaleStrings, 0, 0, 0);
}

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804623B0", _ctors_u80277528_0, DrawDriveLine_InitializeAngleConstant);
#pragma pop
