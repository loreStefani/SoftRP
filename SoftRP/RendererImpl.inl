#ifndef SOFTRP_RENDERER_IMPL_INL_
#define SOFTRP_RENDERER_IMPL_INL_
#include "Renderer.h"
namespace SoftRP {
	
#ifdef SOFTRP_MULTI_THREAD
	template<typename T>
	inline void Renderer::ResizeVectorActivationPolicy::activate(std::vector<T>& v) {}
	
	template<typename T>
	inline void Renderer::ResizeVectorActivationPolicy::activate(std::vector<T>& v, size_t size) {
		v.resize(size);
	}

	template<typename T>
	inline void Renderer::ClearVectorDeactivationPolicy::deactivate(std::vector<T>& v) {
		v.clear();
	}

	template<typename T, typename Factory>
	inline Renderer::CreateManagedPolicy<T, Factory>::CreateManagedPolicy(const Factory* _factory) : factory{ _factory } {}

	template<typename T, typename Factory>
	inline std::unique_ptr<T> Renderer::CreateManagedPolicy<T, Factory>::create() const {
		return std::unique_ptr<T>{factory->create()};
	}

	inline Renderer::Renderer(const ClipperFactory& clipperFactory, const RasterizerFactory& rasterizerFactory, 
							  size_t drawingThreadsCount, size_t clippingThreadsCount,
							  size_t rasterizingThreadsCount, size_t vertexShaderThreadsCount)

		: clipperPool{ CreateManagedPolicy<Clipper, ClipperFactory>{&clipperFactory} },
		rasterizerPool{ CreateManagedPolicy<Rasterizer, RasterizerFactory>{&rasterizerFactory} },
		m_clipperThreadPool{ clippingThreadsCount }, m_rasterizerThreadPool{ rasterizingThreadsCount },
		m_drawThreadPool{ drawingThreadsCount }, m_vertexShaderThreadPool{ vertexShaderThreadsCount }{
	}

#else
	inline Renderer::Renderer(const ClipperFactory& clipperFactory, const RasterizerFactory& rasterizerFactory){
		m_clipper.reset(clipperFactory.create());
		m_rasterizer.reset(rasterizerFactory.create());
	}

#endif
		
	inline void Renderer::handleClear() {
#ifdef SOFTRP_MULTI_THREAD
		bool waitForPendingTasks = false;
#endif
		if (m_clearDepth) {
#ifdef SOFTRP_MULTI_THREAD
			m_rendererState.depthBuffer->clear(m_clearDepthBufferValue, m_rasterizerThreadPool);
			waitForPendingTasks = true;
#else
			m_rendererState.depthBuffer->clear(m_clearDepthBufferValue);
#endif
			m_clearDepth = false;
		}
		if (m_clearRenderTarget) {
#ifdef SOFTRP_MULTI_THREAD
			m_rendererState.renderTarget->clear(m_clearRenderTargetValue, m_rasterizerThreadPool);
			waitForPendingTasks = true;
#else
			m_rendererState.renderTarget->clear(m_clearRenderTargetValue);
#endif
			m_clearRenderTarget = false;
		}
#ifdef SOFTRP_MULTI_THREAD
		if (waitForPendingTasks)
			m_rasterizerFence = m_rasterizerThreadPool.addFence();
#endif
	}

	inline void Renderer::wait() {
#ifdef SOFTRP_MULTI_THREAD
		wait(m_drawFence);
#endif
	}

	inline void Renderer::wait(Fence f) {
#ifdef SOFTRP_MULTI_THREAD
		m_drawThreadPool.waitForFence(f);
#endif
	}

#ifdef SOFTRP_MULTI_THREAD	

	inline Renderer::Fence Renderer::drawIndexed(size_t count, size_t instanceCount) {
		const size_t triangleCount = count / 3;
		if (triangleCount == 0)
			return m_drawFence;

		handleClear();

		count = triangleCount * 3;

		//copy current RenderState
		RendererState rs = m_rendererState;

		if (instanceCount > 1) {
			ThreadPool::Fence rasterizerFence = m_rasterizerFence;
			m_rasterizerFence += instanceCount;
			m_drawFence = m_drawThreadPool.addTaskAndFence([this, rs, count, triangleCount, instanceCount, rasterizerFence]() {
				drawIndexedInstancedTask(std::move(rs), count, triangleCount, instanceCount, rasterizerFence);
			});
		} else {
			ThreadPool::Fence rasterizerFence = m_rasterizerFence++;
			m_drawFence = m_drawThreadPool.addTaskAndFence([this, rs, count, triangleCount, rasterizerFence]() {
				drawIndexedTask(std::move(rs), count, triangleCount, rasterizerFence);
			});
		}

		return m_drawFence;
	}

	inline void Renderer::drawIndexedTask(RendererState renderState, size_t count, size_t triangleCount,
										  ThreadPool::Fence rasterizerFence) {

		VertexLayout& inputVertexLayout = renderState.pipelineState->inputVertexLayout();
		VertexLayout& outputVertexLayout = renderState.pipelineState->outputVertexLayout();

		const auto vertexCount = renderState.vertexBuffer->size() / inputVertexLayout.vertexStride();

		auto deleteVertexArray = [&outputVertexLayout](float* ptr) {
			outputVertexLayout.deallocateVertexArray(ptr);
		};
		std::unique_ptr<float, decltype(deleteVertexArray)> workBufferPtr{
			outputVertexLayout.allocateVertexArray(vertexCount), deleteVertexArray };
		float* workBuffer = workBufferPtr.get();

		vertexVectorPool.acquire();
		std::vector<Vertex> vShaderInputs{ vertexVectorPool.takeOneAcquired(vertexCount) };
		std::vector<Vertex> vShaderOutputs{ vertexVectorPool.takeOneAcquired(vertexCount) };
		vertexVectorPool.release();

		float* vertexData = renderState.vertexBuffer->get();
		uint64_t* indexData = renderState.indexBuffer->get();

		for (size_t i = 0; i < vertexCount; i++) {
			vShaderInputs[i].setVertexData(inputVertexLayout.getVertexData(vertexData, i), &inputVertexLayout);
			vShaderOutputs[i].setVertexData(outputVertexLayout.getVertexData(workBuffer, i), &outputVertexLayout);
		}

		ShaderContext sc{};
		sc.setConstantBuffers(renderState.constantBuffers);
		sc.setTextureUnits(renderState.textureUnits);

		const VertexShader& vertexShader = renderState.pipelineState->vertexShader();
		ThreadPool::Fence vertexShaderFence = vertexShader(sc, vShaderInputs.data(), vShaderOutputs.data(), 
														   vertexCount, 0, m_vertexShaderThreadPool);

		std::vector<uint64_t> outIndices{ indexVectorPool.takeOne() };
		m_vertexShaderThreadPool.waitForFence(vertexShaderFence);
		
		auto clipper = clipperPool.takeOne();
		ThreadPool::Fence fence = clipper->clipTriangles(vShaderOutputs, indexData, triangleCount, outIndices, m_clipperThreadPool);
		
		const PixelShader& pixelShader = renderState.pipelineState->pixelShader();

		auto rasterizer = rasterizerPool.takeOne();
		rasterizer->setRenderTarget(renderState.renderTarget);
		rasterizer->setViewPort(renderState.viewPort);
		rasterizer->setDepthBuffer(renderState.depthBuffer);
		rasterizer->setPixelShader(&pixelShader);
		rasterizer->setShaderContext(&sc);

		m_rasterizerThreadPool.waitForFence(rasterizerFence);
		m_clipperThreadPool.waitForFence(fence);

		fence = rasterizer->rasterizeTriangles(vShaderOutputs, outIndices, 0, m_rasterizerThreadPool);
		clipperPool.putOne(std::move(clipper));
		vertexVectorPool.putOne(std::move(vShaderInputs));
		m_rasterizerThreadPool.waitForFence(fence);

		vertexVectorPool.putOne(std::move(vShaderOutputs));
		indexVectorPool.putOne(std::move(outIndices));
		rasterizerPool.putOne(std::move(rasterizer));
	}

	inline void Renderer::drawIndexedInstancedTask(RendererState renderState, size_t count, size_t triangleCount,
										  size_t instanceCount, ThreadPool::Fence rasterizerFence) {

		VertexLayout& inputVertexLayout = renderState.pipelineState->inputVertexLayout();
		VertexLayout& outputVertexLayout = renderState.pipelineState->outputVertexLayout();

		const auto vertexCount = renderState.vertexBuffer->size() / inputVertexLayout.vertexStride();

		auto deleteVertexArray = [&outputVertexLayout](float* ptr) {
			outputVertexLayout.deallocateVertexArray(ptr);
		};		
		std::unique_ptr<float, decltype(deleteVertexArray)> workBufferPingPtr{
			outputVertexLayout.allocateVertexArray(vertexCount), deleteVertexArray };
		std::unique_ptr<float, decltype(deleteVertexArray)> workBufferPongPtr{
			outputVertexLayout.allocateVertexArray(vertexCount), deleteVertexArray };

		float* workBufferPing = workBufferPingPtr.get();
		float* workBufferPong = workBufferPongPtr.get();
		
		vertexVectorPool.acquire();
		std::vector<Vertex> vShaderInputs{ vertexVectorPool.takeOneAcquired(vertexCount) };
		std::vector<Vertex> vShaderOutputsPing{ vertexVectorPool.takeOneAcquired(vertexCount) };
		std::vector<Vertex> vShaderOutputsPong{ vertexVectorPool.takeOneAcquired(vertexCount) };		
		vertexVectorPool.release();

		float* vertexData = renderState.vertexBuffer->get();
		uint64_t* indexData = renderState.indexBuffer->get();

		for (size_t i = 0; i < vertexCount; i++) {
			vShaderInputs[i].setVertexData(inputVertexLayout.getVertexData(vertexData, i), &inputVertexLayout);
			vShaderOutputsPong[i].setVertexData(outputVertexLayout.getVertexData(workBufferPong, i), &outputVertexLayout);
			vShaderOutputsPing[i].setVertexData(outputVertexLayout.getVertexData(workBufferPing, i), &outputVertexLayout);
		}

		ShaderContext sc{};
		sc.setConstantBuffers(renderState.constantBuffers);
		sc.setTextureUnits(renderState.textureUnits);
		
		indexVectorPool.acquire();
		std::vector<uint64_t> outIndicesPing{ indexVectorPool.takeOneAcquired() };
		std::vector<uint64_t> outIndicesPong{ indexVectorPool.takeOneAcquired() };		
		indexVectorPool.release();

		auto clipper = clipperPool.takeOne();	
		
		const VertexShader& vertexShader = renderState.pipelineState->vertexShader();
		const PixelShader& pixelShader = renderState.pipelineState->pixelShader();

		auto rasterizer = rasterizerPool.takeOne();
		rasterizer->setRenderTarget(renderState.renderTarget);
		rasterizer->setViewPort(renderState.viewPort);
		rasterizer->setDepthBuffer(renderState.depthBuffer);
		rasterizer->setPixelShader(&pixelShader);
		rasterizer->setShaderContext(&sc);
				
		for (size_t instance = 0; instance < instanceCount; instance++) {			
			ThreadPool::Fence f = vertexShader(sc, vShaderInputs.data(), vShaderOutputsPing.data(), vertexCount, 
											   instance, m_vertexShaderThreadPool);			
			m_vertexShaderThreadPool.waitForFence(f);
			
			f = clipper->clipTriangles(vShaderOutputsPing, indexData, triangleCount, outIndicesPing, m_clipperThreadPool);
			
			m_clipperThreadPool.waitForFence(f);
			m_rasterizerThreadPool.waitForFence(rasterizerFence);

			vShaderOutputsPong.swap(vShaderOutputsPing);
			outIndicesPong.swap(outIndicesPing);
			
			rasterizerFence = rasterizer->rasterizeTriangles(vShaderOutputsPong, outIndicesPong, instance, m_rasterizerThreadPool);			
			outIndicesPing.clear();
			vShaderOutputsPing.resize(vertexCount);
		}
				
		clipperPool.putOne(std::move(clipper));
		vertexVectorPool.putOne(std::move(vShaderInputs));
		vertexVectorPool.putOne(std::move(vShaderOutputsPing));
		indexVectorPool.putOne(std::move(outIndicesPing));

		m_rasterizerThreadPool.waitForFence(rasterizerFence);

		vertexVectorPool.putOne(std::move(vShaderOutputsPong));
		indexVectorPool.putOne(std::move(outIndicesPong));
		rasterizerPool.putOne(std::move(rasterizer));
	}

#else

	inline Renderer::Fence Renderer::drawIndexed(size_t count, size_t instanceCount) {
		const size_t triangleCount = count / 3;
		if (triangleCount == 0)
			return 0;

		handleClear();

		count = triangleCount * 3;

		VertexLayout& inputVertexLayout = m_rendererState.pipelineState->inputVertexLayout();
		VertexLayout& outputVertexLayout = m_rendererState.pipelineState->outputVertexLayout();

		const auto vertexCount = m_rendererState.vertexBuffer->size() / inputVertexLayout.vertexStride();

		auto deleteVertexArray = [&outputVertexLayout](float* ptr) {
			outputVertexLayout.deallocateVertexArray(ptr);
		};
		std::unique_ptr<float, decltype(deleteVertexArray)> workBufferPtr{
			outputVertexLayout.allocateVertexArray(vertexCount), deleteVertexArray };
		float* workBuffer = workBufferPtr.get();

		float* vertexData = m_rendererState.vertexBuffer->get();
		uint64_t* indexData = m_rendererState.indexBuffer->get();

		m_vShaderOutputs.resize(vertexCount);
		m_vShaderInputs.resize(vertexCount);
		for (size_t i = 0; i < vertexCount; i++) {
			m_vShaderInputs[i].setVertexData(inputVertexLayout.getVertexData(vertexData, i), &inputVertexLayout);
			m_vShaderOutputs[i].setVertexData(outputVertexLayout.getVertexData(workBuffer, i), &outputVertexLayout);
		}

		ShaderContext sc{};
		sc.setConstantBuffers(m_rendererState.constantBuffers);
		sc.setTextureUnits(m_rendererState.textureUnits);
		const VertexShader& vertexShader = m_rendererState.pipelineState->vertexShader();
		PixelShader& pixelShader = m_rendererState.pipelineState->pixelShader();

		m_rasterizer->setRenderTarget(m_rendererState.renderTarget);
		m_rasterizer->setViewPort(m_rendererState.viewPort);
		m_rasterizer->setDepthBuffer(m_rendererState.depthBuffer);
		m_rasterizer->setPixelShader(&pixelShader);
		m_rasterizer->setShaderContext(&sc);


		for (size_t instance = 0; instance < instanceCount; instance++) {
			m_vShaderOutputs.resize(vertexCount);
			vertexShader(sc, m_vShaderInputs.data(), m_vShaderOutputs.data(), vertexCount, instance);
			m_clipper->clipTriangles(m_vShaderOutputs, indexData, triangleCount, m_outIndices);
			m_rasterizer->rasterizeTriangles(m_vShaderOutputs, m_outIndices, instance);
			m_outIndices.clear();
		}

		m_vShaderInputs.clear();
		m_vShaderOutputs.clear();

		return 0;
	}
#endif

	inline void Renderer::setVertexBuffer(VertexBuffer* vertexBuffer) {
		assert(vertexBuffer != nullptr);
		m_rendererState.vertexBuffer = vertexBuffer;
	}

	inline void Renderer::setIndexBuffer(IndexBuffer* indexBuffer) {
		assert(indexBuffer != nullptr);
		m_rendererState.indexBuffer = indexBuffer;
	}

	inline void Renderer::setRenderTarget(RenderTarget* renderTarget) {
		assert(renderTarget != nullptr);
		m_rendererState.renderTarget = renderTarget;
	}

	inline void Renderer::setDepthBuffer(DepthBuffer* depthBuffer) {
		assert(depthBuffer != nullptr);
		m_rendererState.depthBuffer = depthBuffer;
	}

	inline void Renderer::setViewPort(ViewPort* viewPort) {
		assert(viewPort != nullptr);
		m_rendererState.viewPort = viewPort;
	}

	inline void Renderer::setPipelineState(PipelineState* pipelineState) {
		assert(pipelineState != nullptr);
		m_rendererState.pipelineState = pipelineState;
	}

	inline void Renderer::clearDepthBuffer() {
		m_clearDepth = true;
	}

	inline void Renderer::clearRenderTarget() {
		m_clearRenderTarget = true;
	}

	inline void Renderer::setRenderTargetClearValue(Math::Vector4 clearValue) {
		m_clearRenderTargetValue = clearValue;
	}

	inline void Renderer::setDepthBufferClearValue(float clearValue) {
		m_clearDepthBufferValue = clearValue;
	}

	inline void Renderer::clearRenderTarget(Math::Vector4 clearValue) {
		setRenderTargetClearValue(clearValue);
		clearRenderTarget();
	}

	inline void Renderer::clearDepthBuffer(float clearValue) {
		setDepthBufferClearValue(clearValue);
		clearDepthBuffer();
	}

	inline void Renderer::setConstantBuffer(size_t slot, ConstantBuffer* constantBuffer) {
		assert(slot < MAX_CONSTANT_BUFFERS  && slot >= 0);
		m_rendererState.constantBuffers[slot] = constantBuffer;
	}

	inline void Renderer::setTextureUnit(size_t slot, TextureUnit* textureUnit) {
		assert(slot < MAX_TEXTURE_UNITS && slot >= 0);
		m_rendererState.textureUnits[slot] = textureUnit;
	}

	inline VertexBuffer* Renderer::getVertexBuffer()const { return m_rendererState.vertexBuffer; }
	inline IndexBuffer* Renderer::getIndexBuffer()const { return m_rendererState.indexBuffer; }
	inline RenderTarget* Renderer::getRenderTarget()const { return m_rendererState.renderTarget; }
	inline DepthBuffer* Renderer::getDepthBuffer()const { return m_rendererState.depthBuffer; }
	inline ViewPort* Renderer::getViewPort()const { return m_rendererState.viewPort; }
	inline PipelineState* Renderer::getPipelineState()const { return m_rendererState.pipelineState; }
	inline ConstantBuffer* Renderer::getConstantBuffer(size_t slot)const {
		assert(slot < MAX_CONSTANT_BUFFERS && slot >= 0);
		return m_rendererState.constantBuffers[slot];
	}
	inline TextureUnit* Renderer::getTextureUnit(size_t slot)const {
		assert(slot < MAX_TEXTURE_UNITS && slot >= 0);
		return m_rendererState.textureUnits[slot];
	}

}
#endif
