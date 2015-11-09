#ifndef SOFTRP_RENDERER_H_
#define SOFTRP_RENDERER_H_
#include "SoftRPDefs.h"
#include "PipelineState.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "DepthBuffer.h"
#include "Vertex.h"
#include "ConstantBuffer.h"
#include "TextureUnit.h"
#include "ObjectPool.h"
#include<vector>
#include<memory>
#include <utility>
#include "SHClipper.h"
#include "BinRasterizer.h"
namespace SoftRP {
		
	/*
	Concrete data type which implements access to and use of the rendering pipeline.
	*/
	class Renderer {
	public:

		/*
		ctor. construct a Renderer which uses the Clipper and Rasterizer's implementations provided 
		by the factory objects passed in. These objects must outlive the Renderer instance.
		In the multithreaded version, each of the *ThreadsCount parameters is used to construct a ThreadPool 
		that will be used for the task suggested by the parameter's name.
		*/
#ifdef SOFTRP_MULTI_THREAD
		Renderer(const ClipperFactory& clipperFactory, const RasterizerFactory& rasterizerFactory,
				 size_t drawingThreadsCount = 4, size_t clippingThreadsCount = 16, 
				 size_t rasterizingThreadsCount = 16, size_t vertexShaderThreadsCount = 1);
#else
		Renderer(const ClipperFactory& clipperFactory, const RasterizerFactory& rasterizerFactory);
#endif
		
		//dtor
		~Renderer() = default;

		//copy
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		
		//move
		Renderer(Renderer&&) = delete;		
		Renderer& operator=(Renderer&&) = delete;

		/* setters */
		void setVertexBuffer(VertexBuffer* vertexBuffer);
		void setIndexBuffer(IndexBuffer* indexBuffer);
		void setRenderTarget(RenderTarget* renderTarget);
		void setDepthBuffer(DepthBuffer* depthBuffer);
		void setViewPort(ViewPort* viewPort);
		void setPipelineState(PipelineState* pipelineState);

		constexpr static size_t MAX_CONSTANT_BUFFERS{4};
		constexpr static size_t MAX_TEXTURE_UNITS{8};

		void setConstantBuffer(size_t slot, ConstantBuffer* constantBuffer);
		void setTextureUnit(size_t slot, TextureUnit* textureUnit);

		/* getters */
		VertexBuffer* getVertexBuffer()const;
		IndexBuffer* getIndexBuffer()const;
		RenderTarget* getRenderTarget()const;
		DepthBuffer* getDepthBuffer()const;
		ViewPort* getViewPort()const;
		PipelineState* getPipelineState()const;
		ConstantBuffer* getConstantBuffer(size_t slot)const;
		TextureUnit* getTextureUnit(size_t slot)const;
		
		/*
		clearing methods, the clear operations are not immediate, 
		but deferred to the first draw call
		*/
		//clear the current RenderTarget with the specified value
		void clearRenderTarget(Math::Vector4 clearValue);
		//clear the current DepthBuffer with the specified value
		void clearDepthBuffer(float clearValue);
		//clear the current RenderTarget with the default clear value
		void clearRenderTarget();
		//clear the current DepthBuffer with the default clear value
		void clearDepthBuffer();
		//change the default clear value for RenderTargets
		void setRenderTargetClearValue(Math::Vector4 clearValue);
		//change the default clear value for DepthBuffers
		void setDepthBufferClearValue(float clearValue);
				
#ifdef SOFTRP_MULTI_THREAD
		using Fence = ThreadPool::Fence;		
#else
		using Fence = uint8_t;
#endif
		/*
		draw count indices from the current IndexBuffer for instanceCount instances.
		the indices are used to index in the current VertexBuffer.
		The operation is not immediate, the caller has to use the Fence value returned to
		be notified of the completion. In the meantime, other draw calls can be made to the Renderer
		whose state can also be changed. However, clients are constrained to not change the state of the 
		components (e.g. data of VertexBuffer, IndexBuffer, ConstantBuffer, etc..) that was set up to the time of the call.
		*/
		Fence drawIndexed(size_t count, size_t instanceCount = 1);
		//block the calling thread until the draw call associated with the Fence passed in have been completed
		void wait(Fence f);
		//wait for the last Fence
		void wait();
		
	private:
		
		void handleClear();
		
		struct RendererState {
			PipelineState* pipelineState;
			VertexBuffer* vertexBuffer;
			IndexBuffer* indexBuffer;
			ViewPort* viewPort;
			DepthBuffer* depthBuffer;
			RenderTarget* renderTarget;
			ConstantBuffer* constantBuffers[MAX_CONSTANT_BUFFERS];
			TextureUnit* textureUnits[MAX_TEXTURE_UNITS];			
		};

		void drawIndexedTask(RendererState rendererState,
							 size_t indexCount, size_t triangleCount, ThreadPool::Fence rasterizerFence);

		void drawIndexedInstancedTask(RendererState rendererState,
							 size_t indexCount, size_t triangleCount, 
							 size_t instanceCount, ThreadPool::Fence rasterizerFence);

		bool m_clearDepth;
		bool m_clearRenderTarget;
		float m_clearDepthBufferValue;
		Math::Vector4 m_clearRenderTargetValue;					
		RendererState m_rendererState;
#ifdef SOFTRP_MULTI_THREAD
		ThreadPool::Fence m_rasterizerFence{};
		ThreadPool::Fence m_drawFence{};
		ThreadPool m_clipperThreadPool;
		ThreadPool m_rasterizerThreadPool;
		ThreadPool m_drawThreadPool;
		ThreadPool m_vertexShaderThreadPool;
	
		struct ResizeVectorActivationPolicy {
			template<typename T>
			void activate(std::vector<T>& v);
			template<typename T>
			void activate(std::vector<T>& v, size_t size);
		};

		struct ClearVectorDeactivationPolicy {
			template<typename T>
			void deactivate(std::vector<T>& v);
		};

		ObjectPool<std::vector<Vertex>, 
				   ConstructCreationPolicy<std::vector<Vertex>>, 
			       ResizeVectorActivationPolicy, 
			       ClearVectorDeactivationPolicy> vertexVectorPool{};
		ObjectPool<std::vector<uint64_t>,
			       ConstructCreationPolicy<std::vector<uint64_t>>, 
			       ResizeVectorActivationPolicy, 
			       ClearVectorDeactivationPolicy> indexVectorPool{};

		template<typename T, typename Factory>
		struct CreateManagedPolicy {
			CreateManagedPolicy(const Factory* _factory);
			std::unique_ptr<T> create() const;
			const Factory* factory;
		};

		ObjectPool<std::unique_ptr<Clipper>, CreateManagedPolicy<Clipper, ClipperFactory>> clipperPool;
		ObjectPool<std::unique_ptr<Rasterizer>, CreateManagedPolicy<Rasterizer, RasterizerFactory>> rasterizerPool;

#else
		std::unique_ptr<Clipper> m_clipper{};
		std::unique_ptr<Rasterizer> m_rasterizer{};
		std::vector<Vertex> m_vShaderInputs{};
		std::vector<Vertex> m_vShaderOutputs{};
		std::vector<uint64_t> m_outIndices{};
#endif
	};
}
#include "RendererImpl.inl"
#endif

