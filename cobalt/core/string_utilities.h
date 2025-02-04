#ifndef CBLT_CORE_STRING_UTILITIES_H
#define CBLT_CORE_STRING_UTILITIES_H

#include <string>
#include <type_traits>
#include <vector>

namespace cblt::core {

template<typename T = std::string>
inline std::vector<T> split(const std::string_view valuesString, char delim) {
    size_t index = 0;
    std::vector<T> values;
    while (index < valuesString.length()) {
        const size_t endIndex = valuesString.find_first_of(delim, index);
        const std::string_view value = valuesString.substr(index, endIndex - index);

        index = std::min(valuesString.length(), endIndex) + 1;

        if (value.length() == 0) {
            continue;
        }

        if constexpr (std::is_integral<T>::value) {
            values.push_back(T(std::stoi(value.data())));
        } else if constexpr (std::is_floating_point<T>::value) {
            values.push_back(T(std::stof(value.data())));
        } else {
            static_assert(std::is_convertible_v<std::string, T>);
            values.push_back(T(value));
        }
    }
    return values;
}

inline std::string fileExtension(const std::string_view fileName) {
    const size_t extensionPos = fileName.find_last_of('.');
    if (extensionPos == std::string::npos) {
        return "";
    }

    return std::string(fileName.substr(extensionPos + 1));
}

} // namespace cblt::core

#endif // CBLT_CORE_STRING_UTILITIES_H
