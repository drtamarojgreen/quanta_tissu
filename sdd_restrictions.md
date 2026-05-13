# SDD Restrictions: Web Platform CLI API Construction

## 1. Structural Guardrails
- **No Empty Catch Blocks:** All exceptions must be handled, logged, or explicitly ignored with a comment explaining why.
- **No Unused Logic Paths:** Avoid implementing functionality that is not required by the CLI API or current debugging needs.
- **Minimal Abstraction:** Do not introduce complex design patterns or abstraction layers unless they directly simplify the reuse of existing frontend modules.
- **Single Source of Truth:** All state must reside in or be derived from `AppState`.

## 2. Tool Restrictions
The following tools are permitted for construction and execution:
- `node` (v18+)
- `python3`
- `pip`
- `curl`
- `bash`
- `make`
- `tissdb` (the compiled binary)

Any tool outside this set is prohibited.

## 3. Pattern Restrictions
- **Explicit Mocking:** Mocking of browser APIs (window, document, fetch) must be localized and clearly documented.
- **Deterministic state:** State transitions in `AppState` must be predictable and observable via the API.
- **No Placeholders:** `TODO` or `FIXME` comments are prohibited in source code; use `chai_checkins.md` for deferred work.

## 4. Validation Restrictions
- All API outputs must be valid JSON.
- Tests must verify real outputs against expected empirical results.
- Process management must be verified by observing actual process IDs and log accumulation.
