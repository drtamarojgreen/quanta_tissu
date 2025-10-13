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
#include <set>

using namespace TissDB::TissLM::Core;
using namespace TissNum;

// Helper for basic text analysis
struct BasicTextAnalysis {
    int word_count = 0;
    int sentence_count = 0;
    std::map<std::string, int> bigram_counts;
};

BasicTextAnalysis perform_basic_text_analysis(const std::string& text) {
    BasicTextAnalysis analysis;

    // Word count
    std::vector<std::string> words;
    std::string current_word;
    for (char c : text) {
        if (std::isspace(c) || std::ispunct(c)) {
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

    // Sentence count
    for (char c : text) {
        if (c == '.' || c == '!' || c == '?') {
            analysis.sentence_count++;
        }
    }
    if (analysis.sentence_count == 0 && !text.empty()) { // If no punctuation, but text exists, count as one sentence
        analysis.sentence_count = 1;
    }

    // Bigram counts
    if (words.size() >= 2) {
        for (size_t i = 0; i < words.size() - 1; ++i) {
            std::string bigram = words[i] + " " + words[i+1];
            analysis.bigram_counts[bigram]++;
        }
    }

    return analysis;
}

void run_advanced_analysis_evaluation() {
    std::cout << "=== Running Advanced Text Analysis Evaluation (C++) ===" << std::endl;

    // --- Setup Model and Tokenizer ---
    Tokenizer tokenizer("models/tokenizers/revised_tokenizer");
    int vocab_size = tokenizer.get_vocab_size();
    int max_seq_len = 50;
    int embed_dim = 32;
    int num_heads = 4;
    int num_layers = 2;
    float dropout_rate = 0.1f;
    int lora_rank = 0; 

    std::shared_ptr<TransformerModel> model = std::make_shared<TransformerModel>(vocab_size, max_seq_len, embed_dim, num_heads, num_layers, dropout_rate, lora_rank);
    Tokenizer tokenizer("../dummy"); // Assuming dummy_vocab.json and dummy_merges.txt

    std::cout << "  Model and Tokenizer initialized." << std::endl;

    // --- Generate Text for Analysis ---
    std::vector<std::string> prompts = {
        "The quick brown fox jumps over the lazy dog. The dog barks loudly.",
        "Artificial intelligence is transforming the world. Machine learning is a subset of AI."
    };
    int generation_length = 20;
    Generation::GenerationConfig gen_config = Generation::GenerationConfig::greedy();

    std::vector<std::string> generated_texts;
    for (const std::string& prompt : prompts) {
        Generator generator(model, gen_config);
        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::string generated_text = tokenizer.decode(generator.generate(prompt_tokens, generation_length));
        generated_texts.push_back(generated_text);
        std::cout << "\n  Generated Text: \"" << generated_text.substr(0, 100) << (generated_text.length() > 100 ? "..." : "") << "\"" << std::endl;
    }

    // --- Perform Basic Analysis ---
    std::cout << "\n--- Performing Basic Text Analysis ---" << std::endl;
    for (const std::string& text : generated_texts) {
        BasicTextAnalysis analysis = perform_basic_text_analysis(text);
        std::cout << "\n  Analysis for: \"" << text.substr(0, 50) << (text.length() > 50 ? "..." : "") << "\"" << std::endl;
        std::cout << "    Word Count: " << analysis.word_count << std::endl;
        std::cout << "    Sentence Count: " << analysis.sentence_count << std::endl;
        std::cout << "    Top Bigrams:" << std::endl;
        // Sort bigrams by count (simplified, just print top few)
        std::vector<std::pair<std::string, int>> sorted_bigrams(analysis.bigram_counts.begin(), analysis.bigram_counts.end());
        std::sort(sorted_bigrams.begin(), sorted_bigrams.end(), [](const auto& a, const auto& b) { return a.second > b.second; });
        for (size_t i = 0; i < std::min((size_t)3, sorted_bigrams.size()); ++i) {
            std::cout << "      \"" << sorted_bigrams[i].first << "\": " << sorted_bigrams[i].second << std::endl;
        }
    }

    std::cout << "\n=== Advanced Text Analysis Evaluation (C++) Completed ===" << std::endl;
}

int main() {
    try {
        run_advanced_analysis_evaluation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Advanced Text Analysis Evaluation failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
