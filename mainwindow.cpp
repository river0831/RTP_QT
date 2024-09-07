#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Reactive Compound Transformation");
    this->setWindowIcon(QIcon(":/Resources/logo2.png"));
    connect(ui->rtp_tool_btn_, SIGNAL(clicked()), this, SLOT(onOpenRTPTool()));
    connect(ui->exit_btn_, SIGNAL(clicked(bool)), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpenRTPTool()
{
    // Display RTP dialog
    RTPToolDialog* dialog = new RTPToolDialog(this);
    dialog->show();
}
