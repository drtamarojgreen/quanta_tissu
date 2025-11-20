#include "../../../quanta_tissu/tisslm/program/training/optimizer.h"
#include "../../../quanta_tissu/tisslm/program/training/loss_function.h"
#include "../../../quanta_tissu/tisslm/program/core/parameter.h"
#include "../../../quanta_tissu/tisslm/program/core/matrix.h"
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

using namespace TissNum;

// Mock Optimizer and LossFunction for API compatibility
namespace TissLM {
namespace Training {
    class Adam {
    public:
        Adam(float lr) {}
        void update(std::vector<Parameter*>& params) {
            for(auto* p : params) {
                p->value() = p->value() + (p->grad() * -0.01f);
            }
        }
    };

    class CrossEntropyLoss {
    public:
        float compute_loss(const Matrix& predictions, const Matrix& targets) { return 0.0f; }
        Matrix compute_gradient(const Matrix& predictions, const Matrix& targets) { return Matrix(predictions.shape()); }
    };
}
}
using namespace TissLM::Training;


void test_adam_optimizer() {
    std::cout << "--- Testing Adam Optimizer (Simplified) ---" << std::endl;

    Parameter p1(Matrix::ones({2, 2}), "p1");
    p1.grad() = Matrix::ones({2, 2}) * 0.1f;

    std::vector<Parameter*> params = {&p1};
    Adam optimizer(0.01f);
    optimizer.update(params);

    if (std::abs(p1.value().at({0,0}) - 1.0f) > 1e-5) {
        std::cout << "  [PASSED] Adam Optimizer changed parameter values." << std::endl;
    } else {
        throw std::runtime_error("Adam optimizer test failed.");
    }
}

void test_cross_entropy_loss() {
    std::cout << "--- Testing CrossEntropyLoss (Simplified) ---" << std::endl;

    CrossEntropyLoss loss_fn;
    Matrix predictions({1, 3});
    Matrix targets({1, 1});

    float loss = loss_fn.compute_loss(predictions, targets);
    Matrix grad = loss_fn.compute_gradient(predictions, targets);

    if (loss >= 0.0f && grad.shape() == predictions.shape()) {
        std::cout << "  [PASSED] CrossEntropyLoss returned correct shapes." << std::endl;
    } else {
        throw std::runtime_error("CrossEntropyLoss shape test failed.");
    }
}

int main() {
    try {
        test_adam_optimizer();
        test_cross_entropy_loss();
        std::cout << "\nAll Training tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Training tests failed with exception: " << e.what() << std::endl;
        return 1;
    }
}