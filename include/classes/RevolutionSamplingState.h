// RevolutionSamplingState: next output byte request, seeded by reset and replaced by each completed output header
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_REVOLUTIONSAMPLINGSTATE_H
#define CLASSES_REVOLUTIONSAMPLINGSTATE_H

struct RevolutionSamplingState {
    char pad_000[0x38];
    unsigned int requestedOutputBytes; // 0x38
};

#define OFF(f, o) typedef char RevolutionSamplingState_off_##f[((int)&((RevolutionSamplingState*)0)->f == (o)) ? 1 : -1]
OFF(requestedOutputBytes, 0x38);
#undef OFF
#define AsRevolutionSamplingState(p) ((RevolutionSamplingState*)(p))

#endif
