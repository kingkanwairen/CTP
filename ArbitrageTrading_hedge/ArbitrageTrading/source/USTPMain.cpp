#include <QtCore/QTextCodec>
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include "USTPCtpLoader.h"
#include "USTPConfig.h"
#include "USTPMainWindow.h"
#include "USTPLoginDialog.h"
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

int main(int argc, char* argv[])
{	
	//QMap<QString, QString> test;
	//test.insert("testewe", "ins1");
	//test.insert("testree", "ins2");
	//test.insert("testtrt", "ins3");
	//test.insert("testtrw", "ins4");
	//QList<QString> aa = test.keys();
	//if(foreach(QString ins, aa)){
	//	qDebug() << ins <<endl;
	//}

	QApplication app(argc, argv);
	QTextCodec::setCodecForTr(QTextCodec::codecForLocale());
	if (!USTPCtpLoader::initialize()){
		return CTP_INITINAL_ERROR;
	}
	if (!USTPCtpLoader::start()){
		return CTP_LOAD_ERROR;
	}
	USTPLoginDialog dlg;
	if(dlg.exec() == QDialog::Accepted){
		USTPMainWindow* pWidget = new USTPMainWindow();
		pWidget->show();
		int ret = app.exec();
		delete pWidget;
	}
	
	if (!USTPCtpLoader::stop()){
		return CTP_STOP_ERROR;
	}
	if (!USTPCtpLoader::finalize()){
		return CTP_FINALIZE_ERROR;
	}
	return 0;
}