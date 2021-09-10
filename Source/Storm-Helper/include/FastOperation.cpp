#include "FastOperation.h"

#include "InstructionSet.h"


bool Storm::FastOperation::canUseSIMD()
{
	static const bool k_useSIMD = Storm::InstructionSet::SSE() && Storm::InstructionSet::SSE2();
	return k_useSIMD;
}

bool Storm::FastOperation::canUseAVX512()
{
	static const bool k_useAVX512 = Storm::InstructionSet::AVX512F() && canUseSIMD();
	return k_useAVX512;
}