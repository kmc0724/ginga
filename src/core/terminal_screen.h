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
	const CellAttributes &current_attributes() const;
	void set_attributes(const CellAttributes &attributes);
	void reset_attributes();
	void erase_in_display(int mode);
	void erase_in_line(int mode);
	void resize(int columns, int rows);

	void set_cursor(int column, int row);
	void put_char(char32_t character);
	void write(std::u32string_view text);
	void newline();

	Color current_foreground() const;
	Color current_background() const;

	void set_foreground(Color color);
	void set_background(Color color);

	void reset_foreground();
	void reset_background();
	void reset_style();

private:
	std::size_t index(int column, int row) const;
	void scroll_up();
	void erase_range(std::size_t first, std::size_t last);

	int columns_;
	int rows_;

	int cursor_column_ = 0;
	int cursor_row_ = 0;

	bool wrap_pending_ = false;

	std::vector<TerminalCell> cells_;

	CellAttributes current_attributes_{};

	Color current_foreground_ = default_foreground;
	Color current_background_ = default_background;
};

} // namespace ginga
