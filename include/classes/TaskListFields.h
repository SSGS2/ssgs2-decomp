// TaskListFields: layout view owned by methods in task_manager_and_actors.cpp: hashkey masks key bits with +0x24; lookup indexes +0x2C; CTask_DisplayActorsByPriority tests bit 0 at +0x3A0 before scene setup
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_TASKLISTFIELDS_H
#define CLASSES_TASKLISTFIELDS_H

struct TaskListFields {
    char pad_000[0x24];
    unsigned int hashMask; // 0x24
    char pad_028[0x4];
    void* hashBuckets; // 0x2C
    char pad_030[0x370];
    unsigned int taskFlags; // 0x3A0
    unsigned char skipOtherTaskUpdates; // 0x3A4
    unsigned char namedTaskOnly; // 0x3A5
};

#define OFF(f, o) typedef char TaskListFields_off_##f[((int)&((TaskListFields*)0)->f == (o)) ? 1 : -1]
OFF(hashMask, 0x24);
OFF(hashBuckets, 0x2C);
OFF(taskFlags, 0x3A0);
OFF(skipOtherTaskUpdates, 0x3A4);
OFF(namedTaskOnly, 0x3A5);
#undef OFF
#define AsTaskListFields(p) ((TaskListFields*)(p))

#endif
