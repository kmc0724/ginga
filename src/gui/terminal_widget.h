
#pragma once

#include <QColor>
#include <QString>
#include <QWidget>

#include <vector>

class QPaintEvent;
class QResizeEvent;

class TerminalWidget : public QWidget
{
public:
	explicit TerminalWidget(QWidget *parent = nullptr);

protected:
	void paintEvent(QPaintEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;

private:
	struct Cell {
		char32_t character = U' ';
		QColor foreground = QColor(220, 228, 239);
		QColor background = QColor(16, 20, 27);
	};

	void update_grid_size();
	void clear_grid();
	void put_text(int column, int row, const QString &text);
	void populate_demo();

	int cell_width_ = 1;
	int cell_height_ = 1;
	int columns_ = 0;
	int rows_ = 0;

	int cursor_column_ = 0;
	int cursor_row_ = 0;

	std::vector<Cell> cells_;
};
