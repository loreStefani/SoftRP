#pragma once
#include "Vector.h"
#include "Mesh.h"
#include <string>
#include "WaveFrontReader.h"
#include <memory>
#include <cmath>

namespace SoftRPDemo {

	class MeshFactory {		
	public:
		MeshFactory() = delete;
		~MeshFactory() = delete;

		template<bool genTextCoords = false, bool genNormals = false, bool genTangents = false>
		static Mesh createCube(float width = 1.0f, float height = 1.0f, float depth = 1.0f);

		template<bool genTextCoords = false, bool genNormals = false, bool genTangents = false>
		static Mesh createSphere(float radius = 1.0f, unsigned int sliceCount = 16, unsigned int stackCount = 16);

		template<bool genTextCoords = false, bool genNormals = false, bool genTangents = false>
		static Mesh createGrid(float width = 1.0f, float depth = 1.0f, unsigned int m = 16, unsigned int n = 16);

		template<bool genTextCoords = false, bool genNormals = false, bool genTangents = false>
		static Mesh createFromObj(const std::wstring& fileName);	
	};
			
	template<bool genTextCoords, bool genNormals, bool genTangents>
	inline Mesh MeshFactory::createCube(float width, float height, float depth) {

		//Adapted from '3D Game Programming with DirectX 11' by Frank Luna (http://www.d3dcoder.net/d3d11.htm)

		Mesh mesh{};

		float w2 = 0.5f*width;
		float h2 = 0.5f*height;
		float d2 = 0.5f*depth;

		Vector3 vertices[]{	
			//front face
			{ -w2, -h2, d2 },
			{ -w2, +h2, d2 },
			{ +w2, +h2, d2 },
			{ +w2, -h2, d2 },
			//back face
			{ -w2, -h2, -d2 },
			{ +w2, -h2, -d2 },
			{ +w2, +h2, -d2 },
			{ -w2, +h2, -d2 },
			//top face
			{ -w2, +h2, +d2 },
			{ -w2, +h2, -d2 },
			{ +w2, +h2, -d2 },
			{ +w2, +h2, +d2 },
			//bottom face
			{ -w2, -h2, +d2 },
			{ +w2, -h2, +d2 },
			{ +w2, -h2, -d2 },
			{ -w2, -h2, -d2 },
			//left face
			{ -w2, -h2, -d2 },
			{ -w2, +h2, -d2 },
			{ -w2, +h2, +d2 },
			{ -w2, -h2, +d2 },
			//right face
			{ +w2, -h2, +d2 },
			{ +w2, +h2, +d2 },
			{ +w2, +h2, -d2 },
			{ +w2, -h2, -d2 }
		};

		mesh.setVertices(vertices, sizeof(vertices) / sizeof(Vector3));

		if (genTextCoords) {

			Vector2 textCoords[]{
				//front face
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f },
				//back face
				{ 1.0f, 1.0f },
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				//top face
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f },
				//bottom face
				{ 1.0f, 1.0f },
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				//left face
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f },
				//right face
				{ 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 1.0f, 1.0f }
			};

			mesh.setTextCoords(textCoords, sizeof(textCoords) / sizeof(Vector2));
		}
				
		uint64_t indices[]{

			//front
			0, 2, 1,
			0, 3, 2,
						
			//back
			4, 6, 5,
			4, 7, 6,

			//top
			8, 10, 9,
			8, 11, 10,
			
			//bottom
			12, 14, 13,
			12, 15, 14,
			
			//left
			16, 18, 17,
			16, 19, 18,

			//right
			20, 22, 21,
			20, 23, 22
		};

		mesh.setIndices(indices, sizeof(indices) / sizeof(uint64_t));	


		if (genNormals)
			mesh.computeNormals();

		if (genTangents)
			mesh.computeTangents();

		return mesh;
	}
		
	template<bool genTextCoords, bool genNormals, bool genTangents>
	inline Mesh MeshFactory::createSphere(float radius, unsigned int sliceCount, unsigned int stackCount) {
		
		//Adapted from '3D Game Programming with DirectX 11' by Frank Luna (http://www.d3dcoder.net/d3d11.htm)

		Mesh mesh{};
		
		std::vector<Vector3>& vertices = mesh.getVertices();

		vertices.push_back(Vector3{ 0.0f, radius, 0.0f });
		if (genTextCoords)
			mesh.getTextCoords().push_back(Vector2{ 0.0f, 0.0f });
		if (genNormals)
			mesh.getNormals().push_back(Vector3{ 0.0f, 1.0f, 0.0f });
		if (genTangents)
			mesh.getTangents().push_back(Vector3{ 1.0f, 0.0f, 0.0f });

		const float phiStep = PI / stackCount;
		const float thetaStep = _2PI / sliceCount;
				
		for (unsigned int i = 1; i <= stackCount - 1; i++) {
			const float phi = i*phiStep;
			
			for (unsigned int j = 0; j <= sliceCount; j++) {
				const float theta = j*thetaStep;

				const float radiusTimesSinPhi = radius*std::sinf(phi);
				const float x = radiusTimesSinPhi*std::cosf(theta);
				const float y = radius*std::cosf(phi);
				const float z = -radiusTimesSinPhi*std::sinf(theta);
				vertices.push_back(Vector3{x, y, z});

				if (genTextCoords)
					mesh.getTextCoords().push_back(Vector2{ theta / _2PI, phi / PI });
				if (genNormals)
					mesh.getNormals().push_back(Vector3{x, y, z}.normalize());
				if (genTangents)
					mesh.getTangents().push_back(Vector3{-z, 0.0f, x}.normalize());
			}
		}
		
		vertices.push_back(Vector3{ 0.0f, -radius, 0.0f });
		if (genTextCoords)
			mesh.getTextCoords().push_back(Vector2{ 0.0f, 1.0f });
		if (genNormals)
			mesh.getNormals().push_back(Vector3{ 0.0f, -1.0f, 0.0f });
		if (genTangents)
			mesh.getTangents().push_back(Vector3{ 1.0f, 0.0f, 0.0f });
		
		std::vector<uint64_t>& indices = mesh.getIndices();

		for (unsigned int i = 1; i <= sliceCount; i++) {
			indices.push_back(0);
			indices.push_back(i);
			indices.push_back(i + 1);
		}
				
		unsigned int baseIndex = 1;
		unsigned int ringVertexCount = sliceCount + 1;
		for (unsigned int i = 0; i < stackCount - 2; i++) {
			const unsigned int ringIndex = baseIndex + i*ringVertexCount;
			const unsigned int nextRingIndex = ringIndex + ringVertexCount;
			for (unsigned int j = 0; j < sliceCount; j++) {
				indices.push_back( ringIndex + j);
				indices.push_back(nextRingIndex + j);
				indices.push_back( ringIndex + j + 1);

				indices.push_back(nextRingIndex + j);
				indices.push_back(nextRingIndex + j + 1);
				indices.push_back(ringIndex + j + 1);
			}
		}
		
		const unsigned int southPoleIndex = (unsigned int)vertices.size() - 1;		
		baseIndex = southPoleIndex - ringVertexCount;

		for (unsigned int i = 0; i < sliceCount; ++i) {
			indices.push_back(southPoleIndex);			
			indices.push_back(baseIndex + i + 1);
			indices.push_back(baseIndex + i);
		}
		
		return mesh;
	}

	template<bool genTextCoords, bool genNormals, bool genTangents>
	inline static Mesh MeshFactory::createGrid(float width, float depth, unsigned int m, unsigned int n) {		
		
		//Adapted from '3D Game Programming with DirectX 11' by Frank Luna (http://www.d3dcoder.net/d3d11.htm)
		
		Mesh mesh{};

		unsigned int vertexCount = m*n;
		unsigned int faceCount = (m - 1)*(n - 1) * 2;

		const float halfWidth = 0.5f*width;
		const float halfDepth = 0.5f*depth;

		const float dx = width / (n - 1);
		const float dz = depth / (m - 1);
		
		std::vector<Vector3> vertices{ vertexCount };
		
		for (unsigned int i = 0; i < m; i++) {
			const float z = -halfDepth + i*dz;
			for (unsigned int j = 0; j < n; j++) {
				const float x = -halfWidth + j*dx;
				Vector3& v = vertices[i*n + j];
				v[0] = x;
				v[1] = 0.0f;
				v[2] = z;
			}
		}

		mesh.setVertices(std::move(vertices));

		if (genTextCoords) {
			std::vector<Vector2> textCoords{vertexCount};
			const float du = 1.0f / (n - 1);
			const float dv = 1.0f / (m - 1);

			for (unsigned int i = 0; i < m; i++) {				
				for (unsigned int j = 0; j < n; j++) {
					Vector2& textCoord = textCoords[i*n + j];
					textCoord[0] = j*du;
					textCoord[1] = i*dv;
				}
			}
			mesh.setTextCoords(std::move(textCoords));
		}

		if (genNormals) {
			std::vector<Vector3> normals{};
			normals.resize(vertexCount, Vector3{ 0.0f, 1.0f, 0.0f });
			mesh.setNormals(std::move(normals));
		}

		if (genTangents) {
			std::vector<Vector3> tangents{};
			tangents.resize(vertexCount, Vector3{ 1.0f, 0.0f, 0.0f });
			mesh.setTangents(std::move(tangents));
		}
		

		std::vector<uint64_t> indices{};
		indices.resize(faceCount * 3);
				
		for (unsigned int i = 0, k = 0; i < m - 1; i++) {
			const unsigned int rowIndex = i*n;
			const unsigned int nextRowIndex = rowIndex + n;
			for (unsigned int j = 0; j < n - 1; j++, k+= 6) {
				indices[k] = rowIndex + j;
				indices[k + 1] = nextRowIndex + j;
				indices[k + 2] = rowIndex + j + 1;

				indices[k + 3] = nextRowIndex + j;
				indices[k + 4] = nextRowIndex + j + 1;
				indices[k + 5] = rowIndex + j + 1;
			}
		}

		mesh.setIndices(std::move(indices));

		return mesh;
	}
	
	template<bool genTextCoords, bool genNormals, bool genTangents>
	inline Mesh MeshFactory::createFromObj(const std::wstring& fileName) {

		Mesh mesh{};

		std::unique_ptr<WaveFrontReader<uint64_t>> reader{ new WaveFrontReader<uint64_t>{} };
		/*if (FAILED(reader->Load(fileName.c_str(), true)))
			;*/
		reader->Load(fileName.c_str(), true);

		const std::vector<WaveFrontReader<uint64_t>::Vertex>& vertices = reader->vertices;
		const std::vector<size_t>& indices = reader->indices;

		const auto vertexCount = vertices.size();

		std::vector<Vector3> outVertices{};

		for (auto& v : vertices)
			outVertices.push_back(Vector3{ v.position.x, v.position.y, v.position.z });

		mesh.setVertices(outVertices.data(), outVertices.size());

		std::vector<uint64_t> outIndices{};

		for (auto& i : indices)
			outIndices.push_back(i);

		mesh.setIndices(outIndices.data(), outIndices.size());

		if (genTextCoords) {
			if (!reader->hasTexcoords)
				throw std::runtime_error{"The loaded model doesn't have texture coordinates"};
			std::vector<Vector2> textCoords{};
			for (auto& v : vertices)
				textCoords.push_back(Vector2{ v.textureCoordinate.x, 1.0f - v.textureCoordinate.y });
			mesh.setTextCoords(textCoords.data(), textCoords.size());
		}

		if (genNormals) {
			if (reader->hasNormals) {
				std::vector<Vector3> normals{};
				for (auto& v : vertices)
					normals.push_back(Vector3{ v.normal.x, v.normal.y, v.normal.z });
				mesh.setNormals(normals.data(), normals.size());
			} else 
				mesh.computeNormals();			
		}

		if (genTangents)
			mesh.computeTangents();

		return mesh;
	}
}
