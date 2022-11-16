#pragma once
#include "Vector3.h"

class Triangle
{
public:
	Triangle();
	Triangle(const dae::Vector3& _v0, const dae::Vector3& _v1, const dae::Vector3& _v2);

	dae::Vector3 v0, v1, v2;

	dae::Vector3& operator[](int index);
	dae::Vector3 operator[](int index) const;

	bool contains(const dae::Vector2 point);

private:
	bool CheckEdge(const dae::Vector3 fromVertex, const dae::Vector3 toVertex, const dae::Vector2 point);
};

