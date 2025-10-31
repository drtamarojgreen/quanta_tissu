// NOTE: This is a test file. Per instructions, it should not be executed.
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');
const { FeedForward } = require('../../../../quanta_tissu/tisslm/js/transformer/FeedForward.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function testFeedForward() {
    console.log("Test: FeedForward pass");
    const embedSize = 8;
    const hiddenSize = 16;
    const ff = new FeedForward(embedSize, hiddenSize);

    const seqLen = 4;
    const input = Matrix.random(seqLen, embedSize);

    const output = ff.forward(input);

    assert(output.rows === seqLen, "Output should have correct sequence length.");
    assert(output.cols === embedSize, "Output should have correct embedding size.");
}


function runAllFeedTests() {
    try {
        testFeedForward();
        console.log("-> All FeedForward tests loaded successfully.");
    } catch(e) {
        console.error("-> An FeedForward test definition failed:", e.message);
    }
}

runAllFeedTests();
