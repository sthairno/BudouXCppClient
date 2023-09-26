#include <iostream>
#include <simdutf.h>

#include "Parser.hpp"

static size_t convertUtf8ToUtf16(const std::string_view src, std::u16string &dst)
{
	dst.resize(
		simdutf::utf16_length_from_utf8(src.data(), src.length()),
		u'\0');
	return simdutf::convert_utf8_to_utf16(src.data(), src.length(), dst.data());
}

static size_t convertUtf16ToUtf8(const std::u16string_view src, std::string &dst)
{
	dst.resize(
		simdutf::utf8_length_from_utf16(src.data(), src.length()),
		'\0');
	return simdutf::convert_utf16_to_utf8(src.data(), src.length(), dst.data());
}

int main()
{
	std::u16string u16input;
	{
		std::string input, line;
		while (std::getline(std::cin, line))
		{
			if (line.empty())
				break;
			if (not input.empty())
			{
				input += "\n";
			}
			input += line;
		}
		convertUtf8ToUtf16(input, u16input);
	}

	Parser parser;
	parser.loadByFileName("ja.json");

	auto result = parser.parse(u16input);
	
	if (not result.empty())
	{
		{
			std::string output;
			convertUtf16ToUtf8(result.front(), output);
			std::cout << output;
		}

		for (auto itr = result.begin() + 1; itr != result.end(); itr++)
		{
			std::string output;
			convertUtf16ToUtf8(*itr, output);
			std::cout << " | " << output;
		}

		std::cout << std::endl;
	}
}
