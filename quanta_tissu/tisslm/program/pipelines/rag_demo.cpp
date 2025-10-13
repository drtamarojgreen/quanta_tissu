#include "rag_pipeline.h"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace Pipeline;

void print_separator() {
    std::cout << std::string(70, '=') << std::endl;
}

void demo_basic_rag() {
    std::cout << "\n=== Basic RAG Pipeline Demo ===" << std::endl;
    print_separator();

    // Create pipeline with builder
    auto pipeline = RAGPipelineBuilder()
        .with_database("127.0.0.1", 9876, "rag_demo_db")
        .with_collection("knowledge_base")
        .with_top_k(3)
        .with_similarity_threshold(0.1f)
        .build();

    // Initialize
    std::cout << "Initializing pipeline..." << std::endl;
    if (!pipeline->initialize()) {
        std::cerr << "Failed to initialize pipeline!" << std::endl;
        return;
    }
    std::cout << "✓ Pipeline initialized" << std::endl;

    // Add some documents with embeddings
    std::cout << "\nAdding documents to knowledge base..." << std::endl;

    std::vector<std::pair<std::string, std::vector<float>>> documents = {
        {"The capital of France is Paris.", {0.8f, 0.2f, 0.1f}},
        {"Python is a programming language.", {0.1f, 0.9f, 0.2f}},
        {"Machine learning is a subset of AI.", {0.2f, 0.8f, 0.7f}},
        {"Paris is known for the Eiffel Tower.", {0.7f, 0.3f, 0.2f}},
        {"C++ is used for system programming.", {0.1f, 0.85f, 0.3f}}
    };

    for (const auto& doc : documents) {
        std::string doc_id = pipeline->add_document(doc.first, doc.second);
        std::cout << "  Added: " << doc.first.substr(0, 40) << "... [ID: "
                  << doc_id.substr(0, 8) << "...]" << std::endl;
    }

    // Query the pipeline
    std::cout << "\n--- Querying Pipeline ---" << std::endl;
    std::vector<float> query_embedding = {0.75f, 0.25f, 0.15f};  // Similar to France/Paris docs

    std::cout << "Query embedding: [" << query_embedding[0] << ", "
              << query_embedding[1] << ", " << query_embedding[2] << "]" << std::endl;

    auto results = pipeline->retrieve(query_embedding, 3);

    std::cout << "\nTop " << results.size() << " Retrieved Documents:" << std::endl;
    print_separator();

    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "\n[" << (i + 1) << "] Score: " << std::fixed << std::setprecision(4)
                  << results[i].score << std::endl;
        std::cout << "Content: " << results[i].document.content << std::endl;
    }

    // Show statistics
    std::cout << "\n--- Pipeline Statistics ---" << std::endl;
    auto stats = pipeline->get_statistics();
    for (const auto& pair : stats) {
        std::cout << "  " << pair.first << ": " << pair.second << std::endl;
    }

    print_separator();
}

void demo_text_retrieval() {
    std::cout << "\n=== Text-Based Retrieval Demo (BM25) ===" << std::endl;
    print_separator();

    auto pipeline = RAGPipelineBuilder()
        .with_database("127.0.0.1", 9876, "rag_demo_db")
        .with_collection("text_docs")
        .with_top_k(2)
        .build();

    if (!pipeline->initialize()) {
        std::cerr << "Failed to initialize pipeline!" << std::endl;
        return;
    }

    // Add documents (embeddings can be empty for text-only retrieval)
    std::cout << "Adding text documents..." << std::endl;

    std::vector<std::string> texts = {
        "The quick brown fox jumps over the lazy dog",
        "Machine learning algorithms can learn from data",
        "Natural language processing is a branch of AI",
        "The fox is a clever animal in many stories"
    };

    for (const auto& text : texts) {
        std::vector<float> dummy_embedding = {0.0f};  // Placeholder
        pipeline->add_document(text, dummy_embedding);
        std::cout << "  Added: " << text << std::endl;
    }

    // Query using text
    std::cout << "\n--- Text Query ---" << std::endl;
    std::string query = "fox animal";
    std::cout << "Query: \"" << query << "\"" << std::endl;

    auto results = pipeline->retrieve_by_text(query, 2);

    std::cout << "\nTop " << results.size() << " Results:" << std::endl;
    print_separator();

    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "\n[" << (i + 1) << "] Score: " << std::fixed << std::setprecision(4)
                  << results[i].score << std::endl;
        std::cout << "Content: " << results[i].document.content << std::endl;
    }

    print_separator();
}

void demo_hybrid_retrieval() {
    std::cout << "\n=== Hybrid Retrieval Demo ===" << std::endl;
    print_separator();

    // Create hybrid strategy
    auto hybrid = std::make_shared<Retrieval::HybridStrategy>();
    hybrid->add_strategy(std::make_shared<Retrieval::CosineSimilarityStrategy>(), 0.7f);
    hybrid->add_strategy(std::make_shared<Retrieval::EuclideanDistanceStrategy>(), 0.3f);

    auto pipeline = RAGPipelineBuilder()
        .with_database("127.0.0.1", 9876, "rag_demo_db")
        .with_collection("hybrid_docs")
        .with_top_k(3)
        .with_retrieval_strategy(hybrid)
        .build();

    if (!pipeline->initialize()) {
        std::cerr << "Failed to initialize pipeline!" << std::endl;
        return;
    }

    std::cout << "Using hybrid retrieval (70% Cosine + 30% Euclidean)" << std::endl;

    // Add documents
    std::vector<std::pair<std::string, std::vector<float>>> documents = {
        {"Document about technology and innovation", {0.9f, 0.1f, 0.2f}},
        {"Document about nature and wildlife", {0.1f, 0.9f, 0.3f}},
        {"Document about technology in nature", {0.5f, 0.5f, 0.4f}}
    };

    for (const auto& doc : documents) {
        pipeline->add_document(doc.first, doc.second);
    }

    // Query
    std::vector<float> query_embedding = {0.85f, 0.15f, 0.25f};
    auto results = pipeline->retrieve(query_embedding);

    std::cout << "\nRetrieved " << results.size() << " documents:" << std::endl;
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "  [" << (i + 1) << "] " << results[i].document.content
                  << " (score: " << results[i].score << ")" << std::endl;
    }

    print_separator();
}

void demo_full_rag_query() {
    std::cout << "\n=== Full RAG Query Demo ===" << std::endl;
    print_separator();

    auto pipeline = RAGPipelineBuilder()
        .with_database("127.0.0.1", 9876, "rag_demo_db")
        .with_collection("qa_docs")
        .with_top_k(2)
        .build();

    if (!pipeline->initialize()) {
        std::cerr << "Failed to initialize pipeline!" << std::endl;
        return;
    }

    // Add knowledge base
    std::vector<std::pair<std::string, std::vector<float>>> kb = {
        {"TissDB is a high-performance NoSQL database written in C++.", {0.8f, 0.3f, 0.1f}},
        {"TissLM is a language model for the QuantaTissu project.", {0.7f, 0.4f, 0.2f}},
        {"The RAG pipeline combines retrieval and generation.", {0.6f, 0.5f, 0.3f}}
    };

    for (const auto& doc : kb) {
        pipeline->add_document(doc.first, doc.second);
    }

    // Full RAG query
    std::string query_text = "What is TissDB?";
    std::vector<float> query_embedding = {0.75f, 0.35f, 0.15f};

    std::cout << "Query: \"" << query_text << "\"" << std::endl;
    std::cout << "\nProcessing..." << std::endl;

    std::string response = pipeline->query(query_text, query_embedding);

    std::cout << "\nGenerated Response:" << std::endl;
    std::cout << response << std::endl;

    print_separator();
}

int main() {
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "TissLM RAG Pipeline Demonstration" << std::endl;
    std::cout << "Connecting to TissDB on 127.0.0.1:9876" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    try {
        // Run demos
        demo_basic_rag();
        demo_text_retrieval();
        demo_hybrid_retrieval();
        demo_full_rag_query();

        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "All demos completed successfully!" << std::endl;
        std::cout << std::string(70, '=') << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
