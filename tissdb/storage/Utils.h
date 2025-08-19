//
// Created by 刘育氚 on 2019/5/31.
//

#ifndef BPTREE_UTILS_H
#define BPTREE_UTILS_H

#include <cstdio>
#include <cerrno>
#include <string>
#include <memory>

static const int TEST_ENDIAN_VAL = 1;
static const bool IS_MACHINE_LITTLE_ENDIAN = *((char *) &TEST_ENDIAN_VAL) == 1;

// all file io is in little-endian
namespace bp_tree_utils {

    inline FILE *fopen(const char *path, const char *mode) {
        FILE *fptr = std::fopen(path, mode);
        if (!fptr) {
            throw std::string("cannot open file: ") + path;
        }
        return fptr;
    }

    inline void changeEndian(void *ptr, size_t size, size_t nitems) {
        auto valPtr = (unsigned char *) ptr;
        for (size_t i = 0; i < nitems; ++i) {
            for (size_t j = 0, k = size - 1; j < k; ++j, --k) {
                unsigned char b = valPtr[i * size + j];
                valPtr[i * size + j] = valPtr[i * size + k];
                valPtr[i * size + k] = b;
            }
        }
    }


    inline void fwrite(const void *ptr, size_t size, size_t nitems,
                       FILE *stream) {
        if (std::fwrite(ptr, size, nitems, stream) != nitems) {
            throw std::string("Write to file failed");
        }
    }

    inline void fread(void *ptr, size_t size, size_t nitems,
                      FILE *stream) {
        if (std::fread(ptr, size, nitems, stream) != nitems) {
            throw std::string("Read from file failed");
        }
    }


    template<typename T>
    void writeVal(const T &val, FILE *stream) {
        bp_tree_utils::fwrite(&val, sizeof(T), 1, stream);
    }

    template<typename T>
    void writeValLittle(const T &val, FILE *stream) {
        if (IS_MACHINE_LITTLE_ENDIAN) {
            bp_tree_utils::fwrite(&val, sizeof(T), 1, stream);
        } else {
            unsigned char ptr[sizeof(T)];
            memcpy(ptr, &val, sizeof(T));
            changeEndian(ptr, sizeof(T), 1);
            bp_tree_utils::fwrite(&ptr, 1, sizeof(T), stream);
        }
    }

    template<typename T>
    void writeArrayLittle(const T *arr, size_t length, FILE *stream) {
        if (IS_MACHINE_LITTLE_ENDIAN) {
            bp_tree_utils::fwrite(arr, sizeof(T), length, stream);
        } else {
            std::unique_ptr<unsigned char[]> ptr(new unsigned char[sizeof(T) * length]);
            memcpy(ptr.get(), arr, sizeof(T) * length);
            changeEndian(ptr.get(), sizeof(T), length);
            bp_tree_utils::fwrite(ptr.get(), 1, sizeof(T) * length, stream);
        }
    }

    template<typename T>
    T readValLittle(FILE *stream) {
        T val;
        bp_tree_utils::fread(&val, sizeof(T), 1, stream);
        if (!IS_MACHINE_LITTLE_ENDIAN) {
            changeEndian(&val, sizeof(T), 1);
        }
        return val;
    }

    // Template specialization for std::string
    template<>
    inline void writeValLittle<std::string>(const std::string &val, FILE *stream) {
        size_t len = val.length();
        writeValLittle(len, stream);
        bp_tree_utils::fwrite(val.c_str(), 1, len, stream);
    }

    template<>
    inline std::string readValLittle<std::string>(FILE *stream) {
        size_t len = readValLittle<size_t>(stream);
        if (len > 0) {
            std::vector<char> buf(len);
            bp_tree_utils::fread(buf.data(), 1, len, stream);
            return std::string(buf.data(), len);
        }
        return "";
    }

    template<typename T>
    T readVal(FILE *stream) {
        T val;
        bp_tree_utils::fread(&val, sizeof(T), 1, stream);
        return val;
    }

    template<typename ... Args>
    std::string stringFormat(const std::string &format, Args ... args) {
        // https://stackoverflow.com/questions/35035982/is-snprintfnull-0-behavi
        // get size of result string
        int size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), static_cast<size_t>(size), format.c_str(), args ...);
        return std::string(buf.get(), buf.get() + size - 1);
    }
}

#endif //BPTREE_UTILS_H
