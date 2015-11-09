#include "DemoAppBase.h"

using namespace SoftRP;
using namespace Math;
using namespace WindowsDemo;

namespace SoftRPDemo {

	class LightVertexShader : public VertexShader {
	public:

		LightVertexShader() = default;
		virtual ~LightVertexShader() = default;

#ifdef SOFTRP_MULTI_THREAD
		virtual ThreadPool::Fence operator()(const ShaderContext& sc, const Vertex* input, Vertex* output,
											 size_t vertexCount, size_t instance, ThreadPool& threadPool) const override {
#else
		virtual void operator()(const ShaderContext& sc, const Vertex* input, Vertex* output,
											 size_t vertexCount, size_t instance) const override {
#endif

			const Math::Matrix4* projViewWorld = sc.constantBuffers()[0]->getField(0, static_cast<unsigned int>(instance)).asMatrix4();
			const FMatrix fprojView = createFM(*projViewWorld);

			for (size_t i = 0; i < vertexCount; i++, input++, output++) {

				//assuming world transform == identity

				FVector worldPos = createFV(input->position());
				output->position() = createVector4FV(mulFM(fprojView, worldPos));
				*Math::vectorFromPtr<4>(output->getField(1)) = createVector4FV(worldPos);

				float* normal = output->getField(2);
				const float* inputNormal = input->getField(1);
				for (unsigned int i = 0; i < 3; i++)
					normal[i] = inputNormal[i];
								
				float* textCoords = output->getField(3);
				const float* inputTextCoords = input->getField(2);
				for (unsigned int i = 0; i < 2; i++)
					textCoords[i] = inputTextCoords[i];
			}

#ifdef SOFTRP_MULTI_THREAD
			return threadPool.currFence();
#endif
		}		
	};

	class LightPixelShader : public PixelShader {
	public:

		LightPixelShader() = default;
		virtual ~LightPixelShader() = default;

		virtual void operator() (const ShaderContext& sc, const PSExecutionContext& psec, size_t instance, Math::Vector4* out) const override {
			const ConstantBuffer& constantBuffer = *sc.constantBuffers()[0];
			const Math::Vector4& lightColor = *constantBuffer.getField(1).asVector4();			
			const Math::Vector3& eyePos = *constantBuffer.getField(2).asVector3();	
			const Math::Vector3& lightPos = *constantBuffer.getField(3).asVector3();
			const float specularExp = 650.0f;
			const float normFactor = (specularExp + 2.0f) / 2.0f;

			Math::Vector4 textCoordDerivatives[4];
			computeDDXDDY(psec, 1, textCoordDerivatives);

			const TextureUnit& textureUnit = *sc.textureUnits()[0];
			for (unsigned int i = 0; i < 4; i++) {
				if ((psec.mask & (1 << i)) == 0)
					continue;
				
				//add ambient term
				out[i] = Vector4{ 0.05f, 0.05f, 0.05f, 0.0f };

				const Vector3& position = *Math::vectorFromPtr<3>(psec.interpolated[i].getField(1));
				Math::Vector3 toLight = (lightPos - position).normalize();
				Math::Vector3 normal = *Math::vectorFromPtr<3>(psec.interpolated[i].getField(2));
				normal.normalize();

				const float cosTheta_i = normal.dot(toLight);				
				if (cosTheta_i <= 0.0f)
					//the light source doesn't contribute					
					continue;
								
				//compute brdf

				//get diffuse color
				const Math::Vector2& textCoords = *Math::vectorFromPtr<2>(psec.interpolated[i].getField(3));
				const Math::Vector4* dtcdx = textCoordDerivatives + getDDXIndex(i);
				const Math::Vector4* dtcdy = textCoordDerivatives + getDDYIndex(i);
				Math::Vector4 brdf = textureUnit.sample(textCoords, *Math::vectorFromPtr<2, 4>(dtcdx), *Math::vectorFromPtr<2, 4>(dtcdy));
				
				//get specular color
				Math::Vector4 specColor = Math::Vector4{1.0f, 1.0f, 1.0f, 1.0f} - brdf;
				specColor.w() = 1.0f;
				
				//specular contribution
				Math::Vector3 toCamera = (eyePos - position).normalize();
				Math::Vector3 halfVector = (toLight + toCamera).normalize();
				float cosTheta_h = normal.dot(halfVector);
				if (cosTheta_h > 0.0f)
					brdf += specColor * normFactor * std::powf(cosTheta_h, specularExp);
				
				//compute total contribution												
				out[i] += brdf * lightColor * cosTheta_i;				
			}
		}		
	};

	class ConstBuffColorPixelShader : public PixelShader {
	public:
		ConstBuffColorPixelShader() = default;
		~ConstBuffColorPixelShader() = default;

		virtual void operator() (const ShaderContext& sc,
								 const PSExecutionContext& psec,
								 size_t instance, Math::Vector4* out) const override {
			const Math::Vector4* color = sc.constantBuffers()[0]->getField(1).asVector4();
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
			DemoAppBase(hInstance, width, height), m_texture{ loadTexture("Resources/FloorsMarble0023_S.jpg") } {
					
			Mesh m{ MeshFactory::createSphere<true, true>(1.0f, 32, 32) };

			VertexBuffer* vertexBuffer;
			IndexBuffer* indexBuffer;

			m_inputVertexLayout = m.fillBuffers(&vertexBuffer, &indexBuffer); //position, normal, textCoord
			m_vertexBuffer.reset(vertexBuffer);
			m_indexBuffer.reset(indexBuffer);

			m_indexCount = m.indexCount();

			m_outputVertexLayout = OutputVertexLayout::create(3); //position, normal, textCoord
												
			m_texture.generateMipMaps();

			m_textureUnit0.setTexture(&m_texture);			
			m_textureUnit0.setMinificationSampler(&m_sampler);
			m_textureUnit0.setMagnificationSampler(&m_sampler);

			m_renderer.setTextureUnit(0, &m_textureUnit0);
			
			m_renderer.setRenderTargetClearValue(Math::Vector4{ 0.0f, 0.0f, 0.0f, 1.0f });
			m_renderer.setDepthBufferClearValue(1.0f);			

			m_maxRadius *= 2;
			m_radius = m_maxRadius;
			updateView();

			const Vector3 xAxis{1.0f, 0.0f, 0.0f};
			const Vector3 yAxis{0.0f, 1.0f, 0.0f};
			
			const Vector4 piAboutYQuat = createQuatFromAxisAngle(yAxis, PI);
			const Vector4 halfPiAboutXQuat = createQuatFromAxisAngle(xAxis, PI/2.0f);
						
			long long timePos = 0;
			m_rotationFrames.push_back({ timePos, createQuatFromAxisAngle(xAxis, 0.0f) });

			timePos += 3000;
			Vector4 quat1 = halfPiAboutXQuat;
			m_rotationFrames.push_back({ timePos, quat1 });

			timePos += 3000;
			quat1 = multiplyQuat(piAboutYQuat, quat1);
			m_rotationFrames.push_back({ timePos, quat1 });

			timePos += 3000;
			quat1 = multiplyQuat(piAboutYQuat, quat1);
			m_rotationFrames.push_back({ timePos, quat1 });

			timePos += 3000;
			quat1 = multiplyQuat(createQuatFromAxisAngle(xAxis, PI), quat1);
			m_rotationFrames.push_back({ timePos, quat1 });

			timePos += 3000;
			quat1 = multiplyQuat(piAboutYQuat, quat1);
			m_rotationFrames.push_back({ timePos, quat1 });

			timePos += 3000;
			quat1 = multiplyQuat(piAboutYQuat, quat1);
			m_rotationFrames.push_back({ timePos, quat1 });

			timePos += 3000;
			quat1 = multiplyQuat(halfPiAboutXQuat, quat1);
			m_rotationFrames.push_back({ timePos, quat1 });
		}

		virtual ~DemoApp() = default;
			
		virtual void renderFrame(long long deltaTime) override {

			m_currColorAnimPos += deltaTime;
			m_currPosAnimPos += deltaTime;

			if (m_currColorAnimPos > m_colorAnimTime) {
				m_currColorAnimPos = 0;
				std::swap(m_startLightColor, m_endLightColor);
			}
			
			float t = static_cast<float>(m_currColorAnimPos) / (m_colorAnimTime);
			m_lightColor = m_startLightColor;
			m_lightColor.lerp(t, m_endLightColor);
						
			if (m_currPosAnimPos >= m_rotationFrames[m_currRotationFrame + 1].timePos){
				m_currRotationFrame++;
				if (m_currRotationFrame == m_rotationFrames.size()-1) {
					m_currRotationFrame = 0;
					m_currPosAnimPos = 0;
				}
			}
			AnimationFrame& animFrame1 = m_rotationFrames[m_currRotationFrame];
			AnimationFrame& animFrame2 = m_rotationFrames[m_currRotationFrame + 1];

			t = static_cast<float>(m_currPosAnimPos - animFrame1.timePos) / (animFrame2.timePos - animFrame1.timePos);
			
			Vector4 rot = slerpQuat(animFrame1.rotation, animFrame2.rotation, t);
			Vector3 lightPosition = rotateByQuat(m_lightStartPosition, rot);

			m_renderer.clearDepthBuffer();
			m_renderer.clearRenderTarget();
			
			m_renderer.setIndexBuffer(m_indexBuffer.get());
			m_renderer.setVertexBuffer(m_vertexBuffer.get());
			m_renderer.setDepthBuffer(&m_depthBuffer);
			m_renderer.setRenderTarget(&m_renderTarget);
			m_renderer.setViewPort(&m_viewPort);
			
			//render sphere

			*(m_constantBuffer0.getField(0).asMatrix4()) = m_camera.projView();					
			*(m_constantBuffer0.getField(1).asVector3()) = m_lightColor;
			*(m_constantBuffer0.getField(2).asVector3()) = m_camera.position();			
			*(m_constantBuffer0.getField(3).asVector3()) = lightPosition;
			
			m_renderer.setConstantBuffer(0, &m_constantBuffer0);

			PipelineState pipelineState{ m_inputVertexLayout, m_vertexShader, m_outputVertexLayout, m_pixelShader };
			m_renderer.setPipelineState(&pipelineState);
			
			m_renderer.drawIndexed(m_indexCount);

			//render light source

			const float scale = 1.0f / 4.0f;

			Matrix4 world{
				scale, 0.0f, 0.0f, lightPosition[0],
				0.0f, scale, 0.0f, lightPosition[1],
				0.0f, 0.0f, scale, lightPosition[2],
				0.0f, 0.0f, 0.0f, 1.0f
			};
						
			*(m_constantBuffer1.getField(0).asMatrix4()) = m_camera.projView()*world;
			*(m_constantBuffer1.getField(1).asVector3()) = m_lightColor;
			m_renderer.setConstantBuffer(0, &m_constantBuffer1);

			PipelineState pipelineState1{ m_inputVertexLayout, m_lightVertexShader, m_lightVertexLayout, m_lightPixelShader };
			m_renderer.setPipelineState(&pipelineState1);

			m_renderer.drawIndexed(m_indexCount);

			m_renderer.wait();

			m_renderTarget.present();
		}

	private:

		//pre: quat is unit length
		Vector3 rotateByQuat(Vector3 v, Vector4 quat) {
			return Vector3{ multiplyQuat(multiplyQuat(quat, Vector4{ v, 0.0f }), conjugateQuat(quat)) };
		}

		Vector4 conjugateQuat(Vector4 quat) {
			return Vector4{ -quat[0], -quat[1], -quat[2], quat[3] };
		}

		Vector4 multiplyQuat(Vector4 quat1, Vector4 quat2) {
			const Vector3 v1 = Vector3{ quat1 };
			const Vector3 v2 = Vector3{ quat2 };
			const float s1 = quat1[3];
			const float s2 = quat2[3];
			Vector4 res{v2*s1 + v1*s2 + cross(v1, v2)};
			res[3] = s1*s2 - v1.dot(v2);
			return res;			
		}

		Vector4 slerpQuat(Vector4 quat1, Vector4 quat2, float t) {
			const float cosTheta = quat1.dot(quat2);
			const float theta = std::acosf(cosTheta);
			const float oneOverSinTheta = 1.0f/ std::sinf(theta);
			const float sinThetaT = std::sinf(theta*t);
			const float sinThetaOneMinusT = std::sinf(theta*(1.0f - t));
			return (quat1*sinThetaOneMinusT + quat2*sinThetaT)*oneOverSinTheta;
		}

		Vector4 createQuatFromAxisAngle(Vector3 axis, float theta) {
			const float halfTheta = theta * 0.5f;
			const float cosHalfTheta = std::cosf(halfTheta);
			const float sinHalfTheta = std::sinf(halfTheta);
			return Vector4{axis * sinHalfTheta, cosHalfTheta};
		}

		const long long m_colorAnimTime{ 3000 };
		long long m_currColorAnimPos{ 0 };		
		Vector3 m_startLightColor{ 1.0f, 0.0f, 0.0f};
		Vector3 m_endLightColor{ 0.0f, 0.0f, 1.0f };
		Vector3 m_lightColor{};
		
		struct AnimationFrame {
			long long timePos;
			Vector4 rotation;
		};

		std::vector<AnimationFrame> m_rotationFrames{};
		unsigned int m_currRotationFrame{ 0 };		
		long long m_currPosAnimPos{ 0 };
		const Vector3 m_lightStartPosition{ 0.0f, 2.0f, 0.0f };

		size_t m_indexCount;
		LightVertexShader m_vertexShader{};
		LightPixelShader m_pixelShader{};		
		PositionVertexShader m_lightVertexShader{};
		ConstBuffColorPixelShader m_lightPixelShader{};
		std::unique_ptr<VertexBuffer> m_vertexBuffer{ nullptr };
		std::unique_ptr<IndexBuffer> m_indexBuffer{ nullptr };
		LinearSampler m_sampler{};
		ConstantBuffer m_constantBuffer0{ std::vector<size_t>{16, 4, 3, 3} }; //projViewWorld, light color, view position, light position 
		ConstantBuffer m_constantBuffer1{ std::vector<size_t>{16, 4} }; //projViewWorld, light color
		TextureUnit m_textureUnit0{ };
		Texture2D<Math::Vector4> m_texture;		
		OutputVertexLayout m_lightVertexLayout{ OutputVertexLayout::create(0) };
	};
}