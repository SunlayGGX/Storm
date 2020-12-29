#include "CubicSplineKernel.h"
#include "SplishSplashCubicSplineKernel.h"

#include "Kernel.h"
#include "KernelMode.h"


float Storm::CubicSplineKernel::s_rawPrecoeff = 0.f;
float Storm::CubicSplineKernel::s_gradientPrecoeff = 0.f;
float Storm::CubicSplineKernel::s_kernelZero = 0.f;
float Storm::SplishSplashCubicSplineKernel::s_rawPrecoeff = 0.f;
float Storm::SplishSplashCubicSplineKernel::s_gradientPrecoeff = 0.f;
float Storm::SplishSplashCubicSplineKernel::s_kernelZero = 0.f;


void Storm::CubicSplineKernel::initialize(const float kernelLength)
{
	constexpr float k_constexprRawPrecoeffCoeff = static_cast<float>(8.0 / M_PI);
	constexpr float k_constexprGradientPrecoeffCoeff = static_cast<float>(48.0 / M_PI);
	
	const float h3 = kernelLength * kernelLength * kernelLength;

	s_rawPrecoeff = k_constexprRawPrecoeffCoeff / h3;
	s_gradientPrecoeff = k_constexprGradientPrecoeffCoeff / (h3 * kernelLength);

	s_kernelZero = Storm::CubicSplineKernel::raw(kernelLength, 0.f);
}

float Storm::CubicSplineKernel::raw(const float k_kernelLength, const float norm)
{
	const float q = norm / k_kernelLength;
	if (q < 0.5f)
	{
		return s_rawPrecoeff * (6.f * q * q * (q - 1.f) + 1.f);
	}

	const float oneMinusQ = 1.f - q;
	return s_rawPrecoeff * 2.f * oneMinusQ * oneMinusQ * oneMinusQ;
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

	return vectToNeighbor * (s_gradientPrecoeff * factor / norm);
}

float Storm::CubicSplineKernel::zeroValue()
{
	return s_kernelZero;
}

void Storm::SplishSplashCubicSplineKernel::initialize(const float kernelLength)
{
	constexpr float k_constexprRawPrecoeffCoeff = static_cast<float>(8.0 / M_PI);
	constexpr float k_constexprGradientPrecoeffCoeff = static_cast<float>(48.0 / M_PI);

	const float h3 = kernelLength * kernelLength * kernelLength;

	s_rawPrecoeff = k_constexprRawPrecoeffCoeff / h3;
	s_gradientPrecoeff = k_constexprGradientPrecoeffCoeff / h3;

	s_kernelZero = Storm::CubicSplineKernel::raw(kernelLength, 0.f);
}

float Storm::SplishSplashCubicSplineKernel::raw(const float k_kernelLength, const float norm)
{
	const float q = norm / k_kernelLength;
	if (q < 0.5f)
	{
		return s_rawPrecoeff * (6.f * q * q * (q - 1.f) + 1.f);
	}

	const float oneMinusQ = 1.f - q;
	return s_rawPrecoeff * 2.f * oneMinusQ * oneMinusQ * oneMinusQ;
}

Storm::Vector3 Storm::SplishSplashCubicSplineKernel::gradient(const float k_kernelLength, const Storm::Vector3 &vectToNeighbor, const float norm)
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

	return vectToNeighbor * (s_gradientPrecoeff * factor / norm);
}

float Storm::SplishSplashCubicSplineKernel::zeroValue()
{
	return s_kernelZero;
}

void Storm::initializeKernels(const float kernelLength)
{
	Storm::CubicSplineKernel::initialize(kernelLength);
	Storm::SplishSplashCubicSplineKernel::initialize(kernelLength);
}

Storm::RawKernelMethodDelegate Storm::retrieveRawKernelMethod(const Storm::KernelMode kernelMode)
{
	switch (kernelMode)
	{
	case Storm::KernelMode::CubicSpline: return Storm::CubicSplineKernel::raw;
	case Storm::KernelMode::SplishSplashCubicSpline: return Storm::SplishSplashCubicSplineKernel::raw;
	}

	Storm::throwException<Storm::Exception>("Unknown kernel mode!");
}

Storm::GradKernelMethodDelegate Storm::retrieveGradKernelMethod(const Storm::KernelMode kernelMode)
{
	switch (kernelMode)
	{
	case Storm::KernelMode::CubicSpline: return Storm::CubicSplineKernel::gradient;
	case Storm::KernelMode::SplishSplashCubicSpline: return Storm::SplishSplashCubicSplineKernel::gradient;
	}

	Storm::throwException<Storm::Exception>("Unknown kernel mode!");
}

float Storm::retrieveKernelZeroValue(const Storm::KernelMode kernelMode)
{
	switch (kernelMode)
	{
	case Storm::KernelMode::CubicSpline: return Storm::CubicSplineKernel::zeroValue();
	case Storm::KernelMode::SplishSplashCubicSpline: return Storm::SplishSplashCubicSplineKernel::zeroValue();
	}

	Storm::throwException<Storm::Exception>("Unknown kernel mode!");
}
