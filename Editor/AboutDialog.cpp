#include "AboutDialog.h"
#include "ui_AboutDialog.h"

#include <QFile>
#include <QTextStream>

#include "Version.h"
#include <Engine/Version.h>
#include <Engine/Engine.h>

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

	ui->lblEditorVersion->setText(QString::asprintf("Editor Version: %s", EDITOR_VERSION_STRING));
	ui->lblEngineVersion->setText(QString::asprintf("Engine Version: %s", ENGINE_VERSION_STRING));
	ui->lblRenderer->setText(QString::asprintf("Renderer: %s %d.%d", Engine::GetRenderer()->GetName(), Engine::GetRenderer()->GetMajorVersion(), Engine::GetRenderer()->GetMinorVersion()));
	ui->lblPlatform->setText(QString::asprintf("Platform: %s", ENGINE_PLATFORM_STRING));

	QFile licenseFile(":/LICENSE");

	if(!licenseFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		ui->licenseBox->setPlainText("Failed to open license resource");
		return;
	}

	QTextStream licenseStream(&licenseFile);
	ui->licenseBox->setPlainText(licenseStream.readAll());

	licenseFile.close();
}

AboutDialog::~AboutDialog()
{
	delete ui;
}
