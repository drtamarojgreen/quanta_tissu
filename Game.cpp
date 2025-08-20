#include <iostream>

// A minimal Game class to contain the user's requested code.
class Game {
public:
    Game();
    void printHelp();
};

// Game constructor, which calls printHelp() on initialization.
Game::Game() {
    printHelp();
}

// The printHelp() function, refactored to use a raw string literal for cleaner code.
void Game::printHelp() {
    std::cout << R"---(
--- Game Commands ---
  - [direction]: Type 'north', 'south', 'east', or 'west' to move.
  - 'look':         Look around the room again.
  - 'dance':        Do a little dance.
  - 'help':         Show this list of commands.
  - 'quit':         Exit the game.
---------------------

)---";
}
