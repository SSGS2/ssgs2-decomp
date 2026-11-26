# Real RVL SDK headers

Credit: these headers are from the **Wii Sports decompilation, [doldecomp/ogws](https://github.com/doldecomp/ogws)**
(commit `e5a4d369011062fe243aa63f6c2e2853854c395d`, 2026-10-25), licensed **CC0 1.0 Universal** (public domain
dedication; its `LICENSE` covers the whole repository, and none of the copied files carries a different notice).
Thanks to the doldecomp/ogws authors. Their SDK is the same build as this game's (OS and DVD Apr 24 2007, AX
Dec 18 2006, GX/VI/PAD/NAND Nov 30 2006) and compiled with the same CodeWarrior (GC/3.0a5.2). The Forecast Channel
decompilation ([doldecomp/forecast](https://github.com/doldecomp/forecast), CC0 1.0) and Mario Kart Wii
([doldecomp/mkw](https://github.com/doldecomp/mkw), CC0 1.0) were read only, as a second opinion when
identifying SDK function names; nothing was copied from them.

Only headers are copied: no implementation (`src/`) code, no game data.

## Where the files are

The headers keep ogws's layout and `#include <...>` paths, so they work with the build's existing `-i include`
(no build change). That puts them at the top of `include/`, not under this folder:

| Path | Contents |
|---|---|
| `include/revolution/` | the headers of AI, AX, DVD, FS, GX, IPC, MTX, NAND, OS, PAD, VI and their umbrella headers |
| `include/decomp/`, `include/macros.h` | ogws's helper macros the SDK headers use (`DECL_WEAK`, `DECL_SECTION`, `ALIGN`, ...) |
| `include/types.h` | **adapted** from ogws (below) |
| `include/MSL/` | ogws's MSL C headers (`string.h`, `stdio.h`, `stdlib.h`, `math.h`, `wchar.h`, ...). Reference only: they need `-i include/MSL`, which the build does not pass, so nothing includes them yet |
| `include/sdk_protos.h` | maintained by hand; includes the real headers, then declares each SDK function under its symbol name |

## Deviations from the originals (kept minimal)

* `include/types.h`: `u32`/`s32` are `unsigned int`/`signed int` (ogws uses `long`); this project's sources use the
  int spelling and the two are different types for overloading. `size_t` is declared there, and ogws's
  `<cstdarg>/<cstddef>/<new>` includes are dropped (no MSL include path).
* `static void f() {...}` became `static inline void f() {...}` in `revolution/GX/GXAttr.h` (`GXSetTexCoordGen`),
  `GXDisplayList.h` (`GXFastCallDisplayList`), `GXGeometry.h` (`GXEnd`), `GXTransform.h` (`GXSetViewportv`) and
  `OS/OSFastCast.h` (all functions): a plain `static` function with a body is emitted into every object that
  includes the header, which would add code to that object. Unreferenced `static inline` is dropped.
* Not copied (not CC0 / not needed): `revolution/BTE/*` (Broadcom Bluetooth stack headers with their own
  copyright notices) and therefore everything that includes it: `SC`, `WUD`, `WPAD`, `KPAD`; `MSL/internal/fdlibm.h`
  (carries a Sun copyright notice). The KPAD/WPAD prototypes used for name identification were read in place, not
  copied.

## Using them

`#include "sdk_protos.h"` is the entry point for source files. It reads these headers with every enum typedef renamed,
then gives each enum name back as `typedef int X;` (the project models enums as int, and the existing sources pass
ints), and declares each function under the symbol name the build uses (now the SDK's own name, such as `DVDOpen`) with the
real parameter types.
