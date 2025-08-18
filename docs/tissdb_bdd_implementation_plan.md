# TissDB BDD Test Execution Report

- **Date:** 2025-08-18 04:32:17
- **Duration:** 30.85 seconds
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
- **Compilation Skipped:** No

### Errors

**Database Start Error:**

```
Failed to build database: Command '['make', '-C', '/app/tissdb']' returned non-zero exit status 2.
```

**Fatal Script Error:**

```
Traceback (most recent call last):
  File "/app/tests/bdd_runner.py", line 170, in run
    raise Exception("Database failed to start.")
Exception: Database failed to start.

```

\n## Conclusion\n\nThe test run failed. See error details above.