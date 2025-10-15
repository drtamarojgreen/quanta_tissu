#pragma once

#include <string>

namespace TissDB {
namespace TissLM {
namespace Config {

// Test Configuration
const std::string MODEL_PATH = "../../../training_output/final_model.pt";
const std::string TOKENIZER_PREFIX = "../../../training_output/tokenizer";
const std::string PROMPT = "The meaning of life is";
constexpr int GENERATION_LENGTH = 100;

} // namespace Config
} // namespace TissLM
} // namespace TissDB
