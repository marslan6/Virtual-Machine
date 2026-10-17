# C++ Structure & Formatting Conventions

House style for file organization, member ordering, and comments.
Reference files that follow these rules:
`include/Lc3Types.h`, `include/CPU.h`, `src/CPU.cpp`.

## Class layout (order)
- Access sections in order: `public:` → `protected:` (if used) → `private:`.
  Interface before implementation.
- Within each access section, order members as:
  1. Constructors, then destructor
  2. Static member functions
  3. Non-static member functions
  4. Data members (always last)
- Keep `static` and non-static members grouped, not interleaved.
- Group members into labelled sections with banner comments:
  `// ===== Section name =====`
  Typical sections: Construction, Run state, Program execution,
  Static helpers, Register access, State.

## Source (.cpp) layout
- Definition order mirrors the header's declaration order.
- Use the same `// ===== Section name =====` banners as the header, so the two
  files read in parallel.

## Comment format
- **Headers (class members)**: NO doc comments. Only `// ===== Section =====`
  separators to show structure. Declarations stay clean.
- **Source (.cpp)**: full Doxygen-style docs above each definition:
  - `///` one-line summary
  - `@param <name>` for each parameter
  - `@return` for the return value
  - `@throws` for anything that can throw
- Inline comments explain *why*, not *what*.

## LC-3 types header (`Lc3Types.h`)
- This header holds shared machine vocabulary used across classes:
  type aliases (`Word`, `Address`), the register and error enums
  (`Register`, `ImageError`), the condition-flag constants (`Flag`), and the
  grouped constant namespaces (`Lc3Cpu`, `Lc3CpuImage`).
- Free namespace-scope types and constants defined HERE may carry `///` doc
  comments (unlike class members in other headers), because this is their
  definition site.
- Group related constants in a named namespace scoped by concern
  (e.g. machine constants vs. image/file defaults), each with a `///` summary.
- Order within the header: aliases → constant namespaces → enums → flag
  namespace.

## File organization
- Shared, cross-class declarations live in `Lc3Types.h`.
- Class-specific declarations live in that class's own header.
- Promote a type from a class header to `Lc3Types.h` only once more than one
  class needs it.

## Includes
- Include every header a translation unit uses directly; don't rely on
  transitive includes.
- Match the include to the type actually used.
- Remove unused and platform-dead includes.