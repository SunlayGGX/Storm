// StormPOC-ViscosityFormula.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "StormHelperPrerequisite.h"

#include "StormPathHelper.h"

#include "StormMacro.h"

#include "CSVWriter.h"

#include "SingletonAllocator.h"
#include "SingletonHolder.h"
#include "Singleton.h"
#include "ILoggerManager.h"

#include "LogHelper.h"
#include "LogLevel.h"

#include <fstream>
#include <filesystem>
#include <string>
#include <cmath>
#include <corecrt_math_defines.h>
#include <iostream>


#define STORM_POC_XMACRO_KERNELS																															\
	STORM_POC_XMACRO_KERNEL_ELEM(SplishSplashCubicSpline, gradientSplishSplashCubicSpline, computeSplishSplashCubicSplinePrecoeff)							\
	STORM_POC_XMACRO_KERNEL_ELEM(JJMonaghanCubicSpline, gradientJJMonaghanCubicSpline, computeJJMonaghanCubicSplinePrecoeff)								\
	STORM_POC_XMACRO_KERNEL_ELEM(JJMonaghanCubicSplineClamped, gradientJJMonaghanCubicSplineClamped, computeJJMonaghanCubicSplinePrecoeffClamped)			\


namespace
{
	using ScalarValue = double;

	class LoggerManager final :
		private Storm::Singleton<LoggerManager>,
		public Storm::ILoggerManager
	{
		STORM_DECLARE_SINGLETON(LoggerManager);

	public:
		void log(const std::string_view &moduleName, Storm::LogLevel level, const std::string_view &function, const int line, std::string &&msg) final override;
		Storm::LogLevel getLogLevel() const final override;
		void logToTempFile(const std::string &fileName, const std::string &msg) const final override;
	};


	LoggerManager::LoggerManager() = default;
	LoggerManager::~LoggerManager() = default;

	void LoggerManager::log(const std::string_view & moduleName, Storm::LogLevel level, const std::string_view & function, const int line, std::string && msg)
	{
		const std::string_view levelStr = Storm::parseLogLevel(level);

		std::string totalMsg;
		totalMsg.reserve(16 + function.size() + moduleName.size() + msg.size() + levelStr.size());

		totalMsg += "[";
		totalMsg += levelStr;
		totalMsg += "][";
		totalMsg += moduleName;
		totalMsg += "][";
		totalMsg += function;
		totalMsg += " (";
		totalMsg += std::to_string(line);
		totalMsg += ")]: ";
		totalMsg += msg;
		totalMsg += "\n";

		std::cout << totalMsg;
	}

	Storm::LogLevel LoggerManager::getLogLevel() const
	{
		// No log level, we will log everything.
		return Storm::LogLevel::Debug;
	}

	void LoggerManager::logToTempFile(const std::string & fileName, const std::string & msg) const
	{
		STORM_NOT_IMPLEMENTED;
	}

	using SingletonAllocatorAlias = Storm::SingletonAllocator<
		Storm::SingletonHolder,
		LoggerManager
	>;

	SingletonAllocatorAlias g_singletonMaker;

	struct Vector
	{
	public:
		ScalarValue normSquared() const noexcept
		{
			return _x * _x + _y * _y + _z * _z;
		}

		ScalarValue norm() const noexcept
		{
			return std::sqrt(this->normSquared());
		}

	public:
		ScalarValue dot(const Vector &vect) const noexcept
		{
			return _x * vect._x + _y * vect._y + _z * vect._z;
		}

	public:
		ScalarValue _x = 0.f;
		ScalarValue _y = 0.f;
		ScalarValue _z = 0.f;
	};


	Vector operator*(const ScalarValue coeff, const Vector &other) noexcept
	{
		return Vector{ other._x * coeff, other._y * coeff, other._z * coeff };
	}

	Vector operator*(const Vector &other, const ScalarValue coeff) noexcept
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
		Kernel(ScalarValue h, KernelFunc func) :
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
		Vector gradient(const Vector &xji, const ScalarValue norm) const
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
		ScalarValue computeSplishSplashCubicSplinePrecoeff() const noexcept
		{
			constexpr ScalarValue k_constexprGradientPrecoeffCoeff = static_cast<ScalarValue>(48.0 / M_PI);
			return k_constexprGradientPrecoeffCoeff / (_hSquared * _hSquared);
		}

		ScalarValue computeJJMonaghanCubicSplinePrecoeff() const noexcept
		{
			constexpr ScalarValue k_constexprGradientPrecoeffCoeff = static_cast<ScalarValue>(-3.0 / 4.0 / M_PI);
			return k_constexprGradientPrecoeffCoeff / (_hSquared * _hSquared);
		}

		constexpr ScalarValue computeJJMonaghanCubicSplinePrecoeffClamped() const noexcept
		{
			// 0.02 / 2 * cs² where cs is the maximum speed of a particle. Since we're making and controlling the data ourself, we know the maximum speed is 1.0 (vj speed).
			// The division by 2 is because I simplified the kernel by removing a 1/2 and passing it to the precoeff instead.
			constexpr ScalarValue k_constexprGradientPrecoeffCoeff = static_cast<ScalarValue>(0.01 * 1.0 * 1.0);
			return k_constexprGradientPrecoeffCoeff;
		}

	private:
		Vector gradientSplishSplashCubicSpline(const Vector &xji, const ScalarValue norm) const
		{
			Vector result;

			if (norm <= _h)
			{
				const ScalarValue q = norm / _h;
				ScalarValue factor;

				if (q < 0.5f)
				{
					factor = q * (3.f * q - 2.f);
				}
				else
				{
					const ScalarValue subfactor = 1.f - q;
					factor = -subfactor * subfactor;
				}

				result = xji * (_gradientCoeff * factor / norm);
			}

			return result;
		}

		Vector gradientJJMonaghanCubicSpline(const Vector &xji, const ScalarValue norm) const
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
				const ScalarValue q = norm * 2.f / _h;
				ScalarValue factor;

				if (q < 1.f)
				{
					factor = q * (4.f - 3.f * q);
				}
				else /*if (q < 2.f)*/ // q would always be under 2.f because it is per construction from the variable change
				{
					const ScalarValue twoMinusQ = 2.f - q;
					factor = twoMinusQ * twoMinusQ;
				}

				result = xji * -(factor / norm);
			}

			return result;
		}


		Vector gradientJJMonaghanCubicSplineClamped(const Vector &xji, const ScalarValue norm) const
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
				const ScalarValue q = norm * 2.f / _h;
				ScalarValue factor;

				constexpr ScalarValue twoThird = 2.f / 3.f;

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
					const ScalarValue twoMinusQ = 2.f - q;
					factor = twoMinusQ * twoMinusQ;
				}

				result = xji * (factor / norm);
			}

			return result;
		}

	public:
		ScalarValue _h;
		ScalarValue _hSquared;

		ScalarValue _gradientCoeff;
		const KernelFunc _func;
	};

	class Particle
	{
	public:
		Particle(ScalarValue x, ScalarValue y, ScalarValue z) :
			_position{ x, y, z }
		{}

	public:
		Vector _position;
		Vector _velocity;
	};

	struct CSVConstants
	{
	public:
		inline static const std::string k_xijNormStr = "r";
		inline static const std::string k_anglesStr = "angles";
		inline static const std::string k_xjStr = "xj";
		inline static const std::string k_dotCoeffStr = "dotCoeff";
		inline static const std::string k_normStr = "norm";
		inline static const std::string k_kernelGradStr = "kernelGrad";
	};

	class Viscosity
	{
	public:
		Viscosity(const std::filesystem::path &csvFileName, ScalarValue h, KernelFunc func) :
			_kernel{ h, func },
			_csv{ csvFileName.string() },
			_monaghanStdOffset{ 0.01f * h * h }
		{

		}

	public:
		void operator()(const Particle &pi, const Particle &pj)
		{
			const Vector xij = pi._position - pj._position;
			const Vector vij = pi._velocity - pj._velocity;

			const ScalarValue xijNormSquared = xij.normSquared();
			const ScalarValue xijNorm = xij.norm();
			const ScalarValue vijDotXij = vij.dot(xij);

			const ScalarValue dotCoeff = vijDotXij / (xijNormSquared + 0.01f * _kernel._hSquared);

			const Vector gradValue = _kernel.gradient(-1.f * xij, xijNorm);

			// This is not the real viscosity force. I removed the constant parts that are just here to confuse.
			// The real value I want a graph of is the part of the formula with the kernel and the dot product.
			const Vector forceApprox = gradValue * dotCoeff;

			const ScalarValue forceNorm = 10.f * forceApprox.norm() / (xijNormSquared + _monaghanStdOffset);

			constexpr ScalarValue radToDegreeCoeff = static_cast<ScalarValue>(180.0 / M_PI);

			_csv(CSVConstants::k_xijNormStr, xijNorm);
			_csv(CSVConstants::k_anglesStr, std::sin(pj._position._x / xijNorm) * radToDegreeCoeff);
			_csv(CSVConstants::k_xjStr, pj._position._x);
			_csv(CSVConstants::k_dotCoeffStr, dotCoeff);
			_csv(CSVConstants::k_normStr, forceNorm);
			_csv(CSVConstants::k_kernelGradStr, gradValue.norm());
		}

	public:
		const Kernel _kernel;

		Storm::CSVWriter _csv;
		const ScalarValue _monaghanStdOffset;
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

	void exec(const std::filesystem::path &tempFolderPathToWriteResult, const std::string &csvName, const KernelFunc func, unsigned int layer)
	{
		constexpr ScalarValue h = 5.f;
		constexpr ScalarValue particleRadius = h / 4.5f;

		assert(layer < 2 && "If layer is equal or greater than 2, particle won't be in the influence radius if the kernel.");

		// Radius of r=5m
		Viscosity viscosity{ tempFolderPathToWriteResult / csvName, h, func };

		// Origin particle set at { 0.f, 0.f, 0.f }
		Particle pi{ 0.0, 0.0, 0.0 };

		// Moving particle set on the circle of r=5m.
		// The equation of a circle is x² + y² + z² = r²...
		// The max x is when y = 0 & z = 0 => x²=r² => x=r => x=5,
		// therefore, I can initialize the position of pj with x C [-5, 5].
		// I choose y = (layer + 1) * 2 * particleRadius, and z=0m => x²+y²+z²=r² => x²=25-y²-0 => x_max=sqrt(25 - ((layer + 1) * 2 * particleRadius)²) and x_min=-x_max.
		const ScalarValue y = static_cast<ScalarValue>(layer + 1) * (2.f * particleRadius);
		const ScalarValue xMax = std::sqrt(25.0 - y * y);
		const ScalarValue xMin = -xMax;

		Particle pj{ xMin, y, 0.0 };

		// Pj will have a velocity of { -1.f, 0.f, 0.f }, therefore will pass parallel to the x axis.
		pj._velocity = Vector{ 1.0, 0.0, 0.0 };

		const ScalarValue _xStep = 0.0005f;
		while (pj._position._x < xMax)
		{
			viscosity(pi, pj);
			pj._position._x += _xStep;
		}
	}
}

int main(int argc, char* argv[])
{
	const std::filesystem::path tempFolderPathToWriteResult = initPOC(argc, argv);

	for (unsigned int iter = 0; iter < 3; ++iter)
	{
#define STORM_POC_XMACRO_KERNEL_ELEM(Mode, Func, PrecoeffInit) exec(tempFolderPathToWriteResult, "visco" #Mode "_" + std::to_string(iter) + ".csv", KernelFunc::Mode, iter);
		STORM_POC_XMACRO_KERNELS
#undef STORM_POC_XMACRO_KERNEL_ELEM
	}

	return 0;
}

