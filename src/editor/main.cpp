#include <QApplication>
#include <QMainWindow>

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QMainWindow window;
    
    window.resize(1280, 720);
    window.setWindowTitle("Vivianite Editor");
    window.show();

    return app.exec();
}
