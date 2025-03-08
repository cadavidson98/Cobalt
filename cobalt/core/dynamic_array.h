#ifndef CBLT_CORE_DYNAMIC_ARRAY_H
#define CBLT_CORE_DYNAMIC_ARRAY_H

#include "size_types.h"

#include <utility>
#include <variant>

namespace cblt {

template<typename T>
class CoDynamicArray final {
public:
    CoDynamicArray(size_t capacity = 0ul): _size{capacity} {
        if (_size > 0) {
            _array = new Storage[_size];
        }
    }

    CoDynamicArray(std::nullptr_t): _size{0} {
        _array = nullptr;
    }

    CoDynamicArray(CoDynamicArray &&other): _size{other._size}, _array{other._array} {
        other._size = 0;
        other._array = nullptr;
    }

    CoDynamicArray &operator=(CoDynamicArray &&other) {
        _size = std::move(other._size);
        _array = std::move(other._array);

        other._size = 0;
        other._array = nullptr;

        return *this;
    }

    ~CoDynamicArray() {
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

    CoDynamicArray(CoDynamicArray &) = delete;
    CoDynamicArray &operator=(CoDynamicArray &) = delete;
};

} // namespace cblt

#endif // CBLT_CORE_DYNAMIC_ARRAY_H
