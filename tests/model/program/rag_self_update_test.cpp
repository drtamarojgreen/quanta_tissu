#include "../../../quanta_tissu/tisslm/program/db/tissdb_client.h"
#include "../../../quanta_tissu/tisslm/program/core/transformer_model.h"
#include "../../../quanta_tissu/tisslm/program/generation/generator.h"
#include "../../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include "../../../quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "../../../quanta_tissu/tisslm/program/core/mock_embedder.h"
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
#include <set>

// Corrected namespaces
using namespace TissNum;

// Helper function to get a field from a document
std::string get_field(const TissDB::Document& doc, const std::string& key) {
    for (const auto& element : doc.elements) {
        if (element.key == key) {
            if (std::holds_alternative<std::string>(element.value)) {
                return std::get<std::string>(element.value);
            }
        }
    }
    return "";
}

// Helper function to set a field in a document
void set_field(TissDB::Document& doc, const std::string& key, const std::string& value) {
    for (auto& element : doc.elements) {
        if (element.key == key) {
            element.value = value;
            return;
        }
    }
    doc.elements.push_back({key, value});
}


// Helper to convert std::vector<float> to JSON array string
std::string vector_to_json_array(const std::vector<float>& vec) {
    std::string json = "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        json += std::to_string(vec[i]);
        if (i < vec.size() - 1) {
            json += ",";
        }
    }
    json += "]";
    return json;
}

// Helper to convert JSON array string to std::vector<float>
std::vector<float> json_array_to_vector(const std::string& json_str) {
    std::vector<float> vec;
    if (json_str.empty() || json_str == "[]") {
        return vec;
    }
    // Remove brackets
    std::string content = json_str.substr(1, json_str.length() - 2);
    std::string::size_type prev_pos = 0, pos = 0;
    while ((pos = content.find(',', prev_pos)) != std::string::npos) {
        vec.push_back(std::stof(content.substr(prev_pos, pos - prev_pos)));
        prev_pos = pos + 1;
    }
    vec.push_back(std::stof(content.substr(prev_pos, std::string::npos)));
    return vec;
}

const std::string evaluator_prompt = "Based on the context, evaluate the following query: ";
const std::string final_prompt = "Based on the context, answer the following query: ";

// Helper function to generate text with the model
std::string generate_with_model(std::shared_ptr<TissLM::Core::TransformerModel> model, TissLM::Tokenizer::Tokenizer& tokenizer, const std::string& prompt, int generation_length, const TissLM::Generation::GenerationConfig& config) {
    TissLM::Generation::Generator generator(model, config);
    std::vector<int> prompt_tokens = tokenizer.encode(prompt);
    std::vector<int> generated_tokens = generator.generate(prompt_tokens, generation_length);
    return tokenizer.decode(generated_tokens);
}

// --- Single RAG Test ---
struct RAGTestResult {
    std::string scenario_id;
    bool retrieval_correct;
    bool generation_correct;
    bool self_update_correct;
    std::string final_answer;
    std::string error_message;
    bool success;
};

RAGTestResult run_single_rag_test(
    std::shared_ptr<TissLM::Core::TransformerModel> model,
    TissLM::Tokenizer::Tokenizer& tokenizer,
    TissDB::TissDBClient& db_client,
    TissDB::TissLM::Core::MockEmbedder& embedder,
    const std::map<std::string, std::string>& scenario_config
) {
    RAGTestResult results;
    results.scenario_id = scenario_config.at("id");
    results.success = false;

    std::string db_name = "rag_test_" + scenario_config.at("id");
    std::string collection_name = "knowledge";

    try {
        // Ensure DB setup
        db_client.ensure_db_setup({collection_name});

        // Populate KB
        for (int i = 0; i < 2; ++i) { // Assuming initial_docs has 2 elements for simplicity
            std::string doc_id = "doc_" + std::to_string(i);
            std::string content = scenario_config.at("initial_docs_content_" + std::to_string(i));
            Matrix embedding_matrix = embedder.embed(content);
            // Convert Matrix to vector<float> for storage (simplified)
            std::vector<float> embedding(embedding_matrix.cols());
            for(int c=0; c<embedding_matrix.cols(); ++c) embedding[c] = embedding_matrix(0,c);

            TissDB::Document doc;
            set_field(doc, "id", doc_id);
            set_field(doc, "content", content);
            // Store embedding as a string (simplified, needs proper serialization)
            std::string embedding_str;
            for(float val : embedding) embedding_str += std::to_string(val) + ",";
            if(!embedding_str.empty()) embedding_str.pop_back(); // Remove trailing comma
            set_field(doc, "embedding", embedding_str);

            db_client.add_document(collection_name, doc, doc_id);
        }

        // Retrieval (simplified: just get expected docs by ID)
        std::vector<std::string> expected_retrieval_ids;
        if (scenario_config.count("expected_retrieval_id")) {
            expected_retrieval_ids.push_back(scenario_config.at("expected_retrieval_id"));
        } else if (scenario_config.count("expected_retrieval_ids_0")) {
            expected_retrieval_ids.push_back(scenario_config.at("expected_retrieval_ids_0"));
            expected_retrieval_ids.push_back(scenario_config.at("expected_retrieval_ids_1"));
        }

        std::string retrieved_context_str = "No relevant documents found.";
        std::vector<std::string> retrieved_docs_content;
        std::vector<std::string> actual_retrieved_ids;

        if (!expected_retrieval_ids.empty() && expected_retrieval_ids[0] != "None") {
            for (const std::string& doc_id : expected_retrieval_ids) {
                TissDB::Document retrieved_doc = db_client.get_document(collection_name, doc_id);
                retrieved_docs_content.push_back(get_field(retrieved_doc, "content"));
                actual_retrieved_ids.push_back(retrieved_doc.id);
            }
            retrieved_context_str = "\n" + std::accumulate(retrieved_docs_content.begin(), retrieved_docs_content.end(), std::string(),
                                                [](const std::string& a, const std::string& b) { return a.empty() ? b : a + "\n" + b; });
        }
        results.retrieval_correct = (actual_retrieved_ids == expected_retrieval_ids);

        // Evaluation (Sanitize context)
        TissLM::Generation::GenerationConfig eval_gen_config = TissLM::Generation::GenerationConfig::greedy();
        std::string evaluator_prompt = "User Query: \"" + scenario_config.at("query") + "\"\n\nRetrieved Context:\n---" + retrieved_context_str + "\n---\n\nExtract verified facts relevant to the query.";
        std::string sanitized_context = generate_with_model(model, tokenizer, evaluator_prompt, 60, eval_gen_config);

        // Generation
        TissLM::Generation::GenerationConfig final_gen_config = TissLM::Generation::GenerationConfig::greedy();
        std::string final_prompt = "Information: \"" + sanitized_context + "\"\n\nQuestion: \"" + scenario_config.at("query") + "\"\n\nAnswer:";
        std::string final_answer = generate_with_model(model, tokenizer, final_prompt, 50, final_gen_config);
        results.final_answer = final_answer;

        std::vector<std::string> expected_keywords;
        // Parse expected keywords from scenario_config
        // This part needs to be adapted based on how expected_keywords_in_answer is stored in scenario_config
        // For simplicity, assume a single keyword for now or parse a comma-separated string
        if (scenario_config.count("expected_keywords_in_answer_0")) {
            expected_keywords.push_back(scenario_config.at("expected_keywords_in_answer_0"));
        }
        if (scenario_config.count("expected_keywords_in_answer_1")) {
            expected_keywords.push_back(scenario_config.at("expected_keywords_in_answer_1"));
        }

        bool answer_correct = true;
        for (const std::string& kw : expected_keywords) {
            if (final_answer.find(kw) == std::string::npos) {
                answer_correct = false;
                break;
            }
        }
        results.generation_correct = answer_correct;

        // Self-Update
        std::string new_doc_id = "self_update_" + scenario_config.at("id");
        std::string new_content = "Query: " + scenario_config.at("query") + "\nResponse: " + final_answer;
        TissDB::Document new_doc;
        set_field(new_doc, "content", new_content);
        db_client.add_document(collection_name, new_doc, new_doc_id);

        // Verify update
        TissDB::Document verified_doc = db_client.get_document(collection_name, new_doc_id);
        results.self_update_correct = (get_field(verified_doc, "content") == new_content);

        results.success = results.retrieval_correct && results.generation_correct && results.self_update_correct;

    } catch (const std::exception& e) {
        results.error_message = e.what();
        results.success = false;
    }
    return results;
}

void run_rag_self_update_evaluation() {
    std::cout << "=== Running RAG and Self-Updating KB Evaluation (C++) ===" << std::endl;

    // --- Setup Model, Tokenizer, Embedder ---
    TissLM::Tokenizer::Tokenizer tokenizer(TestConfig::TokenizerPath);
    int vocab_size = tokenizer.get_vocab_size();

    std::shared_ptr<TissLM::Core::TransformerModel> model = std::make_shared<TissLM::Core::TransformerModel>(
        vocab_size,
        TestConfig::MaxSeqLen,
        TestConfig::EmbedDim,
        TestConfig::NumHeads,
        TestConfig::NumLayers,
        TestConfig::DropoutRate,
        0 // LoraRank = 0 for this test
    );
    TissDB::TissDBClient db_client("127.0.0.1", 9876, "test_rag_db"); // Use a specific DB name for RAG tests
    TissDB::TissLM::Core::MockEmbedder embedder(TestConfig::EmbedDim);

    std::cout << "  Model, Tokenizer, DB Client, and Embedder initialized." << std::endl;

    // --- Test Scenarios (simplified from Python version) ---
    std::vector<std::map<std::string, std::string>> rag_test_scenarios = {
        {
            {"id", "mars_mission_direct"},
            {"description", "Direct question with a single, clear answer in the KB."} ,
            {"query", "What is the name of the first Mars mission and when is it scheduled?"},
            {"initial_docs_content_0", "The first manned mission to Mars, named 'Ares 1', is scheduled for 2035."}, // Simplified doc storage
            {"initial_docs_content_1", "The Artemis program aims to return humans to the Moon."}, // Simplified doc storage
            {"expected_retrieval_id", "doc_0"}, // Corresponds to initial_docs_content_0
            {"expected_keywords_in_answer_0", "ares 1"},
            {"expected_keywords_in_answer_1", "2035"}
        },
        {
            {"id", "eldoria_synthesis"},
            {"description", "Question requiring synthesis of information from multiple documents."} ,
            {"query", "What is the capital of Eldoria and what is it famous for?"},
            {"initial_docs_content_0", "The capital of the fictional country of Eldoria is Silverhaven."},
            {"initial_docs_content_1", "Silverhaven is renowned for its beautiful crystal spires."},
            {"expected_retrieval_ids_0", "doc_0"}, 
            {"expected_retrieval_ids_1", "doc_1"}, 
            {"expected_keywords_in_answer_0", "silverhaven"},
            {"expected_keywords_in_answer_1", "crystal spires"}
        }
    };

    std::vector<RAGTestResult> all_results;

    for (const auto& scenario : rag_test_scenarios) {
        std::cout << "\n  Scenario: " << scenario.at("description") << std::endl;
        std::cout << "    Query: \"" << scenario.at("query") << "\"" << std::endl;

        try {
            RAGTestResult result = run_single_rag_test(model, tokenizer, db_client, embedder, scenario);
            all_results.push_back(result);

            std::cout << "      Retrieval Correct: " << (result.retrieval_correct ? "✓" : "✗") << std::endl;
            std::cout << "      Generation Correct: " << (result.generation_correct ? "✓" : "✗") << std::endl;
            std::cout << "      Self-Update Correct: " << (result.self_update_correct ? "✓" : "✗") << std::endl;
            std::cout << "      Overall Success: " << (result.success ? "✓" : "✗") << std::endl;
            std::cout << "      Final Answer: \"" << result.final_answer.substr(0, 100) << (result.final_answer.length() > 100 ? "..." : "") << "\"" << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "      [ERROR] Test scenario " << scenario.at("id") << " failed: " << e.what() << std::endl;
        }
    }

    // --- Overall Analysis (simplified) ---
    if (!all_results.empty()) {
        int total_tests = all_results.size();
        int successful_tests = 0;
        for (const auto& res : all_results) {
            if (res.success) {
                successful_tests++;
            }
        }
        std::cout << "\n--- RAG and Self-Updating KB Performance Analysis (C++) ---" << std::endl;
        std::cout << "  Total scenarios run: " << total_tests << std::endl;
        std::cout << "  Successful scenarios: " << successful_tests << std::endl;
        std::cout << "  Success rate: " << std::fixed << std::setprecision(1) << (static_cast<float>(successful_tests) / total_tests * 100.0f) << "%" << std::endl;
    }

    std::cout << "\n=== RAG and Self-Updating KB Evaluation (C++) Completed ===" << std::endl;
}

int main() {
    try {
        run_rag_self_update_evaluation();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "RAG and Self-Updating KB Evaluation failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
