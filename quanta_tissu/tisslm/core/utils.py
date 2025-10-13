def convert_to_utf8(text, current_encoding='latin-1'):
    """
    Attempts to decode text from its current_encoding and then encode it to UTF-8.
    Useful for handling text with mixed or unknown encodings by first normalizing it.
    """
    if isinstance(text, bytes):
        # If it's bytes, decode it first using the specified current_encoding
        decoded_text = text.decode(current_encoding, errors='replace')
    else:
        # If it's already a string, assume it's already decoded (e.g., from a file read)
        decoded_text = text

    # Ensure the text is then encoded to UTF-8
    return decoded_text.encode('utf-8', errors='replace').decode('utf-8')
