import re

_PATTERNS = {
    'TASK': re.compile(r'^\s*TASK\s+\"([^\"]+)\"\s*$'),
    'STEP': re.compile(r'^\s*STEP\s+\"([^\"]+)\"\s*\{\s*$'),
    'SETUP': re.compile(r'^\s*SETUP\s*\{\s*$'),
    'BLOCK_END': re.compile(r'^\s*\}\s*$'),
    'RUN': re.compile(r'^\s*RUN\s+([\'"])(.*?)\1\s*$'),
    'LOG': re.compile(r'^\s*LOG\s+([\'"])(.*?)\1\s*$'),
    'WRITE': re.compile(r'^\s*WRITE\s+\"([^\"]+)\"\s+<<(\w+)\s*$'),
    'ASSERT': re.compile(r'^\s*ASSERT\s+(.+?)\s*$'),
    'READ': re.compile(r'^\s*READ\s+\"([^\"]+)\"\s+AS\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*$'),
    'PROMPT_AGENT': re.compile(r'^\s*PROMPT_AGENT\s+\"([^\"]+)\"(?:\s+INTO\s+([a-zA-Z_][a-zA-Z0-9_]*))?\s*$'),
    'COMMENT': re.compile(r'^\s*#.*$'),
    'EMPTY': re.compile(r'^\s*$'),
}
