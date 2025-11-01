// NOTE: This is a test file. Per instructions, it should not be executed.
const { Generator } = require('../../../../quanta_tissu/tisslm/js/generation/Generator.js');
const { Matrix } = require('../../../../quanta_tissu/tisslm/js/core/Matrix.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

function testGenerator() {
    console.log("Test: Generator forward pass");

    // Mock Tokenizer
    const mockTokenizer = {
        encode: (prompt) => {
            if (prompt === "a") return [1];
            return [];
        },
        decode: (tokens) => {
             return tokens.map(t => {
                if (t === 1) return "a";
                if (t === 2) return "b";
                if (t === 3) return "c";
                return "";
            }).join('');
        }
    };

    // Mock Model
    const mockModel = {
        forward: (tokens) => {
            // A simple rule-based mock:
            // if input is [1], predict token 2
            // if input is [1, 2], predict token 3 (EOS)
            let nextTokenLogits;
            if (tokens.length === 1 && tokens[0] === 1) {
                // To predict token ID 2, max logit must be at index 2
                nextTokenLogits = [0.1, 0.2, 2.5, 0.1];
            } else if (tokens.length === 2 && tokens[0] === 1 && tokens[1] === 2) {
                 // To predict token ID 3, max logit must be at index 3
                 nextTokenLogits = [0.1, 0.2, 0.1, 3.0];
            } else {
                nextTokenLogits = [0, 0, 0, 0];
            }
            // The model outputs logits for the whole sequence, so we create a dummy history
            const historyData = tokens.length > 1 ? Matrix.zeros(tokens.length - 1, 4).data : [];
            const finalLogits = new Matrix([...historyData, nextTokenLogits]);
            return finalLogits;
        }
    };

    const generator = new Generator(mockModel, mockTokenizer);

    const prompt = "a";
    const maxLen = 5;
    const eosToken = 3;

    const generatedText = generator.generate(prompt, maxLen, eosToken);

    // Expected generation:
    // 1. Input: "a" -> tokens [1]
    // 2. Model([1]) -> predicts token 2. Tokens are now [1, 2]
    // 3. Model([1, 2]) -> predicts token 3 (EOS). Stop.
    // 4. Decode [1, 2] -> "ab"
    const expected = "ab";

    assert(generatedText === expected, `Generated text should be "${expected}", but got "${generatedText}"`);
}


function runAllGeneratorTests() {
    try {
        testGenerator();
        console.log("-> All Generator tests loaded successfully.");
    } catch(e) {
        console.error("-> A Generator test definition failed:", e.message);
    }
}

runAllGeneratorTests();
