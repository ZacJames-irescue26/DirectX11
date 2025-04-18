#pragma once

namespace Engine
{
	struct Triangle
	{
		Triangle() {}
		Triangle(const XMFLOAT3& v0, const XMFLOAT3& v1, const XMFLOAT3& v2)
			: p0(XMLoadFloat3(&v0)), p1(XMLoadFloat3(&v1)), p2(XMLoadFloat3(&v2))
		{ }
		XMVECTOR p0;
		XMVECTOR p1;
		XMVECTOR p2;
		XMVECTOR n0;
		XMVECTOR n1;
		XMVECTOR n2;
		XMVECTOR t0;
		XMVECTOR t1;
		XMVECTOR t2;
		

		float area() const {
			XMVECTOR cross = XMVector3Cross(p2 - p0, p1 - p0);
			XMVECTOR length = XMVector3Length(cross);
			return XMVectorGetX(length) * 0.5f;
		}

		XMVECTOR normal() const {
			return XMVector3Normalize(XMVector3Cross(p2 - p0, p1 - p0));
		}


	};
}