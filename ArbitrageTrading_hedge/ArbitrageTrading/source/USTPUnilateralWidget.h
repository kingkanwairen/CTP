#ifndef USTP_UNILATERAL_WIDGET_H
#define USTP_UNILATERAL_WIDGET_H

#include <QtGui/QTableWidget>
#include <QtGui/QComboBox>
#include <QtCore/QMap>
#include "USTPUserStrategy.h"
#include "USTPOrderWidget.h"
#include "USTPCancelWidget.h"
#include "USTPSpeMarketWidget.h"
#include "USTPTabWidget.h"

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

class USTPUnilateralWidget : public QTableWidget
{
	Q_OBJECT
public:
	USTPUnilateralWidget(USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPSpeMarketWidget* pSpeMarketWidget, QWidget* parent = 0);

	~USTPUnilateralWidget();

signals:
	void onGetUnilateralInsPrice(const QString& insName, const QString& key, const int& indexRow);

	public slots:
		void doCreateUnilateralOrder(const QString& instrumentId, const QString& direction, const QString& offsetFlag, 
			const double& orderPrice, const int& orderQty, const int& cyclePoint, const int& cycleNum);

		void doUnilateralFinished(const QString& orderLabel, const QString& insName, const char& direction, const char& offsetFlag, 
			const double& orderPrice, const int& qty, const double& insBidPrice, const double insAskPrice, const int& cyclePoint, 
			const int& cycleNum, const int& totalCycleNum, bool isFirstIns, bool isAllTraded);

		void doUpdateUnilateralPrice(const QString& selIns, const QString& key, const int& indexRow, const double& bidPrice, const double& askPrice);
		
		void doSubmitOrder();
		
		void doBtnTableDeleteClicked();

private:
	void initConnect(USTPSpeMarketWidget* pSpeMarketWidget, QWidget* pWidget);

public:

	bool loadList(const QString& name);

	QString getList();

private:

	QPushButton* createButton(const QString &text, const char *member);

	QComboBox* createComboBox(const QStringList &itemList);

	void addItem(const int& row, const int& colume, const QString& text);

	int getPrcision(const double& value);

	void createNewOrder(const int& row, const double& insBidPrice, const double insAskPrice);

private:	
	QString mTradeInsKey;
	double mTradeBidPrice;
	double mTradeAskPrice;
	USTPOrderWidget* mOrderWidget;
	USTPCancelWidget* mCancelWidget;
	QMap<QString, USTPStrategyBase*> mUSTPUnilateralStrategyMap;
};

#endif