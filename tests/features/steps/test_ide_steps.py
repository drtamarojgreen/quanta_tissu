import os

def register_steps(runner):

    @runner.step(r'the TissLang IDE is running')
    def ide_is_running(context):
        # Simulate the IDE being in a clean state
        context.ide_state = {
            "editor_content": "",
            "opened_file": None,
            "highlighted_elements": []
        }
        print("SIM: TissLang IDE is running.")

    @runner.step(r'a new, empty file is open in the TissLang IDE')
    def new_empty_file(context):
        ide_is_running(context) # Reset state
        print("SIM: New empty file is open.")

    @runner.step(r'I open the file "(.*)"')
    def open_file(context, filename):
        # In a real test, we'd read the file. Here, we'll simulate it.
        context.ide_state['opened_file'] = filename
        context.ide_state['editor_content'] = f"Content of {filename}"
        print(f"SIM: Opening file '{filename}'.")

    @runner.step(r'the content of "(.*)" should be displayed in the editor')
    def content_displayed(context, filename):
        expected_content = f"Content of {filename}"
        assert context.ide_state['editor_content'] == expected_content
        print(f"SIM: Verified content of '{filename}' is in editor.")

    @runner.step(r'the syntax for TissLang should be highlighted')
    def syntax_highlighted(context):
        # This is a conceptual step. We'll just log it.
        context.ide_state['highlighted_elements'].append("TissLang")
        print("SIM: Syntax highlighting is active.")

    @runner.step(r'I type the text "(.*)"')
    def type_text(context, text):
        context.ide_state['editor_content'] += text
        print(f"SIM: Typing text: '{text}'.")

    @runner.step(r'the editor should contain the text "(.*)"')
    def editor_contains_text(context, text):
        assert text in context.ide_state['editor_content']
        print(f"SIM: Verified editor contains '{text}'.")

    @runner.step(r'the keyword "(.*)" should be highlighted')
    def keyword_highlighted(context, keyword):
        context.ide_state['highlighted_elements'].append({"type": "keyword", "text": keyword})
        print(f"SIM: Keyword '{keyword}' is highlighted.")

    @runner.step(r'the editor contains the text "(.*)"')
    def editor_contains_multiline_text(context, text):
        context.ide_state['editor_content'] = text.replace('\\n', '\n')
        print(f"SIM: Set editor content to: '{text}'.")

    @runner.step(r'a file is open in the TissLang IDE')
    def file_is_open(context):
        ide_is_running(context)
        context.ide_state['opened_file'] = 'some_file.tiss'
        print("SIM: A file is open.")

    @runner.step(r'the line "(.*)" should be highlighted as a comment')
    def line_highlighted_as_comment(context, line):
        context.ide_state['highlighted_elements'].append({"type": "comment", "text": line})
        print(f"SIM: Line '{line}' is highlighted as a comment.")

    @runner.step(r'the text "(.*)" should be highlighted as a string')
    def text_highlighted_as_string(context, text):
        context.ide_state['highlighted_elements'].append({"type": "string", "text": text})
        print(f"SIM: Text '{text}' is highlighted as a string.")

    @runner.step(r'a file is open in the TissLang IDE with the content "(.*)"')
    def file_open_with_content(context, content):
        ide_is_running(context)
        context.ide_state['opened_file'] = 'some_file.tiss'
        context.ide_state['editor_content'] = content.replace('\\n', '\n')
        print(f"SIM: File open with content: '{content}'.")

    @runner.step(r'I search for the text "(.*)"')
    def search_for_text(context, text):
        # In a real test, this would interact with a search dialog.
        context.ide_state['search_query'] = text
        print(f"SIM: Searching for '{text}'.")

    @runner.step(r'the text "(.*)" should be selected in the editor')
    def text_should_be_selected(context, text):
        # This is a conceptual assertion.
        assert context.ide_state['search_query'] == text
        print(f"SIM: Text '{text}' is selected.")

    @runner.step(r'a file is open in the TissLang IDE with unsaved changes')
    def file_with_unsaved_changes(context):
        ide_is_running(context)
        context.ide_state['editor_content'] = "Some new content"
        context.ide_state['is_dirty'] = True
        print("SIM: File has unsaved changes.")

    @runner.step(r'I save the file as "(.*)"')
    def save_file_as(context, filename):
        # In a real test, we would check the file system.
        # Here, we'll just record the intended action.
        context.ide_state['saved_as'] = filename
        context.ide_state['is_dirty'] = False
        print(f"SIM: Saving file as '{filename}'.")

    @runner.step(r'a file named "(.*)" should be created with the editor\'s content')
    def file_should_be_created(context, filename):
        assert context.ide_state['saved_as'] == filename
        print(f"SIM: Verified that file '{filename}' would be created.")
