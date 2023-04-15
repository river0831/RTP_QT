#ifndef LABELVALUEEDITOR_H
#define LABELVALUEEDITOR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QValidator>

class LabelValueEditor : public QWidget
{
    Q_OBJECT
public:
    LabelValueEditor(QString label_name, QWidget *parent = nullptr);

    void setValidator(QValidator* validator);

    QString getText();
    void setText(QString text);

signals:

private:
    QLabel* label_;
    QLineEdit* value_;
};

#endif // LABELVALUEEDITOR_H
