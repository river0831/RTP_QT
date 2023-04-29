#ifndef CHECKBOXLIST_H
#define CHECKBOXLIST_H

#include <QWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVector>
#include <QString>

class CheckboxList : public QWidget
{
    Q_OBJECT
public:
    CheckboxList(QWidget* parent = nullptr);

    void updateCheckboxes(
        const QVector<QString>& names,
        const QVector<bool>& check_state
    );

signals:
    void selectionChanged();

private slots:
    void onCheckAll();
    void onCheckboxToggled();

protected:
    QListWidget* list_;
    QPushButton* check_all_btn_;
};

#endif // CHECKBOXLIST_H
