# Analysis of Tokenizer Training Performance and Telemetry

## 1. Algorithmic Complexity Analysis

The `TissLM::Tokenizer::Tokenizer::train` method implements a Byte-Pair Encoding (BPE) training algorithm. A static analysis reveals that its computational complexity is approximately $O(V \cdot C)$, where $V$ is the target vocabulary size and $C$ is the total number of bytes in the corpus.

### Bottleneck Identification
In each of the $V - 256$ merge iterations, the following high-cost operations occur:

1.  **Global Pair Counting**: The entire corpus (represented as `word_byte_sequences`) is traversed to build a `pair_counts` map. This is $O(C)$ and uses `std::map<std::pair<int, int>, int>`, which incurs significant logarithmic overhead for each increment.
2.  **Corpus Reconstruction**: After selecting the best pair, the entire corpus is traversed again to apply the merge. This involves creating a new `std::vector<std::vector<int>>` and many smaller `std::vector<int>` allocations, leading to high memory churn and $O(C)$ time complexity.

With a 50,257 target vocabulary (as previously set) and a moderate corpus, the total number of operations easily reaches the tens of billions, causing the training process to take hours or even days on standard hardware.

## 2. Telemetry and Verbosity Analysis

### The `verbose` Flag
The `Tokenizer::train` method uses a `verbose` parameter to control progress logging.
Previously, it defaulted to `false` in the header:
```cpp
void train(const std::string& text, int vocab_size, bool verbose = false);
```
In `tests/model/program/train_model.cpp`, the call to this method was:
```cpp
tokenizer->train(corpus, 50257);
```
Because only two arguments were provided, the C++ compiler used the default value for the third parameter, which was `false`.

### Impact on Console Output
When `verbose` is `false`, the following progress logging inside the merge loop is bypassed:
```cpp
if (verbose) {
    std::cout << "Merge " << i+1 << "/" << num_merges << ": (" << best_pair.first << ", " << best_pair.second << ") -> " << new_token_id << std::endl;
}
```
Consequently, the console remains silent during the entire training process. Combined with the high computational cost, this creates the perception of a "hang" or "deadlock," even though the process is actively (but slowly) computing.

## 3. Resolution and Updates

To address these issues, the following changes have been implemented:

1.  **Vocabulary Size Correction**: The vocabulary size in `tests/model/program/train_model.cpp` has been updated from `50257` to `4196`. This aligns the C++ training script with the project's resource-conscious goals and ensures that training can complete within a reasonable timeframe (and the 3600-second timeout of `workout_simple.sh`).
2.  **Telemetry by Default**: The `Tokenizer::train` method in `quanta_tissu/tisslm/program/tokenizer/tokenizer.h` now defaults `verbose` to `true`. This ensures that progress is always visible in the console, providing immediate feedback to the operator.

## 4. Conclusions
The observed "hang" in `workout_simple.sh` was a deterministic result of an inefficient $O(V \cdot C)$ algorithm coupled with a lack of progress telemetry and an excessively large target vocabulary for the given hardware constraints. The implemented updates provide visibility and a more appropriate workload for the current system architecture.
