#pragma once

#include <cstddef>
#include <array>

namespace nano_edr {

template <typename T, std::size_t N>
class RingBuffer {
public:
    class Iterator {
    public:
        Iterator(const RingBuffer* buffer, std::size_t index)
            : buffer_(buffer), index_(index) {}

        const T& operator*() const {
            return (*buffer_)[index_];
        }

        Iterator& operator++() {
            ++index_;
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index_ != other.index_;
        }

    private:
        const RingBuffer* buffer_;
        std::size_t index_;
    };

    RingBuffer() = default;

    void PushBack(const T& value) {
        data_[end_] = value;
        end_ = (end_ + 1) % N;
        
        if (size_ < N) {
            ++size_;
        } else {
            begin_ = (begin_ + 1) % N;
        }
    }

    void Clear() {
        begin_ = 0;
        end_ = 0;
        size_ = 0;
    }

    std::size_t size() const { return size_; }
    static constexpr std::size_t capacity() { return N; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == N; }

    const T& operator[](std::size_t index) const {
        return data_[(begin_ + index) % N];
    }

    const T& back() const {
        std::size_t last_index = (end_ + N - 1) % N;
        return data_[last_index];
    }

    Iterator begin() const { 
        return Iterator(this, 0); 
    }
    
    Iterator end() const { 
        return Iterator(this, size_); 
    }

private:
    std::array<T, N> data_{}; 
    std::size_t begin_ = 0;    
    std::size_t end_ = 0;    
    std::size_t size_ = 0;  
};

} 