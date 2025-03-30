#pragma once

#include <cstring> // For memcpy
#include <cstdint>
#include <cstddef>

template <typename T>
class CircularBuffer {
public:
    CircularBuffer(size_t size);
    ~CircularBuffer();

    int push(const T* data, size_t length);
    size_t pop(T* output, size_t length);
    size_t element_count() const;
    size_t capacity() const;
    bool is_full() const;
    bool is_empty() const;
    void clear();

private:
    size_t size_;
    T* buffer_;
    size_t head_;
    size_t tail_;
    bool full_;
};