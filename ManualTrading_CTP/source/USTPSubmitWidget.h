#ifndef USTP_SUBMIT_WIDGET_H
#define USTP_SUBMIT_WIDGET_H

#include <QtGui/QWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QPainter>
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QLineEdit>
#include <QtGui/QComboBox>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include "USTPOrderWidget.h"
#include "USTPCancelWidget.h"
#include "USTPMarketWidget.h"
#include "USTPUserStrategy.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QGridLayout;
QT_END_NAMESPACE

class USTPSpeMarketWidget;

class USTPSubmitWidget : public QWidget
{
	Q_OBJECT

public:
	USTPSubmitWidget(const int& bKey, const int& sKey, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, 
		USTPMarketWidget* pMarketWidget, QWidget* parent = 0);

	~USTPSubmitWidget();

signals:

	void onGetSpeInsPrice(const QString& selIns, const int& direction);
	void onSubmitOrder(const QString& orderLabel, const QString& speLabel, const QString& ins, const char& direction, const char& offsetFlag, const int& volume, const double& orderPrice);
	void onSubmitParkedOrder(const QString& orderLabel, const QString& speLabel, const QString& ins, const char& direction, const char& offsetFlag, const int& volume, const double& orderPrice);
	void onCancelParkedOrder();

private:
	QComboBox* createComboBox(const QStringList &itemList);
	QPushButton* createButton(const QString &text, const char *member);

protected slots:
	void createOrder();
	void createParkedOrder();
	void createParkedCancel();
	void doUpdateKey(const int& bidKey, const int& askKey);
	void doSubscribeMd(const QStringList& inss);
	void doClickSelIns(const QString& selIns, const int& direction, const double& showPrice);
	void doUpdateSpePrice(const QString& selIns, const int& direction, const double& orderPrice);
	void doKeyDownHock(const int& key);
	void doEnbedOrder(const QString& selIns, const QString& direction, const QString& offsetFlag, const double& priceTick, const int& orderQty,
		const int& openSuperPrice, const int& stopProfitNum, const int& stopLossNum, const double& bidPrice, const double& askPrice, 
		const double& lastPrice);
	void doOrderFinished(const QString& orderLabel, const QString& instrumentId);
	void doUpdatePosition(const QString& instrumentId, const int& direction, const int& qty);

private:
	void initConnect(USTPMarketWidget* pMarketWidget, QWidget* pWidget);
	int getPrcision(const double& value);

private:
	int mBidKey;
	int mAskKey;
	QLabel* mInsLabel;
	QLabel* mDirectionLabel;
	QLabel* mOffsetFlagLabel;
	QLabel* mVolumeLabel;
	QLabel* mPriceLabel;
	QLineEdit* mInsLineEdit;
	QComboBox* mBSComboBox;
	QComboBox* mOffsetComboBox;
	QDoubleSpinBox* mPriceSpinBox;
	QSpinBox* mVolumeSpinBox;
	QCheckBox* mAutoCheckBox;
	QPushButton* mConfirmBtn;
	QPushButton* mParkedOrderBtn;
	QPushButton* mParkedCancelBtn;
	QGridLayout* mGridLayout;
	USTPOrderWidget* mOrderWidget;
	USTPCancelWidget* mCancelWidget;
	USTPMarketWidget* mMarketWidget;
	QMap<QString, QString> mInss;
	QMap<QString, USTPStrategyBase*> mUserStrategys;
};

#endif
