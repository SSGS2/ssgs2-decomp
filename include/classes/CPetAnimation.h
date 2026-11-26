// The object is reference counted (the first word) and owns a table of named animation entries.
#ifndef CLASSES_CPETANIMATION_H
#define CLASSES_CPETANIMATION_H

struct Entry;
struct PetAnimationChunk;
struct PetMaterialMatrix;

class CPetAnimation {
public:
    // Restores the class vtable and releases the entries.
    ~CPetAnimation();

    // Builds the entry table from a chunk and returns the summed duration of the newly loaded resources.
    float LoadEntriesFromChunk(PetAnimationChunk* chunk);
    void Clear();

    void* GetSelectedResource();
    int GetEntryCount(); // int: callers compare it as a signed value
    int SetModeAndAmount(unsigned short mode, float amount);
    float GetAmount();

    int GetValueByName(float* out, const char* key);
    int EvaluateNamedTransforms(int evaluationArg0, int evaluationArg1, float amount, const char* key);
    int EvaluateNamedMaterialMatrix(PetMaterialMatrix* out, int evaluationArg, float amount, const char* key,
                                    unsigned matrixIndex);

    int GetEntryNameSize(unsigned short entryIndex);
    int CopyEntryName(char* destination, unsigned short entryIndex, unsigned size);

    int refs;                  // 0x0: shared reference count
    void* vtbl;                // 0x4: original vtable pointer
    Entry* entries;            // 0x8: array of entryCount entries
    unsigned short entryCount; // 0xC
    char pad_00E[2];           // 0xE
    float amount;              // 0x10: evaluation amount set together with mode
    unsigned short mode;       // 0x14: selects the entry whose mode matches
    char pad_016[2];           // 0x16

    CPetAnimation();
};

#define CPETANIMATION_OFF(f, o) typedef char CPetAnimation_off_##f[((int)&((CPetAnimation*)0)->f == (o)) ? 1 : -1]
CPETANIMATION_OFF(vtbl, 0x4);
CPETANIMATION_OFF(entries, 0x8);
CPETANIMATION_OFF(entryCount, 0xC);
CPETANIMATION_OFF(amount, 0x10);
CPETANIMATION_OFF(mode, 0x14);
#undef CPETANIMATION_OFF
typedef char CPetAnimation_size_check[(sizeof(CPetAnimation) == 0x18) ? 1 : -1];

#endif // CLASSES_CPETANIMATION_H
