# Linting with cpplint

This project uses [cpplint](https://github.com/cpplint/cpplint) to check C++ code
against the Google C++ style guide.

## Common commands

Run all commands from the project root. The linter script lives in `docs/`, and
the code is split across `./src` (sources) and `./include` (headers), so both
paths are passed.

Check the whole project:

```bash
python3 docs/cpplint.py --recursive ./src ./include
```

Check specific files:

```bash
python3 docs/cpplint.py ./include/CPU.h ./src/CPU.cpp
```

Show a summary of errors per category:

```bash
python3 docs/cpplint.py --recursive --counting=toplevel ./src ./include
```

Print nothing when there are no errors (useful in scripts and CI):

```bash
python3 docs/cpplint.py --recursive --quiet ./src ./include
```

List all available check categories:

```bash
python3 docs/cpplint.py --filter=
```

## Silencing warnings

Disable a category for the whole project by adding it to the `filter` line in
`CPPLINT.cfg`, for example `filter=-legal/copyright,-build/include_order`.

Silence a single line:

```cpp
int x = 0;  // NOLINT(category)
```

Silence the next line:

```cpp
// NOLINTNEXTLINE(category)
int x = 0;
```

Silence a block:

```cpp
// NOLINTBEGIN(category)
...
// NOLINTEND
```

Only silence warnings that are false positives or deliberate style choices, and
prefer naming the specific category over a bare `NOLINT`.