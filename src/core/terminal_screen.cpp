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

const CellAttributes &TerminalScreen::current_attributes() const
{
	return current_attributes_;
}

void TerminalScreen::set_attributes(const CellAttributes &attributes)
{
	current_attributes_ = attributes;
}

void TerminalScreen::reset_attributes()
{
	current_attributes_ = CellAttributes{};
}

void TerminalScreen::erase_range(std::size_t first, std::size_t last)
{
	std::fill(
	    cells_.begin() + first, cells_.begin() + last, TerminalCell{});
}

void TerminalScreen::erase_in_display(int mode)
{
	const std::size_t cursor = index(cursor_column_, cursor_row_);

	switch (mode) {
	case 0: // Cursor to end of display
		erase_range(cursor, cells_.size());
		break;

	case 1: // Start of display through cursor
		erase_range(0, cursor + 1);
		break;

	case 2: // Entire display
		erase_range(0, cells_.size());
		break;

	default:
		break;
	}
}

void TerminalScreen::erase_in_line(int mode)
{
	const std::size_t row_start =
	    static_cast<std::size_t>(cursor_row_) * columns_;
	const std::size_t row_end = row_start + columns_;
	const std::size_t cursor = index(cursor_column_, cursor_row_);

	switch (mode) {
	case 0: // Cursor to end of line
		erase_range(cursor, row_end);
		break;

	case 1: // Start of line through cursor
		erase_range(row_start, cursor + 1);
		break;

	case 2: // Entire line
		erase_range(row_start, row_end);
		break;

	default:
		break;
	}
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
	new_cell.attributes = current_attributes_;
	new_cell.foreground = current_foreground_;
	new_cell.background = current_background_;

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

Color TerminalScreen::current_foreground() const
{
	return current_foreground_;
}

Color TerminalScreen::current_background() const
{
	return current_background_;
}

void TerminalScreen::set_foreground(Color color)
{
	current_foreground_ = color;
}

void TerminalScreen::set_background(Color color)
{
	current_background_ = color;
}

void TerminalScreen::reset_foreground()
{
	current_foreground_ = default_foreground;
}

void TerminalScreen::reset_background()
{
	current_background_ = default_background;
}

void TerminalScreen::reset_style()
{
	reset_attributes();
	reset_foreground();
	reset_background();
}

} // namespace ginga
