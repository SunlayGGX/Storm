#include "CSVHelpers.h"


TEST_CASE("CSVHelpers.transcriptLetterPosition", "[classic]")
{
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(0) == "A");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(2) == "C");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(25) == "Z");

#if true // Not implemented algorithm
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(26) == "AA");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(27) == "AB");

	CHECK(Storm::CSVHelpers::transcriptLetterPosition(52) == "BA");

	CHECK(Storm::CSVHelpers::transcriptLetterPosition(702) == "AAA");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(703) == "AAB");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(801) == "ADV");
	CHECK(Storm::CSVHelpers::transcriptLetterPosition(1389) == "BAL");
#endif
}