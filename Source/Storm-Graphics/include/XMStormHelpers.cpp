#include "XMStormHelpers.h"


DirectX::FXMVECTOR Storm::convertToXM(const Storm::Vector3 &trans)
{
	return DirectX::FXMVECTOR{ trans.x(), trans.y(), trans.z(), 1.f };
}

Storm::Vector3 Storm::convertToStorm(const DirectX::XMVECTOR &vect)
{
	return Storm::Vector3{ vect.m128_f32[0], vect.m128_f32[1], vect.m128_f32[2] };
}

DirectX::XMMATRIX Storm::makeTransform(const Storm::Vector3 &trans, const Storm::Quaternion &rot)
{
	return DirectX::XMMatrixTranspose(DirectX::XMMatrixAffineTransformation(
		DirectX::FXMVECTOR{ 1.f, 1.f, 1.f, 0.f },
		DirectX::FXMVECTOR{ 0.f, 0.f, 0.f, 1.f },
		DirectX::XMVECTOR{ rot.x(), rot.y(), rot.z(), rot.w() },
		Storm::convertToXM(trans)
	));
}
