#include "caLogManager.h"
#include <QtCore/QTextStream>


caLogManager *caLogManager::mThis = NULL;

caLogManager::caLogManager()
{
	mIsSave = false;
}

caLogManager::~caLogManager()
{
	
}

bool caLogManager::initialize()
{
	mThis = new caLogManager();
	return true;
}


bool caLogManager::finalize()
{	
	mThis->mLogFile.close();
	if (mThis) {
		delete mThis;
		mThis = NULL;
	}
	return true;
}

void caLogManager::saveData(const QString& data)
{
	if(mThis->mIsSave){
		QTextStream out(&mThis->mLogFile);
		out << data << endl;
	}
}

void caLogManager::setFilePath(bool isSave, const QString& path)
{
	mThis->mIsSave = isSave;
	if(isSave){
		mThis->mLogFile.close();
		mThis->mIsSave = false;
		mThis->mLogFile.setFileName(path);
		if(mThis->mLogFile.open(QFile::WriteOnly | QFile::Append)){
			mThis->mIsSave = true;
		}
	}
}

#include "moc_caLogManager.cpp"