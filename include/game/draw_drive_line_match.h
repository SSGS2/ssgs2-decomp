#ifndef DRAW_DRIVE_LINE_MATCH_H
#define DRAW_DRIVE_LINE_MATCH_H

extern "C" {
extern char s_drawDriveLineSingletonVtable[];
extern char __vt__14CDrawDriveLine[];
extern char __vt__10CGeneTex3D[];
extern void* g_drawDriveLine;
extern void* g_resourceManager;
void BaseObject_ctor(void*);
void* __nw__FUl(unsigned int);
void* ResourceManager_LoadBitmap(void*, void*, int, int, int);
void* BaseObject_dtor(void*, int);
void appFreeUni__FPv(void*);
void ResourceManager_ReleaseId(void*, void*);
void __dl__FPv(void*);
}

class CDrawDriveLine;
template <class T> struct WSingleton;

template <> struct WSingleton<CDrawDriveLine> {
    void* vtable;
    char unk04[12];
    WSingleton() {
        BaseObject_ctor(this);
        vtable = s_drawDriveLineSingletonVtable;
        g_drawDriveLine = this;
    }
    ~WSingleton();
};

struct DriveLineStorage {
    void* data;
    unsigned int count;
    unsigned int unknown08;
    // Descriptive placeholder only; the original member type/name is unknown.
    DriveLineStorage() {}
    ~DriveLineStorage();
};

struct ChildStatus {
    unsigned char state;
};
struct DriveLineChild : ChildStatus {
    virtual void destroy(int);
    virtual void unused1();
    virtual unsigned char invoke(int);
    int unknown08;
};

struct CDrawDriveLine : WSingleton<CDrawDriveLine> {
    DriveLineStorage storage;
    int predictionEnabled;
    DriveLineChild* child;
    void* resource;
    int predictionDirty;
    CDrawDriveLine();
    ~CDrawDriveLine();

    void UpdateShotPrediction();
    void DrawRibbon();
};

#endif
