#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "../../../quanta_tissu/tisslm/program/rules/rule_enforcer.h"
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

// Helper for text analysis (simplified)
struct TextQualityAnalysis {
    int word_count = 0;
    float lexical_diversity = 0.0f;
    float repetition_ratio = 0.0f;
    float capitalization_ratio = 0.0f;
};

TextQualityAnalysis analyze_text_quality(const std::string& text) {
    TextQualityAnalysis analysis;

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
    if (analysis.word_count == 0) return analysis;

    std::set<std::string> unique_words(words.begin(), words.end());
    analysis.lexical_diversity = static_cast<float>(unique_words.size()) / analysis.word_count;

    std::map<std::string, int> word_counts;
    for (const auto& word : words) {
        word_counts[word]++;
    }

    int repeated_word_instances = 0;
    for (const auto& pair : word_counts) {
        if (pair.second > 1) {
            repeated_word_instances += (pair.second - 1);
        }
    }
    analysis.repetition_ratio = static_cast<float>(repeated_word_instances) / analysis.word_count;

    // Capitalization analysis
    int properly_capitalized_sentences = 0;
    int total_sentences = 0;
    std::string sentence_buffer;
    for (char c : text) {
        sentence_buffer += c;
        if (c == '.' || c == '!' || c == '?') {
            std::string trimmed_sentence = sentence_buffer;
            // Trim leading/trailing whitespace
            trimmed_sentence.erase(0, trimmed_sentence.find_first_not_of(" 	\n\r"));
            trimmed_sentence.erase(trimmed_sentence.find_last_not_of(" 	\n\r") + 1);

            if (!trimmed_sentence.empty()) {
                total_sentences++;
                if (std::isupper(trimmed_sentence[0])) {
                    properly_capitalized_sentences++;
                }
            }
            sentence_buffer.clear();
        }
    }
    if (total_sentences > 0) {
        analysis.capitalization_ratio = static_cast<float>(properly_capitalized_sentences) / total_sentences;
    }

    return analysis;
}

std::string generate_with_model(std::shared_ptr<TissDB::TissLM::Core::TransformerModel> model, Tokenizer& tokenizer, const std::string& prompt, int generation_length, const Generation::GenerationConfig& config) {
    Generator generator(model, config);
    std::vector<int> prompt_tokens = tokenizer.encode(prompt);
    std::vector<int> generated_tokens = generator.generate(prompt_tokens, generation_length);
    return tokenizer.decode(generated_tokens);
}

void run_rule_enforcement_evaluation() {
    std::cout << "=== Running Rule Enforcement Evaluation (C++) ===" << std::endl;

    // --- Setup Model and Tokenizer ---
    int vocab_size = 100;
    int max_seq_len = 50;
    int embed_dim = 32;
    int num_heads = 4;
    int num_layers = 2;
    float dropout_rate = 0.1f;
    int lora_rank = 0;

    std::shared_ptr<TransformerModel> model = std::make_shared<TransformerModel>(vocab_size, max_seq_len, embed_dim, num_heads, num_layers, dropout_rate, lora_rank);
    Tokenizer tokenizer("dummy"); // Assuming dummy_vocab.json and dummy_merges.txt

    std::cout << "  Model and Tokenizer initialized." << std::endl;

    // --- Test Cases for Rule Enforcement ---
    struct RuleEnforcementTestCase {
        std::string name;
        std::string text;
        std::vector<std::string> expected_issues; // e.g., "repetition", "capitalization"
    };

    std::vector<RuleEnforcementTestCase> rule_enforcement_test_cases = {
        {
            "Basic repetition and capitalization",
            "this is a test. it has some repeated words. words words. and missing capitalization",
            {"repetition", "capitalization"}
        },
        {
            "Multiple repetitions",
            "the the the cat sat on the mat mat mat. the dog dog ran quickly quickly quickly",
            {"repetition"}
        },
        {
            "Clean text",
            "This is a well-formatted sentence with proper capitalization. It should require minimal cleaning.",
            {}
        }
    };

    std::vector<float> strictness_levels = {0.3f, 0.7f, 1.0f};

    // --- Test Rule Enforcement on Predefined Test Cases ---
    std::cout << "\n--- Testing Rule Enforcement on Predefined Text ---" << std::endl;
    for (const auto& test_case : rule_enforcement_test_cases) {
        std::cout << "\n  Test Case: " << test_case.name << std::endl;
        std::cout << "    Original: \"" << test_case.text.substr(0, 80) << (test_case.text.length() > 80 ? "..." : "") << "\"" << std::endl;
        TextQualityAnalysis original_analysis = analyze_text_quality(test_case.text);

        for (float strictness : strictness_levels) {
            RuleEnforcer enforcer(strictness);
            std::string cleaned_text = enforcer.apply_rules(test_case.text);
            TextQualityAnalysis cleaned_analysis = analyze_text_quality(cleaned_text);

            std::cout << "      Strictness " << strictness << ": \"" << cleaned_text.substr(0, 80) << (cleaned_text.length() > 80 ? "..." : "") << "\"" << std::endl;
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "        Repetition reduction: " << (original_analysis.repetition_ratio - cleaned_analysis.repetition_ratio) << std::endl;
            std::cout << "        Capitalization improvement: " << (cleaned_analysis.capitalization_ratio - original_analysis.capitalization_ratio) << std::endl;
        }
    }

    // --- Test Rule Enforcement on Generated Text ---
    std::cout << "\n--- Testing Rule Enforcement on Generated Text ---" << std::endl;
    std::vector<std::string> generation_prompts = {
        "The quick brown fox",
        "Artificial intelligence will"
    };
    int generation_length = 30;
    Generation::GenerationConfig gen_config = Generation::GenerationConfig::nucleus(0.9f, 0.8f);

    for (const std::string& prompt : generation_prompts) {
        std::cout << "\n  Prompt: \"" << prompt << "\"" << std::endl;
        // Generator generator(model, gen_config);
        std::string generated_text = generate_with_model(model, tokenizer, prompt, generation_length, gen_config);
        TextQualityAnalysis original_gen_analysis = analyze_text_quality(generated_text);

        std::cout << "    Original Generated: \"" << generated_text.substr(0, 80) << (generated_text.length() > 80 ? "..." : "") << "\"" << std::endl;
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "      Original Repetition: " << original_gen_analysis.repetition_ratio << ", Capitalization: " << original_gen_analysis.capitalization_ratio << std::endl;

        for (float strictness : strictness_levels) {
            RuleEnforcer enforcer(strictness);
            std::string cleaned_text = enforcer.apply_rules(generated_text);
            TextQualityAnalysis cleaned_gen_analysis = analyze_text_quality(cleaned_text);

            std::cout << "      Strictness " << strictness << ": \"" << cleaned_text.substr(0, 80) << (cleaned_text.length() > 80 ? "..." : "") << "\"" << std::endl;
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "        Repetition reduction: " << (original_gen_analysis.repetition_ratio - cleaned_gen_analysis.repetition_ratio) << std::endl;
            std::cout << "        Capitalization improvement: " << (cleaned_gen_analysis.capitalization_ratio - original_gen_analysis.capitalization_ratio) << std::endl;
        }
    }

    std::cout << "\n=== Rule Enforcement Evaluation (C++) Completed ===" << std::endl;
}

int main() {
    try {
        run_rule_enforcement_evaluation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Rule Enforcement Evaluation failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
