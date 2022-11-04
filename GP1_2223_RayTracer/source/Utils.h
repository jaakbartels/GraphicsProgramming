#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

#define MOLLER_TRUMBORE

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			float t0, t1;

			Vector3 L = sphere.origin - ray.origin;
			float tca = Vector3::Dot(L, ray.direction);
			if (tca < 0)
			{
				return false;
			}

			float d2 = Vector3::Dot(L, L) - tca * tca;
			float radius2 = sphere.radius * sphere.radius;
			if (d2 > radius2) return false;

			float thc = sqrt(radius2 - d2);
			t0 = tca - thc;
			t1 = tca + thc;

			if (t0 > t1) std::swap(t0, t1);

			if (t0 < ray.min) {
				t0 = t1;  //if t0 is negative, let's use t1 instead 
				if (t0 < ray.min) return false;  //both t0 and t1 are negative 
			}

			if (t0>ray.max)
			{
				return false;
			}

			if (!ignoreHitRecord)
			{
				hitRecord.t = t0;
				hitRecord.didHit = true;
				hitRecord.origin = ray.origin + t0 * ray.direction;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			}
			return true;


			//return false;
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

				if ( t >= ray.min && t <= ray.max)
				{
					if(!ignoreHitRecord)
					{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.materialIndex = plane.materialIndex;
					hitRecord.normal = plane.normal;
					hitRecord.origin = ray.origin + t * ray.direction;
					}
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
#ifdef MOLLER_TRUMBORE
			// M�ller�Trumbore intersection algorithm
			const Vector3 edge1{ triangle.v1 - triangle.v0 };
			const Vector3 edge2{ triangle.v2 - triangle.v0 };

			const Vector3 h{ Vector3::Cross(ray.direction, edge2) };
			const float a{ Vector3::Dot(edge1, h) };

			if (a < -FLT_EPSILON)
			{
				// Backface hit
				if (!ignoreHitRecord && triangle.cullMode == TriangleCullMode::BackFaceCulling)
					// Remove the face if it's "culled" away
					return false;
				if (ignoreHitRecord && triangle.cullMode == TriangleCullMode::FrontFaceCulling)
					// Shadow rays (ignorehitrecord true) have inverted culling
					return false;
			}
			else if (a > FLT_EPSILON)
			{
				// Frontface hit
				if (!ignoreHitRecord && triangle.cullMode == TriangleCullMode::FrontFaceCulling)
					// Remove the face if it's "culled" away
					return false;
				if (ignoreHitRecord && triangle.cullMode == TriangleCullMode::BackFaceCulling)
					// Shadow rays (ignorehitrecord true) have inverted culling
					return false;
			}
			else
			{
				return false;
			}

			const float f{ 1.0f / a };
			const Vector3 s{ ray.origin - triangle.v0 };
			const float u{ f * Vector3::Dot(s, h) };

			if (u < 0.0f || u > 1.0f)
				return false;

			const Vector3 q{ Vector3::Cross(s, edge1) };
			const float v{ f * Vector3::Dot(ray.direction, q) };

			if (v < 0.0f || u + v > 1.0f)
				return false;

			const float t{ f * Vector3::Dot(edge2, q) };
			if (t > ray.min && t < ray.max)
			{
				if (ignoreHitRecord) return true;
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = ray.origin + (t * ray.direction);
				hitRecord.normal = triangle.normal;
				hitRecord.t = t;
				return true;
			}
			return false;
#else
			float dotNormalDirection = Vector3::Dot(triangle.normal, ray.direction);

			if (dotNormalDirection == 0)
			{
				//Ray is parallel with Plane of triangle => no intersection
				return false;
			}

			Vector3 center = (triangle.v0 + triangle.v1 + triangle.v2) * .3333f;
			Vector3 l = center - ray.origin;
			float t = Vector3::Dot(l, triangle.normal) / dotNormalDirection;

			if (t < ray.min || t > ray.max)
			{
				return false;
			}

			Vector3 p = ray.origin + t * ray.direction;

			bool rayHitsFrontSide = (dotNormalDirection < 0) ^ ignoreHitRecord;
			bool hide = (rayHitsFrontSide && triangle.cullMode == TriangleCullMode::FrontFaceCulling) || (!rayHitsFrontSide && triangle.cullMode == TriangleCullMode::BackFaceCulling);

			if (hide)
			{
				return false;
			}
			if (CheckEdge(triangle.v0, triangle.v1, triangle.normal, p)
				&& CheckEdge(triangle.v1, triangle.v2, triangle.normal, p)
				&& CheckEdge(triangle.v2, triangle.v0, triangle.normal, p))
			{

				if (!ignoreHitRecord)
				{
					hitRecord.didHit = true;
					hitRecord.materialIndex = triangle.materialIndex;
					hitRecord.origin = p;
					hitRecord.normal = triangle.normal;
					hitRecord.t = t;
				}
				return true;
			}

			return false;
#endif

		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest

		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			float tx1 = (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x;
			float tx2 = (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x;

			float tmin = std::min(tx1, tx2);
			float tmax = std::max(tx1, tx2);

			float ty1 = (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y;
			float ty2 = (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y;

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			float tz1 = (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z;
			float tz2 = (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z;

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0 && tmax >= tmin;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			if (!SlabTest_TriangleMesh(mesh, ray))
			{
				return false;
			}

			size_t n_indices{ mesh.indices.size() };
			float t_min{ FLT_MAX };
			HitRecord closestHit{};
			size_t normalsIndex{ 0 };

			for (int i0 = 0; i0 < n_indices - 2; i0 += 3)
			{
				Triangle triangle{ mesh.transformedPositions[mesh.indices[i0]], mesh.transformedPositions[mesh.indices[i0 + 1]], mesh.transformedPositions[mesh.indices[i0 + 2]], mesh.transformedNormals[normalsIndex] };
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;

				HitRecord thisTriangleHitRecord{};

				++normalsIndex;

				if (HitTest_Triangle(triangle, ray, thisTriangleHitRecord, ignoreHitRecord))
				{
					if (ignoreHitRecord)
					{
						//No need to find "closer" hits
						return true;
					}

					if (closestHit.t > thisTriangleHitRecord.t)
					{
						closestHit = thisTriangleHitRecord;
					}
				}
			}

			if (!ignoreHitRecord && closestHit.didHit)
			{
				hitRecord = closestHit;
			}

			return closestHit.didHit;
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
			if (light.type == LightType::Directional)
			{
				return  light.color * light.intensity;
			}
			else
			{
				Vector3 diff = (light.origin - target);
				return light.color * (light.intensity / Vector3::Dot(diff, diff));
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

				if (isnan(normal.x))
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