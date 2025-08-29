import argparse
import os
import sys
import glob
import re
import fnmatch

def find_files(pattern):
    """Finds files matching a glob pattern."""
    return glob.glob(pattern, recursive=True)

def search_and_replace_in_file(filepath, search_pattern, replace_string, is_regex, case_sensitive):
    """Searches for a pattern in a single file and performs replacement if replace_string is provided.
    Handles multi-line patterns by reading the entire file content.
    Returns (found_match, original_content, modified_content) where modified_content is None if no replacement or no changes.
    """
    original_content = ""
    modified_content = None
    found_match = False

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            original_content = f.read()

        target_content = original_content
        pattern_to_use = search_pattern

        if not case_sensitive:
            target_content = original_content.lower()
            pattern_to_use = search_pattern.lower()

        if is_regex:
            flags = re.DOTALL # Allow . to match newlines
            if not case_sensitive:
                flags |= re.IGNORECASE
            
            if re.search(pattern_to_use, target_content, flags=flags):
                found_match = True
                if replace_string is not None:
                    modified_content = re.sub(search_pattern, replace_string, original_content, flags=flags)
        else:
            if pattern_to_use in target_content:
                found_match = True
                if replace_string is not None:
                    # For literal multi-line replacement, replace all occurrences
                    modified_content = original_content.replace(search_pattern, replace_string)

        if modified_content == original_content:
            modified_content = None # No actual change made

    except Exception as e:
        print(f"Error processing file {filepath}: {e}", file=sys.stderr)
        return False, "", None

    return found_match, original_content, modified_content

def main():
    parser = argparse.ArgumentParser(description="Search and replace utility for TissLM Python IDE.")


    parser.add_argument(
        "--pattern",
        type=str,
        help="The text string or regular expression pattern to search for."
    )
    parser.add_argument(
        "--pattern-file",
        type=str,
        help="Path to a file containing the text string or regular expression pattern to search for."
    )
    parser.add_argument(
        "--replace",
        type=str,
        default=None,
        help="The string to replace the found pattern with. If omitted, only search is performed."
    )
    parser.add_argument(
        "--replace-file",
        type=str,
        help="Path to a file containing the replacement text."
    )
    parser.add_argument(
        "--files",
        type=str,
        default=None, # Will need to implement glob matching later
        help="A glob pattern to filter which files to include in the search (e.g., 'src/**/*.py'). Defaults to all relevant source files."
    )
    parser.add_argument(
        "--exclude",
        type=str,
        default=None,
        help="A glob pattern to filter which files to exclude from the search (e.g., '*.log')."
    )
    parser.add_argument(
        "--regex",
        action="store_true",
        help="If present, treats the --pattern argument as a regular expression."
    )
    parser.add_argument(
        "--case-sensitive",
        action="store_true",
        help="If present, the search will be case-sensitive."
    )
    parser.add_argument(
        "--in-place",
        action="store_true",
        help="If present, and --replace is also specified, the replacement operation will modify the files directly."
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="If present, and --replace is also specified, shows a preview of changes without modifying files."
    )
    parser.add_argument(
        "--output",
        type=str,
        default=None,
        help="Specifies a file path where the search results should be written. If omitted, results are printed to standard output."
    )

    args = parser.parse_args()

    # Handle mutual exclusivity and read from files
    if args.pattern and args.pattern_file:
        print("Error: Cannot specify both --pattern and --pattern-file.", file=sys.stderr)
        sys.exit(1)
    if not args.pattern and not args.pattern_file:
        print("Error: One of --pattern or --pattern-file must be specified.", file=sys.stderr)
        sys.exit(1)
    if args.pattern_file:
        try:
            with open(args.pattern_file, 'r', encoding='utf-8') as f:
                args.pattern = f.read()
        except Exception as e:
            print(f"Error reading pattern file {args.pattern_file}: {e}", file=sys.stderr)
            sys.exit(1)

    if args.replace and args.replace_file:
        print("Error: Cannot specify both --replace and --replace-file.", file=sys.stderr)
        sys.exit(1)
    if args.replace_file:
        try:
            with open(args.replace_file, 'r', encoding='utf-8') as f:
                args.replace = f.read()
        except Exception as e:
            print(f"Error reading replace file {args.replace_file}: {e}", file=sys.stderr)
            sys.exit(1)

    if args.in_place and not args.replace:
        print("Error: --in-place requires --replace or --replace-file to be specified.", file=sys.stderr)
        sys.exit(1)

    if args.dry_run and not args.replace:
        print("Error: --dry-run requires --replace or --replace-file to be specified.", file=sys.stderr)
        sys.exit(1)

    if args.in_place and args.dry_run:
        print("Error: Cannot use --in-place and --dry-run together.", file=sys.stderr)
        sys.exit(1)

    # Determine files to search
    files_to_search = []
    if args.files:
        files_to_search = find_files(args.files)
    else:
        files_to_search = find_files("**/*")
        files_to_search = [f for f in files_to_search if not (
            f.endswith(('.pyc', '.log', '.tmp', '.bak')) or
            os.path.isdir(f) or
            f.startswith('.') # Ignore dot files/directories
        )]

    # Filter out files based on the --exclude pattern
    if args.exclude:
        excluded_files = {f for f in files_to_search if fnmatch.fnmatch(f, args.exclude)}
        files_to_search = [f for f in files_to_search if f not in excluded_files]

    all_results = [] # Stores filepaths where a match was found
    files_to_modify = {}

    for filepath in files_to_search:
        found_match, original_content, modified_content = search_and_replace_in_file(
            filepath, args.pattern, args.replace, args.regex, args.case_sensitive
        )

        if found_match:
            all_results.append(filepath)

            if modified_content is not None: # Means replacement occurred and content changed
                files_to_modify[filepath] = (original_content, modified_content)

    # Handle output and replacement based on arguments
    if args.replace:
        if args.dry_run:
            print("--- Dry Run: Proposed Changes ---")
            if not files_to_modify:
                print("No replacements would be made.")
            for filepath, (original, modified) in files_to_modify.items():
                print(f"\n--- {filepath} (Original) ---")
                print(original.strip())
                print(f"\n--- {filepath} (Proposed) ---")
                print(modified.strip())

        elif args.in_place:
            print("--- Performing In-Place Replacements ---")
            if not files_to_modify:
                print("No replacements were made.")
            for filepath, (original, modified) in files_to_modify.items():
                try:
                    with open(filepath, 'w', encoding='utf-8') as f:
                        f.write(modified)
                    print(f"Modified: {filepath}")
                except Exception as e:
                    print(f"Error writing to {filepath}: {e}", file=sys.stderr)

        else: # If --replace is used without --dry-run or --in-place, just show files with matches
            if all_results:
                print("Files with matches:")
                for res in all_results:
                    print(res)
            else:
                print("No matches found.")

    else: # Only search, no replacement
        if args.output:
            try:
                with open(args.output, 'w', encoding='utf-8') as outfile:
                    for res in all_results:
                        outfile.write(res + '\n')
                print(f"Search results written to {args.output}")
            except Exception as e:
                print(f"Error writing to output file {args.output}: {e}", file=sys.stderr)
                sys.exit(1)
        else:
            if all_results:
                print("Files with matches:")
                for res in all_results:
                    print(res)
            else:
                print("No matches found.")

if __name__ == "__main__":
    main()
