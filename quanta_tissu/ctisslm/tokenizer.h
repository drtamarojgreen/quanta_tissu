#ifndef QUANTA_TISSU_TOKENIZER_H
#define QUANTA_TISSU_TOKENIZER_H

#include <string>
#include <vector>
#include <map>
#include <regex> // For BPE_SPLIT_PATTERN
#include <fstream>
#include <sstream>
#include <algorithm> // For std::sort
#include <stdexcept>

// For JSON parsing, we'll need a library. For now, we'll use a simple placeholder.
// In a real project, you'd integrate a library like nlohmann/json.

namespace quanta_tissu {

// Helper function to find consecutive pairs of IDs in a list.
std::map<std::pair<int, int>, int> get_pairs(const std::vector<int>& ids) {
    std::map<std::pair<int, int>, int> pairs;
    for (size_t i = 0; i < ids.size() - 1; ++i) {
        pairs[{ids[i], ids[i+1]}]++;
    }
    return pairs;
}

class BPETokenizer {
public:
    std::map<std::pair<int, int>, int> merges; // (id1, id2) -> new_id
    std::map<int, std::vector<unsigned char>> vocab; // id -> byte_sequence
    std::map<std::vector<unsigned char>, int> reverse_vocab; // byte_sequence -> id

    BPETokenizer() {}

    // Placeholder for train method
    void train(const std::string& text, int vocab_size, bool verbose = false) {
        // This will be a complex implementation involving regex, byte handling, and iterative merging.
        // For now, it's a stub.
    }

    // Regex for splitting text into words and punctuation, similar to GPT-2.
    // This needs to be a static member or a global constant if used outside the class.
    // For now, defined here for direct use in encode.
    const std::regex BPE_SPLIT_PATTERN = std::regex(R"('(?:[sdmt]|ll|ve|re)| ?\p{L}+| ?\p{N}+| ?[^\s\p{L}\p{N}]+|\s+(?!\S)|\s+)", std::regex::ECMAScript | std::regex::optimize);

    std::vector<int> _encode_chunk(const std::vector<unsigned char>& text_bytes) {
        std::vector<int> ids;
        for (unsigned char byte : text_bytes) {
            ids.push_back(static_cast<int>(byte));
        }

        while (true) {
            std::pair<int, int> best_pair = {-1, -1};
            int min_new_id = std::numeric_limits<int>::max();

            // Find the next best pair to merge based on the learned order
            for (size_t i = 0; i < ids.size() - 1; ++i) {
                std::pair<int, int> current_pair = {ids[i], ids[i+1]};
                if (merges.count(current_pair)) {
                    int new_id = merges[current_pair];
                    if (new_id < min_new_id) {
                        min_new_id = new_id;
                        best_pair = current_pair;
                    }
                }
            }

            if (best_pair.first == -1) {
                break; // No more merges are possible in this chunk
            }

            // Merge the pair
            int new_id = merges[best_pair];
            std::vector<int> new_ids;
            size_t i = 0;
            while (i < ids.size()) {
                if (i < ids.size() - 1 && ids[i] == best_pair.first && ids[i+1] == best_pair.second) {
                    new_ids.push_back(new_id);
                    i += 2;
                } else {
                    new_ids.push_back(ids[i]);
                    i += 1;
                }
            }
            ids = new_ids;
        }
        return ids;
    }

    std::vector<int> encode(const std::string& text) {
        std::vector<int> all_ids;
        std::sregex_iterator current_match(text.begin(), text.end(), BPE_SPLIT_PATTERN);
        std::sregex_iterator last_match; // End of matches

        while (current_match != last_match) {
            std::smatch match = *current_match;
            std::string word = match.str();
            
            // Encode word to UTF-8 bytes
            std::vector<unsigned char> text_bytes;
            for (char c : word) {
                text_bytes.push_back(static_cast<unsigned char>(c));
            }
            
            std::vector<int> encoded_chunk = _encode_chunk(text_bytes);
            all_ids.insert(all_ids.end(), encoded_chunk.begin(), encoded_chunk.end());
            
            current_match++;
        }
        return all_ids;
    }

    std::string decode(const std::vector<int>& ids) {
        std::vector<unsigned char> text_bytes;
        for (int id : ids) {
            if (vocab.count(id)) {
                const std::vector<unsigned char>& byte_sequence = vocab[id];
                text_bytes.insert(text_bytes.end(), byte_sequence.begin(), byte_sequence.end());
            } else {
                // Handle unknown token: append a replacement character or throw error
                text_bytes.push_back(static_cast<unsigned char>('?')); 
            }
        }
        
        // Convert byte vector to string (UTF-8 decoding)
        // This is a basic conversion. For full UTF-8 handling, consider a library.
        std::string result;
        result.reserve(text_bytes.size());
        for (unsigned char byte : text_bytes) {
            result += static_cast<char>(byte);
        }
        return result;
    }

    // Placeholder for save method
    void save(const std::string& prefix) {
        // This will save vocab and merges to files.
    }

    void load(const std::string& prefix) {
        std::string vocab_file = prefix + "_vocab.json";
        std::string merges_file = prefix + "_merges.txt";

        // Load vocabulary
        std::ifstream vocab_ifs(vocab_file);
        if (!vocab_ifs.is_open()) {
            throw std::runtime_error("Failed to open vocab file: " + vocab_file);
        }
        std::string line;
        std::string json_content;
        while (std::getline(vocab_ifs, line)) {
            json_content += line;
        }
        vocab_ifs.close();

        // Basic JSON parsing for vocab (assuming simple format: {"id": [byte1, byte2, ...]})
        // This is a very rudimentary parser and will break on complex JSON.
        // A proper JSON library (e.g., nlohmann/json) should be used in a real application.
        size_t pos = json_content.find("{");
        if (pos == std::string::npos) throw std::runtime_error("Invalid JSON format in vocab file.");
        json_content = json_content.substr(pos + 1); // Remove leading "{"

        pos = json_content.rfind("}");
        if (pos == std::string::npos) throw std::runtime_error("Invalid JSON format in vocab file.");
        json_content = json_content.substr(0, pos); // Remove trailing "}"

        std::stringstream ss(json_content);
        std::string segment;
        while (std::getline(ss, segment, ',')) {
            segment.erase(0, segment.find_first_not_of(" 	\n\r")); // Trim leading whitespace
            segment.erase(segment.find_last_not_of(" 	\n\r") + 1); // Trim trailing whitespace

            size_t colon_pos = segment.find(":");
            if (colon_pos == std::string::npos) continue;

            std::string key_str = segment.substr(0, colon_pos);
            key_str.erase(0, key_str.find_first_not_of(" \"")); // Trim leading " and whitespace
            key_str.erase(key_str.find_last_not_of(" \"") + 1); // Trim trailing " and whitespace

            int id = std::stoi(key_str);

            std::string value_str = segment.substr(colon_pos + 1);
            value_str.erase(0, value_str.find_first_not_of(" 	\n\r[")); // Trim leading whitespace and "["
            value_str.erase(value_str.find_last_not_of(" 	\n\r]") + 1); // Trim trailing whitespace and "]"

            std::vector<unsigned char> byte_sequence;
            std::stringstream byte_ss(value_str);
            std::string byte_str;
            while (std::getline(byte_ss, byte_str, ',')) {
                byte_str.erase(0, byte_str.find_first_not_of(" 	\n\r"));
                byte_str.erase(byte_str.find_last_not_of(" 	\n\r") + 1);
                if (!byte_str.empty()) {
                    byte_sequence.push_back(static_cast<unsigned char>(std::stoi(byte_str)));
                }
            }
            vocab[id] = byte_sequence;
        }

        // Load merges
        merges.clear();
        std::ifstream merges_ifs(merges_file);
        if (!merges_ifs.is_open()) {
            throw std::runtime_error("Failed to open merges file: " + merges_file);
        }
        while (std::getline(merges_ifs, line)) {
            std::stringstream line_ss(line);
            int p1, p2, new_id;
            if (line_ss >> p1 >> p2 >> new_id) {
                merges[{p1, p2}] = new_id;
            }
        }
        merges_ifs.close();

        // Rebuild reverse vocabulary
        reverse_vocab.clear();
        for (const auto& pair : vocab) {
            reverse_vocab[pair.second] = pair.first;
        }
    }

    size_t get_vocab_size() const {
        return vocab.size();
    }

    int get_token_id(const std::string& token) {
        // This will encode the token and return the first ID.
        // For now, a simple placeholder.
        if (token.length() == 1) {
            unsigned char byte = static_cast<unsigned char>(token[0]);
            if (reverse_vocab.count({byte})) {
                return reverse_vocab[{byte}];
            }
        }
        return 0; // Fallback to unk_token_id
    }

    std::string get_token(int token_id) {
        // This will decode the token ID to a string.
        // For now, a simple placeholder.
        if (vocab.count(token_id)) {
            std::string s;
            for (unsigned char byte : vocab[token_id]) {
                s += static_cast<char>(byte);
            }
            return s;
        }
        return "?"; // Unknown token
    }
};

class Tokenizer {
public:
    BPETokenizer bpe_tokenizer;
    int unk_token_id;
    int pad_token_id;

    Tokenizer(const std::string& tokenizer_path = "") {
        bpe_tokenizer.load(tokenizer_path);
        // Assuming unk_token and pad_token are handled by BPE vocab or default to 0/1
        unk_token_id = 0; 
        pad_token_id = 1;
    }

    std::vector<int> tokenize(const std::string& text) {
        return bpe_tokenizer.encode(text);
    }

    std::string detokenize(const std::vector<int>& token_ids) {
        std::vector<std::string> decoded_tokens;
        for (int id : token_ids) {
            decoded_tokens.push_back(bpe_tokenizer.decode({id}));
        }

        std::string reconstructed_text;
        for (size_t i = 0; i < decoded_tokens.size(); ++i) {
            const std::string& token_str = decoded_tokens[i];
            // Space re-insertion heuristic
            if (i > 0 && 
                token_str.length() > 0 && 
                token_str[0] != '.' && 
                token_str[0] != ',' && 
                token_str[0] != '!' && 
                token_str[0] != '?' && 
                token_str[0] != ':' && 
                token_str[0] != ';' && 
                token_str[0] != ')' && 
                token_str[0] != ']' && 
                token_str[0] != '}' &&
                token_str[0] != "'") { // Added single quote
                reconstructed_text += ' ';
            }
            reconstructed_text += token_str;
        }
        
        // Remove any leading space that might result from the first token
        if (reconstructed_text.length() > 0 && reconstructed_text[0] == ' ') {
            reconstructed_text = reconstructed_text.substr(1);
        }

        return reconstructed_text;
    }

    size_t get_vocab_size() const {
        return bpe_tokenizer.get_vocab_size();
    }

    int get_token_id(const std::string& token) {
        return bpe_tokenizer.get_token_id(token);
    }

    std::string get_token(int token_id) {
        return bpe_tokenizer.get_token(token_id);
    }
};

} // namespace quanta_tissu

#endif // QUANTA_TISSU_TOKENIZER_H
