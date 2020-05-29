#include "StringOperator.h"


std::string operator+(const std::string_view &left, const std::string_view &right)
{
	return left + right;
}

std::string operator+(const std::string_view &left, const std::string &right)
{
	return left + right;
}

std::string operator+(const std::string &left, const std::string_view &right)
{
	return left + right;
}
