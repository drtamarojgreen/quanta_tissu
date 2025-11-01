class Matrix {
    constructor(data) {
        if (!Array.isArray(data) || !Array.isArray(data[0])) {
            throw new Error("Matrix data must be a 2D array.");
        }
        this.data = data;
        this.rows = data.length;
        this.cols = data[0].length;
    }

    static zeros(rows, cols) {
        const data = Array(rows).fill(0).map(() => Array(cols).fill(0));
        return new Matrix(data);
    }

    static random(rows, cols) {
        const data = Array(rows).fill(0).map(() =>
            Array(cols).fill(0).map(() => Math.random() * 2 - 1)
        );
        return new Matrix(data);
    }

    // Element-wise addition with broadcasting for row vectors
    add(other) {
        // Standard case: A.add(B) where A and B are same size
        if (this.rows === other.rows && this.cols === other.cols) {
            const result = this.data.map((row, i) =>
                row.map((val, j) => val + other.data[i][j])
            );
            return new Matrix(result);
        }
        // Broadcasting case: A.add(v) where A is MxN and v is 1xN
        else if (other.rows === 1 && this.cols === other.cols) {
            const rowVector = other.data[0];
            const result = this.data.map(row =>
                row.map((val, j) => val + rowVector[j])
            );
            return new Matrix(result);
        }
        // Otherwise, invalid operation
        else {
            throw new Error(`Matrix dimension mismatch for addition: (${this.rows}x${this.cols}) vs (${other.rows}x${other.cols}). Broadcasting is only supported for a 1xN vector.`);
        }
    }

    // Element-wise subtraction
    subtract(other) {
        if (this.rows !== other.rows || this.cols !== other.cols) {
            throw new Error("Matrices must have the same dimensions for subtraction.");
        }
        const result = this.data.map((row, i) =>
            row.map((val, j) => val - other.data[i][j])
        );
        return new Matrix(result);
    }

    // Element-wise multiplication with broadcasting for row vectors
    multiply(other) {
        // Scalar multiplication
        if (typeof other === 'number') {
            const result = this.data.map(row => row.map(val => val * other));
            return new Matrix(result);
        }
        // Standard case: A.multiply(B) where A and B are same size
        if (this.rows === other.rows && this.cols === other.cols) {
            const result = this.data.map((row, i) =>
                row.map((val, j) => val * other.data[i][j])
            );
            return new Matrix(result);
        }
        // Broadcasting case: A.multiply(v) where A is MxN and v is 1xN
        else if (other.rows === 1 && this.cols === other.cols) {
            const rowVector = other.data[0];
            const result = this.data.map(row =>
                row.map((val, j) => val * rowVector[j])
            );
            return new Matrix(result);
        }
        // Otherwise, invalid operation
        else {
            throw new Error(`Matrix dimension mismatch for multiplication: (${this.rows}x${this.cols}) vs (${other.rows}x${other.cols}). Broadcasting is only supported for a 1xN vector.`);
        }
    }

    // Dot product
    dot(other) {
        if (this.cols !== other.rows) {
            throw new Error("Matrix dimensions are not compatible for dot product.");
        }
        const result = Matrix.zeros(this.rows, other.cols);
        for (let i = 0; i < this.rows; i++) {
            for (let j = 0; j < other.cols; j++) {
                let sum = 0;
                for (let k = 0; k < this.cols; k++) {
                    sum += this.data[i][k] * other.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }
        return result;
    }

    transpose() {
        const result = Matrix.zeros(this.cols, this.rows);
        for (let i = 0; i < this.rows; i++) {
            for (let j = 0; j < this.cols; j++) {
                result.data[j][i] = this.data[i][j];
            }
        }
        return result;
    }

    reshape(rows, cols) {
        if (this.rows * this.cols !== rows * cols) {
            throw new Error("New shape must contain the same number of elements.");
        }
        const flat = this.data.flat();
        const newData = [];
        for (let i = 0; i < rows; i++) {
            newData.push(flat.slice(i * cols, (i + 1) * cols));
        }
        return new Matrix(newData);
    }

    slice(startRow, endRow, startCol, endCol) {
        const slicedData = this.data.slice(startRow, endRow).map(row => row.slice(startCol, endCol));
        return new Matrix(slicedData);
    }

    // Activation Functions
    relu() {
        const result = this.data.map(row => row.map(val => Math.max(0, val)));
        return new Matrix(result);
    }

    softmax() {
        const result = this.data.map(row => {
            const max = Math.max(...row);
            const exps = row.map(val => Math.exp(val - max));
            const sumExps = exps.reduce((a, b) => a + b, 0);
            return exps.map(e => e / sumExps);
        });
        return new Matrix(result);
    }

    // Utility to print the matrix
    print() {
        console.table(this.data);
    }
}

module.exports = { Matrix };
