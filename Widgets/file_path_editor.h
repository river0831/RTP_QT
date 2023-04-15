#ifndef FILE_PATH_EDITOR_H
#define FILE_PATH_EDITOR_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>

class FilePathEditor : public QWidget
{
    Q_OBJECT
public:
    FilePathEditor(QString name, QWidget *parent = nullptr);

    QString getFilePath();
    void setFilePath(QString path);

signals:

public slots:
   void onBrowse();

private:
   QLabel* name_label_;
   QLineEdit* path_editor_;
   QPushButton* browser_btn_;
};

#endif // FILE_PATH_EDITOR_H
