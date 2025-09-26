import numpy as np
from .layers import softmax

class CrossEntropyLoss:
    def __init__(self):
        self.cache = {}
        self.d_inputs = None # Initialize d_inputs

    def forward(self, logits, targets):
        """
        Computes the cross-entropy loss with label smoothing.

        Args:
            logits: Model output of shape (batch_size, seq_len, vocab_size).
            targets: True token IDs of shape (batch_size, seq_len).

        Returns:
            The mean loss.
        """
        batch_size, seq_len, vocab_size = logits.shape

        # Calculate probabilities
        probs = softmax(logits)

        # Create one-hot encoded targets
        one_hot_targets = np.zeros_like(probs)
        one_hot_targets[np.arange(batch_size)[:, None], np.arange(seq_len), targets] = 1

        # Create smoothed targets
        if self.smoothing > 0.0:
            smooth_dist = np.full_like(probs, self.smoothing / (vocab_size - 1))
            smooth_dist[np.arange(batch_size)[:, None], np.arange(seq_len), targets] = 1 - self.smoothing
            one_hot_targets = smooth_dist
        
        # Calculate cross-entropy loss
        log_probs = -np.log(probs + 1e-9)
        loss = np.sum(one_hot_targets * log_probs) / (batch_size * seq_len)

        # Cache for backward pass
        self.cache['probs'] = probs
        self.cache['targets'] = targets
        self.cache['one_hot_targets'] = one_hot_targets


        return loss

    def backward(self):
        """
        Computes the gradient of the loss with respect to the logits.
        """
        probs = self.cache['probs']
        one_hot_targets = self.cache['one_hot_targets']
        batch_size, seq_len, vocab_size = probs.shape

        # The gradient is (probs - smoothed_targets)
        d_logits = probs - one_hot_targets

        # Normalize the gradient by the total number of elements (batch_size * seq_len)
        d_logits /= (batch_size * seq_len)

        self.d_inputs = d_logits # Store d_inputs

        return d_logits