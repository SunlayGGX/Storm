#include "BitField.h"


namespace
{
	enum Flag
	{
		flag1 = Storm::BitField<false, false, false, false>::value,
		flag2 = Storm::BitField<false, false, false, true>::value,
		flag3 = Storm::BitField<false, false, true, false>::value,
		flag4 = Storm::BitField<false, true, false, false>::value,
		flag5 = Storm::BitField<true, false, false, false>::value,
		flag6 = Storm::BitField<true, true, false, false>::value,
	};
}

TEST_CASE("BitField", "[classic]")
{
	CHECK(Flag::flag1 == 0b0000);
	CHECK(Flag::flag2 == 0b0001);
	CHECK(Flag::flag3 == 0b0010);
	CHECK(Flag::flag4 == 0b0100);
	CHECK(Flag::flag5 == 0b1000);
	CHECK(Flag::flag6 == 0b1100);

	CHECK(STORM_IS_BIT_ENABLED(Flag::flag6, Flag::flag4));
	CHECK(STORM_IS_BIT_ENABLED(Flag::flag6, Flag::flag5));
	CHECK(!STORM_IS_BIT_ENABLED(Flag::flag6, Flag::flag2));
	CHECK(!STORM_IS_BIT_ENABLED(Flag::flag6, Flag::flag1));

	Flag flag = Flag::flag3;

	STORM_ADD_BIT_ENABLED(flag, Flag::flag1);
	CHECK(flag == Flag::flag3);

	STORM_ADD_BIT_ENABLED(flag, Flag::flag4);
	CHECK(STORM_IS_BIT_ENABLED(flag, Flag::flag3));
	CHECK(STORM_IS_BIT_ENABLED(flag, Flag::flag4));
}
