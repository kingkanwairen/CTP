#include "USTPComplexMd.h"
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include "USTPMutexId.h"
#include "USTPCtpLoader.h"
#include "USTPMarketWidget.h"
#include "USTPConfig.h"
#include "USTPLogger.h"

USTPComplexMd::USTPComplexMd(const QString& firstIns, const QString& secondIns, const double& firstFactor, const double& secFactor, const double& constValue,
							 USTPMarketWidget* pMarketWidget)
{	
	mFirstBidPrice = 0.0;
	mFirstAskPrice = 0.0;
	mFirstBidQty = 0;
	mFirstAskQty = 0;
	mFirstFactor = firstFactor;
	mSecFactor = secFactor;
    mConstFactor = constValue;
	mFirstInstrument = firstIns;
	mSecondInstrument = secondIns;
	mComplexKey = mFirstInstrument + tr("|") + mSecondInstrument;
	mInsPrecision = getInsPrcision(USTPMutexId::getInsPriceTick(firstIns));
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
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPRtnDepthMarketData(const QString&, const double&, const double&, const double&,const double&, const int&, 
		const double&, const int&, const double&, const double&, const int&)), this, SLOT(doUSTPRtnDepthMarketData(const QString&, const double&, const double&, 
		const double&,const double&, const int&, const double&, const int&, const double&, const double&, const int&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUSTPComplexMd(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&)), pMarketWidget, SLOT(doUSTPComplexMd(const QString&, const QString&, const QString&, const QString&, const QString&,
		const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)), Qt::QueuedConnection);

}

void USTPComplexMd::doUSTPRtnDepthMarketData(const QString& instrumentId, const double& preSettlementPrice, const double& openPrice, const double& lastPrice,
											 const double& bidPrice, const int& bidVolume, const double& askPrice, const int& askVolume, const double& highestPrice, 
											 const double& lowestPrice, const int& volume)
{
	if(mFirstInstrument.compare(instrumentId) == 0){
		mFirstBidPrice = bidPrice;
		mFirstAskPrice = askPrice;
		mFirstBidQty = bidVolume;
		mFirstAskQty = askVolume;
		calculateComplexMd();
	}else if (mSecondInstrument.compare(instrumentId) == 0){
		mSecondBidPrice = bidPrice;
		mSecondAskPrice = askPrice;
		mSecondBidQty = bidVolume;
		mSecondAskQty = askVolume;
		calculateComplexMd();
	}
}

void USTPComplexMd::calculateComplexMd()
{
	double bbPrice = mFirstBidPrice * mFirstFactor + mSecondBidPrice * mSecFactor + mConstFactor;
	double bsPrice = mFirstBidPrice * mFirstFactor + mSecondAskPrice * mSecFactor + mConstFactor;
	double sbPrice = mFirstAskPrice * mFirstFactor + mSecondBidPrice * mSecFactor + mConstFactor;
	double ssPrice = mFirstAskPrice * mFirstFactor + mSecondAskPrice * mSecFactor + mConstFactor;
	
	int bbQty = mFirstBidQty > mSecondBidQty ? mSecondBidQty : mFirstBidQty;
	int bsQty = mFirstBidQty > mSecondAskQty ? mSecondAskQty : mFirstBidQty;
	int sbQty = mFirstAskQty > mSecondBidQty ? mSecondBidQty : mFirstAskQty;
	int ssQty = mFirstAskQty > mSecondAskQty ? mSecondAskQty : mFirstAskQty;
	QString szBBPrice = QString("%1").arg(bbPrice, 0, 'f', mInsPrecision);
	QString szBSPrice = QString("%1").arg(bsPrice, 0, 'f', mInsPrecision);
	QString szSBPrice = QString("%1").arg(sbPrice, 0, 'f', mInsPrecision);
	QString szSSPrice = QString("%1").arg(ssPrice, 0, 'f', mInsPrecision);
	
	QString szBBQty = QString::number(bbQty);
	QString szBSQty = QString::number(bsQty);
	QString szSBQty = QString::number(sbQty);
	QString szSSQty = QString::number(ssQty);
	emit onUSTPComplexMd(mComplexKey, mFirstInstrument, mSecondInstrument, szBBPrice, szBSPrice, szSBPrice, szSSPrice, szBBQty, szBSQty, szSBQty, szSSQty);
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

#include "moc_USTPComplexMd.cpp"