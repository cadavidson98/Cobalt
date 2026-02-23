#include <span>

namespace cobalt::cli {

bool preprocessCommand([[maybe_unused]] std::span<char *> args) {
    return true;
}

} // namespace cobalt::cli
