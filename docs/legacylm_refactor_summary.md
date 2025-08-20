# Legacy Scripts Refactoring Summary

This document summarizes the refactoring task to move legacy scripts into a dedicated `legacylm` directory.

## 1. Files Moved

The following three scripts were moved from `quanta_tissu/tisslm/` to `quanta_tissu/tisslm/legacylm/`:

- `run_inference.py`
- `train_bpe.py`
- `train.py`

## 2. References Updated

All references to the old paths of the moved scripts were updated throughout the codebase. The following 8 files were modified to reflect the new locations:

1.  `README.md`
2.  `docs/implementation_summary.md`
3.  `docs/bdd_error_analysis.md`
4.  `docs/tisslm_tuning.md`
5.  `quanta_tissu/tisslm/generate_text.py`
6.  `quanta_tissu/tisslm/tokenizer.py`
7.  `docs/tissdb_current_status.md`
8.  `docs/tisslm_plan.md`

## 3. Known Issues

- The file `docs/test_status_report.md` contains one remaining reference to the old paths that could not be updated due to a tool failure. This will be fixed separately.
