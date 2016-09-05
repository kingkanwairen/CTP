#ifndef USTP_MARKKET_WIDGET_H
#define USTP_MARKKET_WIDGET_H

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
	void doUSTPMdFrontConnected();
	void doSubscribeMd(const QStringList& inss);
	void doUSTPComplexMd(const QString& key, const QString& firstIns, const QString& secIns, const QString& bbPrice, const QString& bsPrice, const QString& sbPrice,
				const QString& ssPrice, const QString& bbQty, const QString& bsQty, const QString& sbQty, const QString& ssQty);
	void doGetComplexInsPrice(const QString& complexIns, const QString& reqKey, const int& direction, const int& rowIndex);
	void doGetComplexMarketPrice(const QString& complexIns, const QString& reqKey, const int& rowIndex);
	void doUpdateDepthMd(const QString& complexIns, const QString& reqKey, const int& rowId, const int& direction, const double& firstPrice, const double& secPrice,
		const double& thirdPrice, const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);
	void doUpdateComplexDepthPrice(const QString& complexIns, const QString& reqKey, const int& rowId, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
		const double& secMarketBidPrice, const double& secMarketAskPrice);

signals:
	void onUpdateComplexInsPrice(const QString& reqKey, const int& rowId, const int& direction, const double& firstPrice, const double& secPrice, const double& thirdPrice,
		const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice);
	void onUpdateMarketInsPrice(const QString& complexIns, const QString& reqKey, const int& rowId, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
		const double& secMarketBidPrice, const double& secMarketAskPrice);
	void onGetDepthMd(const QString& complexIns, const QString& reqKey, const int& direction, const int& rowIndex);
	void onGetComplexDepthPrice(const QString& complexIns, const QString& reqKey, const int& rowIndex);
private:
	void initConnect();
	void addItem(const int& row, const int& colume, const QString& text);
	void updateItemPrice(const int& row, const int& colume, const QString& price);
	void updateItemVolume(const int& row, const int& colume, const QString& volume);
private:
	QStringList mHeaders;
	QMap<QString, USTPComplexMd*> mUSTPMdMap;
	QMap<QString, QString> mInss;
};
#endif