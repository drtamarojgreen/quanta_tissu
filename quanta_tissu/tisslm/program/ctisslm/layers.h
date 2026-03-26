#ifndef QUANTA_TISSU_LAYERS_H
#define QUANTA_TISSU_LAYERS_H

#include "parameter.h"
#include <vector>
#include <string>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <stdexcept>

namespace quanta_tissu {

/**
 * @brief Softmax activation function.
 */
std::vector<double> softmax(const std::vector<double>& x, double temperature = 1.0) {
    std::vector<double> result(x.size());
    double max_val = x[0];
    for (double val : x) if (val > max_val) max_val = val;
    double sum_exp = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] = std::exp((x[i] - max_val) / temperature);
        sum_exp += result[i];
    }
    for (size_t i = 0; i < x.size(); ++i) result[i] /= sum_exp;
    return result;
}

/**
 * @brief Layer Normalization module.
 */
class LayerNorm {
public:
    Parameter gamma;
    Parameter beta;
    double eps;
    std::string name;

    LayerNorm(size_t d_model, double eps = 1e-6, const std::string& layer_name = "")
        : gamma({d_model}, layer_name + ".gamma"),
          beta({d_model}, layer_name + ".beta"),
          eps(eps),
          name(layer_name) {
        std::fill(gamma.value.begin(), gamma.value.end(), 1.0);
        std::fill(beta.value.begin(), beta.value.end(), 0.0);
    }

    std::vector<double> forward(const std::vector<double>& x) {
        size_t d_model = gamma.shape[0];
        size_t seq_len = x.size() / d_model;
        std::vector<double> out(x.size());
        for (size_t s = 0; s < seq_len; ++s) {
            double mean = 0.0;
            for (size_t j = 0; j < d_model; ++j) mean += x[s * d_model + j];
            mean /= d_model;
            double var = 0.0;
            for (size_t j = 0; j < d_model; ++j) {
                double diff = x[s * d_model + j] - mean;
                var += diff * diff;
            }
            var /= d_model;
            double std_inv = 1.0 / std::sqrt(var + eps);
            for (size_t j = 0; j < d_model; ++j) {
                double x_norm = (x[s * d_model + j] - mean) * std_inv;
                out[s * d_model + j] = gamma.value[j] * x_norm + beta.value[j];
            }
        }
        return out;
    }

    std::vector<double> backward(const std::vector<double>& d_out, const std::vector<double>& cache) {
        size_t d_model = gamma.shape[0];
        size_t seq_len = d_out.size() / d_model;
        std::vector<double> dx(d_out.size());
        for (size_t s = 0; s < seq_len; ++s) {
            double mean = 0.0, var = 0.0;
            for (size_t j = 0; j < d_model; ++j) mean += cache[s * d_model + j];
            mean /= d_model;
            for (size_t j = 0; j < d_model; ++j) var += std::pow(cache[s * d_model + j] - mean, 2);
            var /= d_model;
            double inv_std = 1.0 / std::sqrt(var + eps);
            double d_mean = 0.0, d_var = 0.0;
            for (size_t j = 0; j < d_model; ++j) {
                double x_hat = (cache[s * d_model + j] - mean) * inv_std;
                gamma.grad[j] += d_out[s * d_model + j] * x_hat;
                beta.grad[j] += d_out[s * d_model + j];
                double dx_hat = d_out[s * d_model + j] * gamma.value[j];
                d_var += dx_hat * (cache[s * d_model + j] - mean) * (-0.5) * std::pow(var + eps, -1.5);
                d_mean += dx_hat * (-inv_std);
            }
            for (size_t j = 0; j < d_model; ++j) {
                double dx_hat = d_out[s * d_model + j] * gamma.value[j];
                dx[s * d_model + j] = dx_hat * inv_std + d_var * 2.0 * (cache[s * d_model + j] - mean) / d_model + d_mean / d_model;
            }
        }
        return dx;
    }

    std::vector<Parameter*> parameters() { return {&gamma, &beta}; }
};

/**
 * @brief Multi-Head Attention module.
 */
class MultiHeadAttention {
public:
    size_t d_model;
    size_t num_heads;
    size_t d_k;
    std::string name;

    Parameter Wq, Wk, Wv, Wo;

    MultiHeadAttention(size_t d_model, size_t num_heads, const std::string& layer_name = "")
        : d_model(d_model), num_heads(num_heads), d_k(d_model / num_heads), name(layer_name),
          Wq({d_model, d_model}, layer_name + ".Wq"), Wk({d_model, d_model}, layer_name + ".Wk"),
          Wv({d_model, d_model}, layer_name + ".Wv"), Wo({d_model, d_model}, layer_name + ".Wo") {
        if (d_model % num_heads != 0) throw std::runtime_error("d_model must be divisible by num_heads");
        Wq.fill_random_normal(); Wk.fill_random_normal(); Wv.fill_random_normal(); Wo.fill_random_normal();
    }

    std::vector<double> matmul(const std::vector<double>& A, const std::vector<size_t>& A_shape,
                               const std::vector<double>& B, const std::vector<size_t>& B_shape) {
        size_t M = A_shape[0], K = A_shape[1], N = B_shape[1];
        std::vector<double> C(M * N, 0.0);
        for (size_t i = 0; i < M; ++i) {
            for (size_t k = 0; k < K; ++k) {
                double a_ik = A[i * K + k];
                for (size_t j = 0; j < N; ++j) C[i * N + j] += a_ik * B[k * N + j];
            }
        }
        return C;
    }

    std::vector<double> forward(const std::vector<double>& x, const std::vector<double>& mask = {}, const std::vector<double>& /*kv_cache*/ = {}) {
        size_t seq_len = x.size() / d_model;
        std::vector<double> Q = matmul(x, {seq_len, d_model}, Wq.value, Wq.shape);
        std::vector<double> K = matmul(x, {seq_len, d_model}, Wk.value, Wk.shape);
        std::vector<double> V = matmul(x, {seq_len, d_model}, Wv.value, Wv.shape);
        std::vector<double> attended(seq_len * d_model, 0.0);
        double scale = 1.0 / std::sqrt(static_cast<double>(d_k));
        for (size_t h = 0; h < num_heads; ++h) {
            for (size_t i = 0; i < seq_len; ++i) {
                std::vector<double> scores(seq_len);
                for (size_t j = 0; j < seq_len; ++j) {
                    double dot = 0.0;
                    for (size_t k = 0; k < d_k; ++k) dot += Q[i * d_model + h * d_k + k] * K[j * d_model + h * d_k + k];
                    scores[j] = dot * scale;
                    if (!mask.empty()) scores[j] += mask[i * seq_len + j];
                }
                std::vector<double> weights = softmax(scores);
                for (size_t j = 0; j < seq_len; ++j) {
                    for (size_t k = 0; k < d_k; ++k) attended[i * d_model + h * d_k + k] += weights[j] * V[j * d_model + h * d_k + k];
                }
            }
        }
        return matmul(attended, {seq_len, d_model}, Wo.value, Wo.shape);
    }

    std::vector<double> backward(const std::vector<double>& d_out, const std::vector<double>& cache) {
        size_t seq_len = d_out.size() / d_model;

        std::vector<double> Q = matmul(cache, {seq_len, d_model}, Wq.value, Wq.shape);
        std::vector<double> K = matmul(cache, {seq_len, d_model}, Wk.value, Wk.shape);
        std::vector<double> V = matmul(cache, {seq_len, d_model}, Wv.value, Wv.shape);
        std::vector<double> attended(seq_len * d_model, 0.0);
        std::vector<std::vector<double>> head_weights(num_heads, std::vector<double>(seq_len * seq_len));
        double scale = 1.0 / std::sqrt(static_cast<double>(d_k));
        for (size_t h = 0; h < num_heads; ++h) {
            for (size_t i = 0; i < seq_len; ++i) {
                std::vector<double> scores(seq_len);
                for (size_t j = 0; j < seq_len; ++j) {
                    double dot = 0.0;
                    for (size_t k = 0; k < d_k; ++k) dot += Q[i * d_model + h * d_k + k] * K[j * d_model + h * d_k + k];
                    scores[j] = dot * scale;
                }
                std::vector<double> weights = softmax(scores);
                for (size_t j = 0; j < seq_len; ++j) {
                    head_weights[h][i * seq_len + j] = weights[j];
                    for (size_t k = 0; k < d_k; ++k) attended[i * d_model + h * d_k + k] += weights[j] * V[j * d_model + h * d_k + k];
                }
            }
        }

        for (size_t i = 0; i < d_model; ++i) {
            for (size_t j = 0; j < d_model; ++j) {
                double sum = 0.0;
                for (size_t s = 0; s < seq_len; ++s) sum += attended[s * d_model + i] * d_out[s * d_model + j];
                Wo.grad[i * d_model + j] += sum;
            }
        }

        std::vector<double> d_attended(seq_len * d_model, 0.0);
        for (size_t s = 0; s < seq_len; ++s) {
            for (size_t i = 0; i < d_model; ++i) {
                double sum = 0.0;
                for (size_t j = 0; j < d_model; ++j) sum += d_out[s * d_model + j] * Wo.value[i * d_model + j];
                d_attended[s * d_model + i] = sum;
            }
        }

        std::vector<double> dQ(seq_len * d_model, 0.0), dK(seq_len * d_model, 0.0), dV(seq_len * d_model, 0.0);

        for (size_t h = 0; h < num_heads; ++h) {
            for (size_t i = 0; i < seq_len; ++i) {
                for (size_t j = 0; j < seq_len; ++j) {
                    double w = head_weights[h][i * seq_len + j];
                    for (size_t k = 0; k < d_k; ++k) dV[j * d_model + h * d_k + k] += w * d_attended[i * d_model + h * d_k + k];

                    double d_weight = 0.0;
                    for (size_t k = 0; k < d_k; ++k) d_weight += d_attended[i * d_model + h * d_k + k] * V[j * d_model + h * d_k + k];

                    double d_score = w * d_weight * scale;
                    for (size_t k = 0; k < d_k; ++k) {
                        dQ[i * d_model + h * d_k + k] += d_score * K[j * d_model + h * d_k + k];
                        dK[j * d_model + h * d_k + k] += d_score * Q[i * d_model + h * d_k + k];
                    }
                }
            }
        }

        std::vector<double> dx(seq_len * d_model, 0.0);
        for (size_t i = 0; i < d_model; ++i) {
            for (size_t j = 0; j < d_model; ++j) {
                for (size_t s = 0; s < seq_len; ++s) {
                    Wq.grad[i * d_model + j] += cache[s * d_model + i] * dQ[s * d_model + j];
                    Wk.grad[i * d_model + j] += cache[s * d_model + i] * dK[s * d_model + j];
                    Wv.grad[i * d_model + j] += cache[s * d_model + i] * dV[s * d_model + j];
                    dx[s * d_model + i] += dQ[s * d_model + j] * Wq.value[i * d_model + j] +
                                          dK[s * d_model + j] * Wk.value[i * d_model + j] +
                                          dV[s * d_model + j] * Wv.value[i * d_model + j];
                }
            }
        }

        return dx;
    }

    std::vector<Parameter*> parameters() { return {&Wq, &Wk, &Wv, &Wo}; }
};

/**
 * @brief Feed-Forward network module.
 */
class FeedForward {
public:
    Parameter W1, b1, W2, b2;
    std::string name;

    FeedForward(size_t d_model, size_t d_ff, const std::string& layer_name = "")
        : W1({d_model, d_ff}, layer_name + ".W1"), b1({d_ff}, layer_name + ".b1"),
          W2({d_ff, d_model}, layer_name + ".W2"), b2({d_model}, layer_name + ".b2"),
          name(layer_name) {
        W1.fill_random_normal(); W2.fill_random_normal(); b1.fill_zeros(); b2.fill_zeros();
    }

    std::vector<double> forward(const std::vector<double>& x) {
        size_t d_model = W1.shape[0], d_ff = W1.shape[1], seq_len = x.size() / d_model;
        std::vector<double> out(x.size());
        for (size_t s = 0; s < seq_len; ++s) {
            std::vector<double> h(d_ff);
            for (size_t j = 0; j < d_ff; ++j) {
                double sum = 0.0;
                for (size_t i = 0; i < d_model; ++i) sum += x[s * d_model + i] * W1(i, j);
                h[j] = std::max(0.0, sum + b1.value[j]);
            }
            for (size_t j = 0; j < d_model; ++j) {
                double sum = 0.0;
                for (size_t i = 0; i < d_ff; ++i) sum += h[i] * W2(i, j);
                out[s * d_model + j] = sum + b2.value[j];
            }
        }
        return out;
    }

    std::vector<double> backward(const std::vector<double>& d_out, const std::vector<double>& cache) {
        size_t d_model = W1.shape[0], d_ff = W1.shape[1], seq_len = d_out.size() / d_model;
        std::vector<double> dx(d_out.size(), 0.0);
        for (size_t s = 0; s < seq_len; ++s) {
            std::vector<double> h(d_ff), dh(d_ff, 0.0);
            for (size_t j = 0; j < d_ff; ++j) {
                double sum = 0.0;
                for (size_t i = 0; i < d_model; ++i) sum += cache[s * d_model + i] * W1(i, j);
                h[j] = sum + b1.value[j];
            }
            for (size_t j = 0; j < d_model; ++j) {
                b2.grad[j] += d_out[s * d_model + j];
                for (size_t i = 0; i < d_ff; ++i) {
                    double activated_h = std::max(0.0, h[i]);
                    W2.grad[i * d_model + j] += activated_h * d_out[s * d_model + j];
                    dh[i] += d_out[s * d_model + j] * W2.value[i * d_model + j];
                }
            }
            for (size_t j = 0; j < d_ff; ++j) {
                if (h[j] > 0) {
                    b1.grad[j] += dh[j];
                    for (size_t i = 0; i < d_model; ++i) {
                        W1.grad[i * d_model + j] += cache[s * d_model + i] * dh[j];
                        dx[s * d_model + i] += dh[j] * W1.value[i * d_model + j];
                    }
                }
            }
        }
        return dx;
    }

    std::vector<Parameter*> parameters() { return {&W1, &b1, &W2, &b2}; }
};

}

#endif // QUANTA_TISSU_LAYERS_H
