#pragma once

#include "terminal_cell.h"
#include "utf8_decoder.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace ginga
{

class TerminalScreen;

class AnsiParser
{
public:
	explicit AnsiParser(TerminalScreen &screen);

	void feed(std::string_view bytes);
	void finish();
	void reset();

private:
	enum class State {
		Ground,
		Escape,
		Csi,
		CsiIgnore,
		Osc,
		OscEscape,
		StringIgnore,
		StringEscape
	};

	void process_byte(std::uint8_t byte);
	void process_ground(std::uint8_t byte);
	void process_escape(std::uint8_t byte);
	void process_csi(std::uint8_t byte);
	void process_csi_ignore(std::uint8_t byte);
	void process_osc(std::uint8_t byte);
	void process_osc_escape(std::uint8_t byte);
	void process_string_ignore(std::uint8_t byte);
	void process_string_escape(std::uint8_t byte);

	void flush_text();
	void finish_text_sequence();
	void reset_csi();
	int parameter(std::size_t index, int default_value = 1) const;
	void execute_csi(char command);
	void execute_sgr();
	static Color ansi_color(int index);
	static Color indexed_color(int index);

	TerminalScreen &screen_;
	Utf8Decoder decoder_;

	State state_ = State::Ground;

	std::vector<int> parameters_;
	int current_parameter_ = 0;
	bool has_parameter_ = false;

	std::u32string text_buffer_;

	static constexpr std::size_t max_parameters_ = 32;
	static constexpr int max_parameter_value_ = 65535;
};

} // namespace ginga
