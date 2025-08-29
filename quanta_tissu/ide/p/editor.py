import re

class Editor:
    def search_and_replace(self, content, regex, replacement):
        """
        Performs a regex-based search and replace on the given content.
        """
        return re.sub(regex, replacement, content)

    def search_and_replace_in_block(self, content, start_regex, end_regex, search_regex, replacement):
        """
        Performs search and replace within a block defined by start and end regexes.
        Returns the new content and an error message if any.
        """
        try:
            start_match = re.search(start_regex, content)
            if not start_match:
                return content, f"Starting regex '{start_regex}' not found"

            end_match = re.search(end_regex, content[start_match.end():])
            if not end_match:
                return content, f"Ending regex '{end_regex}' not found after start"

            block_start = start_match.end()
            block_end = start_match.end() + end_match.start()

            block_content = content[block_start:block_end]
            new_block_content = re.sub(search_regex, replacement, block_content)

            return content[:block_start] + new_block_content + content[block_end:], None
        except re.error as e:
            return content, f"Regex error: {e}"

    def format_buffer(self, lines):
        """
        Formats the indentation of the buffer.
        Assumes TASK blocks are not nested.
        """
        new_lines = []
        indent_level = 0
        indent_size = 4

        for line in lines:
            stripped_line = line.strip()

            if not stripped_line:
                new_lines.append("")
                continue

            # TASK always resets the indentation to 0 for itself.
            if stripped_line.startswith("TASK"):
                indent_level = 0

            new_lines.append(" " * indent_level * indent_size + stripped_line)

            # After a TASK line, the next lines should be indented.
            if stripped_line.startswith("TASK"):
                indent_level = 1

        return new_lines
