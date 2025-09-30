#ifndef TISSLM_MODEL_INTERFACE_H
#define TISSLM_MODEL_INTERFACE_H

#include "matrix.h"
#include "parameter.h"
#include <vector>
#include <memory>

namespace TissDB {
namespace TissLM {
namespace Core {

class Model {
public:
    virtual ~Model() = default;
    virtual Matrix forward(const Matrix& input) = 0;
    virtual Matrix backward(const Matrix& grad_output) = 0;
    virtual std::vector<std::shared_ptr<Parameter>> get_parameters() = 0;
};

} // namespace Core
} // namespace TissLM
} // namespace TissDB

#endif // TISSLM_MODEL_INTERFACE_H
