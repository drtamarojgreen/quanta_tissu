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

// THE TRICK: Accessing internals without modifying source
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
    ArithmeticDebugHarness() : step_(0) {}

    void run() {
        std::cout << "====================================================" << std::endl;
        std::cout << "   DETERMINISTIC ARITHMETIC DEBUG HARNESS           " << std::endl;
        std::cout << "====================================================" << std::endl;

        setup_fpe_traps();

        // 0. Specific Bug Investigation
        test_matrix_div_bug();

        instantiate_minimal_tokenizer();

        // 1. Synthetic Dataset
        std::string text = "\x01\x02\x03\x04\x01\x02\x03\x04";
        std::vector<int> tokens = tokenizer_->encode(text);

        token_count_ = tokens.size();
        vocab_size_ = 256;
        std::cout << "[DIAGNOSTIC] Token Count: " << token_count_ << std::endl;
        std::cout << "[DIAGNOSTIC] Vocabulary Size: " << vocab_size_ << std::endl;

        int seq_len = 4;
        TokenDataset dataset(tokens, seq_len);

        // 2. Model Configuration
        model_ = std::make_shared<TransformerModel>(vocab_size_, 8, 16, 2, 1, 32, 0.0f, 0);
        auto adam = std::make_shared<Adam>(1e-3);
        auto loss_fn = std::make_shared<CrossEntropyLoss>();

        Trainer trainer(model_, adam, loss_fn);

        // 3. Training Cycle
        std::cout << "[INFO] Executing Training Cycle..." << std::endl;
        auto item = dataset.get_item(0);

        // Forward
        Matrix predictions = model_->forward(item.first);

        Matrix targets = item.second.reshape({item.second.cols(), 1});
        float loss = loss_fn->compute_loss(predictions, targets);
        std::cout << "[DIAGNOSTIC] Loss: " << loss << std::endl;

        // Backward
        Matrix grad_loss = loss_fn->compute_gradient(predictions, targets);
        model_->backward(grad_loss);

        // Update
        auto params = model_->get_parameters();
        std::vector<Parameter*> raw_params;
        for(auto& p : params) raw_params.push_back(p.get());

        trace_gradient_norms();
        adam->update(raw_params);
        step_++;
        trace_adam_denominators(adam);

        // 4. Generator
        std::cout << "[INFO] Invoking Generator Infrastructure..." << std::endl;
        Generator generator(model_, GenerationConfig::greedy());
        std::vector<int> prompt = {tokens[0]};
        std::vector<int> generated = generator.generate(prompt, 5);
        std::cout << "[DIAGNOSTIC] Generator success." << std::endl;

        // 5. Checkpoint
        verify_checkpointing(trainer, adam);

        std::cout << "====================================================" << std::endl;
        std::cout << "       DEBUG HARNESS FINISHED SUCCESSFULLY          " << std::endl;
        std::cout << "====================================================" << std::endl;
    }

private:
    void setup_fpe_traps() {
#ifdef __linux__
        signal(SIGFPE, [](int sig) {
            std::cerr << "\n[CRITICAL] Floating point exception (SIGFPE)! Division by zero detected." << std::endl;
            exit(sig);
        });
        feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
#endif
    }

    void test_matrix_div_bug() {
        std::cout << "[INFO] Investigating suspicious Matrix division check..." << std::endl;
        Matrix m({1, 1});
        m({0, 0}) = 0.0f;
        // The current implementation of operator/(float, Matrix) checks numerator instead of denominator.
        // It should throw invalid_argument or trigger SIGFPE.
        try {
            // We temporarily disable hardware traps for this specific check to see if it throws or produces inf
#ifdef __linux__
            fedisableexcept(FE_DIVBYZERO);
#endif
            Matrix result = 1.0f / m;
            std::cout << "[CONFIRMED] Logic bug in Matrix operator/(float, Matrix): 1.0f / 0.0f produced " << result({0, 0}) << " without throwing." << std::endl;
#ifdef __linux__
            feenableexcept(FE_DIVBYZERO);
#endif
        } catch (const std::exception& e) {
            std::cout << "[INFO] Caught expected exception: " << e.what() << std::endl;
        }
    }

    void instantiate_minimal_tokenizer() {
        std::ofstream v("tiny_vocab.json");
        v << "{";
        for(int i=0; i<256; ++i) v << "\"" << i << "\": [" << i << "]" << (i == 255 ? "" : ", ");
        v << "}";
        v.close();
        std::ofstream m("tiny_merges.txt"); m.close();
        tokenizer_ = std::make_shared<Tokenizer>("tiny");
    }

    void trace_gradient_norms() {
        auto params = model_->get_parameters();
        for (size_t i = 0; i < std::min((size_t)5, params.size()); ++i) {
            const Matrix& grad = params[i]->grad();
            float sum_sq = 0.0f;
            const float* d = grad.get_data();
            for(size_t j=0; j<grad.data_size(); ++j) sum_sq += d[j]*d[j];
            std::cout << "[DIAGNOSTIC] Param " << i << " [" << params[i]->name() << "] GradNorm: " << std::sqrt(sum_sq) << std::endl;
        }
    }

    void trace_adam_denominators(std::shared_ptr<Adam> adam) {
        if (adam->v_.empty()) return;
        std::cout << "[DIAGNOSTIC] Adam Denominator Stats (Step " << adam->t_ << "):" << std::endl;
        for (size_t i = 0; i < std::min((size_t)3, adam->v_.size()); ++i) {
            const Matrix& v = adam->v_[i];
            float bias_corr = 1.0f - std::pow(adam->beta2_, adam->t_);
            const float* d = v.get_data();
            float min_denom = 1e10;
            for(size_t j=0; j<v.data_size(); ++j) {
                float denom = std::sqrt(d[j] / bias_corr) + adam->epsilon_;
                min_denom = std::min(min_denom, denom);
            }
            std::cout << "  Param " << i << " MinDenom=" << min_denom << std::endl;
        }
    }

    void verify_checkpointing(Trainer& trainer, std::shared_ptr<Adam> adam) {
        std::string cp = "harness_checkpoint.bin";
        trainer.save_checkpoint(cp);
        auto model2 = std::make_shared<TransformerModel>(vocab_size_, 8, 16, 2, 1, 32, 0.0f, 0);
        std::ifstream ifs(cp, std::ios::binary);
        size_t num_params; ifs.read((char*)&num_params, sizeof(num_params));
        auto p2 = model2->get_parameters();
        for(auto& p : p2) {
            size_t ss; ifs.read((char*)&ss, sizeof(ss));
            std::vector<size_t> shape(ss); ifs.read((char*)shape.data(), ss*sizeof(size_t));
            size_t ds; ifs.read((char*)&ds, sizeof(ds));
            ifs.read((char*)p->value().get_data(), ds*sizeof(float));
        }
        auto p1 = model_->get_parameters();
        for(size_t i=0; i<p1.size(); ++i) {
            const float* d1 = p1[i]->value().get_data();
            const float* d2 = p2[i]->value().get_data();
            for(size_t j=0; j<p1[i]->value().data_size(); ++j) {
                if (std::abs(d1[j] - d2[j]) > 1e-7) exit(1);
            }
        }
        std::cout << "[DIAGNOSTIC] Checkpoint Integrity: PASSED" << std::endl;
        std::remove(cp.c_str());
    }

    std::shared_ptr<Tokenizer> tokenizer_;
    std::shared_ptr<TransformerModel> model_;
    int token_count_;
    int vocab_size_;
    int batch_size_;
    int step_;
};

int main() {
    ArithmeticDebugHarness harness;
    harness.run();
    return 0;
}
