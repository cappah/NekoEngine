#include "EditorWindow.h"

#include <QMessageBox>
#include <QApplication>
#include <QSplashScreen>

#include <Engine/Defs.h>
#include <Engine/Version.h>
#include <Windows.h>
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QPixmap splashPixmap(":/splash.png");
	QSplashScreen splash(splashPixmap);
	splash.showMessage(QString::asprintf("Editor: %s\nEngine: %s", "0.5.0.500", ENGINE_VERSION_STRING), Qt::AlignRight);

	splash.show();
	a.processEvents();

	EditorWindow w;
	if (w.initialize() != ENGINE_OK)
	{
		QMessageBox::critical(nullptr, "Fatal error", "Failed to initialize engine");
		return -1;
	}

	w.show();
	splash.finish(&w);

	return a.exec();
}
