from .config import vocab, inv_vocab

def tokenize(text):
    tokens = []
    for word in text.lower().split():
        tokens.append(vocab.get(word, vocab["<unk>"]))
    return tokens

def detokenize(token_ids):
    return " ".join(inv_vocab.get(t, "<unk>") for t in token_ids)
