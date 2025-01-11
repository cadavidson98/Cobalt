#ifndef CBLT_CORE_DYNAMIC_ARRAY_H
#define CBLT_CORE_DYNAMIC_ARRAY_H

namespace cblt {

template<typename T>
class DynamicArray {
    public:
    DynamicArray(size_t size=0ul);

    DynamicArray(DynamicArray&&);
    DynamicArray &operator=(DynamicArray&&);

    ~DynamicArray();

    DynamicArray(DynamicArray&) = delete;
    DynamicArray &operator=(DynamicArray&) = delete;
    DynamicArray operator=(DynamicArray) = delete;

    explicit operator bool() const;

    size_type size();
    size_type capacity();

    private:
    T *_array;

    size_type _size;
    size_type _maxSize;
};

}  // namespace cblt

#endif  // CBLT_CORE_DYNAMIC_ARRAY_H