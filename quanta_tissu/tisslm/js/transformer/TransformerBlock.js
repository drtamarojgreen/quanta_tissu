const { SelfAttention } = require('./SelfAttention.js');
const { FeedForward } = require('./FeedForward.js');
const { LayerNorm } = require('./LayerNorm.js');

class TransformerBlock {
    constructor(embedSize, numHeads, hiddenSize) {
        this.attention = new SelfAttention(embedSize, numHeads);
        this.norm1 = new LayerNorm(embedSize);
        this.norm2 = new LayerNorm(embedSize);
        this.ff = new FeedForward(embedSize, hiddenSize);
    }

    forward(x, mask = null) {
        // x is a Matrix with shape (seq_len, embed_size)

        // Attention sub-layer
        const attentionOutput = this.attention.forward(x, mask);
        // Add & Norm
        const x1 = this.norm1.forward(x.add(attentionOutput));

        // Feed-forward sub-layer
        const ffOutput = this.ff.forward(x1);
        // Add & Norm
        const output = this.norm2.forward(x1.add(ffOutput));

        return output;
    }
}

module.exports = { TransformerBlock };
