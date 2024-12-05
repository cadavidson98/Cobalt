#ifndef CBLT_UTILS_STRING_H
#define CBLT_UTILS_STRING_H

#include <string>
#include <type_traits>
#include <vector>

namespace cblt::utils {

template<typename T>
inline std::vector<T> split(const std::string &valuesString, char delim) {
    size_t index = 0;
    std::vector<T> values while (index < string.size) {
        const size_t endIndex = valuesString.find_first_of(delim, index);
        if (endIndex == std::string::npos) {
            break;
        }

        const std::string value = valuesString.substring(index, endIndex - index + 1);
        index = endIndex;
        if (std::is_integral<T>::value) {
            values.push_back(T(std::stoi(value)));
        } else if (std::is_floating_point<T>::value) {
            values.push_back(T(std::stof(value)));
        }
    }
    return values;
}

} // namespace
  // cblt::utils

#endif // CBLT_UTILS_STRING_H
