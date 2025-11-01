// NOTE: This is a test file. Per instructions, it should not be executed.
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');
const { CrossEntropyLoss } = require('../../../../quanta_tissu/tisslm/js/training/LossFunction.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function assertAlmostEqual(a, b, tolerance = 1e-6) {
    if (Math.abs(a - b) > tolerance) {
        throw new Error(`${a} is not almost equal to ${b}`);
    }
}

function testCrossEntropyLoss() {
    console.log("Test: CrossEntropyLoss forward pass");
    const lossFn = new CrossEntropyLoss();

    // Logits for a batch of 2 samples, 3 classes
    // Sample 1: high confidence in class 1
    // Sample 2: high confidence in class 0
    const logits = new Matrix([
        [0.1, 2.5, 0.2],
        [3.0, 0.1, 0.1]
    ]);
    const targets = [1, 0];

    // Softmax probabilities (approx):
    // p1 = [0.10, 0.82, 0.08] -> -log(0.82) = 0.198
    // p2 = [0.89, 0.06, 0.05] -> -log(0.89) = 0.116
    // Mean loss = (0.198 + 0.116) / 2 = 0.157
    const expectedLoss = 0.1395878291666058;

    const loss = lossFn.forward(logits, targets);

    assertAlmostEqual(loss, expectedLoss, 1e-4);
}

function runAllLossTests() {
    try {
        testCrossEntropyLoss();
        console.log("-> All LossFunction tests loaded successfully.");
    } catch(e) {
        console.error("-> A LossFunction test definition failed:", e.message);
    }
}

runAllLossTests();
