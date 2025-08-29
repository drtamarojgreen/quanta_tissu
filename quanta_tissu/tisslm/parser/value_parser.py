import json

def parse_value(value_str: str):
    """
    Parses a string into its appropriate TissLang value type (string, number, boolean, object, list).
    """
    # Try boolean
    if value_str.lower() == 'true':
        return True
    if value_str.lower() == 'false':
        return False

    # Try number (int or float)
    try:
        return int(value_str)
    except ValueError:
        try:
            return float(value_str)
        except ValueError:
            pass # Not a number, continue

    # Try JSON (object or list)
    if (value_str.startswith('{') and value_str.endswith('}')) or \
       (value_str.startswith('[') and value_str.endswith(']')):
        try:
            return json.loads(value_str)
        except json.JSONDecodeError:
            pass # Not valid JSON, continue

    # Default to string (remove quotes if present)
    if value_str.startswith('"') and value_str.endswith('"'):
        return value_str[1:-1]
    if value_str.startswith("'") and value_str.endswith("'"):
        return value_str[1:-1]
    return value_str
