#ifndef TABLEVIEWER_H
#define TABLEVIEWER_H

#include <QWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QSplitter>
#include <QMainWindow>
#include "checkbox_list.h"

class TableViewer : public QWidget
{
    Q_OBJECT
public:
    explicit TableViewer(QWidget *parent = nullptr);

signals:

public slots:
    bool updateTable(
        const QVector<QString>& header, // Defines the header of a table
        const QVector<QVector<QString>>& content // Content to display
    );

    bool getTableContent(
        QVector<QString>* headers,
        QVector<QVector<QString>>* rows,
        QVector<int>* hidden_cols,
        QVector<int>* hidden_rows
    );

    void getListAttributes(QVector<QString>& attrs, QVector<bool>& check_status);

    void updateColumnVisibility(const QVector<bool>& vis);

    void reset();

private slots:
    void onAttrSelectionChanged();

private:
    QTableWidget* table_;
    CheckboxList* attr_list_;
};

class TableViewerDialog : public QMainWindow
{
    Q_OBJECT
public:
    TableViewerDialog(QWidget* parent = nullptr);

    bool updateTable(
        const QVector<QString>& header, // Defines the header of a table
        const QVector<QVector<QString>>& content // Content to display
    );

    bool getTableContent(
        QVector<QString>* headers,
        QVector<QVector<QString>>* rows,
        QVector<int>* hidden_cols,
        QVector<int>* hidden_rows
    );

    QString getFilePath(QString extension);

public slots:
    void exportTableAsXlsx();

private:
    TableViewer* table_viewer_;
    QMenuBar* menu_bar_;
};

#endif // TABLEVIEWER_H
