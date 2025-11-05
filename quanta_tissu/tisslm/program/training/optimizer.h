#ifndef TISSLM_OPTIMIZER_H
#define TISSLM_OPTIMIZER_H

#include "core/parameter.h"
#include <vector>
#include <memory>

#include <fstream>

namespace TissLM {
namespace Training {

// Forward declaration for Parameter if needed, though it's included above
// class Parameter;

class Optimizer {
public:
    virtual ~Optimizer() = default;
    virtual void update(std::vector<TissNum::Parameter*>& parameters) = 0;
    virtual void save_state(std::ofstream& ofs) const = 0;
    virtual void load_state(std::ifstream& ifs) = 0;
};

class Adam : public Optimizer {
public:
    Adam(float learning_rate = 0.001f, float beta1 = 0.9f, float beta2 = 0.999f, float epsilon = 1e-8f);

    void update(std::vector<TissNum::Parameter*>& parameters) override;
    void save_state(std::ofstream& ofs) const override;
    void load_state(std::ifstream& ifs) override;

private:
    float learning_rate_;
    float beta1_;
    float beta2_;
    float epsilon_;

    // Per-parameter momentum and velocity estimates
    std::vector<TissNum::Matrix> m_;
    std::vector<TissNum::Matrix> v_;
    int t_ = 0; // Timestep
};

} // namespace Training
} // namespace TissLM

#endif // TISSLM_OPTIMIZER_H
