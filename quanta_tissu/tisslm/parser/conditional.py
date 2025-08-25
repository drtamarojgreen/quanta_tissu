import re

CONDITIONAL_PATTERNS = {
    'IF': re.compile(r'^\s*IF\s+\"([^\"]+)\"\s*$'),
    'ELSE': re.compile(r'^\s*ELSE\s*$'),
    'ENDIF': re.compile(r'^\s*ENDIF\s*$'),
}
