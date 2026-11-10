#pragma once

#include <cstdint>

namespace ginga
{

struct Color {
	std::uint8_t red = 220;
	std::uint8_t green = 228;
	std::uint8_t blue = 239;

	bool operator==(const Color &) const = default;
};

inline constexpr Color default_foreground{220, 228, 239};
inline constexpr Color default_background{16, 20, 27};

struct CellAttributes {
	bool bold = false;
	bool italic = false;
	bool underline = false;
	bool inverse = false;

	bool operator==(const CellAttributes &) const = default;
};

struct TerminalCell {
	char32_t character = U' ';
	Color foreground = default_foreground;
	Color background = default_background;
	CellAttributes attributes{};

	bool operator==(const TerminalCell &) const = default;
};

} // namespace ginga
