/**
 * @file hash_embedder.cpp
 * @brief Implementation of a hash-based deterministic embedding generator.
 */

#include "hash_embedder.h"
#include <functional>

namespace TissDB {
namespace TissLM {
namespace Core {

HashEmbedder::HashEmbedder(size_t embedding_dim)
    : embedding_dim_(embedding_dim) {
    rng_.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

TissNum::Matrix HashEmbedder::embed(const std::string& text) {
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

}
}
}
