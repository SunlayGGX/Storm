#pragma once


namespace Storm
{
	DirectX::FXMVECTOR convertToXM(const Storm::Vector3 &trans);
	Storm::Vector3 convertToStorm(const DirectX::XMVECTOR &vect);
	Storm::Vector3 convertToStorm(const DirectX::XMFLOAT3 &vect);
	DirectX::XMMATRIX makeTransform(const DirectX::XMVECTOR &trans, const Storm::Quaternion &rot, const Storm::Vector3 &scaling);
	DirectX::XMMATRIX makeTransform(const Storm::Vector3 &trans, const Storm::Quaternion &rot, const Storm::Vector3 &scaling);
	DirectX::XMMATRIX makeTransform(const Storm::Vector3 &trans, const Storm::Quaternion &rot);
}
