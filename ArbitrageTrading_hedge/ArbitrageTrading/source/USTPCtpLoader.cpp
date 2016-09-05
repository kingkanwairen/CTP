#include "USTPCtpLoader.h"
#include "USTPConfig.h"
#include <QtXml/QDomDocument>
#include <QtCore/QDateTime>
#include "USTPMdApi.h"
#include "USTPTradeApi.h"
#include "USTPMutexId.h"
#include "USTPLogger.h"
#include "USTPProfile.h"


USTPCtpLoader* USTPCtpLoader::mThis = NULL;

bool USTPCtpLoader::initialize()
{	
	mThis = new USTPCtpLoader();
	if (!mThis->m_pBase64->decodeLicense(SYSTEM_LICESE_PATH)){
		return false;
	}
	if(!mThis->m_pBase64->getDateIsValid(mThis->mDateTime)){
		return false;
	}
	mThis->mAuthUser = mThis->m_pBase64->getUserId();
	if (!mThis->openTradeLog()){
		return false;
	}
	if (!mThis->openProfileFile()){
		return false;
	}
	if (!mThis->loadXMLFile(SYSTEM_FILE_PATH)){
		return false;
	}
	return true;
}

bool USTPCtpLoader::finalize()
{
	if (!mThis->closeTradeLog()){
		return false;
	}

	if (!mThis->closeProfileFile()){
		return false;
	}

	if (mThis) {
		delete mThis;
		mThis = NULL;
	}
	return true;
}

bool USTPCtpLoader::start()
{
	if (!mThis->startCtpMdThread()){
		return false;
	}
	if (!mThis->startCtpTradeThread()){
		return false;
	}
	if (!USTPMdApi::initilize(mThis->m_pMdApi)){
		return false;
	}
	if(!USTPTradeApi::initialize(mThis->m_pTradeApi)){
		return false;
	}
	return true;
}

bool USTPCtpLoader::stop()
{	
	if (!USTPMdApi::finalize()){
		return false;
	}
	if(!USTPTradeApi::finalize()){
		return false;
	}

	if(!USTPMutexId::finalize()){
		return false;
	}

	if (mThis->m_pMdApi != NULL){
		mThis->m_pMdApi->Release();
		mThis->m_pMdApi = NULL;
	}
	if (mThis->m_pTradeApi != NULL){
		mThis->m_pTradeApi->Release();
		mThis->m_pTradeApi = NULL;
	}
	return true;
}

USTPCtpLoader::USTPCtpLoader()
{
	m_pMdSpi = new USTPMdSpi();
	m_pTradeSpi = new USTPTradeSpi();
	m_pBase64 = new USTPBase64;
}

USTPCtpLoader::~USTPCtpLoader()
{
	delete m_pMdSpi;
	delete m_pTradeSpi;
	delete m_pBase64;
}

bool USTPCtpLoader::loadXMLFile(const QString& path)
{
	QFile file(path);  
	if (!file.open(QFile::ReadOnly | QFile::Text)) {  
		return false; 
	}  

	QDomDocument domDocument;  
	if (!domDocument.setContent(&file, true)) {  
		file.close();  
		return false;  
	}  
	QDomElement tree = domDocument.documentElement();   
	QDomNodeList secondItemlist = tree.childNodes();  
	//brokerId
	QDomNode nodeBroker = secondItemlist.at(0);
	QString broker = nodeBroker.toElement().attribute("name");
	if (broker.compare("broker") == 0){
		mThis->mBrokerId = nodeBroker.toElement().attribute("value");
	}
	QDomNode nodeOms = secondItemlist.at(1);
	QString omsPath = nodeOms.toElement().attribute("name");
	if (omsPath.compare("oms") == 0){
		mThis->mTradePath = nodeOms.toElement().attribute("value");
	}

	QDomNode nodeMd = secondItemlist.at(2);
	QString mdPath = nodeMd.toElement().attribute("name");
	if (mdPath.compare("md") == 0){
		mThis->mMdPath = nodeMd.toElement().attribute("value");
	}
	if (mThis->mBrokerId.isEmpty() || mThis->mMdPath.isEmpty() || mThis->mTradePath.isEmpty()){
		return false;
	}
	return true;  
}

bool USTPCtpLoader::openTradeLog()
{
	if (USTPLogger::initialize()){
		QDateTime current_date_time = QDateTime::currentDateTime();
		QString current_date = current_date_time.toString("yyyy-MM-dd");
		QString logName = tr(LOG_FILE) + current_date + tr(".txt");
		USTPLogger::setFilePath(true, logName);
		return true;
	}
	return false;
}

bool USTPCtpLoader::closeTradeLog()
{
	if (USTPLogger::finalize()){
		return true;
	}
	return false;
}

bool USTPCtpLoader::openProfileFile()
{
	if (USTPProfile::initialize())
		return true;
	return false;
}

bool USTPCtpLoader::closeProfileFile()
{
	if (USTPProfile::finalize()){
		return true;
	}
	return false;
}

bool USTPCtpLoader::startCtpMdThread()
{
	mThis->m_pMdApi = CThostFtdcMdApi::CreateFtdcMdApi("../log/");
	mThis->m_pMdApi->RegisterFront(mThis->mMdPath.toLatin1().data());
	mThis->m_pMdApi->RegisterSpi(mThis->m_pMdSpi);
	mThis->m_pMdApi->Init();
	return true;
}

bool USTPCtpLoader::startCtpTradeThread()
{
	mThis->m_pTradeApi = CThostFtdcTraderApi::CreateFtdcTraderApi("../log/");	
	mThis->m_pTradeApi->SubscribePublicTopic(THOST_TERT_QUICK);
	mThis->m_pTradeApi->SubscribePrivateTopic(THOST_TERT_QUICK);
	mThis->m_pTradeApi->RegisterFront(mThis->mTradePath.toLatin1().data());	
	mThis->m_pTradeApi->RegisterSpi(m_pTradeSpi);
	mThis->m_pTradeApi->Init();
	return true;
}

QString USTPCtpLoader::getBrokerId()
{
	return mThis->mBrokerId;
}

QString USTPCtpLoader::getAuthUser()
{
	return mThis->mAuthUser;
}

QString USTPCtpLoader::getMdPath()
{
	return mThis->mMdPath;
}

QString USTPCtpLoader::getTradePath()
{
	return mThis->mTradePath;
}

CThostFtdcMdApi* USTPCtpLoader::getMdApi()
{
	return mThis->m_pMdApi;
}

CThostFtdcTraderApi* USTPCtpLoader::getTradeApi()
{
	return mThis->m_pTradeApi;
}

USTPMdSpi* USTPCtpLoader::getMdSpi()
{
	return mThis->m_pMdSpi;
}

USTPTradeSpi* USTPCtpLoader::getTradeSpi()
{
	return mThis->m_pTradeSpi;
}

QString USTPCtpLoader::getDateTime()
{
	return mThis->mDateTime;
}