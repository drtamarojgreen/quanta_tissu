import numpy as np
from .layers import softmax

class CrossEntropyLoss:
    def __init__(self):
        self.cache = {}
        self.d_inputs = None # Initialize d_inputs

    def forward(self, logits, targets):
        """
        Computes the cross-entropy loss.

        Args:
            logits: Model output of shape (batch_size, seq_len, vocab_size).
            targets: True token IDs of shape (batch_size, seq_len).

        Returns:
            The mean loss.
        """
        batch_size, seq_len, vocab_size = logits.shape

        # Calculate probabilities
        probs = softmax(logits)

        # Get the probabilities corresponding to the target tokens
        target_probs = probs[np.arange(batch_size)[:, None], np.arange(seq_len), targets]

        # Calculate negative log-likelihood
        log_probs = -np.log(target_probs + 1e-9) # Add epsilon for stability

        # Calculate the mean loss
        loss = np.mean(log_probs)

        # Cache for backward pass
        self.cache['probs'] = probs
        self.cache['targets'] = targets

        return loss

    def backward(self):
        """
        Computes the gradient of the loss with respect to the logits.
        """
        probs = self.cache['probs']
        targets = self.cache['targets']
        batch_size, seq_len, vocab_size = probs.shape

        # The gradient is (probs - y), where y is one-hot encoded targets
        d_logits = probs.copy()

        # Subtract 1 from the scores of the correct classes
        d_logits[np.arange(batch_size)[:, None], np.arange(seq_len), targets] -= 1

        # Normalize the gradient by the total number of elements (batch_size * seq_len)
        d_logits /= (batch_size * seq_len)

        self.d_inputs = d_logits # Store d_inputs

        return d_logits