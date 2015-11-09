#ifndef SOFTRP_VERTEX_LAYOUT_H_
#define SOFTRP_VERTEX_LAYOUT_H_
#include "SoftRPDefs.h"
#include "AlignedPoolArrayAllocator.h"
#include "SimplePoolArrayAllocator.h"
#include<vector>

namespace SoftRP {
	
	/*
	Abstract data type which represents the layout's specification of a primitive vertex.
	The vertex's position is constrained to be the first field, 4 floats wide, and always included.
	Other fields are optional and have to be specified, in number and size, on construction.
	*/
	class VertexLayout {
	public:				
		
		/*
		ctor. construct a VertexLayout from the size of each field, the fieldsSizes's size is 
		the number of fields (zero-sized fields are not considered)
		*/
		VertexLayout(const std::vector<size_t>& fieldsSizes);

		//dtor
		virtual ~VertexLayout() = default;
		
		//copy
		VertexLayout(const VertexLayout&) = delete;
		VertexLayout& operator=(const VertexLayout&) = default;
		
		//move
		VertexLayout(VertexLayout&&) = default;		
		VertexLayout& operator=(VertexLayout&&) = default;
		
		//number of fields (attributes) in a vertex
		size_t fieldCount() const;
		//stride of a vertex in floats unit
		size_t vertexStride() const;
				
		/*
		in the followings declarations:
		- "data" points to an array of vertex data, each vertexStride() wide
		- "vertexData" points to a specific vertex data, vertexStride() wide
		*/
		
		//get a pointer to the vertexIndex-th vertex's data
		float* getVertexData(float* data, size_t vertexIndex) const;
		//get a pointer to the vertexIndex-th vertex's vertexFieldIndex-th field
		float* getVertexFieldData(float* data, size_t vertexIndex, size_t vertexFieldIndex)const;
		//get a pointer to the vertexFieldIndex-th field
		float* getVertexFieldData(float* vertexData, size_t vertexFieldIndex) const;

		float* getVertexPosition(float* data, size_t vertexIndex)const;
		float* getVertexPosition(float* vertexData)const;
				
		//same as the non-const versions
		const float* getVertexData(const float* data, size_t vertexIndex) const;		
		const float* getVertexFieldData(const float* data, size_t vertexIndex, size_t vertexFieldIndex)const;
		const float* getVertexFieldData(const float* vertexData, size_t vertexFieldIndex) const;
		const float* getVertexPosition(const float* data, size_t vertexIndex)const;
		const float* getVertexPosition(const float* vertexData)const;
						
		//allocate a single vertex data, vertexStride() wide
		virtual float* allocateVertex() = 0;
		//allocate count vertex data, each vertexStride() wide
		virtual float* allocateVertexArray(size_t count) = 0;
		//deallocate a single vertex data, obtained with allocateVertex()
		virtual void deallocateVertex(float* vertex) = 0;
		//deallocate a vertex data array, obtained with allocateVertexArray()
		virtual void deallocateVertexArray(float* vertexArray) = 0;
						
	private:

		struct VertexField {
			//unit = sizeof(float)
			size_t offset;
			size_t size;
		};

		size_t m_vertexStride;
		std::vector<VertexField> m_vertexFields{};
	};

	/*
	VertexLayout specialization which delegates the allocation operations to an allocator object.		
	*/
	template<template<typename T> typename Allocator>
	class AllocatorVertexLayout : public VertexLayout {
	public:
		
		//ctor
		AllocatorVertexLayout(const std::vector<size_t>& fieldsSizes);		
		
		//dtor
		virtual ~AllocatorVertexLayout() = default;
		
		//copy
		AllocatorVertexLayout(const AllocatorVertexLayout&) = delete;
		AllocatorVertexLayout(AllocatorVertexLayout&&) = default;
		
		//move
		AllocatorVertexLayout& operator=(const AllocatorVertexLayout&) = default;
		AllocatorVertexLayout& operator=(AllocatorVertexLayout&&) = default;
		
		/* allocations */
		virtual float* allocateVertex() override;
		virtual float* allocateVertexArray(size_t count) override;
		virtual void deallocateVertex(float* vertex) override;
		virtual void deallocateVertexArray(float* vertexArray) override;		

	private:
		Allocator<float> m_allocator;
	};

	/*
	AllocatorVertexLayout specialization which uses a SimplePoolAllocator for the allocation operations.
	*/
	class InputVertexLayout : public AllocatorVertexLayout<SimplePoolArrayAllocator>{
	public:
		
		/*
		factory method. 
		This is not necessary and it is the cause that the type is not declared 
		with a "using InputVertexLayout = AllocatorVertexLayout<SimplePoolArrayAllocator>".
		The purpose is mirroring the use of the type with the OutputVertexLayout's one, 
		later declared.
		*/
		static InputVertexLayout create(const std::vector<size_t>& fieldsSizes);

		virtual ~InputVertexLayout() = default;
		InputVertexLayout(const InputVertexLayout&) = delete;
		InputVertexLayout(InputVertexLayout&&) = default;
		InputVertexLayout& operator=(const InputVertexLayout&) = default;
		InputVertexLayout& operator=(InputVertexLayout&&) = default;
		
	protected:
		InputVertexLayout(const std::vector<size_t>& fieldsSizes);
	};
	
	/*
	AllocatorVertexLayout specialization which uses a AlignedPoolArrayAllocator for the allocation operations.
	Fields are constrained to be 4 floats wide (16 byte).
	Allocated vertex data is 16-byte aligned.
	*/
	class OutputVertexLayout : public AllocatorVertexLayout<AlignedPoolArrayAllocator>{
	public:
		/*
		factory method.
		Because the size of each field is constrained to be 4 floats wide, the number of fields 
		is the only requirement.
		*/
		static OutputVertexLayout create(const size_t fieldsCount);

		virtual ~OutputVertexLayout() = default;
		OutputVertexLayout(const OutputVertexLayout&) = delete;
		OutputVertexLayout(OutputVertexLayout&&) = default;
		OutputVertexLayout& operator=(const OutputVertexLayout&) = delete;
		OutputVertexLayout& operator=(OutputVertexLayout&&) = default;		
	protected:
		OutputVertexLayout(const std::vector<size_t>& fieldsSizes);
	};
}
#include "VertexLayoutImpl.inl"
#endif
