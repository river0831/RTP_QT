#include "table_viewer.h"

// Constructors
TableViewer::TableViewer(QWidget *parent) : QWidget(parent)
{

}

// Constructors
TableViewer::TableViewer(
    const QVector<QString> &header,
    const QVector<QVector<QString>> &content,
    QWidget *parent
) : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    // List all the headers in list_
    list_ = new QListWidget(this);
    list_->setWindowTitle("Attributes");
    splitter->addWidget(list_);

    // Initialize the table and fill in the contents
    table_ = new QTableWidget(this);
    splitter->addWidget(table_);

    layout->addWidget(splitter);

    updateTable(header, content);
}

/*
    Update the table and list widget using the provided header and content.
*/
bool TableViewer::updateTable(
    const QVector<QString>& header, // Defines the header of a table
    const QVector<QVector<QString>>& content // Content to display
) {
    table_->hide();

    list_->clear();
    for (int i = 0; i < header.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem();
        list_->addItem(item);
        QCheckBox* cb = new QCheckBox(header[i]);
        cb->setChecked(true);
        list_->setItemWidget(item, cb);
        connect(cb, SIGNAL(toggled(bool)), this, SLOT(onVisibilityChanged()));
    }

    table_->setRowCount(content.size());
    table_->setColumnCount(header.size());
    QStringList hrz_labels;
    QStringList ver_labels;
    for (int i = 0; i < header.size(); ++i)
        hrz_labels << header[i];
    for (int i = 1; i <= content.size(); ++i)
        ver_labels << QString::number(i);
    table_->setHorizontalHeaderLabels(hrz_labels);
    table_->setVerticalHeaderLabels(ver_labels);
    for (int i = 0; i < content.size(); ++i) {
        for (int j = 0; j < content[i].size(); ++j) {
            table_->setItem(i, j, new QTableWidgetItem(QString(content[i][j])));
        }
    }

    table_->show();
}

void TableViewer::getListAttributes(QVector<QString>& attrs, QVector<bool>& check_status)
{
    if (list_ == nullptr)
        return;
    attrs.resize(list_->count());
    check_status.resize(list_->count());
    for (int i = 0; i < list_->count(); ++i) {
        QListWidgetItem* lw = list_->item(i);
        QCheckBox* cb = dynamic_cast<QCheckBox*>(list_->itemWidget(lw));
        attrs[i] = cb->text();
        check_status[i] = cb->isChecked();
    }
}

void TableViewer::updateColumnVisibility(const QVector<bool>& vis)
{
    if (table_->columnCount() != vis.size())
        return;
    table_->hide();
    for (int i = 0; i < vis.size(); ++i) {
        if (vis[i])
            table_->showColumn(i);
        else
            table_->hideColumn(i);
    }
    table_->show();
}


/*
    Reset the list and table: clear the list and reset the table to
    10 rows and 2 columns.
*/
void TableViewer::reset()
{
    int def_nb_rows = 10;
    int def_nb_cols = 2;
    list_->clear();
    table_->setRowCount(def_nb_rows);
    table_->setColumnCount(def_nb_cols);
    for (int i = 0; i < table_->rowCount(); ++i) {
        for (int j = 0; j < table_->columnCount(); ++j) {
            table_->item(i, j)->setText("");
        }
    }
}

/*
 * When user toggles the visiblity of attributes in the list, this function is called to update the table.
 */
void TableViewer::onVisibilityChanged()
{
    QVector<QString> attrs;
    QVector<bool> check_status;
    getListAttributes(attrs, check_status);
    updateColumnVisibility(check_status);
}
