#include "utf8_decoder.h"

namespace ginga
{

namespace
{

constexpr char32_t replacement = U'\uFFFD';

} // namespace

void Utf8Decoder::reset()
{
	codepoint_ = 0;
	minimum_ = 0;
	remaining_ = 0;
}

std::u32string Utf8Decoder::decode(std::string_view bytes)
{
	std::u32string output;

	for (unsigned char byte : bytes) {
		bool retry = true;

		while (retry) {
			retry = false;

			if (remaining_ == 0) {
				if (byte <= 0x7F) {
					output.push_back(byte);
				} else if (byte >= 0xC2 && byte <= 0xDF) {
					codepoint_ = byte & 0x1F;
					minimum_ = 0x80;
					remaining_ = 1;
				} else if (byte >= 0xE0 && byte <= 0xEF) {
					codepoint_ = byte & 0x0F;
					minimum_ = 0x800;
					remaining_ = 2;
				} else if (byte >= 0xF0 && byte <= 0xF4) {
					codepoint_ = byte & 0x07;
					minimum_ = 0x10000;
					remaining_ = 3;
				} else {
					output.push_back(replacement);
				}
				continue;
			}

			if ((byte & 0xC0) != 0x80) {
				output.push_back(replacement);
				reset();
				retry = true;
				continue;
			}

			codepoint_ = (codepoint_ << 6) | (byte & 0x3F);
			--remaining_;

			if (remaining_ != 0)
				continue;

			if (codepoint_ < minimum_ || codepoint_ > 0x10FFFF ||
			    (codepoint_ >= 0xD800 && codepoint_ <= 0xDFFF)) {
				output.push_back(replacement);
			} else {
				output.push_back(
				    static_cast<char32_t>(codepoint_));
			}

			reset();
		}
	}

	return output;
}

std::u32string Utf8Decoder::finish()
{
	std::u32string output;

	if (remaining_ != 0)
		output.push_back(replacement);

	reset();
	return output;
}

} // namespace ginga
