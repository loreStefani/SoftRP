#pragma once
#include "Vector.h"
#include <vector>
#include <utility>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexLayout.h"
namespace SoftRPDemo {

	class Mesh {

		using Vector2 = SoftRP::Math::Vector2;
		using Vector3 = SoftRP::Math::Vector3;

	public:

		//ctor
		Mesh() = default;
		//dtor
		~Mesh() = default;
		//copy
		Mesh(const Mesh&) = default;
		Mesh& operator=(const Mesh&) = default;
		//move
		Mesh(Mesh&&) = default;
		Mesh& operator=(Mesh&&) = default;
		
		/* setters */

		void setVertices(Vector3* vertices, size_t count);
		void setIndices(uint64_t* indices, size_t count);
		void setNormals(Vector3* normals, size_t count);
		void setTangents(Vector3* tangents, size_t count);
		void setTextCoords(Vector2* textCoords, size_t count);
		void setVertices(std::vector<Vector3>&& vertices);
		void setIndices(std::vector<uint64_t>&& indices);
		void setNormals(std::vector<Vector3>&& normals);
		void setTangents(std::vector<Vector3>&& tangents);
		void setTextCoords(std::vector<Vector2>&& textCoords);
		
		template<typename It>
		void setVertices(It beg, It end);
		template<typename It>
		void setIndices(It beg, It end);
		template<typename It>
		void setNormals(It beg, It end);
		template<typename It>
		void setTangents(It beg, It end);
		template<typename It>
		void setTextCoords(It beg, It end);

		/* getters */

		std::vector<Vector3>& getVertices();
		std::vector<uint64_t>& getIndices();
		std::vector<Vector3>& getNormals();
		std::vector<Vector3>& getTangents();
		std::vector<Vector2>& getTextCoords();

		/* const getters */

		const std::vector<Vector3>& getVertices() const;
		const std::vector<uint64_t>& getIndices() const;
		const std::vector<Vector3>& getNormals() const;
		const std::vector<Vector3>& getTangents() const;
		const std::vector<Vector2>& getTextCoords() const;
								
		bool hasVertices()const;
		bool hasTextCoords()const;
		bool hasNormals()const;
		bool hasTangents() const;
		bool hasIndices()const;
		
		size_t vertexCount()const;
		size_t indexCount()const;

		//removes all vertices, texture coordinates, normals, tangents and indices
		void clear();

		SoftRP::InputVertexLayout fillBuffers(SoftRP::VertexBuffer**, SoftRP::IndexBuffer**) const;

		void computeNormals();
		void computeTangents();

	private:

		template<typename T, typename It>
		void set(std::vector<T>& dest, It beg, It end);

		template<typename T>
		void set(std::vector<T>& dest, T* src, size_t count);

		std::vector<uint64_t> m_indices{};
		std::vector<Vector2> m_textCoords{};
		std::vector<Vector3> m_vertices{};
		std::vector<Vector3> m_normals{};
		std::vector<Vector3> m_tangents{};
	};
	
	template<typename It>
	inline void Mesh::setVertices(It beg, It end) {
		set(m_vertices, beg, end);
	}

	template<typename It>
	inline void Mesh::setIndices(It beg, It end) {
		set(m_indices, beg, end);
	}

	template<typename It>
	inline void Mesh::setNormals(It beg, It end) {
		set(m_normals, beg, end);
	}

	template<typename It>
	inline void Mesh::setTangents(It beg, It end) {
		set(m_tangents, beg, end);
	}

	template<typename It>
	inline void Mesh::setTextCoords(It beg, It end) {
		set(m_textCoords, beg, end);
	}

	inline void Mesh::setVertices(Vector3* vertices, size_t count) {
		set(m_vertices, vertices, count);
	}

	inline void Mesh::setIndices(uint64_t* indices, size_t count) {
		set(m_indices, indices, count);
	}

	inline void Mesh::setNormals(Vector3* normals, size_t count) {
		set(m_normals, normals, count);
	}

	inline void Mesh::setTangents(Vector3* tangents, size_t count) {
		set(m_tangents, tangents, count);
	}

	inline void Mesh::setTextCoords(Vector2* textCoords, size_t count) {
		set(m_textCoords, textCoords, count);
	}

	inline void Mesh::setVertices(std::vector<Vector3>&& vertices) {
		m_vertices = std::forward<std::vector<Vector3>>(vertices);
	}

	inline void Mesh::setIndices(std::vector<uint64_t>&& indices) {
		m_indices = std::forward<std::vector<uint64_t>>(indices);
	}

	inline void Mesh::setNormals(std::vector<Vector3>&& normals) {
		m_normals = std::forward<std::vector<Vector3>>(normals);
	}

	inline void Mesh::setTangents(std::vector<Vector3>&& tangents) {
		m_tangents = std::forward<std::vector<Vector3>>(tangents);
	}

	inline void Mesh::setTextCoords(std::vector<Vector2>&& textCoords) {
		m_textCoords = std::forward<std::vector<Vector2>>(textCoords);
	}

	inline std::vector<Mesh::Vector3>& Mesh::getVertices() { return m_vertices; }
	inline const std::vector<Mesh::Vector3>& Mesh::getVertices() const { return m_vertices; }

	inline std::vector<uint64_t>& Mesh::getIndices() { return m_indices; }
	inline const std::vector<uint64_t>& Mesh::getIndices() const { return m_indices; }

	inline std::vector<Mesh::Vector3>& Mesh::getNormals() { return m_normals; }
	inline const std::vector<Mesh::Vector3>& Mesh::getNormals() const { return m_normals; }

	inline std::vector<Mesh::Vector3>& Mesh::getTangents() { return m_tangents; }
	inline const std::vector<Mesh::Vector3>& Mesh::getTangents() const { return m_tangents; }

	inline std::vector<Mesh::Vector2>& Mesh::getTextCoords() { return m_textCoords; }
	inline const std::vector<Mesh::Vector2>& Mesh::getTextCoords() const { return m_textCoords; }

	inline void Mesh::clear() {
		m_vertices.clear();
		m_indices.clear();
		m_normals.clear();
		m_tangents.clear();
	}

	inline bool Mesh::hasVertices()const { return m_vertices.size() > 0; }
	inline bool Mesh::hasTextCoords()const { return m_textCoords.size() > 0; }
	inline bool Mesh::hasNormals()const { return m_normals.size() > 0; }
	inline bool Mesh::hasTangents() const { return m_tangents.size() > 0; }
	inline bool Mesh::hasIndices()const { return m_indices.size() > 0; }

	inline size_t Mesh::vertexCount()const { return m_vertices.size(); }
	inline size_t Mesh::indexCount()const { return m_indices.size(); }

	template<typename T, typename It>
	inline void Mesh::set(std::vector<T>& dest, It beg, It end) {
		dest.clear();
		dest.assign(beg, end);
	}

	template<typename T>
	inline void Mesh::set(std::vector<T>& dest, T* src, size_t count) {
		dest.clear();
		dest.reserve(count);
		for (size_t i = 0; i < count; i++)
			dest.push_back(src[i]);
	}

}