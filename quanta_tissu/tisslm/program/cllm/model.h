#ifndef CLLM_MODEL_H
#define CLLM_MODEL_H

#include "config.h"
#include "transformer_block.h"
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace cllm {

class Model {
public:
    explicit Model(const ModelConfig& config);
    Eigen::MatrixXf forward(const std::vector<int>& input_tokens);

    bool load_weights(const std::string& path);
    bool save_weights(const std::string& path);

private:
    void add_positional_encoding(Eigen::MatrixXf& input);
    ModelConfig config_;
    Eigen::MatrixXf token_embeddings_;
    std::vector<std::unique_ptr<TransformerBlock>> layers_;
    Eigen::MatrixXf output_layer_weight_;
};

}

#endif // CLLM_MODEL_H
