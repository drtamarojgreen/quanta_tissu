const { Matrix } = require('../core/Matrix.js');
const { Embedding, PositionalEncoding } = require('./Embeddings.js');
const { TransformerBlock } = require('./TransformerBlock.js');
const { LayerNorm } = require('./LayerNorm.js');

class TransformerModel {
    constructor(vocabSize, embedSize, numHeads, hiddenSize, numLayers) {
        this.embedding = new Embedding(vocabSize, embedSize);
        this.posEncoding = new PositionalEncoding(embedSize);

        this.blocks = [];
        for (let i = 0; i < numLayers; i++) {
            this.blocks.push(new TransformerBlock(embedSize, numHeads, hiddenSize));
        }

        this.finalNorm = new LayerNorm(embedSize);

        // Final linear layer to map to vocabulary
        this.outputLayer = Matrix.random(embedSize, vocabSize);
    }

    forward(tokenIds, mask = null) {
        // tokenIds is a 1D array of integers

        let x = this.embedding.forward(tokenIds);
        x = this.posEncoding.forward(x);

        for (const block of this.blocks) {
            x = block.forward(x, mask);
        }

        x = this.finalNorm.forward(x);

        // Final output layer to get logits
        const logits = x.dot(this.outputLayer);

        return logits; // Shape: (seq_len, vocab_size)
    }
}

module.exports = { TransformerModel };
