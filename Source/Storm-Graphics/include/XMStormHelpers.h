#pragma once


namespace Storm
{
	DirectX::FXMVECTOR convertToXM(const Storm::Vector3 &trans);
	Storm::Vector3 convertToStorm(const DirectX::XMVECTOR &vect);
	DirectX::XMMATRIX makeTransform(const Storm::Vector3 &trans, const Storm::Quaternion &rot);
}
