const { Matrix } = require('../core/Matrix.js');

class FeedForward {
    constructor(embedSize, hiddenSize) {
        this.embedSize = embedSize;
        this.hiddenSize = hiddenSize;

        // Linear layers
        this.w1 = Matrix.random(embedSize, hiddenSize);
        this.b1 = Matrix.zeros(1, hiddenSize);
        this.w2 = Matrix.random(hiddenSize, embedSize);
        this.b2 = Matrix.zeros(1, embedSize);
    }

    forward(x) {
        // x is a Matrix with shape (seq_len, embed_size)

        // First linear layer + ReLU
        let output = x.dot(this.w1).add(this.b1);
        output = output.relu();

        // Second linear layer
        output = output.dot(this.w2).add(this.b2);

        return output;
    }
}

module.exports = { FeedForward };
