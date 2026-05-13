# RESTRICTIONS: quantaprisma

## Pattern Restrictions
- PROHIBITED: Empty catch/except blocks.
- PROHIBITED: Meaningless assertions.
- PROHIBITED: Unused logic paths or imports.
- MANDATORY: Use of `curses` for terminal UI.

## Tool Restrictions
- ALLOWED: `curses` (TUI), `requests` (API communication), `json` (data parsing), `sys`, `os`, `logging`.
- PROHIBITED: External UI libraries beyond standard `curses`.

## Architectural Restrictions
- MANDATORY: Side-panel menu for navigation.
- MANDATORY: Main content area for configuration and output.
- MANDATORY: Minimalist design, avoiding unnecessary abstraction.
- PROHIBITED: TODOs or placeholders in source code.

## Validation Restrictions
- MANDATORY: All validation must reference real API outputs.
- MANDATORY: Empirical execution results must be displayed.
