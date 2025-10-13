import fs from 'fs';
import path, { dirname } from 'path';
import { fileURLToPath } from 'url';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const DATA_DIR = path.join(__dirname, 'lite_data');
const DATA_FILE = path.join(DATA_DIR, 'tissdb.json');

class TissDBLite {
    constructor() {
        this.collections = {};
        this._loadData();
    }

    _loadData() {
        if (!fs.existsSync(DATA_DIR)) {
            fs.mkdirSync(DATA_DIR, { recursive: true });
        }
        if (fs.existsSync(DATA_FILE)) {
            try {
                const data = fs.readFileSync(DATA_FILE, 'utf8');
                this.collections = JSON.parse(data);
            } catch (e) {
                console.error('Error loading TissDBLite data:', e.message);
                this.collections = {}; // Reset on error
            }
        }
    }

    _saveData() {
        try {
            if (!fs.existsSync(DATA_DIR)) {
                fs.mkdirSync(DATA_DIR, { recursive: true });
            }
            fs.writeFileSync(DATA_FILE, JSON.stringify(this.collections, null, 2), 'utf8');
        } catch (e) {
            console.error('Error saving TissDBLite data:', e.message);
        }
    }

    /**
     * Generates a simple unique ID.
     * @private
     * @returns {string} A unique ID.
     */
    _generateId() {
        const timestamp = new Date().getTime();
        const random = Math.random().toString(36).substring(2, 9);
        return `${timestamp}-${random}`;
    }

    /**
     * Creates an in-memory collection.
     * @param {string} name - The name of the collection.
     */
    createCollection(name) {
        this.collections[name] = [];
        this._saveData();
    }

    /**
     * Inserts an item into an in-memory collection.
     * @param {string} collectionName - The name of the collection.
     * @param {object} item - The item to insert.
     * @returns {object} The inserted item with generated fields.
     */
    insert(collectionName, item) {
        if (!this.collections[collectionName]) {
            throw new Error(`Collection '${collectionName}' does not exist.`);
        }
        const newItem = {
            ...item,
            _id: this._generateId(),
            _createdDate: new Date().toISOString(),
            _updatedDate: new Date().toISOString(),
        };
        this.collections[collectionName].push(newItem);
        this._saveData();
        return newItem;
    }

    /**
     * Finds items in an in-memory collection using a query string.
     * @param {string} collectionName - The name of the collection.
     * @param {string} condition_string - The query string for filtering.
     * @returns {Array} The found items.
     */
    find(collectionName, condition_string) {
        if (!this.collections[collectionName]) {
            throw new Error(`Collection '${collectionName}' does not exist.`);
        }
        if (!condition_string) {
            return this.collections[collectionName];
        }

        const filterFunction = this._createFilterFunction(condition_string);
        return this.collections[collectionName].filter(filterFunction);
    }

    /**
     * Updates an item in an in-memory collection.
     * @param {string} collectionName - The name of the collection.
     * @param {object} item - The item to update, must contain an _id.
     * @returns {object} The updated item.
     */
    update(collectionName, item) {
        if (!this.collections[collectionName]) {
            throw new Error(`Collection '${collectionName}' does not exist.`);
        }
        const index = this.collections[collectionName].findIndex(i => i._id === item._id);
        if (index === -1) {
            throw new Error(`Item with id '${item._id}' not found in collection '${collectionName}'.`);
        }
        const updatedItem = {
            ...this.collections[collectionName][index],
            ...item,
            _updatedDate: new Date().toISOString(),
        };
        this.collections[collectionName][index] = updatedItem;
        this._saveData();
        return updatedItem;
    }

    /**
     * Removes an item from an in-memory collection.
     * @param {string} collectionName - The name of the collection.
     * @param {string} itemId - The id of the item to remove.
     */
    remove(collectionName, itemId) {
        if (!this.collections[collectionName]) {
            throw new Error(`Collection '${collectionName}' does not exist.`);
        }
        const index = this.collections[collectionName].findIndex(i => i._id === itemId);
        if (index !== -1) {
            this.collections[collectionName].splice(index, 1);
        }
    }

    /**
     * Inserts multiple items into an in-memory collection.
     * @param {string} collectionName - The name of the collection.
     * @param {Array<object>} items - An array of items to insert.
     * @returns {Array<object>} The inserted items with generated fields.
     */
    bulkInsert(collectionName, items) {
        if (!this.collections[collectionName]) {
            throw new Error(`Collection '${collectionName}' does not exist.`);
        }
        const insertedItems = [];
        for (const item of items) {
            const newItem = {
                ...item,
                _id: this._generateId(),
                _createdDate: new Date().toISOString(),
                _updatedDate: new Date().toISOString(),
            };
            this.collections[collectionName].push(newItem);
            insertedItems.push(newItem);
        }
        this._saveData();
        return insertedItems;
    }

    /**
     * Exports all items from an in-memory collection.
     * @param {string} collectionName - The name of the collection.
     * @returns {Array<object>} An array containing all items in the collection.
     */
    exportCollection(collectionName) {
        if (!this.collections[collectionName]) {
            throw new Error(`Collection '${collectionName}' does not exist.`);
        }
        // Return a deep copy to prevent external modification of the internal state
        return JSON.parse(JSON.stringify(this.collections[collectionName]));
    }

    /**
     * Executes a TissLang QUERY command against a Wix Data collection.
     * @param {object} command - The parsed QUERY command from TissLang.
     * @returns {Promise<Array>} A promise that resolves to the query results.
     */
    async executeQuery(command) {
        if (typeof wixData === 'undefined') {
            throw new Error('Wix Data functionality is only available in the Wix Velo environment.');
        }
        const { collection, condition_string } = command;

        const filter = this.parseCondition(condition_string);

        let query = wixData.query(collection);
        if (filter) {
            query = query.filter(filter);
        }

        const results = await query.find();
        return results.items;
    }

    /**
     * Parses a TissLang condition string into a Wix Data filter.
     * @param {string} condition_string - The condition string from the QUERY command.
     * @returns {object} A Wix Data filter object.
     */
    parseCondition(condition_string) {
        if (typeof wixData === 'undefined') {
            throw new Error('Wix Data functionality is only available in the Wix Velo environment.');
        }
        if (!condition_string) {
            return null;
        }

        const tokens = this._tokenize(condition_string);
        const rpn = this._toRpn(tokens);
        return this._buildWixFilter(rpn);
    }

    _createFilterFunction(condition_string) {
        const tokens = this._tokenize(condition_string);
        const rpn = this._toRpn(tokens);

        return (item) => {
            const stack = [];
            for (const token of rpn) {
                if (token.type === 'LITERAL') {
                    stack.push(token.value);
                } else if (token.type === 'IDENTIFIER') {
                    stack.push(item[token.value]);
                } else if (token.type === 'OPERATOR') {
                    const right = stack.pop();
                    const left = stack.pop();
                    stack.push(this._evaluateOperator(left, token.value, right));
                }
            }
            return stack[0];
        };
    }

    _evaluateOperator(left, op, right) {
        switch (op) {
            case '=': return left == right;
            case '!=': return left != right;
            case '>': return left > right;
            case '>=': return left >= right;
            case '<': return left < right;
            case '<=': return left <= right;
            case 'AND': return left && right;
            case 'OR': return left || right;
            default: throw new Error(`Unsupported operator ${op}`);
        }
    }

    _tokenize(condition_string) {
        const regex = /\s*(=|!=|>=|<=|>|<|\(|\)|AND|OR)\s*|([a-zA-Z_][a-zA-Z0-9_]*)|(".*?")|([0-9]+(?:\.[0-9]+)?)\s*/g;
        const tokens = [];
        let match;
        while ((match = regex.exec(condition_string)) !== null) {
            if (match[1]) { // Operator or parenthesis
                const op = match[1].toUpperCase();
                if (['AND', 'OR'].includes(op)) {
                    tokens.push({ type: 'OPERATOR', value: op, precedence: 1, associativity: 'Left' });
                } else if (['=', '!=', '>', '>=', '<', '<='].includes(op)) {
                    tokens.push({ type: 'OPERATOR', value: op, precedence: 2, associativity: 'Left' });
                } else if (op === '(') {
                    tokens.push({ type: 'LPAREN', value: op });
                } else if (op === ')') {
                    tokens.push({ type: 'RPAREN', value: op });
                }
            } else if (match[2]) { // Identifier
                tokens.push({ type: 'IDENTIFIER', value: match[2] });
            } else if (match[3]) { // String literal
                tokens.push({ type: 'LITERAL', value: match[3].slice(1, -1) });
            } else if (match[4]) { // Number literal
                tokens.push({ type: 'LITERAL', value: Number(match[4]) });
            }
        }
        return tokens;
    }

    _toRpn(tokens) {
        const output = [];
        const operators = [];
        for (const token of tokens) {
            if (token.type === 'IDENTIFIER' || token.type === 'LITERAL') {
                output.push(token);
            } else if (token.type === 'OPERATOR') {
                while (operators.length > 0 && operators[operators.length - 1].type === 'OPERATOR' &&
                       (operators[operators.length - 1].precedence > token.precedence ||
                        (operators[operators.length - 1].precedence === token.precedence && token.associativity === 'Left'))) {
                    output.push(operators.pop());
                }
                operators.push(token);
            } else if (token.type === 'LPAREN') {
                operators.push(token);
            } else if (token.type === 'RPAREN') {
                while (operators.length > 0 && operators[operators.length - 1].type !== 'LPAREN') {
                    output.push(operators.pop());
                }
                if (operators.length === 0) throw new Error("Mismatched parentheses");
                operators.pop(); // Pop LPAREN
            }
        }
        while (operators.length > 0) {
            if (operators[operators.length - 1].type === 'LPAREN') throw new Error("Mismatched parentheses");
            output.push(operators.pop());
        }
        return output;
    }

    _buildWixFilter(rpn) {
        if (typeof wixData === 'undefined') {
            throw new Error('Wix Data functionality is only available in the Wix Velo environment.');
        }
        const stack = [];
        for (const token of rpn) {
            if (token.type === 'LITERAL' || token.type === 'IDENTIFIER') {
                stack.push(token);
            } else if (token.type === 'OPERATOR') {
                const right = stack.pop();
                const left = stack.pop();

                if (token.value === 'AND') {
                    stack.push(left.and(right));
                } else if (token.value === 'OR') {
                    stack.push(left.or(right));
                } else {
                    const field = left.value;
                    const value = right.value;
                    let filter;
                    switch (token.value) {
                        case '=': filter = wixData.filter().eq(field, value); break;
                        case '!=': filter = wixData.filter().ne(field, value); break;
                        case '>': filter = wixData.filter().gt(field, value); break;
                        case '>=': filter = wixData.filter().ge(field, value); break;
                        case '<': filter = wixData.filter().lt(field, value); break;
                        case '<=': filter = wixData.filter().le(field, value); break;
                        default: throw new Error(`Unsupported Wix operator ${token.value}`);
                    }
                    stack.push(filter);
                }
            }
        }
        return stack[0];
    }
}

export { TissDBLite };
