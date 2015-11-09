#ifndef SOFTRP_DEPTH_BUFFER_H_
#define SOFTRP_DEPTH_BUFFER_H_
#include "Texture2D.h"
namespace SoftRP {
	/*
	Specialization of Texture2D which provides storage for depth values.
	*/
	using DepthBuffer = Texture2D<float>;
}
#endif