import numpy as np

class CosineDecayWithWarmup:
    def __init__(self, optimizer, warmup_steps, total_steps, max_lr, min_lr=1e-6):
        self.optimizer = optimizer
        self.warmup_steps = warmup_steps
        self.total_steps = total_steps
        self.max_lr = max_lr
        self.min_lr = min_lr

    def step(self, current_step):
        """Calculates and sets the new learning rate."""
        if current_step < self.warmup_steps:
            # Linear warmup
            lr = self.max_lr * current_step / self.warmup_steps
        elif current_step < self.total_steps:
            # Cosine decay
            decay_ratio = (current_step - self.warmup_steps) / (self.total_steps - self.warmup_steps)
            cosine_decay = 0.5 * (1 + np.cos(np.pi * decay_ratio))
            lr = self.min_lr + (self.max_lr - self.min_lr) * cosine_decay
        else:
            # After total_steps, stick to min_lr
            lr = self.min_lr

        self.optimizer.lr = max(lr, self.min_lr)
        return self.optimizer.lr
