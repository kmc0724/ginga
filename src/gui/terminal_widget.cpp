#include "terminal_widget.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include <algorithm>

namespace
{

constexpr ginga::Color default_background{16, 20, 27};

QColor to_qcolor(const ginga::Color &color)
{
	return QColor(color.red, color.green, color.blue);
}

} // namespace

TerminalWidget::TerminalWidget(QWidget *parent) : QWidget(parent)
{
	QFont terminal_font("Consolas", 12);
	terminal_font.setStyleHint(QFont::Monospace);

	setFont(terminal_font);
	setMinimumSize(320, 200);

	update_grid_size();
}

void TerminalWidget::update_grid_size()
{
	QFontMetrics metrics(font());

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

	screen_.write(U"Ginga Terminal Engine v0.3");
	screen_.newline();

	const QString grid_text =
	    QString("Grid: %1 x %2").arg(screen_.columns()).arg(screen_.rows());

	screen_.write(grid_text.toStdU32String());

	screen_.set_cursor(0, 3);
	screen_.write(U"ginga $");
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
	painter.fillRect(rect(), to_qcolor(default_background));
	painter.setFont(font());

	QFontMetrics metrics(font());
	const int baseline = metrics.ascent();

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

			if (background != default_background) {
				painter.fillRect(x,
						 y,
						 cell_width_,
						 cell_height_,
						 to_qcolor(background));
			}

			if (cell.character == U' ')
				continue;

			painter.setPen(to_qcolor(foreground));

			const char32_t character = cell.character;

			const QString text = QString::fromUcs4(&character, 1);

			painter.drawText(x, y + baseline, text);
		}
	}

	const int cursor_column = screen_.cursor_column();
	const int cursor_row = screen_.cursor_row();

	painter.fillRect(cursor_column * cell_width_,
			 cursor_row * cell_height_,
			 cell_width_,
			 cell_height_,
			 QColor(220, 228, 239));
}
