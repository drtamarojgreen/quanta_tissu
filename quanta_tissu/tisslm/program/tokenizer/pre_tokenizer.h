#ifndef TISSLM_PRE_TOKENIZER_H
#define TISSLM_PRE_TOKENIZER_H

#include <string>
#include <vector>

namespace TissLM {
namespace Tokenizer {

/**
 * @brief A utility to pre-tokenize a string into a sequence of tokens.
 *
 * This function is designed to be a more readable and maintainable alternative to a single complex regex.
 * It splits text based on a set of rules designed for English and Spanish.
 *
 * @param text The input string to tokenize.
 * @return A vector of strings, where each string is a token.
 */
std::vector<std::string> pre_tokenize(const std::string& text);

} // namespace Tokenizer
} // namespace TissLM

#endif // TISSLM_PRE_TOKENIZER_H
