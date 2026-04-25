#ifndef RMA_RPC_PROTOCOL_HPP
#define RMA_RPC_PROTOCOL_HPP

#include <cstdint>
#include <cstring>
#include <atomic>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

namespace rma {

// Shared memory layout for IPC
struct SharedMemoryHeader {
    std::atomic<uint32_t> write_pos;
    std::atomic<uint32_t> read_pos;
    std::atomic<uint32_t> message_count;
    std::atomic<uint32_t> host_pid;
    uint32_t buffer_size;
    uint32_t reserved[3];
    // Data follows header
};

static constexpr size_t SHM_SIZE = 1024 * 1024; // 1MB buffer
static constexpr size_t SHM_HEADER_SIZE = sizeof(SharedMemoryHeader);
static constexpr size_t SHM_DATA_SIZE = SHM_SIZE - SHM_HEADER_SIZE;
static constexpr const char* SHM_NAME_PREFIX = "/rma_shm_";

// Message header in ring buffer
struct MessageHeader {
    uint32_t length;
    uint32_t type_id;
    uint32_t checksum; // simple validation
};

class RpcProtocol {
public:
    // No copy - owns system resources
    RpcProtocol(const RpcProtocol&) = delete;
    RpcProtocol& operator=(const RpcProtocol&) = delete;

    // Move allowed
    RpcProtocol(RpcProtocol&& other) noexcept
        : shm_ptr_(other.shm_ptr_), shm_size_(other.shm_size_),
          is_host_(other.is_host_), initialized_(other.initialized_) {
#ifdef _WIN32
        shm_handle_ = other.shm_handle_;
        other.shm_handle_ = nullptr;
#else
        shm_fd_ = other.shm_fd_;
        other.shm_fd_ = -1;
#endif
        std::strncpy(shm_name_, other.shm_name_, sizeof(shm_name_));
        other.shm_ptr_ = nullptr;
        other.initialized_ = false;
    }

    RpcProtocol& operator=(RpcProtocol&& other) noexcept {
        if (this != &other) {
            cleanup();
            shm_ptr_ = other.shm_ptr_;
            shm_size_ = other.shm_size_;
            is_host_ = other.is_host_;
            initialized_ = other.initialized_;
#ifdef _WIN32
            shm_handle_ = other.shm_handle_;
            other.shm_handle_ = nullptr;
#else
            shm_fd_ = other.shm_fd_;
            other.shm_fd_ = -1;
#endif
            std::strncpy(shm_name_, other.shm_name_, sizeof(shm_name_));
            other.shm_ptr_ = nullptr;
            other.initialized_ = false;
        }
        return *this;
    }

    RpcProtocol() : shm_ptr_(nullptr), shm_size_(SHM_SIZE),
                    is_host_(false), initialized_(false) {
#ifdef _WIN32
        shm_handle_ = nullptr;
#else
        shm_fd_ = -1;
#endif
        shm_name_[0] = '\0';
    }

    ~RpcProtocol() {
        cleanup();
    }

    // Initialize as host (creates shared memory)
    bool init_host(uint32_t session_id = 0) noexcept {
        if (initialized_) return false;
        is_host_ = true;

        std::snprintf(shm_name_, sizeof(shm_name_), "%s%u",
                      SHM_NAME_PREFIX, session_id);

#ifdef _WIN32
        shm_handle_ = CreateFileMappingA(
            INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
            0, static_cast<DWORD>(shm_size_), shm_name_ + 1); // skip leading /

        if (!shm_handle_) return false;

        shm_ptr_ = MapViewOfFile(shm_handle_, FILE_MAP_ALL_ACCESS,
                                  0, 0, shm_size_);
        if (!shm_ptr_) {
            CloseHandle(shm_handle_);
            shm_handle_ = nullptr;
            return false;
        }
#else
        shm_fd_ = shm_open(shm_name_, O_CREAT | O_RDWR, 0666);
        if (shm_fd_ < 0) return false;

        if (ftruncate(shm_fd_, static_cast<off_t>(shm_size_)) < 0) {
            close(shm_fd_);
            shm_unlink(shm_name_);
            shm_fd_ = -1;
            return false;
        }

        shm_ptr_ = mmap(nullptr, shm_size_, PROT_READ | PROT_WRITE,
                        MAP_SHARED, shm_fd_, 0);
        if (shm_ptr_ == MAP_FAILED) {
            close(shm_fd_);
            shm_unlink(shm_name_);
            shm_fd_ = -1;
            shm_ptr_ = nullptr;
            return false;
        }
#endif

        // Initialize header
        auto* header = get_header();
        header->write_pos.store(0, std::memory_order_relaxed);
        header->read_pos.store(0, std::memory_order_relaxed);
        header->message_count.store(0, std::memory_order_relaxed);
        header->host_pid.store(
#ifdef _WIN32
            GetCurrentProcessId(),
#else
            static_cast<uint32_t>(getpid()),
#endif
            std::memory_order_relaxed);
        header->buffer_size = static_cast<uint32_t>(SHM_DATA_SIZE);

        initialized_ = true;
        return true;
    }

    // Initialize as analyzer (attaches to existing shared memory)
    bool init_analyzer(uint32_t session_id = 0) noexcept {
        if (initialized_) return false;
        is_host_ = false;

        std::snprintf(shm_name_, sizeof(shm_name_), "%s%u",
                      SHM_NAME_PREFIX, session_id);

#ifdef _WIN32
        shm_handle_ = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE,
                                        shm_name_ + 1);
        if (!shm_handle_) return false;

        shm_ptr_ = MapViewOfFile(shm_handle_, FILE_MAP_ALL_ACCESS,
                                  0, 0, shm_size_);
        if (!shm_ptr_) {
            CloseHandle(shm_handle_);
            shm_handle_ = nullptr;
            return false;
        }
#else
        shm_fd_ = shm_open(shm_name_, O_RDWR, 0666);
        if (shm_fd_ < 0) return false;

        shm_ptr_ = mmap(nullptr, shm_size_, PROT_READ | PROT_WRITE,
                        MAP_SHARED, shm_fd_, 0);
        if (shm_ptr_ == MAP_FAILED) {
            close(shm_fd_);
            shm_fd_ = -1;
            shm_ptr_ = nullptr;
            return false;
        }
#endif

        initialized_ = true;
        return true;
    }

    // Send data from host to analyzer
    bool send(const uint8_t* data, size_t len, uint32_t type_id = 0) noexcept {
        if (!initialized_ || !data || len == 0) return false;
        if (len > SHM_DATA_SIZE / 2) return false; // sanity limit

        auto* header = get_header();
        uint8_t* buffer = get_data_buffer();

        size_t total_len = sizeof(MessageHeader) + len;
        uint32_t write_pos = header->write_pos.load(std::memory_order_acquire);
        uint32_t read_pos = header->read_pos.load(std::memory_order_acquire);

        // Check space (simplified - doesn't handle wrap perfectly)
        uint32_t available = (read_pos <= write_pos)
            ? (header->buffer_size - write_pos + read_pos)
            : (read_pos - write_pos);

        if (available < total_len + 64) return false; // need margin

        // Write message header
        MessageHeader msg_hdr;
        msg_hdr.length = static_cast<uint32_t>(len);
        msg_hdr.type_id = type_id;
        msg_hdr.checksum = compute_checksum(data, len);

        uint32_t pos = write_pos;
        write_wrapped(buffer, header->buffer_size, pos, &msg_hdr, sizeof(msg_hdr));
        pos = (pos + sizeof(msg_hdr)) % header->buffer_size;

        write_wrapped(buffer, header->buffer_size, pos, data, len);
        pos = (pos + static_cast<uint32_t>(len)) % header->buffer_size;

        header->write_pos.store(pos, std::memory_order_release);
        header->message_count.fetch_add(1, std::memory_order_relaxed);

        return true;
    }

    // Receive data in analyzer (returns bytes read, 0 if no data)
    size_t receive(uint8_t* buffer, size_t max_len, uint32_t* out_type_id = nullptr) noexcept {
        if (!initialized_ || !buffer || max_len == 0) return 0;

        auto* header = get_header();
        uint8_t* shm_buffer = get_data_buffer();

        uint32_t write_pos = header->write_pos.load(std::memory_order_acquire);
        uint32_t read_pos = header->read_pos.load(std::memory_order_acquire);

        if (read_pos == write_pos) return 0; // empty

        // Read message header
        MessageHeader msg_hdr;
        read_wrapped(shm_buffer, header->buffer_size, read_pos,
                     &msg_hdr, sizeof(msg_hdr));

        if (msg_hdr.length > max_len || msg_hdr.length > SHM_DATA_SIZE / 2) {
            // Corrupted or too large - skip
            header->read_pos.store(write_pos, std::memory_order_release);
            return 0;
        }

        uint32_t data_pos = (read_pos + sizeof(msg_hdr)) % header->buffer_size;
        read_wrapped(shm_buffer, header->buffer_size, data_pos,
                     buffer, msg_hdr.length);

        // Validate checksum
        if (compute_checksum(buffer, msg_hdr.length) != msg_hdr.checksum) {
            // Corrupted - skip message
            uint32_t new_pos = (data_pos + msg_hdr.length) % header->buffer_size;
            header->read_pos.store(new_pos, std::memory_order_release);
            return 0;
        }

        if (out_type_id) *out_type_id = msg_hdr.type_id;

        uint32_t new_pos = (data_pos + msg_hdr.length) % header->buffer_size;
        header->read_pos.store(new_pos, std::memory_order_release);

        return msg_hdr.length;
    }

    bool is_initialized() const noexcept { return initialized_; }

    uint32_t pending_messages() const noexcept {
        if (!initialized_) return 0;
        return get_header()->message_count.load(std::memory_order_relaxed);
    }

private:
    void* shm_ptr_;
    size_t shm_size_;
    bool is_host_;
    bool initialized_;
    char shm_name_[64];

#ifdef _WIN32
    HANDLE shm_handle_;
#else
    int shm_fd_;
#endif

    SharedMemoryHeader* get_header() const noexcept {
        return static_cast<SharedMemoryHeader*>(shm_ptr_);
    }

    uint8_t* get_data_buffer() const noexcept {
        return static_cast<uint8_t*>(shm_ptr_) + SHM_HEADER_SIZE;
    }

    static uint32_t compute_checksum(const uint8_t* data, size_t len) noexcept {
        uint32_t sum = 0;
        for (size_t i = 0; i < len; ++i) {
            sum = sum * 31 + data[i];
        }
        return sum;
    }

    static void write_wrapped(uint8_t* buf, uint32_t buf_size,
                              uint32_t pos, const void* data, size_t len) noexcept {
        const uint8_t* src = static_cast<const uint8_t*>(data);
        uint32_t first_part = buf_size - pos;
        if (first_part >= len) {
            std::memcpy(buf + pos, src, len);
        } else {
            std::memcpy(buf + pos, src, first_part);
            std::memcpy(buf, src + first_part, len - first_part);
        }
    }

    static void read_wrapped(const uint8_t* buf, uint32_t buf_size,
                             uint32_t pos, void* data, size_t len) noexcept {
        uint8_t* dst = static_cast<uint8_t*>(data);
        uint32_t first_part = buf_size - pos;
        if (first_part >= len) {
            std::memcpy(dst, buf + pos, len);
        } else {
            std::memcpy(dst, buf + pos, first_part);
            std::memcpy(dst + first_part, buf, len - first_part);
        }
    }

    void cleanup() noexcept {
        if (!initialized_) return;

#ifdef _WIN32
        if (shm_ptr_) UnmapViewOfFile(shm_ptr_);
        if (shm_handle_) CloseHandle(shm_handle_);
        shm_handle_ = nullptr;
#else
        if (shm_ptr_ && shm_ptr_ != MAP_FAILED) {
            munmap(shm_ptr_, shm_size_);
        }
        if (shm_fd_ >= 0) {
            close(shm_fd_);
            if (is_host_) shm_unlink(shm_name_);
        }
        shm_fd_ = -1;
#endif
        shm_ptr_ = nullptr;
        initialized_ = false;
    }
};

} // namespace rma

#endif // RMA_RPC_PROTOCOL_HPP
