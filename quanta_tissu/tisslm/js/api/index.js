// TissLM Node.js Model - Main API
// This file integrates all components and provides a simple top-level API.

const { TissDBClient } = require('../database/TissDBClient.js');
const { Tokenizer } = require('../tokenizer/Tokenizer.js');
const { TransformerModel } = require('../transformer/TransformerModel.js');
const { Generator } = require('../generation/Generator.js');

// --- Main TissLM Class ---

class TissLM {
    constructor(config) {
        this.config = config;
        this.dbClient = new TissDBClient(config.dbUrl, config.dbToken);
        this.tokenizer = new Tokenizer();
        this.model = new TransformerModel(
            config.vocabSize,
            config.embedSize,
            config.numHeads,
            config.hiddenSize,
            config.numLayers
        );
        this.generator = new Generator(this.model, this.tokenizer);
        this.isInitialized = false;
    }

    // Initialization is separate to allow for async loading of tokenizer files.
    async init() {
        if (this.isInitialized) return;
        console.log("Initializing TissLM...");
        await this.tokenizer.load(this.config.tokenizerPath);
        this.isInitialized = true;
        console.log("TissLM Initialized successfully.");
    }

    /**
     * Tokenizes a piece of text.
     * @param {string} text - The text to tokenize.
     * @returns {Array<number>} An array of token IDs.
     */
    tokenize(text) {
        if (!this.isInitialized) throw new Error("TissLM is not initialized. Call init() first.");
        return this.tokenizer.encode(text);
    }

     /**
     * Decodes a sequence of token IDs.
     * @param {Array<number>} tokenIds - The token IDs to decode.
     * @returns {string} The decoded text.
     */
    decode(tokenIds) {
        if (!this.isInitialized) throw new Error("TissLM is not initialized. Call init() first.");
        return this.tokenizer.decode(tokenIds);
    }

    /**
     * Generates text from a prompt.
     * @param {string} prompt - The starting text.
     * @param {number} maxLen - The maximum number of new tokens to generate.
     * @returns {string} The generated text.
     */
    generate(prompt, maxLen = 50) {
        if (!this.isInitialized) throw new Error("TissLM is not initialized. Call init() first.");
        return this.generator.generate(prompt, maxLen);
    }

    // Placeholder for the training functionality
    async train(trainingConfig) {
        if (!this.isInitialized) throw new Error("TissLM is not initialized. Call init() first.");
        console.log("Training is not fully implemented in this version.");
        // The conceptual TrainingLoop.js would be integrated and called here.
        return { status: "Training pipeline is conceptual." };
    }
}


// --- Factory Function ---
// Provides a convenient way to create and initialize a TissLM instance.

async function createTissLM(config) {
    // A default configuration
    const defaultConfig = {
        dbUrl: 'http://localhost:8080',
        dbToken: 'static_test_token',
        tokenizerPath: 'models/tokenizers/revised_tokenizer',
        vocabSize: 1000,
        embedSize: 32,
        numHeads: 4,
        hiddenSize: 64,
        numLayers: 2,
    };

    const finalConfig = { ...defaultConfig, ...config };

    const instance = new TissLM(finalConfig);
    await instance.init();
    return instance;
}


module.exports = {
    TissLM,
    createTissLM
};
