#include "USTPMdSpi.h"
#include "USTPConfig.h"
#include "USTPLogger.h"
#include <QtCore/QThread>

USTPMdSpi::USTPMdSpi()
{
	moveToThread(&mMdThread);
	mMdThread.start();
}

USTPMdSpi::~USTPMdSpi()
{
	mMdThread.quit();
	mMdThread.wait();
}

void USTPMdSpi::OnFrontConnected()
{	
	emit onUSTPMdFrontConnected();
	QString data = QString(tr("[Md-Connected]  "));
	USTPLogger::saveData(data);
}

void USTPMdSpi::OnFrontDisconnected(int nReason)
{	
	emit onUSTPMdFrontDisconnected(nReason);
	QString data = QString(tr("[Md-Disconneted]  ")) +  QString::number(nReason);
	USTPLogger::saveData(data);
}

void USTPMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspUserLogin != NULL && pRspInfo != NULL){
		emit onUSTPMdRspUserLogin(QString(pRspUserLogin->BrokerID), QString(pRspUserLogin->UserID), pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg), bIsLast);
	}
#ifdef _MD_DEBUG
	QString data = QString(tr("[Md-RspUserLogin]  BrokerId: ")) + QString(pRspUserLogin->BrokerID) + tr("  UserId: ") + QString(pRspUserLogin->UserID)
		+ tr("  ErrorMsg: ") + QString::fromLocal8Bit(pRspInfo->ErrorMsg);
	USTPLogger::saveData(data);
#endif
}

void USTPMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (pDepthMarketData != NULL){
		emit onUSTPRtnDepthMarketData(QString(pDepthMarketData->InstrumentID), pDepthMarketData->PreSettlementPrice, pDepthMarketData->OpenPrice, 
			pDepthMarketData->LastPrice, pDepthMarketData->BidPrice1, pDepthMarketData->BidVolume1, pDepthMarketData->AskPrice1, pDepthMarketData->AskVolume1,
			pDepthMarketData->UpperLimitPrice, pDepthMarketData->LowerLimitPrice, pDepthMarketData->Volume, pDepthMarketData->PreClosePrice, pDepthMarketData->OpenInterest);

#ifdef _MD_DEBUG
		QString data = QString(tr("[Depth]  InstrumentId: ")) + QString(pDepthMarketData->InstrumentID) + QString(tr("  BidPrice: ")) +
			QString::number(pDepthMarketData->BidPrice1) + QString(tr("  BidVolume: ")) + QString::number(pDepthMarketData->BidVolume1) + QString(tr("  AskPrice: ")) +
			QString::number(pDepthMarketData->AskPrice1) + QString(tr("  AskVolume: ")) + QString::number(pDepthMarketData->AskVolume1) + QString(tr("  SettlePrice: ")) +
			QString::number(pDepthMarketData->PreSettlementPrice);
		USTPLogger::saveData(data);
#endif

	}
}

#include "moc_USTPMdSpi.cpp"