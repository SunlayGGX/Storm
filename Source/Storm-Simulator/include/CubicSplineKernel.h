#pragma once


namespace Storm
{
	class CubicSplineKernelBase
	{
	public:
		CubicSplineKernelBase(float kernelLength);

	protected:
		const float k_kernelLength;
		float _precoeff;
	};

	class CubicSplineKernel : public Storm::CubicSplineKernelBase
	{
	public:
		using Storm::CubicSplineKernelBase::CubicSplineKernelBase;

	public:
		float operator()(float norm) const
		{
			const float q = norm / k_kernelLength;
			if (q <= 0.5f)
			{
				return _precoeff * (q * q * (6.f * q - 1.f) + 1.f);
			}

			return _precoeff * 2.f * (1.f - q);
		}
	};

	class GradientCubicSplineKernel : public Storm::CubicSplineKernelBase
	{
	public:
		using Storm::CubicSplineKernelBase::CubicSplineKernelBase;

	public:
		float operator()(float particleDistToNeighbor) const
		{
			const float q = particleDistToNeighbor / k_kernelLength;
			if (q <= 0.5f)
			{
				return _precoeff * (18.f * q * q - 12.f * q);
			}

			const float oneMinusQ = 1.f - q;
			return _precoeff * -6.f * oneMinusQ * oneMinusQ;
		}
	};
}
