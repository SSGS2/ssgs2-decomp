// CGolfBg: the background actor that loads a course's model and resource lists.
//
// Only the parts the background code and its reload helpers reach are named: the
// background command handler, the reload bookkeeping, the owned string list, and
// the two builders that turn the shared course selection into archive paths.
#ifndef CLASSES_CGOLFBG_H
#define CLASSES_CGOLFBG_H

// A path string: small text is stored inside the record, longer text on the
// heap when the top bit of flags is set.
struct PathString {
    unsigned int flags;  // 0x0
    unsigned int length; // 0x4
    char* data;          // 0x8
};

class CGolfBg {
public:
    // Owned list of strings, freed by the destructor.
    struct StringList {
        void* data;   // 0x0
        int count;    // 0x4
        int capacity; // 0x8
    };

    static void OnPreLoadInit();

    static void OnInit();
    void OnLoad();
    void HandleMsg(const void* message);

    static void* GetRTTI();

    // Build the weather resource list path and the course rank list path from
    // the shared course selection. Both return 0 when the weather variant has
    // no resource suffix.
    int BuildWeatherResourcePath(PathString* path);
    int BuildCourseRankListPath(PathString* path);

    ~CGolfBg();

    void GroundAndDispatchAmbientPosition(void* input);
    int FinalizeCourseReload();
    void RequestCourseReload(int mode);

    void ReloadCurrentHoleResources();
    void OnDestroy();
    void InitializeCurrentHole();

    void* vtbl;              // 0x00: original vtable pointer
    char pad_004[0x14];      // 0x04
    int courseReloadSettled; // 0x18: Nonzero means no started reload awaits finalization; zero while a new load is pending.
    unsigned char savedHoleIndex; // 0x1C: RequestCourseReload saves currentHoleIndex; FinalizeCourseReload restores it.
    unsigned char savedHoleCount; // 0x1D: same helpers save/restore roundHoleCount; zero means no saved round.
    unsigned char savedHoleOrder[18]; // 0x1E: copied to/from shared document +0x82A in three groups of six.
    char pad_030[4];
    StringList strings; // 0x34
};
#define OFF(f, o) typedef char CGolfBg_off_##f[((int)&((CGolfBg*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(courseReloadSettled, 0x18);
OFF(savedHoleIndex, 0x1C);
OFF(savedHoleCount, 0x1D);
OFF(savedHoleOrder, 0x1E);
OFF(strings, 0x34);
#undef OFF
typedef char CGolfBg_size[(sizeof(CGolfBg) == 0x40) ? 1 : -1];
typedef char PathString_size[(sizeof(PathString) == 0xC) ? 1 : -1];
#endif
