#ifndef SOFTRP_RENDER_TARGET_H_
#define SOFTRP_RENDER_TARGET_H_
#include "SoftRPDefs.h"
#include "Vector.h"
#include "ThreadPool.h"
namespace SoftRP {

	/*
	Abstract data type which represents the destination of the primitives rasterization operation.
	*/

	class RenderTarget {
	public:
		
		RenderTarget() = default;
		virtual ~RenderTarget() = default;
		
		/* dimensions */
		virtual unsigned int width()const = 0;
		virtual unsigned int height() const= 0;
		virtual void resize(unsigned int width, unsigned int height) = 0;

		/* getters */
		virtual Math::Vector4 get(unsigned int i, unsigned int j) = 0;

		/* setters */
		virtual void set(unsigned int i, unsigned int j, Math::Vector4 value) = 0;		
		
		/* clearing */
		virtual void clear(Math::Vector4 value) = 0;
#ifdef SOFTRP_MULTI_THREAD
		virtual void clear(Math::Vector4 value, ThreadPool& threadPool) = 0;
#endif
		
	protected:
		RenderTarget(const RenderTarget&) = delete;
		RenderTarget(RenderTarget&&) = delete;
		RenderTarget& operator=(const RenderTarget&) = delete;
		RenderTarget& operator=(RenderTarget&&) = delete;
	};
}
#endif

