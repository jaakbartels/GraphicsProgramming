#include "Scene.h"
#include "Utils.h"
#include "Material.h"

namespace dae {

#pragma region Base Scene
	//Initialize Scene with Default Solid Color Material (RED)
	Scene::Scene():
		m_Materials({ new Material_SolidColor({1,0,0})})
	{
		m_SphereGeometries.reserve(32);
		m_PlaneGeometries.reserve(32);
		m_TriangleMeshGeometries.reserve(32);
		m_Lights.reserve(32);
	}

	Scene::~Scene()
	{
		for(auto& pMaterial : m_Materials)
		{
			delete pMaterial;
			pMaterial = nullptr;
		}

		m_Materials.clear();
	}

	void dae::Scene::GetClosestHit(const Ray& ray, HitRecord& closestHit) const
	{
		//Check the planes
		const size_t planeGeometriesSize{ m_PlaneGeometries.size() };
		for (size_t i{}; i < planeGeometriesSize; ++i)
		{
			HitRecord hitInfo{};
			GeometryUtils::HitTest_Plane(m_PlaneGeometries[i], ray, hitInfo);
			if (hitInfo.t < closestHit.t)
			{
				closestHit = hitInfo;
			}
		}

		// Check the spheres
		const size_t sphereGeometriesSize{ m_SphereGeometries.size() };
		for (size_t i{}; i < sphereGeometriesSize; ++i)
		{
			HitRecord hitInfo{};
			GeometryUtils::HitTest_Sphere(m_SphereGeometries[i], ray, hitInfo);
			if (hitInfo.t < closestHit.t)
			{
				closestHit = hitInfo;
			}
		}
	}

	bool Scene::DoesHit(const Ray& ray) const
	{
		for (int i = 0; i < m_SphereGeometries.size(); ++i)
		{
			if (GeometryUtils::HitTest_Sphere(m_SphereGeometries[i], ray))
			{
				return true;
			}
		}
		for (int i = 0; i < m_PlaneGeometries.size(); ++i)
		{
			if (GeometryUtils::HitTest_Plane(m_PlaneGeometries[i], ray))
			{
				return true;
			}
		}
		return false;
	}

#pragma region Scene Helpers
	Sphere* Scene::AddSphere(const Vector3& origin, float radius, unsigned char materialIndex)
	{
		Sphere s;
		s.origin = origin;
		s.radius = radius;
		s.materialIndex = materialIndex;

		m_SphereGeometries.emplace_back(s);
		return &m_SphereGeometries.back();
	}

	Plane* Scene::AddPlane(const Vector3& origin, const Vector3& normal, unsigned char materialIndex)
	{
		Plane p;
		p.origin = origin;
		p.normal = normal;
		p.materialIndex = materialIndex;

		m_PlaneGeometries.emplace_back(p);
		return &m_PlaneGeometries.back();
	}

	TriangleMesh* Scene::AddTriangleMesh(TriangleCullMode cullMode, unsigned char materialIndex)
	{
		TriangleMesh m{};
		m.cullMode = cullMode;
		m.materialIndex = materialIndex;

		m_TriangleMeshGeometries.emplace_back(m);
		return &m_TriangleMeshGeometries.back();
	}

	Light* Scene::AddPointLight(const Vector3& origin, float intensity, const ColorRGB& color)
	{
		Light l;
		l.origin = origin;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Point;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	Light* Scene::AddDirectionalLight(const Vector3& direction, float intensity, const ColorRGB& color)
	{
		Light l;
		l.direction = direction;
		l.intensity = intensity;
		l.color = color;
		l.type = LightType::Directional;

		m_Lights.emplace_back(l);
		return &m_Lights.back();
	}

	unsigned char Scene::AddMaterial(Material* pMaterial)
	{
		m_Materials.push_back(pMaterial);
		return static_cast<unsigned char>(m_Materials.size() - 1);
	}
#pragma endregion
#pragma endregion

#pragma region SCENE W1
	void Scene_W1::Initialize()
	{
		//default: Material id0 >> SolidColor Material (RED)
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });

		//Spheres
		AddSphere({ -25.f, 0.f, 100.f }, 50.f, matId_Solid_Red);
		AddSphere({ 25.f, 0.f, 100.f }, 50.f, matId_Solid_Blue);

		//Plane
		//AddPlane({ 0.f, -200.f, 0.f }, { 0.f, 0.7071f, 0.7071f }, matId_Solid_Yellow);
		AddPlane({ -75.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 75.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matId_Solid_Green);
		AddPlane({ 0.f, -75.f, 0.f }, { 0.f, 1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 75.f, 0.f }, { 0.f, -1.f,0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f, 0.f, 125.f }, { 0.f, 0.f,-1.f }, matId_Solid_Magenta);
	}
	void Scene_W2::Initialize()
	{
		m_Camera.origin = { 0.f ,3.f , -9.f };
		m_Camera.fovAngle = 45.f;
		// default : Material id0 >> SolidColor Material ( RED )
		constexpr unsigned char matId_Solid_Red = 0;
		const unsigned char matId_Solid_Blue = AddMaterial(new Material_SolidColor{ colors::Blue });
		const unsigned char matId_Solid_Yellow = AddMaterial(new Material_SolidColor{ colors::Yellow });
		const unsigned char matId_Solid_Green = AddMaterial(new Material_SolidColor{ colors::Green });
		const unsigned char matId_Solid_Magenta = AddMaterial(new Material_SolidColor{ colors::Magenta });
		// Plane
		AddPlane({ -5.f ,0.f ,0.f }, { 1.f , 0.f , 0.f }, matId_Solid_Green);
		AddPlane({ 5.f , 0.f , 0.f }, { -1.f , 0.f , 0.f }, matId_Solid_Green);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f , 1.f , 0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f , 10.f , 0.f }, { 0.f, -1.f, 0.f }, matId_Solid_Yellow);
		AddPlane({ 0.f , 0.f , 10.f }, { 0.f, 0.f, -1.f }, matId_Solid_Magenta);
		// Spheres
		AddSphere({ -1.75f , 1.f , 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 0.f ,  1.f , 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 1.75f ,  1.f , 0.f }, .75f, matId_Solid_Red);
		AddSphere({ -1.75f , 3.f, 0.f }, .75f, matId_Solid_Blue);
		AddSphere({ 0.f ,  3.f , 0.f }, .75f, matId_Solid_Red);
		AddSphere({ 1.75f, 3.f, 0.f }, .75f, matId_Solid_Blue);
		// Light
		AddPointLight({ 0.f, 5.f, -5.f }, 70.f, colors::White);
	}

	void Scene_W3::Initialize()
	{
		m_Camera.origin = { 0.f ,3.f , -9.f };
		m_Camera.fovAngle = 45.f;

		//default: Material id0 >> SolidColor Material (RED)
		const ColorRGB wallColor = ColorRGB{ .49f, .57f, .57f };
		const ColorRGB ballPlasticColor = ColorRGB{ .75f, .75f, .75f };
		const ColorRGB ballMetalColor = ColorRGB{ .972f, .960f, .915f };

		const auto matWhiteRoughPlastic = AddMaterial(new Material_CookTorrence(ballPlasticColor, 0.f, 1.f));
		const auto matWhiteMediumPlastic = AddMaterial(new Material_CookTorrence(ballPlasticColor, 0.f, .6f));
		const auto matWhiteSmoothPlastic = AddMaterial(new Material_CookTorrence(ballPlasticColor, 0.f, .1f));
		const auto matSilverRoughMetal = AddMaterial(new Material_CookTorrence(ballMetalColor, 1.f, 1.f));
		const auto matSilverMediumMetal = AddMaterial(new Material_CookTorrence(ballMetalColor, 1.f, .6f));
		const auto matSilverSmoothMetal = AddMaterial(new Material_CookTorrence(ballMetalColor, 1.f, .1f));

		const auto matWall = AddMaterial(new Material_Lambert(wallColor, 1.f));

		//Spheres
		AddSphere({ -1.75f, 3.f, .0f }, .75f, matWhiteRoughPlastic);
		AddSphere({ 0.f, 3.f, .0f }, .75f, matWhiteMediumPlastic);
		AddSphere({ 1.75f, 3.f, .0f }, .75f, matWhiteSmoothPlastic);
		AddSphere({ -1.75f, 1.f, .0f }, .75f, matSilverRoughMetal);
		AddSphere({ 0.f, 1.f, .0f }, .75f, matSilverMediumMetal);
		AddSphere({ 1.75f, 1.f, .0f }, .75f, matSilverSmoothMetal);

		//Plane
		AddPlane({ 0.f, 0.f, 10.f }, { 0.f, 0.f,-1.f }, matWall);
		AddPlane({ 0.f, 0.f, 0.f }, { 0.f, 1.f,0.f }, matWall);
		AddPlane({ 0.f, 10.f, 0.f }, { 0.f, -1.f,0.f }, matWall);
		AddPlane({ 5.f, 0.f, 0.f }, { -1.f, 0.f,0.f }, matWall);
		AddPlane({ -5.f, 0.f, 0.f }, { 1.f, 0.f,0.f }, matWall);

		// Light
		AddPointLight({ 0.f, 5.f, 5.f }, 50.f, ColorRGB{1.f,.61f,.45f});
		AddPointLight({ -2.5f, 5.f, -5.f }, 70.f, ColorRGB{ 1.f,.8f,.45f });
		AddPointLight({ 2.5f, 2.5f, -5.f }, 50.f, ColorRGB{ .34f,.47f,.68f });
	}
#pragma endregion
}
