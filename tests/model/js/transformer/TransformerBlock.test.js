// NOTE: This is a test file. Per instructions, it should not be executed.
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');
const { TransformerBlock } = require('../../../../quanta_tissu/tisslm/js/transformer/TransformerBlock.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function testTransformerBlock() {
    console.log("Test: TransformerBlock forward pass");
    const embedSize = 8;
    const numHeads = 2;
    const hiddenSize = 16;
    const block = new TransformerBlock(embedSize, numHeads, hiddenSize);

    const seqLen = 4;
    const input = Matrix.random(seqLen, embedSize);

    const output = block.forward(input);

    assert(output.rows === seqLen, "Output should have correct sequence length.");
    assert(output.cols === embedSize, "Output should have correct embedding size.");
}

function runAllBlockTests() {
    try {
        testTransformerBlock();
        console.log("-> All TransformerBlock tests loaded successfully.");
    } catch(e) {
        console.error("-> An TransformerBlock test definition failed:", e.message);
    }
}

runAllBlockTests();
