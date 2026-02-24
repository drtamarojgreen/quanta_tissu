#ifndef TISSLM_STELLAR_TOKENIZER_H
#define TISSLM_STELLAR_TOKENIZER_H

/**
 * @file stellar_tokenizer.h
 * @brief Stellar wrapper for the existing TissLM Tokenizer.
 */

// Accessing internals for stellar visualization without modifying production code
#define private public
#define protected public
#include "tokenizer/tokenizer.h"
#undef private
#undef protected

#include <string>
#include <vector>

namespace TissLM {
namespace Stellar {

/**
 * @class StellarTokenizer
 * @brief Integrates the existing TissLM Tokenizer with Stellar enhancements.
 */
class StellarTokenizer : public TissLM::Tokenizer::Tokenizer {
public:
    StellarTokenizer() : TissLM::Tokenizer::Tokenizer("") {}

    /**
     * @brief Trains the existing tokenizer and adds Stellar visual telemetry.
     */
    void train_stellar(const std::string& text, int vocab_size);

    /**
     * @brief Enhanced robust decoding logic.
     */
    std::string decode_stellar(const std::vector<int>& token_ids) const;

    // Use base encode
};

} // namespace Stellar
} // namespace TissLM

#endif // TISSLM_STELLAR_TOKENIZER_H
