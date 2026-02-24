#ifndef TISSLM_STELLAR_TOKENIZER_H
#define TISSLM_STELLAR_TOKENIZER_H

#include <string>
#include <vector>
#include <map>
#include <utility>

namespace TissLM {
namespace Stellar {

/**
 * @class StellarTokenizer
 * @brief A high-performance implementation of the TissLM BPE Tokenizer.
 *
 * Implements the core BPE logic while providing robust UTF-8 decoding
 * and visual training telemetry.
 */
class StellarTokenizer {
public:
    StellarTokenizer();

    // Core API parity
    std::vector<int> encode(const std::string& text);
    std::string decode(const std::vector<int>& token_ids);
    int get_vocab_size() const;
    void train(const std::string& text, int vocab_size, bool verbose = false);

    // Stellar Enhancements
    void train_stellar(const std::string& text, int vocab_size);
    std::string decode_robust(const std::vector<int>& token_ids) const;

private:
    std::map<std::pair<int, int>, int> merges_;
    std::vector<std::pair<int, int>> ranked_merges_;
    std::map<int, std::vector<unsigned char>> vocab_;
    std::map<std::vector<unsigned char>, int> reverse_vocab_;

    std::vector<int> bpe_encode(const std::vector<unsigned char>& bytes) const;
};

} // namespace Stellar
} // namespace TissLM

#endif // TISSLM_STELLAR_TOKENIZER_H
