import numpy as np

class AdamW:
    def __init__(self, params, lr=1e-3, betas=(0.9, 0.999), eps=1e-8, weight_decay=1e-2):
        self.params = [p for p in params if p is not None]
        self.lr = lr
        self.beta1, self.beta2 = betas
        self.eps = eps
        self.weight_decay = weight_decay

        self.m = [np.zeros_like(p.value) for p in self.params]
        self.v = [np.zeros_like(p.value) for p in self.params]
        self.t = 0

    def step(self):
        """Performs a single optimization step."""
        self.t += 1

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
