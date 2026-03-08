#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "utils.h"

class Stack {
    
public:
    static constexpr std::size_t DEFAULT_CAPACITY = 512;

private:
    std::size_t sp = 0;
    int64_t* data = nullptr;
    std::size_t max_capacity = DEFAULT_CAPACITY;

public:
    explicit Stack() {
        data = new int64_t[max_capacity];
    }

    explicit Stack(const std::size_t size) {
        max_capacity = size;
        data = new int64_t[size];
    }

    ~Stack() {
        delete[] data;
    }

    void push(const int64_t val) {
        if (sp == max_capacity) {
            throw std::overflow_error("Stack overflow");
        }

        data[sp++] = val;
    }

    template <typename T>
    void push_as(const T val) {
        push(utils::convert<int64_t>(val));
    }

    void push_ptr(const void* ptr) {
        const auto val = static_cast<int64_t>(reinterpret_cast<uintptr_t>(ptr));
        push(val);
    }

    void dup() {
        if (sp == max_capacity) {
            throw std::overflow_error("Stack overflow");
        }

        const int64_t val = data[sp - 1];
        data[sp++] = val;
    }

    int64_t pop() {
        if (sp == 0) {
            throw std::underflow_error("Stack underflow");
        }

        return data[--sp];
    }

    template <typename T>
    T pop_as() {
        return utils::convert<T>(pop());
    }

    void* pop_ptr() {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(pop()));
    }

    [[nodiscard]] int64_t peek() const {
        if (sp == 0) {
            throw std::underflow_error("Stack underflow");
        }

        return data[sp - 1];
    }

    template <typename T>
    [[nodiscard]] T peek_as() {
        return utils::convert<T>(peek());
    }

    [[nodiscard]] void* peek_ptr() {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(peek()));
    }

    [[nodiscard]] std::size_t get_SP() const {
        return sp;
    }
};
