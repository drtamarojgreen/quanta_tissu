#pragma once

namespace TissDB {
namespace TissLM {
namespace Config {

// Model Configuration
constexpr int VOCAB_SIZE = 5000;
constexpr int SEQ_LEN = 128;
constexpr int EMBED_DIM = 128;
constexpr int NUM_HEADS = 4;
constexpr int NUM_LAYERS = 2;
constexpr float DROPOUT_RATE = 0.1f;

} // namespace Config
} // namespace TissLM
} // namespace TissDB
