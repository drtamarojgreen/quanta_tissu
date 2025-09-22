# TissLM TUI Editor: Technical Design Document

## 1. Introduction & Objective

This document provides a comprehensive technical design for the TissLang Terminal User Interface (TUI) Editor. The primary objective is to create a robust, non-compilable C++ source code blueprint for a "full fledged" TUI editor. This editor serves as a replacement for the previous Qt-based conceptual IDE and is designed to be easily adapted by a developer for compilation with a TUI library such as `ncurses`.

The blueprint is architecturally sound, well-commented, and provides a solid foundation for a functional text editor.

## 2. System Architecture

The TUI editor follows a standard component-based architecture to ensure a clean separation of concerns. This design enhances modularity, testability, and maintainability. The three primary components are the `Editor`, the `Screen`, and the `Buffer`.

- **`Editor`**: The central orchestrator. It manages the application's main loop, processes user input, and coordinates interactions between the `Buffer` (data model) and the `Screen` (view).
- **`Screen`**: An abstraction layer for the terminal display. It is responsible for all rendering operations, such as drawing text and status bars. It acts as the "view" in a model-view-controller paradigm.
- **`Buffer`**: The data model. It manages the text content of the file being edited, including all modifications, cursor positioning, and file I/O.

This separation ensures that the core editing logic (`Buffer`) is independent of the display logic (`Screen`), and both are managed by the main `Editor` class.

## 3. Component Deep Dive

### 3.1. The `Editor` Component

The `Editor` class is the heart of the application.

- **Source Files**: `Editor.h`, `Editor.cpp`

#### Core Responsibilities:
- **Main Event Loop**: The `run()` method contains the main `while` loop that keeps the application running. On each iteration, it refreshes the screen and waits for user input.

  *Illustrative Snippet (`Editor::run`):*
  ```cpp
  void Editor::run() {
      while (!should_close) {
          refreshScreen();
          int key = getMockKeypress();
          processKeypress(key);
      }
  }
  ```

- **Input Processing**: The `processKeypress()` method uses a `switch` statement to delegate actions based on user input. It handles quitting, cursor movement, and character insertion.

  *Illustrative Snippet (`Editor::processKeypress`):*
  ```cpp
  void Editor::processKeypress(int key) {
      // ...
      switch (key) {
          case 'q':
              should_close = true;
              break;
          case 'w':
              buffer.moveCursor(Buffer::Direction::UP);
              break;
          // ... other cases
          default:
              if (isprint(key)) {
                  buffer.insertChar(key);
              }
              break;
      }
  }
  ```

### 3.2. The `Screen` Component

The `Screen` class isolates all terminal-specific rendering logic.

- **Source Files**: `Screen.h`, `Screen.cpp`

#### Core Responsibilities:
- **Rendering the View**: The `refreshScreen()` method in the `Editor` class calls a sequence of `Screen` methods to draw the UI. This typically involves clearing the screen, drawing the buffer's content, drawing a status bar, and then positioning the cursor correctly.

  *Illustrative Snippet (`Editor::refreshScreen`):*
  ```cpp
  void Editor::refreshScreen() {
      screen.clear();
      screen.drawBuffer(buffer);
      screen.drawStatusBar(status_message);
      screen.setCursor(buffer.getCursorX(), buffer.getCursorY());
      screen.refresh();
  }
  ```

- **Abstraction**: In the blueprint, `Screen` methods write to `std::cout`. A developer would replace these mock implementations with calls to a library like `ncurses` (e.g., `clear()`, `mvprintw()`, `move()`, `refresh()`).

### 3.3. The `Buffer` Component

The `Buffer` class manages the document's state.

- **Source Files**: `Buffer.h`, `Buffer.cpp`

#### Core Responsibilities:
- **Data Storage**: In this blueprint, the text is stored in a `std::vector<std::string>`. This provides a simple, line-based representation of the file. For a production-ready editor, this would be replaced with a more efficient data structure like a gap buffer or rope to make insertions and deletions faster.
- **Cursor Logic**: The `moveCursor()` method contains the logic for moving the cursor up, down, left, or right, including handling boundary conditions (e.g., not moving past the beginning or end of the file) and ensuring the cursor stays within the bounds of shorter lines.

  *Illustrative Snippet (`Buffer::moveCursor`):*
  ```cpp
  void Buffer::moveCursor(Direction dir) {
      switch (dir) {
          // ... cases for UP, DOWN, LEFT, RIGHT
      }
      // Correct cursor X if the new line is shorter
      cursor_x = std::min(cursor_x, (int)lines[cursor_y].length());
  }
  ```
- **Text Modification**: Methods like `insertChar()`, `deleteChar()`, and `newLine()` contain the core logic for modifying the text buffer.

## 4. Development and Functional Path

To convert this blueprint into a functional application, a developer should follow these steps:
1.  **Integrate a TUI Library**: Add `ncurses` or a similar library to the build system.
2.  **Implement `Screen`**: Replace the mock `std::cout` calls in `Screen.cpp` with the corresponding `ncurses` functions.
3.  **Implement Input**: Replace `getMockKeypress()` with `getch()` to capture real user input.
4.  **Enhance `Buffer`**: For better performance, refactor the `Buffer` to use a more appropriate data structure.
5.  **Expand Keybindings**: Add cases for all standard editor keys (Backspace, Enter, Tab, Home, End, etc.) to the `processKeypress` method.

## 5. Native Code Enhancements

This section details 100 potential enhancements for the TUI editor that can be developed using only standard C++ and native OS capabilities, without requiring additional third-party libraries or frameworks beyond a standard TUI library like `ncurses`.

### Core Editing Features (1-25)

1.  **Word-wise Cursor Movement (Ctrl+Left/Right)**
    *   **Challenge**: Defining word boundaries (e.g., whitespace, punctuation).
    *   **Mitigation**: Implement a helper function `findNextWordBoundary(line, position)` that iterates from the current cursor position to find the start of the next/previous word based on a simple character type check (alphanumeric vs. non-alphanumeric).
2.  **Jump to Start/End of Line (Home/End)**
    *   **Challenge**: Minimal.
    *   **Mitigation**: Set the cursor's `x` position to 0 for Home or `lines[y].length()` for End.
3.  **Delete Word Forward (Ctrl+Delete)**
    *   **Challenge**: Combining word boundary logic with text deletion.
    *   **Mitigation**: Use `findNextWordBoundary()` to determine the range of characters to delete from the string in the buffer.
4.  **Delete Word Backward (Ctrl+Backspace)**
    *   **Challenge**: Similar to deleting forward.
    *   **Mitigation**: Use `findNextWordBoundary()` in the reverse direction to determine the deletion range.
5.  **Text Selection (Shift + Movement)**
    *   **Challenge**: Managing the selection state (start and end points) and rendering it correctly.
    *   **Mitigation**: Add `selection_anchor_x` and `selection_anchor_y` to the `Buffer`. When Shift is held, `moveCursor` updates the cursor position but not the anchor. The `Screen` class will need to render the selected text with a different attribute (e.g., inverted color).
6.  **Copy Selection to Internal Clipboard**
    *   **Challenge**: Storing potentially multi-line text.
    *   **Mitigation**: Add a `std::vector<std::string> clipboard` to the `Editor` class. Implement a `getSelectedText()` method in the `Buffer` to return the selected lines.
7.  **Cut Selection**
    *   **Challenge**: Combining copy and delete operations atomically.
    *   **Mitigation**: Implement as a `copy()` followed by a `deleteSelection()` method in the `Buffer`.
8.  **Paste from Internal Clipboard**
    *   **Challenge**: Inserting multi-line text and correctly updating the buffer and cursor position.
    *   **Mitigation**: Implement a `paste()` method in the `Buffer` that iterates through the clipboard lines, inserting them and splitting the current line if necessary.
9.  **Line Duplication (Ctrl+D)**
    *   **Challenge**: Minimal.
    *   **Mitigation**: Get the current line string, and insert a copy of it into the `lines` vector at the next position.
10. **Line Deletion (Ctrl+L)**
    *   **Challenge**: Minimal, but must handle deleting the last line in the file.
    *   **Mitigation**: `lines.erase()` at the cursor's `y` position. If the buffer becomes empty, insert a single empty string to maintain a valid state.
11. **Move Line Up/Down (Alt+Up/Down)**
    *   **Challenge**: Swapping a line with its neighbor and moving the cursor with it.
    *   **Mitigation**: Use `std::swap` on the `lines` vector and decrement/increment the cursor's `y` position.
12. **Undo/Redo Stack**
    *   **Challenge**: Storing and managing a history of editor states or actions. This is one of the most complex "native" features.
    *   **Mitigation**: Implement the Command pattern. Each text-modifying action (e.g., `InsertCharCommand`, `DeleteLineCommand`) is an object with `execute()` and `undo()` methods. Push each executed command onto an undo stack.
13. **Auto-indentation on New Line**
    *   **Challenge**: Determining the correct indentation level from the previous line.
    *   **Mitigation**: When Enter is pressed, get the leading whitespace from the current line and insert it at the beginning of the new line.
14. **Smart Indent/Unindent for Selection (Tab/Shift+Tab)**
    *   **Challenge**: Applying indentation to a block of selected lines.
    *   **Mitigation**: When Tab is pressed with a multi-line selection, iterate through the selected lines and insert an indent (e.g., 4 spaces) at the beginning of each. Shift+Tab would remove an indent.
15. **Bracket/Parenthesis/Brace Auto-completion**
    *   **Challenge**: Inserting the closing character automatically.
    *   **Mitigation**: In `processKeypress`, if the user types `(`, `[`, or `{`, insert the pair `()` and move the cursor back one position between them.
16. **Bracket/Parenthesis/Brace Matching Highlight**
    *   **Challenge**: Finding the matching bracket, which could be far away or nested.
    *   **Mitigation**: When the cursor is on a bracket, implement a function to scan forward (for an opening bracket) or backward (for a closing bracket), keeping a counter to handle nesting. Highlight both found positions.
17. **Toggle Comment for Line/Selection (Ctrl+/)**
    *   **Challenge**: Determining the correct comment character (`//`, `#`, etc.) and adding/removing it.
    *   **Mitigation**: Assume a comment character (e.g., `//`). For each selected line, check if it starts with the comment string. If so, remove it. If not, add it.
18. **Block Indentation (Ctrl+]) and Unindentation (Ctrl+[)**
    *   **Challenge**: Similar to Tab/Shift+Tab on a selection.
    *   **Mitigation**: Identical implementation to enhancement #14, just tied to different keybindings.
19. **Convert Selection to Uppercase/Lowercase**
    *   **Challenge**: Modifying text within a selection in-place.
    *   **Mitigation**: Get the selected text, convert it to the desired case using `std::toupper`/`std::tolower`, and then replace the selection with the new text.
20. **Join Lines (Ctrl+J)**
    *   **Challenge**: Merging the next line onto the end of the current line.
    *   **Mitigation**: Append the string from `lines[y+1]` to `lines[y]`, delete the `y+1` line, and place the cursor at the join point.
21. **Trim Trailing Whitespace on Save**
    *   **Challenge**: Integrating the action into the file saving process.
    *   **Mitigation**: Before writing the buffer to a file, iterate through each line and remove trailing whitespace using string manipulation methods.
22. **Ensure File Ends with a Newline on Save**
    *   **Challenge**: Checking and potentially modifying the last line of the buffer.
    *   **Mitigation**: Before saving, check if the last character of the last line is a newline. If not, append one.
23. **Page Up/Page Down Movement**
    *   **Challenge**: Moving the cursor by a screen's height.
    *   **Mitigation**: Move the cursor's `y` position by `screen.height`, clamping the value to the buffer boundaries.
24. **Jump to Top/Bottom of File (Ctrl+Home/End)**
    *   **Challenge**: Minimal.
    *   **Mitigation**: Set cursor `y` and `x` to 0 for Ctrl+Home. Set `y` to `lines.size() - 1` for Ctrl+End.
25. **Insert Tab Character vs. Spaces**
    *   **Challenge**: Handling two different types of indentation.
    *   **Mitigation**: Add a configuration setting to the `Editor` (`use_tabs`). When the Tab key is pressed, insert either a `\t` character or a configured number of spaces.

### Search and Replace (26-40)

26. **Incremental Search Mode (Ctrl+F)**
    *   **Challenge**: Creating a search mode that takes over the status bar and updates highlights dynamically.
    *   **Mitigation**: Add an `EditorState` enum (e.g., `NORMAL`, `SEARCH`). In `SEARCH` mode, user input is appended to a search query string displayed in the status bar, and `refreshScreen` highlights matches.
27. **Highlight All Search Matches**
    *   **Challenge**: Finding all occurrences without slowing down the UI and rendering them differently.
    *   **Mitigation**: In `SEARCH` mode, run a search function that populates a `std::vector<Position>` of all matches. The `Screen` class uses this vector to apply a highlight attribute when drawing the buffer.
28. **Cycle Through Search Results (F3 / Shift+F3)**
    *   **Challenge**: Jumping the cursor between matches.
    *   **Mitigation**: Keep an index for the current match within the vector of search results. F3 increments the index and moves the cursor to the next match's position.
29. **Case-Sensitive/Insensitive Search Toggle**
    *   **Challenge**: Performing case-insensitive string searching.
    *   **Mitigation**: Implement a search function that takes a boolean for case sensitivity. If insensitive, convert both the search term and the text being searched to lowercase before comparing.
30. **Whole Word Search Toggle**
    *   **Challenge**: Matching the search term only if it's not part of a larger word.
    *   **Mitigation**: After finding a substring match, check the characters immediately before and after the match to ensure they are word boundaries (e.g., whitespace, punctuation, or start/end of line).
31. **Replace Mode (Ctrl+R)**
    *   **Challenge**: Extending the search UI to accept a replacement string.
    *   **Mitigation**: Add a `REPLACE` state to the `EditorState` enum. The status bar now needs to prompt for the search term, then the replace term.
32. **Replace Current Match**
    *   **Challenge**: Replacing the text for the currently highlighted search result.
    *   **Mitigation**: Use the position of the current search result to perform a string replacement in the buffer.
33. **Replace All Matches**
    *   **Challenge**: Performing replacement safely without messing up the indices of subsequent matches.
    *   **Mitigation**: Iterate through the matches from the *bottom* of the file to the *top*. This ensures that replacements do not affect the character indices of matches that have not yet been processed.
34. **Search/Replace History**
    *   **Challenge**: Storing previous search and replace terms.
    *   **Mitigation**: Use two `std::vector<std::string>` members in the `Editor` to store a history of search and replace terms. Allow cycling through them with the up/down arrows while in search/replace mode.
35. **Go to Line Number (Ctrl+G)**
    *   **Challenge**: Prompting the user for a line number and jumping the cursor.
    *   **Mitigation**: Implement a `GOTO_LINE` editor state. The status bar prompts for a number. On Enter, parse the number and set the cursor's `y` position.
36. **Regex-based Search**
    *   **Challenge**: Integrating C++'s `<regex>` library, which can be complex and slow.
    *   **Mitigation**: Add a toggle for regex search. Use `std::regex` and `std::smatch` to find matches. Be aware that this is a significant step up in complexity from simple string searching.
37. **Persistent Search Highlight**
    *   **Challenge**: Keeping search results highlighted after exiting search mode.
    *   **Mitigation**: Don't clear the search results vector when exiting search mode. Add a command to explicitly clear the highlights.
38. **Search Result Count in Status Bar**
    *   **Challenge**: Minimal.
    *   **Mitigation**: After a search, update the status bar message with ` "Found " + std::to_string(matches.size()) + " results."`
39. **Search in Selection**
    *   **Challenge**: Restricting the search to a specific region of the buffer.
    *   **Mitigation**: The search function needs to accept an optional start and end `Position`. If a selection exists, pass the selection's bounds to the search function.
40. **Context-aware Search (future)**
    *   **Challenge**: Integrating with a parser or linter to search for specific code constructs.
    *   **Mitigation**: This would require a full C++ parser, which is outside the scope of "no additional libraries". A simpler version could use regex to find function definitions or variable assignments.

### UI and UX (41-70)

41. **Display Line Numbers**
    *   **Challenge**: Calculating the width of the line number gutter and offsetting the text rendering.
    *   **Mitigation**: The `Screen` class should calculate the number of digits needed for the last line number (e.g., `log10(line_count) + 1`) and reserve that much space on the left.
42. **Status Bar Content**
    *   **Challenge**: Composing a status bar with multiple pieces of information.
    *   **Mitigation**: Create a `drawStatusBar` function that takes multiple arguments (filename, cursor pos, status message) and formats them into a single string.
43. **File Modification Indicator (`*`)**
    *   **Challenge**: Tracking whether the buffer has been modified since the last save.
    *   **Mitigation**: Add a `bool is_dirty` flag to the `Buffer`. Set it to `true` on any text modification and `false` on save/load.
44. **Read-Only Mode**
    *   **Challenge**: Preventing text modification commands.
    *   **Mitigation**: Add a `bool is_readonly` flag to the `Editor`. In `processKeypress`, ignore any key that would modify the buffer if this flag is set.
45. **Confirm on Quit with Unsaved Changes**
    *   **Challenge**: Intercepting the quit command and prompting the user.
    *   **Mitigation**: When 'q' is pressed, if `buffer.is_dirty` is true, change the status message to "Unsaved changes. Press 'q' again to quit." and require a second 'q' press.
46. **Smooth Scrolling**
    *   **Challenge**: A true smooth scroll is not possible in a standard terminal. This refers to line-by-line scrolling instead of page-by-page.
    *   **Mitigation**: The `Screen`'s rendering logic needs a `top_line` offset. Scrolling up/down just decrements/increments this offset and redraws the screen.
47. **Horizontal Scrolling**
    *   **Challenge**: Handling lines that are wider than the screen.
    *   **Mitigation**: The `Screen` class needs a horizontal offset (`left_col`). When drawing lines, it should use `substr()` to only draw the visible portion of each line.
48. **Customizable Color Schemes**
    *   **Challenge**: Managing color pairs and attributes without hardcoding them.
    *   **Mitigation**: Create a `Theme` struct that holds color definitions for different UI elements (text, status bar, selection). Load this struct from a simple config file at startup.
49. **Show/Hide Whitespace Characters**
    *   **Challenge**: Rendering special symbols for spaces and tabs.
    *   **Mitigation**: In the `Screen::drawBuffer` method, if the feature is enabled, iterate through the string and replace ' ' with a visible character (e.g., '·') and '\t' with '»---'.
50. **Configurable Tab Width**
    *   **Challenge**: Rendering tabs as a variable number of spaces.
    *   **Mitigation**: The `Screen::drawBuffer` method needs to handle this. When it encounters a `\t`, it must calculate the number of spaces needed to reach the next tab stop and draw them instead.
51. **Editor "Modes" in Status Bar (e.g., --INSERT--)**
    *   **Challenge**: Tracking the current mode (e.g., for a Vim-like editor).
    *   **Mitigation**: Add an `EditorMode` enum and display the current mode in the status bar.
52. **Simple File Browser (on open)**
    *   **Challenge**: Reading directory contents and displaying them in a navigable list.
    *   **Mitigation**: This is very difficult without libraries. A simple version could use `popen("ls -p", "r")` to get a directory listing and display it as a temporary buffer for the user to select a file from. This is a bit of a hack.
53. **Show File Size in Status Bar**
    *   **Challenge**: Getting the file size in bytes.
    *   **Mitigation**: On file load, get the total number of characters in the buffer and store it.
54. **Show File Type in Status Bar**
    *   **Challenge**: Determining the file type.
    *   **Mitigation**: Implement a simple function that maps file extensions (`.cpp`, `.h`, `.md`) to file type names.
55. **Config File Support (`.tiss-editor-rc`)**
    *   **Challenge**: Parsing a configuration file at startup.
    *   **Mitigation**: Write a simple line-based parser for key-value pairs (e.g., `tab_width=4`). Load this file from the user's home directory.
56. **Command Palette**
    *   **Challenge**: Creating an interactive prompt to execute editor commands by name.
    *   **Mitigation**: Create a `std::map<std::string, std::function<void()>>` of commands. A `COMMAND` editor state would allow the user to type a command name, with basic autocompletion, and execute the corresponding function.
57. **Split Screen View (Horizontal/Vertical)**
    *   **Challenge**: This is a major architectural change. It requires managing multiple viewports, each with its own offsets and cursor, but potentially sharing the same buffer.
    *   **Mitigation**: Abstract the concept of a "View" or "Window" that owns rendering properties (like offsets) but points to a `Buffer`. The `Editor` would manage a list of active views.
58. **Error/Message Log Panel**
    *   **Challenge**: Reserving a portion of the screen for asynchronous messages.
    *   **Mitigation**: A simple version could be a "message history" buffer that can be toggled open in a split view.
59. **Auto-save on a Timer**
    *   **Challenge**: Running a background task without threads.
    *   **Mitigation**: In the main `run()` loop, check the time since the last save. If it exceeds a threshold and the buffer is dirty, perform a save. This requires non-blocking input, which is a feature of `ncurses`.
60. **Session Saving/Loading**
    *   **Challenge**: Storing the state of open files and cursor positions.
    *   **Mitigation**: On exit, write a session file (e.g., `session.dat`) containing the paths to open files and their cursor positions. On startup, check for this file and load it.
61. **Line Ending Type Detection (LF vs. CRLF)**
    *   **Challenge**: Detecting and preserving line endings.
    *   **Mitigation**: On file load, check for the presence of `\r` characters. Store the detected line ending type in a `Buffer` member variable and use it when saving.
62. **Highlight Current Line**
    *   **Challenge**: Rendering one line with a different background color.
    *   **Mitigation**: In `Screen::drawBuffer`, when drawing the line `cursor_y`, use a different color attribute.
63. **Editor Version Display (`--version`)**
    *   **Challenge**: Minimal.
    *   **Mitigation**: In `main`, check for a `--version` argument. If present, print a hardcoded version string and exit.
64. **Help Screen (`--help` or F1)**
    *   **Challenge**: Displaying a formatted help text.
    *   **Mitigation**: Hardcode a `std::vector<std::string>` with help text and display it in a temporary, read-only buffer.
65. **File Lock Warning**
    *   **Challenge**: Detecting if a file is already open.
    *   **Mitigation**: On file open, create a `.lock` file (e.g., `.myfile.txt.lock`). If the lock file already exists, show a warning. Delete the lock file on clean exit. This is not foolproof.
66. **Go to Matching Brace**
    *   **Challenge**: Same as bracket matching highlight.
    *   **Mitigation**: Reuse the bracket matching logic, but instead of just highlighting, move the cursor to the found position.
67. **Multiple Cursors/Columnar Editing**
    *   **Challenge**: A very advanced feature requiring significant changes to the buffer and input processing.
    *   **Mitigation**: Instead of a single `cursor_x`, `cursor_y`, the `Buffer` would need a `std::vector<Cursor>` of cursor positions. `processKeypress` would need to apply changes for every cursor.
68. **Macro Recording/Playback**
    *   **Challenge**: Storing a sequence of user keypresses and replaying them.
    *   **Mitigation**: Add a `RECORDING` state. While in this state, push every keypress into a `std::vector<int>`. A "playback" command would then iterate through the vector and call `processKeypress` for each key.
69. **Syntax-aware Indentation**
    *   **Challenge**: Requires some level of code parsing.
    *   **Mitigation**: A simple version could check if the previous line ends with `{` or `(` and add an extra level of indentation, or ends with `}` or `)` and removes one.
70. **File Reload Prompt on External Change**
    *   **Challenge**: Detecting if the file on disk has changed since it was opened.
    *   **Mitigation**: Store the file's last modification time on load. Periodically (e.g., in the main loop), check the file's current modification time. If it's newer, prompt the user to reload.

### File Management (71-85)

71. **Create New File**
    *   **Challenge**: Handling a new, empty buffer.
    *   **Mitigation**: Clear the current buffer, reset its state (e.g., `is_dirty = false`), and set its associated filename to empty.
72. **Save As...**
    *   **Challenge**: Prompting the user for a new filename.
    *   **Mitigation**: Add a `SAVE_AS` editor state that uses the status bar to get a new filename from the user before calling the buffer's save logic.
73. **Revert Buffer to Saved State**
    *   **Challenge**: Discarding all changes since the last save.
    *   **Mitigation**: Implement a `revert()` method that simply reloads the file from disk using the existing `loadFromFile()` method.
74. **Basic File System Error Handling**
    *   **Challenge**: Reporting errors like "Permission Denied" or "File Not Found".
    *   **Mitigation**: The `Buffer::loadFromFile` and `saveToFile` methods should return a boolean or error code, which the `Editor` then displays in the status bar.
75. **Support for Basic File Globbing on Open**
    *   **Challenge**: Expanding wildcards like `*.cpp` without a library.
    *   **Mitigation**: This is very difficult. A simple hack could be to use `popen` with a command like `ls -1 *.cpp` and let the shell handle the globbing, then parse the output.
76. **Remember Recent Files**
    *   **Challenge**: Storing a list of recently opened files.
    *   **Mitigation**: Maintain a `std::vector<std::string>` of recent file paths. Save this list to a config file (`.tiss-editor-rc`) on exit and load it on startup.
77. **Open Recent Files Menu**
    *   **Challenge**: Displaying the list of recent files for easy opening.
    *   **Mitigation**: A command could display the recent files list in a temporary buffer, allowing the user to select one to open.
78. **Auto-detection of File Changes**
    *   **Challenge**: Same as #70.
    *   **Mitigation**: Check file modification timestamp in the main loop.
79. **Backup File Creation on Save**
    *   **Challenge**: Creating a copy of the file before overwriting it.
    *   **Mitigation**: Before saving, rename the existing file from `file.txt` to `file.txt~`. Then, write the new content to `file.txt`.
80. **Warn on Opening Very Large Files**
    *   **Challenge**: Determining file size before loading it all into memory.
    *   **Mitigation**: Use `std::ifstream` to seek to the end of the file (`seekg(0, std::ios::end)`) and get its size with `tellg()` before attempting to read it.
81. **Atomic File Saves**
    *   **Challenge**: Ensuring that a crash during a save doesn't corrupt the file.
    *   **Mitigation**: Write the buffer's content to a temporary file (`file.txt.tmp`). If successful, atomically rename the temporary file to the final filename (`rename("file.txt.tmp", "file.txt")`).
82. **Support for Opening Multiple Files from Command Line**
    *   **Challenge**: Managing multiple buffers and a way to switch between them.
    *   **Mitigation**: This requires a major architectural change to support multiple buffers. The `Editor` would manage a `std::vector<Buffer>` and a concept of the "active" buffer.
83. **Display File Permissions in Status Bar**
    *   **Challenge**: Getting file permissions requires OS-specific calls (`stat` on Linux).
    *   **Mitigation**: Use `stat()` to get the file mode and format it into a readable string (e.g., `-rw-r--r--`).
84. **Change File Permissions Command**
    *   **Challenge**: Requires OS-specific calls (`chmod` on Linux).
    *   **Mitigation**: Implement a command that takes octal notation and calls the `chmod()` system call.
85. **Warn on Opening a Binary File**
    *   **Challenge**: Heuristically detecting if a file is binary.
    *   **Mitigation**: Read the first chunk of the file and check for a high percentage of non-printable characters or the presence of a null byte (`\0`).

### Advanced/Miscellaneous (86-100)

86. **Hex Editor Mode**
    *   **Challenge**: Displaying file content as hexadecimal values and allowing editing in that mode.
    *   **Mitigation**: Create a separate `HexScreen` class that renders the buffer content as a hex dump. Input processing would need to handle edits in either the hex or ASCII portion of the view.
87. **Read-only View for Large Files**
    *   **Challenge**: Opening a file without loading it all into memory.
    *   **Mitigation**: The `Buffer` would need to be refactored to be a view into a memory-mapped file (`mmap`) instead of loading the whole file into a vector of strings.
88. **Simple Syntax Highlighting Engine**
    *   **Challenge**: Parsing text and applying colors without a dedicated library.
    *   **Mitigation**: Implement a rule-based engine. Create a `std::vector` of `HighlightRule` structs, where each rule has a regex string and a color attribute. Iterate through these rules to color the text during rendering.
89. **Customizable Highlighting Rules via Config File**
    *   **Challenge**: Parsing syntax highlighting rules from a config file.
    *   **Mitigation**: Define a simple format for highlighting rules in the `.tiss-editor-rc` file (e.g., `highlight_rule:cpp:string = \"[^\"]*\"`).
90. **Calculate and Display Document Statistics**
    *   **Challenge**: Processing the buffer to get word count, line count, character count.
    *   **Mitigation**: Implement a function that iterates through the buffer's lines, counting characters, lines, and words (based on whitespace).
91. **Simple Code Folding (by indentation)**
    *   **Challenge**: Identifying foldable code blocks.
    *   **Mitigation**: A simple approach could treat any block of lines with a greater indent than the line before it as a foldable region.
92. **Execute Current Line as Shell Command**
    *   **Challenge**: Security and capturing output.
    *   **Mitigation**: Use `popen()` to execute the line's content. Capture the output from the pipe and display it in a new buffer or a temporary message. This is highly insecure and should come with strong warnings.
93. **Sort Selected Lines**
    *   **Challenge**: Sorting a portion of the `lines` vector.
    *   **Mitigation**: Get the selected lines into a temporary `std::vector`, use `std::sort` on it, and then replace the original lines in the buffer with the sorted ones.
94. **Filter Lines (Grep-like functionality)**
    *   **Challenge**: Displaying only lines that match a given string or regex.
    *   **Mitigation**: Implement a command that takes a search term and creates a new, temporary read-only buffer containing only the lines from the current buffer that match the term.
95. **Auto-reload on File Change**
    *   **Challenge**: Same as #70, but automatic.
    *   **Mitigation**: Add a config option for auto-reload. If enabled and the file changes on disk, reload it automatically instead of prompting (unless there are unsaved changes).
96. **Simple Templating System**
    *   **Challenge**: Inserting predefined file templates (e.g., for a new C++ class).
    *   **Mitigation**: Store templates as plain text files in a config directory. A "New from Template" command would let the user choose a template and load its content into a new buffer.
97. **Vertical Edge Marker (for line length)**
    *   **Challenge**: Drawing a single vertical line at a configured column.
    *   **Mitigation**: In `Screen::drawBuffer`, after drawing each line of text, if the line is long enough to cross the marker column, draw a special character at that column with a different color attribute.
98. **Split and Join Selections**
    *   **Challenge**: Advanced text manipulation for multi-cursor editing.
    *   **Mitigation**: This builds on the multi-cursor enhancement (#67). It would involve commands to merge multiple selections into one or split a single selection by line.
99. **Smart Home Key (jump to first non-whitespace)**
    *   **Challenge**: Differentiating the first keypress from subsequent ones.
    *   **Mitigation**: When Home is pressed, check if the cursor is already at the first non-whitespace character. If it is, move to column 0. If it's not, move to the first non-whitespace character.
100. **File Encoding Support (UTF-8 detection)**
    *   **Challenge**: Correctly handling multi-byte characters. This is a massive challenge without libraries.
    *   **Mitigation**: A *very* basic implementation could check for a UTF-8 BOM at the beginning of the file. True UTF-8 support for cursor movement and editing would require a complete rewrite of the `Buffer` to be aware of multi-byte character boundaries, likely using a different underlying data structure than `std::string`.

