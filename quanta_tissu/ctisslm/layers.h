#ifndef QUANTA_TISSU_LAYERS_H
#define QUANTA_TISSU_LAYERS_H

#include "parameter.h"
#include <vector>
#include <string>
#include <cmath> // For std::sqrt, std::exp, etc.
#include <numeric> // For std::accumulate
#include <algorithm> // For std::max

namespace quanta_tissu {

// Helper functions (placeholders for now, will use a linear algebra library)
// These will need to be implemented with proper matrix/tensor operations

// Softmax function
std::vector<double> softmax(const std::vector<double>& x, double temperature = 1.0) {
    // Placeholder implementation
    std::vector<double> result(x.size());
    double max_val = x[0];
    for (double val : x) {
        if (val > max_val) max_val = val;
    }
    double sum_exp = 0.0;
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] = std::exp((x[i] - max_val) / temperature);
        sum_exp += result[i];
    }
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] /= sum_exp;
    }
    return result;
}

// Derivative of ReLU
std::vector<double> d_relu(const std::vector<double>& x, const std::vector<double>& d_out) {
    // Placeholder implementation
    std::vector<double> result(x.size());
    for (size_t i = 0; i < x.size(); ++i) {
        result[i] = d_out[i] * (x[i] > 0 ? 1.0 : 0.0);
    }
    return result;
}

// LayerNorm class
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
          name(layer_name) {}

    // Forward pass
    std::vector<double> forward(const std::vector<double>& x) {
        // Assuming x is a 1D vector for simplicity, representing a single feature vector.
        // For batch processing, this would need to handle 2D or higher dimensional tensors.
        size_t d_model = x.size();

        double mean = 0.0;
        for (double val : x) {
            mean += val;
        }
        mean /= d_model;

        double var = 0.0;
        for (double val : x) {
            var += (val - mean) * (val - mean);
        }
        var /= d_model;

        std::vector<double> x_norm(d_model);
        for (size_t i = 0; i < d_model; ++i) {
            x_norm[i] = (x[i] - mean) / std::sqrt(var + eps);
        }

        std::vector<double> out(d_model);
        for (size_t i = 0; i < d_model; ++i) {
            out[i] = gamma.value[i] * x_norm[i] + beta.value[i];
        }

        // Cache for backward pass (simplified for now)
        // In a real implementation, you'd store x, x_norm, mean, var, etc.
        // For now, we'll just return the output.
        // The Python version returns a cache, so we need to adapt this.
        // For now, we'll return 'out' and assume 'cache' is handled externally or passed by reference.
        // This will need to be revisited when implementing the full TransformerBlock.
        return out;
    }

    // Backward pass (simplified for now, needs full implementation later)
    std::vector<double> backward(const std::vector<double>& d_out, const std::vector<double>& cache) {
        // This is a highly simplified placeholder.
        // The full backward pass for LayerNorm is complex and involves
        // gradients for mean, variance, gamma, beta, and x.
        // It will require the cached values from the forward pass.
        // For now, just return d_out.
        return d_out;
    }

    std::vector<Parameter*> parameters() {
        return {&gamma, &beta};
    }
};

// MultiHeadAttention class
class MultiHeadAttention {
public:
    size_t d_model;
    size_t num_heads;
    size_t d_k;
    std::string name;

    Parameter Wq;
    Parameter Wk;
    Parameter Wv;
    Parameter Wo;

    MultiHeadAttention(size_t d_model, size_t num_heads, const std::string& layer_name = "")
        : d_model(d_model),
          num_heads(num_heads),
          d_k(d_model / num_heads),
          name(layer_name),
          Wq({d_model, d_model}, layer_name + ".Wq"),
          Wk({d_model, d_model}, layer_name + ".Wk"),
          Wv({d_model, d_model}, layer_name + ".Wv"),
          Wo({d_model, d_model}, layer_name + ".Wo") {
        // Assert d_model is divisible by num_heads
    }

    // Helper for matrix multiplication (A @ B)
    // A_shape: {M, K}, B_shape: {K, N}
    std::vector<double> matmul(const std::vector<double>& A, const std::vector<size_t>& A_shape,
                               const std::vector<double>& B, const std::vector<size_t>& B_shape) {
        size_t M = A_shape[0];
        size_t K_A = A_shape[1];
        size_t K_B = B_shape[0];
        size_t N = B_shape[1];

        if (K_A != K_B) {
            throw std::runtime_error("Matrix multiplication dimension mismatch.");
        }

        std::vector<double> C(M * N, 0.0);
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                for (size_t k = 0; k < K_A; ++k) {
                    C[i * N + j] += A[i * K_A + k] * B[k * N + j];
                }
            }
        }
        return C;
    }

    // Helper for transpose (for 2D matrices)
    // A_shape: {M, N}
    std::vector<double> transpose(const std::vector<double>& A, const std::vector<size_t>& A_shape) {
        size_t M = A_shape[0];
        size_t N = A_shape[1];
        std::vector<double> A_T(N * M);
        for (size_t i = 0; i < M; ++i) {
            for (size_t j = 0; j < N; ++j) {
                A_T[j * M + i] = A[i * N + j];
            }
        }
        return A_T;
    }

    // Helper for splitting heads
    // x_shape: {batch_size, seq_len, d_model}
    // Returns: {batch_size, num_heads, seq_len, d_k}
    std::vector<double> split_heads(const std::vector<double>& x, const std::vector<size_t>& x_shape) {
        size_t batch_size = x_shape[0];
        size_t seq_len = x_shape[1];
        size_t d_model = x_shape[2];

        std::vector<double> reshaped_x(batch_size * num_heads * seq_len * d_k);

        for (size_t b = 0; b < batch_size; ++b) {
            for (size_t s = 0; s < seq_len; ++s) {
                for (size_t h = 0; h < num_heads; ++h) {
                    for (size_t k = 0; k < d_k; ++k) {
                        reshaped_x[b * num_heads * seq_len * d_k + h * seq_len * d_k + s * d_k + k] = 
                            x[b * seq_len * d_model + s * d_model + h * d_k + k];
                    }
                }
            }
        }
        return reshaped_x;
    }

    // Helper for combining heads
    // x_shape: {batch_size, num_heads, seq_len, d_k}
    // Returns: {batch_size, seq_len, d_model}
    std::vector<double> combine_heads(const std::vector<double>& x, const std::vector<size_t>& x_shape) {
        size_t batch_size = x_shape[0];
        size_t num_heads = x_shape[1];
        size_t seq_len = x_shape[2];
        size_t d_k = x_shape[3];

        std::vector<double> combined_x(batch_size * seq_len * d_model);

        for (size_t b = 0; b < batch_size; ++b) {
            for (size_t s = 0; s < seq_len; ++s) {
                for (size_t h = 0; h < num_heads; ++h) {
                    for (size_t k = 0; k < d_k; ++k) {
                        combined_x[b * seq_len * d_model + s * d_model + h * d_k + k] = 
                            x[b * num_heads * seq_len * d_k + h * seq_len * d_k + s * d_k + k];
                    }
                }
            }
        }
        return combined_x;
    }

    // Helper for scaled_dot_product_attention
    // Q_shape: {batch_size, num_heads, seq_len_Q, d_k}
    // K_shape: {batch_size, num_heads, seq_len_K, d_k}
    // V_shape: {batch_size, num_heads, seq_len_V, d_k}
    // mask_shape: {batch_size, num_heads, seq_len_Q, seq_len_K}
    // Returns: {output, weights}
    std::pair<std::vector<double>, std::vector<double>> scaled_dot_product_attention(
        const std::vector<double>& Q, const std::vector<size_t>& Q_shape,
        const std::vector<double>& K, const std::vector<size_t>& K_shape,
        const std::vector<double>& V, const std::vector<size_t>& V_shape,
        const std::vector<double>& mask, const std::vector<size_t>& mask_shape) {

        size_t batch_size = Q_shape[0];
        size_t num_heads = Q_shape[1];
        size_t seq_len_Q = Q_shape[2];
        size_t d_k_Q = Q_shape[3];
        size_t seq_len_K = K_shape[2];
        size_t d_k_K = K_shape[3];
        size_t seq_len_V = V_shape[2];
        size_t d_k_V = V_shape[3];

        // scores = Q @ K.transpose() / sqrt(d_k)
        // K_T shape: {batch_size, num_heads, d_k, seq_len_K}
        std::vector<double> K_T(batch_size * num_heads * d_k_K * seq_len_K);
        for (size_t b = 0; b < batch_size; ++b) {
            for (size_t h = 0; h < num_heads; ++h) {
                for (size_t i = 0; i < d_k_K; ++i) {
                    for (size_t j = 0; j < seq_len_K; ++j) {
                        K_T[b * num_heads * d_k_K * seq_len_K + h * d_k_K * seq_len_K + i * seq_len_K + j] = 
                            K[b * num_heads * seq_len_K * d_k_K + h * seq_len_K * d_k_K + j * d_k_K + i];
                    }
                }
            }
        }

        std::vector<double> scores(batch_size * num_heads * seq_len_Q * seq_len_K, 0.0);
        for (size_t b = 0; b < batch_size; ++b) {
            for (size_t h = 0; h < num_heads; ++h) {
                for (size_t i = 0; i < seq_len_Q; ++i) {
                    for (size_t j = 0; j < seq_len_K; ++j) {
                        double sum = 0.0;
                        for (size_t k = 0; k < d_k_Q; ++k) {
                            sum += Q[b * num_heads * seq_len_Q * d_k_Q + h * seq_len_Q * d_k_Q + i * d_k_Q + k] *
                                   K_T[b * num_heads * d_k_K * seq_len_K + h * d_k_K * seq_len_K + k * seq_len_K + j];
                        }
                        scores[b * num_heads * seq_len_Q * seq_len_K + h * seq_len_Q * seq_len_K + i * seq_len_K + j] = sum / std::sqrt(static_cast<double>(d_k_Q));
                    }
                }
            }
        }

        // Apply mask
        if (!mask.empty()) {
            for (size_t i = 0; i < scores.size(); ++i) {
                scores[i] += mask[i];
            }
        }

        // weights = softmax(scores)
        std::vector<double> weights(scores.size());
        for (size_t b = 0; b < batch_size; ++b) {
            for (size_t h = 0; h < num_heads; ++h) {
                for (size_t i = 0; i < seq_len_Q; ++i) {
                    std::vector<double> row_scores(seq_len_K);
                    for (size_t j = 0; j < seq_len_K; ++j) {
                        row_scores[j] = scores[b * num_heads * seq_len_Q * seq_len_K + h * seq_len_Q * seq_len_K + i * seq_len_K + j];
                    }
                    std::vector<double> row_weights = softmax(row_scores);
                    for (size_t j = 0; j < seq_len_K; ++j) {
                        weights[b * num_heads * seq_len_Q * seq_len_K + h * seq_len_Q * seq_len_K + i * seq_len_K + j] = row_weights[j];
                    }
                }
            }
        }

        // output = weights @ V
        std::vector<double> output(batch_size * num_heads * seq_len_Q * d_k_V, 0.0);
        for (size_t b = 0; b < batch_size; ++b) {
            for (size_t h = 0; h < num_heads; ++h) {
                for (size_t i = 0; i < seq_len_Q; ++i) {
                    for (size_t j = 0; j < d_k_V; ++j) {
                        double sum = 0.0;
                        for (size_t k = 0; k < seq_len_K; ++k) {
                            sum += weights[b * num_heads * seq_len_Q * seq_len_K + h * seq_len_Q * seq_len_K + i * seq_len_K + k] *
                                   V[b * num_heads * seq_len_V * d_k_V + h * seq_len_V * d_k_V + k * d_k_V + j];
                        }
                        output[b * num_heads * seq_len_Q * d_k_V + h * seq_len_Q * d_k_V + i * d_k_V + j] = sum;
                    }
                }
            }
        }
        return {output, weights};
    }

    // Forward pass
    std::vector<double> forward(const std::vector<double>& x, const std::vector<double>& mask = {}, const std::vector<double>& kv_cache = {}) {
        // Assuming x is (batch_size, seq_len, d_model)
        size_t batch_size = 1; // For simplicity, assuming batch_size = 1
        size_t seq_len = x.size() / d_model;

        // Q = x @ Wq.value
        std::vector<double> Q_proj = matmul(x, {batch_size * seq_len, d_model}, Wq.value, Wq.shape);
        
        // K_proj = x @ Wk.value
        std::vector<double> K_proj = matmul(x, {batch_size * seq_len, d_model}, Wk.value, Wk.shape);
        
        // V_proj = x @ Wv.value
        std::vector<double> V_proj = matmul(x, {batch_size * seq_len, d_model}, Wv.value, Wv.shape);

        // Reshape to {batch_size, seq_len, d_model}
        Q_proj.reshape({batch_size, seq_len, d_model});
        K_proj.reshape({batch_size, seq_len, d_model});
        V_proj.reshape({batch_size, seq_len, d_model});

        // Qh, Kh_new, Vh_new = split_heads(Q), split_heads(K_proj), split_heads(V_proj)
        std::vector<double> Qh = split_heads(Q_proj, {batch_size, seq_len, d_model});
        std::vector<double> Kh_new = split_heads(K_proj, {batch_size, seq_len, d_model});
        std::vector<double> Vh_new = split_heads(V_proj, {batch_size, seq_len, d_model});

        // kv_cache handling (simplified)
        std::vector<double> Kh = Kh_new;
        std::vector<double> Vh = Vh_new;

        // attended, attention_weights = scaled_dot_product_attention(Qh, Kh, Vh, mask=mask)
        std::pair<std::vector<double>, std::vector<double>> attention_output =
            scaled_dot_product_attention(Qh, {batch_size, num_heads, seq_len, d_k},
                                         Kh, {batch_size, num_heads, seq_len, d_k},
                                         Vh, {batch_size, num_heads, seq_len, d_k},
                                         mask, {batch_size, num_heads, seq_len, seq_len}); // Assuming square mask

        std::vector<double> attended = attention_output.first;
        std::vector<double> attention_weights = attention_output.second;

        // combined = combine_heads(attended)
        std::vector<double> combined = combine_heads(attended, {batch_size, num_heads, seq_len, d_k});

        // output = combined @ Wo.value
        std::vector<double> output = matmul(combined, {batch_size * seq_len, d_model}, Wo.value, Wo.shape);
        output.reshape({batch_size, seq_len, d_model});

        // Cache for backward pass (simplified)
        // This will need to store all intermediate values for backprop.
        return output;
    }

    // Backward pass (placeholder)
    std::vector<double> backward(const std::vector<double>& d_out, const std::vector<double>& cache) {
        // This is a highly simplified placeholder.
        // The full backward pass for MultiHeadAttention is very complex.
        return d_out;
    }

    std::vector<Parameter*> parameters() {
        return {&Wq, &Wk, &Wv, &Wo};
    }
};

// FeedForward class
class FeedForward {
public:
    Parameter W1;
    Parameter b1;
    Parameter W2;
    Parameter b2;
    std::string name;

    FeedForward(size_t d_model, size_t d_ff, const std::string& layer_name = "")
        : W1({d_model, d_ff}, layer_name + ".W1"),
          b1({d_ff}, layer_name + ".b1"),
          W2({d_ff, d_model}, layer_name + ".W2"),
          b2({d_model}, layer_name + ".b2"),
          name(layer_name) {}

    // Forward pass
    std::vector<double> forward(const std::vector<double>& x) {
        // Assuming x is a 1D vector for simplicity.
        // This will need to be adapted for batch processing (2D input).
        size_t d_model = x.size();
        size_t d_ff = W1.shape[1];

        // z = x @ W1 + b1
        std::vector<double> z(d_ff);
        for (size_t j = 0; j < d_ff; ++j) {
            double sum = 0.0;
            for (size_t i = 0; i < d_model; ++i) {
                sum += x[i] * W1(i, j);
            }
            z[j] = sum + b1.value[j];
        }

        // h = np.maximum(0, z) (ReLU)
        std::vector<double> h(d_ff);
        for (size_t i = 0; i < d_ff; ++i) {
            h[i] = std::max(0.0, z[i]);
        }

        // y = h @ W2 + b2
        std::vector<double> y(d_model);
        for (size_t j = 0; j < d_model; ++j) {
            double sum = 0.0;
            for (size_t i = 0; i < d_ff; ++i) {
                sum += h[i] * W2(i, j);
            }
            y[j] = sum + b2.value[j];
        }

        // In a real implementation, you'd store x, z, h for backward pass.
        // For now, we'll just return y.
        return y;
    }

    // Backward pass (simplified for now, needs full implementation later)
    std::vector<double> backward(const std::vector<double>& d_out, const std::vector<double>& cache) {
        // This is a highly simplified placeholder.
        // The full backward pass for FeedForward is complex and involves
        // gradients for W1, b1, W2, b2, and x.
        // It will require the cached values from the forward pass.
        // For now, just return d_out.
        return d_out;
    }

    std::vector<Parameter*> parameters() {
        return {&W1, &b1, &W2, &b2};
    }
};

} // namespace quanta_tissu

#endif // QUANTA_TISSU_LAYERS_H
