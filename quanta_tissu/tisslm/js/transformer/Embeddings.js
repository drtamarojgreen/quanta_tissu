const { Matrix } = require('../core/Matrix.js');

class Embedding {
    constructor(vocabSize, embedSize) {
        this.vocabSize = vocabSize;
        this.embedSize = embedSize;
        // Initialize weights randomly
        this.weights = Matrix.random(vocabSize, embedSize);
    }

    forward(tokenIds) {
        // tokenIds is a 1D array of integers
        const batchSize = tokenIds.length;
        const outputData = [];
        for (let i = 0; i < batchSize; i++) {
            const tokenId = tokenIds[i];
            if (tokenId >= 0 && tokenId < this.vocabSize) {
                outputData.push(this.weights.data[tokenId]);
            } else {
                // Return a zero vector for out-of-vocab tokens
                outputData.push(Array(this.embedSize).fill(0));
            }
        }
        return new Matrix(outputData);
    }
}

class PositionalEncoding {
    constructor(embedSize, maxLen = 5000) {
        this.embedSize = embedSize;
        this.maxLen = maxLen;
        this.pe = this._createPE();
    }

    _createPE() {
        const pe = Matrix.zeros(this.maxLen, this.embedSize);
        for (let pos = 0; pos < this.maxLen; pos++) {
            for (let i = 0; i < this.embedSize; i++) {
                if (i % 2 === 0) {
                    pe.data[pos][i] = Math.sin(pos / Math.pow(10000, i / this.embedSize));
                } else {
                    pe.data[pos][i] = Math.cos(pos / Math.pow(10000, (i - 1) / this.embedSize));
                }
            }
        }
        return pe;
    }

    forward(x) {
        // x is a Matrix from the Embedding layer (batch_size, embed_size)
        // Since this implementation is simplified and processes one sequence at a time,
        // we assume batch_size is the sequence length.
        const seqLen = x.rows;
        if (seqLen > this.maxLen) {
            throw new Error("Input sequence length exceeds maximum positional encoding length.");
        }

        const positionalEncodingSlice = this.pe.slice(0, seqLen, 0, this.embedSize);
        return x.add(positionalEncodingSlice);
    }
}

module.exports = { Embedding, PositionalEncoding };
