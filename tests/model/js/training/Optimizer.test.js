// NOTE: This is a test file. Per instructions, it should not be executed.
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');
const { SGDOptimizer } = require('../../../../quanta_tissu/tisslm/js/training/Optimizer.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function assertDeepEqual(a, b, tolerance = 1e-9) {
    if (a.rows !== b.rows || a.cols !== b.cols) {
        throw new Error(`Matrix dimension mismatch: (${a.rows}x${a.cols}) vs (${b.rows}x${b.cols})`);
    }
    for (let i = 0; i < a.rows; i++) {
        for (let j = 0; j < a.cols; j++) {
            if (Math.abs(a.data[i][j] - b.data[i][j]) > tolerance) {
                throw new Error(`Matrix data mismatch at (${i},${j}): ${a.data[i][j]} vs ${b.data[i][j]}`);
            }
        }
    }
}


function testOptimizerStep() {
    console.log("Test: Optimizer step (conceptual)");

    // Create a dummy parameter with data and a gradient
    const param = {
        data: new Matrix([[1.0, 2.0]]),
        grad: new Matrix([[0.5, -1.0]]) // Pseudo-gradient
    };

    const optimizer = new SGDOptimizer([param], 0.1);
    optimizer.step();

    // Expected new data: data - (grad * lr)
    // [1.0, 2.0] - ([0.5, -1.0] * 0.1)
    // [1.0, 2.0] - [0.05, -0.1]
    // [0.95, 2.1]
    const expected = new Matrix([[0.95, 2.1]]);

    assertDeepEqual(param.data, expected);
}

function runAllOptimizerTests() {
    try {
        testOptimizerStep();
        console.log("-> All Optimizer tests loaded successfully.");
    } catch(e) {
        console.error("-> An Optimizer test definition failed:", e.message);
    }
}

runAllOptimizerTests();
