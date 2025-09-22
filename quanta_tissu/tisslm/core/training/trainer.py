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

                logits, cache = self.model.forward(x_batch, training=True)
                loss = self.loss_fn.forward(logits, y_batch)
                d_logits = self.loss_fn.backward()
                self.model.backward(d_logits, cache)

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
