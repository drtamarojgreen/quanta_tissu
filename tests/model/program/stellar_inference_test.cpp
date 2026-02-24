#include "core/transformer_model.h"
#include "stellar_inference/stellar_generator.h"
#include "stellar_inference/stellar_tokenizer.h"
#include "stellar_inference/stellar_visualizer.h"
#include "stellar_inference/stellar_meta_analyst.h"
#include "stellar_inference/stellar_trainer.h"
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
    StellarTokenizer tokenizer;
    std::ifstream ifs(source_path);
    std::string corpus((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    tokenizer.train_stellar(corpus, 350);

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
    auto loss = std::make_shared<CrossEntropyLoss>();
    StellarTrainer trainer(model, opt, loss);

    std::vector<int> tokens = tokenizer.encode(corpus);
    std::vector<int> small_tokens(tokens.begin(), tokens.begin() + std::min((size_t)80, tokens.size()));
    TokenDataset dataset(small_tokens, 16);
    trainer.train_stellar(dataset, 2, 4, true);

    // 5. Stellar Inference
    std::cout << "\n[STAGE 5] Inference: Optimized KV-Cache Decoding" << std::endl;
    GenerationConfig config = GenerationConfig::greedy();
    config.eos_ids = {0};
    StellarGenerator generator(model, config);

    std::vector<int> prompt = {tokens[0], tokens[1], tokens[2], tokens[3]};
    auto result = generator.beam_search(prompt, 15, 4, 0);
    std::cout << "   Generated Fragment: " << tokenizer.decode_stellar(result) << std::endl;

    // 6. Final Results
    std::cout << "\n[STAGE 6] Process Synthesis" << std::endl;
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
