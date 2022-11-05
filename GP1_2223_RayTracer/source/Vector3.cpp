#include "Vector3.h"

#include <cassert>

#include "Vector4.h"
#include <cmath>

namespace dae {
	const Vector3 Vector3::UnitX = Vector3{ 1, 0, 0 };
	const Vector3 Vector3::UnitY = Vector3{ 0, 1, 0 };
	const Vector3 Vector3::UnitZ = Vector3{ 0, 0, 1 };
	const Vector3 Vector3::Zero = Vector3{ 0, 0, 0 };

	Vector3::Vector3(float _x, float _y, float _z) : v{ _x,_y,_z,1.f } {}

	Vector3::Vector3(const Vector4& v) : v{ v.x, v.y, v.z, 1.f } {}

	Vector3::Vector3(const Vector3& from, const Vector3& to) : v{ to.x() - from.x(), to.y() - from.y(), to.z() - from.z(), 1.f } {}

	Vector3::Vector3(__m128 v) : v{v} {}

	float Vector3::Magnitude() const
	{
		return sqrtf(x() * x() + y() * y() + z() * z());
	}

	float Vector3::SqrMagnitude() const
	{
		return x() * x() + y() * y() + z() * z();
	}

	float Vector3::Normalize()
	{
		const float m = Magnitude();
		v = __m128 {x() / m, y() / m, z() / m, 1.f};
		return m;
	}

	Vector3 Vector3::Normalized() const
	{
		const float m = Magnitude();
		return { x() / m, y() / m, z() / m};
	}

	float Vector3::Dot(const Vector3& v1, const Vector3& v2)
	{
		const __m128 result = _mm_mul_ps(v1.v, v2.v);
		return result.m128_f32[0]+result.m128_f32[1]+result.m128_f32[2];
	}

	Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2)
	{
		return Vector3{ v1.y() * v2.z() - v1.z() * v2.y(), -(v1.x() * v2.z() - v1.z() * v2.x()), v1.x() * v2.y() - v1.y() * v2.x()};
	}

	Vector3 Vector3::Project(const Vector3& v1, const Vector3& v2)
	{
		return (v2 * (Dot(v1, v2) / Dot(v2, v2)));
	}

	Vector3 Vector3::Reject(const Vector3& v1, const Vector3& v2)
	{
		return (v1 - v2 * (Dot(v1, v2) / Dot(v2, v2)));
	}

	Vector3 Vector3::Reflect(const Vector3& v1, const Vector3& v2)
	{
		return v1 - (2.f * Vector3::Dot(v1, v2) * v2);
	}

	Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2)
	{
		return { std::max(v1.x(), v2.x()), std::max(v1.y(), v2.y()), std::max(v1.z(), v2.z())};
	}

	Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2)
	{
		return { std::min(v1.x(), v2.x()), std::min(v1.y(), v2.y()), std::min(v1.z(), v2.z())};
	}


	Vector4 Vector3::ToPoint4() const
	{
		return { x(), y(), z(), 1};
	}

	Vector4 Vector3::ToVector4() const
	{
		return { x(), y(), z(), 0};
	}

#pragma region Operator Overloads
	Vector3 Vector3::operator*(float scale) const
	{
		return { _mm_mul_ps(v, {scale, scale, scale, scale})};
	}

	Vector3 Vector3::operator/(float scale) const
	{
		return { _mm_div_ps(v, {scale, scale, scale, scale}) };
	}

	Vector3 Vector3::operator+(const Vector3& other) const
	{
		return { _mm_add_ps(v, other.v)};
	}

	Vector3 Vector3::operator-(const Vector3& other) const
	{
		return { _mm_sub_ps(v, other.v) };
	}

	Vector3 Vector3::operator-() const
	{
		return { _mm_mul_ps(v, {-1,-1,-1,-1})};
	}

	Vector3& Vector3::operator*=(float scale)
	{
		v = _mm_mul_ps(v, { scale, scale, scale, scale });
		return *this;
	}

	Vector3& Vector3::operator/=(float scale)
	{
		v = _mm_div_ps(v, { scale, scale, scale, scale });
		return *this;
	}

	Vector3& Vector3::operator-=(const Vector3& other)
	{
		v = _mm_sub_ps(v, other.v);
		return *this;
	}

	Vector3& Vector3::operator+=(const Vector3& other)
	{
		v = _mm_add_ps(v, other.v);
		return *this;
	}

	float& Vector3::operator[](int index)
	{
		assert(index <= 2 && index >= 0);

		return v.m128_f32[index];
	}

	float Vector3::operator[](int index) const
	{
		assert(index <= 2 && index >= 0);

		return v.m128_f32[index];
	}
#pragma endregion
}