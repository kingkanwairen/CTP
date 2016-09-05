#include "USTPComplexMd.h"
#include <QtCore/QThread>
#include "USTPMutexId.h"
#include "USTPCtpLoader.h"
#include "USTPMarketWidget.h"
#include "USTPConfig.h"

#define BS_VALUE -100000.0

USTPComplexMd::USTPComplexMd(const QString& firstIns, const QString& secondIns, USTPMarketWidget* pMarketWidget)
{	
	mFirstBidPrice = 0.0;
	mFirstAskPrice = 0.0;
	mFirstBidQty = 0;
	mFirstAskQty = 0;
	mSecondBidPrice = 0.0;
	mSecondAskPrice = 0.0;

	mFirstBidPriceDiff = BS_VALUE;
	mSecondBidPriceDiff = BS_VALUE;
	mThirdBidPriceDiff = BS_VALUE;

	mFirstAskPriceDiff = BS_VALUE;
	mSecondAskPriceDiff = BS_VALUE;
	mThirdAskPriceDiff = BS_VALUE;

	mSecondBidQty = 0;
	mSecondAskQty = 0;
	mFirstInstrument = firstIns;
	mSecondInstrument = secondIns;
	mComplexKey = mFirstInstrument + tr("|") + mSecondInstrument;
	int firstInsPrecision = getInsPrcision(USTPMutexId::getInsPriceTick(firstIns));
	int secInsPrecision = getInsPrcision(USTPMutexId::getInsPriceTick(secondIns));
	mInsPrecision = firstInsPrecision > secInsPrecision ? firstInsPrecision : secInsPrecision;
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

	connect(pMarketWidget, SIGNAL(onGetDepthMd(const QString&, const QString&, const int&, const int&)), this, SLOT(doGetDepthMd(const QString&, const QString&, const int&, const int&)), Qt::QueuedConnection);
	
	connect(pMarketWidget, SIGNAL(onGetComplexDepthPrice(const QString&, const QString&, const int&)), this, SLOT(doGetComplexDepthPrice(const QString&, const QString&, const int&)), Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateDepthMd(const QString&, const QString&, const int&, const int&, const double&, const double&, const double&, const double&, const double&, const double&, const double&)), 
		pMarketWidget, SLOT(doUpdateDepthMd(const QString&, const QString&, const int&, const int&, const double&, const double&, const double&, const double&, const double&, const double&, const double&)), 
		Qt::QueuedConnection);

	connect(this, SIGNAL(onUpdateComplexDepthPrice(const QString&, const QString&, const int&, const double&, const double&,const double&, const double&)), 
		pMarketWidget, SLOT(doUpdateComplexDepthPrice(const QString&, const QString&, const int&, const double&, const double&,const double&, const double&)), Qt::QueuedConnection);

	connect(pMarketWidget, SIGNAL(onGetEnbedMd(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, const int&, bool)), 
		this, SLOT(doGetEnbedMd(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, const int&, bool)));

	connect(this, SIGNAL(onUpdateEnbedDepthPrice(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, const int&, bool, 
		const double&, const double&,const double&, const double&)), 
		pMarketWidget, SLOT(doUpdateEnbedDepthPrice(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, const int&, bool,
		const double&, const double&,const double&, const double&)));
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
	double bbPrice = mFirstBidPrice - mSecondBidPrice;
	double bsPrice = mFirstBidPrice - mSecondAskPrice;
	double sbPrice = mFirstAskPrice - mSecondBidPrice;
	double ssPrice = mFirstAskPrice - mSecondAskPrice;
	int bbQty = mFirstBidQty > mSecondBidQty ? mSecondBidQty : mFirstBidQty;
	int bsQty = mFirstBidQty > mSecondAskQty ? mSecondAskQty : mFirstBidQty;
	int sbQty = mFirstAskQty > mSecondBidQty ? mSecondBidQty : mFirstAskQty;
	int ssQty = mFirstAskQty > mSecondAskQty ? mSecondAskQty : mFirstAskQty;
	
	mThirdBidPriceDiff = mSecondBidPriceDiff;
	mSecondBidPriceDiff = mFirstBidPriceDiff;
	mFirstBidPriceDiff = bbPrice;

	mThirdAskPriceDiff = mSecondAskPriceDiff;
	mSecondAskPriceDiff = mFirstAskPriceDiff;
	mFirstAskPriceDiff = ssPrice;

	if(mSecondBidPriceDiff <= BS_VALUE)
		mSecondBidPriceDiff = mFirstBidPriceDiff;
	if(mThirdBidPriceDiff <= BS_VALUE)
		mThirdBidPriceDiff = mSecondBidPriceDiff;
	if(mSecondAskPriceDiff <= BS_VALUE)
		mSecondAskPriceDiff = mFirstAskPriceDiff;
	if(mThirdAskPriceDiff <= BS_VALUE)
		mThirdAskPriceDiff = mSecondAskPriceDiff;

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

void USTPComplexMd::doGetDepthMd(const QString& complexIns, const QString& reqKey, const int& direction, const int& rowIndex)
{
	if(mComplexKey == complexIns){
		switch (direction)
		{
		case 0:
			emit onUpdateDepthMd(mComplexKey, reqKey, rowIndex, direction, mFirstBidPriceDiff, mSecondBidPriceDiff, mThirdBidPriceDiff, mFirstBidPrice, 
				mFirstAskPrice, mSecondBidPrice, mSecondAskPrice);
			break;
		case 1:
			emit onUpdateDepthMd(mComplexKey, reqKey, rowIndex, direction, mFirstAskPriceDiff, mSecondAskPriceDiff, mThirdAskPriceDiff, mFirstBidPrice, 
				mFirstAskPrice, mSecondBidPrice, mSecondAskPrice);
			break;
		}
	}
}

void USTPComplexMd::doGetComplexDepthPrice(const QString& complexIns, const QString& reqKey, const int& rowIndex)
{
	if(mComplexKey == complexIns)
		onUpdateComplexDepthPrice(complexIns, reqKey, rowIndex, mFirstBidPrice, mFirstAskPrice, mSecondBidPrice, mSecondAskPrice);
}

void USTPComplexMd::doGetEnbedMd(const QString& firstIns, const QString& secIns, const QString& orderStyle, const QString& direction, const QString& offsetFlag, 
				  const QString& hedgeFlag, const double& priceTick, const int& orderQty, bool isEnbed)
{
	QString szComplexKey = firstIns + tr("|") + secIns;
	if(mComplexKey == szComplexKey)
		emit onUpdateEnbedDepthPrice(firstIns, secIns, orderStyle, direction, offsetFlag, hedgeFlag, priceTick, orderQty, isEnbed, mFirstBidPrice, mFirstAskPrice, mSecondBidPrice, mSecondAskPrice);
}

#include "moc_USTPComplexMd.cpp"