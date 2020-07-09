#include "CubicSplineKernel.h"


float Storm::CubicSplineKernel::s_rawPrecoeff = 0.f;
float Storm::CubicSplineKernel::s_gradientPrecoeff = 0.f;


void Storm::CubicSplineKernel::initialize(const float kernelLength)
{
	constexpr float k_constexprRawPrecoeffCoeff = static_cast<float>(8.0 / M_PI);
	constexpr float k_constexprGradientPrecoeffCoeff = static_cast<float>(48.0 / M_PI);
	
	const float h3 = kernelLength * kernelLength * kernelLength;

	s_rawPrecoeff = k_constexprRawPrecoeffCoeff / h3;
	s_gradientPrecoeff = k_constexprGradientPrecoeffCoeff / h3;
}

float Storm::CubicSplineKernel::raw(const float k_kernelLength, const float norm)
{
	const float q = norm / k_kernelLength;
	if (q < 0.5f)
	{
		return s_rawPrecoeff * (q * q * (6.f * q - 1.f) + 1.f);
	}

	return s_rawPrecoeff * 2.f * (1.f - q);
}

Storm::Vector3 Storm::CubicSplineKernel::gradient(const float k_kernelLength, const Storm::Vector3 &vectToNeighbor, const float norm)
{
	const float q = norm / k_kernelLength;
	float factor;

	if (q < 0.5f)
	{
		factor = q * (3.f * q - 2.f);
	}
	else
	{
		const float subfactor = 1.f - q;
		factor = -subfactor * subfactor;
	}

	return vectToNeighbor * (s_gradientPrecoeff * factor / (norm * k_kernelLength));
}
