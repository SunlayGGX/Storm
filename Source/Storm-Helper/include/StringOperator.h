#pragma once


inline std::string operator+(std::string left, const std::string_view &right)
{
	return left += right;
}

inline std::string operator+(const std::string_view &left, const std::string_view &right)
{
	return std::string{ left } + right;
}
