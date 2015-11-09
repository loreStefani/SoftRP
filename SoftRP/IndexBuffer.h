#ifndef SOFTRP_INDEX_BUFFER_H_
#define SOFTRP_INDEX_BUFFER_H_
#include "Buffer.h"
#include<cstdint>
namespace SoftRP {
	/*
	Specialization of Buffer which provides storage for index data.
	*/
	using IndexBuffer = Buffer<uint64_t>;	
}
#endif