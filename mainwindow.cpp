#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "KEncodePNG.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MainWindow::MainWindow(QWidget * pParent)
    : QMainWindow(pParent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MainWindow::~MainWindow()
{
    delete ui;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void MainWindow::on_encodeButton_clicked()
{
    QImage test_image(":/images/Mac-Screen-Shot.png");
    KEncodePNG encoder(test_image);
    encoder.analyzeColor();
}
