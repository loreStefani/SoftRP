#ifndef SOFTRP_RASTERIZER_H_
#define SOFTRP_RASTERIZER_H_
#include "SoftRPDefs.h"
#include "RenderTarget.h"
#include "ViewPort.h"
#include "PixelShader.h"
#include "DepthBuffer.h"
#include "Vertex.h"
#include "ShaderContext.h"
#include <vector>
#ifdef SOFTRP_MULTI_THREAD
#include "ThreadPool.h"
#endif
namespace SoftRP {
	
	/*
	Abstract data type which represents a primitive rasterizer.	
	The primitives are expected to be inside the canonical view volume and expressed in Clip space (see Clipper), i.e. the 
	clipping operation has already been performed. 
	First of all, primitives are transformed to Screen space with the perspective divide and the transformation defined 
	by the ViewPort provided, then the primitives are rasterized on the RenderTarget provided.
	Lastly, the PixelShader is invoked on a 2x2 pixel area-basis to support derivative calculations. An invokation is made 
	if at least one pixel of the block is produced by the rasterization and it is found to pass the depth-test. 
	The invokation is always supplied with the vertices' attributes	interpolated at the pixels' centers.
	*/

	class Rasterizer{
	public:

		Rasterizer() = default;
		virtual ~Rasterizer() = default;
		
		/*
		Rasterize a list of triangles, defined by vertices indexed by indices.
		In a multi-threaded environment, Rasterizers are allowed to use the ThreadPool passed in as
		argument to execute tasks related to the operation. A ThreadPool::Fence is returned to allow
		clients to be notified of the completion of all the tasks submitted, until then the arguments
		passed in can't be modified.
		*/
#ifdef SOFTRP_MULTI_THREAD
		virtual ThreadPool::Fence rasterizeTriangles(const std::vector<Vertex>& vertices, const std::vector<uint64_t>& indices,
													 size_t instance, ThreadPool& threadPool) = 0;
#else
		virtual void rasterizeTriangles(const std::vector<Vertex>& vertices, const std::vector<uint64_t>& indices,
										size_t instance) = 0;
#endif
		
		/* setters */
		virtual void setRenderTarget(RenderTarget* renderTarget);
		virtual void setViewPort(const ViewPort* viewPort);
		virtual void setDepthBuffer(DepthBuffer* depthBuffer);
		virtual void setPixelShader(const PixelShader* pixelShader);
		virtual void setShaderContext(const ShaderContext* shaderContext);

		/* getters */
		RenderTarget* renderTarget() const;
		const ViewPort* viewPort() const;
		DepthBuffer* depthBuffer()const;
		const PixelShader* pixelShader()const;
		const ShaderContext* shaderContext()const;

	protected:
		Rasterizer(const Rasterizer&) = delete;
		Rasterizer& operator=(const Rasterizer&) = delete;
		Rasterizer(Rasterizer&&) = delete;		
		Rasterizer& operator=(Rasterizer&&) = delete;

		//perform depth test and update DepthBuffer with the new value
		virtual bool depthTest(unsigned int i, unsigned int j, float compare);
		virtual void writePixel(unsigned int i, unsigned int j, Math::Vector4 data);
		
	private:
		RenderTarget* m_renderTarget{nullptr};
		const ViewPort* m_viewPort{ nullptr };
		DepthBuffer* m_depthBuffer{ nullptr };
		const PixelShader* m_pixelShader{ nullptr };
		const ShaderContext* m_shaderContext{ nullptr };
	};

	/*
	Abstract data type which is responsible for the creation of a Rasterizer.
	*/
	class RasterizerFactory {
	public:
		RasterizerFactory() = default;
		~RasterizerFactory() = default;

		virtual Rasterizer* create() const = 0;

	protected:
		RasterizerFactory(const RasterizerFactory&) = delete;
		RasterizerFactory& operator=(const RasterizerFactory&) = delete;
		RasterizerFactory(RasterizerFactory&&) = delete;
		RasterizerFactory& operator=(RasterizerFactory&&) = delete;
	};
		
}
#include "RasterizerImpl.inl"
#endif