#include "table_viewer.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include "FileIO/file_io.h"

/*
 *      class: TableViewer
 *  This class creates a widget with a table and a list of the headers.
 */
// Constructors
TableViewer::TableViewer(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    // List all the headers in attr_list_
    QWidget* attr_list_widget = new QWidget();
    QHBoxLayout* attr_list_layout = new QHBoxLayout();
    attr_list_widget->setLayout(attr_list_layout);
    attr_list_layout->setContentsMargins(0, 0, 0, 0);
    attr_list_ = new CheckboxList("Attributes");
    attr_list_layout->addWidget(attr_list_);
    splitter->addWidget(attr_list_widget);

    // Initialize the table and fill in the contents
    table_ = new QTableWidget(this);
    splitter->addWidget(table_);

    layout->addWidget(splitter);

    reset();

    connect(attr_list_, SIGNAL(selectionChanged()), this, SLOT(onAttrSelectionChanged()));
}

/*
    Update the table and list widget using the provided header and content.
*/
bool TableViewer::updateTable(
    const QVector<QString>& header, // Defines the header of a table
    const QVector<QVector<QString>>& content // Content to display
) {
    if (table_ == nullptr || attr_list_ == nullptr)
        return false;

    table_->hide();
    attr_list_->clear();

    QVector<bool> check_state(header.size(), true);
    attr_list_->updateCheckboxes(header, check_state);

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
    return true;
}

bool TableViewer::getTableContent(
    QVector<QString>* headers,
    QVector<QVector<QString>>* rows,
    QVector<int>* hidden_cols,
    QVector<int>* hidden_rows
){
    if (table_ == nullptr)
        return false;
    int nb_cols = table_->columnCount();
    int nb_rows = table_->rowCount();
    QVector<QString> hdrs(nb_cols);
    QVector<QVector<QString>> table_content(nb_rows);
    QVector<int> hidden_cols_idx;
    QVector<int> hidden_row_idx;
    for (int i = 0; i < nb_cols; ++i) {
        if (table_->isColumnHidden(i))
            hidden_cols_idx.push_back(i);
        QString str = table_->horizontalHeaderItem(i)->text();
        hdrs[i] = str;
    }

    for (int i = 0; i < nb_rows; ++i) {
        if (table_->isRowHidden(i))
            hidden_row_idx.push_back(i);
        table_content[i].resize(nb_cols);
        for (int j = 0; j < nb_cols; ++j) {
            table_content[i][j] = table_->item(i, j)->text();
        }
    }

    if (headers != nullptr)
        *headers = hdrs;
    if (rows != nullptr)
        *rows = table_content;
    if (hidden_cols != nullptr)
        *hidden_cols = hidden_cols_idx;
    if (hidden_rows != nullptr)
        *hidden_rows = hidden_row_idx;

    return true;
}

void TableViewer::getListAttributes(QVector<QString>& attrs, QVector<bool>& check_status)
{
    if (attr_list_ == nullptr)
        return;
    attr_list_->getItemNames(attrs);
    attr_list_->getItemCheckState(check_status);
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
    QVector<QString> header(def_nb_cols, "");
    QVector<QVector<QString>> content;
    content.reserve(def_nb_rows);
    for (int i = 0; i < def_nb_rows; ++i) {
        QVector<QString> tmp(def_nb_cols, "");
        content.push_back(tmp);
    }
    updateTable(header, content);
}

/*
 * When user toggles the visiblity of attributes in the list, this function is called to update the table.
 */
void TableViewer::onAttrSelectionChanged()
{
    if (attr_list_ == nullptr)
        return;
    QVector<bool> check_status;
    attr_list_->getItemCheckState(check_status);
    updateColumnVisibility(check_status);
}


/*
 *      class: TableViewerDialog
 *  This class has a TableViewer widget and menus for other operations.
 */
TableViewerDialog::TableViewerDialog(QWidget* parent)
    : QMainWindow(parent)
{
    QVBoxLayout* dialog_layout = new QVBoxLayout();
    dialog_layout->setContentsMargins(0, 0, 0, 0);
    QWidget* window = new QWidget();
    window->setLayout(dialog_layout);
    setCentralWidget(window);

    table_viewer_ = new TableViewer(this);
    dialog_layout->addWidget(table_viewer_);

    // Set up menu bar
    menu_bar_ = new QMenuBar(this);
    this->setMenuBar(menu_bar_);
    menuBar()->setNativeMenuBar(false); // Need this line, otherwise the menu bar not showning

    QMenu* menu_file = new QMenu("File");
    menu_bar_->addMenu(menu_file);
    QAction* action_export = new QAction("Export...");
    menu_file->addAction(action_export);

    connect(action_export, SIGNAL(triggered(bool)), this, SLOT(exportTableAsXlsx()));
}

bool TableViewerDialog::updateTable(
    const QVector<QString>& header,
    const QVector<QVector<QString>>& content
) {
    return table_viewer_->updateTable(header, content);
}

bool TableViewerDialog::getTableContent(
    QVector<QString>* headers,
    QVector<QVector<QString>>* rows,
    QVector<int>* hidden_cols,
    QVector<int>* hidden_rows
) {
    return table_viewer_->getTableContent(headers, rows, hidden_cols, hidden_rows);
}

/**
 * @brief TableViewerDialog::getFilePath
 *
 * Let user choose a file path with the provided format extension.
 */
QString TableViewerDialog::getFilePath(QString extension)
{
    QString filter = QString("Excel spreadsheet (*.%1)").arg(extension);
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save as"), "", filter, &filter
    );
    return path;
}

/**
 * @brief TableViewerDialog::exportTableAsXlsx
 *
 * Export the table as an xlsx file.
 */
void TableViewerDialog::exportTableAsXlsx()
{
    bool export_hidden_cols = false;
    bool export_hidden_rows = false;
    QString path = getFilePath("xlsx");
    QVector<QString> header;
    QVector<QVector<QString>> content;
    QVector<int> hidden_cols;
    QVector<int> hidden_rows;
    bool info_get = getTableContent(
        &header, &content, &hidden_cols, &hidden_rows
    );
    if (!info_get) {
        QMessageBox::warning(this, "File Export", "The table content is unavailable.");
        return;
    }

    if (!export_hidden_rows) {
        // Remove hidden rows
        for (int i = hidden_rows.size() - 1; i >= 0; --i) {
            content.erase(content.begin() + hidden_rows[i]);
        }
    }

    if (!export_hidden_cols) {
        // Remove columns from the table content
        for (int i = 0; i < content.size(); ++i) {
            for (int j = hidden_cols.size() - 1; j >= 0; --j) {
                content[i].erase(content[i].begin() + hidden_cols[j]);
            }
        }

        // Remove headers
        for (int j = hidden_cols.size() - 1; j >= 0; --j) {
            header.erase(header.begin() + hidden_cols[j]);
        }
    }

    XlsxIO writer(path);
    writer.write(header, content);
}
