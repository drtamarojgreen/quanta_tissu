const { Matrix } = require('../core/Matrix.js');

class LayerNorm {
    constructor(features, epsilon = 1e-5) {
        this.epsilon = epsilon;
        this.gamma = Matrix.ones(1, features); // Learnable scale parameter
        this.beta = Matrix.zeros(1, features); // Learnable shift parameter
    }

    forward(x) {
        // x is a Matrix with shape (batch_size, features)
        const mean = x.data.map(row => row.reduce((a, b) => a + b, 0) / row.length);
        const variance = x.data.map((row, i) => {
            return row.reduce((acc, val) => acc + Math.pow(val - mean[i], 2), 0) / row.length;
        });

        const normalizedData = x.data.map((row, i) => {
            const std = Math.sqrt(variance[i] + this.epsilon);
            return row.map(val => (val - mean[i]) / std);
        });

        const normalizedMatrix = new Matrix(normalizedData);

        // Apply scale and shift
        const scaled = normalizedMatrix.multiply(this.gamma); // Element-wise
        const shifted = scaled.add(this.beta); // Element-wise

        return shifted;
    }
}

// Add a static ones method to Matrix if it's missing from the original implementation
if (!Matrix.ones) {
    Matrix.ones = function(rows, cols) {
        const data = Array(rows).fill(0).map(() => Array(cols).fill(1));
        return new Matrix(data);
    }
}


module.exports = { LayerNorm };
