#ifndef CLASSES_CSTORYCONTE_H
#define CLASSES_CSTORYCONTE_H

#include "game/types.h"

struct StoryCommandMessage;
struct StoryLoadView;
class CStoryConte {
public:
    void* vtbl;                             // 0x0: original vtable pointer
    char pad_004[0x4];                      // 0x4
    unsigned int unk_8;                     // 0x8
    char pad_00C[0xc];                      // 0xc
    void* taskMainVtable;                   // 0x18
    int state;                              // 0x1c
    unsigned int eventIndex;                // 0x20
    signed char eventPathFlags;             // 0x24
    signed char inlineEventPath;            // 0x25
    char pad_026[0x6];                      // 0x26
    void* eventPathData;                    // 0x2c
    void* eventLoader;                      // 0x30
    void* eventScript;                      // 0x34
    void* playerResourceLoader;             // 0x38
    signed char playerResourceNameFlags;    // 0x3c
    unsigned char inlinePlayerResourceName; // 0x3d
    char pad_03E[0x6];                      // 0x3e
    void* playerResourceNameData;           // 0x44
    unsigned char skipFadeUpdateCount;      // 0x48
    char pad_049[0x3];                      // 0x49
    char displayConfiguration;              // 0x4c
    char pad_04D[0x4f];                     // 0x4d
    unsigned int skipCaptionResource;       // 0x9c

    void OnLoad();
    void OnPreLoadInit();
    static void OnInit();
    static void ProcessPlayerEventScripts(StoryLoadView* self, int loadMode);
    void OnProcess();
    static void* GetRTTI();
    void OnDestroy();
    void OnDisplay();
    void HandleMsg(StoryCommandMessage* arg);

    // The task interface at +0x18 forwards its deleting-destructor slot here.
    void* Destroy(int shouldDelete);

    CStoryConte();
    ~CStoryConte();
};

#define CSTORYCONTE_OFF(f, o) typedef char CStoryConte_off_##f[((int)&((CStoryConte*)0)->f == (o)) ? 1 : -1]
CSTORYCONTE_OFF(unk_8, 0x8);
CSTORYCONTE_OFF(taskMainVtable, 0x18);
CSTORYCONTE_OFF(state, 0x1c);
CSTORYCONTE_OFF(eventIndex, 0x20);
CSTORYCONTE_OFF(eventPathFlags, 0x24);
CSTORYCONTE_OFF(inlineEventPath, 0x25);
CSTORYCONTE_OFF(eventPathData, 0x2c);
CSTORYCONTE_OFF(eventLoader, 0x30);
CSTORYCONTE_OFF(eventScript, 0x34);
CSTORYCONTE_OFF(playerResourceLoader, 0x38);
CSTORYCONTE_OFF(playerResourceNameFlags, 0x3c);
CSTORYCONTE_OFF(inlinePlayerResourceName, 0x3d);
CSTORYCONTE_OFF(playerResourceNameData, 0x44);
CSTORYCONTE_OFF(skipFadeUpdateCount, 0x48);
CSTORYCONTE_OFF(displayConfiguration, 0x4c);
CSTORYCONTE_OFF(skipCaptionResource, 0x9c);
#undef CSTORYCONTE_OFF

#endif // CLASSES_CSTORYCONTE_H
