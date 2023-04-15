#include "file_path_editor.h"

#include <QDebug>

FilePathEditor::FilePathEditor(QString name, QWidget *parent) : QWidget(parent)
{
    QHBoxLayout* h_layout = new QHBoxLayout(this);

    name_label_ = new QLabel(name);
    h_layout->addWidget(name_label_);

    path_editor_ = new QLineEdit();
    h_layout->addWidget(path_editor_);

    browser_btn_ = new QPushButton("Browse");
    h_layout->addWidget(browser_btn_);
    connect(browser_btn_, SIGNAL(clicked()), this, SLOT(onBrowse()));
}

QString FilePathEditor::getFilePath()
{
    return path_editor_ != nullptr? path_editor_->text() : QString("");
}

void FilePathEditor::setFilePath(QString path)
{
    if (path_editor_ != nullptr)
        path_editor_->setText(path);
}

void FilePathEditor::onBrowse()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Open file");
    if (!file_name.isEmpty())
        path_editor_->setText(file_name);
}
