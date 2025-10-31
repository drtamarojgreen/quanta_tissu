const { Matrix } = require('../core/Matrix.js');

class SelfAttention {
    constructor(embedSize, numHeads) {
        if (embedSize % numHeads !== 0) {
            throw new Error("Embedding size must be divisible by the number of heads.");
        }
        this.embedSize = embedSize;
        this.numHeads = numHeads;
        this.headSize = embedSize / numHeads;

        // Linear layers for Q, K, V for all heads combined
        this.w_q = Matrix.random(embedSize, embedSize);
        this.w_k = Matrix.random(embedSize, embedSize);
        this.w_v = Matrix.random(embedSize, embedSize);

        // Final output linear layer
        this.w_o = Matrix.random(embedSize, embedSize);
    }

    forward(x, mask = null) {
        // x is a Matrix with shape (seq_len, embed_size)
        const seqLen = x.rows;

        const Q = x.dot(this.w_q);
        const K = x.dot(this.w_k);
        const V = x.dot(this.w_v);

        // At this point, Q, K, V have shape (seq_len, embed_size)
        // In a real implementation, we would split these into heads.
        // This is a simplified version where we perform the calculation
        // without explicitly reshaping for heads, for clarity. A full
        // implementation would require tensor operations.

        let scores = Q.dot(K.transpose());
        // Scale scores
        scores = scores.multiply(new Matrix(Array(seqLen).fill(Array(seqLen).fill(1 / Math.sqrt(this.headSize)))));

        if (mask) {
            // Apply mask (e.g., for decoding) by setting masked values to -Infinity
             const maskedScoresData = scores.data.map((row, i) =>
                row.map((val, j) => (mask.data[i][j] === 0 ? -Infinity : val))
            );
            scores = new Matrix(maskedScoresData);
        }

        const attentionWeights = scores.softmax();

        const output = attentionWeights.dot(V);

        // Final linear layer
        const finalOutput = output.dot(this.w_o);

        return finalOutput;
    }
}

module.exports = { SelfAttention };
