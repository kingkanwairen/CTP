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
		ShowCritical(tr("��ʼ����־�ļ�ʧ��."));
		return false;
	}
	return true;
}

bool caApplication::Finish()
{	
	if (!caLogManager::finalize()) {
		ShowCritical(tr("�ر���־�ļ�ʧ��."));
		return false;
	}
	return true;
}

#include "moc_caApplication.cpp"