#include "Triangle.h"

#include <cassert>

#include "Vector2.h"


Triangle::Triangle()
{}

Triangle::Triangle(const dae::Vector3& _v0, const dae::Vector3& _v1, const dae::Vector3& _v2) : v0(_v0), v1(_v1), v2(_v2)
{}

dae::Vector3& Triangle::operator[](int index)
{
	assert(index >= 0 && index <= 2);

	if (index == 0) return v0;
	if (index == 1) return v1;
	return v2;
}

dae::Vector3 Triangle::operator[](int index) const
{
	assert(index >= 0 && index <= 2);

	if (index == 0) return v0;
	if (index == 1) return v1;
	return v2;
}

bool Triangle::CheckEdge(const dae::Vector3 fromVertex, const dae::Vector3 toVertex, const dae::Vector2 point)
{
	auto edge = dae::Vector3(fromVertex, toVertex).GetXY();
	auto l = dae::Vector2(fromVertex.GetXY(), point);
	return dae::Vector2::Cross(edge, l) > 0;
}

bool Triangle::contains(const dae::Vector2 point)
{
	return CheckEdge(v0, v1, point)
		&& CheckEdge(v1, v2, point)
		&& CheckEdge(v2, v0, point);
}


