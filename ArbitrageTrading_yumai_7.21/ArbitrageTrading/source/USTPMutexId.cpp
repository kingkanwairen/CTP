#include "USTPMutexId.h"
#include "USTPConfig.h"
#include <QtCore/QDateTime>

USTPMutexId* USTPMutexId::mThis = NULL;

USTPMutexId::USTPMutexId()
{
	
}

USTPMutexId::~USTPMutexId()
{

}

bool USTPMutexId::initialize()
{
	mThis = new USTPMutexId;	
	mThis->nRequestIndex = INIT_VALUE;
	mThis->nNewOrderIndex = INIT_VALUE;
	mThis->nMarketIndex = 0;
	mThis->mCurrentIndex = 0;
	mThis->mReferenceIns = "";
	QDateTime current_date_time = QDateTime::currentDateTime();
	QString current_date = current_date_time.toString("hhmmss");
	int nCurrentTime = current_date.toInt();
	if((nCurrentTime >= MORNING_BEGIN_TIME && nCurrentTime <= MORNING_END_TIME) ||(nCurrentTime >= NIGHT_BEGIN_TIME && nCurrentTime <= NIGHT_END_TIME))
		mThis->mIsBaseTime = true;
	else
		mThis->mIsBaseTime = false;
	return true;
}

bool USTPMutexId::setUserInfo(const QString& userId, const QString& psw, int maxId, int frontId, int sessionId)
{
	mThis->mUserId = userId;
	mThis->mInvestorId = userId;
	mThis->mFrontId = frontId;
	mThis->mSessionId = sessionId;
	mThis->nOrderId = maxId;
	mThis->mPsw = psw;
	return true;
}

QString USTPMutexId::getInvestorId()
{
	return mThis->mInvestorId;
}

void USTPMutexId::setInvestorId(const QString& investorId)
{
	mThis->mInvestorId = investorId;
}

int USTPMutexId::getOrderRef()
{
	QMutexLocker locker(&mThis->mOrderRefMutex);
	mThis->nOrderId++;
	return mThis->nOrderId;
}

int USTPMutexId::getMutexId()
{
	QMutexLocker locker(&mThis->mRequestMutex);
	mThis->nRequestIndex++;
	return mThis->nRequestIndex;
}

int USTPMutexId::getNewOrderIndex()
{
	QMutexLocker locker(&mThis->mNewOrderMutex);
	mThis->nNewOrderIndex++;
	return mThis->nNewOrderIndex;
}

QString USTPMutexId::getUserId()
{
	return mThis->mUserId;
}

QString USTPMutexId::getLoginPsw()
{
	return mThis->mPsw;
}

int USTPMutexId::getFrontId()
{
	return mThis->mFrontId;
}

int USTPMutexId::getSessionId()
{
	return mThis->mSessionId;
}

bool USTPMutexId::setInsBidPosition(const QString& ins, const int& qty)
{
	if(mThis->mBidQtys.find(ins) != mThis->mBidQtys.end()){
		mThis->mBidQtys[ins] = qty;
	}else{
		mThis->mBidQtys.insert(ins, qty);
	}
	return true;
}

bool USTPMutexId::setInsAskPosition(const QString& ins, const int& qty)
{
	if(mThis->mAskQtys.find(ins) != mThis->mAskQtys.end()){
		mThis->mAskQtys[ins] = qty;
	}else{
		mThis->mAskQtys.insert(ins, qty);
	}
	return true;
}

bool USTPMutexId::addBidPosition(const QString& ins, const int& qty)
{
	if(mThis->mBidQtys.find(ins) != mThis->mBidQtys.end()){
		mThis->mBidQtys[ins] += qty;
	}else{
		mThis->mBidQtys.insert(ins, qty);
	}
	return true;
}

bool USTPMutexId::addAskPosition(const QString& ins, const int& qty)
{
	if(mThis->mAskQtys.find(ins) != mThis->mAskQtys.end()){
		mThis->mAskQtys[ins] += qty;
	}else{
		mThis->mAskQtys.insert(ins, qty);
	}
	return true;
}

 bool USTPMutexId::setReferenceIns(const QString& ins)
 {	
	 if(mThis->mReferenceIns != "")
		 return true;
	 mThis->mReferenceIns = ins;
	 return true;
 }

int USTPMutexId::getInsBidPosition(const QString& ins)
{
	if(mThis->mBidQtys.find(ins) != mThis->mBidQtys.end()){
		return mThis->mBidQtys[ins];
	}
	return 0;
}

int USTPMutexId::getInsAskPosition(const QString& ins)
{
	if(mThis->mAskQtys.find(ins) != mThis->mAskQtys.end()){
		return mThis->mAskQtys[ins];
	}
	return 0;
}

bool USTPMutexId::setInsPriceTick(const QString& ins, const QString& exchangeId, const double& tick, const int& volumeMultiple)
{
	if(mThis->mInsTicks.find(ins) != mThis->mInsTicks.end()){
		return false;
	}
	mThis->mInsTicks.insert(ins, tick);
	mThis->mInsExhs.insert(ins, exchangeId);
	mThis->mInsMultiple.insert(ins, volumeMultiple);
	return true;
}

bool USTPMutexId::setInsMarketMaxVolume(const QString& ins, const int& volume)
{
	if(mThis->mMaxMarketQtys.find(ins) != mThis->mMaxMarketQtys.end()){
		return false;
	}
	mThis->mMaxMarketQtys.insert(ins, volume);
	return true;
}

int USTPMutexId::getInsMarketMaxVolume(const QString& ins)
{
	if(mThis->mMaxMarketQtys.find(ins) != mThis->mMaxMarketQtys.end()){
		return mThis->mMaxMarketQtys[ins];
	}
	return 0;
}

double USTPMutexId::getInsPriceTick(const QString& ins)
{
	if(mThis->mInsTicks.find(ins) != mThis->mInsTicks.end()){
		return mThis->mInsTicks[ins];
	}
	return 0.0;
}

QString USTPMutexId::getInsExchangeId(const QString& ins)
{
	if(mThis->mInsExhs.find(ins) != mThis->mInsExhs.end()){
		return mThis->mInsExhs[ins];
	}
	return QString(tr(""));
}

int USTPMutexId::getInsMultiple(const QString& ins)
{
	if(mThis->mInsMultiple.find(ins) != mThis->mInsMultiple.end()){
		return mThis->mInsMultiple[ins];
	}
	return 0;
}

bool USTPMutexId::getTotalBidPosition(QMap<QString, int>& bidPostions)
{	
	QMapIterator<QString, int> i(mThis->mBidQtys);
	while (i.hasNext()){
		i.next();
		bidPostions.insert(i.key(), i.value());
	}
	return true;
}

bool USTPMutexId::getTotalAskPosition(QMap<QString, int>& askPostions)
{
	QMapIterator<QString, int> i(mThis->mAskQtys);
	while (i.hasNext()){
		i.next();
		askPostions.insert(i.key(), i.value());
	}
	return true;
}

QString USTPMutexId::getReferenceIns()
{
	return mThis->mReferenceIns;
}

int USTPMutexId::getMarketIndex()
{
	QMutexLocker locker(&mThis->mMarketMutex);
	mThis->nMarketIndex++;
	return mThis->nMarketIndex;
}

bool USTPMutexId::setLimitSpread(const QString& ins, const int& spread)
{
	if(mThis->mLimitSpreads.find(ins) != mThis->mLimitSpreads.end()){
		return false;
	}
	mThis->mLimitSpreads.insert(ins, spread);
	return true;
}

int USTPMutexId::getLimitSpread(const QString& ins)
{
	if(mThis->mLimitSpreads.find(ins) != mThis->mLimitSpreads.end()){
		return mThis->mLimitSpreads[ins];
	}
	return 30;
}

bool USTPMutexId::initActionNum(const QString& ins, const int& num)
{
	if(mThis->mInsActionNums.find(ins) != mThis->mInsActionNums.end()){
		return false;
	}
	if(mThis->mIsBaseTime)
		mThis->mInsActionNums.insert(ins, num);
	else
		mThis->mInsActionNums.insert(ins, 0);
	return true;
}

bool USTPMutexId::updateActionNum(const QString& ins)
{	
	QMutexLocker locker(&mThis->mInsActionMutex);
	if(mThis->mInsActionNums.find(ins) != mThis->mInsActionNums.end())
		mThis->mInsActionNums[ins] += 1;
	else
		mThis->mInsActionNums.insert(ins, 1);
	return true;
}

int USTPMutexId::getActionNum(const QString& ins)
{
	if(mThis->mInsActionNums.find(ins) != mThis->mInsActionNums.end()){
		return mThis->mInsActionNums[ins];
	}
	return 0;
}

bool USTPMutexId::getTotalActionNum(QMap<QString, int>& actionNums)
{
	QMapIterator<QString, int> i(mThis->mInsActionNums);
	while (i.hasNext()){
		i.next();
		actionNums.insert(i.key(), i.value());
	}
	return true;
}

bool USTPMutexId::setUpperLowerPrice(const QString& ins, const double& upperPrice, const double& lowerPrice)
{
	if(mThis->mUpperPrices.find(ins) != mThis->mUpperPrices.end()){
		return false;
	}
	mThis->mUpperPrices.insert(ins, upperPrice);

	if(mThis->mLowerPrices.find(ins) != mThis->mLowerPrices.end()){
		return false;
	}
	mThis->mLowerPrices.insert(ins, lowerPrice);
	return true;
}

double USTPMutexId::getUpperPrice(const QString& ins)
{
	if(mThis->mUpperPrices.find(ins) != mThis->mUpperPrices.end()){
		return mThis->mUpperPrices[ins];
	}
	return 0.0;
}

double USTPMutexId::getLowerPrice(const QString& ins)
{
	if(mThis->mLowerPrices.find(ins) != mThis->mLowerPrices.end()){
		return mThis->mLowerPrices[ins];
	}
	return 0.0;
}

bool USTPMutexId::setCurrentTabIndex(const int& index)
{
	mThis->mCurrentIndex = index;
	return true;
}

int USTPMutexId::getCurrentTabIndex()
{
	return mThis->mCurrentIndex;
}


bool USTPMutexId::finalize()
{
	if (mThis != NULL) {
		delete mThis;
		mThis = NULL;
	}
	return true;
}

#include "moc_USTPMutexId.cpp"