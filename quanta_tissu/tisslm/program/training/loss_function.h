#ifndef TISSLM_LOSS_FUNCTION_H
#define TISSLM_LOSS_FUNCTION_H

#include "core/matrix.h"
#include <vector>
#include <cmath>

namespace TissDB {
namespace TissLM {
namespace Core {

class LossFunction {
public:
    virtual ~LossFunction() = default;
    virtual float compute_loss(const TissNum::Matrix& predictions, const TissNum::Matrix& targets) = 0;
    virtual TissNum::Matrix compute_gradient(const TissNum::Matrix& predictions, const TissNum::Matrix& targets) = 0;
};

class CrossEntropyLoss : public LossFunction {
public:
    float compute_loss(const TissNum::Matrix& predictions, const TissNum::Matrix& targets) override;
    TissNum::Matrix compute_gradient(const TissNum::Matrix& predictions, const TissNum::Matrix& targets) override;

private:
    // Helper to compute softmax, as it's often coupled with Cross-Entropy
    TissNum::Matrix softmax(const TissNum::Matrix& input);
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_LOSS_FUNCTION_H
