#pragma once

#include <vector>
#include "../core/matrix.h"

namespace TissLM {
namespace Training {

class TokenDataset {
public:
    TokenDataset(const std::vector<int>& token_ids, int seq_len);

    size_t size() const;
    std::pair<TissNum::Matrix, TissNum::Matrix> get_item(size_t index) const;

private:
    const std::vector<int>& token_ids_;
    int seq_len_;
};

} // namespace Training
} // namespace TissLM
