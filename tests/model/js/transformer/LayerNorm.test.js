// NOTE: This is a test file. Per instructions, it should not be executed.
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');
const { LayerNorm } = require('../../../../quanta_tissu/tisslm/js/transformer/LayerNorm.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function assertDeepEqual(a, b, tolerance = 1e-5) {
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

function testLayerNormForward() {
    console.log("Test: LayerNorm forward pass");
    const ln = new LayerNorm(3);

    // Manually set gamma and beta for predictable results
    ln.gamma = new Matrix([[1.5, 1.5, 1.5]]);
    ln.beta = new Matrix([[0.5, 0.5, 0.5]]);

    const input = new Matrix([[1, 2, 3]]);
    // Mean = 2
    // Variance = ((1-2)^2 + (2-2)^2 + (3-2)^2) / 3 = (1+0+1)/3 = 2/3
    // Stddev = sqrt(2/3) = 0.816
    // Normalized = [(1-2)/std, (2-2)/std, (3-2)/std] = [-1.22, 0, 1.22]
    // Scaled = [-1.83, 0, 1.83]
    // Shifted = [-1.33, 0.5, 2.33]
    const expected = new Matrix([[-1.3371035288625852, 0.5, 2.337103528862585]]);

    const output = ln.forward(input);

    assertDeepEqual(output, expected);
}

function runAllLayerNormTests() {
    try {
        testLayerNormForward();
        console.log("-> All LayerNorm tests loaded successfully.");
    } catch(e) {
        console.error("-> A LayerNorm test definition failed:", e.message);
    }
}

runAllLayerNormTests();
