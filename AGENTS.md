# AGENTS.md — TinyGSM Arduino Project Guidelines

## Project Overview

This project is an **Arduino-based** library that converts AT-command based modems into an Arduino Client interface.

The goal is to produce reliable embedded C++ code for Arduino while minimizing RAM, flash, and CPU usage.

The library follows an object-oriented design and targets Arduino-compatible platforms using C++11.

---

# General Agent Instructions

## Core Principles

- Make the **smallest change necessary** to accomplish the requested task.
- Prefer the fewest files and functions touched while still delivering a complete and correct solution.
- Keep changes tightly scoped.
- Favor simple, maintainable implementations.
- Do not refactor unrelated code.
- Do not rename unrelated symbols, reorganize files, or perform cleanup work unless explicitly requested.
- Preserve existing behavior unless the requested task requires changing it.
- Preserve existing comments and debugging output.
- Never revert unrelated local changes.

## Environment

- The local environment is Windows.
- Use PowerShell 7:
  - `C:\Program Files\PowerShell\7\pwsh.exe`
- Use Windows PowerShell commands rather than Linux shell commands.

## Finding-Driven Workflow

When working from a reported finding, review comment, audit result, or bug report:

1. Verify the finding against current code before editing.
2. Fix only findings that remain valid.
3. Skip obsolete findings and report why.
4. Keep fixes tightly scoped.
5. Validate after editing.

For normal feature requests, implement the requested change without requiring a finding-verification step.

## API Stability

- Before changing any public API, search the repository for usages.
- Update affected examples and tests when necessary.
- Avoid breaking API changes unless explicitly requested.
- Preserve backward compatibility whenever practical.

## Includes

- Always include required headers.
- Do not rely on transitive includes.

## Compatibility Requirements

- C++11 only.
- No STL.
- No RTTI.
- No exceptions unless explicitly required and platform-supported.

## TinyGSM Conventions

- Prefer existing TinyGSM patterns over new abstractions.
- Reuse existing traits, helpers, and configuration values.
- Review equivalent modem implementations for parity.
- Only modify additional modem files when required for correctness or consistency.
- For PROGMEM strings, use existing flash-safe helper patterns (`GF()`, `GFP()`, etc.).

## Instruction Priority

1. User request.
2. Correctness and platform compatibility.
3. Requirements in this document.
4. Existing implementation conventions.

---

# Workspace Structure

## Source Code

- `src/` contains library source.
- `src/TinyGsmClient.h` is the primary user include.
- `src/ArduinoCompat/` contains Arduino compatibility classes.
- `src/TinyGsmCommon.h` contains common helpers.
- `src/TinyGsmCapabilities.h` contains capability traits.
- `src/mixin/` contains CRTP mixins.
- `src/modems/` contains modem-specific implementations.

## Examples

- Examples are located in `examples/`.
- Each example has its own folder.
- Folder name and `.ino` filename must match.
- Each example folder should contain a `ReadMe.md`.

## Testing Helpers

- Testing helpers are located in `extras/`.
- `extras/tools/test_build/test_build.ino` is the primary compile-test sketch.
- New public APIs should be added to the test sketch.

## Restricted Areas

### lib/

- Do not modify files in `lib/` unless explicitly requested.

### archive/

- Do not modify archived files.
- Do not use archived code as a reference.
- Do not review archived code.

## Planning Documents

- Active planning documents are in `planning/`.
- Completed plans are in `planning/finished`.
- Shelved plans are in `planning/shelved`.
- Update planning documents only when the completed work changes an active plan or design decision.

## Modem Documentation

AT command documentation is stored in:

`CellularAndWifiModules\AT Command Manuals`

---

# Coding Standards

## Language

- Arduino IDE 2.x compatible.
- C++11.
- No STL.
- No RTTI.
- Avoid dynamic allocation when practical.
- Suitable for constrained embedded targets.

## Object-Oriented Design

- Follow the library's existing architecture.
- Keep implementation details out of public APIs when practical.
- Prefer simple user-facing interfaces.

## Datasheet Compliance

- AT commands should follow vendor documentation whenever practical.
- Complex commands should have explanatory comments.
- Intentional deviations from documentation must be documented.

## Documentation

- Public classes, functions, macros, and parameters require Doxygen comments.
- Use Java-style `@commands`.
- Documentation belongs in header files.
- Override functions do not need duplicate documentation unless behavior differs.
- Source folders should contain `ReadMe.md` files.

## Formatting

- C++ formatting follows `.clang-format`.
- Preserve local formatting style during edits.
- If formatting changes are necessary, use `.clang-format`.
- Do not reformat unrelated code.
- Python uses Black.
- Markdown uses `.markdownlint-cli2.yaml`.

## Memory & Performance

- Be conscious of RAM, flash, and CPU usage.
- Use project configuration mechanisms.
- Prefer `GF()` where consistent with project conventions.

---

# Repository Safety

- Do not modify dependencies in `lib/` unless requested.
- Never commit secrets.
- Never copy secrets into examples or documentation.
- Do not commit generated artifacts unless required.

---

# Before Finishing a Task

For substantial changes:

1. Verify requested behavior.
2. Verify includes and declarations.
3. Verify API compatibility.
4. Update relevant examples and tests.
5. Add required Doxygen documentation.
6. Ensure unrelated files were not modified.
7. Run appropriate testing.
8. Update planning documents when applicable.
9. Report completed work and validation.

For finding-driven work, report:

1. Findings addressed.
2. Findings skipped and why.
3. Files changed.
4. Validation performed.

---

# Building and Testing

## General Rules

- If only ignored files change, no testing is required.
- Never claim hardware testing unless hardware testing occurred.
- Report any testing limitations.
- Documentation-only changes do not require source compilation.

## Formatting

Run:

```powershell
.\clang_format_all.bat
```

Markdown:

```powershell
npx.cmd --yes markdownlint-cli2 --fix --config ".markdownlint-cli2.yaml" "*.md"
```

## Source Testing

Use:

`C:\platformio\penv\Scripts\platformio.exe`

Do not modify:

- `platformio.ini`
- `platformio_extra_envs.ini`

Default environment mapping:

- Modified modem file: use mapped environment(s).
- All other files: `sim7080` and `esp32`.

    | Changed File                        | Testing Environment Names         |
    | ----------------------------------- | --------------------------------- |
    | `src/TinyGsmClientA6.h`             | `a6`                              |
    | `src/TinyGsmClientA7672x.h`         | `a7672x`                          |
    | `src/TinyGsmClientBG96.h`           | `bg96`                            |
    | `src/TinyGsmClientEspressif.h`      | `esp32` `esp8266` `esp8266_nonos` |
    | `src/TinyGsmClientESP32.h`          | `esp32`                           |
    | `src/TinyGsmClientESP8266.h`        | `esp8266`                         |
    | `src/TinyGsmClientESP8266NonOS.h`   | `esp8266_nonos`                   |
    | `src/TinyGsmClientM95.h`            | `m95`                             |
    | `src/TinyGsmClientM590.h`           | `m590`                            |
    | `src/TinyGsmClientMC60.h`           | `mc60`                            |
    | `src/TinyGsmClientSaraR4.h`         | `sarar4`                          |
    | `src/TinyGsmClientSaraR5.h`         | `sarar5`                          |
    | `src/TinyGsmClientSequansMonarch.h` | `monarch`                         |
    | `src/TinyGsmClientSIM70xx.h`        | `sim7000` `sim7000ssl` `sim7080`  |
    | `src/TinyGsmClientSIM800.h`         | `sim800` `sim900` `sim808`        |
    | `src/TinyGsmClientSIM808.h`         | `sim808`                          |
    | `src/TinyGsmClientSIM5360.h`        | `sim5320`                         |
    | `src/TinyGsmClientSIM7000.h`        | `sim7000`                         |
    | `src/TinyGsmClientSIM7000SSL.h`     | `sim7000ssl`                      |
    | `src/TinyGsmClientSIM7080.h`        | `sim7080`                         |
    | `src/TinyGsmClientSIM7600.h`        | `sim7600`                         |
    | `src/TinyGsmClientUBLOX.h`          | `ublox`                           |
    | `src/TinyGsmClientXBEE.h`           | `xbee`                            |
    | All other files                     | `sim7080` `esp32`                 |

Primary test sketch:

`extras/tools/test_build/test_build.ino`

Validation expectations:

- File diagnostics are the minimum validation level.
- When AGENTS testing rules require PlatformIO builds, PlatformIO builds take precedence.

Example:

```powershell
$env:PLATFORMIO_CORE_DIR = "C:\platformio"
$env:PLATFORMIO_SRC_DIR = "extras/tools/test_build"
C:\platformio\penv\Scripts\platformio.exe run -e=sim7080 -e=esp32
```

## Documentation Testing

Run:

```powershell
.\runDoxygen.bat
```

Review logs only if the build fails.

---

# Review Guidelines

When reviewing code:

- Verify compliance with this document.
- Review grammar, spelling, and clarity.
- Verify Doxygen compliance.
- Review C++ snippets in documentation.
- Review `ChangeLog.md` for accuracy.
- Validate AT commands against vendor documentation.

Ignore credentials already removed through repository clean filters.

---

<!-- cSpell:ignore RTTI -->
