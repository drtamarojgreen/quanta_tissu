import os
import re
import sys

def check_file(filepath):
    errors = []
    with open(filepath, 'r') as f:
        content = f.read()

        # 1. No Empty Catch Blocks
        if re.search(r'catch\s*\([^)]*\)\s*\{\s*\}', content):
            errors.append("Empty catch block found.")

        # 2. No Placeholders (TODO, FIXME)
        if re.search(r'TODO|FIXME', content):
            errors.append("Placeholder (TODO/FIXME) found.")

        # 3. Basic abstraction check (e.g., too many classes)
        # For our vanilla JS project, we prefer objects over classes
        # Use a more specific regex for JS class definitions to avoid matching HTML class attributes
        class_count = len(re.findall(r'\bclass\s+[A-Z][a-zA-Z0-9]*\s*\{', content))
        if class_count > 5: # Arbitrary limit for 'minimal abstraction'
            errors.append(f"Too many classes ({class_count}) found. Favor modular objects.")

    return errors

def main():
    target_dirs = [
        'web_platform/frontend/js/api',
        'web_platform/frontend/js/modules'
    ]

    total_errors = 0
    for d in target_dirs:
        for root, _, files in os.walk(d):
            for file in files:
                if file.endswith('.js'):
                    path = os.path.join(root, file)
                    file_errors = check_file(path)
                    if file_errors:
                        print(f"Errors in {path}:")
                        for err in file_errors:
                            print(f"  - {err}")
                        total_errors += len(file_errors)

    if total_errors > 0:
        print(f"\nSDD Validation FAILED with {total_errors} errors.")
        sys.exit(1)
    else:
        print("SDD Validation PASSED.")
        sys.exit(0)

if __name__ == "__main__":
    main()
