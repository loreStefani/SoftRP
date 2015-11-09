#ifndef SOFTRP_MATRIX_FUN_INL_
#define SOFTRP_MATRIX_FUN_INL
#include "Matrix.h"
namespace SoftRP {
	namespace Math {

		template<typename T, unsigned int N>
		inline Matrix<T, N> operator+(const Matrix<T, N>& v1, const Matrix<T, N>& v2) {
			Matrix<T, N> value{ v1 };
			value += v2;
			return value;
		}

		template<typename T, unsigned int N>
		inline Matrix<T, N> operator-(const Matrix<T, N>& v1, const Matrix<T, N>& v2) {
			Matrix<T, N> value{ v1 };
			value -= v2;
			return value;
		}

		template<typename T, unsigned int N>
		inline Matrix<T, N> operator*(const Matrix<T, N>& v1, const Matrix<T, N>& v2) {
			Matrix<T, N> value{ v1 };
			value *= v2;
			return value;
		}

		template<unsigned int N, typename T>
		inline Matrix<T, N>* MatrixFromPtr(T* data) {
			return reinterpret_cast<Matrix<T, N>*>(data);
		}

		template<unsigned int N, typename T>
		inline const Matrix<T, N>* MatrixFromPtr(const T* data) {
			return reinterpret_cast<const Matrix<T, N>*>(data);
		}
	}
}
#endif
