#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QtConcurrentRun>
#include <../FATX/Drive.h>
#include <qgraphicsscene.h>

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
    ProgressDialog(QWidget *parent, Operations Operation, std::string *Path, std::string OutPath, Drive *drive);
    ~ProgressDialog();
    void PerformOperation( Operations Operation, std::string Path, std::string OutPath, Drive *Drive );

private:
    Ui::ProgressDialog *ui;
    void CopyFileToLocalDisk( string *Path, std::string OutPath, Drive *Drive );
    Operations operation;
    QGraphicsScene *scene;
public slots:
    void OnFileProgressChanged(const Progress& p);
};

#endif // PROGRESSDIALOG_H
