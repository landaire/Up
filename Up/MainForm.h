#ifndef MAINFORM_H
#define MAINFORM_H

// Qt referencces
#include <QtGui/QMainWindow>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcommandlinkbutton.h>
#include <qdesktopwidget.h>
#include <qdatetime>
#include <QPainter>
#include <QSettings>

// Form references
#include "ui_MainForm.h"
#include "AboutForm.h"
#include "progressdialog.h"

// FATX references
#include "../FATX/typedefs.h"
#include "../FATX/FATX/Helpers.h"
#include "../FATX/FATX/Drive.h"
#include "../FATX/IO/xDeviceFileStream.h"
#include "../FATX/FATX/StaticInformation.h"
#include "../FATX/FATX/stfspackage.h"

// std lib references
#include <map>
#include <algorithm>
#include <string>

#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>

#endif

using namespace Streams;

class MainForm : public QMainWindow
{
        Q_OBJECT

public:
        MainForm(QWidget *parent = 0, Qt::WFlags flags = 0);
        ~MainForm();
public slots:
        void OnLoadDevicesClick( void );
        void ShowAbout( void );
        void OnTreeItemExpand(QTreeWidgetItem* Item);
        void OnCopyToLocalDiskClick( void );
        void OnTreeItemDoubleClick(QTreeWidgetItem* Item, int);
private:
        QIcon iFolder;
        QIcon iDisk;
        QIcon iFile;
        QIcon iPartition;
        QIcon iUsb;        
        std::map<DWORD, QIcon*> Icons;

        Ui::MainFormClass ui;
        void                    Center          ( void );
        void                    DoEvents        ( void );
        std::string             GetItemPath     (QTreeWidgetItem* Item);
        std::vector<Drive*>           ActiveDrives;
        QTreeWidgetItem         *AddFolder      (QTreeWidgetItem *Item, Folder *f);
        QTreeWidgetItem         *AddFile        (QTreeWidgetItem *Item, File *f, Drive *device);
        void                    PopulateTreeItems(QTreeWidgetItem *Item, bool expand);
        Drive                   *GetCurrentItemDrive(QTreeWidgetItem* Item);
        void                    SetContextMenus ( void );
        std::string             GetCurrentItemPath( QTreeWidgetItem *Item );
        void                    SetTitleIdName  (QTreeWidgetItem* Item);
        QSettings               *cache;
};

#endif // MAINFORM_H
