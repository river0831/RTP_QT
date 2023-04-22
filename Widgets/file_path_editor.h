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
    enum PathMode {
      OPEN_FILE,
      OPEN_DIRECTORY,
      SAVE_FILE
    };

public:
    FilePathEditor(QString name, QWidget *parent = nullptr);

    QString getFilePath();
    void setFilePath(QString path);
    void setMode(PathMode mode);

signals:

public slots:
   void onBrowse();

private:
   QLabel* name_label_;
   QLineEdit* path_editor_;
   QPushButton* browser_btn_;
   PathMode mode_;
};

#endif // FILE_PATH_EDITOR_H
