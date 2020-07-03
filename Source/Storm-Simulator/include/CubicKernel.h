#pragma once


namespace Storm
{
	// Cubic kernel as presented by "Smoothed Particle Hydrodynamics, Joe J Monaghan, Annual Review of Astronomy and Astrophysics 1992 30:1, 543-574"

	class CubicKernelBase
	{
	public:
		CubicKernelBase(float kernelLength);

	protected:
		const float k_kernelLength;
		float _precoeff;
	};

	class CubicKernel : public Storm::CubicKernelBase
	{
	public:
		using Storm::CubicKernelBase::CubicKernelBase;

	public:
		float operator()(float norm) const
		{
			const float q = norm / k_kernelLength;
			if (q < 1.f)
			{
				return _precoeff * (2.f / 3.f - q * q * (0.5f * q - 1.f));
			}
			else if (q < 2.f)
			{
				const float qCoeff = (2.f - q);
				return _precoeff / 6.f * qCoeff * qCoeff * qCoeff;
			}

			return 0.f;
		}
	};

	class GradientCubicKernel : public Storm::CubicKernelBase
	{
	public:
		GradientCubicKernel(float kernelLength);

	public:
		float operator()(float particleDistToNeighbor) const
		{
			const float q = particleDistToNeighbor / k_kernelLength;
			if (q < 1.f)
			{
				return _precoeff * q * (1.5f * q - 2.f);
			}
			else if (q < 2.f)
			{
				const float qCoeff = (2.f - q);
				return _precoeff / -2.f * qCoeff * qCoeff;
			}

			return 0.f;
		}
	};
}
