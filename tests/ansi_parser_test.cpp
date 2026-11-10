#include "ansi_parser.h"
#include "terminal_screen.h"

#include <iostream>
#include <stdexcept>

using ginga::AnsiParser;
using ginga::TerminalScreen;

void check(bool condition, const char *message)
{
	if (!condition)
		throw std::runtime_error(message);
}

int main()
{
	try {
		TerminalScreen screen(40, 10);
		AnsiParser parser(screen);

		// Split CSI sequence.
		parser.feed("Hello ");
		parser.feed("\x1b");
		parser.feed("[31");
		parser.feed("mWorld");

		check(screen.cell(0, 0).character == U'H', "Plain text failed");
		check(screen.cell(6, 0).character == U'W',
		      "Split CSI handling failed");

		// Split UTF-8 sequence.
		screen.clear();
		parser.reset();

		parser.feed("\xEA");
		parser.feed("\xB0");
		parser.feed("\x80");

		check(screen.cell(0, 0).character == U'\uAC00',
		      "Split UTF-8 failed");

		// OSC terminated by BEL.
		screen.clear();
		parser.reset();

		parser.feed("A\x1b]0;Hidden");
		parser.feed("\x07"
			    "B");

		check(screen.cell(0, 0).character == U'A', "OSC prefix failed");
		check(screen.cell(1, 0).character == U'B',
		      "OSC BEL termination failed");

		// OSC terminated by ESC backslash.
		screen.clear();
		parser.reset();

		parser.feed("A\x1b]0;Hidden\x1b");
		parser.feed("\\B");

		check(screen.cell(1, 0).character == U'B',
		      "OSC ST termination failed");

		// Invalid UTF-8 followed by ESC.
		screen.clear();
		parser.reset();

		parser.feed("\xEA");
		parser.feed("\x1b[0mX");

		check(screen.cell(0, 0).character == U'\uFFFD',
		      "Incomplete UTF-8 replacement failed");
		check(screen.cell(1, 0).character == U'X',
		      "Recovery after ESC failed");

		// Unsupported private CSI must be ignored.
		screen.clear();
		parser.reset();

		parser.feed("A\x1b[?25lB");

		check(screen.cell(0, 0).character == U'A',
		      "Private CSI prefix failed");
		check(screen.cell(1, 0).character == U'B',
		      "Private CSI ignore failed");

		// Oversized CSI parameter must not leak text.
		screen.clear();
		parser.reset();

		parser.feed("A\x1b[999999999mB");

		check(screen.cell(1, 0).character == U'B',
		      "Oversized CSI handling failed");

		// Cursor movement: CUU, CUD, CUF, CUB.
		screen.clear();
		parser.reset();

		parser.feed("\x1b[3;5H");
		check(screen.cursor_row() == 2 && screen.cursor_column() == 4,
		      "CUP positioning failed");

		parser.feed("\x1b[2A");
		check(screen.cursor_row() == 0, "CUU failed");

		parser.feed("\x1b[4B");
		check(screen.cursor_row() == 4, "CUD failed");

		parser.feed("\x1b[3D");
		check(screen.cursor_column() == 1, "CUB failed");

		parser.feed("\x1b[2C");
		check(screen.cursor_column() == 3, "CUF failed");

		// Absolute positioning.
		parser.feed("\x1b[10G");
		check(screen.cursor_column() == 9, "CHA failed");

		parser.feed("\x1b[7d");
		check(screen.cursor_row() == 6, "VPA failed");

		// Home position.
		parser.feed("\x1b[H");
		check(screen.cursor_row() == 0 && screen.cursor_column() == 0,
		      "Home positioning failed");

		// Cursor movement followed by text overwrite.
		screen.clear();
		parser.reset();

		parser.feed("Hello");
		parser.feed("\x1b[2D");
		parser.feed("XX");

		check(screen.cell(0, 0).character == U'H' &&
			  screen.cell(1, 0).character == U'e' &&
			  screen.cell(2, 0).character == U'l' &&
			  screen.cell(3, 0).character == U'X' &&
			  screen.cell(4, 0).character == U'X',
		      "Cursor overwrite failed");

		// Clamp cursor to screen boundaries.
		screen.clear();
		parser.reset();

		parser.feed("\x1b[999B");
		check(screen.cursor_row() == screen.rows() - 1,
		      "Cursor bottom clamp failed");

		parser.feed("\x1b[999C");
		check(screen.cursor_column() == screen.columns() - 1,
		      "Cursor right clamp failed");

		// Zero parameters use the default value.
		parser.feed("\x1b[0A");
		check(screen.cursor_row() == screen.rows() - 2,
		      "Zero parameter default failed");

		// ED 0: erase from cursor to end of display.
		screen.clear();
		parser.reset();
		parser.feed("ABCDE");
		parser.feed("\x1b[1;3H\x1b[0J");

		check(screen.cell(0, 0).character == U'A' &&
			  screen.cell(1, 0).character == U'B' &&
			  screen.cell(2, 0).character == U' ',
		      "ED 0 failed");

		check(screen.cursor_column() == 2 && screen.cursor_row() == 0,
		      "ED 0 moved cursor");

		// ED 1: erase from start through cursor.
		screen.clear();
		parser.reset();
		parser.feed("ABCDE");
		parser.feed("\x1b[1;3H\x1b[1J");

		check(screen.cell(0, 0).character == U' ' &&
			  screen.cell(2, 0).character == U' ' &&
			  screen.cell(3, 0).character == U'D',
		      "ED 1 failed");

		// ED 2: erase entire display without moving cursor.
		screen.clear();
		parser.reset();
		parser.feed("ABCDE");
		parser.feed("\x1b[1;3H\x1b[2J");

		check(screen.cell(0, 0).character == U' ' &&
			  screen.cell(4, 0).character == U' ',
		      "ED 2 failed");

		check(screen.cursor_column() == 2 && screen.cursor_row() == 0,
		      "ED 2 moved cursor");

		// EL 0: erase from cursor to end of line.
		screen.clear();
		parser.reset();
		parser.feed("ABCDE");
		parser.feed("\x1b[1;3H\x1b[0K");

		check(screen.cell(1, 0).character == U'B' &&
			  screen.cell(2, 0).character == U' ' &&
			  screen.cell(4, 0).character == U' ',
		      "EL 0 failed");

		// EL 1: erase from start of line through cursor.
		screen.clear();
		parser.reset();
		parser.feed("ABCDE");
		parser.feed("\x1b[1;3H\x1b[1K");

		check(screen.cell(0, 0).character == U' ' &&
			  screen.cell(2, 0).character == U' ' &&
			  screen.cell(3, 0).character == U'D',
		      "EL 1 failed");

		// EL 2: erase entire line.
		screen.clear();
		parser.reset();
		parser.feed("ABCDE");
		parser.feed("\x1b[1;3H\x1b[2K");

		check(screen.cell(0, 0).character == U' ' &&
			  screen.cell(4, 0).character == U' ',
		      "EL 2 failed");

		check(screen.cursor_column() == 2 && screen.cursor_row() == 0,
		      "EL 2 moved cursor");

		// Omitted parameters default to mode 0.
		screen.clear();
		parser.reset();
		parser.feed("ABCDE");
		parser.feed("\x1b[1;3H\x1b[K");

		check(screen.cell(1, 0).character == U'B' &&
			  screen.cell(2, 0).character == U' ',
		      "EL default parameter failed");

		// Unsupported erase modes must not change the screen.
		screen.clear();
		parser.reset();
		parser.feed("ABCDE");
		parser.feed("\x1b[1;3H\x1b[9K");

		check(screen.cell(2, 0).character == U'C',
		      "Unsupported EL mode changed screen");

		// SGR: Bold on and reset.
		screen.clear();
		screen.reset_attributes();
		parser.reset();

		parser.feed("A\x1b[1mB\x1b[0mC");

		check(!screen.cell(0, 0).attributes.bold,
		      "SGR normal text failed");
		check(screen.cell(1, 0).attributes.bold, "SGR bold failed");
		check(!screen.cell(2, 0).attributes.bold, "SGR reset failed");

		// SGR: Multiple attributes.
		screen.clear();
		screen.reset_attributes();
		parser.reset();

		parser.feed("\x1b[1;3;4;7mX");

		const auto &styled = screen.cell(0, 0).attributes;

		check(styled.bold && styled.italic && styled.underline &&
			  styled.inverse,
		      "SGR combined attributes failed");

		// SGR: Selective attribute reset.
		parser.feed("\x1b[22;24mY");

		const auto &partial = screen.cell(1, 0).attributes;

		check(!partial.bold && partial.italic && !partial.underline &&
			  partial.inverse,
		      "SGR selective reset failed");

		// SGR: Remaining attributes off.
		parser.feed("\x1b[23;27mZ");

		const auto &normal = screen.cell(2, 0).attributes;

		check(!normal.bold && !normal.italic && !normal.underline &&
			  !normal.inverse,
		      "SGR attribute disable failed");

		// SGR: Empty parameter resets attributes.
		screen.clear();
		screen.reset_attributes();
		parser.reset();

		parser.feed("\x1b[1mA\x1b[mB");

		check(screen.cell(0, 0).attributes.bold &&
			  !screen.cell(1, 0).attributes.bold,
		      "SGR empty parameter reset failed");

		// SGR must not modify previously written cells.
		screen.clear();
		screen.reset_attributes();
		parser.reset();

		parser.feed("\x1b[1mA\x1b[0mB");

		check(screen.cell(0, 0).attributes.bold &&
			  !screen.cell(1, 0).attributes.bold,
		      "SGR changed previous cell attributes");

		// ANSI 16-color foreground.
		screen.clear();
		screen.reset_style();
		parser.reset();

		parser.feed("\x1b[31mR\x1b[39mN");

		check(screen.cell(0, 0).foreground == ginga::Color{205, 49, 49},
		      "ANSI foreground color failed");

		check(screen.cell(1, 0).foreground == ginga::default_foreground,
		      "ANSI foreground reset failed");

		// ANSI background color.
		screen.clear();
		screen.reset_style();
		parser.reset();

		parser.feed("\x1b[44mB\x1b[49mN");

		check(screen.cell(0, 0).background ==
			  ginga::Color{36, 114, 200},
		      "ANSI background color failed");

		check(screen.cell(1, 0).background == ginga::default_background,
		      "ANSI background reset failed");

		// 256-color foreground.
		screen.clear();
		screen.reset_style();
		parser.reset();

		parser.feed("\x1b[38;5;196mX");

		check(screen.cell(0, 0).foreground == ginga::Color{255, 0, 0},
		      "ANSI 256-color failed");

		// True Color foreground and background.
		screen.clear();
		screen.reset_style();
		parser.reset();

		parser.feed("\x1b[38;2;255;100;0;48;2;10;20;30mX");

		check(screen.cell(0, 0).foreground == ginga::Color{255, 100, 0},
		      "True Color foreground failed");

		check(screen.cell(0, 0).background == ginga::Color{10, 20, 30},
		      "True Color background failed");

		// SGR reset must reset attributes and colors.
		screen.clear();
		screen.reset_style();
		parser.reset();

		parser.feed("\x1b[1;31;44mA\x1b[0mB");

		check(screen.cell(0, 0).attributes.bold,
		      "Combined SGR bold failed");

		check(screen.cell(1, 0).foreground ==
			      ginga::default_foreground &&
			  screen.cell(1, 0).background ==
			      ginga::default_background &&
			  !screen.cell(1, 0).attributes.bold,
		      "SGR full reset failed");

		// Invalid 256-color index must not modify colors.
		screen.clear();
		screen.reset_style();
		parser.reset();

		parser.feed("\x1b[38;5;999mX");

		check(screen.cell(0, 0).foreground == ginga::default_foreground,
		      "Invalid color index changed foreground");

		// Split True Color sequence across feeds.
		screen.clear();
		screen.reset_style();
		parser.reset();

		parser.feed("\x1b[38;2;255;");
		parser.feed("128;64mX");

		check(screen.cell(0, 0).foreground ==
			  ginga::Color{255, 128, 64},
		      "Split True Color sequence failed");

		std::cout << "AnsiParser tests passed\n";
		return 0;
	} catch (const std::exception &error) {
		std::cerr << "AnsiParser test failed: " << error.what() << '\n';
		return 1;
	}
}
