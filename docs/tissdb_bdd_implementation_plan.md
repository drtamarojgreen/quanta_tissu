# TissDB BDD Test Execution Report

- **Date:** 2025-08-18 06:49:48
- **Duration:** 0.00 seconds
- **Overall Result:** FAIL (FATAL)

## Summary

| Metric             | Count |
| ------------------ | ----- |
| Scenarios Run      | 0      |
| Scenarios Passed   | 0    |
| Scenarios Failed   | 0    |
| Steps Run          | 0          |
| Steps Passed       | 0        |
| Steps Failed       | 0        |

## Details

### Environment
- **Compilation Skipped:** Yes

### Errors

**Database Start Error:**

```
Database executable not found at /app/tissdb/tissdb and compilation was skipped via NO_COMPILE flag.
```

**Fatal Script Error:**

```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 170, in run
    raise Exception("Database failed to start.")
Exception: Database failed to start.

```

\n## Conclusion\n\nThe test run failed. See error details above.