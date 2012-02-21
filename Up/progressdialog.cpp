#include "progressdialog.h"
#include "ui_progressdialog.h"
#include <QThread>
#include <QtConcurrentRun.h>
#include "../FATX/StaticInformation.h"
#include "../FATX/IO/xDeviceFileStream.h"
#include "../FATX/stfspackage.h"

ProgressDialog::ProgressDialog(QWidget *parent, Operations Operation, std::string *Path, std::string OutPath, Drive *drive) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    scene = 0;
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
    if (ui->progressCurrent->maximum() != p.Maximum)
        ui->progressCurrent->setMaximum(p.Maximum);

    ui->progressCurrent->setValue(p.Current);

    if (ui->groupBoxCurrent->title().toStdString() != p.FilePath)
    {
        ui->groupBoxCurrent->setTitle(QString::fromStdString(p.FilePath));

        // Determine if the file is a valid STFS package
        Streams::xDeviceFileStream *fs = new Streams::xDeviceFileStream(p.FilePath, p.Device);
        STFSPackage pack(fs);
        for (int i = 0; i < 2; i++)
        {
            if (pack.IsStfsPackage())
            {
                // Get the package icon
                QImage image(pack.ThumbnailImage());
                if (scene)
                {
                    delete scene;
                }
                scene = new QGraphicsScene(ui->graphicsView);
                scene->setSceneRect(0,0,image.width(), image.height());
                ui->graphicsView->setFixedSize(64, 64);
                scene->addPixmap(QPixmap::fromImage(image));
                ui->graphicsView->setScene(scene);
                ui->graphicsView->show();

                // Get the package name (real)
                ui->labelFileName->setText(pack.DisplayName());
                break;
            }
            else
            {
                QThread::currentThread()->wait(250);
                if (i == 1)
                {
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
            fs->Close();
        }
    }
}
