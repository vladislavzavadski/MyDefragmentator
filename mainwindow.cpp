#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    WCHAR buffer[] = L"";
    WCHAR buff[] = L"";
    DWORD write;
    char buf[1024];
    ui->setupUi(this);
    hFindVolume = FindFirstVolume(buffer, ARRAYSIZE(buffer));
    GetVolumePathNamesForVolumeName(buffer, buff, ARRAYSIZE(buffer), &write);
    sprintf(buf, "%s", buff);

    ui->volume->addItem(buf);
}



MainWindow::~MainWindow()
{
    delete ui;
}