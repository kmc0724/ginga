
#include "terminal_widget.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include <algorithm>

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

	int new_columns = width() / cell_width_;
	int new_rows = height() / cell_height_;

	if (new_columns == columns_ && new_rows == rows_)
		return;

	columns_ = new_columns;
	rows_ = new_rows;

	clear_grid();
	populate_demo();
	update();
}

void TerminalWidget::clear_grid()
{
	cells_.assign(columns_ * rows_, Cell{});
}

void TerminalWidget::put_text(int column, int row, const QString &text)
{
	if (row < 0 || row >= rows_)
		return;

	for (QChar ch : text) {
		if (column >= columns_)
			break;

		if (column >= 0 && ch.unicode() < 128) {
			Cell &cell = cells_[row * columns_ + column];
			cell.character = ch.unicode();
		}

		++column;
	}
}

void TerminalWidget::populate_demo()
{
	put_text(0, 0, "Ginga Terminal Engine v0.2");

	put_text(0, 1, QString("Grid: %1 x %2").arg(columns_).arg(rows_));

	put_text(0, 3, "ginga $");

	cursor_column_ = 7;
	cursor_row_ = 3;
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
	painter.fillRect(rect(), QColor(16, 20, 27));
	painter.setFont(font());

	QFontMetrics metrics(font());
	int baseline = metrics.ascent();

	for (int row = 0; row < rows_; ++row) {
		for (int column = 0; column < columns_; ++column) {
			const Cell &cell = cells_[row * columns_ + column];

			int x = column * cell_width_;
			int y = row * cell_height_;

			if (cell.background != QColor(16, 20, 27)) {
				painter.fillRect(x,
						 y,
						 cell_width_,
						 cell_height_,
						 cell.background);
			}

			if (cell.character == U' ')
				continue;

			painter.setPen(cell.foreground);

			painter.drawText(x,
					 y + baseline,
					 QString(QChar(static_cast<ushort>(
					     cell.character))));
		}
	}

	if (cursor_column_ >= 0 && cursor_column_ < columns_ &&
	    cursor_row_ >= 0 && cursor_row_ < rows_) {
		painter.fillRect(cursor_column_ * cell_width_,
				 cursor_row_ * cell_height_,
				 cell_width_,
				 cell_height_,
				 QColor(220, 228, 239));
	}
}
