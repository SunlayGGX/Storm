#pragma once


namespace Storm
{
	// Cubics Kernel as presented inside SplishSplash engine written by D. Koschier, J. Bender, B. Solenthaler & M. Teschner

	class SplishSplashCubicSplineKernel
	{
	public:
		static void initialize(const float kernelLength);

	public:
		static float raw(const float k_kernelLength, const float norm);
		static Storm::Vector3 gradient(const float k_kernelLength, const Storm::Vector3 &vectToNeighbor, const float norm);
		static float zeroValue();

	private:
		static float s_rawPrecoeff;
		static float s_gradientPrecoeff;
		static float s_kernelZero;
	};
}
