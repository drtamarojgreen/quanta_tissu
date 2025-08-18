# TissDB BDD Test Execution Report

- **Date:** 2025-08-18 00:49:47
- **Duration:** 2.34 seconds
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

**Fatal Script Error:**

```
Traceback (most recent call last):
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\bdd_runner.py", line 176, in run
    from tests.features.steps import (test_kv_cache_steps, test_tokenizer_steps, test_predict_steps,
    ...<3 lines>...
                                      test_select_queries_steps)
  File "C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\tests\features\steps\test_kv_cache_steps.py", line 10, in <module>
    from quanta_tissu.tisslm.tokenizer import Tokenizer, vocab
ImportError: cannot import name 'vocab' from 'quanta_tissu.tisslm.tokenizer' (C:\Users\tamar\Documents\DataAnnotation\Gemini\quanta_tissu\quanta_tissu\tisslm\tokenizer.py)

```

\n## Conclusion\n\nThe test run failed. See error details above.