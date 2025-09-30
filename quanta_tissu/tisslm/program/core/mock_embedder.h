#ifndef TISSLM_MOCK_EMBEDDER_H
#define TISSLM_MOCK_EMBEDDER_H

#include "matrix.h"
#include <string>
#include <vector>
#include <random>
#include <chrono>

namespace TissDB {
namespace TissLM {
namespace Core {

class MockEmbedder {
public:
    MockEmbedder(size_t embedding_dim = 128);
    Matrix embed(const std::string& text);

private:
    size_t embedding_dim_;
    std::mt19937 rng_;
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_MOCK_EMBEDDER_H
