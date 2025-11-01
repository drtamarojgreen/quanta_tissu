// NOTE: This is a test file. Per instructions, it should not be executed.
const fs = require('fs');
const path = require('path');
const { Tokenizer } = require('../../../../quanta_tissu/tisslm/js/tokenizer/Tokenizer.js');

// A simple assertion function for testing purposes.
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
    await fs.promises.writeFile(path.join(dir, 'test_vocab.json'), JSON.stringify(vocab));
    await fs.promises.writeFile(path.join(dir, 'test_merges.txt'), merges);
}

const DUMMY_VOCAB = {
    "a": 1, "b": 2, "c": 3, "ab": 4, "bc": 5, "abc": 6, " ": 7
};

const DUMMY_MERGES = `
# These are merges
a b
b c
ab c
`;

async function testTokenizerLoading() {
    console.log("Test: Tokenizer loading");
    const tokenizer = new Tokenizer();
    const modelDir = path.join(__dirname, 'temp_model');
    await createDummyTokenizerFiles(modelDir, DUMMY_VOCAB, DUMMY_MERGES);

    await tokenizer.load(path.join(modelDir, 'test'));

    assert(tokenizer.vocabSize === 7, "Vocab size should be set correctly.");
    assert(tokenizer.merges.has('a,b'), "Merges should be loaded correctly.");
    assert(tokenizer.merges.get('ab,c') === 2, "Merge rank should be correct.");
}

async function testEncode() {
    console.log("Test: Encode");
    const tokenizer = new Tokenizer();
    const modelDir = path.join(__dirname, 'temp_model');
    await tokenizer.load(path.join(modelDir, 'test'));

    const text = "abc";
    const encoded = tokenizer.encode(text); // Should become "ab" + "c" -> "abc"
    assert(encoded.length === 1 && encoded[0] === 6, "Encode should correctly apply merges.");

    const text2 = "a b c";
    const encoded2 = tokenizer.encode(text2); // Should be [a, space, b, space, c] -> [1, 7, 2, 7, 3]
    assert(encoded2.join(',') === '1,7,2,7,3', "Encode should handle spaces correctly.");
}

async function testDecode() {
    console.log("Test: Decode");
    const tokenizer = new Tokenizer();
    const modelDir = path.join(__dirname, 'temp_model');
    await tokenizer.load(path.join(modelDir, 'test'));

    const tokens = [6];
    const decoded = tokenizer.decode(tokens);
    assert(decoded === "abc", "Decode should correctly convert token ID to string.");

    const tokens2 = [1, 7, 2, 7, 3];
    const decoded2 = tokenizer.decode(tokens2);
    assert(decoded2.trim() === "a b c", "Decode should handle spaces correctly.");
}


async function runAllTokenizerTests() {
    try {
        await testTokenizerLoading();
        await testEncode();
        await testDecode();
        console.log("-> All Tokenizer tests loaded successfully.");
    } catch (e) {
        console.error("-> A tokenizer test definition failed:", e.message);
    } finally {
        // Clean up dummy files
        const modelDir = path.join(__dirname, 'temp_model');
        if (fs.existsSync(modelDir)) {
            fs.rmSync(modelDir, { recursive: true, force: true });
        }
    }
}

runAllTokenizerTests();
