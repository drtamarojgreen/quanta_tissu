cmake_minimum_required(VERSION 3.5)

project(TissLangIDE LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt5
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# Automatically handle Qt's meta-object compiler (moc), user interface compiler (uic),
# and resource compiler (rcc)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Define the executable and its source files
add_executable(TissLangIDE
    main.cpp
    MainWindow.cpp
    TissEditor.cpp
    TissSyntaxHighlighter.cpp
    SearchDialog.cpp
    MainWindow.h
    TissEditor.h
    TissSyntaxHighlighter.h
    SearchDialog.h
)

# Link to the Qt libraries
target_link_libraries(TissLangIDE PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
