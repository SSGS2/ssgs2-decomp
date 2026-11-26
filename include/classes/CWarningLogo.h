// CWarningLogo: the startup warning and publisher-logo task.
//
// It starts the warning and logo animation entries through the BP controller,
// steps them through the states in warning_logo_screens.cpp, and then plays the
// logo movie. The task finishes when the movie ends or a skip input ends it.
#ifndef CLASSES_CWARNINGLOGO_H
#define CLASSES_CWARNINGLOGO_H

class CWarningLogo {
public:
    CWarningLogo();

    // Advance the state machine by one update; returns 1 once the task has finished.
    int UpdateSequence();
    void StartWarningSequence();
    void Draw();
    // Deletes the animation entry; frees the task too when shouldDelete is positive.
    void* Destroy(int shouldDelete);
    int IsFinished();
    static int PollAnimationEntriesLoaded();
    void vfunc_7();

    void* vtbl;              // 0x00
    char pad_04[0x14];       // 0x04
    int scriptId;            // 0x18  BP entry ID, -1 until requested
    int state;               // 0x1C
    int timelineUpdateCount; // 0x20
    int delayCounter;        // 0x24  counts the Nintendo hold
    int finished;            // 0x28
    char pad_2C[0x4];        // 0x2C
    int configuration[4];    // 0x30  copied from the initial configuration words
    char pad_40[0x3C];       // 0x40  output configuration, written by the constructor
};

#define CWARNINGLOGO_OFF(f, o) typedef char CWarningLogo_off_##f[((int)&((CWarningLogo*)0)->f == (o)) ? 1 : -1]
CWARNINGLOGO_OFF(vtbl, 0x00);
CWARNINGLOGO_OFF(scriptId, 0x18);
CWARNINGLOGO_OFF(state, 0x1C);
CWARNINGLOGO_OFF(timelineUpdateCount, 0x20);
CWARNINGLOGO_OFF(delayCounter, 0x24);
CWARNINGLOGO_OFF(finished, 0x28);
CWARNINGLOGO_OFF(configuration, 0x30);
#undef CWARNINGLOGO_OFF
typedef char CWarningLogo_size[(sizeof(CWarningLogo) == 0x7C) ? 1 : -1];

#endif // CLASSES_CWARNINGLOGO_H
