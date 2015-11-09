#ifndef SOFTRP_VECTOR_IMPL_INL_
#define SOFTRP_VECTOR_IMPL_INL_
#include "Vector.h"
#include<cmath>
#include<stdexcept>
namespace SoftRP {
	namespace Math {

		template<typename T, unsigned int N>
		inline Vector<T,N>::Vector(std::initializer_list<T> init) {
			if (init.size() != N)
				throw std::runtime_error{ "Invalid initializer_list size." };
			unsigned int i = 0;
			for (T x : init)
				m_data[i++] = x;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>::Vector(const T(&_data)[N]) {
			for (unsigned int i = 0; i < N; i++)
				m_data[i] = _data[i];
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>::Vector(T* beg, T* end) : Vector{const_cast<const T*>(beg), const_cast<const T*>(end) } {			
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>::Vector(const T* beg, const T* end) {
			auto range = end - beg;
			if (range != N)
				throw std::runtime_error{ "Invalid range size." };
			unsigned int i = 0;
			while (beg != end) {
				m_data[i] = *beg;
				i++;
				beg++;
			}
		}

		template<typename T, unsigned int N>
		inline T& Vector<T, N>::x() { return m_data[0]; }

		template<typename T, unsigned int N>
		inline T& Vector<T, N>::y() { return m_data[1]; }

		template<typename T, unsigned int N>
		inline T& Vector<T, N>::operator[](const int i) {
			checkRange(i);
			return m_data[i];
		}

		template<typename T, unsigned int N>
		inline T* Vector<T, N>::begin() { return m_data; }
		
		template<typename T, unsigned int N>
		inline T* Vector<T, N>::end() { return m_data + DIMENSION; }
		
		template<typename T, unsigned int N>
		inline T* Vector<T, N>::data() { return m_data; }

		template<typename T, unsigned int N>
		inline const T& Vector<T, N>::x() const { return m_data[0]; }

		template<typename T, unsigned int N>
		inline const T& Vector<T, N>::y() const { return m_data[1]; }
		
		template<typename T, unsigned int N>
		inline const T& Vector<T, N>::operator[](const int i)const {
			checkRange(i);
			return m_data[i];
		}

		template<typename T, unsigned int N>
		inline const T* Vector<T, N>::begin() const { return m_data; }
		
		template<typename T, unsigned int N>
		inline const T* Vector<T, N>::end() const { return m_data + DIMENSION; }

		template<typename T, unsigned int N>
		inline const T* Vector<T, N>::data()const { return m_data; }

		template<typename T, unsigned int N>
		inline void Vector<T, N>::set(T x) {
			m_data[0] = x;
		}

		template<typename T, unsigned int N>
		inline void Vector<T, N>::set(T x, T y) {
			m_data[0] = x;
			m_data[1] = y;
		}
				
		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::operator+=(const Vector& v) {
			generalized_operator(v, [](T a, T b) { return a + b; });
			return *this;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::operator-=(const Vector& v) {
			generalized_operator(v, [](T a, T b) { return a - b; });
			return *this;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::operator*=(const Vector& v) {
			generalized_operator(v, [](T a, T b) { return a * b; });
			return *this;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::operator/=(const Vector& v) {
			generalized_operator(v, [](T a, T b) { return a / b; });
			return *this;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::operator*=(const T s) {
			for (unsigned int i = 0; i < N; i++)
				m_data[i] *= s;
			return *this;
		}

		template<typename T, unsigned int N>
		inline T Vector<T, N>::dot(const Vector& v) const {
			T res{};
			for (unsigned int i = 0; i < N; i++)
				res += m_data[i] * v.m_data[i];
			return res;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::min(const Vector& v) {
			for (unsigned int i = 0; i < N; i++)
				if (m_data[i] > v.m_data[i])
					m_data[i] = v.m_data[i];
			return *this;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::max(const Vector& v) {
			for (unsigned int i = 0; i < N; i++)
				if (m_data[i] < v.m_data[i])
					m_data[i] = v.m_data[i];
			return *this;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::lerp(T t, const Vector& v) {
			T oneMinusT = static_cast<T>(1) - t;
			for (unsigned int i = 0; i < N; i++) {
				T d = m_data[i] * oneMinusT;
				m_data[i] = d + v.m_data[i] * t;
			}
			return *this;
		}

		template<typename T, unsigned int N>
		inline T Vector<T, N>::squaredLength()const {
			return dot(*this);
		}

		template<typename T, unsigned int N>
		inline T Vector<T, N>::length()const {
			return std::sqrt(squaredLength());
		}

		template<typename T, unsigned int N>
		inline Vector<T, N>& Vector<T, N>::normalize() {
			T len = length();
			//TODO: division by zero
			T invLen = static_cast<T>(1) / len;
			for (unsigned int i = 0; i < N; i++)
				m_data[i] *= invLen;
			return *this;
		}

		template<typename T, unsigned int N>
		inline void Vector<T, N>::checkRange(const int i) const {
#ifdef _DEBUG
			if (i < 0 || i >= N)
				throw std::out_of_range{ "Invalid index" };
#endif
		}

		template<typename T, unsigned int N>
		template<typename F>
		inline void Vector<T, N>::generalized_operator(const Vector<T, N>& v, const F f) {
			for (unsigned int i = 0; i < N; i++)
				m_data[i] = f(m_data[i], v.m_data[i]);
		}
	}
}
#endif