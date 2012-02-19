#pragma once
#include <qdialog.h>
#include "ui_AboutForm.h"
#include <qdesktopwidget.h>
#include <QtGui/QMainWindow>
#include <QtGui/QDialog>

class AboutForm :
	public QDialog
{
	Q_OBJECT
public:
	AboutForm(QWidget *parent = 0, Qt::WFlags flags = 0);
	~AboutForm(void);
private:
	Ui::AboutForm ui;
public slots:
	void OnOkClick( void );
};

