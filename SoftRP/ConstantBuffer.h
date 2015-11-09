#ifndef SOFTRP_CONSTANT_BUFFER_H_
#define SOFTRP_CONSTANT_BUFFER_H_
#include "Buffer.h"
#include "Vector.h"
#include "Matrix.h"
#include <vector>
namespace SoftRP {
	
	/*
	Specialization of Buffer which provides storage for general purpose float data.
	A ConstantBuffer is structured by fields specified, in number and size, at construction time. 
	The fields can have multiple instances inside the ConstantBuffer. The number of instances is also specified at 
	construction time and is the same for all the fields.
	*/
	class ConstantBuffer : public Buffer<float>{
	public:
		
		/*
		ctor, Construct a ConstantBuffer allocating the necessary memory to hold all the fields and all their instances.
		Number of fields and size of each field is specified by fieldsSizes, while the number of instances of each field 
		is specified by instanceCount.
		*/
		ConstantBuffer(const std::vector<size_t>& fieldsSizes, size_t instanceCount = 1);
		/*
		ctor, Construct a ConstantBuffer which refers to the memory pointed to by data. 
		The memory area is assumed to be large enough to contain all the fields and all their instances.
		Number of fields and size of each field is specified by fieldsSizes, while the number of instances of each field 
		is specified by instanceCount.
		*/
		ConstantBuffer(const std::vector<size_t>& fieldsSizes, float* data, size_t instanceCount = 1);
		
		//dtor
		~ConstantBuffer() = default;

		//copy
		ConstantBuffer(const ConstantBuffer&);
		ConstantBuffer& operator=(const ConstantBuffer&);

		//move
		ConstantBuffer(ConstantBuffer&&);
		ConstantBuffer& operator=(ConstantBuffer&&);

		//Concrete data type which represents a proxy to a field.
		class ConstantBufferField;

		//accessors to the i-th field of the specified instance.
		ConstantBufferField getField(size_t i, size_t instance = 0);
		const ConstantBufferField getField(size_t i, size_t instance = 0) const;

		//number of fields
		size_t fieldCount() const;
		//size of an instance, i.e. size of all the fields of an instance, in unit of number of float. 
		size_t perInstanceSize()const;
				
		class ConstantBufferField {
		public:
			//ctors. Construct a ConstantBufferField from the start of the field in the ConstantBuffer
			ConstantBufferField(float* data);
			ConstantBufferField(const float* data);
			//dtor
			~ConstantBufferField() = default;
			//copy
			ConstantBufferField(const ConstantBufferField&) = default;
			ConstantBufferField& operator=(const ConstantBufferField&) = default;
			//move
			ConstantBufferField(ConstantBufferField&&) = default;
			ConstantBufferField& operator=(ConstantBufferField&&) = default;

			//accessors to the field
			Math::Vector2* asVector2();
			Math::Vector3* asVector3();
			Math::Vector4* asVector4();

			Math::Matrix2* asMatrix2();
			Math::Matrix3* asMatrix3();
			Math::Matrix4* asMatrix4();

			const Math::Vector2* asVector2() const;
			const Math::Vector3* asVector3() const;
			const Math::Vector4* asVector4() const;

			const Math::Matrix2* asMatrix2() const;
			const Math::Matrix3* asMatrix3() const;
			const Math::Matrix4* asMatrix4() const;
			
			float* asFloat();
			const float* asFloat() const;

		private:
			union {
				float* const m_data;
				const float* const m_dataConst;
			};
		};		

	private:

		static size_t computeSize(const std::vector<size_t>& fieldsSizes);				
		void initFields(const std::vector<size_t>& fieldsSizes);

		size_t m_instanceCount;
		size_t m_perInstanceSize;
		std::vector<size_t> m_offsets{};
	};	
}
#include "ConstantBufferImpl.inl"
#endif
