
#include "utf8_decoder.h"

#include <iostream>
#include <stdexcept>
#include <string>

using ginga::Utf8Decoder;

void check(bool condition, const char *message)
{
	if (!condition)
		throw std::runtime_error(message);
}

int main()
{
	try {
		Utf8Decoder decoder;

		check(decoder.decode("Hello") == U"Hello",
		      "ASCII decoding failed");

		decoder.reset();
		check(decoder.decode("\xEA\xB0\x80") == U"가",
		      "Korean decoding failed");

		decoder.reset();
		check(decoder.decode("\xE3\x81\x82") == U"あ",
		      "Japanese decoding failed");

		decoder.reset();
		check(decoder.decode("\xF0\x9F\x8C\x8C") == U"🌌",
		      "4-byte UTF-8 decoding failed");

		decoder.reset();
		check(decoder.decode("\xEA\xB0").empty(),
		      "Incomplete sequence should wait");
		check(decoder.decode("\x80") == U"가",
		      "Split UTF-8 decoding failed");

		decoder.reset();
		check(decoder.decode("\xFF") == U"\uFFFD",
		      "Invalid byte handling failed");

		decoder.reset();
		check(decoder.decode("\xEA").empty(),
		      "Incomplete sequence handling failed");
		check(decoder.finish() == U"\uFFFD",
		      "Incomplete sequence finalization failed");

		std::cout << "Utf8Decoder tests passed\n";
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "Utf8Decoder test failed: " << error.what()
			  << '\n';
		return 1;
	}
}
