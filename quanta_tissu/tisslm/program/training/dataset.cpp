#include "dataset.h"
#include <stdexcept>

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
    if (index + seq_len_ + 1 > token_ids_.size()) {
        throw std::out_of_range("Index out of range in TokenDataset::get_item");
    }

    TissNum::Matrix x({1, (size_t)seq_len_});
    TissNum::Matrix y({1, (size_t)seq_len_});

    for (size_t i = 0; i < seq_len_; ++i) {
        x({0, i}) = static_cast<float>(token_ids_[index + i]);
        y({0, i}) = static_cast<float>(token_ids_[index + i + 1]);
    }

    return {x, y};
}

} // namespace Training
} // namespace TissLM
