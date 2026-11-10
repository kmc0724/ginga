#include <QApplication>

#include "gui/terminal_widget.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	TerminalWidget window;
	window.setWindowTitle("Ginga v0.4.7");
	window.resize(960, 600);
	window.show();

	return app.exec();
}
