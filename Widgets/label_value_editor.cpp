#include "label_value_editor.h"

LabelValueEditor::LabelValueEditor(QString label_name, QWidget *parent) : QWidget(parent)
{
    QHBoxLayout* h_layout = new QHBoxLayout(this);

    label_ = new QLabel(label_name);
    h_layout->addWidget(label_);

    value_ = new QLineEdit();
    h_layout->addWidget(value_);
}

void LabelValueEditor::setValidator(QValidator *validator)
{
    if (value_ != nullptr)
        value_->setValidator(validator);
}

QString LabelValueEditor::getText()
{
    return value_ != nullptr? value_->text() : "";
}

void LabelValueEditor::setText(QString text)
{
    if (value_ != nullptr)
        value_->setText(text);
}
