# SDD Restrictions: Web Platform CLI API Construction

## 1. Structural Guardrails
- **No Empty Catch Blocks:** All exceptions must be handled, logged, or explicitly ignored with a comment explaining why.
- **No Unused Logic Paths:** Avoid implementing functionality that is not required by the CLI API or current debugging needs.
- **Minimal Abstraction:** Do not introduce complex design patterns or abstraction layers unless they directly simplify the reuse of existing frontend modules.
- **Single Source of Truth:** All state must reside in or be derived from `AppState`.
- **Process Management Consolidation:** Long-running processes must be managed by the Java Orchestrator. No module-local polling or process ownership is permitted in the frontend.
- **C++ Only SDD Tests:** All verification cards and runners in `tests/sorrel/sdd/` must be written in C++. Orchestration of Node.js or Python scripts is allowed via system calls.

## 2. Tool Restrictions
The following tools are permitted for construction and execution:
- `node` (v18+)
- `python3`
- `pip`
- `curl`
- `bash`
- `make`
- `g++` (C++17+)
- `tissdb` (the compiled binary)

Any tool outside this set is prohibited.

## 3. Pattern Restrictions
- **Explicit Mocking:** Mocking of browser APIs (window, document, fetch) must be localized and clearly documented.
- **Deterministic state:** State transitions in `AppState` must be predictable and observable via the API.
- **No Placeholders:** `TODO` or `FIXME` comments are prohibited in source code; use `sorrel_checkins.md` for deferred work.

## 4. Validation Restrictions
- All API outputs must be valid JSON.
- Tests must verify real outputs against expected empirical results.
- **Process Continuity Validation:** Verification must confirm log continuity and state persistence across tab switches and browser sessions.
- Process management must be verified by observing actual process IDs and log accumulation.
- SDD Cards must strictly use decorators (`// @Card`, `// @Is`, `// @Needs`, `// @Results`) for requirement enforcement.
