#include "CSVHelpers.h"


std::string Storm::CSVHelpers::transcriptLetterPosition(std::size_t position)
{
	std::string result;

	if (position < 26)
	{
		result += static_cast<const char>('A' + position);
	}
	else
	{
		std::size_t powCount = 0;
		for (std::size_t val = position; val != 0; val /= 26)
		{
			++powCount;
		}

		result.reserve(powCount);

		for (std::size_t iter = 0; iter != powCount; ++iter)
		{
			result += static_cast<const char>('A' + position - (position / 26) * 26);
			position = std::max(static_cast<int64_t>(position / 26) - 1, 0ll);
		}

		std::reverse(std::begin(result), std::end(result));
	}

	return result;
}
