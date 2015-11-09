#ifndef SOFTRP_MATRIX_H_
#define SOFTRP_MATRIX_H_
#include "MathCommon.h"
#include"Vector.h"
#include<type_traits>
#include<initializer_list>
namespace SoftRP {

	namespace Math {

		/*
		Concrete data type which represents a square matrix. 
		T is the elements' type, DIMENSION is the number of rows and the number of columns.		
		The elements are stored row-major.
		*/

		template<typename T, unsigned int DIMENSION>
		class Matrix{

			//The only dimensions allowed are 2, 3 and 4
			static_assert(DIMENSION >= 2 && DIMENSION <= 4, "Unexpected Matrix dimension. Dimensions allowed are 2, 3 and 4.");
			
		public:

			static constexpr unsigned int COUNT = DIMENSION * DIMENSION;
			
			//ctors
			//initialize a Matrix with the identity value
			explicit Matrix();
			//initialize a Matrix from an initializer_list. The elements are considered row-major.
			Matrix(std::initializer_list<T> init);
			//initialize a Matrix from a built-in array. The elements are considered row-major.
			Matrix(const T(&_data)[COUNT]);

			//dtor			
			~Matrix() = default;
			//copy
			Matrix& operator=(const Matrix& v) = default;
			Matrix(const Matrix& v) = default;
			//move
			Matrix(Matrix&& v) = default;
			Matrix& operator=(Matrix&& v) = default;
			
			/* accessors */
			T& operator[](const int i);
			T& get(unsigned int i, unsigned int j);
			T* data();

			/* const accessors */
			const T& operator[](const int i)const;
			const T& get(unsigned int i, unsigned int j)const;
			const T* data()const;		

			/*	operators	*/
			Matrix& operator+=(const Matrix& v);
			Matrix& operator-=(const Matrix& v);
			Matrix& operator*=(const Matrix& v);
			Vector<T, DIMENSION> operator*(const Vector<T, DIMENSION>& v) const;
						
		private:
			
			void checkRange(const unsigned int i) const;
			template<typename F>
			void generalized_operator(const Matrix& v, const F f);
					
			T m_data[COUNT];
		};
		
		using Matrix2 = Matrix<float, 2>;
		using Matrix3 = Matrix<float, 3>;
		using Matrix4 = Matrix<float, 4>;
	}
}
#include "MatrixImpl.inl"
#include "MatrixFun.inl"
#endif


