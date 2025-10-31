// TissLM BPE Tokenizer for Node.js
const fs = require('fs');

class Tokenizer {
    constructor() {
        this.vocab = null;
        this.merges = null;
        this.vocabSize = 0;
    }

    // Loads vocabulary and merges from file paths.
    // The tokenizerPath should be the base path, e.g., 'models/tokenizers/revised_tokenizer'
    async load(tokenizerPath) {
        const vocabPath = `${tokenizerPath}_vocab.json`;
        const mergesPath = `${tokenizerPath}_merges.txt`;

        try {
            const vocabData = await fs.promises.readFile(vocabPath, 'utf8');
            this.vocab = JSON.parse(vocabData);
            this.vocabSize = Object.keys(this.vocab).length;
        } catch (error) {
            throw new Error(`Failed to load or parse vocabulary file: ${vocabPath}. Error: ${error.message}`);
        }

        try {
            const mergesData = await fs.promises.readFile(mergesPath, 'utf8');
            this.merges = this._parseMerges(mergesData);
        } catch (error) {
            throw new Error(`Failed to load or parse merges file: ${mergesPath}. Error: ${error.message}`);
        }
    }

    _parseMerges(mergesData) {
        const merges = new Map();
        mergesData.split('\n')
            .filter(line => line && !line.startsWith('#')) // Ignore empty lines and comments
            .forEach((line, i) => {
                const parts = line.trim().split(/\s+/);
                if (parts.length === 2) {
                    merges.set(parts.join(','), i); // Store merge pair with its rank
                }
            });
        return merges;
    }

    _getPairs(word) {
        const pairs = new Set();
        for (let i = 0; i < word.length - 1; i++) {
            pairs.add([word[i], word[i + 1]]);
        }
        return pairs;
    }

    // Encodes a string into a sequence of token IDs.
    encode(text) {
        if (!this.vocab || !this.merges) {
            throw new Error("Tokenizer has not been loaded. Call load() first.");
        }

        // Basic pre-tokenization: split by space and punctuation
        const pretokenized = text.split(/(\s+)|([!?".,;'])/).filter(Boolean);
        let output = [];

        for (const token of pretokenized) {
            let symbols = token.split('');

            while (symbols.length > 1) {
                let minRank = Infinity;
                let bestPair = null;

                for (let i = 0; i < symbols.length - 1; i++) {
                    const pairKey = [symbols[i], symbols[i+1]].join(',');
                    const rank = this.merges.get(pairKey);
                    if (rank !== undefined && rank < minRank) {
                        minRank = rank;
                        bestPair = [symbols[i], symbols[i+1]];
                    }
                }

                if (bestPair === null) {
                    break; // No more merges possible
                }

                // Apply the best merge
                const newSymbols = [];
                let i = 0;
                while (i < symbols.length) {
                    if (i < symbols.length - 1 && symbols[i] === bestPair[0] && symbols[i+1] === bestPair[1]) {
                        newSymbols.push(bestPair.join(''));
                        i += 2;
                    } else {
                        newSymbols.push(symbols[i]);
                        i += 1;
                    }
                }
                symbols = newSymbols;
            }

            // Convert merged symbols to token IDs
            output.push(...symbols.map(s => this.vocab[s] || 0)); // Use 0 for unknown tokens
        }

        return output;
    }

    // Decodes a sequence of token IDs back into a string.
    decode(tokenIds) {
        if (!this.vocab) {
            throw new Error("Tokenizer has not been loaded. Call load() first.");
        }

        const invVocab = Object.fromEntries(Object.entries(this.vocab).map(([k, v]) => [v, k]));

        return tokenIds.map(id => invVocab[id] || '').join('');
    }
}

module.exports = { Tokenizer };
