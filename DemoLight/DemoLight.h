#pragma once
#include <DemoAppBase.h>
#include "LightVertexShader.h"
#include "LightPixelShader.h"
#include "ConstBuffColorPixelShader.h"

namespace SoftRPDemo
{
	using namespace SoftRP;
	using namespace Math;

	class DemoApp : public DemoAppBase
	{
	public:

		explicit DemoApp(HINSTANCE hInstance, unsigned int width = defaultWindowSize(), unsigned int height = defaultWindowSize()) :
			DemoAppBase(hInstance, width, height), m_texture{ loadTexture("Resources/FloorsMarble0023_S.jpg") }
		{

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

			const Vector3 xAxis{ 1.0f, 0.0f, 0.0f };
			const Vector3 yAxis{ 0.0f, 1.0f, 0.0f };

			const Vector4 piAboutYQuat = createQuatFromAxisAngle(yAxis, PI);
			const Vector4 halfPiAboutXQuat = createQuatFromAxisAngle(xAxis, PI / 2.0f);

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

		virtual void renderFrame(long long deltaTime) override
		{

			m_currColorAnimPos += deltaTime;
			m_currPosAnimPos += deltaTime;

			if (m_currColorAnimPos > m_colorAnimTime)
			{
				m_currColorAnimPos = 0;
				std::swap(m_startLightColor, m_endLightColor);
			}

			float t = static_cast<float>(m_currColorAnimPos) / (m_colorAnimTime);
			m_lightColor = m_startLightColor;
			m_lightColor.lerp(t, m_endLightColor);

			if (m_currPosAnimPos >= m_rotationFrames[m_currRotationFrame + 1].timePos)
			{
				m_currRotationFrame++;
				if (m_currRotationFrame == m_rotationFrames.size() - 1)
				{
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
		Vector3 rotateByQuat(Vector3 v, Vector4 quat)
		{
			return Vector3{ multiplyQuat(multiplyQuat(quat, Vector4{ v, 0.0f }), conjugateQuat(quat)) };
		}

		Vector4 conjugateQuat(Vector4 quat)
		{
			return Vector4{ -quat[0], -quat[1], -quat[2], quat[3] };
		}

		Vector4 multiplyQuat(Vector4 quat1, Vector4 quat2)
		{
			const Vector3 v1 = Vector3{ quat1 };
			const Vector3 v2 = Vector3{ quat2 };
			const float s1 = quat1[3];
			const float s2 = quat2[3];
			Vector4 res{ v2*s1 + v1*s2 + cross(v1, v2) };
			res[3] = s1*s2 - v1.dot(v2);
			return res;
		}

		Vector4 slerpQuat(Vector4 quat1, Vector4 quat2, float t)
		{
			const float cosTheta = quat1.dot(quat2);
			const float theta = std::acosf(cosTheta);
			const float oneOverSinTheta = 1.0f / std::sinf(theta);
			const float sinThetaT = std::sinf(theta*t);
			const float sinThetaOneMinusT = std::sinf(theta*(1.0f - t));
			return (quat1*sinThetaOneMinusT + quat2*sinThetaT)*oneOverSinTheta;
		}

		Vector4 createQuatFromAxisAngle(Vector3 axis, float theta)
		{
			const float halfTheta = theta * 0.5f;
			const float cosHalfTheta = std::cosf(halfTheta);
			const float sinHalfTheta = std::sinf(halfTheta);
			return Vector4{ axis * sinHalfTheta, cosHalfTheta };
		}

		const long long m_colorAnimTime{ 3000 };
		long long m_currColorAnimPos{ 0 };
		Vector3 m_startLightColor{ 1.0f, 0.0f, 0.0f };
		Vector3 m_endLightColor{ 0.0f, 0.0f, 1.0f };
		Vector3 m_lightColor{};

		struct AnimationFrame
		{
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
		TextureUnit m_textureUnit0{};
		Texture2D<Math::Vector4> m_texture;
		OutputVertexLayout m_lightVertexLayout{ OutputVertexLayout::create(0) };
	};
}