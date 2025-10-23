#pragma once
#include <string>

struct TestConfig {
    static constexpr const char* TokenizerPath = "../../../../models/tokenizers/revised_tokenizer";
    static constexpr int MaxSeqLen = 512;
    static constexpr int EmbedDim = 32;
    static constexpr int NumHeads = 4;
    static constexpr int NumLayers = 2;
    static constexpr float DropoutRate = 0.1f;
    static constexpr int LoraRank = 4;
};
