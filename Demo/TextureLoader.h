#pragma once
#include "stb_image.h"
#include "Texture2D.h"
#include "Vector.h"
#include <string>

namespace SoftRPDemo {

	inline SoftRP::Texture2D<SoftRP::Math::Vector4> loadTexture(std::string filePath) {

		int width;
		int height;
		int componentsCount;
		unsigned char* imageData = stbi_load(filePath.c_str(), &width, &height, &componentsCount, 0);
		if (imageData == nullptr) {
#ifdef _DEBUG
			throw std::runtime_error{ stbi_failure_reason() };
#else
			throw std::runtime_error{ "Image loading failed!" };
#endif
		}

		SoftRP::Texture2D<SoftRP::Math::Vector4> res{ static_cast<unsigned int>(width), static_cast<unsigned int>(height) };
		int currentComponent = 0;

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				const float red = static_cast<float>(imageData[currentComponent++]) / 255.0f;
				const float green = static_cast<float>(imageData[currentComponent++]) / 255.0f;
				const float blue = static_cast<float>(imageData[currentComponent++]) / 255.0f;
				const float alpha = componentsCount == 4 ? static_cast<float>(imageData[currentComponent++]) / 255.0f : 1.0f;
				res.set(i, j, SoftRP::Math::Vector4{ red, green, blue, alpha });
			}
		}

		stbi_image_free(imageData);
		return res;
	}

}