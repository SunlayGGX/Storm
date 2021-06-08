#include "XMStormHelpers.h"


namespace
{
	__forceinline DirectX::XMMATRIX makeTransformImpl(const DirectX::XMVECTOR &trans, const DirectX::XMVECTOR &rot, const DirectX::XMVECTOR &scaling)
	{
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixAffineTransformation(
			scaling,
			DirectX::FXMVECTOR{ 0.f, 0.f, 0.f, 1.f },
			rot,
			trans
		));
	}
}


DirectX::FXMVECTOR Storm::convertToXM(const Storm::Vector3 &trans)
{
	return DirectX::FXMVECTOR{ trans.x(), trans.y(), trans.z(), 1.f };
}

Storm::Vector3 Storm::convertToStorm(const DirectX::XMVECTOR &vect)
{
	return Storm::Vector3{ vect.m128_f32[0], vect.m128_f32[1], vect.m128_f32[2] };
}

Storm::Vector3 Storm::convertToStorm(const DirectX::XMFLOAT3 &vect)
{
	return Storm::Vector3{ vect.x, vect.y, vect.z };
}

DirectX::XMMATRIX Storm::makeTransform(const Storm::Vector3 &trans, const Storm::Quaternion &rot, const Storm::Vector3 &scaling)
{
	return makeTransformImpl(Storm::convertToXM(trans), DirectX::XMVECTOR{ rot.x(), rot.y(), rot.z(), rot.w() }, Storm::convertToXM(scaling));
}

DirectX::XMMATRIX Storm::makeTransform(const Storm::Vector3 &trans, const Storm::Quaternion &rot)
{
	return makeTransformImpl(Storm::convertToXM(trans), DirectX::XMVECTOR{ rot.x(), rot.y(), rot.z(), rot.w() }, DirectX::FXMVECTOR{ 1.f, 1.f, 1.f, 0.f });
}

DirectX::XMMATRIX Storm::makeTransform(const DirectX::XMVECTOR &trans, const Quaternion &rot, const Storm::Vector3 &scaling)
{
	return makeTransformImpl(trans, DirectX::XMVECTOR{ rot.x(), rot.y(), rot.z(), rot.w() }, Storm::convertToXM(scaling));
}
