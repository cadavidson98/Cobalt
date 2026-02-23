#ifndef COBALT_CORE_STRING_UTILITIES_H
#define COBALT_CORE_STRING_UTILITIES_H

#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace cobalt::core {

template<typename T = std::string>
inline std::vector<T> split(const std::string_view valuesString, char delim) {
    size_t index = 0;
    std::vector<T> values;
    while (index < valuesString.length()) {
        const size_t endIndex = valuesString.find_first_of(delim, index);
        const std::string value(valuesString, index, endIndex - index);

        index = (endIndex == std::string_view::npos) ? valuesString.length() : endIndex + 1;

        if (value.length() == 0) {
            continue;
        }

        if constexpr (std::is_integral<T>::value) {
            values.push_back(T(std::stoi(value)));
        } else if constexpr (std::is_floating_point<T>::value) {
            values.push_back(T(std::stof(value)));
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

inline std::string appendFileToPath(const std::string_view path, const std::string_view fileName) {
    return std::string(path) + '/' + std::string(fileName);
}

} // namespace cobalt::core

#endif // COBALT_CORE_STRING_UTILITIES_H
