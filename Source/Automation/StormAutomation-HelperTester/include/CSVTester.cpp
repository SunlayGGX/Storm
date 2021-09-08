#include "CSVHelpers.h"


TEST_CASE("CSVHelpers.transcriptLetterPosition", "[classic]")
{
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(0) == "A");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(2) == "C");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(25) == "Z");
	
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(26) == "AA");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(27) == "AB");

	CHECK(Storm::CSVHelpers::transcriptLetterPosition(52) == "BA");

	CHECK(Storm::CSVHelpers::transcriptLetterPosition(702) == "AAA");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(703) == "AAB");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(801) == "ADV");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(1389) == "BAL");

	CHECK(Storm::CSVHelpers::transcriptLetterPosition(3816) == "EPU");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(7678) == "KII");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(12779) == "RWN");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(16363) == "XEJ");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(16383) == "XFD");
}