#include "stellar_tokenizer.h"
#include "stellar_visualizer.h"
#include <iostream>
#include <map>

namespace TissLM {
namespace Stellar {

void StellarTokenizer::train_stellar(const std::string& text, int vocab_size) {
    std::cout << "[STELLAR] Existing Engine: Initiating Tokenizer Training..." << std::endl;

    // Call existing implementation
    this->train(text, vocab_size, true);

    std::cout << "[STELLAR] Finalizing Vision: Vocab Size = " << this->get_vocab_size() << std::endl;

    // Stellar Visualization: Show some of the learned merges
    std::cout << "\n[VIZ] Stellar Merge Analysis (Top Merges)" << std::endl;
    std::map<std::string, std::vector<float>> grid;
    for (size_t i = 0; i < std::min((size_t)10, ranked_merges.size()); ++i) {
        auto p = ranked_merges[i];
        std::string label = "(" + std::to_string(p.first) + "," + std::to_string(p.second) + ")";
        grid[label] = {(float)i};
    }
    std::cout << StellarVisualizer::render_analysis_grid(grid, {"Rank"});
}

std::string StellarTokenizer::decode_stellar(const std::vector<int>& token_ids) const {
    // We implement a more robust version here but utilizing the existing vocab
    std::vector<unsigned char> byte_buffer;
    for (int id : token_ids) {
        auto it = vocab.find(id);
        if (it != vocab.end()) {
            byte_buffer.insert(byte_buffer.end(), it->second.begin(), it->second.end());
        }
    }

    std::string text;
    for (size_t i = 0; i < byte_buffer.size(); ) {
        unsigned char byte = byte_buffer[i];
        size_t len = 0;
        if (byte < 0x80) len = 1;
        else if ((byte & 0xE0) == 0xC0) len = 2;
        else if ((byte & 0xF0) == 0xE0) len = 3;
        else if ((byte & 0xF8) == 0xF0) len = 4;
        else { i++; continue; }

        if (i + len <= byte_buffer.size()) {
            bool valid = true;
            for (size_t j = 1; j < len; ++j) if ((byte_buffer[i + j] & 0xC0) != 0x80) { valid = false; break; }
            if (valid) {
                for (size_t j = 0; j < len; ++j) text += static_cast<char>(byte_buffer[i + j]);
                i += len;
            } else { i++; }
        } else { i++; }
    }
    return text;
}

} // namespace Stellar
} // namespace TissLM
