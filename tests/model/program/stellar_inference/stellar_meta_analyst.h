#ifndef TISSLM_STELLAR_META_ANALYST_H
#define TISSLM_STELLAR_META_ANALYST_H

#include <string>
#include <vector>
#include <map>
#include "core/model_interface.h"

namespace TissLM {
namespace Stellar {

struct SourceMetrics {
    size_t line_count;
    size_t char_count;
    float complexity_index;
    std::map<char, size_t> char_distribution;
};

struct ModelMetrics {
    size_t parameter_count;
    size_t layer_count;
    float param_density;
};

/**
 * @class StellarMetaAnalyst
 * @brief Analyzes source code and model architectures.
 */
class StellarMetaAnalyst {
public:
    static SourceMetrics analyze_source(const std::string& filepath);
    static ModelMetrics analyze_model(std::shared_ptr<TissLM::Core::Model> model);
    static std::vector<struct Point3D> extract_3d_points(const SourceMetrics& metrics);
};

} // namespace Stellar
} // namespace TissLM

#endif // TISSLM_STELLAR_META_ANALYST_H
