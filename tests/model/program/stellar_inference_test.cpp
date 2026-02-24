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
 * @brief Ultimate Stellar End-to-End Showcase.
 *
 * This suite demonstrates the full cycle:
 * 1. Self-Meta-Analysis of the code.
 * 2. Structural Analysis of the Model.
 * 3. High-Performance Tokenizer Training on code.
 * 4. Model Training with 3D Loss Visualization.
 * 5. Optimized Stellar Inference.
 * 6. Result Synthesis in ASCII Grids.
 */
void run_stellar_ultimate_showcase() {
    std::cout << "================================================================" << std::endl;
    std::cout << "    QuantaTissu Frontier - STELLAR ULTIMATE SHOWCASE" << std::endl;
    std::cout << "================================================================" << std::endl;

    // Try both current and parent directory to handle execution from 'build'
    std::string source_path = "stellar_inference_test.cpp";
    std::ifstream check(source_path);
    if (!check.is_open()) {
        source_path = "../stellar_inference_test.cpp";
    }
    check.close();

    // 1. Meta-Analysis
    std::cout << "\n[STAGE 1] Meta-Programming: Self-Source Analysis" << std::endl;
    auto s_metrics = StellarMetaAnalyst::analyze_source(source_path);
    std::cout << "   Source lines detected: " << s_metrics.line_count << std::endl;
    std::cout << "   3D Character Cloud Projection:" << std::endl;
    std::cout << StellarVisualizer::render_3d_graph(StellarMetaAnalyst::extract_3d_points(s_metrics), 75, 20);

    // 2. Tokenizer Integration
    std::cout << "\n[STAGE 2] Tokenizer: Core BPE Implementation" << std::endl;
    StellarTokenizer tokenizer;
    std::ifstream ifs(source_path);
    std::string corpus((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    tokenizer.train_stellar(corpus, 350);

    // 3. Model Architecture Analysis
    std::cout << "\n[STAGE 3] Model Intelligence: Structural Analysis" << std::endl;
    int vocab_size = tokenizer.get_vocab_size();
    auto model = std::make_shared<TransformerModel>(
        vocab_size, 64, 16, 1, 1, 32, 0.1f, 0
    );
    auto m_metrics = StellarMetaAnalyst::analyze_model(model);
    std::map<std::string, std::vector<float>> m_grid;
    m_grid["TransformerModel"] = {(float)m_metrics.parameter_count, (float)m_metrics.layer_count, m_metrics.param_density};
    std::cout << StellarVisualizer::render_analysis_grid(m_grid, {"Params", "Layers", "Density"});

    // 4. Stellar Training
    std::cout << "\n[STAGE 4] Training Engine: 3D Telemetry" << std::endl;
    auto opt = std::make_shared<Adam>(5e-4);
    auto loss = std::make_shared<CrossEntropyLoss>();
    StellarTrainer trainer(model, opt, loss);

    std::vector<int> tokens = tokenizer.encode(corpus);
    // Only use first 50 tokens for dataset to speed up
    std::vector<int> small_tokens(tokens.begin(), tokens.begin() + std::min((size_t)50, tokens.size()));
    TokenDataset dataset(small_tokens, 8);
    trainer.train_stellar(dataset, 1, 2, true);

    // 5. Stellar Inference
    std::cout << "\n[STAGE 5] Inference: Optimized Generation" << std::endl;
    GenerationConfig config = GenerationConfig::greedy();
    config.eos_ids = {0};
    StellarGenerator generator(model, config);

    std::vector<int> prompt = {tokens[0], tokens[1], tokens[2], tokens[3]};
    auto result = generator.beam_search(prompt, 10, 5, 0);
    std::cout << "   Generated Fragment: " << tokenizer.decode(result) << std::endl;

    // 6. Synthesis
    std::cout << "\n[STAGE 6] Final Synthesis" << std::endl;
    std::map<std::string, std::vector<float>> f_grid;
    f_grid["Process-Metrics"] = {(float)s_metrics.complexity_index, (float)vocab_size, (float)result.size()};
    std::cout << StellarVisualizer::render_analysis_grid(f_grid, {"Complexity", "VocabSize", "GenLen"});

    std::cout << "================================================================" << std::endl;
    std::cout << "    STELLAR MISSION SUCCESS: VISION REALIZED" << std::endl;
    std::cout << "================================================================" << std::endl;
}

int main() {
    try {
        run_stellar_ultimate_showcase();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Stellar Critical Failure: " << e.what() << std::endl;
        return 1;
    }
}
