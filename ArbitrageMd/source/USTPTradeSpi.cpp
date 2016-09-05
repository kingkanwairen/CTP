#include "USTPTradeSpi.h"
#include "USTPConfig.h"
#include "USTPLogger.h"
#include <QtCore/QThread>
#include <QtCore/QDebug>

USTPTradeSpi::USTPTradeSpi()
{
	moveToThread(&mTradeThread);
	mTradeThread.start();
}

USTPTradeSpi::~USTPTradeSpi()
{
	mTradeThread.quit();
	mTradeThread.wait();
}

void USTPTradeSpi::OnFrontConnected()
{	
	emit onUSTPTradeFrontConnected();
	QString data = QString(tr("[Trade-Connected]  "));
	USTPLogger::saveData(data);
}

void USTPTradeSpi::OnFrontDisconnected(int nReason)
{	
	emit onUSTPTradeFrontDisconnected(nReason);
	QString data = QString(tr("[Trade-Disconneted]  ")) +  QString::number(nReason);
	USTPLogger::saveData(data);
}

void USTPTradeSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
#ifdef _DEBUG
	QString data = QString(tr("[Trade-RspError]  ErrorId:")) + QString::number(pRspInfo->ErrorID) + tr("  ErrorMsg: ") + QString::fromLocal8Bit(pRspInfo->ErrorMsg);
	USTPLogger::saveData(data);
#endif
}

void USTPTradeSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pRspUserLogin != NULL && pRspInfo != NULL){
		emit onUSTPTradeRspUserLogin(QString(pRspUserLogin->TradingDay), QString(pRspUserLogin->BrokerID), QString(pRspUserLogin->UserID),
			atoi(pRspUserLogin->MaxOrderRef), pRspUserLogin->FrontID, pRspUserLogin->SessionID, pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg), bIsLast);
#ifdef _DEBUG
		QString data = QString(tr("[Trade-RspUserLogin]  UserId: ")) + QString(pRspUserLogin->UserID) + tr("  TradingDate: ") + QString(pRspUserLogin->TradingDay)
			+ tr("  ErrorMsg: ") + QString::fromLocal8Bit(pRspInfo->ErrorMsg);
		USTPLogger::saveData(data);
#endif
	}
}

void USTPTradeSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pUserLogout != NULL && pRspInfo != NULL){
		emit onUSTPTradeRspUserLogout(QString(pUserLogout->BrokerID), QString(pUserLogout->UserID), pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg));
	}
}

void USTPTradeSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pSettlementInfoConfirm != NULL && pRspInfo != NULL){
		emit onUSTPTradeSettlementInfoConfirm(QString(pSettlementInfoConfirm->BrokerID), QString(pSettlementInfoConfirm->InvestorID), pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg));
	}
}

void USTPTradeSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	if (pTrade != NULL){
		emit onUSTPRtnTrade(QString(pTrade->TradeID), QString(pTrade->InstrumentID), pTrade->Direction, pTrade->Volume, pTrade->Price,
			pTrade->OffsetFlag, pTrade->HedgeFlag, QString(pTrade->BrokerID), QString(pTrade->ExchangeID), QString(pTrade->InvestorID), 
			QString(pTrade->OrderSysID), QString(pTrade->OrderLocalID), QString(pTrade->OrderRef), QString(pTrade->TradeTime));
#ifdef _DEBUG
		QString data = QString(tr("[OnRtnTrade]  InvestorId: ")) + QString(pTrade->InvestorID) + tr("  OrderSysID: ") + QString(pTrade->OrderSysID)
			+ tr("  InstrumentId: ") + QString(pTrade->InstrumentID) + tr("  Direction: ") + QString(pTrade->Direction) + tr("  TradePrice: ") + 
			QString::number(pTrade->Price) + tr("  TradeVolume: ") + QString::number(pTrade->Volume) + tr("  TradeTime: ") + QString(pTrade->TradeTime)
			+ tr("  OrderRef: ") + QString(pTrade->OrderRef);
		USTPLogger::saveData(data);
#endif
	}
}

void USTPTradeSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{	
	if (pOrder != NULL){
		emit onUSTPRtnOrder(QString(pOrder->OrderLocalID), QString(pOrder->OrderRef), QString(pOrder->InstrumentID), pOrder->Direction, pOrder->LimitPrice, pOrder->VolumeTotalOriginal,
			pOrder->VolumeTotal, pOrder->VolumeTraded, pOrder->CombOffsetFlag[0], pOrder->OrderPriceType, pOrder->CombHedgeFlag[0], pOrder->OrderStatus,
			QString(pOrder->BrokerID), QString(pOrder->ExchangeID), QString(pOrder->InvestorID), QString(pOrder->OrderSysID), QString(pOrder->StatusMsg), 
			pOrder->TimeCondition, pOrder->RequestID);
#ifdef _DEBUG
		QString data = QString(tr("[OnRtnOrder]  InvestorId: ")) + QString(pOrder->InvestorID) + tr("  OrderRef: ") + QString(pOrder->OrderRef)
			+ tr("  InstrumentId: ") + QString(pOrder->InstrumentID) + tr("  Direction: ") + QString(pOrder->Direction) + tr("  OrderPrice: ") + 
			QString::number(pOrder->LimitPrice) + tr("  OrderStatus: ") + QString(pOrder->OrderStatus) + tr("  OrderSysID: ") + QString(pOrder->OrderSysID);
		USTPLogger::saveData(data);
#endif
	}
}

void USTPTradeSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	if (pInputOrder != NULL && pRspInfo != NULL){
		emit onUSTPErrRtnOrderInsert(QString(pInputOrder->UserID), QString(pInputOrder->BrokerID), pInputOrder->Direction, QString(pInputOrder->GTDDate), pInputOrder->CombHedgeFlag[0],
			QString(pInputOrder->InstrumentID), QString(pInputOrder->InvestorID), pInputOrder->CombOffsetFlag[0], pInputOrder->OrderPriceType, pInputOrder->TimeCondition,
			QString(pInputOrder->OrderRef), pInputOrder->LimitPrice, pInputOrder->VolumeTotalOriginal, pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg), pInputOrder->RequestID);
#ifdef _DEBUG
		QString data = QString(tr("[ErrRtnOrderInsert]  InvestorId: ")) + QString(pInputOrder->InvestorID) + tr("  OrderRef: ") + QString(pInputOrder->OrderRef)
			+ tr("  InstrumentId: ") + QString(pInputOrder->InstrumentID) + tr("  Direction: ") + QString(pInputOrder->Direction) + tr("  OrderPrice: ") + 
			QString::number(pInputOrder->LimitPrice) + tr("  OrderVolume: ") + QString::number(pInputOrder->VolumeTotalOriginal) + tr("  ErrorMsg: ") +  QString::fromLocal8Bit(pRspInfo->ErrorMsg);
		USTPLogger::saveData(data);
#endif
	}
}

void USTPTradeSpi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	if (pOrderAction != NULL && pRspInfo != NULL){
		emit onUSTPErrRtnOrderAction(pOrderAction->ActionFlag, QString(pOrderAction->BrokerID), QString(pOrderAction->ExchangeID), QString(pOrderAction->InvestorID),
			QString(pOrderAction->OrderSysID), QString(pOrderAction->ActionLocalID), QString(pOrderAction->OrderRef), pOrderAction->LimitPrice, 
			pOrderAction->VolumeChange, pRspInfo->ErrorID, QString::fromLocal8Bit(pRspInfo->ErrorMsg), pOrderAction->RequestID);
#ifdef _DEBUG
		QString data = QString(tr("[ErrRtnOrderAction]  InvestorId: ")) + QString(pOrderAction->InvestorID) + tr("  OrderSysID: ") + QString(pOrderAction->OrderSysID)
			+ tr("  ActionLocalID: ") + QString(pOrderAction->ActionLocalID) + tr("  OrderRef: ") + QString(pOrderAction->OrderRef) + tr("  ErrorMsg: ") + 
			QString::fromLocal8Bit(pRspInfo->ErrorMsg);
		USTPLogger::saveData(data);
#endif
	}
}

void USTPTradeSpi::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
	if (pInstrumentStatus != NULL){
		emit onUSTPRtnInstrumentStatus(QString(pInstrumentStatus->ExchangeID), QString(pInstrumentStatus->InstrumentID), pInstrumentStatus->InstrumentStatus);
	}
}

void USTPTradeSpi::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (pInstrument != NULL ){
		emit onUSTPRspQryInstrument((pInstrument->ExchangeID), QString(pInstrument->ProductID), QString(pInstrument->InstrumentID),
			pInstrument->PriceTick, pInstrument->VolumeMultiple, pInstrument->MaxMarketOrderVolume, bIsLast);
	}

}

void USTPTradeSpi::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pInvestorPosition == NULL){
		emit onUSTPRspQryInvestorPosition("", -1, 0, 0, '1', "", "","", bIsLast);
		return;
	}
	if (pInvestorPosition != NULL){	
		emit onUSTPRspQryInvestorPosition(QString(pInvestorPosition->InstrumentID), pInvestorPosition->PosiDirection, pInvestorPosition->Position, 
			pInvestorPosition->YdPosition, pInvestorPosition->HedgeFlag, QString(pInvestorPosition->BrokerID), QString(pInvestorPosition->TradingDay),
			QString(pInvestorPosition->InvestorID), bIsLast);
#ifdef _DEBUG
		QString data = QString(tr("[RspQryInvestorPosition]  InvestorId: ")) + QString(pInvestorPosition->InvestorID) + tr("  InstrumentId: ") + QString(pInvestorPosition->InstrumentID)
			+ tr("  Direction: ") + QString(pInvestorPosition->PosiDirection) + tr("  Postion: ") + QString::number(pInvestorPosition->Position) + tr("  YPostion: ") +
			QString::number(pInvestorPosition->YdPosition);
		USTPLogger::saveData(data);
#endif
	}
}

void USTPTradeSpi::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if(pTrade != NULL){
		emit onUSTPRspQryTrade(QString(pTrade->TradeID), QString(pTrade->InstrumentID), pTrade->Direction, pTrade->Volume, pTrade->Price,
			pTrade->OffsetFlag, pTrade->HedgeFlag, QString(pTrade->BrokerID), QString(pTrade->ExchangeID), QString(pTrade->InvestorID), QString(pTrade->OrderSysID),
			"", QString(pTrade->OrderLocalID), QString(pTrade->TradeTime));
#ifdef _DEBUG
		QString data = QString(tr("[RspQryTrade]  InvestorId: ")) + QString(pTrade->InvestorID) + tr("  InstrumentId: ") + QString(pTrade->InstrumentID)
			+ tr("  Direction: ") + QString(pTrade->Direction) + tr("  TradeId: ") + QString(pTrade->TradeID) + tr("  Price: ") + QString::number(pTrade->Price) +
			tr("  Volume: ") + QString::number(pTrade->Volume) + tr("  TradeTime: ") + QString(pTrade->TradeTime);
		USTPLogger::saveData(data);
#endif
	}
}

#include "moc_USTPTradeSpi.cpp"