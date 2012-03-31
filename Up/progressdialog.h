#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QtConcurrentRun>
#include "../FATX/FATX/StaticInformation.h"
#include "../FATX/IO/xDeviceFileStream.h"
#include "../FATX/FATX/stfspackage.h"
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
    ProgressDialog(QWidget *parent, Operations Operation, std::vector<std::string> Paths, std::string OutPath, std::vector<Drive*>& Drives);
    ~ProgressDialog();
    void PerformOperation( Operations Operation, std::vector<std::string> Paths, std::string OutPath, std::vector<Drive*>& Drives);

private:
    Ui::ProgressDialog *ui;
    void CopyFileToLocalDisk( std::vector<std::string> Paths, std::string OutPath, std::vector<Drive*>& Drives );
    Operations operation;
    QGraphicsScene *scene;
    int PathCount;
    int FilesTotal;
    int FilesCompleted;
    QFuture<void> WorkerThread;
public slots:
    void OnFileProgressChanged(const Progress& p);
};

#endif // PROGRESSDIALOG_H
