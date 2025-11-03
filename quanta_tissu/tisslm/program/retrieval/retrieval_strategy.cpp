#include "retrieval_strategy.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <numeric>
#include <set>
#include <any>
#include <random>

namespace TissLM {
namespace Retrieval {

// CosineSimilarityStrategy implementation
float CosineSimilarityStrategy::dot_product(const std::vector<float>& a, const std::vector<float>& b) const {
    if (a.size() != b.size()) {
        return 0.0f;
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

float CosineSimilarityStrategy::vector_norm(const std::vector<float>& vec) const {
    float sum = 0.0f;
    for (float val : vec) {
        sum += val * val;
    }
    return std::sqrt(sum);
}

float CosineSimilarityStrategy::cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) const {
    float norm_a = vector_norm(a);
    float norm_b = vector_norm(b);
    
    if (norm_a == 0.0f || norm_b == 0.0f) {
        return 0.0f;
    }
    
    return dot_product(a, b) / (norm_a * norm_b);
}

std::vector<float> CosineSimilarityStrategy::calculate_similarity(
    const std::vector<float>& query_embedding,
    const std::vector<std::vector<float>>& doc_embeddings,
    const std::map<std::string, std::any>& kwargs) {
    
    std::vector<float> similarities;
    similarities.reserve(doc_embeddings.size());
    
    for (const auto& doc_emb : doc_embeddings) {
        similarities.push_back(cosine_similarity(query_embedding, doc_emb));
    }
    
    return similarities;
}

// EuclideanDistanceStrategy implementation
float EuclideanDistanceStrategy::euclidean_distance(const std::vector<float>& a, const std::vector<float>& b) const {
    if (a.size() != b.size()) {
        return std::numeric_limits<float>::max();
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

std::vector<float> EuclideanDistanceStrategy::calculate_similarity(
    const std::vector<float>& query_embedding,
    const std::vector<std::vector<float>>& doc_embeddings,
    const std::map<std::string, std::any>& kwargs) {
    
    std::vector<float> similarities;
    similarities.reserve(doc_embeddings.size());
    
    for (const auto& doc_emb : doc_embeddings) {
        float distance = euclidean_distance(query_embedding, doc_emb);
        // Convert distance to similarity (inverse relationship)
        float similarity = 1.0f / (1.0f + distance);
        similarities.push_back(similarity);
    }
    
    return similarities;
}

// DotProductStrategy implementation
float DotProductStrategy::dot_product(const std::vector<float>& a, const std::vector<float>& b) const {
    if (a.size() != b.size()) {
        return 0.0f;
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

std::vector<float> DotProductStrategy::calculate_similarity(
    const std::vector<float>& query_embedding,
    const std::vector<std::vector<float>>& doc_embeddings,
    const std::map<std::string, std::any>& kwargs) {
    
    std::vector<float> similarities;
    similarities.reserve(doc_embeddings.size());
    
    for (const auto& doc_emb : doc_embeddings) {
        similarities.push_back(dot_product(query_embedding, doc_emb));
    }
    
    return similarities;
}

// BM25RetrievalStrategy implementation
BM25RetrievalStrategy::BM25RetrievalStrategy(const std::vector<std::string>& corpus, float k1, float b)
    : k1_(k1), b_(b), corpus_(corpus) {
    initialize();
}

std::vector<std::string> BM25RetrievalStrategy::tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    std::istringstream iss(text);
    std::string token;
    
    while (iss >> token) {
        // Simple tokenization - split by whitespace
        // Convert to lowercase
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        tokens.push_back(token);
    }
    
    return tokens;
}

void BM25RetrievalStrategy::initialize() {
    // Calculate document lengths
    size_t total_len = 0;
    for (const auto& doc : corpus_) {
        auto tokens = tokenize(doc);
        doc_len_.push_back(tokens.size());
        total_len += tokens.size();
    }
    
    avgdl_ = corpus_.empty() ? 0.0f : static_cast<float>(total_len) / corpus_.size();
    
    // Calculate document frequencies and term frequencies
    std::map<std::string, size_t> df;
    
    for (const auto& doc : corpus_) {
        auto tokens = tokenize(doc);
        std::map<std::string, size_t> term_freq;
        std::set<std::string> unique_terms;
        
        for (const auto& token : tokens) {
            term_freq[token]++;
            unique_terms.insert(token);
        }
        
        term_freqs_.push_back(term_freq);
        
        for (const auto& term : unique_terms) {
            df[term]++;
        }
    }
    
    // Calculate IDF
    size_t N = corpus_.size();
    for (const auto& pair : df) {
        float idf_val = std::log(1.0f + (N - pair.second + 0.5f) / (pair.second + 0.5f));
        idf_[pair.first] = idf_val;
    }
}

float BM25RetrievalStrategy::calculate_idf(const std::string& term) const {
    auto it = idf_.find(term);
    return (it != idf_.end()) ? it->second : 0.0f;
}

std::vector<float> BM25RetrievalStrategy::calculate_similarity(
    const std::vector<float>& query_embedding,
    const std::vector<std::vector<float>>& doc_embeddings,
    const std::map<std::string, std::any>& kwargs) {
    
    // BM25 requires query text, not embedding
    auto it = kwargs.find("query_text");
    if (it == kwargs.end()) {
        // Return zeros if no query text provided
        return std::vector<float>(corpus_.size(), 0.0f);
    }
    
    std::string query_text = std::any_cast<std::string>(it->second);
    auto query_terms = tokenize(query_text);
    
    std::vector<float> scores(corpus_.size(), 0.0f);
    
    for (size_t i = 0; i < corpus_.size(); ++i) {
        float score = 0.0f;
        
        for (const auto& term : query_terms) {
            auto tf_it = term_freqs_[i].find(term);
            if (tf_it != term_freqs_[i].end()) {
                float tf = static_cast<float>(tf_it->second);
                float idf = calculate_idf(term);
                
                float numerator = tf * (k1_ + 1.0f);
                float denominator = tf + k1_ * (1.0f - b_ + b_ * doc_len_[i] / avgdl_);
                
                score += idf * (numerator / denominator);
            }
        }
        
        scores[i] = score;
    }
    
    return scores;
}

// HybridStrategy implementation
void HybridStrategy::add_strategy(std::shared_ptr<RetrievalStrategy> strategy, float weight) {
    strategies_.push_back({strategy, weight});
}

std::vector<float> HybridStrategy::normalize_scores(const std::vector<float>& scores) const {
    if (scores.empty()) {
        return scores;
    }
    
    float min_score = *std::min_element(scores.begin(), scores.end());
    float max_score = *std::max_element(scores.begin(), scores.end());
    
    if (max_score == min_score) {
        return std::vector<float>(scores.size(), 0.5f);
    }
    
    std::vector<float> normalized;
    normalized.reserve(scores.size());
    
    for (float score : scores) {
        normalized.push_back((score - min_score) / (max_score - min_score));
    }
    
    return normalized;
}

std::vector<float> HybridStrategy::calculate_similarity(
    const std::vector<float>& query_embedding,
    const std::vector<std::vector<float>>& doc_embeddings,
    const std::map<std::string, std::any>& kwargs) {
    
    if (strategies_.empty() || doc_embeddings.empty()) {
        return std::vector<float>(doc_embeddings.size(), 0.0f);
    }
    
    // Initialize combined scores
    std::vector<float> combined_scores(doc_embeddings.size(), 0.0f);
    float total_weight = 0.0f;
    
    // Calculate scores for each strategy
    for (const auto& strategy_pair : strategies_) {
        auto strategy = strategy_pair.first;
        float weight = strategy_pair.second;
        
        auto scores = strategy->calculate_similarity(query_embedding, doc_embeddings, kwargs);
        auto normalized = normalize_scores(scores);
        
        // Add weighted scores
        for (size_t i = 0; i < combined_scores.size(); ++i) {
            combined_scores[i] += normalized[i] * weight;
        }
        
        total_weight += weight;
    }
    
    // Normalize by total weight
    if (total_weight > 0.0f) {
        for (float& score : combined_scores) {
            score /= total_weight;
        }
    }
    
    return combined_scores;
}

// BayesianSimilarityStrategy implementation
std::vector<float> BayesianSimilarityStrategy::calculate_similarity(
    const std::vector<float>& query_embedding,
    const std::vector<std::vector<float>>& doc_embeddings,
    const std::map<std::string, std::any>& kwargs) {

    auto it = kwargs.find("eigenvalues");
    if (it == kwargs.end()) {
        throw std::runtime_error("BayesianSimilarityStrategy requires 'eigenvalues' in kwargs.");
    }

    const auto& eigenvalues = std::any_cast<const std::vector<float>&>(it->second);

    std::vector<float> posterior_variance;
    if (eigenvalues.size() != query_embedding.size()) {
        float mean_uncertainty = 1.0f;
        if (!eigenvalues.empty()) {
            float sum = 0.0f;
            for(float val : eigenvalues) sum += val;
            mean_uncertainty = 1.0f / (sum / eigenvalues.size() + 1e-6f);
        }
        posterior_variance.assign(query_embedding.size(), mean_uncertainty);
    } else {
        for (float val : eigenvalues) {
            posterior_variance.push_back(1.0f / (val + 1e-6f));
        }
    }

    std::vector<float> noisy_query = query_embedding;

    // Use the provided RNG if available, otherwise create a local one.
    std::mt19937 local_gen(std::random_device{}());
    std::mt19937* gen_ptr = &local_gen;

    auto rng_it = kwargs.find("rng");
    if (rng_it != kwargs.end()) {
        try {
            gen_ptr = std::any_cast<std::mt19937*>(rng_it->second);
        } catch (const std::bad_any_cast& e) {
            // Do nothing, just use the local generator.
        }
    }

    for (size_t i = 0; i < noisy_query.size(); ++i) {
        float variance = std::max(posterior_variance[i], 1e-9f);
        std::normal_distribution<> d(0, std::sqrt(variance));
        noisy_query[i] += d(*gen_ptr);
    }

    CosineSimilarityStrategy cosine_strategy;
    return cosine_strategy.calculate_similarity(noisy_query, doc_embeddings, {});
}

} // namespace Retrieval
} // namespace TissLM
