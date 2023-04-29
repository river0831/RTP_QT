#include "checkbox_list.h"

CheckboxList::CheckboxList(QString title, QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    list_ = new QListWidget(this);
    list_->setWindowTitle(title);
    layout->addWidget(list_);

    check_all_btn_ = new QPushButton("Check all");
    layout->addWidget(check_all_btn_);
}

void CheckboxList::updateCheckboxes(
    const QVector<QString>& names,
    const QVector<bool>& check_state
) {
    list_->clear();
    for (int i = 0; i < names.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem();
        list_->addItem(item);
        QCheckBox* cb = new QCheckBox(names[i]);
        cb->setChecked(check_state[i]);
        list_->setItemWidget(item, cb);
        connect(cb, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled()));
    }
}

void CheckboxList::getItemNames(QVector<QString>& names)
{
    int nb_items = list_->count();
    names.resize(nb_items);
    for (int i = 0; i < nb_items; ++i) {
        QListWidgetItem* lw = list_->item(i);
        QCheckBox* cb = dynamic_cast<QCheckBox*>(list_->itemWidget(lw));
        names[i] = cb->text();
    }
}

void CheckboxList::getItemCheckState(QVector<bool>& state)
{
    int nb_items = list_->count();
    state.resize(nb_items);
    for (int i = 0; i < nb_items; ++i) {
        QListWidgetItem* lw = list_->item(i);
        QCheckBox* cb = dynamic_cast<QCheckBox*>(list_->itemWidget(lw));
        state[i] = cb->isChecked();
    }
}

void CheckboxList::clear()
{
    list_->clear();
}

void CheckboxList::onCheckAll()
{
    int nb_items = list_->count();
    for (int i = 0; i < nb_items; ++i) {
        QListWidgetItem* lw = list_->item(i);
        QCheckBox* cb = dynamic_cast<QCheckBox*>(list_->itemWidget(lw));
        cb->setChecked(true);
    }

    emit selectionChanged();
}

void CheckboxList::onCheckboxToggled()
{
    emit selectionChanged();
}

void CheckboxList::disconnectCheckboxes()
{
    int nb_items = list_->count();
    for (int i = 0; i < nb_items; ++i) {
        QListWidgetItem* item = list_->item(i);
        QCheckBox* cb = dynamic_cast<QCheckBox*>(item);
        disconnect(cb, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled()));
    }
}

void CheckboxList::reconnectCheckboxes()
{
    int nb_items = list_->count();
    for (int i = 0; i < nb_items; ++i) {
        QListWidgetItem* item = list_->item(i);
        QCheckBox* cb = dynamic_cast<QCheckBox*>(item);
        connect(cb, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled()));
    }
}
