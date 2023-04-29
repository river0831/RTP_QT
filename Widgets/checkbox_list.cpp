#include "checkbox_list.h"

CheckboxList::CheckboxList(QWidget* parent) : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    list_ = new QListWidget(this);
    list_->setWindowTitle("Attributes");
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
