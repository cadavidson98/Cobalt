#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include "vec.h"
#include <vector>

namespace cblt {

    class Mat4 {
        public:
        // Constructors assume matrix is column major
        Mat4(float diag = 1.0f);
        Mat4(float data[16]);
        Mat4(const Vec4 &col1, const Vec4 &col2, const Vec4 &col3, const Vec4 &col4);

        Vec4 operator *(const Vec4 &rhs);
        Mat4 operator *(const Mat4 &rhs);
        
        friend Mat4 Inverse(const Mat4 &matrix);
        friend Mat4 OrthoInverse(const Mat4 &matrix);
        friend Mat4 Transpose(const Mat4 &matrix);
        
        private:
        // Column Major
        float data_[16];
    };

    // namespace functions that operate on matrices
    Mat4 Inverse(const Mat4 &matrix);
    Mat4 OrthoInverse(const Mat4 &matrix);
    Mat4 Transpose(const Mat4 &matrix);
}
#endif  // MATH_MATRIX_H