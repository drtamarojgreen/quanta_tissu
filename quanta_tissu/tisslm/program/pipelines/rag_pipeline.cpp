#include "rag_pipeline.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace Pipeline {

RAGPipeline::RAGPipeline(const RAGConfig& config)
    : config_(config), cache_valid_(false),
      total_queries_(0), total_retrievals_(0), cache_hits_(0) {
    retrieval_strategy_ = std::make_shared<Retrieval::CosineSimilarityStrategy>();
}

bool RAGPipeline::initialize() {
    try {
        db_client_ = std::make_unique<TissDB::TissDBClient>(
            config_.db_host, config_.db_port, config_.db_name);
        if (!db_client_->test_connection()) return false;
        std::vector<std::string> collections = {config_.collection_name};
        db_client_->ensure_db_setup(collections);
        load_documents_from_db();
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

std::string RAGPipeline::add_document(const std::string& content,
                                      const std::vector<float>& embedding,
                                      const std::map<std::string, std::string>& metadata) {
    TissDB::Document doc;
    doc.set_field("content", content);
    
    // Serialize embedding as a comma-separated string
    std::ostringstream oss;
    for (size_t i = 0; i < embedding.size(); ++i) {
        oss << embedding[i] << (i == embedding.size() - 1 ? "" : ",");
    }
    doc.set_field("embedding", oss.str());
    doc.set_field("embedding_dim", std::to_string(embedding.size()));

    for (const auto& pair : metadata) doc.set_field(pair.first, pair.second);
    std::string doc_id = db_client_->add_document(config_.collection_name, doc);
    
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
    if (top_k == 0) top_k = config_.top_k;
    if (!cache_valid_) load_documents_from_db();
    if (document_cache_.empty()) return {};
    auto doc_embeddings = get_all_embeddings();
    auto similarities = retrieval_strategy_->calculate_similarity(query_embedding, doc_embeddings);
    std::vector<RetrievalResult> results;
    for (size_t i = 0; i < document_cache_.size(); ++i) {
        if (similarities[i] >= config_.similarity_threshold) {
            results.emplace_back(document_cache_[i], similarities[i]);
        }
    }
    std::sort(results.begin(), results.end());
    if (results.size() > top_k) results.resize(top_k);
    return results;
}

std::vector<RetrievalResult> RAGPipeline::retrieve_by_text(const std::string& query_text,
                                                           size_t top_k) {
    total_retrievals_++;
    if (top_k == 0) top_k = config_.top_k;
    if (!cache_valid_) load_documents_from_db();
    if (document_cache_.empty()) return {};
    std::vector<std::string> corpus;
    for (const auto& doc : document_cache_) corpus.push_back(doc.content);
    Retrieval::BM25RetrievalStrategy bm25(corpus);
    std::vector<float> empty_embedding;
    std::vector<std::vector<float>> empty_docs;
    std::map<std::string, std::string> kwargs;
    kwargs["query_text"] = query_text;
    auto scores = bm25.calculate_similarity(empty_embedding, empty_docs, kwargs);
    std::vector<RetrievalResult> results;
    for (size_t i = 0; i < document_cache_.size(); ++i) {
        if (scores[i] >= config_.similarity_threshold) {
            results.emplace_back(document_cache_[i], scores[i]);
        }
    }
    std::sort(results.begin(), results.end());
    if (results.size() > top_k) results.resize(top_k);
    return results;
}

std::string RAGPipeline::generate_with_context(const std::string& query,
                                               const std::vector<float>& /*query_embedding*/,
                                               const std::vector<RetrievalResult>& context) {
    std::string formatted_context = format_context(context);
    std::ostringstream prompt;
    prompt << "Context:\n" << formatted_context << "\n\nQuery: " << query << "\n\nAnswer based on the context above:\n";
    if (model_) {
        // QuantaTissu::generate uses size_t for max_len
        return model_->generate(prompt.str(), (size_t)100);
    }
    return "Generated response based on " + std::to_string(context.size()) + " retrieved documents (model not initialized).";
}

std::string RAGPipeline::query(const std::string& query,
                               const std::vector<float>& query_embedding) {
    total_queries_++;
    auto results = retrieve(query_embedding);
    return generate_with_context(query, query_embedding, results);
}

void RAGPipeline::set_retrieval_strategy(std::shared_ptr<Retrieval::RetrievalStrategy> strategy) {
    retrieval_strategy_ = strategy;
}

void RAGPipeline::set_model(std::shared_ptr<QuantaTissu> qt) {
    model_ = qt;
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
    if (!db_client_) return;
    try {
        auto docs = db_client_->search_documents(config_.collection_name, "{}");
        document_cache_.clear();
        for (const auto& doc : docs) {
            DocumentWithEmbedding doc_with_emb(doc.id, "");
            for (const auto& elem : doc.elements) {
                if (!std::holds_alternative<std::string>(elem.value)) continue;
                std::string val = std::get<std::string>(elem.value);

                if (elem.key == "content") {
                    doc_with_emb.content = val;
                } else if (elem.key == "embedding") {
                    std::stringstream ss(val);
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        doc_with_emb.embedding.push_back(std::stof(item));
                    }
                } else {
                    doc_with_emb.metadata[elem.key] = val;
                }
            }
            document_cache_.push_back(doc_with_emb);
        }
        cache_valid_ = true;
    } catch (...) {
        cache_valid_ = false;
    }
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
    for (const auto& doc : document_cache_) embeddings.push_back(doc.embedding);
    return embeddings;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_database(const std::string& host, int port, const std::string& db_name) {
    config_.db_host = host; config_.db_port = port; config_.db_name = db_name;
    return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_collection(const std::string& collection_name) {
    config_.collection_name = collection_name; return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_top_k(size_t top_k) {
    config_.top_k = top_k; return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_similarity_threshold(float threshold) {
    config_.similarity_threshold = threshold; return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_hybrid_retrieval(bool use_hybrid) {
    config_.use_hybrid_retrieval = use_hybrid; return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_retrieval_strategy(std::shared_ptr<Retrieval::RetrievalStrategy> strategy) {
    strategy_ = strategy; return *this;
}

RAGPipelineBuilder& RAGPipelineBuilder::with_model(std::shared_ptr<QuantaTissu> qt) {
    qt_model_ = qt; return *this;
}

std::unique_ptr<RAGPipeline> RAGPipelineBuilder::build() {
    auto pipeline = std::make_unique<RAGPipeline>(config_);
    if (strategy_) pipeline->set_retrieval_strategy(strategy_);
    if (qt_model_) pipeline->set_model(qt_model_);
    return pipeline;
}

}
