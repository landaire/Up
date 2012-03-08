#include "progressdialog.h"
#include "ui_progressdialog.h"
#include <QThread>
#include <QtConcurrentRun.h>
#include <algorithm>

ProgressDialog::ProgressDialog(QWidget *parent, Operations Operation, std::vector<std::string> Paths, std::string OutPath, std::vector<Drive*>& Drives) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    scene = 0;
    PathCount = Paths.size();
    FilesTotal = PathCount;
    FilesCompleted = 0;
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
    // For each top-level path
    for (int i = 0; i < Paths.size(); i++)
    {
        // Split the path so that we can get the disk name
        std::vector<std::string> Split;
        Helpers::split(Paths.at(i), '/', Split);
        Drive *d;
        // For each drive
        for (int j = 0; j < Drives.size(); j++)
        {
            // Set d, break when we've found the proper drive.
            // POSSIBLE BUG: we'll go ahead and trust that everything is handled properly in MainForm though
            d = Drives.at(j);
            if (QString::fromStdWString(d->FriendlyName) == Helpers::QStringFromStdString(Split.at(0)))
                break;
        }

        // Find the drive in our ConnectedDrives vector
        vector<Drive*>::iterator it = std::find(ConnectedDrives.begin(), ConnectedDrives.end(), d);
        if (it == ConnectedDrives.end())
        {
            // If the drive was not connected, connect it
            connect(d, SIGNAL(FileProgressChanged(const Progress&)), this, SLOT(OnFileProgressChanged(const Progress&)), Qt::QueuedConnection);
            // Add the drive to our vector of connected drives
            ConnectedDrives.push_back(d);
        }
        // We were passed a folder path
        if (PathCount > 1)
        {
            try
            {
                // Get the file
                File *f = d->FileFromPath(Paths.at(i));
                // Write the files to folder path + file name
                d->CopyFileToLocalDisk(f, (OutPath + "/") + Split.at(Split.size() - 1));
            }
            // Exception was thrown, we're dealing with a folder path
            catch(...)
            {
                // Add folder handling
            }
        }
        else
        {
            // See if this is a file...
            try
            {
                std::string path(Paths.at(i));
                File *f = d->FileFromPath(path);
                // Succeeded, write the file to the destined folder
                d->CopyFileToLocalDisk(f, OutPath);
            }
            catch (...)
            {
                // Add folder handling here
            }
        }
        FilesCompleted++;
    }
}

void ProgressDialog::OnFileProgressChanged(const Progress& p)
{
    if (ui->progressCurrent->maximum() != p.Maximum)
        ui->progressCurrent->setMaximum(p.Maximum);

    ui->progressCurrent->setValue(p.Current);

    if (FilesTotal == 1 && PathCount == 1)
    {
        ui->progressTotal->setValue(p.Current);
        if (ui->progressTotal->maximum() != p.Maximum)
            ui->progressTotal->setMaximum(p.Maximum);
    }
    ui->groupBoxTotal->setTitle(QString("Total - File %1 out of %2").arg(FilesCompleted + 1).arg(FilesTotal));

    if (Helpers::QStringToStdString(ui->groupBoxCurrent->title()) != p.FilePath)
    {
        ui->groupBoxCurrent->setTitle(QString::fromWCharArray((p.Device->FriendlyName + L"/").c_str()) + Helpers::QStringFromStdString(p.FilePath));

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

            ui->labelFileName->setText(Helpers::QStringFromStdString(p.FileName));


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
