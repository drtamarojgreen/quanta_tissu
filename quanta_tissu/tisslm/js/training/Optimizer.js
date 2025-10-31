// NOTE: This is a conceptual implementation. A real optimizer would need access
// to model parameters and their gradients, which are not part of the current
// simplified `Matrix` and model architecture.

class SGDOptimizer {
    /**
     * @param {Array<Object>} parameters - A list of parameters (e.g., Matrix weights) to update.
     * @param {number} learningRate - The learning rate.
     */
    constructor(parameters, learningRate = 0.01) {
        if (!parameters || parameters.length === 0) {
            throw new Error("Optimizer must be initialized with model parameters.");
        }
        this.parameters = parameters;
        this.lr = learningRate;
    }

    // In a real scenario, this would be called after a backward pass.
    step() {
        for (const param of this.parameters) {
            if (param.grad && param.data) {
                // This is pseudo-code as `param.grad` is not calculated.
                // It assumes `param` is an object like `{ data: Matrix, grad: Matrix }`
                const update = param.grad.multiply(new Matrix(Array(param.grad.rows).fill(Array(param.grad.cols).fill(this.lr))));
                param.data = param.data.subtract(update);
            }
        }
    }

    // Resets gradients. Would be called at the start of each training iteration.
    zero_grad() {
         for (const param of this.parameters) {
            if (param.grad) {
                param.grad = Matrix.zeros(param.grad.rows, param.grad.cols);
            }
        }
    }
}

module.exports = { SGDOptimizer };
