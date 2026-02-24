#include "core/transformer_model.h"
#include "stellar_inference/stellar_generator.h"
#include "stellar_inference/stellar_visualizer.h"
#include "stellar_inference/stellar_meta_analyst.h"

// Non-invasive inspection for Stellar visualization
#define private public
#include "quantatissu.h"
#include "tokenizer/tokenizer.h"
#include "training/trainer.h"
#undef private

#include "training/optimizer.h"
#include "training/loss_function.h"
#include "training/dataset.h"
#include "config/TestConfig.h"
#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <iomanip>

using namespace TissLM::Core;
using namespace TissLM::Stellar;
using namespace TissLM::Generation;
using namespace TissLM::Training;
using namespace TissLM::Tokenizer;
using namespace TissNum;

/**
 * @brief Robust UTF-8 decoding logic ported from StellarTokenizer.
 */
std::string robust_decode(const std::vector<int>& token_ids, const Tokenizer& tokenizer) {
    const auto& vocab = tokenizer.vocab;

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

/**
 * @brief Ultimate Stellar Showcase - Refined to use existing core implementations.
 */
void run_stellar_showcase() {
    std::cout << "================================================================" << std::endl;
    std::cout << "    QuantaTissu Frontier - STELLAR SHOWCASE (CORE POWERED)" << std::endl;
    std::cout << "================================================================" << std::endl;

    std::string source_path = "stellar_inference_test.cpp";
    std::ifstream check(source_path);
    if (!check.is_open()) source_path = "../stellar_inference_test.cpp";
    check.close();

    // 1. Meta-Analysis
    std::cout << "\n[STAGE 1] Meta-Programming: Code Topology Analysis" << std::endl;
    auto s_metrics = StellarMetaAnalyst::analyze_source(source_path);
    std::cout << "   3D Character Cloud Projection (ASCII Mapping):" << std::endl;
    std::cout << StellarVisualizer::render_3d_graph(StellarMetaAnalyst::extract_3d_points(s_metrics), 75, 20);

    // 2. Tokenizer Integration (Implementing existing tokenizer)
    std::cout << "\n[STAGE 2] Tokenizer: Core BPE Algorithm Integration" << std::endl;
    Tokenizer tokenizer("");
    std::ifstream ifs(source_path);
    std::string corpus((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    tokenizer.train(corpus, 350, true);

    // 3. Model Architecture Analysis
    std::cout << "\n[STAGE 3] Model Intelligence: Core Structure Audit" << std::endl;
    int vocab_size = tokenizer.get_vocab_size();
    auto model = std::make_shared<TransformerModel>(
        vocab_size, 64, 32, 4, 2, 128, 0.1f, 0
    );
    auto m_metrics = StellarMetaAnalyst::analyze_model(model);
    std::map<std::string, std::vector<float>> m_grid;
    m_grid["TransformerCore"] = {(float)m_metrics.parameter_count, (float)m_metrics.layer_count, m_metrics.param_density};
    std::cout << StellarVisualizer::render_analysis_grid(m_grid, {"Params", "Layers", "Density"});

    // 4. Stellar Training (Implementing existing training model)
    std::cout << "\n[STAGE 4] Training Engine: Delegated Backpropagation" << std::endl;
    auto opt = std::make_shared<Adam>(1e-3);
    auto loss_fn = std::make_shared<CrossEntropyLoss>();

    std::vector<int> tokens = tokenizer.encode(corpus);
    std::vector<int> small_tokens(tokens.begin(), tokens.begin() + std::min((size_t)80, tokens.size()));
    TokenDataset dataset(small_tokens, 16);

    std::vector<Point3D> loss_history;
    int epochs = 2;
    int batch_size = 4;

    for (int epoch = 0; epoch < epochs; ++epoch) {
        size_t num_batches = (dataset.size() + batch_size - 1) / batch_size;
        for (size_t b = 0; b < num_batches; ++b) {
            size_t batch_start = b * batch_size;
            size_t batch_end = std::min((size_t)batch_start + batch_size, (size_t)dataset.size());
            size_t current_batch_size = batch_end - batch_start;

            TissNum::Matrix batch_input({current_batch_size, dataset.get_item(0).first.cols()});
            TissNum::Matrix batch_target({current_batch_size, dataset.get_item(0).second.cols()});

            for (size_t i = 0; i < current_batch_size; ++i) {
                auto item = dataset.get_item(batch_start + i);
                for (size_t col = 0; col < item.first.cols(); ++col) batch_input({i, col}) = item.first({0, col});
                for (size_t col = 0; col < item.second.cols(); ++col) batch_target({i, col}) = item.second({0, col});
            }

            TissNum::Matrix predictions = model->forward(batch_input);
            TissNum::Matrix flat_target({batch_target.rows() * batch_target.cols(), 1});
            for (size_t r = 0; r < batch_target.rows(); ++r) {
                for (size_t c = 0; c < batch_target.cols(); ++c) {
                    flat_target({r * batch_target.cols() + c, 0}) = batch_target({r, c});
                }
            }

            float loss_val = loss_fn->compute_loss(predictions, flat_target);
            loss_history.push_back({(float)epoch, (float)b, loss_val});

            TissNum::Matrix grad_loss = loss_fn->compute_gradient(predictions, flat_target);
            model->backward(grad_loss);

            auto params = model->get_parameters();
            std::vector<TissNum::Parameter*> raw_params;
            for(const auto& par : params) raw_params.push_back(par.get());
            opt->update(raw_params);

            std::cout << "\r[STELLAR] Epoch " << epoch << " Step " << b << "/" << num_batches << " Loss: " << loss_val << std::flush;
        }
        std::cout << std::endl;
    }
    std::cout << "\n[STELLAR] Loss Topology (3D ASCII Perspective)" << std::endl;
    std::cout << StellarVisualizer::render_3d_graph(loss_history, 80, 25);

    // 5. Stellar Inference
    std::cout << "\n[STAGE 5] Inference: Optimized KV-Cache Decoding" << std::endl;
    GenerationConfig config = GenerationConfig::greedy();
    config.eos_ids = {0};
    StellarGenerator generator(model, config);

    std::vector<int> prompt_ids = {tokens[0], tokens[1], tokens[2], tokens[3]};
    auto result = generator.beam_search(prompt_ids, 15, 4, 0);
    std::cout << "   Generated Fragment: " << robust_decode(result, tokenizer) << std::endl;

    // 6. System Synthesis: QuantaTissu Integration
    std::cout << "\n[STAGE 6] System Synthesis: QuantaTissu Program Integration" << std::endl;
    ModelConfig qt_config = { (int)tokenizer.get_vocab_size(), 64, 2, 4, 128 };
    QuantaTissu qt(qt_config, "");
    std::string prompt_text = "QuantaTissu Integration Test";
    std::string qt_gen = qt.generate(prompt_text, 5);
    std::cout << "   QuantaTissu Prototype Output: " << qt_gen << std::endl;

    // 7. Final Results
    std::cout << "\n[STAGE 7] Process Synthesis" << std::endl;
    std::map<std::string, std::vector<float>> f_grid;
    f_grid["Mission-Metrics"] = {(float)s_metrics.complexity_index, (float)vocab_size, (float)result.size()};
    std::cout << StellarVisualizer::render_analysis_grid(f_grid, {"Complexity", "VocabSize", "GenLen"});

    std::cout << "================================================================" << std::endl;
    std::cout << "    STELLAR MISSION: CORE ENGINE SUCCESS" << std::endl;
    std::cout << "================================================================" << std::endl;
}

int main() {
    try {
        run_stellar_showcase();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Stellar Critical Error: " << e.what() << std::endl;
        return 1;
    }
}
