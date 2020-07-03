#include "Poly6Kernel.h"
#include "SpikyKernel.h"
#include "CubicKernel.h"
#include "CubicSplineKernel.h"



Storm::Poly6Kernel::Poly6Kernel(float kernelLength) :
	_kernelLengthSquared{ kernelLength * kernelLength }
{
	constexpr const float constexprCoeff = static_cast<float>(315.0 / (64.0 * M_PI));

	// h^4
	const float kernelFourth = _kernelLengthSquared * _kernelLengthSquared;

	// (315 / 64 * pi * h^9)
	_precomputedCoeff = constexprCoeff / (kernelFourth * kernelFourth * kernelLength);
}

Storm::GradientSpikyKernel::GradientSpikyKernel(float kernelLength) :
	_kernelLength{ kernelLength }
{
	const float kernelLengthSquared = _kernelLength * _kernelLength;
	constexpr float constexprCoeff = static_cast<float>(-3.0 * 15.0 * M_1_PI);
	const float h6 = kernelLengthSquared * kernelLengthSquared * kernelLengthSquared;

	_precomputedCoefficient = constexprCoeff / h6;
}

Storm::CubicKernelBase::CubicKernelBase(float kernelLength) :
	k_kernelLength{ kernelLength }
{
	constexpr float k_constexprPrecoeffCoeff = static_cast<float>(2.0 / (3.0 * M_PI));
	_precoeff = k_constexprPrecoeffCoeff / (kernelLength * kernelLength * kernelLength);
}

Storm::GradientCubicKernel::GradientCubicKernel(float kernelLength) :
	Storm::CubicKernelBase{ kernelLength }
{
	_precoeff /= kernelLength;
}

Storm::CubicSplineKernelBase::CubicSplineKernelBase(float kernelLength) :
	k_kernelLength{ kernelLength }
{
	constexpr float k_constexprPrecoeffCoeff = static_cast<float>(8.0 / M_PI);
	_precoeff = k_constexprPrecoeffCoeff / (kernelLength * kernelLength * kernelLength);
}
