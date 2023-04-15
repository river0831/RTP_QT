#ifndef FILEIO_H
#define FILEIO_H

#include <QString>
#include <QVector>
#include "xlsxdocument.h" // xlsx file reading

using namespace std;

class FileIO
{
public:
    FileIO(QString name);

protected:
    QString file_name_;
};

class XlsxIO : public FileIO
{
public:
    XlsxIO(QString name);

    bool read(
        vector<QString>& header,
        vector<vector<QString>>& content,
        bool has_header = true
    );

    bool write(

    );
};

#endif // FILEIO_H
