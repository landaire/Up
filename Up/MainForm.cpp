#include "MainForm.h"
#include <algorithm>

MainForm::MainForm(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    Center();
    DoEvents();
    ui.fileSystemTree->setColumnWidth(0, 240);
    ui.fileSystemTree->setColumnWidth(1, 150);

    // Set the context menus
    SetContextMenus();

    iDisk.addFile(QString::fromUtf8(":/File System Icons/iDisk"), QSize(), QIcon::Normal, QIcon::Off);
    iFile.addFile(QString::fromUtf8(":/File System Icons/iFile"), QSize(), QIcon::Normal, QIcon::Off);
    iUsb.addFile(QString::fromUtf8(":/File System Icons/iUsb"), QSize(), QIcon::Normal, QIcon::Off);
    iFolder.addFile(QString::fromUtf8(":/File System Icons/iFolder"), QSize(), QIcon::Normal, QIcon::Off);
    iPartition.addFile(QString::fromUtf8(":/File System Icons/iPartition"), QSize(), QIcon::Normal, QIcon::Off);
}

MainForm::~MainForm()
{

}

string MainForm::GetCurrentItemPath(QTreeWidgetItem *Item)
{
    vector<std::string> ItemNames;
    QTreeWidgetItem *temp = Item;
    while (temp != 0)
    {
        ItemNames.push_back(temp->text(0).toStdString());
        qDebug("Item: %s", temp->text(0).toStdString().c_str());
        temp = temp->parent();
    }
    string built;
    for (int i = ItemNames.size() - 1; i >= 0; i--)
    {
        qDebug("Item: %s", ItemNames.at(i).c_str());
        built += ItemNames.at(i);
        if (i != 0)
            built += "/";
    }
    return built;
}

void MainForm::OnCopyToLocalDiskClick( void )
{
    QString s;
    int size = ui.fileSystemTree->selectedItems().size();
    if (size == 0)
        return;
    else if (size == 1 && ui.fileSystemTree->selectedItems().at(0)->text(2) != QString::fromAscii("Folder"))
    {
        s = QFileDialog::getSaveFileName(this, QString::fromAscii("Select Where to Save File"), ui.fileSystemTree->selectedItems().at(0)->text(0));
    }
    else
    {
        QFileDialog qfd;
        qfd.setFileMode(QFileDialog::Directory);
        qfd.setOption(QFileDialog::ShowDirsOnly);
        s = qfd.getExistingDirectory(this, QString::fromAscii("Select Directory to Save Files"));
    }
    if (s.isEmpty() || s.isNull())
    {
        return;
    }

    // Get the paths of all selected items
    vector<std::string> Paths;
    for (int i = 0; i < ui.fileSystemTree->selectedItems().size(); i++)
        Paths.push_back(GetCurrentItemPath(ui.fileSystemTree->selectedItems().at(i)));

    ProgressDialog *pd = new ProgressDialog(this, OperationCopyToDisk, Paths, s.toStdString(), ActiveDrives);
    //pd->setWindowFlags(Qt::WindowMaximizeButtonHint);
    pd->setModal(true);
    pd->show();
}

void MainForm::SetContextMenus( void )
{
    ui.fileSystemTree->addAction(ui.actionCopy_File_to_Local_Disk);
    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    ui.fileSystemTree->addAction(sep);
    ui.fileSystemTree->addAction(ui.actionRename);

    connect(ui.actionCopy_File_to_Local_Disk, SIGNAL(triggered()), this, SLOT(OnCopyToLocalDiskClick()));
}

void MainForm::Center( void )
{
    QRect frect = frameGeometry();
    frect.moveCenter(QDesktopWidget().availableGeometry().center());
    move(frect.topLeft());
}

void MainForm::DoEvents( void )
{
    connect(ui.actionLoad_Devices, SIGNAL(triggered()), this, SLOT(OnLoadDevicesClick()));
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(ShowAbout()));
    connect(ui.fileSystemTree, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(OnTreeExpand(QTreeWidgetItem*)));
}

Drive *MainForm::GetCurrentItemDrive(QTreeWidgetItem* Item)
{
    QString text = Item->text(0);
    qDebug(text.toStdString().c_str());
    QTreeWidgetItem* Parent = Item;
    while (Parent->parent() != 0)
        Parent = Parent->parent();

    for (int i = 0; i < (int)ActiveDrives.size(); i++)
    {
        Drive* d = ActiveDrives.at(i);
        qDebug(QString::fromWCharArray(d->FriendlyName.c_str()).toStdString().c_str());
        if (QString::fromWCharArray(d->FriendlyName.c_str()) == Parent->text(0))
            return d;
    }
    qDebug("returned nothing");
    return 0;
}

std::string MainForm::GetItemPath(QTreeWidgetItem *Item)
{
    string Path;
    QTreeWidgetItem* Parent = Item;
    do
    {
        if (Parent->parent() == 0)
        {
            break;
        }
        if (Path.length() != 0)
        {
            string pathtemp = Path;
            Path = "/";
            Path += pathtemp;
        }
        string pathtemp = Path;
        Path = Parent->text(0).toStdString() + pathtemp;
        Parent = Parent->parent();
    }
    while (Parent != 0);

    return Path;
}

QTreeWidgetItem *MainForm::AddFolder(QTreeWidgetItem* Item, Folder *f)
{
    QTreeWidgetItem *fItem = new QTreeWidgetItem(Item);
    fItem->setText(0, QString::fromAscii(f->Dirent.Name));
    fItem->setData(0, Qt::UserRole, QVariant(false));
    QDateTime ModifiedDate = f->DateModified;
    fItem->setText(1, ModifiedDate.toString());
    fItem->setText(2, QString::fromAscii("Folder"));
    fItem->setIcon(0, iFolder);
    return fItem;
}

QTreeWidgetItem *MainForm::AddFile(QTreeWidgetItem* Item, File *f, Drive *device)
{
    QTreeWidgetItem *fItem = new QTreeWidgetItem(Item);
    fItem->setText(0, QString::fromAscii(f->Dirent.Name));
    fItem->setIcon(0, iFile);

    QDateTime ModifiedDate = f->DateModified;
    fItem->setText(1, ModifiedDate.toString());
    fItem->setText(2, QString::fromAscii("File"));
    fItem->setText(3, QString::fromStdString(Helpers::ConvertToFriendlySize(f->Dirent.FileSize)));

    // Determine if the file is a valid STFS package
    Streams::xDeviceFileStream *fs = new Streams::xDeviceFileStream(f->FullPath, device);
    STFSPackage pack(fs);
    if (pack.IsStfsPackage())
    {
        fItem->setText(2, pack.ContentType_s());
        QIcon *Icon = 0;
        if (Icons.size())
        {
            map<DWORD, QIcon*>::iterator it;
            it = Icons.find(pack.TitleId());
            if (it != Icons.end())
                Icon = it->second;
        }
        if (!Icon)
        {
            Icon = new QIcon(QPixmap::fromImage(pack.ThumbnailImage()));
            Icons.insert(Icons.begin(), pair<DWORD, QIcon*>(pack.TitleId(), Icon));
        }
        fItem->setIcon(0, *Icon);
        fItem->setText(4, pack.DisplayName());
    }

    return fItem;
}

void MainForm::PopulateTreeItems(QTreeWidgetItem *Item, bool expand)
{
    string Path = GetItemPath(Item);
    qDebug("Item path: %s", Path.c_str());
    Drive* currentDrive = GetCurrentItemDrive(Item);
    Folder *f = currentDrive->FolderFromPath(Path);
    int Folders = f->CachedFolders.size();
    for (int i = 0; i < Folders; i++)
    {
        Folder *sub = f->CachedFolders.at(i);
        string path = sub->FullPath;
        sub = currentDrive->FolderFromPath(path);

        QTreeWidgetItem *subItem = 0;
        if (!expand)
            subItem = AddFolder(Item, sub);
        else
        {
            for (int x = 0; x < Item->childCount(); x++)
            {
                if (Item->child(x)->text(0) == QString::fromAscii(sub->Dirent.Name))
                {
                    subItem = Item->child(x);
                    break;
                }
            }
        }

        for (int j = 0; j < (int)sub->CachedFolders.size(); j++)
            AddFolder(subItem, sub->CachedFolders.at(j));

        for (int x = 0;  x < (int)sub->CachedFiles.size(); x++)
            AddFile(subItem, sub->CachedFiles.at(x), currentDrive);
    }
    if (!expand)
    {
        for (int i = 0; i < (int)f->CachedFiles.size(); i++)
        {
            File *file = f->CachedFiles.at(i);
            AddFile(Item, file, currentDrive);
        }
    }
    Item->setData(0, Qt::UserRole, QVariant(true));
}

void MainForm::OnTreeExpand( QTreeWidgetItem* Item)
{
    if (Item->parent() != 0 && Item->parent()->parent() != 0)
    {
        if (!Item->data(0, Qt::UserRole).toBool())
        {
            PopulateTreeItems(Item, true);
        }
    }
}

void MainForm::ShowAbout( void )
{
    AboutForm* about = new AboutForm(this);
    about->show();
}

void MainForm::OnLoadDevicesClick( void )
{
    vector<Drive *> Drives;
    try
    {
        Drives = Drive::GetFATXDrives(true);
    }
    catch (exception e)
    {
        QMessageBox mb;
        mb.setText("Exception was thrown");
        mb.show();
    }

    for (int i = 0; i < (int)Drives.size(); i++)
    {
        Drive *current = Drives.at(i);
        bool Skip = false;
        // Do a check to make sure we aren't adding the same device twice
        for (int j = 0; j < (int)ui.fileSystemTree->topLevelItemCount(); j++)
        {
            if (QString::fromWCharArray(current->FriendlyName.c_str()) == ui.fileSystemTree->topLevelItem(j)->text(0))
            {
                // Skip this item
                Skip = true;
                break;
            }
        }
        if (Skip)
        {
            current->Close();
            delete current;
            continue;
        }
        ActiveDrives.push_back(current);
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, QString::fromWCharArray(current->FriendlyName.c_str()));			// Drive name
        char* Type = 0;
        switch (current->Type)
        {
        case DeviceUsb:
            Type = "USB Removable Storage";
            break;
        case DeviceDisk:
            Type = "Disk Drive";
            break;
        case DeviceBackup:
            Type = "Disk Backup";
            break;
        }

        item->setText(2, QString::fromAscii(Type));     									// Drive type
        item->setText(3, QString::fromAscii(current->FriendlySize.c_str()));                // Drive size
        // if it's a USB device
        if (current->Type == DeviceUsb)
        {
            // Assign it the USB icon
            item->setIcon(0, iUsb);
        }
        else
        {
            item->setIcon(0, iDisk);
        }

        for (int j = 0; j < (int)current->Partitions().size(); j++)
        {
            if (!item)
            {
                break;
            }
            QTreeWidgetItem* partition = new QTreeWidgetItem(item);
            string currentVolume = current->Partitions().at(j);
            partition->setText(0, QString::fromAscii(currentVolume.c_str()));	// Partition name
            partition->setText(2, QString::fromAscii("Partition"));
            partition->setText(3, QString::fromAscii(Helpers::ConvertToFriendlySize((INT64)current->PartitionGetLength(currentVolume)).c_str()));
            partition->setIcon(0, iPartition);
            partition->setData(0, Qt::UserRole, QVariant(false));

            PopulateTreeItems(partition, false);
        }
        ui.fileSystemTree->insertTopLevelItem(ui.fileSystemTree->topLevelItemCount(), item);
        ui.activeDevicesComboBox->addItem(item->icon(0), item->text(0));
    }
}
