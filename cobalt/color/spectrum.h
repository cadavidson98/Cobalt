#ifndef COBALT_COLOR_SPECTRUM_H
#define COBALT_COLOR_SPECTRUM_H

namespace cobalt::color {

enum class SpectrumType {
    kPolynomial,
    kBlackbody,
    kSampled
};

}  // namespace cobalt::color

#endif  // COBALT_COLOR_SPECTRUM_H