#include "USTPSubmitWidget.h"
#include <QtGui/QKeyEvent>
#include "USTPCtpLoader.h"
#include "USTPMutexId.h"
#include "USTPConfig.h"
#include "USTPLogger.h"

USTPSubmitWidget::USTPSubmitWidget(const int& bKey, const int& sKey, USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, 
								   USTPMarketWidget* pMarketWidget, QWidget* parent) : QWidget(parent)
{	
	mBidKey = bKey;
	mAskKey = sKey;
	mOrderWidget = pOrderWidget;
	mCancelWidget = pCancelWidget;
	mMarketWidget = pMarketWidget;
	QFont serifFont("Times", 11, QFont::Bold);
	setFont(serifFont);

	mAutoCheckBox = new QCheckBox(tr("快捷键"));
	mAutoCheckBox->setChecked(false);

	mInsLabel = new QLabel(tr("合约代码"));
	mInsLineEdit = new QLineEdit;
	mInsLabel->setBuddy(mInsLineEdit);

	mDirectionLabel = new QLabel(tr("买/卖"));
	QStringList bsList;
	bsList << tr("0-买入") << tr("1-卖出");
	mBSComboBox = createComboBox(bsList);
	mBSComboBox->setItemIcon(0, QIcon("../image/red.png"));
	mBSComboBox->setItemIcon(1, QIcon("../image/green.png"));
	mDirectionLabel->setBuddy(mBSComboBox);

	mOffsetFlagLabel = new QLabel(tr("开/平"));
	QStringList offsetList;
	offsetList << tr("0-开仓") << tr("1-平仓") << tr("2-平今") << tr("3-平昨");
	mOffsetComboBox = createComboBox(offsetList);
	mOffsetComboBox->setItemIcon(0, QIcon("../image/red.png"));
	mOffsetComboBox->setItemIcon(1, QIcon("../image/green.png"));
	mOffsetComboBox->setItemIcon(2, QIcon("../image/green.png"));
	mOffsetComboBox->setItemIcon(3, QIcon("../image/green.png"));
	mOffsetComboBox->setCurrentIndex(2);
	mOffsetFlagLabel->setBuddy(mOffsetComboBox);
	
	mVolumeLabel = new QLabel(tr("委托数量"));
	mVolumeSpinBox = new QSpinBox;
	mVolumeSpinBox->setRange(1, 10000);
	mVolumeSpinBox->setSingleStep(1);
	mVolumeSpinBox->setValue(1);
	mVolumeLabel->setBuddy(mVolumeSpinBox);
	
	mPriceLabel = new QLabel(tr("委托价格"));
	mPriceSpinBox = new QDoubleSpinBox;
	mPriceSpinBox->setRange(0.0, 100000.0);
	mPriceSpinBox->setSingleStep(1.0);
	mPriceSpinBox->setValue(1000.0);
	mPriceLabel->setBuddy(mPriceSpinBox);


	mConfirmBtn = createButton(tr("提 交"), SLOT(createOrder()));
	QLabel* pNullLabel = new QLabel;
	mGridLayout = new QGridLayout;
	mGridLayout->addWidget(mInsLabel, 0, 1, 1, 1);	
	mGridLayout->addWidget(mDirectionLabel, 0, 2, 1, 1);
	mGridLayout->addWidget(mOffsetFlagLabel, 0, 3, 1, 1);
	mGridLayout->addWidget(mVolumeLabel, 0, 4, 1, 1);
	mGridLayout->addWidget(mPriceLabel, 0, 5, 1, 1);
	
	mGridLayout->addWidget(mAutoCheckBox, 1, 0, 1, 1);
	mGridLayout->addWidget(mInsLineEdit, 1, 1, 1, 1);
	mGridLayout->addWidget(mBSComboBox, 1, 2, 1, 1);
	mGridLayout->addWidget(mOffsetComboBox, 1, 3, 1, 1);
	mGridLayout->addWidget(mVolumeSpinBox, 1, 4, 1, 1);
	mGridLayout->addWidget(mPriceSpinBox, 1, 5, 1, 1);
	mGridLayout->addWidget(mConfirmBtn, 1, 6, 1, 1);
	mGridLayout->addWidget(pNullLabel, 2, 0, 4, 7);
	setLayout(mGridLayout);
	initConnect(pMarketWidget, parent);
}

USTPSubmitWidget::~USTPSubmitWidget()
{

}

void USTPSubmitWidget::initConnect(USTPMarketWidget* pMarketWidget, QWidget* pWidget)
{	
	connect(pWidget, SIGNAL(onUpdateKey(const int&, const int&)), this, SLOT(doUpdateKey(const int&, const int&)));
	connect(this, SIGNAL(onGetSpeInsPrice(const QString&, const int&)), pMarketWidget, SLOT(doGetSpeInsPrice(const QString&, const int&)));
	connect(pMarketWidget, SIGNAL(onClickSelIns(const QString&, const int&, const double&)), this, SLOT(doClickSelIns(const QString&, const int&, const double&)));
	connect(pMarketWidget, SIGNAL(onUpdateSpePrice(const QString&, const int&, const double&)), this, SLOT(doUpdateSpePrice(const QString&, const int&, const double&)));
	connect(pMarketWidget, SIGNAL(onEnbedOrder(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const int&, const double&,
		const double&, const double&)), this, SLOT(doEnbedOrder(const QString&, const QString&, const QString&, const double&, const int&, const int&, const int&, const int&,
		const double&, const double&, const double&)));
}

QComboBox* USTPSubmitWidget::createComboBox(const QStringList &itemList)
{
	QComboBox *comboBox = new QComboBox;
	foreach(QString item, itemList){
		comboBox->addItem(item);
	}
	comboBox->setEditable(true);
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	return comboBox;
}

QPushButton *USTPSubmitWidget::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}

void USTPSubmitWidget::createOrder()
{	
	QString insId = mInsLineEdit->text();
	if(insId == "")
		return;
	QString label = tr("[K交易]") + QString::number(USTPMutexId::getNewOrderIndex());
	QStringList bsItems = mBSComboBox->currentText().split("-");
	char direction = (bsItems.at(0) == tr("0")) ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;

	QStringList offsetItems = mOffsetComboBox->currentText().split("-");
	int selOffsetFlag = offsetItems.at(0).toInt();
	char offstFlag;
	switch (selOffsetFlag)
	{
	case 0:
		offstFlag = THOST_FTDC_OF_Open;
		break;
	case 1:
		offstFlag = THOST_FTDC_OF_Close;
		break;
	case 2:
		offstFlag = THOST_FTDC_OF_CloseToday;
		break;
	case 3:
		offstFlag = THOST_FTDC_OF_CloseYesterday;
		break;
	default:
		offstFlag = THOST_FTDC_OF_Open;
		break;
	}
	int qty = mVolumeSpinBox->value();
	double price = mPriceSpinBox->value();
	QString speLabel;
	if(THOST_FTDC_D_Buy == direction)
		speLabel = insId + tr("_B");
	else
		speLabel = insId + tr("_S");
	emit onSubmitOrder(label, speLabel, insId, direction, offstFlag, qty, price);
}

void USTPSubmitWidget::doUpdateKey(const int& bidKey, const int& askKey)
{
	mBidKey = bidKey;
	mAskKey = askKey;
}

void USTPSubmitWidget::doClickSelIns(const	QString& selIns, const int& direction, const double& showPrice)
{	
	mInsLineEdit->setText(selIns);
	double tick = USTPMutexId::getInsPriceTick(selIns);
	mPriceSpinBox->setDecimals(getPrcision(tick));
	mPriceSpinBox->setSingleStep(tick);
	if(direction == -1)
		return;
	mBSComboBox->setCurrentIndex(direction);
	mPriceSpinBox->setValue(showPrice);
}

void USTPSubmitWidget::doUpdateSpePrice(const QString& selIns, const int& direction, const double& orderPrice)
{
	if(mInsLineEdit->text() == selIns){
		double adjustPrice = 0.0;
		double tick = USTPMutexId::getInsPriceTick(selIns);
		if(direction == 0){
			mBSComboBox->setCurrentIndex(0);
			adjustPrice = orderPrice + tick;
		}else if(direction == 1){
			mBSComboBox->setCurrentIndex(1);
			adjustPrice = orderPrice - tick;
		}else if(direction == 2){
			mBSComboBox->setCurrentIndex(0);
			adjustPrice = orderPrice;
		}else if(direction == 3){
			mBSComboBox->setCurrentIndex(1);
			adjustPrice = orderPrice;
		}
		mPriceSpinBox->setValue(adjustPrice);
		createOrder();
	}
}


int USTPSubmitWidget::getPrcision(const double& value)
{
	if (value >= VALUE_1){
		return 0;
	}else if(value * 10 >= VALUE_1){
		return 1;
	}else if(value * 100 >= VALUE_1){
		return 2;
	}else if(value * 1000 >= VALUE_1){
		return 3;
	}
	return 0;
}

void USTPSubmitWidget::doSubscribeMd(const QStringList& inss)
{	
	foreach (USTPStrategyBase* pStrategy, mUserStrategys){
		delete pStrategy;
		pStrategy = NULL;
	}
	mUserStrategys.clear();
	foreach(QString ins, inss){
		QString label_B = ins + tr("_B");
		QString label_S = ins + tr("_S");

		USTPStrategyBase* pStrategy_B = new USTPSpeculateOrder("", label_B, ins, 0.0, 0, THOST_FTDC_D_Buy, THOST_FTDC_OF_Open,
			0, 0, 0, mOrderWidget, mCancelWidget, this);
		USTPStrategyBase* pStrategy_S = new USTPSpeculateOrder("", label_S, ins, 0.0, 0, THOST_FTDC_D_Sell, THOST_FTDC_OF_Open,
			0, 0, 0, mOrderWidget, mCancelWidget, this);
		mUserStrategys.insert(label_B, pStrategy_B);
		mUserStrategys.insert(label_S, pStrategy_S);
		mInss.insert(ins, ins);
	}

}

void USTPSubmitWidget::doKeyDownHock(const int& key)
{

	bool isAutoType = mAutoCheckBox->isChecked();
	if (key == KEY_BID && isAutoType){
		emit onGetSpeInsPrice(mInsLineEdit->text(), 0);
	}else if (key == KEY_ASK && isAutoType){
		emit onGetSpeInsPrice(mInsLineEdit->text(), 1);
	}else if (key == KEY_BID_SUPER && isAutoType){
		emit onGetSpeInsPrice(mInsLineEdit->text(), 2);
	}else if (key == KEY_ASK_SUPER && isAutoType){
		emit onGetSpeInsPrice(mInsLineEdit->text(), 3);
	}
}

void USTPSubmitWidget::doEnbedOrder(const QString& selIns, const QString& direction, const QString& offsetFlag, const double& priceTick, const int& orderQty,
									const int& openSuperPrice, const int& stopProfitNum, const int& stopLossNum, const double& bidPrice, const double& askPrice, const double& lastPrice)
{
	QString label = tr("[K交易]") + QString::number(USTPMutexId::getNewOrderIndex());
	QStringList bsItems = direction.split("-");
	char bs = (bsItems.at(0) == tr("0")) ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;
	QStringList offsetItems = offsetFlag.split("-");
	int selOffsetFlag = offsetItems.at(0).toInt();
	char offstFlag;
	switch (selOffsetFlag)
	{
	case 0:
		offstFlag = THOST_FTDC_OF_Open;
		break;
	case 1:
		offstFlag = THOST_FTDC_OF_Close;
		break;
	case 2:
		offstFlag = THOST_FTDC_OF_CloseToday;
		break;
	case 3:
		offstFlag = THOST_FTDC_OF_CloseYesterday;
		break;
	default:
		offstFlag = THOST_FTDC_OF_Open;
		break;
	}
	USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label, selIns, priceTick, orderQty, bs, offstFlag, openSuperPrice,
		stopProfitNum, stopLossNum, bidPrice, askPrice, lastPrice, mOrderWidget, mCancelWidget, this);
	mUserStrategys.insert(label, pStrategy);
}

void USTPSubmitWidget::doOrderFinished(const QString& orderLabel, const QString& instrumentId)
{
	if(mUserStrategys.find(orderLabel) != mUserStrategys.end()){
		USTPStrategyBase* pStrategy = mUserStrategys[orderLabel];
		delete pStrategy;
		pStrategy = NULL;
		mUserStrategys.remove(orderLabel);
		USTPLogger::saveData(QString(tr("[Del Order]  ")) + orderLabel);
	}
}

void USTPSubmitWidget::doUpdatePosition(const QString& instrumentId, const int& direction, const int& qty)
{
	mInsLineEdit->setText(instrumentId);
	mBSComboBox->setCurrentIndex(direction);
	mVolumeSpinBox->setValue(qty);
}

#include "moc_USTPSubmitWidget.cpp"
