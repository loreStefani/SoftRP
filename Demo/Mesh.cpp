#include "Mesh.h"
#include<stdexcept>
#include<utility>

using namespace SoftRP;
using namespace Math;

using namespace SoftRPDemo;

void Mesh::computeNormals() {
	if(!hasVertices())
		throw std::runtime_error{ "Can't compute normals without vertices' positions" };

	bool hadIndices = hasIndices();
	if (!hadIndices) {		
		m_indices.resize(vertexCount());
		for (size_t i = 0; i < vertexCount(); i++)
			m_indices[i] = i;		
	}

	std::vector<Vector3> m_perVertexNormal{};
	m_perVertexNormal.resize(vertexCount(), Vector3{});
	
	const size_t triangleCount = indexCount() / 3;
	for (size_t i = 0, index = 0; i < triangleCount; i++, index += 3) {
		uint64_t t0 = m_indices[index];
		uint64_t t1 = m_indices[index + 1];
		uint64_t t2 = m_indices[index + 2];

		const Vector3& pos0 = m_vertices[t0];
		const Vector3& pos1 = m_vertices[t1];
		const Vector3& pos2 = m_vertices[t2];

		const Vector3 v0 = pos1 - pos0;
		const Vector3 v2 = pos2 - pos0;
		Vector3 faceNormal = Math::cross(v0, v2).normalize();

		m_perVertexNormal[t0] += faceNormal;
		m_perVertexNormal[t1] += faceNormal;
		m_perVertexNormal[t2] += faceNormal;
	}

	for (auto& n : m_perVertexNormal)
		n.normalize();

	setNormals(std::move(m_perVertexNormal));

	if (!hadIndices)
		m_indices.clear();
}

void Mesh::computeTangents() {
	throw std::runtime_error{ "Not implemented yet" };
}

InputVertexLayout Mesh::fillBuffers(SoftRP::VertexBuffer** vertexBuffer, SoftRP::IndexBuffer** indexBuffer) const{

#ifdef _DEBUG
	if (!hasVertices())
		throw std::runtime_error{ "Invalid Mesh state : can't fill buffers without vertices." };
#endif

	std::vector<size_t> fieldsSizes{};
	if (hasNormals()) 
		fieldsSizes.push_back(3);
	
	if (hasTangents()) 
		fieldsSizes.push_back(3);	
	
	if (hasTextCoords()) 
		fieldsSizes.push_back(2);		
	
	InputVertexLayout inputVertexLayout{ InputVertexLayout::create(fieldsSizes) };
	
	std::unique_ptr<VertexBuffer> vertexBufferPtr{ new VertexBuffer{ inputVertexLayout.vertexStride()* vertexCount() } };

	float* start = vertexBufferPtr->get();
	
	for (size_t i = 0; i < vertexCount(); i++, start += inputVertexLayout.vertexStride()) {		
		*Math::vectorFromPtr<4>(start) = Vector4{ m_vertices[i], 1.0f };
		size_t currOffset = 4;
		if (hasNormals()) {
			*Math::vectorFromPtr<3>(start + currOffset) =  m_normals[i];
			currOffset += 3;
		}
		if (hasTangents()) {
			*Math::vectorFromPtr<3>(start + currOffset) = m_tangents[i] ;
			currOffset += 3;
		}
		if (hasTextCoords()) {
			*Math::vectorFromPtr<2>(start + currOffset) = m_textCoords[i];
			currOffset += 2;
		}
	}

	*vertexBuffer = vertexBufferPtr.release();

	if (hasIndices()) {
		std::unique_ptr<IndexBuffer> indexBufferPtr{ new IndexBuffer{ indexCount() } };
		uint64_t* indexStart = indexBufferPtr->get();

		for (size_t i = 0; i < indexCount(); i++)
			indexStart[i] = m_indices[i];

		*indexBuffer = indexBufferPtr.release();

	} else
		*indexBuffer = nullptr;
		
	return inputVertexLayout;
}