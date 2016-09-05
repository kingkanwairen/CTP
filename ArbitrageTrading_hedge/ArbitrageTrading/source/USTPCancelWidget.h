#ifndef USTP_CANCEL_WIDGET_H
#define USTP_CANCEL_WIDGET_H

#include <QtGui/QTableWidget>
#include <QtGui/QPushButton>

class USTPCancelWidget : public  QTableWidget
{
	Q_OBJECT
public:

	USTPCancelWidget(QWidget* parent = 0);

	~USTPCancelWidget();

public slots:

	void doUpdateOrderShow(const QString& userLocalId, const QString& instrument, const QString& orderStyle, const char& status, const char& bs, const double& price, const int&
		orderVolume, const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedge, const double& orderPrice);

	void doOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
		const int& actionSuperNum, const double& worstPrice, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
		const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);

	void doUnilateralFinished(const QString& orderLabel, const QString& insName, const char& direction, const char& offsetFlag, 
		const double& orderPrice, const int& qty, const double& insBidPrice, const double insAskPrice, const int& cyclePoint, 
		const int& cycleNum, const int& totalCycleNum, bool isFirstIns, bool isAllTraded);
	
	void doBtnDeleteClicked();

	void doBtnCancelClicked();

	void doKeyDownHock(const int& key);
signals:

	void onDelOrder(const QString& orderStyle);
private:
	void addItem(const int& row, const int& colume, const QString& text);
	QPushButton* createButton(const QString &text, const char *member);
private:
	QString getBSText(const char& bs);
	QString getOffsetFlagText(const char& offsetFlag);
private:
	QStringList mHeaders;
};
#endif