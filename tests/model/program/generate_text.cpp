#include "quanta_tissu/tisslm/program/core/transformer_model.h"
#include "quanta_tissu/tisslm/program/tokenizer/tokenizer.h"
#include "quanta_tissu/tisslm/program/generation/generator.h"
#include "quanta_tissu/tisslm/program/generation/generation_config.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

void load_model_weights_only(std::shared_ptr<TissLM::Core::TransformerModel> model, const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("Cannot open checkpoint file for reading: " + path);
    }
    auto params = model->get_parameters();
    size_t num_params;
    ifs.read(reinterpret_cast<char*>(&num_params), sizeof(num_params));
    if (num_params != params.size()) {
        throw std::runtime_error("Checkpoint parameter count (" + std::to_string(num_params) + 
                                 ") does not match model parameter count (" + std::to_string(params.size()) + ").");
    }
    for (const auto& p : params) {
        auto& matrix = p->value();
        size_t shape_size;
        ifs.read(reinterpret_cast<char*>(&shape_size), sizeof(shape_size));
        std::vector<size_t> shape(shape_size);
        ifs.read(reinterpret_cast<char*>(shape.data()), shape_size * sizeof(size_t));
        if (shape != matrix.get_shape()) {
            throw std::runtime_error("Checkpoint parameter shape does not match model parameter shape.");
        }
        size_t data_size;
        ifs.read(reinterpret_cast<char*>(&data_size), sizeof(data_size));
        ifs.read(reinterpret_cast<char*>(matrix.get_data()), data_size * sizeof(float));
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <checkpoint_path> \"<prompt>\" [generation_length]" << std::endl;
        return 1;
    }

    std::string checkpoint_path = argv[1];
    std::string prompt = argv[2];
    int gen_len = 30;
    if (argc > 3) {
        gen_len = std::atoi(argv[3]);
    }

    try {
        std::cout << "Loading tokenizer..." << std::endl;
        TissLM::Tokenizer::Tokenizer tokenizer("trained_tokenizer");

        std::cout << "Initializing model..." << std::endl;
        int vocab_size = tokenizer.get_vocab_size();
        auto model = std::make_shared<TissLM::Core::TransformerModel>(
            vocab_size,
            128,  // max_seq_len
            128,  // embed_dim
            2,    // num_heads
            2,    // num_layers
            0.0f, // dropout_rate (0 for generation)
            1     // lora_rank
        );

        std::cout << "Loading weights from checkpoint " << checkpoint_path << "..." << std::endl;
        load_model_weights_only(model, checkpoint_path);

        std::cout << "Generating text..." << std::endl;
        TissLM::Generation::GenerationConfig gen_config = TissLM::Generation::GenerationConfig::nucleus(0.9f, 0.8f);
        TissLM::Generation::Generator generator(model, gen_config);

        std::vector<int> prompt_tokens = tokenizer.encode(prompt);
        std::vector<int> generated_tokens = generator.generate(prompt_tokens, gen_len);
        std::string generated_text = tokenizer.decode(generated_tokens);

        std::cout << "\n========================================" << std::endl;
        std::cout << "Prompt: \"" << prompt << "\"" << std::endl;
        std::cout << "Generated: \"" << generated_text << "\"" << std::endl;
        std::cout << "========================================" << std::endl;

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
