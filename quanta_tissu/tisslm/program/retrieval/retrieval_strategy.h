#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <any>

namespace TissLM {
namespace Retrieval {

/**
 * Abstract base class for all retrieval similarity strategies.
 */
class RetrievalStrategy {
public:
    virtual ~RetrievalStrategy() = default;
    
    /**
     * Calculate similarity between query embedding and document embeddings.
     */
    virtual std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) = 0;
    
    virtual std::string get_name() const = 0;
};

class CosineSimilarityStrategy : public RetrievalStrategy {
public:
    std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) override;
    
    std::string get_name() const override { return "CosineSimilarity"; }

private:
    float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) const;
    float vector_norm(const std::vector<float>& vec) const;
    float dot_product(const std::vector<float>& a, const std::vector<float>& b) const;
};

class EuclideanDistanceStrategy : public RetrievalStrategy {
public:
    std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) override;
    
    std::string get_name() const override { return "EuclideanDistance"; }

private:
    float euclidean_distance(const std::vector<float>& a, const std::vector<float>& b) const;
};

class DotProductStrategy : public RetrievalStrategy {
public:
    std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) override;
    
    std::string get_name() const override { return "DotProduct"; }

private:
    float dot_product(const std::vector<float>& a, const std::vector<float>& b) const;
};

class BM25RetrievalStrategy : public RetrievalStrategy {
public:
    BM25RetrievalStrategy(const std::vector<std::string>& corpus, 
                         float k1 = 1.5f, float b = 0.75f);
    
    std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) override;
    
    std::string get_name() const override { return "BM25"; }

private:
    float k1_, b_;
    std::vector<std::string> corpus_;
    std::vector<size_t> doc_len_;
    float avgdl_;
    std::map<std::string, size_t> doc_freqs_;
    std::map<std::string, float> idf_;
    std::vector<std::map<std::string, size_t>> term_freqs_;
    
    void initialize();
    std::vector<std::string> tokenize(const std::string& text) const;
    float calculate_idf(const std::string& term) const;
};

class HybridStrategy : public RetrievalStrategy {
public:
    void add_strategy(std::shared_ptr<RetrievalStrategy> strategy, float weight = 1.0f);
    
    std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) override;
    
    std::string get_name() const override { return "Hybrid"; }

private:
    std::vector<std::pair<std::shared_ptr<RetrievalStrategy>, float>> strategies_;
    std::vector<float> normalize_scores(const std::vector<float>& scores) const;
};

/**
 * @brief Bayesian Similarity Strategy.
 * Computes similarity based on posterior variance and eigenvalues.
 */
class BayesianSimilarityStrategy : public RetrievalStrategy {
public:
    std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) override;
    
    std::string get_name() const override { return "BayesianSimilarity"; }
};

}
}
