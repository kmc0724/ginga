#include "terminal_screen.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace ginga
{

TerminalScreen::TerminalScreen(int columns, int rows)
    : columns_(columns), rows_(rows)
{
	if (columns <= 0 || rows <= 0)
		throw std::invalid_argument("Invalid terminal dimensions");

	cells_.resize(static_cast<std::size_t>(columns_) *
		      static_cast<std::size_t>(rows_));
}

int TerminalScreen::columns() const
{
	return columns_;
}

int TerminalScreen::rows() const
{
	return rows_;
}

int TerminalScreen::cursor_column() const
{
	return cursor_column_;
}

int TerminalScreen::cursor_row() const
{
	return cursor_row_;
}

std::size_t TerminalScreen::index(int column, int row) const
{
	if (column < 0 || column >= columns_ || row < 0 || row >= rows_)
		throw std::out_of_range("Terminal cell out of bounds");

	return static_cast<std::size_t>(row) *
		   static_cast<std::size_t>(columns_) +
	       static_cast<std::size_t>(column);
}

const TerminalCell &TerminalScreen::cell(int column, int row) const
{
	return cells_[index(column, row)];
}

void TerminalScreen::set_cell(int column, int row, const TerminalCell &cell)
{
	cells_[index(column, row)] = cell;
}

void TerminalScreen::clear()
{
	std::fill(cells_.begin(), cells_.end(), TerminalCell{});
	cursor_column_ = 0;
	cursor_row_ = 0;
	wrap_pending_ = false;
}

void TerminalScreen::resize(int columns, int rows)
{
	if (columns <= 0 || rows <= 0)
		throw std::invalid_argument("Invalid terminal dimensions");

	const auto new_columns = static_cast<std::size_t>(columns);
	const auto new_rows = static_cast<std::size_t>(rows);

	if (new_columns > std::numeric_limits<std::size_t>::max() / new_rows)
		throw std::length_error("Terminal dimensions too large");

	if (columns == columns_ && rows == rows_)
		return;

	std::vector<TerminalCell> new_cells(new_columns * new_rows);

	const int copy_columns = std::min(columns_, columns);
	const int copy_rows = std::min(rows_, rows);

	for (int row = 0; row < copy_rows; ++row) {
		for (int column = 0; column < copy_columns; ++column) {
			const std::size_t new_index =
			    static_cast<std::size_t>(row) * new_columns +
			    static_cast<std::size_t>(column);

			new_cells[new_index] = cell(column, row);
		}
	}

	cells_.swap(new_cells);

	columns_ = columns;
	rows_ = rows;

	cursor_column_ = std::min(cursor_column_, columns_ - 1);
	cursor_row_ = std::min(cursor_row_, rows_ - 1);

	wrap_pending_ = false;
}

void TerminalScreen::set_cursor(int column, int row)
{
	if (column < 0 || column >= columns_ || row < 0 || row >= rows_)
		throw std::out_of_range("Cursor out of bounds");

	cursor_column_ = column;
	cursor_row_ = row;
	wrap_pending_ = false;
}

void TerminalScreen::scroll_up()
{
	for (int row = 1; row < rows_; ++row) {
		for (int column = 0; column < columns_; ++column) {
			cells_[index(column, row - 1)] =
			    cells_[index(column, row)];
		}
	}

	for (int column = 0; column < columns_; ++column)
		cells_[index(column, rows_ - 1)] = TerminalCell{};
}

void TerminalScreen::newline()
{
	wrap_pending_ = false;
	cursor_column_ = 0;

	if (cursor_row_ == rows_ - 1)
		scroll_up();
	else
		++cursor_row_;
}

void TerminalScreen::put_char(char32_t character)
{
	if (character == U'\n') {
		newline();
		return;
	}

	if (character == U'\r') {
		cursor_column_ = 0;
		wrap_pending_ = false;
		return;
	}

	if (wrap_pending_)
		newline();

	TerminalCell new_cell;
	new_cell.character = character;

	set_cell(cursor_column_, cursor_row_, new_cell);

	if (cursor_column_ == columns_ - 1) {
		wrap_pending_ = true;
	} else {
		++cursor_column_;
	}
}

void TerminalScreen::write(std::u32string_view text)
{
	for (char32_t character : text)
		put_char(character);
}

} // namespace ginga
