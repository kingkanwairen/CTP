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
#include "USTPSpeMarketWidget.h"
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
		USTPSpeMarketWidget* pSpeMarketWidget, QWidget* parent = 0);

	~USTPSubmitWidget();

signals:

	void onGetSpeInsPrice(const QString& selIns, const int& direction);
	void onSubmitOrder(const QString& orderLabel, const QString& speLabel, const QString& ins, const char& direction, const char& offsetFlag, const int& volume, const double& orderPrice);

private:
	QComboBox* createComboBox(const QStringList &itemList);
	QPushButton* createButton(const QString &text, const char *member);

protected slots:
	void createOrder();
	void doUpdateKey(const int& bidKey, const int& askKey);
	void doSubscribeMd(const QStringList& inss);
	void doClickSelIns(const QString& selIns, const int& direction, const double& showPrice);
	void doUpdateSpePrice(const QString& selIns, const int& direction, const double& orderPrice);
	void doKeyDownHock(const int& key);

private:
	void initConnect(USTPSpeMarketWidget* pSpeMarketWidget, QWidget* pWidget);
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
	QGridLayout* mGridLayout;
	USTPOrderWidget* mOrderWidget;
	USTPCancelWidget* mCancelWidget;
	USTPSpeMarketWidget* mSpeMarketWidget;
	QMap<QString, QString> mInss;
	QMap<QString, USTPStrategyBase*> mUSTPSpeStrategyMap;
};

#endif
