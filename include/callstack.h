#pragma once
#include <cstddef>
#include <stdexcept>

class CallStack {
  public:
    static constexpr std::size_t DEFAULT_CAPACITY = 64;

  private:
    std::size_t  index        = 0;
    std::size_t* data         = nullptr;
    std::size_t  max_capacity = DEFAULT_CAPACITY;

  public:
    explicit CallStack() {
        data = new std::size_t[max_capacity];
    }

    explicit CallStack(const std::size_t size) {
        max_capacity = size;
        data         = new std::size_t[size];
    }

    ~CallStack() {
        delete[] data;
    }

    CallStack(const CallStack&) = delete;

    CallStack& operator=(const CallStack&) = delete;

    void push(const std::size_t val) {
        if (index == max_capacity) {
            throw std::overflow_error("Stack overflow");
        }

        data[index++] = val;
    }

    std::size_t pop() {
        if (index == 0) {
            throw std::underflow_error("Stack underflow");
        }

        return data[--index];
    }

    [[nodiscard]] std::size_t peek() const {
        if (index == 0) {
            throw std::underflow_error("Stack underflow");
        }

        return data[index - 1];
    }

    [[nodiscard]] std::size_t get_index() const {
        return index;
    }
};
