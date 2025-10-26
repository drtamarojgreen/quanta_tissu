#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <set> // For std::set

using namespace TissLM::Core;
using namespace TissLM::Generation;
using namespace TissLM::Tokenizer;
using namespace TissNum;

// Helper to print generated tokens
void print_tokens_int(const std::vector<int>& tokens, const std::string& prefix = "") {
    std::cout << prefix;
    for (int token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;
}

// Helper function to generate text with the model
std::string generate_with_model(std::shared_ptr<TransformerModel> model, Tokenizer& tokenizer, const std::string& prompt, int generation_length, const GenerationConfig& config) {
    Generator generator(model, config);
    std::vector<int> prompt_tokens = tokenizer.encode(prompt);
    std::vector<int> generated_tokens;
    if (config.method == "beam_search") {
        generated_tokens = generator.beam_search(prompt_tokens, generation_length, config.beam_width, -1);
    } else if (config.method == "contrastive_search") {
        generated_tokens = generator.contrastive_search(prompt_tokens, generation_length, config.beam_width, config.contrastive_alpha, -1);
    } else if (config.method == "mirostat_sampling") {
        generated_tokens = generator.mirostat_sampling(prompt_tokens, generation_length, config.mirostat_tau, config.mirostat_eta, -1);
    } else if (config.method == "speculative_sampling") {
        generated_tokens = generator.speculative_sampling(prompt_tokens, generation_length);
    } else {
        generated_tokens = generator.generate(prompt_tokens, generation_length);
    }
    return tokenizer.decode(generated_tokens);
}

// Helper for text analysis (simplified)
struct GenerationAnalysis {
    int word_count = 0;
    float lexical_diversity = 0.0f;
    float repetition_ratio = 0.0f;
    float avg_word_length = 0.0f;
    float generation_time = 0.0f;
};

GenerationAnalysis analyze_generated_text(const std::string& text, float gen_time) {
    GenerationAnalysis analysis;
    analysis.generation_time = gen_time;

    std::vector<std::string> words;
    std::string current_word;
    for (char c : text) {
        if (std::isspace(c)) {
            if (!current_word.empty()) {
                words.push_back(current_word);
                current_word.clear();
            }
        } else {
            current_word += c;
        }
    }
    if (!current_word.empty()) {
        words.push_back(current_word);
    }

    analysis.word_count = words.size();
    if (analysis.word_count == 0) return analysis;

    std::set<std::string> unique_words(words.begin(), words.end());
    analysis.lexical_diversity = static_cast<float>(unique_words.size()) / analysis.word_count;

    std::map<std::string, int> word_counts;
    for (const auto& word : words) {
        word_counts[word]++;
    }

    int repeated_word_count = 0;
    float total_word_length = 0.0f;
    for (const auto& pair : word_counts) {
        if (pair.second > 1) {
            repeated_word_count++;
        }
        total_word_length += pair.first.length() * pair.second;
    }
    analysis.repetition_ratio = static_cast<float>(repeated_word_count) / analysis.word_count;
    analysis.avg_word_length = total_word_length / analysis.word_count;

    return analysis;
}

void run_standard_generation_evaluation() {
    std::cout << "=== Running Standard Generation Evaluation (C++) ===" << std::endl;

    // --- Setup Model and Tokenizer ---
    Tokenizer tokenizer(TestConfig::TokenizerPath);
    int vocab_size = tokenizer.get_vocab_size();

    std::shared_ptr<TransformerModel> model = std::make_shared<TransformerModel>(
        vocab_size,
        TestConfig::MaxSeqLen,
        TestConfig::EmbedDim,
        TestConfig::NumHeads,
        TestConfig::NumLayers,
        TestConfig::DropoutRate,
        0 // LoraRank = 0 for this test
    );

    std::cout << "  Model and Tokenizer initialized." << std::endl;

    // --- Test Configurations ---
    struct TestConfig {
        std::string prompt;
        std::string method;
        int length;
        GenerationConfig gen_config;
    };

    std::vector<TestConfig> test_configurations = {
        // Greedy Method (baseline)
        {"The definition of Cognitive Behavioral Therapy is", "greedy", 60, GenerationConfig::greedy()},

        // Nucleus Sampling: Temperature variations
        {"To overcome negative thought patterns, one can", "nucleus", 70, GenerationConfig::nucleus(0.9f, 0.5f)},
        {"To overcome negative thought patterns, one can", "nucleus", 70, GenerationConfig::nucleus(0.9f, 0.8f)},
        {"To overcome negative thought patterns, one can", "nucleus", 70, GenerationConfig::nucleus(0.9f, 1.2f)},

        // Nucleus Sampling: Top-p variations
        {"A cognitive distortion is a biased perspective on", "nucleus", 80, GenerationConfig::nucleus(0.7f, 0.8f)},
        {"A cognitive distortion is a biased perspective on", "nucleus", 80, GenerationConfig::nucleus(0.9f, 0.8f)},
        {"A cognitive distortion is a biased perspective on", "nucleus", 80, GenerationConfig::nucleus(0.99f, 0.8f)},

        // Different prompt types
        {"To build a strong therapeutic alliance, a therapist must", "nucleus", 90, GenerationConfig::nucleus(0.9f, 0.9f)},
        {"Once upon a time, in a therapy session, a client realized", "nucleus", 100, GenerationConfig::nucleus(0.95f, 0.85f)},

        // Random Sampling
        {"The patient sat on the", "random", 50, GenerationConfig::sampling(1.0f)},

        // Repetition Penalty
        {"This is a test of challenging automatic thoughts. This is a test of challenging automatic thoughts.", "nucleus", 100, []() {
            auto config = GenerationConfig::nucleus(0.9f, 0.8f);
            config.repetition_penalty = 1.5f;
            return config;
        }()},

        // Logit Bias (assuming a relevant token for 'anxiety')
        {"A common symptom of social anxiety is", "greedy", 1, []() {
            auto config = GenerationConfig::greedy();
            // Assuming token 40 represents 'avoidance'
            config.logit_bias = {{40, 10.0f}};
            return config;
        }()},

        // No-repeat N-gram
        {"This is a test of a behavioral experiment. This is a test of a behavioral experiment.", "nucleus", 50, []() {
            auto config = GenerationConfig::nucleus(0.9f, 0.8f);
            config.no_repeat_ngram_size = 3;
            return config;
        }()},

        // Beam Search
        {"The best way to practice mindfulness is", "beam_search", 50, []() {
            auto config = GenerationConfig::greedy();
            config.method = "beam_search";
            config.beam_width = 3;
            return config;
        }()},

        // Contrastive Search
        {"The core belief behind impostor syndrome is", "contrastive_search", 60, []() {
            auto config = GenerationConfig::greedy();
            config.method = "contrastive_search";
            config.beam_width = 5;
            config.contrastive_alpha = 0.6f;
            return config;
        }()},

        // Mirostat Sampling
        {"In a session focused on exposure therapy,", "mirostat_sampling", 70, []() {
            auto config = GenerationConfig::greedy();
            config.method = "mirostat_sampling";
            config.mirostat_tau = 5.0f;
            config.mirostat_eta = 0.1f;
            return config;
        }()},

        // Speculative Sampling (placeholder)
        {"The journey to mental wellness begins with", "speculative_sampling", 40, []() {
            auto config = GenerationConfig::greedy();
            config.method = "speculative_sampling";
            return config;
        }()},
    };

    std::vector<GenerationAnalysis> all_results;

    for (const auto& config_item : test_configurations) {
        std::cout << "\n  --- Config (Method: " << config_item.method << ") ---" << std::endl;
        std::cout << "  Prompt: \"" << config_item.prompt.substr(0, 50) << (config_item.prompt.length() > 50 ? "..." : "") << "\"" << std::endl;
        std::cout << "  Length: " << config_item.length << ", Temp: " << config_item.gen_config.temperature;
        if (config_item.gen_config.top_k.has_value()) std::cout << ", TopK: " << config_item.gen_config.top_k.value();
        if (config_item.gen_config.top_p.has_value()) std::cout << ", TopP: " << config_item.gen_config.top_p.value();
        std::cout << std::endl;

        try {
            auto start_time = std::chrono::high_resolution_clock::now();
            std::string generated_text = generate_with_model(model, tokenizer, config_item.prompt, config_item.length, config_item.gen_config);
            auto end_time = std::chrono::high_resolution_clock::now();
            float generation_time = std::chrono::duration<float>(end_time - start_time).count();

            GenerationAnalysis analysis = analyze_generated_text(generated_text, generation_time);
            all_results.push_back(analysis);

            std::cout << "    Generated Text: \"" << generated_text.substr(0, 100) << (generated_text.length() > 100 ? "..." : "") << "\n";
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "    Generation Time: " << analysis.generation_time << "s\n";
            std::cout << "    Word Count: " << analysis.word_count << ", Lexical Diversity: " << analysis.lexical_diversity << ", Repetition Ratio: " << analysis.repetition_ratio << "\n";

        } catch (const std::exception& e) {
            std::cerr << "      [ERROR] Test failed: " << e.what() << std::endl;
        }
    }

    // --- Overall Analysis (simplified) ---
    if (!all_results.empty()) {
        std::cout << "\n--- Standard Generation Performance Analysis (C++) ---" << std::endl;
        float avg_diversity = 0.0f;
        float avg_repetition = 0.0f;
        float avg_time = 0.0f;
        int count = 0;

        for (const auto& res : all_results) {
            avg_diversity += res.lexical_diversity;
            avg_repetition += res.repetition_ratio;
            avg_time += res.generation_time;
            count++;
        }

        if (count > 0) {
            avg_diversity /= count;
            avg_repetition /= count;
            avg_time /= count;
        }

        std::cout << "  Average Lexical Diversity: " << avg_diversity << std::endl;
        std::cout << "  Average Repetition Ratio: " << avg_repetition << std::endl;
        std::cout << "  Average Generation Time: " << avg_time << "s" << std::endl;
    }

    std::cout << "\n=== Standard Generation Evaluation (C++) Completed ===" << std::endl;
}

int main() {
    try {
        run_standard_generation_evaluation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Standard Generation Evaluation failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
