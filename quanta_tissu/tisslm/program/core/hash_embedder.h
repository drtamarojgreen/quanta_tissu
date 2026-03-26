#ifndef TISSLM_HASH_EMBEDDER_H
#define TISSLM_HASH_EMBEDDER_H

#include "matrix.h"
#include <string>
#include <vector>
#include <random>
#include <chrono>

namespace TissDB {
namespace TissLM {
namespace Core {

/**
 * @brief Hash-based deterministic embedding generator.
 * Useful for consistent document representation in environments
 * without a pre-trained semantic model.
 */
class HashEmbedder {
public:
    HashEmbedder(size_t embedding_dim = 128);
    TissNum::Matrix embed(const std::string& text);

private:
    size_t embedding_dim_;
    std::mt19937 rng_;
};

}
}
}

#endif // TISSLM_HASH_EMBEDDER_H
