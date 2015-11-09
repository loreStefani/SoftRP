#ifndef SOFTRP_VECTOR_H_
#define SOFTRP_VECTOR_H_
#include"MathCommon.h"
#include<type_traits>
#include<initializer_list>
namespace SoftRP{
			
	namespace Math {

		/*
		Concrete data type which represents a vector.
		T is the elements' type, DIMENSION is the number of elements.
		*/

		template<typename T, unsigned int DIMENSION>
		class Vector{

			//The only dimensions allowed are 2, 3 and 4
			static_assert(DIMENSION >= 2 && DIMENSION <=4, "Unexpected Vector dimension. Dimensions allowed are 2, 3 and 4.");

		public:
			
			//ctors - continue
			explicit Vector() = default;
			Vector(std::initializer_list<T> init);			
			Vector(const T(&_data)[DIMENSION]);
			explicit Vector(T* beg, T* end);
			explicit Vector(const T* beg, const T* end);
															
			//dtor
			~Vector() = default;
			//copy
			Vector(const Vector& v) = default;			
			Vector& operator=(const Vector& v) = default;
			//move
			Vector(Vector&& v) = default;			
			Vector& operator=(Vector&& v) = default;
			
			/* accessors - continue */
			T& x();
			T& y();
			T& operator[](const int i);
			T* begin();
			T* end();
			T* data();
			
			/*	const accessors - continue */
			const T& x() const;
			const T& y() const;			
			const T& operator[](const int i)const;
			const T* begin() const;
			const T* end() const;
			const T* data()const;
				
			/* setters - continue */			
			void set(T x);
			void set(T x, T y);
			
			/* operators - continue */
			Vector& operator+=(const Vector& v);
			Vector& operator-=(const Vector& v);
			Vector& operator*=(const Vector& v);
			Vector& operator/=(const Vector& v);
			Vector& operator*=(const T s);
						
			Vector& min(const Vector& v);
			Vector& max(const Vector& v);
			Vector& lerp(T t, const Vector& v);
			Vector& normalize();
			T dot(const Vector& v) const;
			T squaredLength()const;
			T length()const;			

			//ctors - part 2
			template<typename U = T, typename = std::enable_if<(DIMENSION != 4)>::type>
			explicit Vector(Vector<T, DIMENSION + 1> v) {
				for (unsigned int i = 0; i < DIMENSION; i++)
					m_data[i] = v[i];
			}

			template<typename U = T, typename = std::enable_if<(DIMENSION != 2)>::type>
			explicit Vector(Vector<T, DIMENSION - 1> v, T last = 0) {
				for (unsigned int i = 0; i < DIMENSION - 1; i++)
					m_data[i] = v[i];
				m_data[DIMENSION - 1] = last;
			}

			template<typename U = T, typename = std::enable_if<(DIMENSION == 4)>::type>
			explicit Vector(Vector<T, DIMENSION - 2> v, T z = 0, T w = 0) {
				for (unsigned int i = 0; i < DIMENSION - 2; i++)
					m_data[i] = v[i];
				m_data[DIMENSION - 2] = z;
				m_data[DIMENSION - 1] = w;
			}

			/* accessors - part 2 */
			template<typename U = T>
			typename std::enable_if<(DIMENSION > 2), U&>::type z() { return m_data[2]; }

			template<typename U = T>
			typename std::enable_if<(DIMENSION > 3), U&>::type w() { return m_data[3]; }

			/*	const accessors - part 2 */
			template<typename U = T>
			typename std::enable_if<(DIMENSION > 2), const U&>::type z() const {
				return m_data[2];
			}

			template<typename U = T>
			typename std::enable_if<(DIMENSION > 3), const U&>::type w() const {
				return m_data[3];
			}

			/* setters - part 2 */
			template<typename = std::enable_if<(DIMENSION >= 3)>::type>
			void set(T x, T y, T z) {
				m_data[0] = x;
				m_data[1] = y;
				m_data[2] = z;
			}

			template<typename = std::enable_if<(DIMENSION == 4)>::type>
			void set(T x, T y, T z, T w) {
				m_data[0] = x;
				m_data[1] = y;
				m_data[2] = z;
				m_data[3] = w;
			}

			/* operators part 2 */
			template<typename U = T, typename = std::enable_if<(DIMENSION == 3)>::type>
			Vector& cross(const Vector& v) {
				const T uYvZ = m_data[1] * v.m_data[2];
				const T uZvX = m_data[2] * v.m_data[0];
				const T uXvY = m_data[0] * v.m_data[1];
				const T uZvY = m_data[2] * v.m_data[1];
				const T uXvZ = m_data[0] * v.m_data[2];
				const T uYvX = m_data[1] * v.m_data[0];
				m_data[0] = uYvZ - uZvY;
				m_data[1] = uZvX - uXvZ;
				m_data[2] = uXvY - uYvX;
				return *this;
			}			
															
		private:
							
			void checkRange(const int i) const;

			template<typename F>
			void generalized_operator(const Vector& v, const F f);
		
			T m_data[DIMENSION]{};
		};	
		
		using Vector2 = Vector<float, 2>;
		using Vector3 = Vector<float, 3>;
		using Vector4 = Vector<float, 4>;
	}
}
#include "VectorImpl.inl"
#include "VectorFun.inl"
#endif



