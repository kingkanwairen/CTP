#include <QtCore/QTextCodec>
#include "caApplication.h"
#include "caMainWindow.h"

#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
int main(int argc, char* argv[])
{	
	caApplication app(argc, argv);
	if (!app.Initialize())
	{
		fprintf(stderr, "Error when  initialize application!\n");
		return -1;
	}
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
	caMainWindow* pWidget = new caMainWindow(0);
	pWidget->show();
	int ret = app.exec();
	delete pWidget;
	if (!app.Finish()){
		fprintf(stderr, "Error when finish application!\n");
		return -1;
	}
	return ret;
}