#ifndef COBALT_CORE_DYNAMIC_ARRAY_H
#define COBALT_CORE_DYNAMIC_ARRAY_H

#include "size_types.h"

#include <utility>
#include <variant>

namespace cobalt {

template<typename T>
class DynamicArray final {
public:
    DynamicArray(size_t capacity = 0ul): _size{capacity} {
        if (_size > 0) {
            _array = new Storage[_size];
        }
    }

    DynamicArray(std::nullptr_t): _size{0} {
        _array = nullptr;
    }

    DynamicArray(DynamicArray &&other): _size{other._size}, _array{other._array} {
        other._size = 0;
        other._array = nullptr;
    }

    DynamicArray &operator=(DynamicArray &&other) {
        _size = std::move(other._size);
        _array = std::move(other._array);

        other._size = 0;
        other._array = nullptr;

        return *this;
    }

    ~DynamicArray() {
        delete[] _array;
    }

    explicit operator bool() const {
        return _size != 0;
    }

    T &operator[](size_t idx) {
        return _array[idx].value;
    }

    const T &operator[](size_t idx) const {
        return _array[idx].value;
    }

    T *data() {
        return _array[0].value;
    }

    size_t size() const {
        return _size;
    }

private:
    using Byte = unsigned char;

    union Storage {
        std::monostate monostate = {};
        T value;
        ~Storage() {};
    };

    Storage *_array = nullptr;

    size_t _size = 0;

    DynamicArray(DynamicArray &) = delete;
    DynamicArray &operator=(DynamicArray &) = delete;
};

} // namespace cobalt

#endif // COBALT_CORE_DYNAMIC_ARRAY_H
