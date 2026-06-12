# Sorrel SDD Checkins

## Active Task: Web Platform CLI API & Process Management

### Requirements
- Expose frontend modules as a CLI API in `web_platform/frontend/js/api/`.
- API must accept JSON and execute platform commands.
- API must mock platform state and tab navigation.
- Fix process management issue: loss of training/analyzer output during tab navigation.
- Add `platform_logs` collection to TissDB via the API.
- Implement unit, BDD, and E2E tests in `/tests/web_platform/`.
- Implement SDD tests in `tests/sorrel/sdd/`.

### Deferred Work
- E2E testing of full C++ model training (currently using minimal parameters).
- Full browser environment simulation (currently using basic DOM mock).

### Current Sip: SDD Completion & Cleanup
- [x] Create checkin/checkout files.
- [x] Refine Restrictions in `sdd_restrictions.md`.
- [x] Design C++ Verification Cards.
- [x] Fix JSON parsing errors in `UnitTestsClass.cpp`.
- [x] Ensure `platform_api.js` handles non-JSON responses from backend.
- [x] Implement all unit/BDD tests as SDD cards.

## Active Task: Training-Analyzer Integration Telemetry
- **Goal**: Enable and verify telemetry between Model Training and the Runtime Model Analyzer.
- **Tasks**:
  - Instrument `trainer.cpp` with `RMA_ERROR_VAL` for epoch loss reporting.
  - Create `verify_training_instrumentation` SDD card.
  - Create `verify_training_telemetry` SDD card.
- **Constraints**:
  - Must use existing `rma::ErrorType` (e.g., `INFO`).
  - Must not break existing training loop logic.
