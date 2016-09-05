#include "USTPTradeApi.h"
#include <QtCore/QDebug>
#include<math.h>
#include "USTPMutexId.h"

#define CHAR_BUF_SIZE 128
#define INS_TICK 0.2
USTPTradeApi* USTPTradeApi::mThis = NULL;

bool USTPTradeApi::initialize(CThostFtdcTraderApi* pTradeApi)
{	
	mThis = new USTPTradeApi();
	mThis->m_pTradeApi = pTradeApi;
	return true;
}

bool USTPTradeApi::finalize()
{
	if(mThis != NULL){
		delete mThis;
		mThis = NULL;
	}
	return true;
}

USTPTradeApi::USTPTradeApi()
{
	nRequestId = 0;
	m_pTradeApi = NULL;
}

USTPTradeApi::~USTPTradeApi()
{

}

int USTPTradeApi::reqUserLogin(const int& reqId, const QString& brokerId, const QString& userId, const QString& password)
{
	if (mThis->m_pTradeApi != NULL){
		CThostFtdcReqUserLoginField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, brokerId.toStdString().data());
		strcpy(req.UserID, userId.toStdString().data());
		strcpy(req.Password, password.toStdString().data());
		int nResult =mThis->m_pTradeApi->ReqUserLogin(&req, reqId);
		return nResult;
	}
	return -1;
}

int USTPTradeApi::reqUserLogout(const int& reqId, const QString& brokerId, const QString& userId)
{
	if (mThis->m_pTradeApi != NULL){
		CThostFtdcUserLogoutField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, brokerId.toStdString().data());
		strcpy(req.UserID, userId.toStdString().data());
		int nResult = mThis->m_pTradeApi->ReqUserLogout(&req, reqId);
		return nResult;
	}
	return -1;
}

int USTPTradeApi::reqSettlementInfoConfirm(const int& reqId, const QString& brokerId, const QString& investorId)
{
	if (mThis->m_pTradeApi != NULL){
		CThostFtdcSettlementInfoConfirmField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, brokerId.toStdString().data());
		strcpy(req.InvestorID, investorId.toStdString().data());
		int nResult = mThis->m_pTradeApi->ReqSettlementInfoConfirm(&req, reqId);
		return nResult;
	}
	return -1;
}

int USTPTradeApi::reqOrderInsert(const int& reqId, QString& orderRef, const QString& brokerId, const QString& userId, const QString& investorId, const QString& instrumentId, const char& priceType, 
								 const char& timeCondition, const double& orderPrice, const int& volume, const char& direction, const char& offsetFlag, const char& hedgeFlag, const char& volumeCondition)
{
	if (mThis->m_pTradeApi != NULL){		
		QMutexLocker locker(&mThis->mOrderMutex);
		CThostFtdcInputOrderField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, brokerId.toStdString().data());
		req.Direction = direction;
		req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
		req.CombOffsetFlag[0] = offsetFlag;
		sprintf(req.OrderRef, "%012d", USTPMutexId::getOrderRef());
		strcpy(req.InstrumentID, instrumentId.toStdString().data());
		strcpy(req.InvestorID, investorId.toStdString().data());
		strcpy(req.UserID, userId.toStdString().data());
		orderRef = QString(req.OrderRef);
		req.IsAutoSuspend = 0;
		req.LimitPrice = orderPrice;
		req.MinVolume = 1;
		req.CombHedgeFlag[0] = hedgeFlag;
		req.OrderPriceType = priceType;
		req.TimeCondition = timeCondition;
		req.VolumeTotalOriginal = volume;
		req.VolumeCondition = volumeCondition;
		req.StopPrice = 0;
		req.ContingentCondition = THOST_FTDC_CC_Immediately;
		req.RequestID = reqId;
		int nResult = mThis->m_pTradeApi->ReqOrderInsert(&req, reqId);
		return nResult;
	}
	return -1;
}

int USTPTradeApi::reqOrderAction(const int& reqId, const QString& brokerId, const QString& userId, const QString& investorId, const QString& instrumentId, const QString& orderRef,
								 const int& frontId, const int& sessionId)
{
	if (mThis->m_pTradeApi != NULL){
		QMutexLocker locker(&mThis->mOrderMutex);
		CThostFtdcInputOrderActionField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, brokerId.toStdString().data());
		strcpy(req.UserID, userId.toStdString().data());
		strcpy(req.InvestorID, investorId.toStdString().data());
		strcpy(req.OrderRef, orderRef.toStdString().data());
		strcpy(req.InstrumentID, instrumentId.toStdString().data());
		req.FrontID = frontId;
		req.SessionID = sessionId;
		req.ActionFlag = THOST_FTDC_AF_Delete;
		int nResult = mThis->m_pTradeApi->ReqOrderAction(&req, reqId);
		return nResult;
	}
	return -1;
}

int USTPTradeApi::reqQryInstrument(const int& reqId, const QString& instrumentId, const QString& exchangeId, const QString& productId)
{	
	if (mThis->m_pTradeApi != NULL){
		CThostFtdcQryInstrumentField req;
		memset(&req, 0,sizeof(req));
		strcpy(req.InstrumentID, instrumentId.toStdString().data());
		strcpy(req.ExchangeID, exchangeId.toStdString().data());
		strcpy(req.ProductID, productId.toStdString().data());
		int nResult = mThis->m_pTradeApi->ReqQryInstrument(&req, reqId);
		return nResult;
	}
	return -1;
}

int USTPTradeApi::reqQryInvestorPosition(const int& reqId, const QString& brokerId, const QString& investorId, const QString& instrumentId)
{	
	if (mThis->m_pTradeApi != NULL){
		CThostFtdcQryInvestorPositionField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, brokerId.toStdString().data());
		strcpy(req.InvestorID, investorId.toStdString().data());
		strcpy(req.InstrumentID, instrumentId.toStdString().data());	
		int iRet = mThis->m_pTradeApi->ReqQryInvestorPosition(&req, reqId);
	}
	return -1;
}

int USTPTradeApi::reqQryTrade(const int& reqId, const QString& brokerId, const QString& investorId, const QString& instrumentId)
{	
	if (mThis->m_pTradeApi != NULL){
		CThostFtdcQryTradeField req;
		memset(&req, 0, sizeof(req));
		strcpy(req.BrokerID, brokerId.toStdString().data());
		strcpy(req.InvestorID, investorId.toStdString().data());
		strcpy(req.InstrumentID, instrumentId.toStdString().data());	
		int iRet = mThis->m_pTradeApi->ReqQryTrade(&req, reqId);
	}
	return -1;
}

#include "moc_USTPTradeApi.cpp"