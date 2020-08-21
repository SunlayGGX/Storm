#pragma once


namespace Storm
{
	DirectX::FXMVECTOR convertToXM(const Storm::Vector3 &trans);
	DirectX::XMMATRIX makeTransform(const Storm::Vector3 &trans, const Storm::Quaternion &rot);
}
