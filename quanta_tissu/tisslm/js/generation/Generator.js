const { Matrix } = require('../core/Matrix.js');

class Generator {
    /**
     * @param {TransformerModel} model - The trained Transformer model.
     * @param {Tokenizer} tokenizer - The tokenizer.
     */
    constructor(model, tokenizer) {
        this.model = model;
        this.tokenizer = tokenizer;
    }

    /**
     * Generates a sequence of text.
     * @param {string} prompt - The initial text to start generation from.
     * @param {number} maxLen - The maximum length of the sequence to generate.
     * @param {number} eosToken - The End-Of-Sequence token ID to stop generation.
     */
    generate(prompt, maxLen, eosToken = -1) {
        if (!this.model || !this.tokenizer) {
            throw new Error("Generator must be initialized with a model and tokenizer.");
        }

        console.log(`Generating text with prompt: "${prompt}"`);

        // 1. Tokenize the input prompt
        let tokens = this.tokenizer.encode(prompt);

        // 2. Generate tokens one by one
        for (let i = 0; i < maxLen; i++) {
            // Get the model's prediction for the next token
            const logits = this.model.forward(tokens);

            // Focus on the logits for the very last token in the sequence
            const nextTokenLogits = logits.slice(logits.rows - 1, logits.rows, 0, logits.cols);

            // 3. Use a sampling strategy (greedy decoding)
            // Find the token with the highest probability (logit)
            const probabilities = nextTokenLogits.data[0];
            let nextToken = 0;
            let maxProb = -Infinity;
            for (let j = 0; j < probabilities.length; j++) {
                if (probabilities[j] > maxProb) {
                    maxProb = probabilities[j];
                    nextToken = j;
                }
            }

            // 4. Stop if EOS token is generated
            if (nextToken === eosToken) {
                console.log("EOS token generated. Stopping.");
                break;
            }

            // 5. Append the new token to the sequence
            tokens.push(nextToken);

            // Optional: Log the generated token
            // console.log(`Generated token ID: ${nextToken}, Decoded: "${this.tokenizer.decode([nextToken])}"`);
        }

        // 6. Decode the full sequence and return the text
        const generatedText = this.tokenizer.decode(tokens);
        console.log(`Generated sequence: "${generatedText}"`);
        return generatedText;
    }
}

module.exports = { Generator };
