#include "terminal_widget.h"

#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include <algorithm>
#include <utility>

namespace
{

QColor to_qcolor(const ginga::Color &color)
{
	return QColor(color.red, color.green, color.blue);
}

} // namespace

TerminalWidget::TerminalWidget(QWidget *parent) : QWidget(parent)
{
	QFont terminal_font("Consolas", 12);
	terminal_font.setStyleHint(QFont::Monospace);
	terminal_font.setFixedPitch(true);

	setFont(terminal_font);
	setMinimumSize(320, 200);

	update_grid_size();
	populate_demo();
	update();
}

void TerminalWidget::update_grid_size()
{
	const QFontMetrics metrics(font());

	cell_width_ = std::max(1, metrics.horizontalAdvance('M'));
	cell_height_ = std::max(1, metrics.height());

	const int columns = std::max(1, width() / cell_width_);
	const int rows = std::max(1, height() / cell_height_);

	if (columns == screen_.columns() && rows == screen_.rows())
		return;

	screen_.resize(columns, rows);
	populate_demo();
	update();
}

void TerminalWidget::populate_demo()
{
	screen_.clear();
	screen_.reset_style();
	parser_.reset();

	parser_.feed("\x1b[1;38;2;139;233;253m"
		     "Ginga Terminal Engine v0.4.7"
		     "\x1b[0m");

	parser_.feed("\r\n");

	const QString grid_text =
	    QString("Grid: %1 x %2").arg(screen_.columns()).arg(screen_.rows());

	parser_.feed(grid_text.toUtf8().toStdString());
	parser_.feed("\r\n\r\n");

	parser_.feed("\x1b[1;31mBold Red\x1b[0m  ");
	parser_.feed("\x1b[3;36mItalic Cyan\x1b[0m  ");
	parser_.feed("\x1b[4;33mUnderline Yellow\x1b[0m");

	parser_.feed("\r\n\r\n");

	parser_.feed("\x1b[38;5;196m256 Color Red\x1b[0m");
	parser_.feed("\r\n");

	parser_.feed("\x1b[38;2;255;184;108m"
		     "True Color RGB"
		     "\x1b[0m");

	parser_.feed("\r\n");

	parser_.feed("\x1b[7mInverse Text\x1b[0m");

	parser_.feed("\r\n\r\n");

	parser_.feed("\x1b[1;32mginga\x1b[0m $ ");
}

void TerminalWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	update_grid_size();
}

void TerminalWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	painter.fillRect(rect(), to_qcolor(ginga::default_background));

	const QFont base_font = font();
	const QFontMetrics base_metrics(base_font);
	const int baseline = base_metrics.ascent();

	for (int row = 0; row < screen_.rows(); ++row) {
		for (int column = 0; column < screen_.columns(); ++column) {
			const ginga::TerminalCell &cell =
			    screen_.cell(column, row);

			const int x = column * cell_width_;
			const int y = row * cell_height_;

			ginga::Color foreground = cell.foreground;
			ginga::Color background = cell.background;

			if (cell.attributes.inverse)
				std::swap(foreground, background);

			if (background != ginga::default_background) {
				painter.fillRect(x,
						 y,
						 cell_width_,
						 cell_height_,
						 to_qcolor(background));
			}

			if (cell.character == U' ')
				continue;

			QFont character_font = base_font;

			character_font.setBold(cell.attributes.bold);
			character_font.setItalic(cell.attributes.italic);
			character_font.setUnderline(cell.attributes.underline);

			painter.setFont(character_font);
			painter.setPen(to_qcolor(foreground));

			const char32_t character = cell.character;
			const QString text = QString::fromUcs4(&character, 1);

			painter.drawText(x, y + baseline, text);
		}
	}

	// Restore the base font after styled rendering.
	painter.setFont(base_font);

	const int cursor_column = screen_.cursor_column();
	const int cursor_row = screen_.cursor_row();

	painter.fillRect(cursor_column * cell_width_,
			 cursor_row * cell_height_,
			 cell_width_,
			 cell_height_,
			 to_qcolor(ginga::default_foreground));
}
