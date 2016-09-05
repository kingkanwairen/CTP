#ifndef USTP_STRATEGY_WIDGET_H
#define USTP_STRATEGY_WIDGET_H

#include <QtGui/QTableWidget>
#include <QtGui/QComboBox>
#include <QtCore/QMap>
#include "USTPUserStrategy.h"
#include "USTPOrderWidget.h"
#include "USTPCancelWidget.h"
#include "USTPMarketWidget.h"
#include "USTPTabWidget.h"

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

class USTPStrategyWidget : public QTableWidget
{
	Q_OBJECT
public:
	USTPStrategyWidget(const int& index, const int& bKey, const int& sKey, USTPMarketWidget* pMarketWidget, USTPOrderWidget* pOrderWidget, 
		USTPCancelWidget* pCancelWidget, USTPTabWidget* pTabWidget, QWidget* parent = 0);

	~USTPStrategyWidget();

signals:

	void onGetComplexInsPrice(const QString& complexIns, const QString& reqKey, const int& direction, const int& rowIndex);
	
	void onGetComplexMarketPrice(const QString& complexIns, const QString& reqKey, const int& rowIndex);

	public slots:
		void doCreateNewOrder(const QString& firstIns, const QString& secIns, const QString& orderStyle, const QString& direction, const QString& offsetFlag, 
			const QString& hedgeFlag, const double& priceTick, const int& orderQty, const int& firstElapseTime, const int& secElapseTime, const int& openFirstPrice,
			const int& openSecondPrice, const int& referTickNum, const int& superPriceStall, const int& cyclePrice, const int& actionReferTickNum, const int& actionSuperSilpNum, 
			const int& mdOrderQty, bool isDefineOrder, bool isOpponentPriceType, bool isCancelFirstOrder, bool isCancelSecOrder, bool isCylce, bool isActionReferTick);

		void doSubmitOrder();

		void doBtnTableDeleteClicked();

		void doOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
			const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
			const int& actionSuperNum, const int& mdOrderQty, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
			const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);

		void doUpdateKey(const int& bidKey, const int& askKey);

		void doCellClicked(int row, int column);

		void doTabIndexChanged(int tabIndex);

		void doUpdateComplexInsPrice(const QString& reqKey, const int& rowId, const int& direction, const double& firstPrice, const double& secPrice, const double& thirdPrice, 
			const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);
		
		void doUpdateMarketInsPrice(const QString& complexIns, const QString& reqKey, const int& rowId, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
			const double& secMarketBidPrice, const double& secMarketAskPrice);

		void doEnbedOrder(const QString& firstIns, const QString& secIns, const QString& orderStyle, const QString& direction, const QString& offsetFlag, 
			const QString& hedgeFlag, const double& priceTick, const int& orderQty, bool isEnbed, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
			const double& secMarketBidPrice, const double& secMarketAskPrice);
protected:

	virtual void keyPressEvent(QKeyEvent* event);

	virtual void focusInEvent( QFocusEvent* event);

	virtual void focusOutEvent( QFocusEvent* event);

	virtual bool eventFilter( QObject * watched, QEvent * event);

public:

	bool loadList(const QString& name);

	QString getList();

private:

	void initConnect(USTPTabWidget* pTabWidget, USTPMarketWidget* pMarketWidget, QWidget* pWidget);

	QPushButton* createButton(const QString &text, const char *member);

	QComboBox* createComboBox(const QStringList &itemList);

	void addItem(const int& row, const int& colume, const QString& text);

	int getPrcision(const double& value);

	void createNewOrder(const int& row, const double& firstBidPrice, const double& firstAskPrice, const double& secBidPrice, const double& secAskPrice);

	void createStrategy(const int& orderType, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
		const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
		const int& actionSuperNum, const int& mdOrderQty, bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isActionRefer,
		const double& firstBidPrice, const double& firstAskPrice, const double& secBidPrice, const double& secAskPrice);


private:
	QMap<QString, USTPStrategyBase*> mUserStrategys;
	USTPOrderWidget* mOrderWidget;
	USTPCancelWidget* mCancelWidget;
	USTPMarketWidget* mMarketWidget;
	int mBidKey;
	int mAskKey;
	int mCurrentIndex;
	QString mMarketKey;
	QString mOrderKey;
	int mCurrentTabIndex;
};

#endif