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
    content.resize(has_header? nb_row - 1 : nb_row);
    for (; i <= nb_row; ++i) {
        content[i-1].resize(nb_column);
        for (int j = 1; j <= nb_column; ++j) {
            QVariant var = file.read(i, j);
            content[i-1][j-1] = var.toString();
        }
    }

    return true;
}

bool XlsxIO::write() {
    /*
    QXlsx::Document xlsx;
    xlsx.write(1, 1, "Hell, this is A1");
    xlsx.write(1, 3, "This is A2");
    xlsx.write(1, 4, 222);
    xlsx.write(2, 1, 123);
    QString out_path = QString();
    xlsx.saveAs(out_path);*/
}
