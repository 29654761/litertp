
/**
 * @file ring_buffer.hpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#pragma once
#include <cstddef>
#include <stdexcept>
#include <cstring>


namespace sys{

template <typename T>
class ring_buffer {
private:
    T* buffer_;
    size_t head_ = 0;
    size_t tail_ = 0;
    const size_t capacity_;
public:
    ring_buffer(size_t capacity)
        :capacity_(capacity)
    {
        buffer_ = new T[capacity]();
    }

    ~ring_buffer()
    {
        delete[] buffer_;
    }

    bool write(const T* data, size_t length)
    {
        size_t available = available_to_write();
        if (length > available) {
            return false;
        }

        if (head_>tail_)
        {
            std::memcpy(buffer_ + tail_, data, length);
        }
        else
        {
            size_t right_length = capacity_ - tail_;
            if (length <= right_length) {
                std::memcpy(buffer_ + tail_, data, length * sizeof(T));
            }
            else {
                std::memcpy(buffer_ + tail_, data, right_length * sizeof(T));
                std::memcpy(buffer_, data + right_length, (length - right_length) * sizeof(T));
            }
        }


        tail_ += length;
        tail_ %= capacity_;
        return true;
    }


    bool read(T* data, size_t length)
    {
        size_t available = available_to_read();
        if (length > available) {
            return false;
        }

        if (tail_<head_) {
            size_t right_length = capacity_ - head_;
            if (length <= right_length) {
                std::memcpy(data, buffer_ + head_, length * sizeof(T));
            }
            else {
                std::memcpy(data, buffer_ + head_, right_length * sizeof(T));
                std::memcpy(data + right_length, buffer_, (length - right_length) * sizeof(T));
            }
        }
        else {
            std::memcpy(data, buffer_ + head_, length * sizeof(T));
        }
        head_ += length;
        head_ %= capacity_;
        return true;
    }


    size_t available_to_read() const
    {
        if (head_ > tail_) {
            return capacity_ - head_ + tail_;
        }
        else {
            return tail_ - head_;
        }
    }

    size_t available_to_write() const
    {
        if (head_ > tail_) {
            return head_ - tail_;
        }
        else {
            return capacity_ - tail_ + head_;
        }
    }


};

}
