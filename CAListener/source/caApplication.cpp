#include "caApplication.h"
#include "caLogManager.h"
#include "caPublicDefine.h"
#include <QtGui/QMessageBox>

caApplication::caApplication(int& argc, char** argv)
	: QApplication(argc, argv)
{
	
}

caApplication::~caApplication()
{

}

bool caApplication::Initialize()
{	
	if (! caLogManager::initialize()) {
		ShowCritical(tr("初始化日志文件失败."));
		return false;
	}
	return true;
}

bool caApplication::Finish()
{	
	if (!caLogManager::finalize()) {
		ShowCritical(tr("关闭日志文件失败."));
		return false;
	}
	return true;
}

#include "moc_caApplication.cpp"