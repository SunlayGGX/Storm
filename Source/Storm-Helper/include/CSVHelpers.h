#pragma once

#include "NonInstanciable.h"

namespace Storm
{
	class CSVHelpers : private Storm::NonInstanciable
	{
	public:
		// Transform the position of the column to its letter codename we could see from any spreadsheet software.
		// Note tant "position" is 0-based. It means that position is not the same as the result got from =COLONNE formula (offset by 1 because columns are 1-based index).
		//
		//  I.e. 0 becomes 'A', 1 is 'B', 25 is 'Z', 26 is 'AA' and 27 is 'AB' ...
		static std::string transcriptLetterPosition(std::size_t position);
	};
}
