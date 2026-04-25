#ifndef RMA_KEYBOARD_INPUT_HPP
#define RMA_KEYBOARD_INPUT_HPP

#include <atomic>
#include <functional>
#include <thread>

#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/select.h>
#endif

namespace rma {

class KeyboardInput {
public:
    using KeyCallback = std::function<void(char)>;

    // Non-copyable, non-movable (has thread)
    KeyboardInput(const KeyboardInput&) = delete;
    KeyboardInput& operator=(const KeyboardInput&) = delete;
    KeyboardInput(KeyboardInput&&) = delete;
    KeyboardInput& operator=(KeyboardInput&&) = delete;

    KeyboardInput() : running_(false) {}

    ~KeyboardInput() {
        stop();
    }

    void start(KeyCallback callback) {
        if (running_.exchange(true)) return; // already running

        callback_ = std::move(callback);
        thread_ = std::thread(&KeyboardInput::input_loop, this);
    }

    void stop() {
        running_.store(false);
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    bool is_running() const {
        return running_.load();
    }

private:
    void input_loop() {
#ifndef _WIN32
        // Set terminal to raw mode
        struct termios old_term, new_term;
        tcgetattr(STDIN_FILENO, &old_term);
        new_term = old_term;
        new_term.c_lflag &= ~(ICANON | ECHO);
        new_term.c_cc[VMIN] = 0;
        new_term.c_cc[VTIME] = 1; // 100ms timeout
        tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
#endif

        while (running_.load()) {
            char ch = 0;

#ifdef _WIN32
            if (_kbhit()) {
                ch = static_cast<char>(_getch());
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }
#else
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(STDIN_FILENO, &fds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 100000; // 100ms

            int ret = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
            if (ret > 0 && FD_ISSET(STDIN_FILENO, &fds)) {
                if (read(STDIN_FILENO, &ch, 1) != 1) continue;
            } else {
                continue;
            }
#endif

            if (ch != 0 && callback_) {
                callback_(ch);
            }
        }

#ifndef _WIN32
        // Restore terminal
        tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
#endif
    }

    std::atomic<bool> running_;
    std::thread thread_;
    KeyCallback callback_;
};

} // namespace rma

#endif // RMA_KEYBOARD_INPUT_HPP
