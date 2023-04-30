#include "file_io.h"

/**********************************
    class: FileIO
**********************************/
FileIO::FileIO(QString name) : file_name_(name)
{

}


/**********************************
    class: XlsxIO
**********************************/
XlsxIO::XlsxIO(QString name) : FileIO(name)
{

}

bool XlsxIO::read(
    vector<QString>& header,
    vector<vector<QString>>& content,
    bool has_header // indicate if the Xlsx file has a header line
) {
    QXlsx::Document file(file_name_); // Open a file
    QXlsx::CellRange range = file.dimension();
    int nb_column = range.columnCount();
    int nb_row = range.rowCount();

    if (nb_column * nb_row == 0)
        return false; // No data to read

    int i = 1;
    if (has_header) {
        // Read the header line (first line)
        header.resize(nb_column);
        for (int j = 1; j <= nb_column; ++j) {
            QVariant var = file.read(i, j);
            header[j-1] = var.toString();
        }
        ++i;
    }

    // Read each row
    content.reserve(has_header? nb_row - 1 : nb_row);
    for (; i <= nb_row; ++i) {
        vector<QString> line_content(nb_column);
        for (int j = 1; j <= nb_column; ++j) {
            QVariant var = file.read(i, j);
            line_content[j-1] = var.toString();
        }
        content.push_back(line_content);
    }

    return true;
}

bool XlsxIO::write(
    const vector<QString>& header,
    const vector<vector<QString>>& content
) {
    QXlsx::Document xlsx;

    // Writer header
    int row = 1;
    if (header.size() != 0) {
        for (int i = 0; i < header.size(); ++i) {
            xlsx.write(row, i + 1, header[i].toStdString().c_str());
        }
    }

    // Write content
    ++row;
    for (int i = 0; i < content.size(); ++i) {
        for (int j = 0; j < content[i].size(); ++j) {
            xlsx.write(row, j + 1, content[i][j].toStdString().c_str());
        }
        ++row;
    }

    xlsx.saveAs(file_name_);
}

bool XlsxIO::write(
    const QVector<QString>& header,
    const QVector<QVector<QString>>& content
) {
    vector<QString> header_tmp = header.toStdVector();
    vector<vector<QString>> content_tmp(content.size());
    for (int i = 0; i < content.size(); ++i)
        content_tmp[i] = content[i].toStdVector();
    return write(header_tmp, content_tmp);
}
