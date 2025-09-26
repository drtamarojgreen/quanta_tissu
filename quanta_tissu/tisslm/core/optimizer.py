import numpy as np

class AdamW:
    def __init__(self, params, lr=1e-3, betas=(0.9, 0.999), eps=1e-8, weight_decay=1e-2, clip_grad_norm=None):
        self.params = [p for p in params if p is not None]
        self.lr = lr
        self.beta1, self.beta2 = betas
        self.eps = eps
        self.weight_decay = weight_decay
        self.clip_grad_norm = clip_grad_norm

        self.m = [np.zeros_like(p.value) for p in self.params]
        self.v = [np.zeros_like(p.value) for p in self.params]
        self.t = 0

    def step(self):
        """Performs a single optimization step."""
        self.t += 1

        if self.clip_grad_norm is not None:
            total_norm = 0
            for p in self.params:
                if p.grad is not None:
                    total_norm += np.sum(p.grad ** 2)
            total_norm = np.sqrt(total_norm)

            if total_norm > self.clip_grad_norm:
                clip_coef = self.clip_grad_norm / (total_norm + 1e-6)
                for p in self.params:
                    if p.grad is not None:
                        p.grad *= clip_coef

        for i, p in enumerate(self.params):
            if p.grad is None:
                continue

            # AdamW update
            self.m[i] = self.beta1 * self.m[i] + (1 - self.beta1) * p.grad
            self.v[i] = self.beta2 * self.v[i] + (1 - self.beta2) * (p.grad ** 2)

            # Bias correction
            m_hat = self.m[i] / (1 - self.beta1 ** self.t)
            v_hat = self.v[i] / (1 - self.beta2 ** self.t)

            # Parameter update
            update = self.lr * m_hat / (np.sqrt(v_hat) + self.eps)
            p.value -= update

            # Weight decay
            if self.weight_decay > 0:
                p.value -= self.lr * self.weight_decay * p.value

    def zero_grad(self):
        """Resets the gradients of all parameters."""
        for p in self.params:
            if p.grad is not None:
                p.grad.fill(0)

class LRScheduler:
    def __init__(self, optimizer):
        self.optimizer = optimizer
        self.last_epoch = -1
        self.step()

    def step(self):
        self.last_epoch += 1
        lr = self.get_lr()
        self.optimizer.lr = lr

    def get_lr(self):
        raise NotImplementedError


class CosineAnnealingLR(LRScheduler):
    def __init__(self, optimizer, T_max, eta_min=0):
        self.T_max = T_max
        self.eta_min = eta_min
        super().__init__(optimizer)

    def get_lr(self):
        return self.eta_min + (self.optimizer.lr - self.eta_min) * \
               (1 + np.cos(np.pi * self.last_epoch / self.T_max)) / 2