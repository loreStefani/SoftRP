#ifndef SOFTRP_VECTOR_FUN_INL_
#define SOFTRP_VECTOR_FUN_INL_
#include "Vector.h"
namespace SoftRP {
	namespace Math {

		template<typename T, unsigned int N>
		inline Vector<T, N> operator+(const Vector<T, N>& v1, const Vector<T, N>& v2) {
			Vector<T, N> value{ v1 };
			value += v2;
			return value;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N> operator-(const Vector<T, N>& v1, const Vector<T, N>& v2) {
			Vector<T, N> value{ v1 };
			value -= v2;
			return value;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N> operator*(const Vector<T, N>& v1, const Vector<T, N>& v2) {
			Vector<T, N> value{ v1 };
			value *= v2;
			return value;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N> operator/(const Vector<T, N>& v1, const Vector<T, N>& v2) {
			Vector<T, N> value{ v1 };
			value /= v2;
			return value;
		}

		template<typename T, unsigned int N>
		inline Vector<T, N> operator*(const Vector<T, N>& v1, const T s) {
			Vector<T, N> value{ v1 };
			value *= s;
			return value;
		}

		template<typename T, unsigned int N>
		inline T dot(const Vector<T, N>& v1, const Vector<T, N>& v2) {
			return v1.dot(v2);
		}

		template<typename T, unsigned int N>
		inline Vector<T, N> min(const Vector<T, N>& v1, const Vector<T, N>& v2) {
			Vector<T, N> v{ v1 };
			return v.min(v2);
		}

		template<typename T, unsigned int N>
		inline Vector<T, N> max(const Vector<T, N>& v1, const Vector<T, N>& v2) {
			Vector<T, N> v{ v1 };
			return v.max(v2);
		}

		template<typename T, unsigned int N>
		inline Vector<T, N> lerp(T t, const Vector<T, N>& v1, const Vector<T, N>& v2) {
			Vector<T, N> v{ v1 };
			return v.lerp(t, v2);
		}

		template<typename T>
		inline Vector<T, 3> cross(const Vector<T, 3>& v1, const Vector<T, 3>& v2) {
			Vector<T, 3> v{ v1 };
			return v.cross(v2);
		}

		template<unsigned int N, typename T>
		inline Vector<T, N>* vectorFromPtr(T* data) {
			return reinterpret_cast<Vector<T, N>*>(data);
		}

		template<unsigned int N, typename T>
		inline const Vector<T, N>* vectorFromPtr(const T* data) {
			return reinterpret_cast<const Vector<T, N>*>(data);
		}

		template<unsigned int N, unsigned int M, typename = std::enable_if<(M > N)>::type, typename T>
		inline const Vector<T, N>* vectorFromPtr(const Vector<T, M>* data) {
			return reinterpret_cast<const Vector<T, N>*>(data);
		}
	}
}
#endif

