
import unittest
import numpy as np
from unittest.mock import MagicMock, patch

# Adjust path to import modules from the project root
import os
import sys
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..'))
sys.path.insert(0, project_root)

from quanta_tissu.tisslm.core.layers import LayerNorm, MultiHeadAttention, FeedForward, scaled_dot_product_attention, backward_scaled_dot_product_attention
from quanta_tissu.tisslm.core.parameter import Parameter

class TestLayerNorm(unittest.TestCase):

    def test_forward_pass(self):
        d_model = 10
        ln = LayerNorm(d_model)
        x = np.random.randn(2, 5, d_model) # Batch, Seq_len, d_model
        
        out, cache = ln(x)
        
        self.assertEqual(out.shape, x.shape)
        self.assertIsNotNone(cache)
        self.assertIn('x', cache)
        self.assertIn('x_norm', cache)
        self.assertIn('mean', cache)
        self.assertIn('var', cache)

        # Manual calculation for verification
        mean = x.mean(axis=-1, keepdims=True)
        var = x.var(axis=-1, keepdims=True)
        x_norm_manual = (x - mean) / np.sqrt(var + ln.eps)
        out_manual = ln.gamma.value * x_norm_manual + ln.beta.value
        
        np.testing.assert_allclose(out, out_manual, atol=1e-6)

    def test_backward_pass_numerical_gradient(self):
        d_model = 5
        ln = LayerNorm(d_model)
        x = np.random.randn(1, 3, d_model) # Batch, Seq_len, d_model
        
        # Initialize gradients to zero for parameters
        ln.gamma.grad = np.zeros_like(ln.gamma.value)
        ln.beta.grad = np.zeros_like(ln.beta.value)

        # --- Test gradient for x ---
        # Numerical gradient for x
        epsilon = 1e-4
        grad_x_numerical = np.zeros_like(x)
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    x_plus = x.copy()
                    x_plus[i, j, k] += epsilon
                    out_plus, _ = ln(x_plus)
                    loss_plus = np.sum(out_plus) # Simple sum as loss

                    x_minus = x.copy()
                    x_minus[i, j, k] -= epsilon
                    out_minus, _ = ln(x_minus)
                    loss_minus = np.sum(out_minus) # Simple sum as loss

                    grad_x_numerical[i, j, k] = (loss_plus - loss_minus) / (2 * epsilon)

        # Analytical gradient for x
        out, cache = ln(x)
        d_out = np.ones_like(out) # Gradient of sum(out) w.r.t. out is ones
        grad_x_analytical = ln.backward(d_out, cache)
        
        np.testing.assert_allclose(grad_x_numerical, grad_x_analytical, rtol=1e-3, atol=1e-4)

        # --- Test gradients for gamma and beta ---
        # Reset gradients for parameters
        ln.gamma.grad = np.zeros_like(ln.gamma.value)
        ln.beta.grad = np.zeros_like(ln.beta.value)

        # Numerical gradient for gamma
        grad_gamma_numerical = np.zeros_like(ln.gamma.value)
        for i in range(ln.gamma.value.shape[0]):
            gamma_orig = ln.gamma.value[i]
            
            ln.gamma.value[i] = gamma_orig + epsilon
            out_plus, _ = ln(x)
            loss_plus = np.sum(out_plus)

            ln.gamma.value[i] = gamma_orig - epsilon
            out_minus, _ = ln(x)
            loss_minus = np.sum(out_minus)

            grad_gamma_numerical[i] = (loss_plus - loss_minus) / (2 * epsilon)
            ln.gamma.value[i] = gamma_orig # Restore original value

        # Numerical gradient for beta
        grad_beta_numerical = np.zeros_like(ln.beta.value)
        for i in range(ln.beta.value.shape[0]):
            beta_orig = ln.beta.value[i]
            
            ln.beta.value[i] = beta_orig + epsilon
            out_plus, _ = ln(x)
            loss_plus = np.sum(out_plus)

            ln.beta.value[i] = beta_orig - epsilon
            out_minus, _ = ln(x)
            loss_minus = np.sum(out_minus)

            grad_beta_numerical[i] = (loss_plus - loss_minus) / (2 * epsilon)
            ln.beta.value[i] = beta_orig # Restore original value

        # Analytical gradients for gamma and beta
        out, cache = ln(x)
        d_out = np.ones_like(out)
        ln.backward(d_out, cache) # This will accumulate gradients in ln.gamma.grad and ln.beta.grad
        
        np.testing.assert_allclose(grad_gamma_numerical, ln.gamma.grad, rtol=1e-3, atol=1e-4)
        np.testing.assert_allclose(grad_beta_numerical, ln.beta.grad, rtol=1e-3, atol=1e-4)

    def test_forward_pass_edge_cases(self):
        # Test with different batch sizes and sequence lengths
        ln = LayerNorm(5)
        x_single_batch = np.random.randn(1, 10, 5)
        out, _ = ln(x_single_batch)
        self.assertEqual(out.shape, x_single_batch.shape)

        x_single_seq = np.random.randn(3, 1, 5)
        out, _ = ln(x_single_seq)
        self.assertEqual(out.shape, x_single_seq.shape)

        x_single_element = np.random.randn(1, 1, 5)
        out, _ = ln(x_single_element)
        self.assertEqual(out.shape, x_single_element.shape)

        # Test with zero variance (all elements in a feature dimension are same)
        ln_zero_var = LayerNorm(3)
        x_zero_var = np.array([[[1.0, 1.0, 1.0], [2.0, 2.0, 2.0]]])
        out, _ = ln_zero_var(x_zero_var)
        # Expected output for zero variance: (x - mean) / sqrt(eps) * gamma + beta
        # For [1,1,1], mean=1, var=0. (1-1)/sqrt(eps) = 0. So output should be beta.
        # For [2,2,2], mean=2, var=0. (2-2)/sqrt(eps) = 0. So output should be beta.
        np.testing.assert_allclose(out, np.array([[[0.0, 0.0, 0.0], [0.0, 0.0, 0.0]]]), atol=1e-6) # beta is zeros

class TestMultiHeadAttention(unittest.TestCase):

    def test_forward_pass(self):
        d_model = 32
        num_heads = 4
        mha = MultiHeadAttention(d_model, num_heads)
        x = np.random.randn(2, 5, d_model) # Batch, Seq_len, d_model

        output, cache = mha(x)

        self.assertEqual(output.shape, x.shape)
        self.assertIsNotNone(cache)
        self.assertIn('x', cache)
        self.assertIn('Qh', cache)
        self.assertIn('Kh', cache)
        self.assertIn('Vh', cache)
        self.assertIn('attention_weights', cache)
        self.assertIn('combined', cache)
        self.assertIn('Wq', cache)
        self.assertIn('Wk', cache)
        self.assertIn('Wv', cache)
        self.assertIn('Wo', cache)

    def test_backward_pass_numerical_gradient(self):
        d_model = 16
        num_heads = 2
        mha = MultiHeadAttention(d_model, num_heads)
        x = np.random.randn(1, 3, d_model) # Batch, Seq_len, d_model

        # Initialize gradients to zero for parameters
        for p in mha.parameters():
            p.grad = np.zeros_like(p.value)

        epsilon = 1e-4

        # --- Test gradient for x ---
        grad_x_numerical = np.zeros_like(x)
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    x_plus = x.copy()
                    x_plus[i, j, k] += epsilon
                    out_plus, _ = mha(x_plus)
                    loss_plus = np.sum(out_plus)

                    x_minus = x.copy()
                    x_minus[i, j, k] -= epsilon
                    out_minus, _ = mha(x_minus)
                    loss_minus = np.sum(out_minus)

                    grad_x_numerical[i, j, k] = (loss_plus - loss_minus) / (2 * epsilon)

        output, cache = mha(x)
        d_out = np.ones_like(output)
        grad_x_analytical = mha.backward(d_out, cache)
        
        np.testing.assert_allclose(grad_x_numerical, grad_x_analytical, rtol=1e-3, atol=1e-4)

        # --- Test gradients for parameters (Wq, Wk, Wv, Wo) ---
        param_names = ['Wq', 'Wk', 'Wv', 'Wo']
        params = [mha.Wq, mha.Wk, mha.Wv, mha.Wo]

        for param, name in zip(params, param_names):
            param.grad = np.zeros_like(param.value) # Reset gradient

            grad_numerical = np.zeros_like(param.value)
            it = np.nditer(param.value, flags=['multi_index'], op_flags=['readwrite'])
            while not it.finished:
                idx = it.multi_index
                original_value = param.value[idx]

                param.value[idx] = original_value + epsilon
                out_plus, _ = mha(x)
                loss_plus = np.sum(out_plus)

                param.value[idx] = original_value - epsilon
                out_minus, _ = mha(x)
                loss_minus = np.sum(out_minus)

                grad_numerical[idx] = (loss_plus - loss_minus) / (2 * epsilon)
                param.value[idx] = original_value # Restore original value
                it.iternext()
            
            # Compute analytical gradient
            output, cache = mha(x)
            d_out = np.ones_like(output)
            mha.backward(d_out, cache) # This will accumulate gradients in param.grad

            np.testing.assert_allclose(grad_numerical, param.grad, rtol=1e-3, atol=1e-4, err_msg=f"Gradient mismatch for {name}")

    def test_forward_pass_with_mask(self):
        d_model = 32
        num_heads = 4
        mha = MultiHeadAttention(d_model, num_heads)
        x = np.random.randn(1, 5, d_model) # Batch, Seq_len, d_model
        
        # Create a causal mask
        mask = np.triu(np.ones((1, 5, 5)), k=1) * -1e9 # Batch, Seq_len, Seq_len
        
        output, cache = mha(x, mask=mask)
        self.assertEqual(output.shape, x.shape)
        self.assertIsNotNone(cache)

    def test_forward_pass_with_kv_cache(self):
        d_model = 32
        num_heads = 4
        mha = MultiHeadAttention(d_model, num_heads)
        x1 = np.random.randn(1, 3, d_model) # First part of sequence
        x2 = np.random.randn(1, 2, d_model) # Second part of sequence

        # Process first part to get initial KV cache
        output1, cache1 = mha(x1)
        kv_cache = {'kh': cache1['K_full'], 'vh': cache1['V_full']}

        # Process second part using KV cache
        output2, cache2 = mha(x2, kv_cache=kv_cache)

        self.assertEqual(output2.shape, x2.shape)
        self.assertIsNotNone(cache2)
        # Check if kv_cache was updated correctly
        self.assertIn('kh', kv_cache)
        self.assertIn('vh', kv_cache)
        self.assertEqual(kv_cache['kh'].shape[2], x1.shape[1] + x2.shape[1])
        self.assertEqual(kv_cache['vh'].shape[2], x1.shape[1] + x2.shape[1])

        # Concatenate x1 and x2 and process as a whole sequence
        x_full = np.concatenate((x1, x2), axis=1)
        output_full, cache_full = mha(x_full)

        # Compare the output of processing in parts with processing as a whole
        # This is a simplified check, more rigorous would be to compare the actual values
        # of the last tokens generated.
        np.testing.assert_allclose(output2, output_full[:, x1.shape[1]:, :], rtol=1e-3, atol=1e-4)

class TestFeedForward(unittest.TestCase):

    def test_forward_pass(self):
        d_model = 10
        d_ff = 20
        ffn = FeedForward(d_model, d_ff)
        x = np.random.randn(2, 5, d_model) # Batch, Seq_len, d_model

        output, cache = ffn(x)

        self.assertEqual(output.shape, x.shape)
        self.assertIsNotNone(cache)
        self.assertIn('x', cache)
        self.assertIn('z', cache)
        self.assertIn('h', cache)
        self.assertIn('W1', cache)
        self.assertIn('b1', cache)
        self.assertIn('W2', cache)
        self.assertIn('b2', cache)

    def test_backward_pass_numerical_gradient(self):
        d_model = 5
        d_ff = 10
        ffn = FeedForward(d_model, d_ff)
        x = np.random.randn(1, 3, d_model) # Batch, Seq_len, d_model

        # Initialize gradients to zero for parameters
        for p in ffn.parameters():
            p.grad = np.zeros_like(p.value)

        epsilon = 1e-4

        # --- Test gradient for x ---
        grad_x_numerical = np.zeros_like(x)
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    x_plus = x.copy()
                    x_plus[i, j, k] += epsilon
                    out_plus, _ = ffn(x_plus)
                    loss_plus = np.sum(out_plus)

                    x_minus = x.copy()
                    x_minus[i, j, k] -= epsilon
                    out_minus, _ = ffn(x_minus)
                    loss_minus = np.sum(out_minus)

                    grad_x_numerical[i, j, k] = (loss_plus - loss_minus) / (2 * epsilon)

        output, cache = ffn(x)
        d_out = np.ones_like(output)
        grad_x_analytical = ffn.backward(d_out, cache)
        
        np.testing.assert_allclose(grad_x_numerical, grad_x_analytical, rtol=1e-3, atol=1e-4)

        # --- Test gradients for parameters (W1, b1, W2, b2) ---
        param_names = ['W1', 'b1', 'W2', 'b2']
        params = [ffn.W1, ffn.b1, ffn.W2, ffn.b2]

        for param, name in zip(params, param_names):
            param.grad = np.zeros_like(param.value) # Reset gradient

            grad_numerical = np.zeros_like(param.value)
            it = np.nditer(param.value, flags=['multi_index'], op_flags=['readwrite'])
            while not it.finished:
                idx = it.multi_index
                original_value = param.value[idx]

                param.value[idx] = original_value + epsilon
                out_plus, _ = ffn(x)
                loss_plus = np.sum(out_plus)

                param.value[idx] = original_value - epsilon
                out_minus, _ = ffn(x)
                loss_minus = np.sum(out_minus)

                grad_numerical[idx] = (loss_plus - loss_minus) / (2 * epsilon)
                param.value[idx] = original_value # Restore original value
                it.iternext()
            
            # Compute analytical gradient
            output, cache = ffn(x)
            d_out = np.ones_like(output)
            ffn.backward(d_out, cache) # This will accumulate gradients in param.grad

            np.testing.assert_allclose(grad_numerical, param.grad, rtol=1e-3, atol=1e-4, err_msg=f"Gradient mismatch for {name}")

if __name__ == '__main__':
    unittest.main()
