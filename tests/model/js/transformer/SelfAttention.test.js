// NOTE: This is a test file. Per instructions, it should not be executed.
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');
const { SelfAttention } = require('../../../../quanta_tissu/tisslm/js/transformer/SelfAttention.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function testSelfAttentionForward() {
    console.log("Test: SelfAttention forward pass");
    const embedSize = 8;
    const numHeads = 2;
    const attention = new SelfAttention(embedSize, numHeads);

    const seqLen = 3;
    const input = Matrix.random(seqLen, embedSize);

    // Make weights identity for predictable Q, K, V
    attention.w_q = Matrix.identity(embedSize);
    attention.w_k = Matrix.identity(embedSize);
    attention.w_v = Matrix.identity(embedSize);
    attention.w_o = Matrix.identity(embedSize);

    const output = attention.forward(input);

    assert(output.rows === seqLen, "Output should have correct sequence length.");
    assert(output.cols === embedSize, "Output should have correct embedding size.");
}

function testSelfAttentionWithMask() {
    console.log("Test: SelfAttention forward pass with mask");
    const embedSize = 4;
    const numHeads = 2;
    const attention = new SelfAttention(embedSize, numHeads);

    const seqLen = 3;
    const input = Matrix.random(seqLen, embedSize);

    // Causal mask (for decoding)
    const mask = new Matrix([
        [1, 0, 0],
        [1, 1, 0],
        [1, 1, 1]
    ]);

    const output = attention.forward(input, mask);

    // Basic check, more detailed checks would require inspecting attention weights
    assert(output.rows === seqLen, "Masked output should have correct sequence length.");
    assert(output.cols === embedSize, "Masked output should have correct embedding size.");
}


// Add identity matrix helper if not present
if (!Matrix.identity) {
    Matrix.identity = function(size) {
        const data = Array(size).fill(0).map((_, i) =>
            Array(size).fill(0).map((__, j) => (i === j ? 1 : 0))
        );
        return new Matrix(data);
    }
}

function runAllAttentionTests() {
    try {
        testSelfAttentionForward();
        testSelfAttentionWithMask();
        console.log("-> All SelfAttention tests loaded successfully.");
    } catch(e) {
        console.error("-> An SelfAttention test definition failed:", e.message);
    }
}

runAllAttentionTests();
