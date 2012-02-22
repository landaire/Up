#include "progressdialog.h"
#include "ui_progressdialog.h"
#include <QThread>
#include <QtConcurrentRun.h>
#include "../FATX/StaticInformation.h"
#include "../FATX/IO/xDeviceFileStream.h"
#include "../FATX/stfspackage.h"
#include <algorithm>

ProgressDialog::ProgressDialog(QWidget *parent, Operations Operation, std::vector<std::string> Paths, std::string OutPath, std::vector<Drive*>& Drives) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    scene = 0;
    PathCount = Paths.size();
    QtConcurrent::run(this, &ProgressDialog::PerformOperation, Operation, Paths, OutPath, Drives);
    operation = Operation;
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::PerformOperation(Operations Operation, std::vector<std::string> Paths, std::string OutPath, std::vector<Drive*>& Drives)
{
    switch(Operation)
    {
    case OperationCopyToDisk:
        CopyFileToLocalDisk(Paths, OutPath, Drives);
        break;
    }
}

void ProgressDialog::CopyFileToLocalDisk(std::vector<std::string> Paths, std::string OutPath, std::vector<Drive*>& Drives)
{
    qRegisterMetaType<Progress>();
    std::vector<Drive*> ConnectedDrives;
    for (int i = 0; i < Paths.size(); i++)
    {
        std::vector<std::string> Split;
        Helpers::split(Paths.at(i), '/', Split);
        Drive *d;
        for (int j = 0; j < Drives.size(); j++)
        {
            d = Drives.at(j);
            if (QString::fromStdWString(d->FriendlyName) == QString::fromStdString(Split.at(0)))
                break;
        }
        vector<Drive*>::iterator it = std::find(ConnectedDrives.begin(), ConnectedDrives.end(), d);
        if (it == ConnectedDrives.end())
        {
            connect(d, SIGNAL(FileProgressChanged(const Progress&)), this, SLOT(OnFileProgressChanged(const Progress&)), Qt::QueuedConnection);
            ConnectedDrives.push_back(d);
        }
        // We were passed a folder
        if (PathCount > 1)
        {
            d->CopyFileToLocalDisk(Paths.at(i), (OutPath + "/") + Split.at(Split.size() - 1));
        }
        else
        {
            std::string path(Paths.at(i));
            d->CopyFileToLocalDisk(path, OutPath);
        }
    }
}

void ProgressDialog::OnFileProgressChanged(const Progress& p)
{
    if (ui->progressCurrent->maximum() != p.Maximum)
        ui->progressCurrent->setMaximum(p.Maximum);

    ui->progressCurrent->setValue(p.Current);

    if (PathCount == 1)
    {
        ui->progressTotal->setValue(p.Current);
        if (ui->progressTotal->maximum() != p.Maximum)
            ui->progressTotal->setMaximum(p.Maximum);
    }

    if (ui->groupBoxCurrent->title().toStdString() != p.FilePath)
    {
        ui->groupBoxCurrent->setTitle(QString::fromStdWString(p.Device->FriendlyName + L"/") + QString::fromStdString(p.FilePath));

        if (p.IsStfsPackage)
        {
            if (scene)
            {
                delete scene;
            }
            QImage image(p.PackageImage);
            scene = new QGraphicsScene(ui->graphicsView);
            scene->setSceneRect(0,0,image.width(), image.height());
            ui->graphicsView->setFixedSize(64, 64);
            scene->addPixmap(QPixmap::fromImage(image));
            ui->graphicsView->setScene(scene);
            ui->graphicsView->show();
            ui->graphicsView->fitInView(ui->graphicsView->rect(), Qt::KeepAspectRatio);

            // Get the package name (real)
            ui->labelFileName->setText(p.PackageName);
        }
        else
        {
            if (scene)
                delete scene;

            ui->labelFileName->setText(QString::fromStdString(p.FileName));


            QImage image(":/File System Icons/iFile");
            QPixmap pixmap = QPixmap::fromImage(image);
            scene = new QGraphicsScene(ui->graphicsView);
            scene->setSceneRect(0,0,64, 64);
            ui->graphicsView->setFixedSize(64, 64);
            scene->addPixmap(pixmap.scaled(QSize(64, 64), Qt::KeepAspectRatio));
            ui->graphicsView->setScene(scene);
            ui->graphicsView->show();
            ui->graphicsView->fitInView(ui->graphicsView->rect(), Qt::KeepAspectRatio);
        }
    }
}
