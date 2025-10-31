// NOTE: This is a test file. Per instructions, it should not be executed.
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');
const { Embedding, PositionalEncoding } = require('../../../../quanta_tissu/tisslm/js/transformer/Embeddings.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function testEmbeddingForward() {
    console.log("Test: Embedding forward pass");
    const vocabSize = 10;
    const embedSize = 4;
    const embedding = new Embedding(vocabSize, embedSize);

    // Manually set a weight for predictability
    embedding.weights.data[3] = [0.1, 0.2, 0.3, 0.4];

    const input = [3, 0]; // Batch of 2 tokens
    const output = embedding.forward(input);

    assert(output.rows === 2, "Output should have correct number of rows (batch size).");
    assert(output.cols === embedSize, "Output should have correct number of columns (embed size).");
    assert(output.data[0][1] === 0.2, "Embedding lookup should be correct.");
    assert(output.data[1].every(v => typeof v === 'number'), "Embedding for token 0 should exist.");
}

function testPositionalEncodingForward() {
    console.log("Test: PositionalEncoding forward pass");
    const embedSize = 4;
    const pe = new PositionalEncoding(embedSize);

    const input = Matrix.zeros(3, embedSize); // A sequence of 3 tokens
    const output = pe.forward(input);

    // sin(0/...) = 0, cos(0/...) = 1
    assert(output.data[0][0] === 0, "PE for pos 0, dim 0 should be sin(0)=0");
    assert(output.data[0][1] === 1, "PE for pos 0, dim 1 should be cos(0)=1");

    // Check that it's added, not just the PE value
    const input2 = new Matrix([[1,1,1,1], [1,1,1,1]]);
    const output2 = pe.forward(input2);
    assert(output2.data[0][0] === 1, "PE should be added to the input matrix (pos 0, dim 0).");
    assert(output2.data[0][1] === 2, "PE should be added to the input matrix (pos 0, dim 1).");
}


function runAllEmbeddingTests() {
    try {
        testEmbeddingForward();
        testPositionalEncodingForward();
        console.log("-> All Embedding tests loaded successfully.");
    } catch(e) {
        console.error("-> An Embedding test definition failed:", e.message);
    }
}

runAllEmbeddingTests();
