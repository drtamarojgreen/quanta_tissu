# Checkpointing Implementation Notes

This document outlines key implementation details and fixes related to model checkpointing in QuantaTissu.

## Optimizer State Bug and Fix

### Problem
The initial implementation of `save_checkpoint` in `quanta_tissu/utils.py` failed with a `ValueError: setting an array element with a sequence`. This occurred because the optimizer's state (the moving averages `m` and `v`) is stored as a Python list of NumPy arrays, where each array has a different shape corresponding to the parameters of a different model layer.

The `np.savez` function was being passed this list of arrays under a single key (e.g., `"optimizer_m": optimizer.m`). `np.savez` attempts to create a single NumPy array from this list, which fails due to the varying shapes of the elements (it's a "ragged" or "inhomogeneous" array).

### Solution
The fix involves changing the structure of the saved checkpoint file. Instead of saving the list of optimizer state arrays directly, each array within the list is saved as a separate, individually keyed entry in the `.npz` file.

**Saving:**
The `save_checkpoint` function was modified to iterate through the optimizer's state lists (`m` and `v`) and save each array with a unique key, like `optimizer_m_0`, `optimizer_m_1`, `optimizer_v_0`, and so on. A separate key, `num_params`, is also saved to know how many parameter arrays to load.

**Loading:**
The `load_checkpoint` function was updated to read these individual arrays and reconstruct the state lists (`m` and `v`) in the correct order. It uses the `num_params` value to loop the correct number of times.

This approach ensures that each array is handled individually by `np.savez`, avoiding the shape-related `ValueError` and creating a robust checkpointing system.
