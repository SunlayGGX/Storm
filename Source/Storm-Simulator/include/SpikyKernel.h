#pragma once


namespace Storm
{
	class GradientSpikyKernel
	{
	public:
		GradientSpikyKernel(float kernelLength);

	public:
		float operator()(float particleDistToNeighbor) const
		{
			const float kernelBaseVal = _kernelLength - particleDistToNeighbor;
			return _precomputedCoefficient * kernelBaseVal * kernelBaseVal;
		}

	private:
		const float _kernelLength;
		float _precomputedCoefficient;
	};
}
