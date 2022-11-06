#pragma once

#ifdef SIMD
#include <xmmintrin.h>
#endif

namespace dae
{
	struct Vector3;
	struct Vector4
	{
#ifdef SIMD
		union
		{
			__m128 m;
			struct
			{
				float x, y, z, w;
			};
		};
#else
		float x;
		float y;
		float z;
		float w;
#endif

		Vector4() = default;
		Vector4(float _x, float _y, float _z, float _w);
		Vector4(const Vector3& v, float _w);
#ifdef SIMD
		Vector4(__m128 m);
#endif

		float Magnitude() const;
		float SqrMagnitude() const;
		float Normalize();
		Vector4 Normalized() const;

		static float Dot(const Vector4& v1, const Vector4& v2);

		// operator overloading
		Vector4 operator*(float scale) const;
		Vector4 operator+(const Vector4& v) const;
		Vector4 operator-(const Vector4& v) const;
		Vector4& operator+=(const Vector4& v);
		float& operator[](int index);
		float operator[](int index) const;
	};
}
