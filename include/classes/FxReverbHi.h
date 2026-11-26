// gtl::nw4r::FxReverbHi: the high-quality reverb effect object. Its vtable is installed
// explicitly by the constructor and destructor; no virtual functions are declared here.
#ifndef CLASSES_FXREVERBHI_H
#define CLASSES_FXREVERBHI_H

namespace gtl {
namespace nw4r {

class FxReverbHi {
public:
    // Deinitialize and destroy the effect, then free it with operator delete.
    ~FxReverbHi();

    void* vtbl; // 0x0
};

} // namespace nw4r
} // namespace gtl

#endif // CLASSES_FXREVERBHI_H
