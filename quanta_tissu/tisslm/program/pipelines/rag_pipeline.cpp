#include "rag_pipeline.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace Pipeline {

// RAGPipeline implementation
RAGPipeline::RAGPipeline(const RAGConfig& config)
    : config_(config), cache_valid_(false),
      total_queries_(0), total_retrievals_(0), cache_hits_(0) {
    
    // Initialize with cosine similarity by default
    retrieval_strategy_ = std::make_shared<Retrieval::CosineSimilarityStrategy>();
}

bool RAGPipeline::initialize() {
    try {
        // Create database client
        db_client_ = std::make_unique<TissDB::TissDBClient>(
            config_.db_host, config_.db_port, config_.db_name);
        
        // Test connection
        if (!db_client_->test_connection()) {
            return false;
        }
        
        // Ensure database and collection exist
        std::vector<std::string> collections = {config_.collection_name};
        db_client_->ensure_db_setup(collections);
        
        // Load existing documents
        load_documents_from_db();
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string RAGPipeline::add_document(const std::string& content,
                                      const std::vector<float>& embedding,
                                      const std::map<std::string, std::string>& metadata) {
    // Create document for database
    TissDB::Document doc;
    doc.set_field("content", content);
    doc.set_field("embedding_dim", std::to_string(embedding.size()));
    
    // Add metadata
    for (const auto& pair : metadata) {
        doc.set_field(pair.first, pair.second);
    }
    
    // Store in database
    std::string doc_id = db_client_->add_document(config_.collection_name, doc);
    
    // Add to cache
    DocumentWithEmbedding doc_with_emb(doc_id, content);
    doc_with_emb.embedding = embedding;
    doc_with_emb.metadata = metadata;
    document_cache_.push_back(doc_with_emb);
    
    cache_valid_ = true;
    
    return doc_id;
}

std::vector<RetrievalResult> RAGPipeline::retrieve(const std::vector<float>& query_embedding,
                                                   size_t top_k) {
    total_retrievals_++;
    
    if (top_k == 0) {
        top_k = config_.top_k;
    }
    
    // Ensure cache is loaded
    if (!cache_valid_) {
        load_documents_from_db();
    }
    
    if (document_cache_.empty()) {
        return {};
    }
    
    // Get all document embeddings
    auto doc_embeddings = get_all_embeddings();
    
    // Calculate similarities
    auto similarities = retrieval_strategy_->calculate_similarity(
        query_embedding, doc_embeddings);
    
    // Create results with scores
    std::vector<RetrievalResult> results;
    for (size_t i = 0; i < document_cache_.size(); ++i) {
        if (similarities[i] >= config_.similarity_threshold) {
            results.emplace_back(document_cache_[i], similarities[i]);
        }
    }
    
    // Sort by score (descending)
    std::sort(results.begin(), results.end());
    
    // Return top-k results
    if (results.size() > top_k) {
        results.resize(top_k);
    }
    
    return results;
}

std::vector<RetrievalResult> RAGPipeline::retrieve_by_text(const std::string& query_text,
                                                           size_t top_k) {
    total_retrievals_++;
    
    if (top_k == 0) {
        top_k = config_.top_k;
    }
    
    // Ensure cache is loaded
    if (!cache_valid_) {
        load_documents_from_db();
    }
    
    if (document_cache_.empty()) {
        return {};
    }
    
    // For text-based retrieval, we need BM25 or similar
    // Create a corpus from cached documents
    std::vector<std::string> corpus;
    for (const auto& doc : document_cache_) {
        corpus.push_back(doc.content);
    }
    
    // Use BM25 strategy
    Retrieval::BM25RetrievalStrategy bm25(corpus);
    
    std::vector<float> dummy_embedding;
    std::vector<std::vector<float>> dummy_docs;
    std::map<std::string, std::string> kwargs;
    kwargs["query_text"] = query_text;
    
    auto scores = bm25.calculate_similarity(dummy_embedding, dummy_docs, kwargs);
    
    // Create results
    std::vector<RetrievalResult> results;
    for (size_t i = 0; i < document_cache_.size(); ++i) {
        if (scores[i] >= config_.similarity_threshold) {
            results.emplace_back(document_cache_[i], scores[i]);
        }
    }
    
    // Sort and limit
    std::sort(results.begin(), results.end());
    if (results.size() > top_k) {
        results.resize(top_k);
    }
    
    return results;
}

std::string RAGPipeline::generate_with_context(const std::string& query,
                                               const std::vector<float>& query_embedding,
                                               const std::vector<RetrievalResult>& context) {
    // Format context
    std::string formatted_context = format_context(context);
    
    // Create prompt with context
    std::ostringstream prompt;
    prompt << "Context:\n" << formatted_context << "\n\n";
    prompt << "Query: " << query << "\n\n";
    prompt << "Answer based on the context above:\n";
    
    // In a full implementation, this would call the language model
    // For now, return a placeholder
    return "Generated response based on " + std::to_string(context.size()) + " retrieved documents.";
}

std::string RAGPipeline::query(const std::string& query,
                               const std::vector<float>& query_embedding) {
    total_queries_++;
    
    // Retrieve relevant documents
    auto results = retrieve(query_embedding);
    
    // Generate response with context
    return generate_with_context(query, query_embedding, results);
}

void RAGPipeline::set_retrieval_strategy(std::shared_ptr<Retrieval::RetrievalStrategy> strategy) {
    retrieval_strategy_ = strategy;
}

std::map<std::string, std::string> RAGPipeline::get_statistics() const {
    std::map<std::string, std::string> stats;
    stats["total_queries"] = std::to_string(total_queries_);
    stats["total_retrievals"] = std::to_string(total_retrievals_);
    stats["cache_hits"] = std::to_string(cache_hits_);
    stats["cached_documents"] = std::to_string(document_cache_.size());
    stats["cache_valid"] = cache_valid_ ? "true" : "false";
    return stats;
}

void RAGPipeline::clear_cache() {
    document_cache_.clear();
    cache_valid_ = false;
}

void RAGPipeline::load_documents_from_db() {
    // In a full implementation, this would query the database
    // For now, we rely on the cache being populated via add_document
    cache_valid_ = true;
}

std::string RAGPipeline::format_context(const std::vector<RetrievalResult>& results) const {
    std::ostringstream oss;
    
    for (size_t i = 0; i < results.size(); ++i) {
        oss << "[Document " << (i + 1) << " (score: " << results[i].score << ")]" << std::endl;
        oss << results[i].document.content << std::endl << std::endl;
    }
    
    return oss.str();
}

std::vector<std::vector<float>> RAGPipeline::get_all_embeddings() const {
    std::vector<std::vector<float>> embeddings;
    embeddings.reserve(document_cache_.size());
    
    for (const auto& doc : document_cache_) {
        embeddings.push_back(doc.embedding);
    }
    
    return embeddings;
}

// RAGPipelineBuilder implementation
RAGPipelineBuilder& RAGPipelineBuilder::with_database(const std::string& host, int port, const std::string& db_name) {
    config_.db_host = host;
    config_.db_port = port;
    config_.db_name = db_name;
    return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_collection(const std::string& collection_name) {
    config_.collection_name = collection_name;
    return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_top_k(size_t top_k) {
    config_.top_k = top_k;
    return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_similarity_threshold(float threshold) {
    config_.similarity_threshold = threshold;
    return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_hybrid_retrieval(bool use_hybrid) {
    config_.use_hybrid_retrieval = use_hybrid;
    return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_retrieval_strategy(std::shared_ptr<Retrieval::RetrievalStrategy> strategy) {
    strategy_ = strategy;
    return *this;
}

std::unique_ptr<RAGPipeline> RAGPipelineBuilder::build() {
    auto pipeline = std::make_unique<RAGPipeline>(config_);
    
    if (strategy_) {
        pipeline->set_retrieval_strategy(strategy_);
    }
    
    return pipeline;
}

} // namespace Pipeline
