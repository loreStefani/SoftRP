#ifndef SOFTRP_SH_CLIPPER_H_
#define SOFTRP_SH_CLIPPER_H_
#include "Clipper.h"
#include<memory>
#include "SoftRPDefs.h"
#ifdef SOFTRP_MULTI_THREAD
#include "ThreadPool.h"
#endif
namespace SoftRP {

	/*
	Clipper implementation based on the Sutherland–Hodgman algorithm.
	See https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
	*/

	class SHClipper : public Clipper{				
	public:
		SHClipper() = default;
		virtual ~SHClipper() = default;
		
		/*
		If SOFTRP_USE_SIMD is defined, the implementation requires Vertexs' data to be 16-byte aligned.
		*/
#ifdef SOFTRP_MULTI_THREAD
		virtual ThreadPool::Fence clipTriangles(std::vector<Vertex>& vertices, uint64_t* inIndices, 
												size_t triangleCount, std::vector<uint64_t>& outIndices, 
												ThreadPool& threadPool) override final;
#else
		virtual void clipTriangles(std::vector<Vertex>& vertices, uint64_t* inIndices, 
						  size_t triangleCount, std::vector<uint64_t>& outIndices) override final;
#endif

	protected:
		SHClipper(const SHClipper&) = delete;
		SHClipper(SHClipper&&) = delete;
		SHClipper& operator=(const SHClipper&) = delete;
		SHClipper& operator=(SHClipper&&) = delete;

	private:
		
		void clipTriangleTask(std::vector<Vertex>* vertices, uint64_t* inIndices, size_t triangleOutputIndex);
		void prepareOutputTask(size_t triangleCount, std::vector<uint64_t>* outIndices);		
		
		struct TriangleTaskOutput;
		std::vector<TriangleTaskOutput> m_triangleTaskOutputs{};
#ifdef SOFTRP_MULTI_THREAD		
		std::mutex m_mutex{};
#endif
		
		struct TriangleTaskOutput {
			TriangleTaskOutput() = default;
			~TriangleTaskOutput() = default;
			TriangleTaskOutput(const TriangleTaskOutput&) = delete;			
			TriangleTaskOutput& operator=(const TriangleTaskOutput&) = delete;
			TriangleTaskOutput(TriangleTaskOutput&& tto);
			TriangleTaskOutput& operator=(TriangleTaskOutput&&) = delete;

#ifdef SOFTRP_MULTI_THREAD
			bool done{ false };
			std::mutex mutex{};
			std::condition_variable triangleClippedReady{};
#endif
			std::vector<uint64_t> indices{};
		};
	};

	/*
	ClipperFactory implementation which creates SHClippers.
	*/
	class SHClipperFactory : public ClipperFactory{
	public:
		SHClipperFactory() = default;
		~SHClipperFactory() = default;

		virtual Clipper* create()const override final {
			return new SHClipper{};
		}

	protected:
		SHClipperFactory(const SHClipperFactory&) = delete;
		SHClipperFactory& operator=(const SHClipperFactory&) = delete;
		SHClipperFactory(SHClipperFactory&&) = delete;
		SHClipperFactory& operator=(SHClipperFactory&&) = delete;		
	};
}
#endif
