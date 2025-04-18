#include "pch.h"
#include "AABB.h"

namespace Engine
{

bool AABB::ContainsPoint(XMFLOAT3 point)
{
	
	XMFLOAT3 minFloat;
	XMStoreFloat3(& minFloat, min);
	XMFLOAT3 maxFloat;
	XMStoreFloat3(&maxFloat, max);

	return point.x >= minFloat.x && point.x <= maxFloat.x &&
		point.y >= minFloat.y && point.y <= maxFloat.y &&
		point.z >= minFloat.z && point.z <= maxFloat.z;
}
bool AABB::ContainsPoint(XMVECTOR point)
{

	XMFLOAT3 minFloat;
	XMStoreFloat3(&minFloat, min);
	XMFLOAT3 maxFloat;
	XMStoreFloat3(&maxFloat, max);
	XMFLOAT3 pointFloat;
	XMStoreFloat3(&pointFloat, point);
	return pointFloat.x >= minFloat.x && pointFloat.x <= maxFloat.x &&
		pointFloat.y >= minFloat.y && pointFloat.y <= maxFloat.y &&
		pointFloat.z >= minFloat.z && pointFloat.z <= maxFloat.z;
}
bool AABB::intersect(const Ray& ray, float& tNear, float& tFar) {


	XMVECTOR invDir = XMVectorReciprocal(ray.direction);
	XMVECTOR tMin = (min - ray.origin) * invDir;
	XMVECTOR tMax = (max - ray.origin) * invDir;

	XMFLOAT3 t1; XMStoreFloat3(&t1, XMVectorMin(tMin, tMax));
	XMFLOAT3 t2; XMStoreFloat3(&t2, XMVectorMax(tMin, tMax));



	tNear = std::max(std::max(t1.x, t1.y), t1.z);
	tFar = std::min(std::min(t2.x, t2.y), t2.z);

	return tNear <= tFar && tFar > 0.0f;
}
bool AABB::ContainsTriangle(Triangle triangle)
{
	

	return ContainsPoint(triangle.p0) && ContainsPoint(triangle.p1) && ContainsPoint(triangle.p2);

}

bool AABB::ContainsAABB(AABB aabb)
{
	return ContainsPoint(aabb.min) && ContainsPoint(aabb.max);
}
bool AABB::OverlappingwithSphere(const XMFLOAT3& c, float radius)
{

	
	// Find the closest point on the AABB to the sphere's center
	float closestX = std::max(minf.x, std::min(c.x, maxf.x));
	float closestY = std::max(minf.y, std::min(c.y, maxf.y));
	float closestZ = std::max(minf.z, std::min(c.z, maxf.z));

	// Compute the squared distance between the sphere's center and the closest point
	float distanceSquared = (c.x - closestX) * (c.x - closestX) +
		(c.y - closestY) * (c.y - closestY) +
		(c.z - closestZ) * (c.z - closestZ);

	// Check if the squared distance is less than or equal to the sphere's radius squared
	return distanceSquared <= (radius * radius);
}
XMVECTOR AABB::Center()
{
	return (min + max) / 2.0f;
}

std::array<Engine::AABB*, 8> AABB::SplitIntoOct()
{
	XMVECTOR c = Center();

	XMFLOAT3 minf;
	XMStoreFloat3(&minf, min);
	XMFLOAT3 maxf;
	XMStoreFloat3(&maxf, max);
	XMFLOAT3 cf;
	XMStoreFloat3(&cf, c);

	return { new AABB{minf, cf},
			new AABB{XMFLOAT3{cf.x, minf.y, minf.z},XMFLOAT3 {maxf.x, cf.y, cf.z}},
			new AABB{XMFLOAT3{minf.x, cf.y, minf.z},XMFLOAT3 {cf.x, maxf.y, cf.z}},
			new AABB{XMFLOAT3{cf.x, cf.y, minf.z}, XMFLOAT3{maxf.x, maxf.y, cf.z}},
			new AABB{XMFLOAT3{minf.x, minf.y, cf.z},XMFLOAT3 {cf.x, cf.y, maxf.z}},
			new AABB{XMFLOAT3{cf.x, minf.y, cf.z}, XMFLOAT3{maxf.x, cf.y, maxf.z} },
			new AABB{XMFLOAT3{minf.x, cf.y, cf.z}, XMFLOAT3{cf.x, maxf.y, maxf.z}},
			new AABB{cf, maxf} };
}

//void AABB::CreateDebugBox()
//{
//	glm::vec3 GREEN = glm::vec3(0.0, 1.0, 0.0);
//	std::array<DebugLine*, 12> Lines = {
//	new DebugLine {{ max.x, min.y, min.z }, { max.x, min.y, max.z }, GREEN},// 12
//	new DebugLine {{ max.x, min.y, max.z }, { min.x, min.y, max.z }, GREEN},// 24
//	new DebugLine {{ min.x, min.y, max.z }, { min.x, min.y, min.z }, GREEN},// 43
//	new DebugLine {{ min.x, min.y, min.z }, { max.x, min.y, min.z }, GREEN},// 31
//
//	new DebugLine {{ max.x, max.y, min.z }, { max.x, max.y, max.z }, GREEN},// 78
//	new DebugLine {{ max.x, max.y, max.z }, { min.x, max.y, max.z }, GREEN},// 86
//	new DebugLine {{ min.x, max.y, max.z }, { min.x, max.y, min.z }, GREEN},// 65
//	new DebugLine {{ min.x, max.y, min.z }, { max.x, max.y, min.z }, GREEN},// 57
//
//	new DebugLine {{ max.x, min.y, min.z }, { max.x, max.y, min.z }, GREEN},// 17
//	new DebugLine {{ max.x, min.y, max.z }, { max.x, max.y, max.z }, GREEN},// 28
//	new DebugLine {{ min.x, min.y, max.z }, { min.x, max.y, max.z }, GREEN},// 46
//	new DebugLine {{ min.x, min.y, min.z }, { min.x, max.y, min.z }, GREEN}// 35
//	};
//
//	/*
//			1		2
//		   /-------/
//		  /       /|
//		 /       / |
//   min 3/-------/4 |
//		|  7    |  |8 max
//		|       | /
//	   5|-------|/6
//					*/
//
//					/* iterate through your array */
//	for (int i = 0; i < 12; i++)
//	{
//		/* set the value for your array */
//		this->DebugLines[i] = Lines[i];
//	}
//}

AABB& AABB::Combine(const AABB& currentBounds, const AABB& addingbounds)
{

	XMVECTOR min = XMVectorMin(currentBounds.min, addingbounds.min);
	XMVECTOR max = XMVectorMax(currentBounds.max, addingbounds.max);
	AABB aabb(min, max);
	return aabb;
}

//void AABB::DebugDraw(Program* shader, const glm::mat4& viewproj)
//{
//
//	for (int i = 0; i < DebugLines.size(); i++)
//	{
//		DebugLines[i]->Draw(shader, viewproj);
//	}
//}

void AABB::extend(const XMFLOAT3& vertex)
{
	
	min = XMVectorMin(min, XMLoadFloat3(&vertex));
	max = XMVectorMax(max, XMLoadFloat3(&vertex));
	XMStoreFloat3(&minf, min);
	XMStoreFloat3(&maxf, max);

}

}