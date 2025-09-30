#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace Generation {

/**
 * Configuration for text generation
 */
struct GenerationConfig {
    // Sampling method: "greedy", "top_k", "nucleus", "top_a", "random", "beam_search"
    std::string method = "greedy";
    
    // Temperature for sampling (higher = more random)
    float temperature = 1.0f;
    
    // Top-k sampling: keep only top k tokens
    std::optional<int> top_k = std::nullopt;
    
    // Nucleus (top-p) sampling: keep tokens with cumulative probability >= p
    std::optional<float> top_p = std::nullopt;
    
    // Top-a sampling: keep tokens with probability > a
    float top_a = 0.0f;
    
    // Repetition penalty (> 1.0 discourages repetition)
    float repetition_penalty = 1.0f;
    
    // Bias for specific token
    std::optional<int> bias_token_id = std::nullopt;
    float bias_strength = 0.0f;
    
    // End-of-sequence token ID(s)
    std::vector<int> eos_ids;
    
    // Prevent repeating n-grams of this size
    int no_repeat_ngram_size = 0;
    
    // Logit bias for specific tokens
    std::map<int, float> logit_bias;
    
    // Temperature schedule (optional)
    std::vector<float> temperature_schedule;
    
    // Suppress EOS token in output
    bool suppress_eos = false;
    
    // Beam search parameters
    int beam_width = 3;
    
    // Contrastive search parameters
    float contrastive_alpha = 0.6f;
    
    // Mirostat parameters
    float mirostat_tau = 5.0f;
    float mirostat_eta = 0.1f;
    
    /**
     * Create default greedy config
     */
    static GenerationConfig greedy() {
        GenerationConfig config;
        config.method = "greedy";
        return config;
    }
    
    /**
     * Create default sampling config
     */
    static GenerationConfig sampling(float temp = 1.0f) {
        GenerationConfig config;
        config.method = "random";
        config.temperature = temp;
        return config;
    }
    
    /**
     * Create top-k sampling config
     */
    static GenerationConfig top_k(int k, float temp = 1.0f) {
        GenerationConfig config;
        config.method = "top_k";
        config.top_k = k;
        config.temperature = temp;
        return config;
    }
    
    /**
     * Create nucleus (top-p) sampling config
     */
    static GenerationConfig nucleus(float p, float temp = 1.0f) {
        GenerationConfig config;
        config.method = "nucleus";
        config.top_p = p;
        config.temperature = temp;
        return config;
    }
};

} // namespace Generation
