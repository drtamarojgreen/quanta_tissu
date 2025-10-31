const { Matrix } = require('../core/Matrix.js');

class CrossEntropyLoss {
    /**
     * Calculates the cross-entropy loss.
     * @param {Matrix} logits - The raw output from the model. Shape: (batch_size, num_classes).
     * @param {Array<number>} targets - The ground truth labels. Shape: (batch_size).
     * @returns {number} The average loss over the batch.
     */
    forward(logits, targets) {
        if (logits.rows !== targets.length) {
            throw new Error("Number of logit rows must match number of targets.");
        }

        const probabilities = logits.softmax();
        let totalLoss = 0;

        for (let i = 0; i < targets.length; i++) {
            const targetIndex = targets[i];
            if (targetIndex >= probabilities.cols) {
                throw new Error(`Target index ${targetIndex} is out of bounds for ${probabilities.cols} classes.`);
            }

            const probabilityOfCorrectClass = probabilities.data[i][targetIndex];

            // Add a small epsilon to prevent log(0)
            const loss = -Math.log(probabilityOfCorrectClass + 1e-9);
            totalLoss += loss;
        }

        return totalLoss / targets.length; // Return the mean loss
    }
}

module.exports = { CrossEntropyLoss };
