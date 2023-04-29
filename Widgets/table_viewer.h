#ifndef TABLEVIEWER_H
#define TABLEVIEWER_H

#include <QWidget>
#include <QTableWidget>
#include <QListWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QSplitter>
#include "checkbox_list.h"

class TableViewer : public QWidget
{
    Q_OBJECT
public:
    explicit TableViewer(QWidget *parent = nullptr);

    TableViewer(
        const QVector<QString>& header, // Defines the header of a table
        const QVector<QVector<QString>>& content, // Content to display
        QWidget* parent = nullptr
    );

signals:

public slots:
    bool updateTable(
        const QVector<QString>& header, // Defines the header of a table
        const QVector<QVector<QString>>& content // Content to display
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

#endif // TABLEVIEWER_H
