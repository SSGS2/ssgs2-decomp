// GolfPlayerPendingNode. Pending per-player text tree nodes in golf_player_animation_and_model_loads.cpp.
// Explicit layout view without inheritance or virtual methods.
#ifndef CLASSES_GOLFPLAYERPENDINGNODE_H
#define CLASSES_GOLFPLAYERPENDINGNODE_H

class GolfPlayerPendingNode {
public:
    void*
        left; // 0x0: CGolfPlayer_PollPendingModelLoads, CGolfPlayer_RebuildPendingModelLoads tree traversal and string cleanup
    void*
        right; // 0x4: CGolfPlayer_PollPendingModelLoads, CGolfPlayer_RebuildPendingModelLoads tree traversal and string cleanup
    unsigned int
        parentFlags; // 0x8: CGolfPlayer_PollPendingModelLoads, CGolfPlayer_RebuildPendingModelLoads tree traversal and string cleanup
    unsigned int
        playerIndex; // 0xC: CGolfPlayer_PollPendingModelLoads, CGolfPlayer_RebuildPendingModelLoads tree traversal and string cleanup
    unsigned int
        stringFlags; // 0x10: CGolfPlayer_PollPendingModelLoads, CGolfPlayer_RebuildPendingModelLoads tree traversal and string cleanup
    unsigned int
        stringLength; // 0x14: CGolfPlayer_PollPendingModelLoads, CGolfPlayer_RebuildPendingModelLoads tree traversal and string cleanup
    char*
        stringData; // 0x18: CGolfPlayer_PollPendingModelLoads, CGolfPlayer_RebuildPendingModelLoads tree traversal and string cleanup
};

#define OFF(f, o) typedef char GolfPlayerPendingNode_off_##f[((int)&((GolfPlayerPendingNode*)0)->f == (o)) ? 1 : -1]
OFF(left, 0x0);
OFF(right, 0x4);
OFF(parentFlags, 0x8);
OFF(playerIndex, 0xC);
OFF(stringFlags, 0x10);
OFF(stringLength, 0x14);
OFF(stringData, 0x18);
#undef OFF
#define AsGolfPlayerPendingNode(p) ((GolfPlayerPendingNode*)(p))

#endif
