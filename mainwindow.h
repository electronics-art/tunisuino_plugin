#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QLabel>
#include <QTextStream>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_path_button_clicked();

    void on_install_button_clicked();

private:
    Ui::MainWindow *ui;
    void print_info(QString);
    void print_warning(QString);

    static const QString boards_txt_path;
    static const QString usbcore_cpp_path;
    static const QString caterina_path;
    static const QString tunisuino_board;
    static const QString tunisuino_iproduct;
    static const QString tunisuino_imanufacture;

};

#endif // MAINWINDOW_H
