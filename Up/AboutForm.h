#pragma once
#include <qdialog.h>
#include "ui_AboutForm.h"
#include <qdesktopwidget.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QDialog>

class AboutForm :
	public QDialog
{
	Q_OBJECT
public:
    AboutForm(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~AboutForm(void);
private:
	Ui::AboutForm ui;
public slots:
	void OnOkClick( void );
};

