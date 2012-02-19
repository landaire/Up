#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent, Operations Operation, std::string *Path, std::string OutPath, Drive *Drive) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    PerformOperation(Operation, Path, OutPath, Drive);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::PerformOperation(Operations Operation, std::string *Path, std::string OutPath, Drive *Drive)
{
    switch(Operation)
    {
    case OperationCopyToDisk:
        CopyFileToLocalDisk(Path, OutPath, Drive); //QtConcurrent::run(CopyFileToLocalDisk, Path, OutPath);
        break;
    }
}

void ProgressDialog::CopyFileToLocalDisk(std::string *Path, std::string OutPath, Drive *Drive)
{
    std::string InPath = *Path;
    Drive->CopyFileToLocalDisk(InPath, OutPath);
}
