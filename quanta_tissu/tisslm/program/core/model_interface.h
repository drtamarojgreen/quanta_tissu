#ifndef TISSLM_MODEL_INTERFACE_H
#define TISSLM_MODEL_INTERFACE_H

#include "matrix.h"
#include "parameter.h"
#include <vector>
#include <memory>

namespace TissLM {
namespace Core {

class Model {
public:
    virtual ~Model() = default;
    virtual TissNum::Matrix forward(const TissNum::Matrix& input) = 0;
    virtual void backward(const TissNum::Matrix& grad_output) = 0;
    virtual std::vector<std::shared_ptr<TissNum::Parameter>> get_parameters() = 0;
    virtual std::vector<std::vector<float>> get_embeddings_as_vectors() const = 0;
    virtual int get_vocab_size() const = 0;
};

} // namespace Core
} // namespace TissLM

#endif // TISSLM_MODEL_INTERFACE_H
