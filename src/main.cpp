#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QWidget window;
    window.setWindowTitle(QStringLiteral("Ginga v0.1"));
    window.resize(640, 400);
    auto* layout = new QVBoxLayout(&window);
    auto* label = new QLabel(QStringLiteral("Ginga v0.1 - Hello Window"), &window);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    window.show();
    return app.exec();
}
