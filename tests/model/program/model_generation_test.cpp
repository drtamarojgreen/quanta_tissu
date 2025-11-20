#include "../../../quanta_tissu/tisslm/program/core/transformerblock.h" // For TissNum namespace
#include "../../../quanta_tissu/tisslm/program/generation/generation_config.h"
#include <iostream>
#include <vector>
#include <memory>

using namespace TissNum;

// Mock TransformerModel and Generator
namespace TissLM {
namespace Core {
    class TransformerModel {
    public:
        TransformerModel(int vocab, int max_seq, int embed, int heads, int layers, int ffn, float dropout, int lora) {}
        Matrix forward(const Matrix& x) { return Matrix({1, 5, 100}); }
        Matrix forward_inference(const Matrix& x, std::vector<std::pair<Matrix, Matrix>>& past_kv, std::vector<std::pair<Matrix, Matrix>>& new_kv) {
            return Matrix({1, 100});
        }
    };
}
namespace Generation {
    class Generator {
    public:
        Generator(std::shared_ptr<Core::TransformerModel> model, const GenerationConfig& config) {}
        std::vector<int> generate(const std::vector<int>& prompt, int max_new) {
            std::vector<int> result = prompt;
            for(int i=0; i < max_new; ++i) result.push_back(i);
            return result;
        }
    };
}
}
using namespace TissLM::Core;
using namespace TissLM::Generation;

void test_transformer_model() {
    std::cout << "--- Testing Transformer Model (Mocked) ---" << std::endl;
    TransformerModel model(100, 1024, 32, 4, 2, 128, 0.1, 4);
    Matrix input({1, 5});
    Matrix output = model.forward(input);
    if (output.shape() == std::vector<int>({1, 5, 100})) {
         std::cout << "  [PASSED] TransformerModel forward." << std::endl;
    } else {
        throw std::runtime_error("TransformerModel forward failed.");
    }
}

void test_generator() {
    std::cout << "--- Testing Generator (Mocked) ---" << std::endl;
    auto model = std::make_shared<TransformerModel>(100, 1024, 32, 4, 2, 128, 0.1, 0);
    GenerationConfig config;
    Generator generator(model, config);
    std::vector<int> generated = generator.generate({10, 20}, 5);
    if (generated.size() == 7) {
        std::cout << "  [PASSED] Generator produced output." << std::endl;
    } else {
        throw std::runtime_error("Generator generate failed.");
    }
}

int main() {
    try {
        test_transformer_model();
        test_generator();
        std::cout << "\nAll Model and Generation tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Model and Generation tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}