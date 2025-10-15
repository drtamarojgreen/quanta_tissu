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

// Helper to convert logits to probabilities (softmax)
Matrix softmax_cpp(const Matrix& input) {
    Matrix output(input.rows(), input.cols());
    for (int r = 0; r < input.rows(); ++r) {
        float max_val = -std::numeric_limits<float>::infinity();
        for (int c = 0; c < input.cols(); ++c) {
            if (input(r, c) > max_val) {
                max_val = input(r, c);
            }
        }

        float sum_exp = 0.0f;
        for (int c = 0; c < input.cols(); ++c) {
            output(r, c) = std::exp(input(r, c) - max_val);
            sum_exp += output(r, c);
        }

        for (int c = 0; c < input.cols(); ++c) {
            output(r, c) = output(r, c) / sum_exp;
        }
    }
    return output;
}

// --- Simplified sampling for no-cache version ---
int sample_token_greedy(const Matrix& logits) {
    float max_logit = -std::numeric_limits<float>::infinity();
    int sampled_token = -1;

    for (int c = 0; c < logits.cols(); ++c) {
        if (logits(0, c) > max_logit) {
            max_logit = logits(0, c);
            sampled_token = c;
        }
    }
    return sampled_token;
}

// --- Single KV Cache Test ---
struct KVCacheTestResult {
    std::string prompt;
    int n_new_tokens;
    std::string method;
    float time_no_cache;
    float time_cache;
    std::vector<int> generated_tokens_no_cache;
    std::vector<int> generated_tokens_cache;
    bool correctness;
    float speedup_percent;
};

KVCacheTestResult run_single_kv_cache_test(
    std::shared_ptr<TransformerModel> model,
    Tokenizer& tokenizer,
    const std::string& prompt,
    int n_new_tokens,
    const Generation::GenerationConfig& config
) {
    KVCacheTestResult results;
    results.prompt = prompt;
    results.n_new_tokens = n_new_tokens;
    results.method = (config.top_k.value_or(0) <= 1) ? "greedy" : "top_k";

    // --- 1. Baseline Generation (No Cache) ---
    auto start_time_no_cache = std::chrono::high_resolution_clock::now();

    std::vector<int> generated_tokens_no_cache_ids = tokenizer.encode(prompt);
    std::vector<int> current_tokens_no_cache = generated_tokens_no_cache_ids;

    for (int i = 0; i < n_new_tokens; ++i) {
        Matrix input_token_matrix(1, current_tokens_no_cache.size());
        for(size_t j=0; j<current_tokens_no_cache.size(); ++j) {
            input_token_matrix(0, j) = static_cast<float>(current_tokens_no_cache[j]);
        }
        
        Matrix logits = model->forward(input_token_matrix);
        int next_token = sample_token_greedy(logits); // Always greedy for no-cache baseline for simplicity
        generated_tokens_no_cache_ids.push_back(next_token);
        current_tokens_no_cache.push_back(next_token);
    }
    auto end_time_no_cache = std::chrono::high_resolution_clock::now();
    results.time_no_cache = std::chrono::duration<float>(end_time_no_cache - start_time_no_cache).count();
    results.generated_tokens_no_cache = generated_tokens_no_cache_ids;

    // --- 2. Cached Generation ---
    auto start_time_cache = std::chrono::high_resolution_clock::now();

    std::vector<int> generated_tokens_cache_ids = tokenizer.encode(prompt);
    std::vector<std::pair<Matrix, Matrix>> kv_cache;

    // Process prompt tokens to initialize KV cache
    for (size_t i = 0; i < generated_tokens_cache_ids.size(); ++i) {
        Matrix input_token(1, 1);
        input_token(0, 0) = static_cast<float>(generated_tokens_cache_ids[i]);

        std::vector<std::pair<Matrix, Matrix>> new_kv_cache_for_step;
        model->forward_inference(input_token, kv_cache, new_kv_cache_for_step);
        kv_cache = new_kv_cache_for_step;
    }

    // Generate new tokens using cache
    Generator generator(model, config);
    for (int i = 0; i < n_new_tokens; ++i) {
        Matrix input_token(1, 1);
        input_token(0, 0) = static_cast<float>(generated_tokens_cache_ids.back());

        std::vector<std::pair<Matrix, Matrix>> new_kv_cache_for_step;
        Matrix logits = model->forward_inference(input_token, kv_cache, new_kv_cache_for_step);
        kv_cache = new_kv_cache_for_step;

        int next_token = generator.sample_token(logits, generated_tokens_cache_ids);
        generated_tokens_cache_ids.push_back(next_token);
    }

    auto end_time_cache = std::chrono::high_resolution_clock::now();
    results.time_cache = std::chrono::duration<float>(end_time_cache - start_time_cache).count();
    results.generated_tokens_cache = generated_tokens_cache_ids;

    // --- 3. Analysis ---
    results.correctness = (results.generated_tokens_no_cache == results.generated_tokens_cache);
    results.speedup_percent = (results.time_no_cache > 0) ? ((results.time_no_cache - results.time_cache) / results.time_no_cache * 100) : 0;

    return results;
}

void run_kv_cache_evaluation() {
    std::cout << "=== Running KV Cache Evaluation (C++) ===" << std::endl;

    // --- Setup Model and Tokenizer ---
    Tokenizer tokenizer("models/tokenizers/revised_tokenizer");
    int vocab_size = tokenizer.get_vocab_size();
    int max_seq_len = 500; // Increased max_seq_len for longer prompts
    int embed_dim = 32;
    int num_heads = 4;
    int num_layers = 2;
    float dropout_rate = 0.1f;
    int lora_rank = 0; // Disable LoRA for this test for simplicity

    std::shared_ptr<TransformerModel> model = std::make_shared<TransformerModel>(vocab_size, max_seq_len, embed_dim, num_heads, num_layers, dropout_rate, lora_rank);

    std::cout << "  Model and Tokenizer initialized." << std::endl;

    // --- Test Scenarios ---
    std::vector<std::tuple<std::string, int, std::string>> kv_cache_test_scenarios = {
        {"The laws of physics state that", 80, "Short physics prompt"},
        {"In the beginning of the universe, there was nothing but darkness and void", 200, "Medium cosmology prompt"},
        {"The development of artificial intelligence has progressed rapidly in recent years, with breakthroughs in machine learning, natural language processing, and computer vision leading to", 400, "Long AI development prompt"},
    };

    std::vector<Generation::GenerationConfig> generation_methods = {
        Generation::GenerationConfig::greedy(),
        Generation::GenerationConfig::with_top_k(10, 1.0f) // Top-k with temperature 1.0
    };

    std::vector<KVCacheTestResult> all_results;

    for (const auto& scenario : kv_cache_test_scenarios) {
        std::string prompt = std::get<0>(scenario);
        int tokens_to_generate = std::get<1>(scenario);
        std::string description = std::get<2>(scenario);

        std::cout << "\n  Scenario: " << description << std::endl;
        std::cout << "    Prompt: \"" << prompt.substr(0, 50) << (prompt.length() > 50 ? "..." : "") << "\"" << std::endl;
        std::cout << "    Tokens to generate: " << tokens_to_generate << std::endl;

        for (const auto& method_config : generation_methods) {
            std::cout << "    Method: " << ((method_config.top_k.value_or(0) <= 1) ? "greedy" : "top_k") << " (TopK: " << method_config.top_k.value_or(-1) << ", Temp: " << method_config.temperature << ")" << std::endl;
            
            try {
                KVCacheTestResult result = run_single_kv_cache_test(model, tokenizer, prompt, tokens_to_generate, method_config);
                all_results.push_back(result);

                std::cout << std::fixed << std::setprecision(4);
                std::cout << "      No Cache Time: " << result.time_no_cache << "s" << std::endl;
                std::cout << "      Cached Time:   " << result.time_cache << "s" << std::endl;
                std::cout << "      Speedup:       " << result.speedup_percent << "%" << std::endl;
                std::cout << "      Correctness:   " << (result.correctness ? "✓" : "✗") << std::endl;

                if (!result.correctness) {
                    std::cout << "      [WARNING] Output mismatch detected!" << std::endl;
                    std::cout << "        No Cache: "; print_tokens_int(result.generated_tokens_no_cache);
                    std::cout << "        Cached:   "; print_tokens_int(result.generated_tokens_cache);
                }
            } catch (const std::exception& e) {
                std::cerr << "      [ERROR] Test failed: " << e.what() << std::endl;
            }
        }
    }

    // --- Overall Analysis ---
    if (!all_results.empty()) {
        int total_tests = all_results.size();
        int successful_tests = 0;
        float total_speedup = 0.0f;

        for (const auto& res : all_results) {
            if (res.correctness) {
                successful_tests++;
                total_speedup += res.speedup_percent;
            }
        }

        std::cout << "\n--- KV Cache Performance Analysis (C++) ---" << std::endl;
        std::cout << "  Total tests run: " << total_tests << std::endl;
        std::cout << "  Successful tests: " << successful_tests << std::endl;
        std::cout << "  Correctness rate: " << std::fixed << std::setprecision(1) << (static_cast<float>(successful_tests) / total_tests * 100.0f) << "%" << std::endl;
        std::cout << "  Average speedup (correct tests): " << std::fixed << std::setprecision(2) << (successful_tests > 0 ? (total_speedup / successful_tests) : 0.0f) << "%" << std::endl;
    }

    std::cout << "\n=== KV Cache Evaluation (C++) Completed ===" << std::endl;
}

int main() {
    try {
        run_kv_cache_evaluation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "KV Cache Evaluation failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
