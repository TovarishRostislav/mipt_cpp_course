#pragma once

#include <cstddef>
#include <array>

namespace nano_edr {

template <typename T, size_t N>
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
        const RingBuffer* buffer_ = nullptr;
        std::size_t index_ = 0;
    };

    RingBuffer() = default;

    void PushBack(const T& value) {
        data_[(begin_ + size_) % N] = value;
        if (full()) {
            begin_ = (begin_ + 1) % N;
        } else {
            ++size_;
        }
    }

    void Clear() {
        for (std::size_t i = 0; i < size_; ++i) {
            T empty{};
            std::swap(data_[i], empty);
        }

        begin_ = 0;
        size_ = 0;
    }

    size_t size() const { return size_; }
    static constexpr size_t capacity() { return N; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == N; }

    const T& operator[](size_t index) const {
        return data_[(begin_ + index) % N];
    }

    const T& back() const {
        return (*this)[size_ - 1];
    }

    Iterator begin() const { 
        return Iterator(this, 0); 
    }
    
    Iterator end() const { 
        return Iterator(this, size_); 
    }

private:
    std::array<T, N> data_; 
    size_t begin_ = 0;     
    size_t size_ = 0;  
};

} 