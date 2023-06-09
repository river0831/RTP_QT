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
    CheckboxList(QString title, QWidget* parent = nullptr);

    void updateCheckboxes(
        const QVector<QString>& names,
        const QVector<bool>& check_state
    );

    void getItemNames(QVector<QString>& names);
    void getItemCheckState(QVector<bool>& state);

    void clear();

signals:
    void selectionChanged();

private slots:
    void onCheckAll();
    void onCheckboxToggled();

private:
    void disconnectCheckboxes();
    void reconnectCheckboxes();

protected:
    QListWidget* list_;
    QPushButton* check_all_btn_;
};

#endif // CHECKBOXLIST_H
