#include "VisualStudioOutputCompliantParser.h"


bool Storm::VisualStudioOutputCompliantParser::produceVSOutputCompliantLine(std::string &inOutAppend, const std::string_view filePath, const std::size_t line)
{
	if (!filePath.empty())
	{
		inOutAppend += filePath;
		inOutAppend += '(';
		inOutAppend += std::to_string(line);
		inOutAppend += ')';

		return true;
	}

	return false;
}

bool Storm::VisualStudioOutputCompliantParser::produceVSOutputCompliantLine(std::string &inOutAppend, const std::size_t preNumber, const std::string_view filePath, const std::size_t line)
{
	inOutAppend += std::to_string(preNumber);
	inOutAppend += '>';
	return Storm::VisualStudioOutputCompliantParser::produceVSOutputCompliantLine(inOutAppend, filePath, line);
}
