// gtl::nw4r::FxReverbHiDpl2: the high-quality reverb effect object with Dolby Pro Logic II output.
// Its vtable is installed explicitly by the constructor and destructor; no virtual functions are declared here.
#ifndef CLASSES_FXREVERBHIDPL2_H
#define CLASSES_FXREVERBHIDPL2_H

namespace gtl {
namespace nw4r {

class FxReverbHiDpl2 {
public:
    // Finish and destroy the effect, then free it with operator delete.
    ~FxReverbHiDpl2();

    void* vtbl; // 0x0
};

} // namespace nw4r
} // namespace gtl

#endif // CLASSES_FXREVERBHIDPL2_H
