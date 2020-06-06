#pragma once


namespace Storm
{
	DirectX::FXMVECTOR convertToXM(const Storm::Vector3 &trans)
	{
		return DirectX::FXMVECTOR{ trans.x(), trans.y(), trans.z(), 1.f };
	}
}
