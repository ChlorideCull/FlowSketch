#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <fstream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

std::fstream* openFileUI(QWidget* parent, bool save, QString filter, QString* name = 0) {
    QFileDialog dialog (parent);
    dialog.setFileMode(save ? QFileDialog::FileMode::AnyFile : QFileDialog::FileMode::ExistingFile);
    dialog.setNameFilter(filter);
    dialog.setAcceptMode(save ? QFileDialog::AcceptMode::AcceptSave : QFileDialog::AcceptMode::AcceptOpen);
    if (dialog.exec()) {
        if (name != nullptr)
            *name = dialog.selectedFiles().front();
        std::fstream* stream = new std::fstream();
        stream->open(dialog.selectedFiles().front().toStdString(), save ? std::ios_base::out : std::ios_base::in);
        return stream;
    } else {
        return nullptr;
    }
}

void MainWindow::openLoader() {
    QString fname = "";
    std::fstream* file = openFileUI(this, false, "Sketch XML (*.sketch *.xml)", &fname);
    if (file == nullptr)
        return;
    ui->widget->LoadXML(*file);
    //TODO: Generate Tree from nodes
    setWindowTitle("Flow Sketch - " + fname);
    file->close();
}

void MainWindow::openSaver() {
    std::fstream* file = openFileUI(this, true, "Sketch XML (*.sketch *.xml)");
    if (file == nullptr)
        return;
    ui->widget->SaveXML(*file);
    file->close();
}

void MainWindow::openExportSaver() {
    std::fstream* file = openFileUI(this, true, "Scaleable Vector Graphics (*.svg)");
    if (file == nullptr)
        return;
    ui->widget->SaveSVG(*file);
    file->close();
}
