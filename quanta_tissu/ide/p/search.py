import re
import os
import glob

class Search:
    """
    Handles all search and replace functionality for the IDE.
    """
    def search_and_replace(self, content, regex, replacement, count=0):
        """
        Performs a regex-based search and replace on the given content.
        Returns the new string and the number of substitutions.
        If count is 0, all occurrences are replaced.
        """
        return re.subn(regex, replacement, content, count=count)

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

    def bulk_search_and_replace(self, directory, file_pattern, search_regex, replacement):
        """
        Performs search and replace across multiple files in a directory.
        """
        try:
            pathname = os.path.join(directory, file_pattern)
            files_to_process = glob.glob(pathname, recursive=True)

            files_changed_count = 0
            total_replacements = 0

            if not files_to_process:
                return "No files found matching the pattern.", 0, 0

            for filepath in files_to_process:
                if os.path.isfile(filepath):
                    try:
                        with open(filepath, 'r', encoding='utf-8') as f:
                            content = f.read()

                        new_content, num_replacements = re.subn(search_regex, replacement, content)

                        if num_replacements > 0:
                            with open(filepath, 'w', encoding='utf-8') as f:
                                f.write(new_content)
                            files_changed_count += 1
                            total_replacements += num_replacements
                    except Exception as e:
                        print(f"Could not process file {filepath}: {e}")

            return f"Replaced {total_replacements} occurrences in {files_changed_count} files.", total_replacements, files_changed_count
        except re.error as e:
            return f"Regex error: {e}", 0, 0
        except Exception as e:
            return f"An error occurred: {e}", 0, 0
