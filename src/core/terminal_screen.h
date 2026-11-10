#pragma once

#include "terminal_cell.h"

#include <cstddef>
#include <string_view>
#include <vector>

namespace ginga
{

class TerminalScreen
{
public:
	TerminalScreen(int columns, int rows);

	int columns() const;
	int rows() const;

	int cursor_column() const;
	int cursor_row() const;

	const TerminalCell &cell(int column, int row) const;

	void set_cell(int column, int row, const TerminalCell &cell);
	void clear();
	void resize(int columns, int rows);

	void set_cursor(int column, int row);
	void put_char(char32_t character);
	void write(std::u32string_view text);
	void newline();

private:
	std::size_t index(int column, int row) const;
	void scroll_up();

	int columns_;
	int rows_;

	int cursor_column_ = 0;
	int cursor_row_ = 0;

	bool wrap_pending_ = false;

	std::vector<TerminalCell> cells_;
};

} // namespace ginga
