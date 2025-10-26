#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <any>

namespace TissLM {
namespace Retrieval {

/**
 * Abstract base class for all retrieval similarity strategies
 */
class RetrievalStrategy {
public:
    virtual ~RetrievalStrategy() = default;
    
    /**
     * Calculate similarity between query embedding and document embeddings
     * @param query_embedding Query embedding vector
     * @param doc_embeddings List of document embedding vectors
     * @param kwargs Additional parameters for specific strategies
     * @return Vector of similarity scores
     */
    virtual std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) = 0;
    
    /**
     * Get the name of this strategy
     */
    virtual std::string get_name() const = 0;
};

/**
 * Cosine Similarity Strategy
 * Calculates similarity using cosine similarity between vectors
 */
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

/**
 * Euclidean Distance Strategy
 * Calculates similarity using inverse Euclidean distance
 */
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

/**
 * Dot Product Strategy
 * Calculates similarity using dot product (assumes normalized vectors)
 */
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

/**
 * BM25 Retrieval Strategy
 * Keyword-based search algorithm (does not use embeddings)
 */
class BM25RetrievalStrategy : public RetrievalStrategy {
public:
    /**
     * Constructor
     * @param corpus List of documents (as strings)
     * @param k1 BM25 k1 parameter (default: 1.5)
     * @param b BM25 b parameter (default: 0.75)
     */
    BM25RetrievalStrategy(const std::vector<std::string>& corpus, 
                         float k1 = 1.5f, float b = 0.75f);
    
    std::vector<float> calculate_similarity(
        const std::vector<float>& query_embedding,
        const std::vector<std::vector<float>>& doc_embeddings,
        const std::map<std::string, std::any>& kwargs = {}
    ) override;
    
    std::string get_name() const override { return "BM25"; }

private:
    float k1_;
    float b_;
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

/**
 * Hybrid Strategy
 * Combines multiple strategies with weighted scores
 */
class HybridStrategy : public RetrievalStrategy {
public:
    /**
     * Add a strategy with a weight
     * @param strategy Retrieval strategy
     * @param weight Weight for this strategy (default: 1.0)
     */
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
 * Bayesian Similarity Strategy
 * A simplified Bayesian-based similarity approach
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

} // namespace Retrieval
} // namespace TissLM
