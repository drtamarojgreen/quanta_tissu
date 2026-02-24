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

#include "core/transformer_model.h"
#include "tokenizer/tokenizer.h"
#include "training/trainer.h"
#include "training/dataset.h"
#include "training/optimizer.h"
#include "training/loss_function.h"

using namespace TissLM::Core;
using namespace TissLM::Tokenizer;
using namespace TissLM::Training;
using namespace TissNum;

// --- Floating Point Exception Handler ---
void fpe_handler(int sig) {
    std::cerr << "\n[CRITICAL] Floating point exception caught (SIGFPE)! This indicates a hardware-level arithmetic error." << std::endl;
    exit(sig);
}

void setup_fpe_traps() {
#ifdef __linux__
    signal(SIGFPE, fpe_handler);
    feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
    std::cout << "[INFO] Hardware FP traps enabled (DivByZero, Invalid, Overflow)." << std::endl;
#else
    std::cout << "[WARNING] Hardware FP traps not supported on this platform." << std::endl;
#endif
}

void breadcrumb(const std::string& msg) {
    std::cout << "[BREADCRUMB] " << msg << std::endl;
}

// --- Diagnostic Loss Wrapper ---
class DiagnosticLoss : public LossFunction {
public:
    DiagnosticLoss(std::shared_ptr<LossFunction> base) : base_(base) {}

    float compute_loss(const Matrix& predictions, const Matrix& targets) override {
        float loss = base_->compute_loss(predictions, targets);
        std::cout << "[DIAGNOSTIC] Computed Loss: " << std::fixed << std::setprecision(6) << loss << std::endl;
        if (!std::isfinite(loss)) {
            std::cerr << "[ERROR] Non-finite loss detected!" << std::endl;
            exit(1);
        }
        return loss;
    }

    Matrix compute_gradient(const Matrix& predictions, const Matrix& targets) override {
        return base_->compute_gradient(predictions, targets);
    }

private:
    std::shared_ptr<LossFunction> base_;
};

// --- Diagnostic Optimizer Wrapper (Non-Invasive) ---
class DiagnosticOptimizer : public Optimizer {
public:
    DiagnosticOptimizer(std::shared_ptr<Optimizer> base) : base_(base), step_(0) {}

    void update(std::vector<Parameter*>& parameters) override {
        step_++;
        std::cout << "\n--- Diagnostic Trace (Step " << step_ << ") ---" << std::endl;

        std::vector<Matrix> vals_before;
        for (size_t i = 0; i < parameters.size(); ++i) {
            Parameter* p = parameters[i];
            const Matrix& grad = p->grad();

            float sum_sq = 0.0f;
            float max_grad = 0.0f;
            const float* grad_data = grad.get_data();
            for (size_t j = 0; j < grad.data_size(); ++j) {
                float val = grad_data[j];
                sum_sq += val * val;
                max_grad = std::max(max_grad, std::abs(val));
            }
            float l2_norm = std::sqrt(sum_sq);

            if (step_ % 10 == 1 || !std::isfinite(l2_norm)) {
                std::cout << "  Param " << i << " [" << p->name() << "]: GradNorm=" << std::scientific << std::setprecision(4) << l2_norm
                          << ", MaxGrad=" << max_grad << std::defaultfloat << std::endl;
            }

            if (!std::isfinite(l2_norm)) {
                std::cerr << "[ERROR] Non-finite gradient detected in Param " << i << std::endl;
                exit(1);
            }
            vals_before.push_back(p->value());
        }

        // Perform the actual update on the full parameter set
        base_->update(parameters);

        for (size_t i = 0; i < parameters.size(); ++i) {
            const Matrix& val_after = parameters[i]->value();
            const Matrix& val_before = vals_before[i];
            float max_delta = 0.0f;
            const float* d_after = val_after.get_data();
            const float* d_before = val_before.get_data();
            for(size_t j=0; j<val_after.data_size(); ++j) {
                max_delta = std::max(max_delta, std::abs(d_after[j] - d_before[j]));
            }

            if (!std::isfinite(max_delta)) {
                std::cerr << "[ERROR] Non-finite values detected in Param " << i << " after update!" << std::endl;
                exit(1);
            }
        }
        std::cout << "--- End Trace ---" << std::endl;
    }

    void save_state(std::ofstream& ofs) const override { base_->save_state(ofs); }
    void load_state(std::ifstream& ifs) override { base_->load_state(ifs); }

private:
    std::shared_ptr<Optimizer> base_;
    int step_;
};

bool compare_models(TransformerModel& m1, TransformerModel& m2) {
    auto p1 = m1.get_parameters();
    auto p2 = m2.get_parameters();

    if(p1.size() != p2.size()) return false;
    for(size_t i=0; i<p1.size(); ++i) {
        auto& mat1 = p1[i]->value();
        auto& mat2 = p2[i]->value();
        if(mat1.data_size() != mat2.data_size()) return false;
        const float* d1 = mat1.get_data();
        const float* d2 = mat2.get_data();
        for(size_t j=0; j<mat1.data_size(); ++j) {
            if(std::abs(d1[j] - d2[j]) > 1e-6) return false;
        }
    }
    return true;
}

void run_debug_harness() {
    breadcrumb("Starting Deterministic Debug Harness");
    setup_fpe_traps();

    // 1. Synthetic Dataset (In-Memory)
    breadcrumb("Initializing Tokenizer");
    std::string synthetic_corpus = "the quick brown fox jumps over the lazy dog. ";
    synthetic_corpus += "cognitive behavioral therapy focuses on cognitive distortions. ";

    auto tokenizer = std::make_shared<Tokenizer>("");
    tokenizer->train(synthetic_corpus, 256, false);

    std::vector<int> tokens = tokenizer->encode(synthetic_corpus);
    TokenDataset dataset(tokens, 4);
    std::cout << "[INFO] Vocab Size: " << tokenizer->get_vocab_size()
              << ", Token Count: " << tokens.size() << std::endl;

    // 2. Model & Trainer Setup
    breadcrumb("Initializing Model & Diagnostic Components");
    auto model = std::make_shared<TransformerModel>(tokenizer->get_vocab_size(), 8, 32, 2, 2, 64, 0.0f, 0);
    auto real_adam = std::make_shared<Adam>(1e-3);
    auto diagnostic_optimizer = std::make_shared<DiagnosticOptimizer>(real_adam);
    auto real_loss_fn = std::make_shared<CrossEntropyLoss>();
    auto diagnostic_loss_fn = std::make_shared<DiagnosticLoss>(real_loss_fn);

    // NOTE: TransformerModel current forward() implementation only supports batch_size = 1 correctly.
    Trainer trainer(model, diagnostic_optimizer, diagnostic_loss_fn);

    // 3. Training Loop
    breadcrumb("Executing Training Steps");
    trainer.train(dataset, 1, 1, 0, "");
    breadcrumb("Training Finished");

    // 4. Checkpoint Verification
    breadcrumb("Verifying Checkpointing");
    std::string cp_path = "debug_checkpoint_harness.bin";
    trainer.save_checkpoint(cp_path);

    auto model2 = std::make_shared<TransformerModel>(tokenizer->get_vocab_size(), 8, 32, 2, 2, 64, 0.0f, 0);
    Trainer trainer2(model2, real_adam, real_loss_fn);
    trainer2.load_checkpoint(cp_path);

    if (compare_models(*model, *model2)) {
        std::cout << "[SUCCESS] Checkpoint Load/Save Roundtrip Verified." << std::endl;
    } else {
        std::cerr << "[FAILURE] Checkpoint Data Mismatch!" << std::endl;
        exit(1);
    }

    std::remove(cp_path.c_str());
    breadcrumb("Harness Completed Successfully");
}

int main() {
    try {
        run_debug_harness();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[ABORT] Harness failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
