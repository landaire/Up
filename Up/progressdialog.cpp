#include "progressdialog.h"
#include "ui_progressdialog.h"
#include <QThread>
#include <QtConcurrentRun.h>
#include <algorithm>

using namespace std;

ProgressDialog::ProgressDialog(QWidget *parent, Operations Operation, std::vector<std::string> Paths, std::string OutPath, std::vector<Drive*>& Drives) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    scene = 0;
    PathCount = Paths.size();
    FilesTotal = PathCount;
    FilesCompleted = 0;
    WorkerThread = QtConcurrent::run(this, &ProgressDialog::PerformOperation, Operation, Paths, OutPath, Drives);
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
            if (QString::fromWCharArray(d->FriendlyName.c_str()) == Helpers::QStringFromStdString(Split.at(0)))
                break;
        }

        // Find the drive in our ConnectedDrives vector
        std::vector<Drive*>::iterator it = std::find(ConnectedDrives.begin(), ConnectedDrives.end(), d);
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
                // Get the folder
                Folder *folder = d->FolderFromPath(Paths.at(i));
                // Get the total number of files in this folder (recursively)
                FilesTotal += d->GetTotalFileCount(folder) -1; // subtract 1 because this folder will already have added 1 to the total files thing
                d->CopyFolderToLocalDisk(folder, OutPath);
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
                // Get the folder
                Folder *folder = d->FolderFromPath(Paths.at(i));
                // Get the total number of files in this folder (recursively)
                FilesTotal += d->GetTotalFileCount(folder) -1; // subtract 1 because this folder will already have added 1 to the total files thing
                d->CopyFolderToLocalDisk(folder, OutPath);
            }
        }
    }
}

void ProgressDialog::OnFileProgressChanged(const Progress& p)
{
    // If the maximums don't match
    if (ui->progressCurrent->maximum() != p.Maximum)
        // Set the progress bar maximum value
        ui->progressCurrent->setMaximum(p.Maximum);

    // Set the progress bar's current value
    ui->progressCurrent->setValue(p.Current);

    // If we're dealing with one file and the path count is one
    if (FilesTotal == 1 && PathCount == 1)
    {
        // Set the total progress bar's values to that of the current progress bar
        if (ui->progressTotal->maximum() != p.Maximum)
            ui->progressTotal->setMaximum(p.Maximum);
        ui->progressTotal->setValue(p.Current);
    }
    // If we're dealing with multiple files
    else
    {
        // Set the value of the total progressbar to the number of files we're doing
        ui->progressTotal->setMaximum(FilesTotal);
        ui->progressTotal->setValue(FilesCompleted);
    }

    // Set the title of the total groupbox to reflect the current file out of x
    ui->groupBoxTotal->setTitle(QString("Completed %1 file(s) out of %2").arg(FilesCompleted + 1).arg(FilesTotal));

    string Title = Helpers::QStringToStdString(ui->groupBoxCurrent->title());
    // If we've just moved on to a new file
    if (Title.substr(Title.find_first_of('/') + 1) != p.FilePath)
    {
        ui->groupBoxCurrent->setTitle(QString::fromWCharArray((p.Device->FriendlyName + L"/").c_str()) + Helpers::QStringFromStdString(p.FilePath));

        // If the file is an STFS package
        if (p.IsStfsPackage)
        {
            // Delete the current scene for the graphics view
            if (scene)
            {
                delete scene;
            }
            // Get the package image
            QImage image(p.PackageImage);

            // Create a new scene with the dimensions of the image
            scene = new QGraphicsScene(ui->graphicsView);
            scene->setSceneRect(0,0,image.width(), image.height());

            // Set the graphics view size
            ui->graphicsView->setFixedSize(64, 64);

            // Set the image that the scene will display
            scene->addPixmap(QPixmap::fromImage(image));

            // Set the scene and display the image
            ui->graphicsView->setScene(scene);
            ui->graphicsView->show();

            // Makes the graphics view scale the image down so there's no scrolling
            ui->graphicsView->fitInView(ui->graphicsView->rect(), Qt::KeepAspectRatio);

            // Get the package name (real)
            ui->labelFileName->setText(p.PackageName);
        }
        // Not an STFS package -- use the default file image
        else
        {
            // Delete the current scene if there is one
            if (scene)
                delete scene;

            ui->labelFileName->setText(Helpers::QStringFromStdString(p.FileName));


            // Get the file icon image
            QImage image(":/File System Icons/iFile");

            // Get the file icon pixmap
            QPixmap pixmap = QPixmap::fromImage(image);

            // Create a new scene
            scene = new QGraphicsScene(ui->graphicsView);
            scene->setSceneRect(0,0,64, 64);

            // Set the graphics view's fixed size
            ui->graphicsView->setFixedSize(64, 64);

            // Set the scene image
            scene->addPixmap(pixmap.scaled(QSize(64, 64), Qt::KeepAspectRatio));

            // Set the graphics view's scene, display the image
            ui->graphicsView->setScene(scene);
            ui->graphicsView->show();

            // Makes the image fit within the bounds of the graphics view without scrolling
            ui->graphicsView->fitInView(ui->graphicsView->rect(), Qt::KeepAspectRatio);
        }
    }
    if (p.Done)
        FilesCompleted++;
    if (FilesCompleted == FilesTotal)
        this->close();
}
