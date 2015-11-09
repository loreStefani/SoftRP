#include "SHClipper.h"
#include<utility>
#include"Vertex.h"
#include "SIMDInclude.h"
#ifdef _DEBUG
#include <stdexcept>
#endif

using namespace SoftRP;
using Math::Vector4;
using std::vector;


SHClipper::TriangleTaskOutput::TriangleTaskOutput(TriangleTaskOutput&& tto) {
#ifdef SOFTRP_MULTI_THREAD
	std::lock_guard<std::mutex> lock{ tto.mutex };
	done = tto.done;
#endif
	indices = std::move(tto.indices);
}


#ifdef SOFTRP_MULTI_THREAD
ThreadPool::Fence SHClipper::clipTriangles(std::vector<Vertex>& vertices, uint64_t* inIndices, 
										   size_t triangleCount, std::vector<uint64_t>& outIndices, 
										   ThreadPool& threadPool)
#else
void SHClipper::clipTriangles(std::vector<Vertex>& vertices, uint64_t* inIndices,
							  size_t triangleCount,	std::vector<uint64_t>& outIndices)
#endif
																										{
#ifdef _DEBUG
	if (triangleCount == 0)
		throw std::runtime_error{ "Invalid triangleCount" };
#endif

	/*
	A task for each triangle is executed. Each task consists of clipping a triangle against all the clipping planes.
	Another task is then executed which prepares the tasks' outputs and write them in the correct order in outIndices.
	A different approach could be considered, where all triangles are clipped against one clipping planes at the time,
	but it can't be	parallelized easily as the former.
	*/

	m_triangleTaskOutputs.resize(triangleCount);
	/*
	the following pointers are necessary because capturing a reference in a lambda
	doesn't actually capture the object referenced to, but the reference variable itself
	*/
	std::vector<Vertex>* verticesPtr = &vertices;
	std::vector<uint64_t>* indicesPtr = &outIndices;

	for (size_t i = 0; i < triangleCount; i++) {
#ifdef SOFTRP_MULTI_THREAD
		m_triangleTaskOutputs[i].done = false;
		threadPool.addTask([this, verticesPtr, inIndices, i]() {
			clipTriangleTask(verticesPtr, inIndices, i);
		});
#else
		clipTriangleTask(verticesPtr, inIndices, i);
#endif
	}

#ifdef SOFTRP_MULTI_THREAD
	return threadPool.addTaskAndFence(
		[this, triangleCount, indicesPtr]() {
		prepareOutputTask(triangleCount, indicesPtr);
	});
#else
	prepareOutputTask(triangleCount, indicesPtr);
#endif
}

//helper functions forward declarations
static float intersectEdgePlane(const float p1DotPlane, float p2DotPlane);
#ifdef SOFTRP_USE_SIMD
static __m128 intersectEdgePlane(const __m128 p1DotPlane, const __m128 p2DotPlane);
static void lerpVertex(Vertex& v0, const Vertex& v1, __m128 t);
#endif
static void triangulate(std::vector<uint64_t>& indices, std::vector<uint64_t>* outIndices);


void SHClipper::clipTriangleTask(std::vector<Vertex>* _vertices, uint64_t* inIndices,
								 size_t triangleOutputIndex) {

	/*
	clipping planes defined in Clip space, the 4D space in which the vertices are expressed before the 
	perspective divide is performed.
	A 4D plane P=(a, b, c, d, e) is the set of points (x, y, z, w) that satisfy the equation:
	ax + by + cz + dw + e = 0
	Because all the clipping planes cross the origin (i.e. e = 0), a plane can be represented with a 4D vector, and 
	a point can be classified against a plane using the dot product:
	dot((a, b, c, d),(x, y, z, w)) = ax + by + cz + dw + 0 = ax + by + cz + dw = 0
	*/

#ifdef SOFTRP_USE_SIMD
	const __m128 planes[] = {
		//x+w >= 0
		_mm_set_ps(1.0f, 0.0f, 0.0f, 1.0f),
		//w-x >= 0
		_mm_set_ps(1.0f, 0.0f, 0.0f, -1.0f),
		//y+w >= 0
		_mm_set_ps(1.0f, 0.0f, 1.0f, 0.0f),
		//w-y >= 0
		_mm_set_ps(1.0f, 0.0f, -1.0f, 0.0f),
		//z+w >= 0
		_mm_set_ps(1.0f, 1.0f, 0.0f, 0.0f),
		//w-z >= 0
		_mm_set_ps(1.0f, -1.0f, 0.0f, 0.0f)
	};
#else
	const Vector4 planes[] = {
		//x+w >= 0
		Vector4{ 1.0f, 0.0f, 0.0f, 1.0f },
		//w-x >= 0
		Vector4{ -1.0f, 0.0f, 0.0f, 1.0f },
		//y+w >= 0
		Vector4{ 0.0f, 1.0f, 0.0f, 1.0f },
		//w-y >= 0
		Vector4{ 0.0f, -1.0f, 0.0f, 1.0f },
		//z+w >= 0
		Vector4{ 0.0f, 0.0f, 1.0f, 1.0f },
		//w-z >= 0
		Vector4{ 0.0f, 0.0f, -1.0f, 1.0f }
	};
#endif

	TriangleTaskOutput& output = m_triangleTaskOutputs[triangleOutputIndex];
	std::vector<Vertex>& vertices = *_vertices;
	size_t vertexIndex = triangleOutputIndex * 3;

	/*
	iterate through the clipping planes. for each clip plane, inList is used as input and outList as output. the two are then
	swapped before considering the next plane.
	because the lists' contents are swapped	at the beginning of the main loop, the initial indices are placed in outList.
	*/
	vector<uint64_t> inList{};
	vector<uint64_t>& outList = output.indices;
	outList.push_back(inIndices[vertexIndex]);
	outList.push_back(inIndices[vertexIndex + 1]);
	outList.push_back(inIndices[vertexIndex + 2]);

	constexpr size_t planeCount = sizeof(planes) / sizeof(planes[0]);
	//clipping a convex polygon with k vertices against a plane yields a convex polygon with k+1 vertices
	constexpr size_t MAX_VERTICES = 3 + planeCount;

#ifdef SOFTRP_USE_SIMD
	__m128i insideTest[MAX_VERTICES];
	__m128 planeTests[MAX_VERTICES];
#else
	bool insideTest[MAX_VERTICES];
	float planeTests[MAX_VERTICES];
#endif

	for (size_t k = 0; k < planeCount; k++) {

		size_t vertexCount = outList.size();
		
		if (vertexCount < 2)
			break;

#ifdef _DEBUG
		if (vertexCount > MAX_VERTICES)
			throw std::runtime_error{ "Programmer's ignorance produced an error!" };
#endif

		//move outList content to inList
		inList = std::move(outList);

#ifdef SOFTRP_USE_SIMD
		const __m128& plane = planes[k];
		const __m128 zero = _mm_set_ps1(0.0f);
		__m128i cull = _mm_set_epi32(0, 0, 0, 0);
#else
		const Vector4& plane = planes[k];
		bool cull = true;
#endif

#ifdef SOFTRP_MULTI_THREAD
		{
			//access to vertices is shared
			std::lock_guard<std::mutex> lock{ m_mutex };
#endif
			/*
			test all vertices against the plane, i.e. find out for each vertex, in which side of the plane lies.
			this is done using the dot product.
			*/
			for (size_t i = 0; i < vertexCount; i++) {
#ifdef SOFTRP_USE_SIMD
				const Vector4& position = vertices[inList[i]].position();
				__m128 posVec = _mm_load_ps(position.data());
				const __m128 planeTest = _mm_dp_ps(plane, posVec, 0xFF);
				planeTests[i] = planeTest;
				insideTest[i] = _mm_castps_si128(_mm_cmpge_ps(planeTest, zero));
				cull = _mm_or_si128(cull, insideTest[i]);
#else
				const float planeTest = plane.dot(vertices[inList[i]].position());
				planeTests[i] = planeTest;
				const bool test = planeTest >= 0;
				insideTest[i] = test;
				cull = cull && !test;
#endif
			}
#ifdef SOFTRP_MULTI_THREAD
		}
#endif

#ifdef SOFTRP_USE_SIMD
		if (cull.m128i_i32[0] == 0)
			break;
#else
		if (cull)
			break;
#endif
		
		/*
		consider all triangle's edges, four cases are possible depending on which side the edge's vertices are:
				
		first case : first outside and second inside
		following the edge we're moving inside the volume.
		replace the first vertex with the intersection between the edge and the clip volume; i.e. add the new vertex and 
		the second one to the output list.

		second case : first inside and second outside
		following the edge we're moving outside the volume.
		replace the second vertex with the intersection between the edge and the clip volume; i.e. add the new vertex 
		to the output list.

		third case : both outside
		do nothing
		
		fourth case : both inside
		add the second vertex to the output list		
		*/

		uint64_t first = inList[0];
		uint64_t second;
#ifdef SOFTRP_USE_SIMD
		bool firstInside = insideTest[0].m128i_i32[0] != 0;
#else
		bool firstInside = insideTest[0];
#endif
		
		bool secondInside;

		for (size_t i = 0; i < vertexCount; i++, first = second, firstInside = secondInside) {

			const size_t secondIndex = (i + 1) % vertexCount;
			second = inList[secondIndex];

#ifdef SOFTRP_USE_SIMD
			secondInside = insideTest[secondIndex].m128i_i32[0] != 0;
#else
			secondInside = insideTest[secondIndex];
#endif		
			//first or second case?
			if (firstInside != secondInside) {

#ifdef SOFTRP_USE_SIMD
				const __m128 t = intersectEdgePlane(planeTests[i], planeTests[secondIndex]);
#else
				const float t = intersectEdgePlane(planeTests[i], planeTests[secondIndex]);
#endif
				uint64_t newVertexIndex;
#ifdef SOFTRP_MULTI_THREAD
				{
					//access to vertices is shared
					std::lock_guard<std::mutex> lock{ m_mutex };
#endif
					Vertex newVertex{ vertices[first] };
#ifdef SOFTRP_USE_SIMD
					lerpVertex(newVertex, vertices[second], t);
#else
					newVertex.lerp(t, vertices[second]);
#endif
					//move newVertex
					newVertexIndex = vertices.size();
					vertices.push_back(std::move(newVertex));
#ifdef SOFTRP_MULTI_THREAD
				}
#endif
				outList.push_back(newVertexIndex);
				if (!firstInside)
					//cover first case
					outList.push_back(second);
			}
			//fourth case
			else if (firstInside && secondInside)
				outList.push_back(second);
			//else third case
		}
	}

	if (outList.size() < 3)
		//discard triangle
		outList.clear();

#ifdef SOFTRP_MULTI_THREAD
	//notify the end of the task
	{		
		std::lock_guard<std::mutex> lock{ output.mutex };
		output.done = true;
	}
	output.triangleClippedReady.notify_one();
#endif
	
	/*
	TODO: Consider triangulating.
	Another approach could be considering one triangle at the time (triangulating) for each plane and
	using the fact that clipping a triangle against a plane yields 0, 3 or 4 vertices.
	The first case is trivial and happens when the entire triangle is in the negative half space of the plane.
	The second case includes the other trival situation where the entire triangle is in the positive half space of the plane,
	but also the situation when only one of the vertices is in such space:
	    v2
	   *  *   dot(plane, triangle) > 0
	--*----*---- dot(plane, triangle) == 0
	 ******** dot(plane, triangle) < 0
	v0     v1

	The third case happens when only two of the vertices is in the positive half space of the plane:
	 v2      v1
	 ********  dot(plane, triangle) > 0
	--*----*---- dot(plane, triangle) == 0
	   *  *    dot(plane, triangle) < 0
	    v0
	*/
}


void SHClipper::prepareOutputTask(size_t triangleCount, std::vector<uint64_t>* outIndices) {
	
	for (size_t i = 0; i < triangleCount; i++) {
		TriangleTaskOutput& output = m_triangleTaskOutputs[i];
#ifdef SOFTRP_MULTI_THREAD
		{
			//wait for the triangle to be clipped
			std::unique_lock<std::mutex> lock{ output.mutex };
			while (!output.done)
				output.triangleClippedReady.wait(lock);
			output.done = false;
		}
#endif
		std::vector<uint64_t>& triangleIndices = output.indices;
		size_t indexCount = triangleIndices.size();
		if (indexCount == 0)
			continue;
		//clipping a triangle may result in a convex polygon that needs to be triangulated
		triangulate(triangleIndices, outIndices);
		triangleIndices.clear();
	}
}


inline static float intersectEdgePlane(const float p1DotPlane, float p2DotPlane) {

	/*
	Precondition : sign(p1DotPlane) != sign(p2DotPlane)
	Following what have been said about clipping planes in the definition of SHClipper::clipTriangleTask, 
	find intersection between an edge "(1-t)p1 + tp2, t in [0,1]" and a plane P = (a, b, c, d) in 4D, where 
	p1 = (x1, y1, z1, w1) and p2 = (x2, y2, z2, w2) are the two end point of the edge.
	The instersection is the point p' = (1-t)p1 + tp2 = p1 + t(p2 - p1) that satisfy the following:
	dot(P, p') = 0.
	Substituting P and p' yields:
	dot(P, p') = (a, b, c, d)(p1 + t(p2 - p1)) 
			   = (ax1 + by1 + cz1 + dw1) + t( ax2 + by2 + cz2 + dw2 - (ax1 + by1 + cz1 + dw1)) = 0
	-> t = - (ax1 + by1 + cz1 + dw1) / ( ax2 + by2 + cz2 + dw2 - (ax1 + by1 + cz1 + dw1))
		 = - dot(P, p1) / (dot(P, p2) - dot(P, p1))
	and t is in [0, 1] if p' lies on the segment.
	*/	
	const auto v = -p1DotPlane;
	const auto v1 = p2DotPlane;
	return v / (v1 + v);
}

#ifdef SOFTRP_USE_SIMD
inline static __m128 intersectEdgePlane(const __m128 p1DotPlane, const __m128 p2DotPlane) {
	const __m128 minusP1DotPlane = _mm_sub_ps(_mm_set_ps1(0.0f), p1DotPlane);
	return _mm_div_ps(minusP1DotPlane, _mm_add_ps(minusP1DotPlane, p2DotPlane));
}

inline static void lerpVertex(Vertex& v0, const Vertex& v1, __m128 t) {
	//assuming each field is 4-float wide and 16-byte aligned
	const size_t fieldCount = v0.vertexLayout().fieldCount();
	const __m128 oneMinusTVec = _mm_sub_ps(_mm_set_ps1(1.0f), t);
	float* field = v0.vertexData();
	const float* vField = v1.vertexData();
	for (size_t f = 0; f < fieldCount; f++, field += 4, vField += 4) {
		__m128 fieldVec = _mm_load_ps(field);
		__m128 vFieldVec = _mm_load_ps(vField);
		fieldVec = _mm_mul_ps(fieldVec, oneMinusTVec);
		vFieldVec = _mm_mul_ps(vFieldVec, t);
		fieldVec = _mm_add_ps(fieldVec, vFieldVec);
		_mm_store_ps(field, fieldVec);
	}
}
#endif

inline static void triangulate(std::vector<uint64_t>& indices, std::vector<uint64_t>* outIndices) {
	
	//triangulate in a triangle-fan fashion
	size_t size = indices.size();
	const uint64_t i0 = indices[0];
	uint64_t i1 = indices[1];
	uint64_t i2 = indices[2];
	size_t next = 3;

	while (size-- > 3) {
		outIndices->push_back(i0);
		outIndices->push_back(i1);
		outIndices->push_back(i2);
		i1 = i2;
		i2 = indices[next++];
	}

	outIndices->push_back(i0);
	outIndices->push_back(i1);
	outIndices->push_back(i2);
}
