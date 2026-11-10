
#pragma once

#include "ansi_parser.h"
#include "terminal_screen.h"

#include <QString>
#include <QWidget>

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
	void update_grid_size();
	void populate_demo();

	ginga::TerminalScreen screen_{1, 1};
	ginga::AnsiParser parser_{screen_};

	int cell_width_ = 1;
	int cell_height_ = 1;
};
