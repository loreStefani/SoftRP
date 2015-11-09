#ifndef SOFTRP_CLIPPER_H_
#define SOFTRP_CLIPPER_H_
#include <vector>
#include "Vertex.h"
#ifdef SOFTRP_MULTI_THREAD
#include "ThreadPool.h"
#endif
namespace SoftRP {
	
	/*	
	Abstract data type which represents a primitive clipper.
	Primitives are clipped against the canonical view volume which has, in NDC(normalized device coordinate) space, minimum 
	point at {-1, -1, -1} and maximum point at {1, 1, 1}. 
	
	However, primitives' vertices are expected to be in Clip space, the homogeneous 4D space in which they are 
	expressed before the perspective divide is performed.
	A point (x, y, z, w) in Clip space is inside the clipping volume iff: 
	-1 <= x/w <= 1
	-1 <= y/w <= 1
	-1 <= z/w <= 1
	or equivalently:
	-w <= x <= w	->	w-x >= 0, x + w >= 0
	-w <= y <= w	->	w-y >= 0, y + w >= 0
	-w <= z <= w	->	w-z >= 0, z + w >= 0	
	*/

	class Clipper{
	public:
				
		Clipper() = default;
		virtual ~Clipper() = default;

		/*
		Clip a list of triangles defined by vertices indexed by inIndices.
		The result, indexed with outIndices, is returned in vertices itself.
		In a multi-threaded environment, Clippers are allowed to use the ThreadPool passed in as 
		argument to execute tasks related to the operation. A ThreadPool::Fence is returned to allow 
		clients to be notified of the completion of all the tasks submitted, until then the arguments 
		passed in can't be modified.
		*/
#ifdef SOFTRP_MULTI_THREAD
		virtual ThreadPool::Fence clipTriangles(std::vector<Vertex>& vertices, uint64_t* inIndices, 
												size_t triangleCount, std::vector<uint64_t>& outIndices,
												ThreadPool& threadPool) = 0;
#else
		virtual void clipTriangles(std::vector<Vertex>& vertices, uint64_t* inIndices,
								   size_t triangleCount, std::vector<uint64_t>& outIndices) = 0;
#endif
	protected:
		Clipper(const Clipper&) = delete;
		Clipper(Clipper&&) = delete;
		Clipper& operator=(const Clipper&) = delete;
		Clipper& operator=(Clipper&&) = delete;
	};

	/*
	Abstract data type which is responsible for the creation of a Clipper.
	*/
	class ClipperFactory {
	public:		
		ClipperFactory() = default;
		~ClipperFactory() = default;
						
		virtual Clipper* create()const = 0;

	protected:
		ClipperFactory(const ClipperFactory&) = delete;
		ClipperFactory& operator=(const ClipperFactory&) = delete;
		ClipperFactory(ClipperFactory&&) = delete;
		ClipperFactory& operator=(ClipperFactory&&) = delete;
	};

}
#endif
