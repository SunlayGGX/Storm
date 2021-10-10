// Storm-MaterialAvailability.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "InstructionSet.h"
#include "ExitCode.h"
#include "RAII.h"
#include "StaticAssertionsMacros.h"
#include "StormMacro.h"

#include <iostream>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#	include <Windows.h>
#undef NOMINMAX
#undef WIN32_LEAN_AND_MEAN

namespace
{
	using PairedSupportedElement = std::pair<std::string, char>;

	template<std::size_t index, std::size_t count, class ... Args>
	void extractCapabilities(PairedSupportedElement(&inOutExtracted)[count], const std::string_view feature, const bool isSupported, const Args &... args)
	{
		extractCapabilities<index>(inOutExtracted, feature, isSupported);
		extractCapabilities<index + 1>(inOutExtracted, args...);
	}

	template<std::size_t index, std::size_t count>
	void extractCapabilities(PairedSupportedElement(&inOutExtracted)[count], const std::string_view feature, const bool isSupported)
	{
		STORM_STATIC_ASSERT(index < count, "Out of range of what must be extracted.");

		std::string &lineToPrint = inOutExtracted[index].first;
		lineToPrint.reserve(feature.size() + 24);

		lineToPrint += feature;
		if (isSupported)
		{
			lineToPrint += " supported.";
			inOutExtracted[index].second = 'O';
		}
		else
		{
			lineToPrint += " not supported.";
			inOutExtracted[index].second = 'X';
		}
	}

	template<class ... Args>
	void printCapabilities(const Args &... args)
	{
		STORM_STATIC_ASSERT(sizeof...(args) % 2 == 0, "ARGS count should be even.");

		PairedSupportedElement capabilities[sizeof...(args) / 2];

		extractCapabilities<0>(capabilities, args...);

		std::size_t maxLength = 0;
		for (const PairedSupportedElement &elem : capabilities)
		{
			const std::size_t currentMsgLength = elem.first.size();
			if (maxLength < currentMsgLength)
			{
				maxLength = currentMsgLength;
			}
		}

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		for (const PairedSupportedElement &elem : capabilities)
		{
			std::cout << elem.first;

			const std::size_t alignCount = (maxLength - elem.first.size()) + 2;
			for (std::size_t iter = 0; iter < alignCount; ++iter)
			{
				std::cout << ' ';
			}

			auto consoleColorReverter = Storm::makeLazyRAIIObject([hConsole]()
			{
				SetConsoleTextAttribute(hConsole, 7);
			});

			if (elem.second == 'X')
			{
				SetConsoleTextAttribute(hConsole, 12);
			}
			else if (elem.second == 'O')
			{
				SetConsoleTextAttribute(hConsole, 10);
			}
			else STORM_UNLIKELY
			{
				SetConsoleTextAttribute(hConsole, 7);
			}

			std::cout << elem.second;

			std::cout << std::endl;
		}
	}
}

int main(int argc, const char*const argv[]) try
{
	const bool calledAsTool = argc == 2 && !::strcmp(argv[1], "--calledAsTool");

	std::cout << "Welcome to Storm-MaterialAvailability.exe.\nWe will now print all availabilities on the current station.\n\n";
	auto supportMessageLambda = [](const std::string_view feature, const bool isSupported)
	{
		std::string lineToPrint;
		lineToPrint.reserve(feature.size() + 24);

		lineToPrint += feature;
		if (isSupported)
		{
			lineToPrint += " supported.";
			return PairedSupportedElement{ std::move(lineToPrint), 'O' };
		}
		else
		{
			lineToPrint += " not supported.";
			return PairedSupportedElement{ std::move(lineToPrint), 'X' };
		}
	};

	std::cout << "Vendor : " << Storm::InstructionSet::vendor() << std::endl;
	std::cout << "Brand : " << Storm::InstructionSet::brand() << std::endl;
	std::cout << std::endl;
	
	printCapabilities(
		"SSE", Storm::InstructionSet::SSE(),
		"SSE2", Storm::InstructionSet::SSE2(),
		"SSE3", Storm::InstructionSet::SSE3(),
		"SSSE3", Storm::InstructionSet::SSSE3(),
		"SSE4.1", Storm::InstructionSet::SSE41(),
		"SSE4.2", Storm::InstructionSet::SSE42(),
		"SSE4a", Storm::InstructionSet::SSE4a(),
		"AVX", Storm::InstructionSet::AVX(),
		"AVX2", Storm::InstructionSet::AVX2(),
		"AVX512CD", Storm::InstructionSet::AVX512CD(),
		"AVX512ER", Storm::InstructionSet::AVX512ER(),
		"AVX512F", Storm::InstructionSet::AVX512F(),
		"AVX512PF", Storm::InstructionSet::AVX512PF(),
		"3DNOW", Storm::InstructionSet::_3DNOW(),
		"3DNOWEXT", Storm::InstructionSet::_3DNOWEXT(),
		"ABM", Storm::InstructionSet::ABM(),
		"ADX", Storm::InstructionSet::ADX(),
		"AES", Storm::InstructionSet::AES(),
		"BMI1", Storm::InstructionSet::BMI1(),
		"BMI2", Storm::InstructionSet::BMI2(),
		"CLFSH", Storm::InstructionSet::CLFSH(),
		"CMPXCHG16B", Storm::InstructionSet::CMPXCHG16B(),
		"CX8", Storm::InstructionSet::CX8(),
		"ERMS", Storm::InstructionSet::ERMS(),
		"F16C", Storm::InstructionSet::F16C(),
		"FMA", Storm::InstructionSet::FMA(),
		"FSGSBASE", Storm::InstructionSet::FSGSBASE(),
		"FXSR", Storm::InstructionSet::FXSR(),
		"HLE", Storm::InstructionSet::HLE(),
		"INVPCID", Storm::InstructionSet::INVPCID(),
		"LAHF", Storm::InstructionSet::LAHF(),
		"LZCNT", Storm::InstructionSet::LZCNT(),
		"MMX", Storm::InstructionSet::MMX(),
		"MMXEXT", Storm::InstructionSet::MMXEXT(),
		"MONITOR", Storm::InstructionSet::MONITOR(),
		"MOVBE", Storm::InstructionSet::MOVBE(),
		"MSR", Storm::InstructionSet::MSR(),
		"OSXSAVE", Storm::InstructionSet::OSXSAVE(),
		"PCLMULQDQ", Storm::InstructionSet::PCLMULQDQ(),
		"POPCNT", Storm::InstructionSet::POPCNT(),
		"PREFETCHWT1", Storm::InstructionSet::PREFETCHWT1(),
		"RDRAND", Storm::InstructionSet::RDRAND(),
		"RDSEED", Storm::InstructionSet::RDSEED(),
		"RDTSCP", Storm::InstructionSet::RDTSCP(),
		"RTM", Storm::InstructionSet::RTM(),
		"SEP", Storm::InstructionSet::SEP(),
		"SHA", Storm::InstructionSet::SHA(),
		"SYSCALL", Storm::InstructionSet::SYSCALL(),
		"TBM", Storm::InstructionSet::TBM(),
		"XOP", Storm::InstructionSet::XOP(),
		"XSAVE", Storm::InstructionSet::XSAVE()
	);

	std::cout << std::endl;
	std::cout << "Storm-MaterialAvailability.exe finished.\n";

	if (!calledAsTool)
	{
		::system("pause");
	}

	return static_cast<int>(Storm::ExitCode::k_success);
}
catch (const Storm::Exception &ex)
{
	std::cerr <<
		"Unhandled storm exception happened!\n"
		"Message was " << ex.what() << ".\n" << ex.stackTrace()
		;
	return static_cast<int>(Storm::ExitCode::k_stdException);
}
catch (const std::exception &ex)
{
	std::cerr << "Unhandled std exception happened! Message was " << ex.what();
	return static_cast<int>(Storm::ExitCode::k_stdException);
}
catch (...)
{
	std::cerr << "Unhandled unknown exception happened!";
	return static_cast<int>(Storm::ExitCode::k_unknownException);
}
