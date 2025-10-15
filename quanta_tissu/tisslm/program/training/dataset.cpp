#include "dataset.h"

namespace TissDB {
namespace TissLM {
namespace Training {

TokenDataset::TokenDataset(const std::vector<int>& token_ids, int seq_len)
    : token_ids_(token_ids), seq_len_(seq_len) {
}

size_t TokenDataset::size() const {
    if (token_ids_.size() <= seq_len_) {
        return 0;
    }
    return token_ids_.size() - seq_len_;
}

std::pair<TissNum::Matrix, TissNum::Matrix> TokenDataset::get_item(size_t index) const {
    TissNum::Matrix x(1, seq_len_);
    TissNum::Matrix y(1, seq_len_);

    for (int i = 0; i < seq_len_; ++i) {
        x(0, i) = static_cast<float>(token_ids_[index + i]);
        y(0, i) = static_cast<float>(token_ids_[index + i + 1]);
    }

    return {x, y};
}

} // namespace Training
} // namespace TissLM
} // namespace TissDB
