#include "CircularBuffer.hpp"

template <typename T>
CircularBuffer<T>::CircularBuffer(size_t size) 
    : size_(size), buffer_(new T[size]), head_(0), tail_(0), full_(false) {}

template <typename T>
CircularBuffer<T>::~CircularBuffer()
{
    delete[] buffer_;
}

template <typename T>
int CircularBuffer<T>::push(const T* data, size_t length) {
    if (length > size_) {
        return -1; // Data length exceeds buffer size
    }

    // Check if the buffer is full before pushing
    bool was_full = full_;

    // Copy data into the buffer
    for (size_t i = 0; i < length; ++i) {
        buffer_[head_] = data[i];
        head_ = (head_ + 1) % size_;

        // If we are about to overwrite the tail, move the tail forward
        if (head_ == tail_) {
            tail_ = (tail_ + 1) % size_; // Move tail to overwrite the oldest data
            full_ = true; // Set full_ to true since we are overwriting
        }
    }

    // Return 1 if data was overridden, otherwise return 0
    return was_full ? 1 : 0;
}

template <typename T>
size_t CircularBuffer<T>::pop(T* output, size_t length)
{
    size_t available_elements = element_count();
    size_t elements_to_pop = (length > available_elements) ? available_elements : length;

    for (size_t i = 0; i < elements_to_pop; ++i) {
        output[i] = buffer_[tail_]; // Copy the oldest data to output
        tail_ = (tail_ + 1) % size_; // Move tail forward
    }

    full_ = false; // After popping, the buffer cannot be full
    return elements_to_pop; // Return the number of elements popped
}

template <typename T>
size_t CircularBuffer<T>::element_count() const
{
    if (full_) {
        return size_; // Buffer is full
    }
    return (head_ >= tail_) ? (head_ - tail_) : (size_ + head_ - tail_);
}

template <typename T>
size_t CircularBuffer<T>::capacity() const
{
    return size_;
}

template <typename T>
bool CircularBuffer<T>::is_full() const
{
    return full_;
}

template <typename T>
bool CircularBuffer<T>::is_empty() const
{
    return (!full_ && (head_ == tail_));
}

template <typename T>
void CircularBuffer<T>::clear()
{
    head_ = tail_;
    full_ = false;
}

// Explicit instantiation for commonly used types (optional)
template class CircularBuffer<uint32_t>; 
template class CircularBuffer<uint8_t>;