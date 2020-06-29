#include "Poly6Kernel.h"
#include "SpikyKernel.h"



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
