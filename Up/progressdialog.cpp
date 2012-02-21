#include "progressdialog.h"
#include "ui_progressdialog.h"
#include <QThread>
#include <QtConcurrentRun.h>
#include "../FATX/StaticInformation.h"

ProgressDialog::ProgressDialog(QWidget *parent, Operations Operation, std::string *Path, std::string OutPath, Drive *drive) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);

    QtConcurrent::run(this, &ProgressDialog::PerformOperation, Operation, *Path, OutPath, drive);
    operation = Operation;
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::PerformOperation(Operations Operation, std::string Path, std::string OutPath, Drive *Drive)
{
    switch(Operation)
    {
    case OperationCopyToDisk:
        CopyFileToLocalDisk(&Path, OutPath, Drive);
        break;
    }
}

void ProgressDialog::CopyFileToLocalDisk(std::string *Path, std::string OutPath, Drive *Drive)
{
    std::string InPath = *Path;
    qRegisterMetaType<Progress>();
    connect(Drive, SIGNAL(FileProgressChanged(const Progress&)), this, SLOT(OnFileProgressChanged(const Progress&)), Qt::QueuedConnection);
    Drive->CopyFileToLocalDisk(InPath, OutPath);
}

void ProgressDialog::OnFileProgressChanged(const Progress& p)
{
    if (ui->progressCurrent->maximum() != p.Maximium)
        ui->progressCurrent->setMaximum(p.Maximium);

    ui->progressCurrent->setValue(p.Current);

    if (ui->groupBoxCurrent->title().toStdString() != p.FilePath)
        ui->groupBoxCurrent->setTitle(QString::fromStdString(p.FilePath));
}
