#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ginga
{

class Utf8Decoder
{
public:
	std::u32string decode(std::string_view bytes);
	std::u32string finish();
	void reset();

private:
	std::uint32_t codepoint_ = 0;
	std::uint32_t minimum_ = 0;
	int remaining_ = 0;
};

} // namespace ginga
