#ifndef SOFTRP_BIN_RASTERIZER_H_
#define SOFTRP_BIN_RASTERIZER_H_
#include "Rasterizer.h"
#include "Vector.h"
#ifdef SOFTRP_MULTI_THREAD
#include "ThreadPool.h"
#include <mutex>
#endif
#include <unordered_set>
#include <queue>
namespace SoftRP {

	/*
	Rasterizer implementation which performs a tile-based rasterization. 
	The RenderTarget is divided into tiles, each of which is given a list of primitives. A bin is a term used to refer 
	collectively to a tile and its list.
	Once a primitive is transformed to Screen space, it is added to all the bins it covers, completely or partially. 
	Then for all nonempty bins, all the primitives on the list are rasterized on the correspondent tile.
	*/

	class BinRasterizer : public Rasterizer{
	public:

		BinRasterizer() = default;			
		virtual ~BinRasterizer() = default;
				
		/*
		If SOFTRP_USE_SIMD is defined, the implementation requires Vertexs' data to be 16-byte aligned.
		*/
#ifdef SOFTRP_MULTI_THREAD
		virtual ThreadPool::Fence rasterizeTriangles(const std::vector<Vertex>& vertices, const std::vector<uint64_t>& indices, 
													 size_t instance, ThreadPool& threadPool) override final;
#else
		virtual void rasterizeTriangles(const std::vector<Vertex>& vertices, const std::vector<uint64_t>& indices,
										size_t instance) override final;
#endif
		
		virtual void setRenderTarget(RenderTarget* renderTarget) override final;

	protected:
		BinRasterizer(const BinRasterizer&) = delete;
		BinRasterizer& operator=(const BinRasterizer&) = delete;
		BinRasterizer(BinRasterizer&&) = delete;
		BinRasterizer& operator=(BinRasterizer&&) = delete;

	private:

		static constexpr unsigned int TILE_WIDTH = 64;
		static constexpr unsigned int TILE_HEIGHT = 64;

		struct Bin;
		void rasterizeBin(Bin& bin, const std::vector<Vertex>* vertices, size_t instance);

		struct Triangle;
		struct TransformedVertex;

		unsigned int m_tilesPerWidth{ 0 };
		unsigned int m_tilesPerHeight{ 0 };
		unsigned int m_renderTargetWidth{ 0 };
		unsigned int m_renderTargetHeight{ 0 };
		size_t m_binsCount{ 0 };		
		std::vector<Bin> m_bins{};
		std::vector<TransformedVertex> m_transformedVertices{};
		std::vector<Triangle> m_triangles{};
		std::unordered_set<size_t> m_activeBins{};
		
		struct TransformedVertex {
			float invW;
			Math::Vector4 position;			
		};

		struct Triangle {			
			int32_t alpha0;
			int32_t beta0;
			int32_t gamma0;
			int32_t alphaXdecr;
			int32_t alphaYdecr;
			int32_t betaXdecr;
			int32_t betaYdecr;
			int32_t gammaXdecr;
			int32_t gammaYdecr;
			int32_t alphaBias;
			int32_t betaBias;
			int32_t gammaBias;
			float twiceArea;
			uint64_t i0;
			uint64_t i1;
			uint64_t i2;
		};

		struct Bin {
			Bin() = default;
			~Bin() = default;
			Bin(Bin&&);

			void addTriangle(size_t i);
			bool hasNext();
			size_t getNext();			
#ifdef SOFTRP_MULTI_THREAD						
			void setDone(bool _done);
#endif
			int32_t xMin;
			int32_t yMin;
			int32_t xMax;
			int32_t yMax;
#ifdef SOFTRP_MULTI_THREAD
			bool done{ false };
			std::mutex mutex{};
			std::condition_variable existsNext{};
#endif
			std::queue<size_t> triangleQueue{};//a queue of indices in m_triangles
		};		
	};

	/*
	RasterizerFactory implementation which creates BinRasterizers.
	*/
	class BinRasterizerFactory : public RasterizerFactory {
	public:
		BinRasterizerFactory() = default;
		~BinRasterizerFactory() = default;

		virtual Rasterizer* create()const override final {
			return new BinRasterizer{};
		}

	protected:
		BinRasterizerFactory(const BinRasterizerFactory&) = delete;
		BinRasterizerFactory& operator=(const BinRasterizerFactory&) = delete;
		BinRasterizerFactory(BinRasterizerFactory&&) = delete;
		BinRasterizerFactory& operator=(BinRasterizerFactory&&) = delete;
	};
}
#endif