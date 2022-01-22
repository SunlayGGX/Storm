#include "InstructionSet.h"

#include <intrin.h>
#include <array>



// Initialize static member data
const Storm::InstructionSet::InstructionSet_Internal Storm::InstructionSet::s_cpuRep;

Storm::InstructionSet::InstructionSet_Internal::InstructionSet_Internal() :
	_nIds{ 0 },
	_nExIds{ 0 },
	_isIntel{ false },
	_isAMD{ false },
	_f_1_ECX{ 0 },
	_f_1_EDX{ 0 },
	_f_7_EBX{ 0 },
	_f_7_ECX{ 0 },
	_f_81_ECX{ 0 },
	_f_81_EDX{ 0 },
	_data{},
	_extdata{}
{
    std::array<int, 4> cpui;

    // Calling __cpuid with 0x0 as the function_id argument
    // gets the number of the highest valid function ID.
    ::__cpuid(cpui.data(), 0);
    _nIds = cpui[0];

    _data.reserve(_nIds + 1);
    for (int i = 0; i <= _nIds; ++i)
    {
        ::__cpuidex(cpui.data(), i, 0);
        _data.emplace_back(cpui);
    }

    // Capture vendor string
    char vendor[32];
    ::memset(vendor, 0, sizeof(vendor));
    *reinterpret_cast<int*>(vendor) = _data[0][1];
    *reinterpret_cast<int*>(vendor + 4) = _data[0][3];
    *reinterpret_cast<int*>(vendor + 8) = _data[0][2];

    _vendor = vendor;
    if (_vendor == "GenuineIntel")
    {
        _isIntel = true;
    }
    else if (_vendor == "AuthenticAMD")
    {
        _isAMD = true;
    }

    // load bitset with flags for function 0x00000001
    if (_nIds >= 1)
    {
        _f_1_ECX = _data[1][2];
        _f_1_EDX = _data[1][3];
    }

    // load bitset with flags for function 0x00000007
    if (_nIds >= 7)
    {
        _f_7_EBX = _data[7][1];
        _f_7_ECX = _data[7][2];
    }

    // Calling __cpuid with 0x80000000 as the function_id argument
    // gets the number of the highest valid extended ID.
    ::__cpuid(cpui.data(), 0x80000000);
    _nExIds = cpui[0];

    char brand[64];
    ::memset(brand, 0, sizeof(brand));

    if (_nExIds >= 0x80000000)
    {
        _extdata.reserve(_nExIds - 0x80000000 + 1);
        for (int i = 0x80000000; i <= _nExIds; ++i)
        {
            ::__cpuidex(cpui.data(), i, 0);
            _extdata.emplace_back(cpui);
        }
    }

    // load bitset with flags for function 0x80000001
    if (_nExIds >= 0x80000001)
    {
        _f_81_ECX = _extdata[1][2];
        _f_81_EDX = _extdata[1][3];
    }

    // Interpret CPU brand string if reported
    if (_nExIds >= 0x80000004)
    {
        ::memcpy(brand, _extdata[2].data(), sizeof(cpui));
        ::memcpy(brand + 16, _extdata[3].data(), sizeof(cpui));
        ::memcpy(brand + 32, _extdata[4].data(), sizeof(cpui));
        _brand = brand;
    }
}

const std::string& Storm::InstructionSet::vendor()
{
    return s_cpuRep._vendor;
}

const std::string& Storm::InstructionSet::brand()
{
    return s_cpuRep._brand;
}

bool Storm::InstructionSet::SSE3()
{
    return s_cpuRep._f_1_ECX[0];
}

bool Storm::InstructionSet::PCLMULQDQ()
{
    return s_cpuRep._f_1_ECX[1];
}

bool Storm::InstructionSet::MONITOR()
{
    return s_cpuRep._f_1_ECX[3];
}

bool Storm::InstructionSet::SSSE3()
{
    return s_cpuRep._f_1_ECX[9];
}

bool Storm::InstructionSet::FMA()
{
    return s_cpuRep._f_1_ECX[12];
}

bool Storm::InstructionSet::CMPXCHG16B()
{
    return s_cpuRep._f_1_ECX[13];
}

bool Storm::InstructionSet::SSE41()
{
    return s_cpuRep._f_1_ECX[19];
}

bool Storm::InstructionSet::SSE42()
{
    return s_cpuRep._f_1_ECX[20];
}

bool Storm::InstructionSet::MOVBE()
{
    return s_cpuRep._f_1_ECX[22];
}

bool Storm::InstructionSet::POPCNT()
{
    return s_cpuRep._f_1_ECX[23];
}

bool Storm::InstructionSet::AES()
{
    return s_cpuRep._f_1_ECX[25];
}

bool Storm::InstructionSet::XSAVE()
{
    return s_cpuRep._f_1_ECX[26];
}

bool Storm::InstructionSet::OSXSAVE()
{
    return s_cpuRep._f_1_ECX[27];
}

bool Storm::InstructionSet::AVX()
{
    return s_cpuRep._f_1_ECX[28];
}

bool Storm::InstructionSet::F16C()
{
    return s_cpuRep._f_1_ECX[29];
}

bool Storm::InstructionSet::RDRAND()
{
    return s_cpuRep._f_1_ECX[30];
}

bool Storm::InstructionSet::MSR()
{
    return s_cpuRep._f_1_EDX[5];
}

bool Storm::InstructionSet::CX8()
{
    return s_cpuRep._f_1_EDX[8];
}

bool Storm::InstructionSet::SEP()
{
    return s_cpuRep._f_1_EDX[11];
}

bool Storm::InstructionSet::CMOV()
{
    return s_cpuRep._f_1_EDX[15];
}

bool Storm::InstructionSet::CLFSH()
{
    return s_cpuRep._f_1_EDX[19];
}

bool Storm::InstructionSet::MMX()
{
    return s_cpuRep._f_1_EDX[23];
}

bool Storm::InstructionSet::FXSR()
{
    return s_cpuRep._f_1_EDX[24];
}

bool Storm::InstructionSet::SSE()
{
    return s_cpuRep._f_1_EDX[25];
}

bool Storm::InstructionSet::SSE2()
{
    return s_cpuRep._f_1_EDX[26];
}

bool Storm::InstructionSet::FSGSBASE()
{
    return s_cpuRep._f_7_EBX[0];
}

bool Storm::InstructionSet::BMI1()
{
    return s_cpuRep._f_7_EBX[3];
}

bool Storm::InstructionSet::HLE()
{
    return s_cpuRep._isIntel && s_cpuRep._f_7_EBX[4];
}

bool Storm::InstructionSet::AVX2()
{
	return s_cpuRep._f_7_EBX[5];
}

bool Storm::InstructionSet::BMI2()
{
    return s_cpuRep._f_7_EBX[8];
}

bool Storm::InstructionSet::ERMS()
{
    return s_cpuRep._f_7_EBX[9];
}

bool Storm::InstructionSet::INVPCID()
{
    return s_cpuRep._f_7_EBX[10];
}

bool Storm::InstructionSet::RTM()
{
    return s_cpuRep._isIntel && s_cpuRep._f_7_EBX[11];
}

bool Storm::InstructionSet::AVX512F()
{
    return s_cpuRep._f_7_EBX[16];
}

bool Storm::InstructionSet::RDSEED()
{
    return s_cpuRep._f_7_EBX[18];
}

bool Storm::InstructionSet::ADX()
{
    return s_cpuRep._f_7_EBX[19];
}

bool Storm::InstructionSet::AVX512PF()
{
    return s_cpuRep._f_7_EBX[26];
}

bool Storm::InstructionSet::AVX512ER()
{
    return s_cpuRep._f_7_EBX[27];
}

bool Storm::InstructionSet::AVX512CD()
{
    return s_cpuRep._f_7_EBX[28];
}

bool Storm::InstructionSet::SHA()
{
    return s_cpuRep._f_7_EBX[29];
}

bool Storm::InstructionSet::PREFETCHWT1()
{
    return s_cpuRep._f_7_ECX[0];
}

bool Storm::InstructionSet::LAHF()
{
    return s_cpuRep._f_81_ECX[0];
}

bool Storm::InstructionSet::LZCNT()
{
    return s_cpuRep._isIntel && s_cpuRep._f_81_ECX[5];
}

bool Storm::InstructionSet::ABM()
{
    return s_cpuRep._isAMD && s_cpuRep._f_81_ECX[5];
}

bool Storm::InstructionSet::SSE4a()
{
    return s_cpuRep._isAMD && s_cpuRep._f_81_ECX[6];
}

bool Storm::InstructionSet::XOP()
{
    return s_cpuRep._isAMD && s_cpuRep._f_81_ECX[11];
}

bool Storm::InstructionSet::TBM()
{
    return s_cpuRep._isAMD && s_cpuRep._f_81_ECX[21];
}

bool Storm::InstructionSet::SYSCALL()
{
    return s_cpuRep._isIntel && s_cpuRep._f_81_EDX[11];
}

bool Storm::InstructionSet::MMXEXT()
{
    return s_cpuRep._isAMD && s_cpuRep._f_81_EDX[22];
}

bool Storm::InstructionSet::RDTSCP()
{
    return s_cpuRep._isIntel && s_cpuRep._f_81_EDX[27];
}

bool Storm::InstructionSet::_3DNOWEXT()
{
    return s_cpuRep._isAMD && s_cpuRep._f_81_EDX[30];
}

bool Storm::InstructionSet::_3DNOW()
{
    return s_cpuRep._isAMD && s_cpuRep._f_81_EDX[31];
};
