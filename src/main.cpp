#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Ajustes visuales de la aplicación
    QApplication::setApplicationName("TetrisQt6");
    QApplication::setApplicationDisplayName("Tetris Qt6");

    MainWindow window;
    window.show();

    return app.exec();
}