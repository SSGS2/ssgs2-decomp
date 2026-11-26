# Getting started

Set up and build as described in the [README](../README.md). The build always produces a `main.dol` identical to
the original; functions that don't match yet are taken from your own copy of the game.

## Pick a function

```sh
python tools/unmatched.py --min-percent 90
```

lists the functions that don't match yet, smallest first, with how close each one is. Small functions that are
already close are the easiest start.

## Compare it

```sh
build/tools/objdiff-cli diff -p . -u <unit> -o - <function>
```

or open the project folder in the objdiff GUI. Change the C++, run `python -m ninja`, compare again.

## A worked example

`FlushVertexBuffer` in `src/engine/gtl/gtl_render_state_and_primitive.cpp` (48 bytes) was 91% as a plain function
on a raw pointer:

```cpp
void gtl_IG3DXPrimitive_FlushVertexBuffer(void* p) {
    void* q = p;
    GraphicsBuffer_Flush(AsGtlPrimitive(q)->vertexBuffer);
}
```

Written as a member of its class, as the original was, it matches exactly:

```cpp
void gtl::IG3DXPrimitive::FlushVertexBuffer() {
    GraphicsBuffer_Flush(vertexBuffer);
}
```

Renaming a function also renames its symbol: update its line in `config/R2PE9B/symbols.txt` to the new name.

## When a file is done

When every function in a file matches, change its `Object(NonMatching, ...)` in `configure.py` to
`Object(Matching, ...)`. The build then checks the whole `main.dol` with your code linked in.
