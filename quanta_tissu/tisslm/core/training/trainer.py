import numpy as np
import logging
import os

from ..utils import save_checkpoint, load_checkpoint

logger = logging.getLogger(__name__)

class Trainer:
    """
    Encapsulates the entire training process for the TissLM model.
    It handles the training loop, forward pass, backpropagation, and optimization.
    """

    def __init__(self, model, optimizer, loss_fn, scheduler, dataset, config):
        self.model = model
        self.optimizer = optimizer
        self.loss_fn = loss_fn
        self.scheduler = scheduler
        self.dataset = dataset
        self.config = config # Expects a dict with keys like 'epochs', 'max_grad_norm', etc.

    def train(self):
        """
        Starts and manages the training loop over epochs and batches.
        """
        start_epoch = 0
        step = 0

        if self.config.get("resume_from"):
            logger.info(f"Resuming training from checkpoint: {self.config['resume_from']}")
            start_epoch, step = load_checkpoint(self.model, self.optimizer, self.config["resume_from"])
            if self.scheduler:
                self.scheduler.optimizer.lr = self.optimizer.lr

        logger.info(f"Starting training from epoch {start_epoch + 1}, step {step}.")

        for epoch in range(start_epoch, self.config["epochs"]):
            logger.info(f"\n--- Epoch {epoch + 1}/{self.config['epochs']} ---")
            for x_batch, y_batch in self.dataset:

                logits, cache = self.model.forward(x_batch)
                loss = self.loss_fn.forward(logits, y_batch)
                d_logits = self.loss_fn.backward()
                self._backward(d_logits, cache)

                params = self.model.parameters()
                grads = [p.grad for p in params if p.grad is not None]
                global_norm = np.sqrt(sum(np.sum(g**2) for g in grads))
                if self.config.get("max_grad_norm") and global_norm > self.config["max_grad_norm"]:
                    clip_coef = self.config["max_grad_norm"] / (global_norm + 1e-6)
                    for p in params:
                        if p.grad is not None:
                            p.grad *= clip_coef

                self.optimizer.step()
                current_lr = self.scheduler.step(step) if self.scheduler else self.optimizer.lr
                self.optimizer.zero_grad()

                if step % 10 == 0:
                    logger.info(f"Step {step}, Loss: {loss:.4f}, LR: {current_lr:.6f}, Grad Norm: {global_norm:.4f}")

                if self.config.get("save_every") and step > 0 and step % self.config["save_every"] == 0:
                    save_checkpoint(self.model, self.optimizer, epoch, step, self.config["checkpoint_dir"], keep_last=self.config.get("keep_checkpoints", -1))
                step += 1

        logger.info("\n--- Training complete ---")
        save_checkpoint(self.model, self.optimizer, self.config["epochs"] - 1, step, self.config["checkpoint_dir"], keep_last=self.config.get("keep_checkpoints", -1))

    def _backward(self, d_logits, model_cache):
        final_x, output_proj, embeddings, token_ids, block_caches = \
            model_cache['final_x'], model_cache['output_proj'], model_cache['embeddings'], model_cache['token_ids'], model_cache['block_caches']

        batch_size, seq_len, d_model = final_x.shape
        final_x_reshaped = final_x.reshape(batch_size * seq_len, d_model)
        d_logits_reshaped = d_logits.reshape(batch_size * seq_len, -1)

        output_proj.grad += final_x_reshaped.T @ d_logits_reshaped
        dx = d_logits @ output_proj.value.T

        for i in reversed(range(len(self.model.transformer_blocks))):
            dx = self._backward_transformer_block(dx, block_caches[i])

        d_embeddings = np.zeros_like(embeddings.value)
        np.add.at(d_embeddings, token_ids, dx)
        embeddings.grad += d_embeddings

    def _backward_transformer_block(self, d_out, cache):
        dx_plus_ffn = self._backward_layernorm(d_out, cache['ln2_cache'])
        dx_norm1_from_ffn = self._backward_ffn(dx_plus_ffn, cache['ffn_cache'])
        dx_norm1 = dx_plus_ffn + dx_norm1_from_ffn
        dx_plus_attn = self._backward_layernorm(dx_norm1, cache['ln1_cache'])
        dx_from_mha = self._backward_mha(dx_plus_attn, cache['mha_cache'])
        return dx_plus_attn + dx_from_mha

    def _backward_layernorm(self, d_out, cache):
        x, x_norm, mean, var, gamma, beta, eps = \
            cache['x'], cache['x_norm'], cache['mean'], cache['var'], cache['gamma'], cache['beta'], cache['eps']

        D = x.shape[-1]
        beta.grad += d_out.sum(axis=(0, 1))
        gamma.grad += (d_out * x_norm).sum(axis=(0, 1))

        dx_norm = d_out * gamma.value
        dvar = np.sum(dx_norm * (x - mean) * -0.5 * (var + eps)**(-1.5), axis=-1, keepdims=True)
        dmean = np.sum(dx_norm * -1 / np.sqrt(var + eps), axis=-1, keepdims=True) - 2 * dvar * np.sum(x - mean, axis=-1, keepdims=True) / D
        return (dx_norm / np.sqrt(var + eps)) + (dvar * 2 * (x - mean) / D) + (dmean / D)

    def _backward_ffn(self, d_out, cache):
        x, z, h, W1, b1, W2, b2 = \
            cache['x'], cache['z'], cache['h'], cache['W1'], cache['b1'], cache['W2'], cache['b2']

        batch_size, seq_len, d_model = x.shape
        d_ff = h.shape[-1]

        h_reshaped = h.reshape(batch_size * seq_len, d_ff)
        d_out_reshaped = d_out.reshape(batch_size * seq_len, d_model)
        x_reshaped = x.reshape(batch_size * seq_len, d_model)

        W2.grad += h_reshaped.T @ d_out_reshaped
        b2.grad += d_out_reshaped.sum(axis=0)

        dh = d_out @ W2.value.T
        dz = dh * (z > 0)
        dz_reshaped = dz.reshape(batch_size * seq_len, d_ff)

        W1.grad += x_reshaped.T @ dz_reshaped
        b1.grad += dz_reshaped.sum(axis=0)

        return dz @ W1.value.T

    def _backward_mha(self, d_out, cache):
        x, Qh, Kh, Vh, attn_weights, combined, Wq, Wk, Wv, Wo, mha_instance = \
            cache['x'], cache['Qh'], cache['Kh'], cache['Vh'], cache['attention_weights'], cache['combined'], \
            cache['Wq'], cache['Wk'], cache['Wv'], cache['Wo'], cache['layer_instance']

        batch_size, seq_len, d_model = x.shape

        combined_reshaped = combined.reshape(batch_size * seq_len, d_model)
        d_out_reshaped = d_out.reshape(batch_size * seq_len, d_model)

        Wo.grad += combined_reshaped.T @ d_out_reshaped
        d_combined = d_out @ Wo.value.T

        d_attended = mha_instance.split_heads(d_combined)

        d_weights = d_attended @ Vh.transpose(0, 1, 3, 2)
        dVh = attn_weights.transpose(0, 1, 3, 2) @ d_attended

        d_scores = d_weights # Simplified softmax backward
        d_scores_unscaled = d_scores * np.sqrt(Qh.shape[-1])

        dQh = d_scores_unscaled @ Kh
        dKh_transposed = Qh.transpose(0, 1, 3, 2) @ d_scores_unscaled
        dKh = dKh_transposed.transpose(0, 1, 3, 2)

        dQ, dK, dV = mha_instance.combine_heads(dQh), mha_instance.combine_heads(dKh), mha_instance.combine_heads(dVh)

        x_reshaped = x.reshape(batch_size * seq_len, d_model)
        Wq.grad += x_reshaped.T @ dQ.reshape(-1, d_model)
        Wk.grad += x_reshaped.T @ dK.reshape(-1, d_model)
        Wv.grad += x_reshaped.T @ dV.reshape(-1, d_model)

        return (dQ @ Wq.value.T) + (dK @ Wk.value.T) + (dV @ Wv.value.T)
