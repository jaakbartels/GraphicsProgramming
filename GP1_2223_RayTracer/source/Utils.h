#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float a { Vector3::Dot(ray.direction, ray.direction) };
			Vector3 oDiff{ ray.origin - sphere.origin };
			float b { 2 * Vector3::Dot(ray.direction, oDiff) };
			float c { Vector3::Dot(oDiff, oDiff) - sphere.radius * sphere.radius};

			//d is the discriminant of the equation
			float d = b * b - 4 * a * c;

			//We are only interested in full intersection (so discriminant > 0).
			if (d > 0)
			{
				//Use subtraction, except when t < tMin, then use addition for t.
				float t = (-b - sqrtf(d)) / 2 / a;

				if (t < ray.min)
				{
					t = (-b + sqrtf(d)) / 2 / a;
				}

				if (t>= ray.min && t <= ray.max)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.origin = ray.origin + t * ray.direction;
					hitRecord.materialIndex = sphere.materialIndex;
					hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
					return true;
				}
			}

			return false;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			const float dotProduct{ Vector3::Dot(ray.direction, plane.normal) };

			if (dotProduct < 0)
			{
				float t = Vector3::Dot(plane.origin - ray.origin, plane.normal) / dotProduct;

				if (t >= ray.min && t <= ray.max)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.normal = plane.normal;
					hitRecord.origin = ray.origin + t * ray.direction;
					return true;
				}
			}
			return false;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS

		inline bool CheckEdge(const Vector3& from, const Vector3& to, const Vector3& normal, const Vector3& p)
		{
			Vector3 edge = to - from;
			Vector3 pointToSide = p - from;
			return Vector3::Dot(normal, Vector3::Cross(edge, pointToSide)) >= 0;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			Vector3 a = triangle.v1 - triangle.v0;
			Vector3 b = triangle.v2 - triangle.v0;
			Vector3 normal = Vector3::Cross(a, b);

			if (Vector3::Dot(normal, ray.direction) == 0)
			{
				//Ray is parallel with Plane of triangle => no intersection
				return false;
			}

			Vector3 center = (triangle.v0 + triangle.v1 + triangle.v2) / 3.f;
			Vector3 l = center - ray.origin;
			float t = Vector3::Dot(l, normal) / Vector3::Dot(ray.direction, normal);

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			Vector3 p = ray.origin + t * ray.direction;

			if (CheckEdge(triangle.v0, triangle.v1, normal, p)
				&& CheckEdge(triangle.v1, triangle.v2, normal, p)
				&& CheckEdge(triangle.v2, triangle.v0, normal, p))
			{
				if (ignoreHitRecord)
				{
					//TODO: shadowCalculation
				}
				else
				{
					hitRecord.didHit = true;
					hitRecord.materialIndex = triangle.materialIndex;
					hitRecord.origin = p;
					hitRecord.normal = normal;
					hitRecord.t = t;
				}
				return true;
			}

			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			
			return light.origin - origin;
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			if(light.type == LightType::Directional)
			{
				return  light.color * light.intensity;
			}else
			{
				Vector3 diff = (light.origin - target);
				return light.color * (light.intensity/Vector3::Dot(diff,diff));
			}
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}