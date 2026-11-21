// ProjectOwnedListNode: layout view; methods in project_base_and_grid_surface.cpp (CProjectG_dtor unlinks previous/next before freeing node).
// No virtual methods, inheritance, or emitted runtime objects.
#ifndef CLASSES_PROJECTLISTNODE801C9600_H
#define CLASSES_PROJECTLISTNODE801C9600_H

class ProjectOwnedListNode {
public:
    char pad_000[0xC];
    void* previous; // 0xC
    void* next; // 0x10
};

#define OFF(f, o) typedef char ProjectListNode801C9600_off_##f[((int)&((ProjectOwnedListNode*)0)->f == (o)) ? 1 : -1]
OFF(previous, 0xC);
OFF(next, 0x10);
#undef OFF

#define AsProjectListNode801C9600(p) ((ProjectOwnedListNode*)(p))

#endif
