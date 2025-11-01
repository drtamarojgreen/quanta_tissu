// NOTE: This is a test file. Per instructions, it should not be executed.
// This test file checks the integration of components in the main API.
const fs = require('fs');
const path = require('path');
const { TissLM, createTissLM } = require('../../../../quanta_tissu/tisslm/js/api/index.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

// Helper to create dummy tokenizer files for testing
async function createDummyTokenizerFiles(dir, vocab, merges) {
    if (!fs.existsSync(dir)){
        fs.mkdirSync(dir, { recursive: true });
    }
    await fs.promises.writeFile(path.join(dir, 'test_tokenizer_vocab.json'), JSON.stringify(vocab));
    await fs.promises.writeFile(path.join(dir, 'test_tokenizer_merges.txt'), merges);
}

const DUMMY_VOCAB = { "a": 1, "b": 2, "ab": 3, " ": 4 };
const DUMMY_MERGES = "a b";


async function testApiInitializationAndTokenization() {
    console.log("Test: API Initialization and Tokenization");

    const modelDir = path.join(__dirname, 'temp_model_api');
    await createDummyTokenizerFiles(modelDir, DUMMY_VOCAB, DUMMY_MERGES);

    const config = {
        tokenizerPath: path.join(modelDir, 'test_tokenizer'),
        vocabSize: 4
    };

    const tisslm = await createTissLM(config);

    assert(tisslm.isInitialized, "TissLM instance should be initialized.");

    const text = "ab";
    const tokens = tisslm.tokenize(text); // "a"+"b" -> "ab"

    assert(tokens.length === 1 && tokens[0] === 3, "API tokenization should work correctly.");

    const decoded = tisslm.decode([3]);
    assert(decoded === "ab", "API decoding should work correctly.");
}

// This is conceptual as it depends on the generator and a mock model
function testApiGeneration() {
    console.log("Test: API Generation (conceptual)");
     const config = {
        tokenizerPath: 'dummy',
        vocabSize: 4,
        dbUrl: 'http://localhost:8080' // Add the required dbUrl
    };
    const tisslm = new TissLM(config);
    tisslm.isInitialized = true; // Manually set for this test

    // Mock the generator
    tisslm.generator = {
        generate: (prompt, maxLen) => {
            return prompt + " generated";
        }
    };

    const result = tisslm.generate("hello", 10);
    assert(result === "hello generated", "API should call the generator.");
}


async function runAllApiTests() {
    const modelDir = path.join(__dirname, 'temp_model_api');
    try {
        await testApiInitializationAndTokenization();
        testApiGeneration();
        console.log("-> All API tests loaded successfully.");
    } catch(e) {
        console.error("-> An API test definition failed:", e.message);
    } finally {
        if (fs.existsSync(modelDir)) {
            fs.rmSync(modelDir, { recursive: true, force: true });
        }
    }
}

runAllApiTests();
