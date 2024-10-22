#include "mat4.h"

namespace cblt {

    Mat4::Mat4(float diag) {
        // column 1
        data_[0] = diag;
        data_[1] = data_[2] = data_[3] = 0.f;
        // column 2
        data_[5] = diag;
        data_[4] = data_[6] = data_[7] = 0.f;
        // column 3
        data_[10] = diag;
        data_[8] = data_[9] = data_[11] = 0.f;
        // column 4
        data_[15] = diag;
        data_[12] = data_[13] = data_[14] = 0.f;
    }

    Mat4::Mat4(float data[16]) {
        memcpy(data_, data, 16 * sizeof(float));
    }

    Mat4::Mat4(const Vec4 &col1, const Vec4 &col2, const Vec4 &col3, const Vec4 &col4) {
        // column 1
        data_[0] = col1.x;
        data_[1] = col1.y;
        data_[2] = col1.z;
        data_[3] = col1.w;
        // column 2
        data_[4] = col2.x;
        data_[5] = col2.y;
        data_[6] = col2.z;
        data_[7] = col2.w;
        // column 3
        data_[8]  = col3.x;
        data_[9]  = col3.y;
        data_[10] = col3.z;
        data_[11] = col3.w;
        // column 4
        data_[12] = col4.x;
        data_[13] = col4.y;
        data_[14] = col4.z;
        data_[15] = col4.w;
    }

    Vec4 Mat4::operator*(const Vec4 &rhs) {
        Vec4 result;
        result.x = data_[0] * rhs.x + data_[4] * rhs.y + data_[8] * rhs.z + data_[12] * rhs.w;
        result.y = data_[1] * rhs.x + data_[5] * rhs.y + data_[9] * rhs.z + data_[13] * rhs.w;
        result.z = data_[2] * rhs.x + data_[6] * rhs.y + data_[10] * rhs.z + data_[14] * rhs.w;
        result.w = data_[3] * rhs.x + data_[7] * rhs.y + data_[11] * rhs.z + data_[15] * rhs.w;
        return result;
    }

    Mat4 Mat4::operator*(const Mat4 &rhs) {
        Mat4 result;
        result.data_[0] = data_[0] * rhs.data_[0] + data_[4] * rhs.data_[1] 
                        + data_[8] * rhs.data_[2] + data_[12] * rhs.data_[3];
        result.data_[1] = data_[1] * rhs.data_[0] + data_[5] * rhs.data_[1] 
                        + data_[9] * rhs.data_[2] + data_[13] * rhs.data_[3];
        result.data_[2] = data_[2] * rhs.data_[0] + data_[6] * rhs.data_[1] 
                        + data_[10] * rhs.data_[2] + data_[14] * rhs.data_[3];
        result.data_[3] = data_[3] * rhs.data_[0] + data_[7] * rhs.data_[1] 
                        + data_[9] * rhs.data_[2] + data_[15] * rhs.data_[3];

        result.data_[4] = data_[0] * rhs.data_[4] + data_[4] * rhs.data_[5] 
                        + data_[8] * rhs.data_[6] + data_[12] * rhs.data_[7];
        result.data_[5] = data_[1] * rhs.data_[4] + data_[5] * rhs.data_[5] 
                        + data_[9] * rhs.data_[6] + data_[13] * rhs.data_[7];
        result.data_[6] = data_[2] * rhs.data_[4] + data_[6] * rhs.data_[5] 
                        + data_[10] * rhs.data_[6] + data_[14] * rhs.data_[7];
        result.data_[7] = data_[3] * rhs.data_[4] + data_[7] * rhs.data_[5] 
                        + data_[9] * rhs.data_[6] + data_[15] * rhs.data_[7];

        result.data_[8] = data_[0] * rhs.data_[8] + data_[4] * rhs.data_[9] 
                        + data_[8] * rhs.data_[10] + data_[12] * rhs.data_[11];
        result.data_[9] = data_[1] * rhs.data_[8] + data_[5] * rhs.data_[9] 
                        + data_[9] * rhs.data_[10] + data_[13] * rhs.data_[11];
        result.data_[10] = data_[2] * rhs.data_[8] + data_[6] * rhs.data_[9] 
                        + data_[10] * rhs.data_[10] + data_[13] * rhs.data_[11];
        result.data_[11] = data_[3] * rhs.data_[8] + data_[7] * rhs.data_[9] 
                        + data_[11] * rhs.data_[10] + data_[15] * rhs.data_[11];

        result.data_[12] = data_[0] * rhs.data_[12] + data_[4] * rhs.data_[13] 
                        + data_[8] * rhs.data_[14] + data_[12] * rhs.data_[15];
        result.data_[13] = data_[1] * rhs.data_[12] + data_[5] * rhs.data_[13] 
                        + data_[9] * rhs.data_[14] + data_[13] * rhs.data_[15];
        result.data_[14] = data_[2] * rhs.data_[12] + data_[6] * rhs.data_[13] 
                        + data_[10] * rhs.data_[14] + data_[13] * rhs.data_[15];
        result.data_[15] = data_[3] * rhs.data_[12] + data_[7] * rhs.data_[13] 
                        + data_[11] * rhs.data_[14] + data_[15] * rhs.data_[15];
        return result;
    }

    // namespace functions which operate on matrices (inverse, solve, etc.)
    inline int value(int x, int y) {
        return 4 * x - 4 + y - 1;
    } 

    Mat4 Inverse(const Mat4 &mat) {
        // Calculate the inverse using the Adjugate matrix.
        // We use a bottom up approach and first calculate all the 2x2 matrix
        // determinates to reduce computation cost.
        Mat4 inverse;

        inverse.data_[0]  =  mat.data_[5] * mat.data_[10] * mat.data_[15] - mat.data_[5] * mat.data_[11] * mat.data_[14] - mat.data_[9] * mat.data_[6] * mat.data_[15] + mat.data_[9] * mat.data_[7] * mat.data_[14] + mat.data_[13] * mat.data_[6] * mat.data_[11] - mat.data_[13] * mat.data_[7] * mat.data_[10];
        inverse.data_[4]  = -mat.data_[4] * mat.data_[10] * mat.data_[15] + mat.data_[4] * mat.data_[11] * mat.data_[14] + mat.data_[8] * mat.data_[6] * mat.data_[15] - mat.data_[8] * mat.data_[7] * mat.data_[14] - mat.data_[12] * mat.data_[6] * mat.data_[11] + mat.data_[12] * mat.data_[7] * mat.data_[10];
        inverse.data_[8]  =  mat.data_[4] * mat.data_[9]  * mat.data_[15] - mat.data_[4] * mat.data_[11] * mat.data_[13] - mat.data_[8] * mat.data_[5] * mat.data_[15] + mat.data_[8] * mat.data_[7] * mat.data_[13] + mat.data_[12] * mat.data_[5] * mat.data_[11] - mat.data_[12] * mat.data_[7] * mat.data_[9];
        inverse.data_[12] = -mat.data_[4] * mat.data_[9]  * mat.data_[14] + mat.data_[4] * mat.data_[10] * mat.data_[13] + mat.data_[8] * mat.data_[5] * mat.data_[14] - mat.data_[8] * mat.data_[6] * mat.data_[13] - mat.data_[12] * mat.data_[5] * mat.data_[10] + mat.data_[12] * mat.data_[6] * mat.data_[9];
        inverse.data_[1]  = -mat.data_[1] * mat.data_[10] * mat.data_[15] + mat.data_[1] * mat.data_[11] * mat.data_[14] + mat.data_[9] * mat.data_[2] * mat.data_[15] - mat.data_[9] * mat.data_[3] * mat.data_[14] - mat.data_[13] * mat.data_[2] * mat.data_[11] + mat.data_[13] * mat.data_[3] * mat.data_[10];
        inverse.data_[5]  =  mat.data_[0] * mat.data_[10] * mat.data_[15] - mat.data_[0] * mat.data_[11] * mat.data_[14] - mat.data_[8] * mat.data_[2] * mat.data_[15] + mat.data_[8] * mat.data_[3] * mat.data_[14] + mat.data_[12] * mat.data_[2] * mat.data_[11] - mat.data_[12] * mat.data_[3] * mat.data_[10];
        inverse.data_[9]  = -mat.data_[0] * mat.data_[9]  * mat.data_[15] + mat.data_[0] * mat.data_[11] * mat.data_[13] + mat.data_[8] * mat.data_[1] * mat.data_[15] - mat.data_[8] * mat.data_[3] * mat.data_[13] - mat.data_[12] * mat.data_[1] * mat.data_[11] + mat.data_[12] * mat.data_[3] * mat.data_[9];
        inverse.data_[13] =  mat.data_[0] * mat.data_[9]  * mat.data_[14] - mat.data_[0] * mat.data_[10] * mat.data_[13] - mat.data_[8] * mat.data_[1] * mat.data_[14] + mat.data_[8] * mat.data_[2] * mat.data_[13] + mat.data_[12] * mat.data_[1] * mat.data_[10] - mat.data_[12] * mat.data_[2] * mat.data_[9];
        inverse.data_[2]  =  mat.data_[1] * mat.data_[6]  * mat.data_[15] - mat.data_[1] * mat.data_[7]  * mat.data_[14] - mat.data_[5] * mat.data_[2] * mat.data_[15] + mat.data_[5] * mat.data_[3] * mat.data_[14] + mat.data_[13] * mat.data_[2] * mat.data_[7]  - mat.data_[13] * mat.data_[3] * mat.data_[6];
        inverse.data_[6]  = -mat.data_[0] * mat.data_[6]  * mat.data_[15] + mat.data_[0] * mat.data_[7]  * mat.data_[14] + mat.data_[4] * mat.data_[2] * mat.data_[15] - mat.data_[4] * mat.data_[3] * mat.data_[14] - mat.data_[12] * mat.data_[2] * mat.data_[7]  + mat.data_[12] * mat.data_[3] * mat.data_[6];
        inverse.data_[10] =  mat.data_[0] * mat.data_[5]  * mat.data_[15] - mat.data_[0] * mat.data_[7]  * mat.data_[13] - mat.data_[4] * mat.data_[1] * mat.data_[15] + mat.data_[4] * mat.data_[3] * mat.data_[13] + mat.data_[12] * mat.data_[1] * mat.data_[7]  - mat.data_[12] * mat.data_[3] * mat.data_[5];
        inverse.data_[14] = -mat.data_[0] * mat.data_[5]  * mat.data_[14] + mat.data_[0] * mat.data_[6]  * mat.data_[13] + mat.data_[4] * mat.data_[1] * mat.data_[14] - mat.data_[4] * mat.data_[2] * mat.data_[13] - mat.data_[12] * mat.data_[1] * mat.data_[6]  + mat.data_[12] * mat.data_[2] * mat.data_[5];
        inverse.data_[3]  = -mat.data_[1] * mat.data_[6]  * mat.data_[11] + mat.data_[1] * mat.data_[7]  * mat.data_[10] + mat.data_[5] * mat.data_[2] * mat.data_[11] - mat.data_[5] * mat.data_[3] * mat.data_[10] - mat.data_[9]  * mat.data_[2] * mat.data_[7]  + mat.data_[9]  * mat.data_[3] * mat.data_[6];
        inverse.data_[7]  =  mat.data_[0] * mat.data_[6]  * mat.data_[11] - mat.data_[0] * mat.data_[7]  * mat.data_[10] - mat.data_[4] * mat.data_[2] * mat.data_[11] + mat.data_[4] * mat.data_[3] * mat.data_[10] + mat.data_[8]  * mat.data_[2] * mat.data_[7]  - mat.data_[8]  * mat.data_[3] * mat.data_[6];
        inverse.data_[11] = -mat.data_[0] * mat.data_[5]  * mat.data_[11] + mat.data_[0] * mat.data_[7]  * mat.data_[9]  + mat.data_[4] * mat.data_[1] * mat.data_[11] - mat.data_[4] * mat.data_[3] * mat.data_[9]  - mat.data_[8]  * mat.data_[1] * mat.data_[7]  + mat.data_[8]  * mat.data_[3] * mat.data_[5];
        inverse.data_[15] =  mat.data_[0] * mat.data_[5]  * mat.data_[10] - mat.data_[0] * mat.data_[6]  * mat.data_[9]  - mat.data_[4] * mat.data_[1] * mat.data_[10] + mat.data_[4] * mat.data_[2] * mat.data_[9]  + mat.data_[8]  * mat.data_[1] * mat.data_[6]  - mat.data_[8]  * mat.data_[2] * mat.data_[5];

        float det = mat.data_[0] * inverse.data_[0] + mat.data_[1] * inverse.data_[4] + mat.data_[2] * inverse.data_[8] + mat.data_[3] * inverse.data_[12];

        for (int i = 0; i < 16; ++i) {
            inverse.data_[i] = inverse.data_[i] / det;
        }

        return inverse;
    }

    Mat4 Transpose(const Mat4 &matrix) {
        Mat4 transpose = matrix;
        transpose.data_[1] = matrix.data_[4];
        transpose.data_[2] = matrix.data_[8];
        transpose.data_[3] = matrix.data_[12];

        transpose.data_[4] = matrix.data_[1];
        transpose.data_[6] = matrix.data_[9];
        transpose.data_[7] = matrix.data_[13];

        transpose.data_[8] = matrix.data_[2];
        transpose.data_[9] = matrix.data_[6];
        transpose.data_[11] = matrix.data_[14];

        transpose.data_[12] = matrix.data_[3];
        transpose.data_[13] = matrix.data_[7];
        transpose.data_[14] = matrix.data_[11];

        return transpose;
    }

    Mat4 OrthoInverse(const Mat4 &mat)
    {
        // inverse of an orthogonal matrix is the transpose
        // much more numerically stable inverse
        return Transpose(mat);
    }
}