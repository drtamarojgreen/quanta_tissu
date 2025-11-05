#include "mock_embedder.h"
#include <functional> // For std::hash

namespace TissDB {
namespace TissLM {
namespace Core {

MockEmbedder::MockEmbedder(size_t embedding_dim)
    : embedding_dim_(embedding_dim) {
    // Initialize RNG with a seed based on current time or a fixed value for reproducibility
    rng_.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

TissNum::Matrix MockEmbedder::embed(const std::string& text) {
    // Use a simple hash-based deterministic mock embedding
    std::hash<std::string> hasher;
    size_t seed = hasher(text);
    rng_.seed(seed);

    TissNum::Matrix embedding({1, embedding_dim_});
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (size_t i = 0; i < embedding_dim_; ++i) {
        embedding({0, i}) = dist(rng_);
    }
    return embedding;
}

} // namespace Core
} // namespace TissLM
} // namespace TissDB
