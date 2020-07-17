#pragma once

namespace Storm
{
	enum class KernelMode;

	using RawKernelMethodDelegate = float(*)(const float k_kernelLength, const float norm);
	using GradKernelMethodDelegate = Storm::Vector3(*)(const float, const Storm::Vector3 &, const float);

	void initializeKernels(const float kernelLength);

	Storm::RawKernelMethodDelegate retrieveRawKernelMethod(const Storm::KernelMode kernelMode);
	Storm::GradKernelMethodDelegate retrieveGradKernelMethod(const Storm::KernelMode kernelMode);
	float retrieveKernelZeroValue(const Storm::KernelMode kernelMode);
}
