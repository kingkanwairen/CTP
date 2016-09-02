#ifndef USTP_MD_SPI_H
#define USTP_MD_SPI_H

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QThread>
#include "ThostFtdcMdApi.h"

class USTPMdSpi : public QObject, public CThostFtdcMdSpi
{
	Q_OBJECT
public:

	USTPMdSpi();

	virtual~USTPMdSpi();

	virtual void OnFrontConnected();

	virtual void OnFrontDisconnected(int nReason);

	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

signals:
	void onUSTPMdFrontConnected();
	void onUSTPMdFrontDisconnected(int reason);
	void onUSTPMdRspUserLogin(const QString& brokerId, const QString& userId, const int& errorId, const QString& errorMsg, bool bIsLast);
	void onUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
		const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
		const double& lowestPrice, const int& volume, const double& preClosePrice, const double& holdPosition);

private:
	QThread mMdThread;
};

#endif