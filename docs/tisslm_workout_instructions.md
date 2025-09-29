# TissLM Workout Instructions

This document outlines the parameters for running the TissLM workout scripts. The main script for running the workouts is `full_fledged_workout.py`, located in `quanta_tissu/tisslm/evaluation/`.

## Running the Workout

To run the workout, execute the `full_fledged_workout.py` script with the following parameters:

```bash
python quanta_tissu/tisslm/evaluation/full_fledged_workout.py [--test <test_number>] [--save-results]
```

### Parameters

*   `--test <test_number>`: Specifies which test suite to run. If this parameter is omitted, no tests will be run. The available test numbers are:
    *   `1`: Runs the KV Cache test (`full_fledged_workout01.py`).
    *   `2`: Runs the RAG and Self-Updating Knowledge Base test (`full_fledged_workout02.py`).
    *   `3`: Runs the Experimental Sampling tests (`full_fledged_workout03.py`).
    *   `4`: Runs the Standard Generation tests (`full_fledged_workout04.py`).
    *   `5`: Runs the Rule Enforcement test (`full_fledged_workout05.py`).
    *   `6`: Runs the Advanced Text Analysis tests (`full_fledged_workout06.py`).
    *   `7`: Runs the Advanced Text Analysis tests (`full_fledged_workout07.py`).
    *   `8`: Runs the TissDB-Lite integration tests (`full_fledged_workout08.py`).
    *   `9`: Runs the TissLang Parser tests (`full_fledged_workout09.py`).
    *   `expanded`: Runs an expanded parameter evaluation.
    *   `all`: Runs all available tests.

*   `--save-results`: If this flag is included, the script will save detailed evaluation results to files in the `evaluation_logs` directory.

### Individual Workout Scripts

The following scripts do not accept command-line arguments directly. They are executed as part of the main `full_fledged_workout.py` script:

*   `full_fledged_workout01.py`
*   `full_fledged_workout02.py`
*   `full_fledged_workout03.py`
*   `full_fledged_workout04.py`
*   `full_fledged_workout05.py`
*   `full_fledged_workout06.py`
*   `full_fledged_workout07.py`
*   `full_fledged_workout08.py`
*   `full_fledged_workout09.py`