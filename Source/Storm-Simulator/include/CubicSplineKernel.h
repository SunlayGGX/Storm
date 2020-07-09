#pragma once


namespace Storm
{
	// Cubics Kernel as presented by SplishSplash tutorial by D. Koschier, J. Bender, B. Solenthaler & M. Teschner
	// https://interactivecomputergraphics.github.io/SPH-Tutorial/pdf/SPH_Tutorial.pdf

	class CubicSplineKernel
	{
	public:
		static void initialize(const float kernelLength);

	public:
		static float raw(const float k_kernelLength, const float norm);
		static Storm::Vector3 gradient(const float k_kernelLength, const Storm::Vector3 &vectToNeighbor, const float norm);

	private:
		static float s_rawPrecoeff;
		static float s_gradientPrecoeff;
	};
}
