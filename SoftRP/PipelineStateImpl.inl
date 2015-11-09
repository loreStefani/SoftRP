#ifndef SOFTRP_PIPELINE_STATE_IMPL_INL_
#define SOFTRP_PIPELINE_STATE_IMPL_INL_
#include "PipelineState.h"
namespace SoftRP {

	inline PipelineState::PipelineState(InputVertexLayout& inputVertexLayout, VertexShader& vertexShader,
										OutputVertexLayout& outputVertexLayout, PixelShader& pixelShader)
		: m_inputVertexLayout{ inputVertexLayout },
		m_vertexShader{ vertexShader },
		m_outputVertexLayout{ outputVertexLayout },
		m_pixelShader{ pixelShader } {

		//vertexShader.validateInput(inputVertexLayout)
		//vertexShader.validateOutput(outputVertexLayout)
		//pixelShader.validateInput(outputVertexLayout)
	}

	inline InputVertexLayout& PipelineState::inputVertexLayout() { return m_inputVertexLayout; }
	inline VertexShader& PipelineState::vertexShader() { return m_vertexShader; }
	inline OutputVertexLayout& PipelineState::outputVertexLayout() { return m_outputVertexLayout; }
	inline PixelShader& PipelineState::pixelShader() { return m_pixelShader; }
}
#endif
