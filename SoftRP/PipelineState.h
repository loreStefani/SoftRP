#ifndef SOFTRP_PIPELINE_STATE_H_
#define SOFTRP_PIPELINE_STATE_H_
#include "VertexLayout.h"
#include "VertexShader.h"
#include "PixelShader.h"
namespace SoftRP {

	/*
	Concrete data type which represents the portion of the rendering pipeline whose components
	are highly dependent to each other.
	*/

	class PipelineState {
	public:

		//ctor. Construct a PipelineState from all the components
		PipelineState(InputVertexLayout& inputVertexLayout, VertexShader& vertexShader,
					  OutputVertexLayout& outputVertexLayout, PixelShader& pixelShader);
		
		~PipelineState() = default;

		//copy
		PipelineState(const PipelineState&) = default;
		PipelineState& operator=(const PipelineState&) = delete;
		
		//move
		PipelineState(PipelineState&&) = default;
		PipelineState& operator=(PipelineState&&) = delete;

		/* getters */
		InputVertexLayout& inputVertexLayout();
		VertexShader& vertexShader();
		OutputVertexLayout& outputVertexLayout();
		PixelShader& pixelShader();

	private:
		InputVertexLayout& m_inputVertexLayout;		
		VertexShader& m_vertexShader;
		OutputVertexLayout& m_outputVertexLayout;
		PixelShader& m_pixelShader;
	};
}
#include "PipelineStateImpl.inl"
#endif
	
