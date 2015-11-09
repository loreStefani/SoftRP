#ifndef SOFTRP_MATRIX_IMPL_INL_
#define SOFTRP_MATRIX_IMPL_INL_
#include "Matrix.h"
#include<stdexcept>
namespace SoftRP {
	namespace Math {

		template<typename T, unsigned int N>
		inline Matrix<T, N>::Matrix() {
			for (unsigned int i = 0, rowIndex = 0, columnIndex = 0; i < N; i++, rowIndex += N, columnIndex++) {
				unsigned int j = rowIndex;
				for (; j < rowIndex + columnIndex; j++)
					m_data[j] = static_cast<T>(0);
				m_data[j] = static_cast<T>(1);
				j++;
				for (; j < rowIndex + N; j++)
					m_data[j] = static_cast<T>(0);
			}
		}

		template<typename T, unsigned int N>
		inline Matrix<T, N>::Matrix(std::initializer_list<T> init) {
			if (init.size() != COUNT)
				throw std::runtime_error{ "Invalid initializer_list size." };
			unsigned int i = 0;
			for (T x : init)
				m_data[i++] = x;
		}

		template<typename T, unsigned int N>
		inline Matrix<T, N>::Matrix(const T(&_data)[COUNT]) {
			for (unsigned int i = 0; i < COUNT; i++)
				m_data[i] = _data[i];
		}

		template<typename T, unsigned int N>
		inline T& Matrix<T, N>::operator[](const int i) {
			checkRange(i);
			return m_data[i];
		}

		template<typename T, unsigned int N>
		inline T& Matrix<T, N>::get(unsigned int i, unsigned int j) {
			return operator[](i*N + j);
		}

		template<typename T, unsigned int N>
		inline T* Matrix<T, N>::data() {
			return m_data;
		}
		
		template<typename T, unsigned int N>
		inline const T& Matrix<T, N>::operator[](const int i)const {
			checkRange(i);
			return m_data[i];
		}

		template<typename T, unsigned int N>
		inline const T& Matrix<T, N>::get(unsigned int i, unsigned int j)const {
			return operator[](i*N + j);
		}

		template<typename T, unsigned int N>
		inline const T* Matrix<T, N>::data()const {
			return m_data;
		}

		template<typename T, unsigned int N>
		inline Matrix<T, N>& Matrix<T, N>::operator+=(const Matrix& v) {
			generalized_operator(v, [](T a, T b) { return a + b; });
			return *this;
		}

		template<typename T, unsigned int N>
		inline Matrix<T, N>& Matrix<T, N>::operator-=(const Matrix& v) {
			generalized_operator(v, [](T a, T b) { return a - b; });
			return *this;
		}

		template<typename T, unsigned int N>
		inline Matrix<T, N>& Matrix<T, N>::operator*=(const Matrix& v) {
			for (unsigned int i = 0, rowIndex = 0; i < N; i++, rowIndex += N) {
				T row[N];
				for (unsigned int j = 0; j < N; j++)
					row[j] = m_data[rowIndex + j];
				for (unsigned int j = 0; j < N; j++) {
					T value{};
					for (unsigned int s = 0, k = 0; s < N; s++, k += N)
						value += row[s] * v.m_data[j + k];
					m_data[rowIndex + j] = value;
				}
			}
			return *this;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N> Matrix<T, N>::operator*(const Vector<T, N>& v) const {
			Vector<T, N> result;
			for (unsigned int i = 0; i < N; i++) {
				const unsigned int rowIndex = i*N;
				T row[N];
				for (unsigned int j = 0; j < N; j++)
					row[j] = m_data[rowIndex + j];

				T value{};
				for (unsigned int s = 0; s < N; s++)
					value += row[s] * v[s];
				result[i] = value;
			}

			return result;
		}
			
		template<typename T, unsigned int N>
		inline void Matrix<T, N>::checkRange(const unsigned int i) const {
#ifdef _DEBUG
			if (i < 0 || i >= COUNT)
				throw std::out_of_range{ "Invalid index." };
#endif
		}

		template<typename T, unsigned int N>
		template<typename F>
		inline void Matrix<T, N>::generalized_operator(const Matrix& v, const F f) {
			for (unsigned int i = 0; i < N; i++)
				m_data[i] = f(m_data[i], v.m_data[i]);
		}

	}
}
#endif
