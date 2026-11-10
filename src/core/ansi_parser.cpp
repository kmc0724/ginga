#include "ansi_parser.h"

#include "terminal_screen.h"

#include <algorithm>

namespace ginga
{

namespace
{

constexpr std::uint8_t esc = 0x1B;
constexpr std::uint8_t bel = 0x07;
constexpr std::uint8_t can = 0x18;
constexpr std::uint8_t sub = 0x1A;

} // namespace

AnsiParser::AnsiParser(TerminalScreen &screen) : screen_(screen)
{
}

void AnsiParser::reset_csi()
{
	parameters_.clear();
	current_parameter_ = 0;
	has_parameter_ = false;
}

void AnsiParser::reset()
{
	decoder_.reset();
	state_ = State::Ground;
	reset_csi();
	text_buffer_.clear();
}

void AnsiParser::flush_text()
{
	if (text_buffer_.empty())
		return;

	screen_.write(text_buffer_);
	text_buffer_.clear();
}

void AnsiParser::finish_text_sequence()
{
	text_buffer_ += decoder_.finish();
	flush_text();
}

void AnsiParser::process_ground(std::uint8_t byte)
{
	if (byte == esc) {
		finish_text_sequence();
		state_ = State::Escape;
		return;
	}

	// CAN and SUB cancel pending control sequences.
	if (byte == can || byte == sub) {
		finish_text_sequence();
		return;
	}

	// C0 controls are handled separately from UTF-8 text.
	if (byte < 0x20 || byte == 0x7F) {
		finish_text_sequence();

		if (byte == '\r' || byte == '\n') {
			const char character = static_cast<char>(byte);
			screen_.put_char(static_cast<unsigned char>(character));
		}

		return;
	}

	const char character = static_cast<char>(byte);
	text_buffer_ += decoder_.decode(std::string_view(&character, 1));
}

void AnsiParser::process_escape(std::uint8_t byte)
{
	if (byte == '[') {
		reset_csi();
		state_ = State::Csi;
		return;
	}

	if (byte == ']') {
		state_ = State::Osc;
		return;
	}

	if (byte == 'P' || byte == '_' || byte == '^' || byte == 'X') {
		state_ = State::StringIgnore;
		return;
	}

	if (byte == esc) {
		state_ = State::Escape;
		return;
	}

	state_ = State::Ground;
}

void AnsiParser::process_csi(std::uint8_t byte)
{
	if (byte == esc) {
		reset_csi();
		state_ = State::Escape;
		return;
	}

	if (byte == can || byte == sub) {
		reset_csi();
		state_ = State::Ground;
		return;
	}

	if (byte >= '0' && byte <= '9') {
		const int digit = byte - '0';

		if (current_parameter_ > (max_parameter_value_ - digit) / 10) {
			state_ = State::CsiIgnore;
			return;
		}

		current_parameter_ = current_parameter_ * 10 + digit;
		has_parameter_ = true;
		return;
	}

	if (byte == ';') {
		if (parameters_.size() >= max_parameters_) {
			state_ = State::CsiIgnore;
			return;
		}

		parameters_.push_back(has_parameter_ ? current_parameter_ : 0);

		current_parameter_ = 0;
		has_parameter_ = false;
		return;
	}

	// CSI private prefixes and intermediate bytes are
	// unsupported in v0.4.2.
	if (byte >= 0x20 && byte <= 0x3F) {
		state_ = State::CsiIgnore;
		return;
	}

	if (byte >= 0x40 && byte <= 0x7E) {
		if (has_parameter_ || !parameters_.empty()) {
			if (parameters_.size() >= max_parameters_) {
				reset_csi();
				state_ = State::Ground;
				return;
			}

			parameters_.push_back(
			    has_parameter_ ? current_parameter_ : 0);
		}

		execute_csi(static_cast<char>(byte));
		reset_csi();
		state_ = State::Ground;
		return;
	}

	reset_csi();
	state_ = State::Ground;
}

void AnsiParser::process_csi_ignore(std::uint8_t byte)
{
	if (byte == esc) {
		reset_csi();
		state_ = State::Escape;
		return;
	}

	if (byte == can || byte == sub || (byte >= 0x40 && byte <= 0x7E)) {
		reset_csi();
		state_ = State::Ground;
	}
}

void AnsiParser::process_osc(std::uint8_t byte)
{
	if (byte == bel || byte == can || byte == sub) {
		state_ = State::Ground;
		return;
	}

	if (byte == esc)
		state_ = State::OscEscape;
}

void AnsiParser::process_osc_escape(std::uint8_t byte)
{
	if (byte == '\\' || byte == bel || byte == can || byte == sub) {
		state_ = State::Ground;
		return;
	}

	if (byte != esc)
		state_ = State::Osc;
}

void AnsiParser::process_string_ignore(std::uint8_t byte)
{
	if (byte == can || byte == sub) {
		state_ = State::Ground;
		return;
	}

	if (byte == esc)
		state_ = State::StringEscape;
}

void AnsiParser::process_string_escape(std::uint8_t byte)
{
	if (byte == '\\' || byte == can || byte == sub) {
		state_ = State::Ground;
		return;
	}

	if (byte != esc)
		state_ = State::StringIgnore;
}

int AnsiParser::parameter(std::size_t index, int default_value) const
{
	if (index >= parameters_.size() || parameters_[index] == 0)
		return default_value;

	return parameters_[index];
}

Color AnsiParser::ansi_color(int index)
{
	static constexpr Color palette[16] = {
	    {0, 0, 0},
	    {205, 49, 49},
	    {13, 188, 121},
	    {229, 229, 16},
	    {36, 114, 200},
	    {188, 63, 188},
	    {17, 168, 205},
	    {229, 229, 229},
	    {102, 102, 102},
	    {241, 76, 76},
	    {35, 209, 139},
	    {245, 245, 67},
	    {59, 142, 234},
	    {214, 112, 214},
	    {41, 184, 219},
	    {255, 255, 255},
	};

	return palette[index];
}

Color AnsiParser::indexed_color(int index)
{
	if (index < 16)
		return ansi_color(index);

	if (index < 232) {
		index -= 16;

		const int red = index / 36;
		const int green = (index / 6) % 6;
		const int blue = index % 6;

		auto component = [](int value) -> std::uint8_t {
			return static_cast<std::uint8_t>(
			    value == 0 ? 0 : 55 + value * 40);
		};

		return {
		    component(red),
		    component(green),
		    component(blue),
		};
	}

	const auto gray = static_cast<std::uint8_t>(8 + (index - 232) * 10);

	return {gray, gray, gray};
}

void AnsiParser::execute_sgr()
{
	if (parameters_.empty()) {
		screen_.reset_style();
		return;
	}

	for (std::size_t i = 0; i < parameters_.size(); ++i) {
		const int value = parameters_[i];

		if (value == 0) {
			screen_.reset_style();
			continue;
		}

		CellAttributes attributes = screen_.current_attributes();

		switch (value) {
		case 1:
			attributes.bold = true;
			break;
		case 3:
			attributes.italic = true;
			break;
		case 4:
			attributes.underline = true;
			break;
		case 7:
			attributes.inverse = true;
			break;
		case 22:
			attributes.bold = false;
			break;
		case 23:
			attributes.italic = false;
			break;
		case 24:
			attributes.underline = false;
			break;
		case 27:
			attributes.inverse = false;
			break;
		case 39:
			screen_.reset_foreground();
			break;
		case 49:
			screen_.reset_background();
			break;
		default:
			break;
		}

		screen_.set_attributes(attributes);

		// Standard ANSI foreground colors.
		if (value >= 30 && value <= 37) {
			screen_.set_foreground(ansi_color(value - 30));
			continue;
		}

		// Standard ANSI background colors.
		if (value >= 40 && value <= 47) {
			screen_.set_background(ansi_color(value - 40));
			continue;
		}

		// Bright ANSI foreground colors.
		if (value >= 90 && value <= 97) {
			screen_.set_foreground(ansi_color(value - 90 + 8));
			continue;
		}

		// Bright ANSI background colors.
		if (value >= 100 && value <= 107) {
			screen_.set_background(ansi_color(value - 100 + 8));
			continue;
		}

		// Extended foreground/background colors.
		if (value != 38 && value != 48)
			continue;

		const bool foreground = value == 38;

		if (i + 1 >= parameters_.size())
			continue;

		const int mode = parameters_[i + 1];

		// 256-color mode: 38;5;n or 48;5;n.
		if (mode == 5) {
			if (i + 2 >= parameters_.size()) {
				++i;
				continue;
			}

			const int index = parameters_[i + 2];

			if (index >= 0 && index <= 255) {
				const Color color = indexed_color(index);

				if (foreground)
					screen_.set_foreground(color);
				else
					screen_.set_background(color);
			}

			i += 2;
			continue;
		}

		// True Color mode: 38;2;R;G;B or 48;2;R;G;B.
		if (mode == 2) {
			if (i + 4 >= parameters_.size()) {
				++i;
				continue;
			}

			const int red = parameters_[i + 2];
			const int green = parameters_[i + 3];
			const int blue = parameters_[i + 4];

			if (red >= 0 && red <= 255 && green >= 0 &&
			    green <= 255 && blue >= 0 && blue <= 255) {
				const Color color{
				    static_cast<std::uint8_t>(red),
				    static_cast<std::uint8_t>(green),
				    static_cast<std::uint8_t>(blue),
				};

				if (foreground)
					screen_.set_foreground(color);
				else
					screen_.set_background(color);
			}

			i += 4;
			continue;
		}

		// Unsupported extended color modes are ignored.
		++i;
	}
}

void AnsiParser::execute_csi(char command)
{
	const int column = screen_.cursor_column();
	const int row = screen_.cursor_row();

	const int max_column = screen_.columns() - 1;
	const int max_row = screen_.rows() - 1;

	const int amount = parameter(0);

	switch (command) {
	case 'A': // CUU
		screen_.set_cursor(column, std::max(0, row - amount));
		break;

	case 'B': // CUD
		screen_.set_cursor(column, std::min(max_row, row + amount));
		break;

	case 'C': // CUF
		screen_.set_cursor(std::min(max_column, column + amount), row);
		break;

	case 'D': // CUB
		screen_.set_cursor(std::max(0, column - amount), row);
		break;

	case 'E': // CNL
		screen_.set_cursor(0, std::min(max_row, row + amount));
		break;

	case 'F': // CPL
		screen_.set_cursor(0, std::max(0, row - amount));
		break;

	case 'G': // CHA
		screen_.set_cursor(std::clamp(amount - 1, 0, max_column), row);
		break;

	case 'J': // ED - Erase in Display
		screen_.erase_in_display(parameter(0, 0));
		break;

	case 'K': // EL - Erase in Line
		screen_.erase_in_line(parameter(0, 0));
		break;

	case 'H': // CUP
	case 'f': // HVP
		screen_.set_cursor(std::clamp(parameter(1) - 1, 0, max_column),
				   std::clamp(parameter(0) - 1, 0, max_row));
		break;

	case 'd': // VPA
		screen_.set_cursor(column, std::clamp(amount - 1, 0, max_row));
		break;

	case 'm': // SGR - Select Graphic Rendition
		execute_sgr();
		break;

	default:
		break;
	}
}

void AnsiParser::process_byte(std::uint8_t byte)
{
	switch (state_) {
	case State::Ground:
		process_ground(byte);
		break;
	case State::Escape:
		process_escape(byte);
		break;
	case State::Csi:
		process_csi(byte);
		break;
	case State::CsiIgnore:
		process_csi_ignore(byte);
		break;
	case State::Osc:
		process_osc(byte);
		break;
	case State::OscEscape:
		process_osc_escape(byte);
		break;
	case State::StringIgnore:
		process_string_ignore(byte);
		break;
	case State::StringEscape:
		process_string_escape(byte);
		break;
	}
}

void AnsiParser::feed(std::string_view bytes)
{
	for (unsigned char byte : bytes)
		process_byte(byte);

	flush_text();
}

void AnsiParser::finish()
{
	finish_text_sequence();
	state_ = State::Ground;
	reset_csi();
}

} // namespace ginga
