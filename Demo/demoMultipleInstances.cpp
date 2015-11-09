#include "DemoAppBase.h"
#include <random>

using namespace SoftRP;
using namespace Math;
using namespace WindowsDemo;

namespace SoftRPDemo {
	
	class PosVertexShader : public VertexShader {
	public:
		PosVertexShader() = default;
		virtual ~PosVertexShader() = default;
#ifdef SOFTRP_MULTI_THREAD
		virtual	ThreadPool::Fence operator()(const ShaderContext& sc,
											 const Vertex* input, Vertex* output, size_t vertexCount, size_t instance,
											 ThreadPool& threadPool) const override {
#else
		virtual	void operator()(const ShaderContext& sc,
											 const Vertex* input, Vertex* output, size_t vertexCount, 
											 size_t instance) const override {
#endif
			const Math::Matrix4* projView = sc.constantBuffers()[0]->getField(0).asMatrix4();
			const Math::Matrix4* world = sc.constantBuffers()[1]->getField(0, instance).asMatrix4();			
			const FMatrix fprojViewWorld = mulFM(createFM(*projView), createFM(*world));
			for (size_t i = 0; i < vertexCount; i++, input++, output++)
				output->position() = createVector4FV(mulFM(fprojViewWorld, createFV(input->position())));

#ifdef SOFTRP_MULTI_THREAD
			return threadPool.currFence();
#endif
		}		
	};

	class ConstBuffColorPixelShader : public PixelShader {
	public:
		ConstBuffColorPixelShader() = default;
		~ConstBuffColorPixelShader() = default;
		virtual void operator() (const ShaderContext& sc,
													  const PSExecutionContext& psec,
													  size_t instance, Math::Vector4* out) const override{
			const Math::Vector4* color = sc.constantBuffers()[1]->getField(1, instance).asVector4();			
			for (unsigned int i = 0; i < 4; i++) {
				if ((psec.mask & (1 << i)) == 0)
					continue;
				out[i] = *color;
			}
		}
	};
		
	class DemoApp : public DemoAppBase {
				
	public:

		explicit DemoApp(HINSTANCE hInstance, unsigned int width = defaultWindowSize(), unsigned int height = defaultWindowSize()) :
			DemoAppBase(hInstance, width, height){

			const float cubeSize = 1.0f;

			Mesh m{ MeshFactory::createCube<true>(cubeSize, cubeSize, cubeSize) };

			VertexBuffer* vertexBuffer;
			IndexBuffer* indexBuffer;
			m_inputVertexLayout = m.fillBuffers(&vertexBuffer, &indexBuffer);
			m_vertexBuffer.reset(vertexBuffer);
			m_indexBuffer.reset(indexBuffer);

			m_indexCount = m.indexCount();

			m_outputVertexLayout = OutputVertexLayout::create(0);
						
			m_renderer.setRenderTargetClearValue(Math::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f });
			m_renderer.setDepthBufferClearValue(1.0f);
									
			const Vector3 bigBoxSize = Vector3{ 5.0f, 5.0f, 5.0f };
			const Vector3 halfBigBoxSize = bigBoxSize * 0.5f;
			const float halfCubeSize = cubeSize / 2.0f;			
			const Vector3 cubesSpan = Vector3{ cubeSize + halfCubeSize, cubeSize + halfCubeSize,cubeSize + halfCubeSize };

			std::vector<Matrix4> worlds{};
			std::vector<Vector4> colors{};

			for (float x = -halfBigBoxSize[0]; x <= halfBigBoxSize[0]; x += cubesSpan[0]) 
				for (float y = -halfBigBoxSize[1]; y <= halfBigBoxSize[1]; y += cubesSpan[1])
					for (float z = -halfBigBoxSize[2]; z <= halfBigBoxSize[2]; z += cubesSpan[2]) {
						worlds.emplace_back(Math::Matrix4{
							1.0f, 0.0f, 0.0f, x,
							0.0f, 1.0f, 0.0f, y,
							0.0f, 0.0f, 1.0f, z,
							0.0f, 0.0f, 0.0f, 1.0f
						});
						colors.push_back(randomVec4());
					}				
			m_instanceCount = worlds.size();
			m_constantBuffer1.reset(new ConstantBuffer{ std::vector<size_t>{16, 4}, m_instanceCount });
			
			for (size_t i = 0; i < m_instanceCount; i++) {
				*m_constantBuffer1->getField(0, i).asMatrix4() = worlds[i];
				*m_constantBuffer1->getField(1, i).asVector4() = colors[i];
			}

			m_renderer.setConstantBuffer(0, &m_constantBuffer0);
			m_renderer.setConstantBuffer(1, m_constantBuffer1.get());

			m_maxRadius = std::sqrtf(bigBoxSize.dot(bigBoxSize));
			m_radius = m_maxRadius;
			updateView();
		}

		virtual ~DemoApp() = default;

		virtual void renderFrame(long long deltaTime) override {

			m_renderer.clearDepthBuffer();
			m_renderer.clearRenderTarget();

			PipelineState pipelineState{ m_inputVertexLayout, m_vertexShader, m_outputVertexLayout, m_pixelShader };
			m_renderer.setPipelineState(&pipelineState);
			
			m_renderer.setVertexBuffer(m_vertexBuffer.get());
			m_renderer.setIndexBuffer(m_indexBuffer.get());
			m_renderer.setDepthBuffer(&m_depthBuffer);
			m_renderer.setRenderTarget(&m_renderTarget);
			m_renderer.setViewPort(&m_viewPort);
			
			*(m_constantBuffer0.getField(0).asMatrix4()) = m_camera.projView();

			m_renderer.drawIndexed(m_indexCount, m_instanceCount);
			
			m_renderer.wait();
			m_renderTarget.present();
		}

	private:

		Vector4 randomVec4(){
			float x = m_distribution(m_randomEngine);
			float y = m_distribution(m_randomEngine);
			float z = m_distribution(m_randomEngine);
			return Vector4{ x, y, z, 1.0f };
		}
		
		size_t m_indexCount;
		size_t m_instanceCount;
		std::unique_ptr<VertexBuffer> m_vertexBuffer{ nullptr };
		std::unique_ptr<IndexBuffer> m_indexBuffer{ nullptr };
		PosVertexShader m_vertexShader{};
		ConstBuffColorPixelShader m_pixelShader{};		
		ConstantBuffer m_constantBuffer0{ std::vector<size_t>{16} };//projView
		std::unique_ptr<ConstantBuffer> m_constantBuffer1{ nullptr };
		std::random_device m_randomEngine{};
		std::uniform_real_distribution<float> m_distribution{0.0f, 1.0f};
	};
}