#include "FastOperation.h"


namespace
{
	struct Vec { double _x; double _y; double _z; auto operator<=>(const Vec &) const = default; };
}

TEST_CASE("FastOperation.zeroMemory", "[classic]")
{
	std::vector<int64_t> val;
	
	if (Storm::FastOperation::canUseAVX512())
	{
		val = std::vector<int64_t>(static_cast<std::size_t>(1259), 500);
		Storm::FastOperation::zeroMemory<true, true>(val);
		CHECK(val == std::vector<int64_t>(static_cast<std::size_t>(1259), 0));
	}

	if (Storm::FastOperation::canUseSIMD())
	{
		val = std::vector<int64_t>(static_cast<std::size_t>(1259), 500);
		Storm::FastOperation::zeroMemory<true, false>(val);
		CHECK(val == std::vector<int64_t>(static_cast<std::size_t>(1259), 0));
	}

	val = std::vector<int64_t>(static_cast<std::size_t>(1259), 500);
	Storm::FastOperation::zeroMemory<false, false>(val);
	CHECK(val == std::vector<int64_t>(static_cast<std::size_t>(1259), 0));
}

TEST_CASE("FastOperation.copyMemory", "[classic]")
{
	const std::vector<Vec> orig = std::vector<Vec>(static_cast<std::size_t>(1259), Vec{ 0.0, 5.0, -10.0 });
	std::vector<Vec> val;
	std::vector<Vec> val2;
	
	if (Storm::FastOperation::canUseAVX512())
	{
		val = orig;
		val2 = std::vector<Vec>(static_cast<std::size_t>(1259), Vec{ 0.0, 0.0, 0.0 });
		Storm::FastOperation::copyMemory<true, true>(val, val2);
		CHECK(val == val2);
		CHECK(val == orig);
	}

	if (Storm::FastOperation::canUseSIMD())
	{
		val = orig;
		val2 = std::vector<Vec>(static_cast<std::size_t>(1259), Vec{ 0.0, 0.0, 0.0 });
		Storm::FastOperation::copyMemory<true, false>(val, val2);
		CHECK(val == val2);
		CHECK(val == orig);
	}

	val = orig;
	val2 = std::vector<Vec>(static_cast<std::size_t>(1259), Vec{ 0.0, 0.0, 0.0 });
	Storm::FastOperation::copyMemory<false, false>(val, val2);
	CHECK(val == val2);
	CHECK(val == orig);
}

TEST_CASE("FastOperation.copyMemoryV2", "[classic]")
{
	const std::vector<Vec> orig = std::vector<Vec>(static_cast<std::size_t>(1259), Vec{ 0.0, 5.0, -10.0 });
	std::vector<Vec> val;
	std::vector<Vec> val2;

	if (Storm::FastOperation::canUseAVX512())
	{
		val = orig;
		val2 = std::vector<Vec>(static_cast<std::size_t>(1259), Vec{ 0.0, 0.0, 0.0 });
		Storm::FastOperation::copyMemory_V2<Storm::SIMDUsageMode::AVX512>(val, val2);
		CHECK(val == val2);
		CHECK(val == orig);
	}

	if (Storm::FastOperation::canUseSIMD())
	{
		val = orig;
		val2 = std::vector<Vec>(static_cast<std::size_t>(1259), Vec{ 0.0, 0.0, 0.0 });
		Storm::FastOperation::copyMemory_V2<Storm::SIMDUsageMode::SSE>(val, val2);
		CHECK(val == val2);
		CHECK(val == orig);
	}

	val = orig;
	val2 = std::vector<Vec>(static_cast<std::size_t>(1259), Vec{ 0.0, 0.0, 0.0 });
	Storm::FastOperation::copyMemory_V2<Storm::SIMDUsageMode::SISD>(val, val2);
	CHECK(val == val2);
	CHECK(val == orig);
}
