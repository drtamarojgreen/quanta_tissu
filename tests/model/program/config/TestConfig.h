#pragma once
#include <string>

#ifndef TOKENIZER_PATH_FROM_CMAKE
// This is a fallback for IDEs that don't get the definition from CMake
#define TOKENIZER_PATH_FROM_CMAKE "../../../../test_tokenizer/test_tokenizer"
#endif

struct TestConfig {
    static constexpr const char* TokenizerPath = TOKENIZER_PATH_FROM_CMAKE;
    static constexpr int MaxSeqLen = 512;
    static constexpr int EmbedDim = 32;
    static constexpr int NumHeads = 4;
    static constexpr int NumLayers = 2;
    static constexpr int FFNDim = 128;
    static constexpr float DropoutRate = 0.1f;
    static constexpr int LoraRank = 4;
};