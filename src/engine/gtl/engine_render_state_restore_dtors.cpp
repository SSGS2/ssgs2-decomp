// Translation unit 0x80264238-0x8026456C (7 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

enum SavedStateLayout {
    kSavedRenderStateCount = 21, kSavedRenderWordsOffset = 0xA8,
    kSavedTextureStateCount = 8, kSavedTextureWordsOffset = 0x40,
    kSavedSamplerCount = 5, kSavedSamplerWordsOffset = 0x28
};

void appFreeUni__FPv(void*);
void Engine_SetTextureStageParameter(void*, void*, void*);
void __dl__FPv(void*);
void Engine_SetCachedRenderState(void*, void*);
void Engine_SetTextureStageState(void*, void*, void*);

// Two-word owner view: object is appFreeUni-owned; the meaning of state is unresolved.
// All destructors here accept null and return the original self address. A positive
// deleting flag also releases self with operator delete; other values keep its storage.
struct DtorObject {
    void* object;
    void* state;
};

// Clear the owner's second word, free its payload, and optionally delete the owner.
void* dtor_80264238(DtorObject* self, int deleting) {
    if (self != 0) {
        self->state = 0;
        if (self != 0) {
            if (self->object != 0) {
                self->state = 0;
                appFreeUni__FPv(self->object);
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Apply the same payload cleanup to the second owner specialization.
void* dtor_802642AC(DtorObject* self, int deleting) {
    if (self != 0) {
        self->state = 0;
        if (self != 0) {
            if (self->object != 0) {
                self->state = 0;
                appFreeUni__FPv(self->object);
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// Restore five saved sampler parameters on texture stage zero, then optionally delete self.
void* TextureStageSamplerGuard_dtor(void* self, int deleting) {
    if (self != 0) {
        // Layout: five {parameter ID, replacement value} pairs at +0, saved words at +0x28.
        // IDs/values are raw 32-bit words transported through the existing pointer ABI.
        char* savedWordCursor = (char*)self;
        void** pairCursor = (void**)self;
        unsigned int i = 0;
        for (; i < kSavedSamplerCount; ++i) {
            Engine_SetTextureStageParameter(0, pairCursor[0], *(void**)(savedWordCursor + kSavedSamplerWordsOffset));
            savedWordCursor += 4;
            pairCursor += 2;
        }
        // The caller's sampler state is restored before the guard storage can be freed.
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma pop

// Apply payload cleanup to the third owner specialization, optionally deleting self.
void* dtor_80264320(DtorObject* self, int deleting) {
    if (self != 0) {
        self->state = 0;
        if (self != 0) {
            if (self->object != 0) {
                self->state = 0;
                appFreeUni__FPv(self->object);
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Apply payload cleanup to the fourth owner specialization, optionally deleting self.
void* dtor_80264394(DtorObject* self, int deleting) {
    if (self != 0) {
        self->state = 0;
        if (self != 0) {
            if (self->object != 0) {
                self->state = 0;
                appFreeUni__FPv(self->object);
            }
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// Restore 21 saved render states, then optionally delete the scope guard.
void* RenderStateScope_dtor(void* self, int deleting) {
    if (self != 0) {
        // Layout: 21 {state ID, replacement value} pairs, followed by saved words at +0xA8.
        char* savedWordCursor = (char*)self;
        void** pairCursor = (void**)self;
        unsigned int i = 0;
        for (; i < kSavedRenderStateCount; ++i) {
            Engine_SetCachedRenderState(pairCursor[0], *(void**)(savedWordCursor + kSavedRenderWordsOffset));
            savedWordCursor += 4;
            pairCursor += 2;
        }
        // Restore all state before releasing the guard; the pair table itself owns no resources.
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Restore eight saved texture-stage states on stage zero, then optionally delete self.
void* TextureStageStateRestorer_dtor(void* self, int deleting) {
    if (self != 0) {
        // Layout: eight {state ID, replacement value} pairs, saved words at +0x40.
        char* savedWordCursor = (char*)self;
        void** pairCursor = (void**)self;
        unsigned int i = 0;
        for (; i < kSavedTextureStateCount; ++i) {
            Engine_SetTextureStageState(0, pairCursor[0], *(void**)(savedWordCursor + kSavedTextureWordsOffset));
            savedWordCursor += 4;
            pairCursor += 2;
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma pop

} // extern "C"
