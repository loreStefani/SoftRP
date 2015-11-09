#ifndef SOFTRP_VERTEX_H_
#define SOFTRP_VERTEX_H_
#include "VertexLayout.h"
#include "Vector.h"
namespace SoftRP {

	/*
	Concrete data type which represents a reference to the data of a primitive vertex.
	Besides raw access to the fields, common operations that operate on them are offered.
	Vertex instances can be copied, operation that causes the vertex data to be copied too. In this case, 
	the VertexLayout, set along with the data, is used for the allocation and the deallocation. The 
	latter is done once the destructor is called (if not moved).
	*/
	class Vertex{
	public:
		
		//ctors
		Vertex() = default;
		Vertex(float* vertexData, VertexLayout* vertexLayout);
		
		//dtor
		~Vertex();

		//copy
		Vertex(const Vertex& v);
		Vertex& operator=(const Vertex& v);
		//move
		Vertex(Vertex&& v);
		Vertex& operator=(Vertex&& v);				

		/*
		set the reference to the data. The data layout must be the one 
		specified by vertexLayout.
		*/
		void setVertexData(float* vertexData, VertexLayout* vertexLayout);
		
		/* accessors */		
		float* vertexData();
		float* getField(size_t vertexFieldIndex);
		Math::Vector4& position();
		size_t fieldCount()const;
		VertexLayout& vertexLayout() const;

		/* const accessors */
		const float* vertexData() const;
		const float* getField(size_t vertexFieldIndex)const;
		const Math::Vector4& position()const;

		//scale the data
		void scale(float s);
		//add v's data to the data
		void add(const Vertex& v);
		//linearly interpolate between the data and v's data
		void lerp(float t, const Vertex& v);

		//same as scale, but exclude position
		void scaleVertexData(float s);
		//same as add, but exclude position
		void addVertexData(const Vertex& v);
		//same as lerp, but exclude position
		void lerpVertexData(float t, const Vertex& v);
						
	private:

		void move(Vertex&& v);
		void copy(const Vertex& v);
		void clean();		
		void scaleVertexData(float* dest, float s, size_t size);
		void addVertexData(float* dest, const float* src, size_t size);
		void lerpVertexData(float* dest, float t, const float* src, size_t size);
		
		bool m_own{ false };
		float* m_data{nullptr};
		size_t m_count{ 0 };
		VertexLayout* m_vertexLayout{ nullptr };
	};
}
#include "VertexImpl.inl"
#endif