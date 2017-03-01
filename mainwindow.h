#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *pParent = 0);
    ~MainWindow();

private slots:
    void on_encodeButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
