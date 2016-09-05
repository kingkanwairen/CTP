#include "USTPMutexId.h"
#include "USTPConfig.h"

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

QString USTPMutexId::getUserId()
{
	return mThis->mUserId;
}

QString USTPMutexId::getLoginPsw()
{
	return mThis->mPsw;
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

bool USTPMutexId::finalize()
{
	if (mThis != NULL) {
		delete mThis;
		mThis = NULL;
	}
	return true;
}

#include "moc_USTPMutexId.cpp"