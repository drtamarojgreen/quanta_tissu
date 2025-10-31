// NOTE: This is a test file. Per instructions, it should not be executed.
const fs = require('fs');
const path = require('path');
const { TokenDataset } = require('../../../../quanta_tissu/tisslm/js/training/Dataset.js');

function assert(condition, message) {
    if (!condition) {
        throw new Error(message || "Assertion failed");
    }
}

async function createDummyDataset(dir, content) {
    if (!fs.existsSync(dir)){
        fs.mkdirSync(dir, { recursive: true });
    }
    await fs.promises.writeFile(path.join(dir, 'train.txt'), content);
}

async function testDatasetLoadingAndBatching() {
    console.log("Test: Dataset loading and batching");

    const dataDir = path.join(__dirname, 'temp_data');
    const dataPath = path.join(dataDir, 'train.txt');
    await createDummyDataset(dataDir, "1 2 3 4 5 6 7 8 9 10");

    const dataset = new TokenDataset(dataPath);
    await dataset.load();

    assert(dataset.data.length === 10, "Dataset should load all tokens.");

    const batch = dataset.getBatch(0, 2, 3); // batch index 0, 2 sequences, 3 tokens long

    // Expected inputs: [[1,2,3], [2,3,4]]
    // Expected targets: [[2,3,4], [3,4,5]]
    assert(batch.inputs.length === 2, "Should create correct number of sequences.");
    assert(batch.inputs[0].join(',') === '1,2,3', "First input sequence is incorrect.");
    assert(batch.targets[1].join(',') === '3,4,5', "Second target sequence is incorrect.");
}

async function runAllDatasetTests() {
    const dataDir = path.join(__dirname, 'temp_data');
    try {
        await testDatasetLoadingAndBatching();
        console.log("-> All Dataset tests loaded successfully.");
    } catch(e) {
        console.error("-> A Dataset test definition failed:", e.message);
    } finally {
        if (fs.existsSync(dataDir)) {
            fs.rmSync(dataDir, { recursive: true, force: true });
        }
    }
}

runAllDatasetTests();
