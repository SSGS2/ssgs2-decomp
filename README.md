Super Swing Golf Season 2 decompilation
=======================================

A matching decompilation of *Super Swing Golf Season 2* (Wii, USA, `R2PE9B`, disc revision 0), built on
[decomp-toolkit](https://github.com/encounter/decomp-toolkit) and
[dtk-template](https://github.com/encounter/dtk-template), with [objdiff](https://github.com/encounter/objdiff)
for diffing.

The original `main.dol` rebuilds byte-for-byte. Units marked `Matching` in `configure.py` are compiled from
the C++ in `src/`; the rest are linked from the original code until they match.

No game data and no compiler binaries are in this repository. You need your own copy of the game.

AI disclosure: AI tools were used in making this project. Every match is checked against the original game code,
but names and comments may contain mistakes.

Status
------

The game's own code is 10,001 functions:

* 7,285 are **matched**: the C++ in `src/` compiles to exactly the original instructions.
* 2,716 are written in C++ but don't compile to exactly the original yet.

The other 2,909 functions are Nintendo SDK, Metrowerks runtime and other library code, which isn't decompiled here.

Anything that isn't matched is taken from the original game code (from your own `main.dol`) when you build, so the
finished `main.dol` is always identical to the retail one. Run `python -m ninja` to see the current numbers.

Setup
-----

Needs Python 3 and git; ninja comes from pip (`python -m pip install ninja`).

Put the game's `main.dol` at `orig/R2PE9B/sys/main.dol` (git-ignored). decomp-toolkit can extract it from a disc
image (ISO, WBFS, RVZ):

```sh
python configure.py
python -m ninja build/tools/dtk.exe
build/tools/dtk vfs cp "<disc image>:sys/main.dol" orig/R2PE9B/sys/main.dol
```

Expected SHA-1 of `orig/R2PE9B/sys/main.dol`: `d89a0066a52e8140f11e0907879441a5836e00d5`.

The first `configure.py`/`ninja` run downloads decomp-toolkit, objdiff-cli, sjiswrap and the community
Metrowerks compilers archive into `build/`.

Build and verify
----------------

```sh
python configure.py
python -m ninja
```

`ninja` splits the DOL, compiles `src/`, links `build/R2PE9B/main.dol` and checks its SHA-1 against
`config/R2PE9B/build.sha1`. Success prints `build/R2PE9B/main.dol: OK`; a mismatch fails the build.

Matching a function
-------------------

To find a target, `python tools/unmatched.py` lists unmatched functions, smallest first;
[docs/getting_started.md](docs/getting_started.md) has a worked example.

1. A source file needs a split in `config/R2PE9B/splits.txt` covering its whole translation unit and an
   `Object(...)` entry in `configure.py`.
2. Write the function and build: `python -m ninja`.
3. Compare it in the objdiff GUI (open the project folder) or with
   `build/tools/objdiff-cli diff -p . -u main/<unit> -o - <function>`.
4. When every function in a unit matches, change `Object(NonMatching, ...)` to `Object(Matching, ...)`; the
   SHA-1 check then proves the whole DOL with the compiled object linked in.

Compiler: CodeWarrior GC/3.0a5.2 for the game code (`-O4,p`, C++ exceptions on). More in `docs/`.

Layout
------

| Path | Contents |
|---|---|
| `src/` | decompiled C++ (`game/`, `engine/gtl/`, `sdk/`, `platform/`) |
| `include/` | headers: game classes, SDK (`revolution/`, `rvl/`), MSL |
| `config/R2PE9B/` | decomp-toolkit config: splits, symbols, expected SHA-1 |
| `configure.py`, `tools/` | the dtk-template build system |
| `docs/` | dtk-template reference docs and game file formats |

Reading the code
----------------

* Every source file starts with a short description of what it holds. `src/game/` is the game, by area (`golf/`,
  `ui/`, `modes/`, `chara/`, `fx/`, `audio/`, `save/`, `system/`); `src/engine/gtl/` is the engine layer under it.
* Class layouts are in `include/classes/`, one header per class.
* Names come from the game's own text, its type names, or what the code plainly does. Without that evidence a name
  stays a placeholder: `unk_<offset>` for a field, `vfunc_<n>` for a virtual function slot, `arg<n>` for a
  parameter, a trailing address (`Foo8040C2C8`) for an unidentified type. A placeholder means "not known yet".
* Unmatched functions are readable C++, but not proven identical to the original.

Contributing
------------

This project isn't actively maintained. Pull requests that add new matches are welcome and may be merged when time
allows, but there's no promise of review. Forks and ports are encouraged.

Credits
-------

Tools:

* [decomp-toolkit](https://github.com/encounter/decomp-toolkit), [objdiff](https://github.com/encounter/objdiff)
  (Apache-2.0) and [dtk-template](https://github.com/encounter/dtk-template) (CC0), on which the build system is based.
* The CodeWarrior compilers (from the decomp.dev community archive) and, on Linux,
  [wibo](https://github.com/decompals/wibo) are downloaded at build time; they are not part of this repository.

Code copied into this repository:

* RVL SDK headers in `include/revolution/`, `include/MSL/` and `include/decomp/` from the Wii Sports decompilation,
  [doldecomp/ogws](https://github.com/doldecomp/ogws) (CC0). See `include/rvl/README.md`.

Used as references only (names, layouts, file formats, cross-checks; no code copied):

* [hsreina/YaPang](https://github.com/hsreina/YaPang) (CC0), a decompilation of the PangYa PC client: class,
  field and physics names.
* [pangbox/rebang](https://github.com/pangbox/rebang), a decompilation of the PangYa PC client: a few class layouts
  and method names.
* [retreev/pyssg-decomp](https://codeberg.org/retreev/pyssg-decomp) (CC0), the first Super Swing Golf: cross-check of
  SDK names.
* [doldecomp/forecast](https://github.com/doldecomp/forecast) and [doldecomp/mkw](https://github.com/doldecomp/mkw)
  (CC0): cross-check of SDK names.
* [retreev/PangLib](https://github.com/retreev/PangLib), [retreev/pangya.ksy](https://github.com/retreev/pangya.ksy),
  [retreev/Documentation](https://github.com/retreev/Documentation), [pangbox/pangfiles](https://github.com/pangbox/pangfiles)
  and [Acrisio-Filho/SuperSS-Dev](https://github.com/Acrisio-Filho/SuperSS-Dev): PangYa file formats and shot
  math, used for `docs/iff-formats.md` and comments.
* Hardware documentation from [Dolphin](https://github.com/dolphin-emu/dolphin),
  [YAGCD](https://www.gc-forever.com/yagcd/) and [WiiBrew](https://wiibrew.org/).

Thanks to everyone behind these projects.

License
-------

CC0 1.0 (see `LICENSE`). This covers the code and documentation written for this project; it gives no rights to the
game itself. Super Swing Golf Season 2 and PangYa are trademarks of their respective owners. This project is not
affiliated with them.
