#ifdef __linux__
#define _GNU_SOURCE
#include <fenv.h>
#endif
#include <signal.h>
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <cassert>

// THE TRICK: Probing internals non-invasively
#define private public
#define protected public
#include "core/transformer_model.h"
#include "core/layernorm.h"
#include "tokenizer/tokenizer.h"
#include "training/trainer.h"
#include "training/dataset.h"
#include "training/optimizer.h"
#include "training/loss_function.h"
#include "generation/generator.h"
#include "generation/generation_config.h"
#undef private
#undef protected

using namespace TissLM::Core;
using namespace TissLM::Tokenizer;
using namespace TissLM::Training;
using namespace TissLM::Generation;
using namespace TissNum;

class ArithmeticDebugHarness {
public:
    void run() {
        std::cout << "====================================================" << std::endl;
        std::cout << "   COMPREHENSIVE ARITHMETIC DEBUG HARNESS           " << std::endl;
        std::cout << "====================================================" << std::endl;

        setup_fpe_traps();

        // 1. Synthetic Minimal Configuration
        breadcrumb("Initializing Pipeline");
        std::string text = "the quick brown fox jumps over the lazy dog. ";
        instantiate_tokenizer(text);

        int vocab_size = tokenizer_->get_vocab_size();
        std::vector<int> tokens = tokenizer_->encode(text);

        // Metrics logging (Directive point 6)
        std::cout << "[DIAGNOSTIC] Token Count: " << tokens.size() << std::endl;
        std::cout << "[DIAGNOSTIC] Vocabulary Size: " << vocab_size << std::endl;

        TokenDataset dataset(tokens, 16);
        std::cout << "[DIAGNOSTIC] Batch Size: 1" << std::endl;

        auto model = std::make_shared<TransformerModel>(
            vocab_size, 32, 64, 2, 2, 128, 0.1f, 0
        );
        auto adam = std::make_shared<Adam>(1e-3);
        auto loss_fn = std::make_shared<CrossEntropyLoss>();
        Trainer trainer(model, adam, loss_fn);

        // 2. Training Loop Invocation (Directive point 2)
        breadcrumb("Starting Training Loop Execution");
        for (int step = 1; step <= 2; ++step) {
            auto item = dataset.get_item(step % dataset.size());

            // Forward
            Matrix predictions = model->forward(item.first);

            // Log Intermediate Scalars: LayerNorm denominators
            probe_layernorm(model->final_layer_norm_);

            // Loss
            Matrix targets = item.second.reshape({item.second.cols(), 1});
            float loss = loss_fn->compute_loss(predictions, targets);
            std::cout << "[DIAGNOSTIC] Step " << step << " Loss: " << loss << std::endl;
            assert(std::isfinite(loss));

            // Backward
            Matrix grad_loss = loss_fn->compute_gradient(predictions, targets);
            model->backward(grad_loss);

            // Diagnostic: Gradient Norms
            trace_gradient_norms(model);

            // Update
            auto params = model->get_parameters();
            std::vector<Parameter*> raw_params;
            for(auto& p : params) raw_params.push_back(p.get());

            // Diagnostic: Adam Denominators
            if (!adam->v_.empty()) trace_adam_denominators(adam);

            adam->update(raw_params);
        }

        // 3. Generator Invocation (Directive point 2)
        breadcrumb("Invoking Generator Infrastructure");
        GenerationConfig gen_config = GenerationConfig::greedy();
        Generator generator(model, gen_config);
        std::vector<int> prompt = {tokens[0], tokens[1]};
        std::vector<int> generated = generator.generate(prompt, 5);
        std::cout << "[DIAGNOSTIC] Generated token ids: ";
        for(int id : generated) std::cout << id << " ";
        std::cout << std::endl;

        std::cout << "====================================================" << std::endl;
        std::cout << "       DEBUG HARNESS FINISHED SUCCESSFULLY          " << std::endl;
        std::cout << "====================================================" << std::endl;
    }

private:
    void breadcrumb(const std::string& msg) {
        std::cout << "[BREADCRUMB] " << msg << std::endl;
    }

    void setup_fpe_traps() {
#ifdef __linux__
        signal(SIGFPE, [](int sig) {
            std::cerr << "\n[CRITICAL] SIGFPE TRAPPED! Floating point exception detected." << std::endl;
            exit(sig);
        });
        feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
    }

    void instantiate_tokenizer(const std::string& text) {
        tokenizer_ = std::make_shared<Tokenizer>("");
        tokenizer_->train(text, 258, false);
    }

    void probe_layernorm(const LayerNorm& ln) {
        if (ln.eps_ <= 0.0f) {
            std::cerr << "[FATAL] LayerNorm epsilon is non-positive." << std::endl;
            exit(1);
        }
    }

    void trace_gradient_norms(std::shared_ptr<TransformerModel> model) {
        auto params = model->get_parameters();
        for (size_t i = 0; i < std::min((size_t)3, params.size()); ++i) {
            const Matrix& grad = params[i]->grad();
            float sum_sq = 0.0f;
            const float* d = grad.get_data();
            for(size_t j=0; j<grad.data_size(); ++j) sum_sq += d[j]*d[j];
            std::cout << "[DIAGNOSTIC] Param " << i << " [" << params[i]->name() << "] GradNorm: " << std::sqrt(sum_sq) << std::endl;
        }
    }

    void trace_adam_denominators(std::shared_ptr<Adam> adam) {
        std::cout << "[DIAGNOSTIC] Probing Adam internal denominators..." << std::endl;
        for (size_t i = 0; i < std::min((size_t)3, adam->v_.size()); ++i) {
            float bias_corr = 1.0f - std::pow(adam->beta2_, adam->t_ + 1);
            const float* d = adam->v_[i].get_data();
            float min_denom = 1e10;
            for(size_t j=0; j<adam->v_[i].data_size(); ++j) {
                float denom = std::sqrt(d[j] / bias_corr) + adam->epsilon_;
                min_denom = std::min(min_denom, denom);
            }
            std::cout << "  Param " << i << " Min Denom: " << min_denom << std::endl;
        }
    }

    std::shared_ptr<Tokenizer> tokenizer_;
};

int main() {
    ArithmeticDebugHarness harness;
    harness.run();
    return 0;
}
