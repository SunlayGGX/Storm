// StormPOC-ViscosityFormula.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "StormHelperPrerequisite.h"

#include "StormPathHelper.h"

#include <fstream>
#include <filesystem>
#include <string>
#include <cmath>
#include <corecrt_math_defines.h>


#define STORM_POC_XMACRO_KERNELS																															\
	STORM_POC_XMACRO_KERNEL_ELEM(SplishSplashCubicSpline, gradientSplishSplashCubicSpline, computeSplishSplashCubicSplinePrecoeff)							\
	STORM_POC_XMACRO_KERNEL_ELEM(JJMonaghanCubicSpline, gradientJJMonaghanCubicSpline, computeJJMonaghanCubicSplinePrecoeff)								\
	STORM_POC_XMACRO_KERNEL_ELEM(JJMonaghanCubicSplineClamped, gradientJJMonaghanCubicSplineClamped, computeJJMonaghanCubicSplinePrecoeffClamped)			\


namespace
{
	struct Vector
	{
	public:
		float normSquared() const noexcept
		{
			return _x * _x + _y * _y + _z * _z;
		}

		float norm() const noexcept
		{
			return std::sqrtf(this->normSquared());
		}

	public:
		float dot(const Vector &vect) const noexcept
		{
			return _x * vect._x + _y * vect._y + _z * vect._z;
		}

	public:
		float _x = 0.f;
		float _y = 0.f;
		float _z = 0.f;
	};


	Vector operator*(const float coeff, const Vector &other) noexcept
	{
		return Vector{ other._x * coeff, other._y * coeff, other._z * coeff };
	}

	Vector operator*(const Vector &other, const float coeff) noexcept
	{
		return coeff * other;
	}

	Vector operator-(const Vector &left, const Vector &right) noexcept
	{
		return Vector{ left._x - right._x, left._y - right._y, left._z - right._z };
	}

	enum class KernelFunc
	{
#define STORM_POC_XMACRO_KERNEL_ELEM(Mode, Func, PrecoeffInit) Mode,
		STORM_POC_XMACRO_KERNELS
#undef STORM_POC_XMACRO_KERNEL_ELEM
	};

	class Kernel
	{
	public:
		Kernel(float h, KernelFunc func) :
			_h{ h },
			_hSquared{ h * h },
			_func{ func }
		{
			switch (_func)
			{
#define STORM_POC_XMACRO_KERNEL_ELEM(Mode, Func, PrecoeffInitFunc) case KernelFunc::Mode: _gradientCoeff = this->PrecoeffInitFunc(); break;
				STORM_POC_XMACRO_KERNELS
#undef STORM_POC_XMACRO_KERNEL_ELEM

			default:
				throw std::exception{ "Unknown kernel function" };
			}
		}

	public:
		Vector gradient(const Vector &xji, const float norm) const
		{
			switch (_func)
			{
#define STORM_POC_XMACRO_KERNEL_ELEM(Mode, Func, PrecoeffInit) case KernelFunc::Mode: return this->Func(xji, norm);
				STORM_POC_XMACRO_KERNELS
#undef STORM_POC_XMACRO_KERNEL_ELEM

			default:
				__assume(false);
			}
		}

	private:
		float computeSplishSplashCubicSplinePrecoeff() const noexcept
		{
			constexpr float k_constexprGradientPrecoeffCoeff = static_cast<float>(48.0 / M_PI);
			return k_constexprGradientPrecoeffCoeff / (_hSquared * _hSquared);
		}

		float computeJJMonaghanCubicSplinePrecoeff() const noexcept
		{
			constexpr float k_constexprGradientPrecoeffCoeff = static_cast<float>(-3.0 / 4.0 / M_PI);
			return k_constexprGradientPrecoeffCoeff / (_hSquared * _hSquared);
		}

		constexpr float computeJJMonaghanCubicSplinePrecoeffClamped() const noexcept
		{
			// 0.02 / 2 * cs� where cs is the maximum speed of a particle. Since we're making and controlling the data ourself, we know the maximum speed is 1.0 (vj speed).
			// The division by 2 is because I simplified the kernel by removing a 1/2 and passing it to the precoeff instead.
			constexpr float k_constexprGradientPrecoeffCoeff = static_cast<float>(0.01 * 1.0 * 1.0);
			return k_constexprGradientPrecoeffCoeff;
		}

	private:
		Vector gradientSplishSplashCubicSpline(const Vector &xji, const float norm) const
		{
			Vector result;

			if (norm <= _h)
			{
				const float q = norm / _h;
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

				result = xji * (_gradientCoeff * factor / norm);
			}

			return result;
		}

		Vector gradientJJMonaghanCubicSpline(const Vector &xji, const float norm) const
		{
			Vector result;

			if (norm <= _h)
			{
				// The multiplication by 2 is because JJ Monaghan kernel (named JJk) is defined between 0 and 2 * a normal neighbor kernel length,
				// But we consider our kernel length _h to be a normal neighborhood kernel, therefore to represent the total range of JJk.
				// In another word, JJk considers the kernel length _h to be halved.
				// Then with a variable change (or variable substitution) :
				// JJk = _h / 2
				// q = norm / JJk	=>	q = norm / (_h / 2)		=>	q = norm * 2 / _h
				const float q = norm * 2.f / _h;
				float factor;

				if (q < 1.f)
				{
					factor = q * (4.f - 3.f * q);
				}
				else /*if (q < 2.f)*/ // q would always be under 2.f because it is per construction from the variable change
				{
					const float twoMinusQ = 2.f - q;
					factor = twoMinusQ * twoMinusQ;
				}

				result = xji * -(factor / norm);
			}

			return result;
		}


		Vector gradientJJMonaghanCubicSplineClamped(const Vector &xji, const float norm) const
		{
			Vector result;

			if (norm <= _h)
			{
				// The multiplication by 2 is because JJ Monaghan kernel (named JJk) is defined between 0 and 2 * a normal neighbor kernel length,
				// But we consider our kernel length _h to be a normal neighborhood kernel, therefore to represent the total range of JJk.
				// In another word, JJk considers the kernel length _h to be halved.
				// Then with a variable change (or variable substitution) :
				// JJk = _h / 2
				// q = norm / JJk	=>	q = norm / (_h / 2)		=>	q = norm * 2 / _h
				const float q = norm * 2.f / _h;
				float factor;

				constexpr float twoThird = 2.f / 3.f;

				if (q < twoThird)
				{
					// 2.0 was forwarded to the precoeff.
					factor = 2.f * twoThird;
				}
				else if (q < 1.f)
				{
					factor = q * (4.f - 3.f * q);
				}
				else /*if (q < 2.f)*/ // q would always be under 2.f because it is per construction from the variable change
				{
					const float twoMinusQ = 2.f - q;
					factor = twoMinusQ * twoMinusQ;
				}

				result = xji * (factor / norm);
			}

			return result;
		}

	public:
		float _h;
		float _hSquared;

		float _gradientCoeff;
		const KernelFunc _func;
	};

	class Particle
	{
	public:
		Particle(float x, float y, float z) :
			_position{ x, y, z }
		{}

	public:
		Vector _position;
		Vector _velocity;
	};

	class Viscosity
	{
	public:
		Viscosity(const std::filesystem::path &csvFileName, float h, KernelFunc func) :
			_kernel{ h, func },
			_csvFile{ csvFileName }
		{
			Viscosity::reserveCsvData(_rValues, "r");
			Viscosity::reserveCsvData(_normValues, "norm");
			Viscosity::reserveCsvData(_normCoeffValues, "dotCoeff");
			Viscosity::reserveCsvData(_xjValues, "xj");
			Viscosity::reserveCsvData(_kernelValues, "kernelGrad");
		}

		~Viscosity()
		{
			this->writeCsvData(_rValues);
			this->writeCsvData(_xjValues);
			this->writeCsvData(_normValues);
			this->writeCsvData(_normCoeffValues);
			this->writeCsvData(_kernelValues);
		}

	public:
		__forceinline static void appendCsvData(std::string &inOutCsvLine, const float value)
		{
			inOutCsvLine += std::to_string(value);
			inOutCsvLine += ',';
		}

		static void reserveCsvData(std::string &inOutCsvLine, const std::string_view &dataName)
		{
			inOutCsvLine.reserve(256 + dataName.size());
			inOutCsvLine += dataName;
			inOutCsvLine += ',';
		}

		void writeCsvData(std::string &inOutCsvLine)
		{
			// Remove trailing commas if any
			while (inOutCsvLine.back() == ',')
			{
				inOutCsvLine.pop_back();
			}

			_csvFile << inOutCsvLine << '\n';
		}

	public:
		void operator()(const Particle &pi, const Particle &pj)
		{
			const Vector xij = pi._position - pj._position;
			const Vector vij = pi._velocity - pj._velocity;

			const float xijNormSquared = xij.normSquared();
			const float xijNorm = xij.norm();
			const float vijDotXij = vij.dot(xij);

			const float dotCoeff = vijDotXij / (xijNormSquared + 0.01f * _kernel._hSquared);

			const Vector gradValue = _kernel.gradient(-1.f * xij, xijNorm);

			// This is not the real viscosity force. I removed the constant parts that are just here to confuse.
			// The real value I want a graph of is the part of the formula with the kernel and the dot product.
			const Vector forceApprox = gradValue * dotCoeff;

			Viscosity::appendCsvData(_rValues, xijNorm);
			Viscosity::appendCsvData(_xjValues, pj._position._x);
			Viscosity::appendCsvData(_normCoeffValues, dotCoeff);
			Viscosity::appendCsvData(_normValues, forceApprox.norm());
			Viscosity::appendCsvData(_kernelValues, gradValue.norm());
		}

	public:
		const Kernel _kernel;

		std::string _rValues;
		std::string _normValues;
		std::string _normCoeffValues;
		std::string _xjValues;
		std::string _kernelValues;

		std::ofstream _csvFile;
	};

	std::filesystem::path initPOC(int argc, char* argv[])
	{
		std::filesystem::path stormRootPath = Storm::StormPathHelper::findStormRootPath(argv[0]);

#if defined(_DEBUG) || defined(DEBUG)
		std::filesystem::current_path(stormRootPath / "bin" / "Debug");
#else
		std::filesystem::current_path(stormRootPath / "bin" / "Release");
#endif

		std::filesystem::path currentTempPath = stormRootPath / "Intermediate" / "POC" / "ViscosityFormulaTest";
		std::filesystem::create_directories(currentTempPath);

		return currentTempPath;
	}

	void exec(const std::filesystem::path &tempFolderPathToWriteResult, const std::string_view &csvName, const KernelFunc func)
	{
		// Radius of r=5m
		Viscosity viscosity{ tempFolderPathToWriteResult / csvName, 5.f, func };

		// Origin particle set at { 0.f, 0.f, 0.f }
		Particle pi{ 0.f, 0.f, 0.f };

		// Moving particle set on the circle of r=5m.
		// The equation of a circle is x� + y� + z� = r�...
		// The max x is when y = 0 & z = 0 => x�=r� => x=r => x=5,
		// therefore, I can initialize the position of pj with x C [-5, 5].
		// I choose x=4.75m and z=0m => x�+y�+z�=r� => x�+y�+0�=5� => y�=25-x�-0 => y=sqrt(25-x�) or y=-sqrt(25-x�), I choose the positive one
		constexpr float chosenXStart = 4.75f;
		constexpr float chosenXEnd = -chosenXStart;
		Particle pj{ chosenXStart, std::sqrtf(25.f - (chosenXStart * chosenXStart)), 0.f };

		// Pj will have a velocity of { -1.f, 0.f, 0.f }, therefore will pass parallel to the x axis.
		pj._velocity = Vector{ -1.f, 0.f, 0.f };

		const float _xStep = 0.001f;
		while (pj._position._x > chosenXEnd)
		{
			viscosity(pi, pj);
			pj._position._x -= _xStep;
		}
	}
}

int main(int argc, char* argv[])
{
	const std::filesystem::path tempFolderPathToWriteResult = initPOC(argc, argv);

#define STORM_POC_XMACRO_KERNEL_ELEM(Mode, Func, PrecoeffInit) exec(tempFolderPathToWriteResult, "visco" #Mode ".csv", KernelFunc::Mode);
	STORM_POC_XMACRO_KERNELS
#undef STORM_POC_XMACRO_KERNEL_ELEM

	return 0;
}

