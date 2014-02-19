#include "AboutForm.h"


AboutForm::AboutForm(QWidget *parent, Qt::WindowFlags flags)
	: QDialog(parent, flags)
{
	ui.setupUi(this);
	QRect frect = frameGeometry();
	frect.moveCenter(QDesktopWidget().availableGeometry().center());
	move(frect.topLeft());

	connect(ui.okButton, SIGNAL(click()), this, SLOT(OnOkClick()));
}


AboutForm::~AboutForm(void)
{
}

void AboutForm::OnOkClick( void )
{
	this->close();
}
