#include<cassert>
#define STBI_ASSERT(x) assert((x))
#ifndef _DEBUG
#define STBI_NO_FAILURE_STRINGS
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"