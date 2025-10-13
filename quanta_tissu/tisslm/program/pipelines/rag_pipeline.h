#pragma once

#include "../db/tissdb_client.h"
#include "../retrieval/retrieval_strategy.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace Pipeline {

/**
 * Document with embedding for RAG pipeline
 */
struct DocumentWithEmbedding {
    std::string id;
    std::string content;
    std::vector<float> embedding;
    std::map<std::string, std::string> metadata;
    
    DocumentWithEmbedding() = default;
    DocumentWithEmbedding(const std::string& doc_id, const std::string& doc_content)
        : id(doc_id), content(doc_content) {}
};

/**
 * Result from retrieval with score
 */
struct RetrievalResult {
    DocumentWithEmbedding document;
    float score;
    
    RetrievalResult(const DocumentWithEmbedding& doc, float s)
        : document(doc), score(s) {}
    
    bool operator<(const RetrievalResult& other) const {
        return score > other.score;  // Higher scores first
    }
};

/**
 * Configuration for RAG pipeline
 */
struct RAGConfig {
    std::string db_host = "127.0.0.1";
    int db_port = 9876;
    std::string db_name = "rag_db";
    std::string collection_name = "documents";
    size_t top_k = 5;
    float similarity_threshold = 0.0f;
    bool use_hybrid_retrieval = false;
};

/**
 * RAG (Retrieval-Augmented Generation) Pipeline
 * Combines document retrieval with text generation
 */
class RAGPipeline {
public:
    /**
     * Constructor
     * @param config Pipeline configuration
     */
    explicit RAGPipeline(const RAGConfig& config = RAGConfig());
    
    /**
     * Initialize the pipeline
     * @return true if successful
     */
    bool initialize();
    
    /**
     * Add a document to the knowledge base
     * @param content Document content
     * @param embedding Document embedding vector
     * @param metadata Optional metadata
     * @return Document ID
     */
    std::string add_document(const std::string& content,
                            const std::vector<float>& embedding,
                            const std::map<std::string, std::string>& metadata = {});
    
    /**
     * Retrieve relevant documents for a query
     * @param query_embedding Query embedding vector
     * @param top_k Number of documents to retrieve
     * @return Vector of retrieval results
     */
    std::vector<RetrievalResult> retrieve(const std::vector<float>& query_embedding,
                                         size_t top_k = 0);
    
    /**
     * Retrieve relevant documents using text query (requires BM25)
     * @param query_text Query text
     * @param top_k Number of documents to retrieve
     * @return Vector of retrieval results
     */
    std::vector<RetrievalResult> retrieve_by_text(const std::string& query_text,
                                                  size_t top_k = 0);
    
    /**
     * Generate response using retrieved context
     * @param query Query text
     * @param query_embedding Query embedding
     * @param context Retrieved documents
     * @return Generated response
     */
    std::string generate_with_context(const std::string& query,
                                      const std::vector<float>& query_embedding,
                                      const std::vector<RetrievalResult>& context);
    
    /**
     * Complete RAG pipeline: retrieve and generate
     * @param query Query text
     * @param query_embedding Query embedding
     * @return Generated response with context
     */
    std::string query(const std::string& query,
                     const std::vector<float>& query_embedding);
    
    /**
     * Set the retrieval strategy
     * @param strategy Retrieval strategy to use
     */
    void set_retrieval_strategy(std::shared_ptr<Retrieval::RetrievalStrategy> strategy);
    
    /**
     * Get pipeline statistics
     * @return Statistics map
     */
    std::map<std::string, std::string> get_statistics() const;
    
    /**
     * Clear the document cache
     */
    void clear_cache();

private:
    RAGConfig config_;
    std::unique_ptr<TissDB::TissDBClient> db_client_;
    std::shared_ptr<Retrieval::RetrievalStrategy> retrieval_strategy_;
    
    // Cache for documents and embeddings
    std::vector<DocumentWithEmbedding> document_cache_;
    bool cache_valid_;
    
    // Statistics
    size_t total_queries_;
    size_t total_retrievals_;
    size_t cache_hits_;
    
    // Helper methods
    void load_documents_from_db();
    std::string format_context(const std::vector<RetrievalResult>& results) const;
    std::vector<std::vector<float>> get_all_embeddings() const;
};

/**
 * Pipeline builder for easy configuration
 */
class RAGPipelineBuilder {
public:
    RAGPipelineBuilder& with_database(const std::string& host, int port, const std::string& db_name);
    RAGPipelineBuilder& with_collection(const std::string& collection_name);
    RAGPipelineBuilder& with_top_k(size_t top_k);
    RAGPipelineBuilder& with_similarity_threshold(float threshold);
    RAGPipelineBuilder& with_hybrid_retrieval(bool use_hybrid);
    RAGPipelineBuilder& with_retrieval_strategy(std::shared_ptr<Retrieval::RetrievalStrategy> strategy);
    
    std::unique_ptr<RAGPipeline> build();

private:
    RAGConfig config_;
    std::shared_ptr<Retrieval::RetrievalStrategy> strategy_;
};

} // namespace Pipeline
