# Nexus

Nexus is a small interpreted scripting language implemented in C++.

This README tracks project progress and documents what currently works.

## Current Status

### ✅ Done
- CLI entrypoint that reads source from a file path or stdin.
- Lexer with:
  - identifiers, numbers, strings
  - operators: `+ - * / > < = ==`
  - punctuation: `() { } ;`
  - keywords: `let const out if else repeat times`
  - comments: `// line` and `/* block */`
  - line/column diagnostics with source name
- Parser + AST with:
  - declarations: `let`, `const`
  - output statement: `out(...)`
  - control flow: `if / else if / else`
  - loops: `repeat <ident> <expr> times { ... }`
  - expression precedence: equality, comparison, add/sub, mul/div, grouping
- Runtime with:
  - integer arithmetic and comparisons
  - variable declarations and reassignment (`x = ...;`)
  - block-based lexical scoping for `{ ... }`, `if/else`, and `repeat` bodies
  - nested loop/conditional scope isolation (inner variables do not overwrite outer scope bindings)
  - runtime diagnostics (with caret and location)

### 🚧 In Progress / Not Implemented Yet
- Functions and function calls
- Modules/imports
- Rich types (arrays, maps/objects, booleans as first-class type)
- Standard library (`fs`, `http`, `json`, `process`, etc.)
- Bytecode VM / JIT (performance-focused execution backend)
- Automated tests in CTest

### 🎯 Next Milestones
1. Add functions + call stack.
2. Add core stdlib (IO, JSON, HTTP, process).
3. Move from AST-walk execution toward bytecode for speed.
4. Add test suite + benchmarks.

---

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j"$(nproc)"
```

Run from file:

```bash
./build/nexus examples.nxt
```

Or pipe from stdin:

```bash
echo 'out("hello");' | ./build/nexus
```

Source files are required to use the `.nxt` extension when passed as a file argument.

---

## Language Examples

## 1) Variables and output

```nxt
let x = 10;
const y = 3;
out(x + y);
```

Expected output:

```text
13
```

## 2) Looping

```nxt
repeat i 5 times {
  out(i);
}
```

Expected output:

```text
0
1
2
3
4
```


## 3) Reassignment and scoped blocks

```nxt
let x = 1;
{
  let x = 10;
  x = x + 5;
  out(x);
}
out(x);
```

Expected output:

```text
15
1
```

## 4) Nested loop scope isolation

```nxt
let i = 99;
repeat i 2 times {
  out(i);
  {
    let i = 42;
    out(i);
  }
}
out(i);
```

Expected output:

```text
0
42
1
42
99
```

## 5) Conditionals

```nxt
let n = 7;
if n > 10 {
  out("big");
} else if n == 7 {
  out("lucky");
} else {
  out("small");
}
```

Expected output:

```text
lucky
```

## 6) Comments and grouping

```nxt
// line comment
/* block
   comment */
out((2 + 3) * 4);
```

Expected output:

```text
20
```

---

## Docker

Build image:

```bash
docker build -t nexus:local .
```

Run with stdin:

```bash
echo 'out("from container");' | docker run --rm -i nexus:local
```

---

## CI/CD Notes

The Docker publish workflow builds the project via CMake inside the image build stage.
If C++ compilation fails, the Docker build step will fail as expected.

