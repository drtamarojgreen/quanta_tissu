#pragma once

#include <string>

namespace TissDB {
namespace TissLM {
namespace Config {

// Training Configuration
const std::string CORPUS_PATH = "../../corpus/corpus.txt";
const std::string SAVE_DIR = "../../training_output";
constexpr int BATCH_SIZE = 32;
constexpr int EPOCHS = 5;
constexpr float LEARNING_RATE = 0.001f;

} // namespace Config
} // namespace TissLM
} // namespace TissDB
