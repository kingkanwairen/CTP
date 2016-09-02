#include "USTPComplexMd.h"
#include <QtCore/QThread>
#include "USTPMutexId.h"
#include "USTPCtpLoader.h"
#include "USTPMarketWidget.h"
#include "USTPConfig.h"


USTPComplexMd::USTPComplexMd(const QString& selIns, USTPMarketWidget* pMarketWidget)
{	
	mBidPrice = 0.0;
	mAskPrice = 0.0;
	mBidQty = 0;
	mAskQty = 0;
	mTradeQty = 0;
	mInstrument = selIns;
	mIsInit = true;
	mInsPrecision = getInsPrcision(USTPMutexId::getInsPriceTick(selIns));
	initConnect(pMarketWidget);
	moveToThread(&mComplexMdThread);	
	mComplexMdThread.start();
}

USTPComplexMd::~USTPComplexMd()
{
	mComplexMdThread.quit();
	mComplexMdThread.wait();
}

void USTPComplexMd::initConnect(USTPMarketWidget* pMarketWidget)
{	
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPRtnDepthMarketData(const QString&, const double&, const double&, const double&,const double&, 
		const int&, const double&, const int&, const double&, const double&, const int&, const double&, const double&)), this, SLOT(doUSTPRtnDepthMarketData(const QString&, const double&, 
		const double&, const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&, const double&, const double&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUSTPSpeComplexMd(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&)), pMarketWidget, SLOT(doUSTPSpeComplexMd(const QString&, const QString&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

	connect(pMarketWidget, SIGNAL(onGetSpeDepthMd(const QString&, const int&)), this, SLOT(doGetSpeDepthMd(const QString&, const int&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateSpeDepthMd(const QString&, const int&, const double&)), pMarketWidget, SLOT(doUpdateSpeDepthMd(const QString&, const int&, const double&)), Qt::QueuedConnection);
}

void USTPComplexMd::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
											 const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
											 const double& lowestPrice, const int& volume, const double& preClosePrice, const double& holdPosition)
{
	if(mInstrument.compare(instrumentId) == 0){
		mBidPrice = bidPrice;
		mAskPrice = askPrice;
		mBidQty = bidVolume;
		mAskQty = askVolume;
		mTradeQty = volume;
		double fUpDownPrice = lastPrice - preSettlementPrice;
		QString szUpDownPrice = QString("%1").arg(fUpDownPrice, 0, 'f', mInsPrecision);
		QString szPreClosePrice = QString("%1").arg(preClosePrice, 0, 'f', mInsPrecision);
		QString szOpenPrice = QString("%1").arg(openPrice, 0, 'f', mInsPrecision);
		QString szBidPrice = QString("%1").arg(bidPrice, 0, 'f', mInsPrecision);
		QString szAskPrice = QString("%1").arg(askPrice, 0, 'f', mInsPrecision);
		QString szLastPrice = QString("%1").arg(lastPrice, 0, 'f', mInsPrecision);
		QString szHoldQty = QString("%1").arg((long)holdPosition);
		QString szBidQty = QString::number(bidVolume);
		QString szAskQty = QString::number(askVolume);
		QString szTradeQty = QString::number(volume);

		emit onUSTPSpeComplexMd(instrumentId, szBidPrice, szBidQty, szAskPrice, szAskQty, szTradeQty, szLastPrice, szUpDownPrice, szPreClosePrice, szOpenPrice,
			szHoldQty);
		if(mIsInit && highestPrice > 0.0 && lowestPrice > 0.0){
			mIsInit = false;
			USTPMutexId::setUpperLowerPrice(instrumentId, highestPrice, lowestPrice);
		}
	}
}


int USTPComplexMd::getInsPrcision(const double& value)
{
	if (value >= VALUE_1){
		return 0;
	}else if(value * 10 >= VALUE_1){
		return 1;
	}else if(value * 100 >= VALUE_1){
		return 2;
	}else if(value * 1000 >= VALUE_1){
		return 3;
	}
	return 0;
}

void USTPComplexMd::doGetSpeDepthMd(const QString& selIns, const int& direction)
{
	if(mInstrument == selIns){
		switch (direction)
		{
		case 0:
			emit onUpdateSpeDepthMd(mInstrument, 0, mBidPrice);
			break;
		case 1:
			emit onUpdateSpeDepthMd(mInstrument, 1, mAskPrice);
			break;
		case 2:
			emit onUpdateSpeDepthMd(mInstrument, 2, mBidPrice);
			break;
		case 3:
			emit onUpdateSpeDepthMd(mInstrument, 3, mAskPrice);
			break;
		}
	}
}


#include "moc_USTPComplexMd.cpp"