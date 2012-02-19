#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QtConcurrentRun>
#include <../FATX/Drive.h>

enum Operations
{
    OperationCopyToDisk
};

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT
    
public:
    //explicit ProgressDialog(QWidget *parent = 0, Operations Operation, std::string Path[], std::string OutPath);
    ProgressDialog(QWidget *parent, Operations Operation, std::string *Path, std::string OutPath, Drive *Drive);
    ~ProgressDialog();
    
private:
    Ui::ProgressDialog *ui;
    void PerformOperation( Operations Operation, std::string *Path, std::string OutPath, Drive *Drive );
    void CopyFileToLocalDisk( string *Path, std::string OutPath, Drive *Drive );
};

#endif // PROGRESSDIALOG_H
