// NOTE: This is a test file. Per instructions, it should not be executed.
// To run these tests, one would typically use a test runner like Jest or Mocha,
// or simply run `node <path_to_this_file>`.

// A simple assertion function for testing purposes. In a real scenario,
// a dedicated assertion library (like Chai) would be used.
function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function assertThrows(fn, message) {
    try {
        fn();
        throw new Error("Expected function to throw an error, but it did not.");
    } catch (e) {
        // Test passed
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


console.log("Loading tests for Matrix class...");

const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');

function testMatrixCreation() {
    console.log("Test: Matrix creation");
    const m = new Matrix([[1, 2], [3, 4.5]]);
    assert(m.rows === 2, "Constructor should set rows correctly.");
    assert(m.cols === 2, "Constructor should set cols correctly.");
    assert(m.data[1][1] === 4.5, "Constructor should set data correctly.");
    assertThrows(() => new Matrix([1, 2]), "Constructor should throw on invalid data.");
}

function testStaticMethods() {
    console.log("Test: Static methods");
    const zeros = Matrix.zeros(2, 3);
    assertDeepEqual(zeros, new Matrix([[0, 0, 0], [0, 0, 0]]));
    const rand = Matrix.random(2, 2);
    assert(rand.rows === 2 && rand.cols === 2, "Random matrix should have correct dimensions.");
}

function testAdditionAndSubtraction() {
    console.log("Test: Addition and Subtraction");
    const m1 = new Matrix([[1, 2], [3, 4]]);
    const m2 = new Matrix([[5, 6], [7, 8]]);
    const sum = m1.add(m2);
    assertDeepEqual(sum, new Matrix([[6, 8], [10, 12]]));
    const diff = m2.subtract(m1);
    assertDeepEqual(diff, new Matrix([[4, 4], [4, 4]]));
}

function testMultiplication() {
    console.log("Test: Multiplication (element-wise and dot product)");
    const m1 = new Matrix([[1, 2], [3, 4]]);
    const m2 = new Matrix([[2, 0], [1, 2]]);

    // Element-wise
    const elemWise = m1.multiply(m2);
    assertDeepEqual(elemWise, new Matrix([[2, 0], [3, 8]]));

    // Dot product
    const dot = m1.dot(m2);
    assertDeepEqual(dot, new Matrix([[4, 4], [10, 8]]));
}

function testTransposeAndReshape() {
    console.log("Test: Transpose and Reshape");
    const m = new Matrix([[1, 2, 3], [4, 5, 6]]);
    const t = m.transpose();
    assertDeepEqual(t, new Matrix([[1, 4], [2, 5], [3, 6]]));

    const r = m.reshape(3, 2);
    assertDeepEqual(r, new Matrix([[1, 2], [3, 4], [5, 6]]));
    assertThrows(() => m.reshape(4, 2), "Reshape should throw on mismatched element count.");
}

function testActivationFunctions() {
    console.log("Test: Activation functions");
    const m = new Matrix([[-1, 0.5], [2, -3]]);
    const relu = m.relu();
    assertDeepEqual(relu, new Matrix([[0, 0.5], [2, 0]]));

    const m2 = new Matrix([[1, 2]]);
    const softmax = m2.softmax();
    // exp(1)/(exp(1)+exp(2)) = 0.2689...
    // exp(2)/(exp(1)+exp(2)) = 0.7310...
    assertDeepEqual(softmax, new Matrix([[0.2689414213699951, 0.7310585786300049]]));
}

function testSlice() {
    console.log("Test: Slicing");
    const m = new Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]]);
    const slice = m.slice(0, 2, 1, 3);
    assertDeepEqual(slice, new Matrix([[2, 3], [5, 6]]));
}


function runAllMatrixTests() {
    try {
        testMatrixCreation();
        testStaticMethods();
        testAdditionAndSubtraction();
        testMultiplication();
        testTransposeAndReshape();
        testActivationFunctions();
        testSlice();
        console.log("-> All Matrix tests loaded successfully.");
    } catch (e) {
        console.error("-> A test definition failed:", e.message);
    }
}

runAllMatrixTests();
