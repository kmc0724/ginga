
#include "terminal_screen.h"

#include <iostream>
#include <stdexcept>
#include <string>

using ginga::TerminalCell;
using ginga::TerminalScreen;

void check(bool condition, const char *message)
{
	if (!condition)
		throw std::runtime_error(message);
}

int main()
{
	try {
		TerminalScreen screen(5, 3);

		check(screen.columns() == 5, "Invalid columns");
		check(screen.rows() == 3, "Invalid rows");

		TerminalCell cell;
		cell.character = U'A';
		screen.set_cell(0, 0, cell);

		check(screen.cell(0, 0).character == U'A',
		      "Cell write/read failed");

		screen.clear();
		check(screen.cell(0, 0).character == U' ', "Clear failed");
		check(screen.cursor_column() == 0 && screen.cursor_row() == 0,
		      "Cursor reset failed");

		screen.write(U"HELLO");
		check(screen.cell(4, 0).character == U'O',
		      "Last column write failed");

		screen.put_char(U'X');
		check(screen.cell(0, 1).character == U'X', "Auto wrap failed");

		screen.set_cursor(0, 2);
		screen.write(U"ABCDE");
		screen.put_char(U'Z');

		check(screen.cell(0, 2).character == U'Z',
		      "Scroll/write failed");
		check(screen.cell(0, 0).character == U'X',
		      "Scroll preservation failed");

		screen.resize(8, 4);
		check(screen.columns() == 8 && screen.rows() == 4,
		      "Resize dimensions failed");
		check(screen.cell(0, 2).character == U'Z',
		      "Resize preservation failed");

		screen.clear();
		check(screen.cell(0, 0).character == U' ',
		      "Final clear failed");

		std::cout << "TerminalScreen tests passed\n";
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "TerminalScreen test failed: " << error.what()
			  << '\n';
		return 1;
	}
}
