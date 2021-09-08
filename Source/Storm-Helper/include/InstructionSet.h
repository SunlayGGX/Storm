// Code is extracted from https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=msvc-160
// And changed a little to ensure stabler, prettier code and to respect our coding standards.
#pragma once


namespace Storm
{
    class InstructionSet
    {
    private:
        class InstructionSet_Internal
        {
        public:
            InstructionSet_Internal();

            int _nIds;
            int _nExIds;
            std::string _vendor;
            std::string _brand;
            bool _isIntel;
            bool _isAMD;
            std::bitset<32> _f_1_ECX;
            std::bitset<32> _f_1_EDX;
            std::bitset<32> _f_7_EBX;
            std::bitset<32> _f_7_ECX;
            std::bitset<32> _f_81_ECX;
            std::bitset<32> _f_81_EDX;
            std::vector<std::array<int, 4>> _data;
            std::vector<std::array<int, 4>> _extdata;
        };

    public:
        // getters
        static const std::string& vendor();
        static const std::string& brand();

        static bool SSE3();
        static bool PCLMULQDQ();
        static bool MONITOR();
        static bool SSSE3();
        static bool FMA();
        static bool CMPXCHG16B();
        static bool SSE41();
        static bool SSE42();
        static bool MOVBE();
        static bool POPCNT();
        static bool AES();
        static bool XSAVE();
        static bool OSXSAVE();
        static bool AVX();
        static bool F16C();
        static bool RDRAND();

        static bool MSR();
        static bool CX8();
        static bool SEP();
        static bool CMOV();
        static bool CLFSH();
        static bool MMX();
        static bool FXSR();
        static bool SSE();
        static bool SSE2();

        static bool FSGSBASE();
        static bool BMI1();
        static bool HLE();
        static bool AVX2();
        static bool BMI2();
        static bool ERMS();
        static bool INVPCID();
        static bool RTM();
        static bool AVX512F();
        static bool RDSEED();
        static bool ADX();
        static bool AVX512PF();
        static bool AVX512ER();
        static bool AVX512CD();
        static bool SHA();

        static bool PREFETCHWT1();

        static bool LAHF();
        static bool LZCNT();
        static bool ABM();
        static bool SSE4a();
        static bool XOP();
        static bool TBM();

        static bool SYSCALL();
        static bool MMXEXT();
        static bool RDTSCP();
        static bool _3DNOWEXT();
        static bool _3DNOW();

    private:
        static const InstructionSet_Internal s_cpuRep;
    };
}
