#ifndef USTP_MARKET_WIDGET_H
#define USTP_MARKET_WIDGET_H

#include <QtGui/QTableWidget>
#include <QtCore/QMap>
#include "USTPComplexMd.h"


class USTPMarketWidget : public  QTableWidget
{
	Q_OBJECT

public:
	USTPMarketWidget(QWidget* parent = 0);
	~USTPMarketWidget();
public slots:
	void doUSTPSpeComplexMd(const QString& selIns, const QString& bidPrice, const QString& bidVolume, const QString& askPrice, const QString& askVolume,
		const QString& tradeVolume, const QString& lastPrice, const QString& upDownPrice, const QString& preClosePrice, const QString& openPrice, const QString& holdQty);
	void doSubscribeMd(const QStringList& inss);
	void doGetSpeInsPrice(const QString& selIns, const int& direction);
	void doUpdateSpeDepthMd(const QString& selIns, const int& direction, const double& orderPrice);
	void doSelectItem(QTableWidgetItem* it);
	void doCellDoubleClicked(int row, int column);
	void doCreateEnbedOrder(const QString& selIns, const QString& direction, const QString& offsetFlag, const double& priceTick, const int& orderQty,
		const int& openSuperPrice, const int& stopProfitNum, const int& stopLossNum, const int& row);
signals:
	void onGetSpeDepthMd(const QString& selIns, const int& direction);
	void onUpdateSpePrice(const QString& selIns, const int& direction, const double& orderPrice);
	void onClickSelIns(const QString& selIns, const int& direction, const double& showPrice);
	void onEnbedOrder(const QString& selIns, const QString& direction, const QString& offsetFlag, const double& priceTick, const int& orderQty,
		const int& openSuperPrice, const int& stopProfitNum, const int& stopLossNum, const double& bidPrice, const double& askPrice, 
		const double& lastPrice);
private:
	void initConnect();
	void addItem(const int& row, const int& colume, const QString& text);
	void updateItemPrice(const int& row, const int& colume, const QString& price);
	void updateItemVolume(const int& row, const int& colume, const QString& volume);
private:
	QStringList mHeaders;
	QMap<QString, USTPComplexMd*> mUSTPSpeMdMap;
	QMap<int, double> mPriceMap;
};
#endif