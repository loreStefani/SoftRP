#pragma once
namespace SoftRPDemo {
	extern float PI;
	extern float _2PI;

	inline float degsToRadians(float degs) {
		return degs * (PI / 180.0f);
	}

	inline float radsToDegrees(float rads) {
		return rads * (180.0f / PI);
	}
}