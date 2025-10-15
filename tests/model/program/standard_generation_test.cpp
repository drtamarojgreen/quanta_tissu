#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
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

using namespace TissDB::TissLM::Core;
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
std::string generate_with_model(std::shared_ptr<TissDB::TissLM::Core::TransformerModel> model, Tokenizer& tokenizer, const std::string& prompt, int generation_length, const Generation::GenerationConfig& config) {
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
    Tokenizer tokenizer("models/tokenizers/revised_tokenizer");
    int vocab_size = tokenizer.get_vocab_size();
    int max_seq_len = 500;
    int embed_dim = 32;
    int num_heads = 4;
    int num_layers = 2;
    float dropout_rate = 0.1f;
    int lora_rank = 0; 

    std::shared_ptr<TransformerModel> model = std::make_shared<TransformerModel>(vocab_size, max_seq_len, embed_dim, num_heads, num_layers, dropout_rate, lora_rank);

    std::cout << "  Model and Tokenizer initialized." << std::endl;

    // --- Test Configurations ---
    struct TestConfig {
        std::string prompt;
        std::string method;
        int length;
        Generation::GenerationConfig gen_config;
    };

    std::vector<TestConfig> test_configurations = {
        // Greedy Method (baseline)
        {"The definition of science is", "greedy", 60, Generation::GenerationConfig::greedy()},

        // Nucleus Sampling: Temperature variations
        {"The future of space exploration involves", "nucleus", 70, Generation::GenerationConfig::nucleus(0.9f, 0.5f)},
        {"The future of space exploration involves", "nucleus", 70, Generation::GenerationConfig::nucleus(0.9f, 0.8f)},
        {"The future of space exploration involves", "nucleus", 70, Generation::GenerationConfig::nucleus(0.9f, 1.2f)},

        // Nucleus Sampling: Top-p variations
        {"A novel is a work of fiction that", "nucleus", 80, Generation::GenerationConfig::nucleus(0.7f, 0.8f)},
        {"A novel is a work of fiction that", "nucleus", 80, Generation::GenerationConfig::nucleus(0.9f, 0.8f)},
        {"A novel is a work of fiction that", "nucleus", 80, Generation::GenerationConfig::nucleus(0.99f, 0.8f)},

        // Different prompt types
        {"To build a successful startup, one must first", "nucleus", 90, Generation::GenerationConfig::nucleus(0.9f, 0.9f)},
        {"Once upon a time, in a land of dragons and magic,", "nucleus", 100, Generation::GenerationConfig::nucleus(0.95f, 0.85f)},

        // Random Sampling
        {"The cat sat on the", "random", 50, Generation::GenerationConfig::sampling(1.0f)},

        // Repetition Penalty
        {"This is a test of the repetition penalty.", "nucleus", 100, []() {
            auto config = Generation::GenerationConfig::nucleus(0.9f, 0.8f);
            config.repetition_penalty = 1.5f;
            return config;
        }()},

        // Logit Bias (assuming token 29 is 'Paris')
        {"The capital of France is", "greedy", 1, []() {
            auto config = Generation::GenerationConfig::greedy();
            config.logit_bias = {{29, 10.0f}};
            return config;
        }()},

        // No-repeat N-gram
        {"This is a test of the no-repeat n-gram. This is a test of the no-repeat n-gram.", "nucleus", 50, []() {
            auto config = Generation::GenerationConfig::nucleus(0.9f, 0.8f);
            config.no_repeat_ngram_size = 3;
            return config;
        }()},

        // Beam Search
        {"The best way to learn is", "beam_search", 50, []() {
            auto config = Generation::GenerationConfig::greedy();
            config.method = "beam_search";
            config.beam_width = 3;
            return config;
        }()},

        // Contrastive Search
        {"The meaning of life is", "contrastive_search", 60, []() {
            auto config = Generation::GenerationConfig::greedy();
            config.method = "contrastive_search";
            config.beam_width = 5;
            config.contrastive_alpha = 0.6f;
            return config;
        }()},

        // Mirostat Sampling
        {"In a world where AI is king,", "mirostat_sampling", 70, []() {
            auto config = Generation::GenerationConfig::greedy();
            config.method = "mirostat_sampling";
            config.mirostat_tau = 5.0f;
            config.mirostat_eta = 0.1f;
            return config;
        }()},

        // Speculative Sampling (placeholder)
        {"The journey of a thousand miles begins with", "speculative_sampling", 40, []() {
            auto config = Generation::GenerationConfig::greedy();
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
