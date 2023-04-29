#include "table_viewer.h"
#include <QMenuBar>

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
    attr_list_ = new CheckboxList("Attributes", this);
    splitter->addWidget(attr_list_);

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
}

bool TableViewerDialog::updateTable(
    const QVector<QString>& header,
    const QVector<QVector<QString>>& content
) {
    table_viewer_->updateTable(header, content);
}
