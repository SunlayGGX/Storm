#pragma once


namespace Storm
{
	class Poly6Kernel
	{
	public:
		Poly6Kernel(float kernelLength);

	public:
		float operator()(float particleDistSquared) const
		{
			const float baseKernelVal = _kernelLengthSquared - particleDistSquared;
			return _precomputedCoeff * baseKernelVal * baseKernelVal * baseKernelVal;
		}

	private:
		float _kernelLengthSquared;
		float _precomputedCoeff;
	};
}
