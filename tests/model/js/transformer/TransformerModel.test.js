// NOTE: This is a test file. Per instructions, it should not be executed.
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');
const { TransformerModel } = require('../../../../quanta_tissu/tisslm/js/transformer/TransformerModel.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function testTransformerModelForward() {
    console.log("Test: TransformerModel forward pass");
    const vocabSize = 10;
    const embedSize = 8;
    const numHeads = 2;
    const hiddenSize = 16;
    const numLayers = 2;

    const model = new TransformerModel(vocabSize, embedSize, numHeads, hiddenSize, numLayers);

    const seqLen = 5;
    const inputTokenIds = [1, 2, 3, 4, 5];

    const logits = model.forward(inputTokenIds);

    assert(logits.rows === seqLen, "Output logits should have correct sequence length.");
    assert(logits.cols === vocabSize, "Output logits should have correct vocabulary size dimension.");
}

function runAllModelTests() {
    try {
        testTransformerModelForward();
        console.log("-> All TransformerModel tests loaded successfully.");
    } catch(e) {
        console.error("-> An TransformerModel test definition failed:", e.message);
    }
}

runAllModelTests();
