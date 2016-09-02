#include "USTPStrategyWidget.h"
#include "USTPConfig.h"
#include <QtGui/QSpinBox>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtGui/QKeyEvent>
#include <QtCore/QDebug>
#include "USTPCtpLoader.h"
#include "USTPLogger.h"
#include "USTPMutexId.h"
#include "USTPProfile.h"


USTPStrategyWidget::USTPStrategyWidget(const int& index, const int& bKey, const int& sKey, USTPMarketWidget* pMarketWidget, USTPOrderWidget* pOrderWidget,
									   USTPCancelWidget* pCancelWidget, USTPTabWidget* pTabWidget, QWidget *parent) :
QTableWidget(parent)
{		
	mBidKey = bKey;
	mAskKey = sKey;
	mCurrentIndex = -1;
	mCurrentTabIndex = index;
	mOrderWidget = pOrderWidget;
	mCancelWidget = pCancelWidget;
	setColumnCount(STRATEGYHEAD_LENGTH);
	QStringList headers;
	headers << STRATEGY_WIDGET_TAB_HEAD_0 << STRATEGY_WIDGET_TAB_HEAD_1<< STRATEGY_WIDGET_TAB_HEAD_2 << STRATEGY_WIDGET_TAB_HEAD_3 << STRATEGY_WIDGET_TAB_HEAD_4
		<< STRATEGY_WIDGET_TAB_HEAD_5 << STRATEGY_WIDGET_TAB_HEAD_6 << STRATEGY_WIDGET_TAB_HEAD_7 << STRATEGY_WIDGET_TAB_HEAD_8 << STRATEGY_WIDGET_TAB_HEAD_9
		<< STRATEGY_WIDGET_TAB_HEAD_10 << STRATEGY_WIDGET_TAB_HEAD_11 << STRATEGY_WIDGET_TAB_HEAD_12 << STRATEGY_WIDGET_TAB_HEAD_13 << STRATEGY_WIDGET_TAB_HEAD_14
		<< STRATEGY_WIDGET_TAB_HEAD_15 << STRATEGY_WIDGET_TAB_HEAD_16 << STRATEGY_WIDGET_TAB_HEAD_17 << STRATEGY_WIDGET_TAB_HEAD_18 << STRATEGY_WIDGET_TAB_HEAD_19
		<< STRATEGY_WIDGET_TAB_HEAD_20 << STRATEGY_WIDGET_TAB_HEAD_21 << STRATEGY_WIDGET_TAB_HEAD_22 << STRATEGY_WIDGET_TAB_HEAD_23 << STRATEGY_WIDGET_TAB_HEAD_24
		<< STRATEGY_WIDGET_TAB_HEAD_25;
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setVisible(false);
	horizontalHeader()->setMovable(true);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setEditTriggers(QAbstractItemView::NoEditTriggers); 
	setShowGrid(true);
	initConnect(pTabWidget, pMarketWidget, parent);
}

USTPStrategyWidget::~USTPStrategyWidget()
{	
}

void USTPStrategyWidget::initConnect(USTPTabWidget* pTabWidget, USTPMarketWidget* pMarketWidget, QWidget* pWidget)
{	
	connect(pTabWidget, SIGNAL(onTabIndexChanged(int)), this, SLOT(doTabIndexChanged(int)));
	connect(this, SIGNAL(onGetComplexInsPrice(const QString&, const QString&, const int&, const int&)), pMarketWidget, SLOT(doGetComplexInsPrice(const QString&, const QString&, const int&, const int&)));
	connect(this, SIGNAL(onGetComplexMarketPrice(const QString&, const QString&, const int&)), pMarketWidget, SLOT(doGetComplexMarketPrice(const QString&, const QString&, const int&)));

	connect(pMarketWidget, SIGNAL(onUpdateComplexInsPrice(const QString&,  const int&, const int&, const double&, const double&, const double&, const double&, const double&, const double&, const double&)),
		this, SLOT(doUpdateComplexInsPrice(const QString&, const int&, const int&, const double&, const double&, const double&, const double&, const double&, const double&, const double&)));
	connect(pMarketWidget, SIGNAL(onUpdateMarketInsPrice(const QString&, const QString&, const int&, const double&, const double&, const double&, const double&)),
		this, SLOT(doUpdateMarketInsPrice(const QString&, const QString&, const int&, const double&, const double&, const double&, const double&)));

	connect(pWidget, SIGNAL(onUpdateKey(const int&, const int&)), this, SLOT(doUpdateKey(const int&, const int&)));
	connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(doCellClicked(int, int)));

	connect(pMarketWidget, SIGNAL(onEnbedOrder(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, const int&, bool,
		const double&, const double&, const double&, const double&)), 
		this, SLOT(doEnbedOrder(const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const double&, const int&, bool,
		const double&, const double&, const double&, const double&)));
}

bool USTPStrategyWidget::loadList(const QString& name)
{	
	QStringList orderList = name.split(",");
	for(int nIndex = 0; nIndex < orderList.size(); nIndex++){
		QString eachList = orderList.at(nIndex);
		QStringList cellItems = eachList.split("|");
		if(cellItems.size() < SAVE_ITEM_LENGTH)
			continue;
		doCreateNewOrder(cellItems.at(0), cellItems.at(1), cellItems.at(2), cellItems.at(3), cellItems.at(4),
			cellItems.at(23), cellItems.at(6).toDouble(), cellItems.at(5).toInt(), cellItems.at(7).toInt(), cellItems.at(8).toInt(), cellItems.at(9).toInt(),
			cellItems.at(10).toInt(), cellItems.at(11).toInt(), cellItems.at(12).toInt(), cellItems.at(13).toInt(), cellItems.at(14).toInt(), cellItems.at(15).toInt(), 
			cellItems.at(16).toDouble(), cellItems.at(17).toInt(), cellItems.at(18).toInt(), cellItems.at(19).toInt(), cellItems.at(20).toInt(), cellItems.at(21).toInt(),
			cellItems.at(22).toInt());
	}
	return true;
}

void  USTPStrategyWidget::doCreateNewOrder(const QString& firstIns, const QString& secIns, const QString& orderStyle, const QString& direction, const QString& offsetFlag, 
										   const QString& hedgeFlag, const double& priceTick, const int& orderQty, const int& firstElapseTime, const int& secElapseTime, const int& openFirstPrice,
										   const int& openSecondPrice, const int& referTickNum, const int& superPriceStall, const int& cyclePrice, const int& actionReferTickNum, const int& actionSuperSilpNum, 
										   const double& worstSuperPrice, bool isDefineOrder, bool isOpponentPriceType, bool isCancelFirstOrder, bool isCancelSecOrder, bool isCylce, bool isActionReferTick)
{
	int nRow = rowCount();
	insertRow(nRow);
	addItem(nRow, 0, firstIns);
	addItem(nRow, 1, secIns);

	QStringList list;
	list << tr("0-盯价套利") << tr("1-条件套利") << tr("2-条件对价套利") << tr("3-条件盯价套利") << tr("4-新条件套利");
	QComboBox* priceTypeCombox = createComboBox(list);
	int nType = priceTypeCombox->findText(orderStyle);
	priceTypeCombox->setCurrentIndex(nType);
	setCellWidget(nRow, 2, priceTypeCombox);


	QStringList bsList;
	bsList << tr("0-买入") << tr("1-卖出");
	QComboBox* bsCombox = createComboBox(bsList);
	bsCombox->setItemIcon(0, QIcon("../image/red.png"));
	bsCombox->setItemIcon(1, QIcon("../image/green.png"));
	int nBS = bsCombox->findText(direction);
	bsCombox->setCurrentIndex(nBS);
	setCellWidget(nRow, 3, bsCombox);

	QStringList offsetList;
	offsetList << tr("0-开仓") << tr("1-平仓") << tr("2-平今") << tr("3-平昨");
	QComboBox* offsetCombox = createComboBox(offsetList);
	offsetCombox->setItemIcon(0, QIcon("../image/red.png"));
	offsetCombox->setItemIcon(1, QIcon("../image/green.png"));
	offsetCombox->setItemIcon(2, QIcon("../image/green.png"));
	offsetCombox->setItemIcon(3, QIcon("../image/green.png"));
	int nOffset = offsetCombox->findText(offsetFlag);
	offsetCombox->setCurrentIndex(nOffset);
	setCellWidget(nRow, 4, offsetCombox);

	QSpinBox *qtySpinBox = new QSpinBox();
	qtySpinBox->setRange(1, 1000); 
	qtySpinBox->setSingleStep(1);
	qtySpinBox->setValue(orderQty);
	setCellWidget(nRow, 5, qtySpinBox);

	QDoubleSpinBox *priceSpinBox = new QDoubleSpinBox();
	priceSpinBox->setRange(-10000.0, 10000.0); 
	double tick = USTPMutexId::getInsPriceTick(firstIns);
	int pricePrcision = getPrcision(tick);
	priceSpinBox->setDecimals(pricePrcision);
	priceSpinBox->setSingleStep(tick);
	priceSpinBox->setValue(priceTick);
	setCellWidget(nRow, 6, priceSpinBox);

	QPushButton* pOrderButton = createButton(tr("提 交"), SLOT(doSubmitOrder()));
	setCellWidget(nRow, 7, pOrderButton);

	QSpinBox *referTickSpinBox = new QSpinBox();
	referTickSpinBox->setRange(1, 3); 
	referTickSpinBox->setSingleStep(1);
	referTickSpinBox->setValue(referTickNum);
	setCellWidget(nRow, 8, referTickSpinBox);

	QSpinBox *superPriceSpinBox = new QSpinBox();
	superPriceSpinBox->setRange(-5000, 5000); 
	superPriceSpinBox->setSingleStep(1);
	superPriceSpinBox->setValue(superPriceStall);
	setCellWidget(nRow, 9, superPriceSpinBox);

	QSpinBox *firstTimeSpinBox = new QSpinBox();
	firstTimeSpinBox->setRange(0, 60000); 
	firstTimeSpinBox->setSingleStep(1);
	firstTimeSpinBox->setValue(firstElapseTime);
	setCellWidget(nRow, 10, firstTimeSpinBox);

	QSpinBox *secondTimeSpinBox = new QSpinBox();
	secondTimeSpinBox->setRange(0, 60000); 
	secondTimeSpinBox->setSingleStep(1);
	secondTimeSpinBox->setValue(secElapseTime);
	setCellWidget(nRow, 11, secondTimeSpinBox);

	QSpinBox *oneInsOpenSpinBox = new QSpinBox();
	oneInsOpenSpinBox->setRange(-1000, 1000); 
	oneInsOpenSpinBox->setSingleStep(1);
	oneInsOpenSpinBox->setValue(openFirstPrice);
	setCellWidget(nRow, 12, oneInsOpenSpinBox);

	QSpinBox *secInsOpenSpinBox = new QSpinBox();
	secInsOpenSpinBox->setRange(-1000, 1000); 
	secInsOpenSpinBox->setSingleStep(1);
	secInsOpenSpinBox->setValue(openSecondPrice);
	setCellWidget(nRow, 13, secInsOpenSpinBox);

	QSpinBox *cycleSpinBox = new QSpinBox();
	cycleSpinBox->setRange(1, 1000); 
	cycleSpinBox->setSingleStep(1);
	cycleSpinBox->setValue(cyclePrice);
	setCellWidget(nRow, 14, cycleSpinBox);

	QSpinBox *actionReferNumSpinBox = new QSpinBox();
	actionReferNumSpinBox->setRange(1, 10); 
	actionReferNumSpinBox->setSingleStep(1);
	actionReferNumSpinBox->setValue(actionReferTickNum);
	setCellWidget(nRow, 15, actionReferNumSpinBox);

	QSpinBox *actionSuperSlipSpinBox = new QSpinBox();
	actionSuperSlipSpinBox->setRange(-100, 100); 
	actionSuperSlipSpinBox->setSingleStep(1);
	actionSuperSlipSpinBox->setValue(actionSuperSilpNum);
	setCellWidget(nRow, 16, actionSuperSlipSpinBox);

	QDoubleSpinBox *worstPriceSpinBox = new QDoubleSpinBox();
	worstPriceSpinBox->setRange(0.0, 10000.0); 
	worstPriceSpinBox->setDecimals(pricePrcision);
	worstPriceSpinBox->setSingleStep(tick);
	worstPriceSpinBox->setValue(worstSuperPrice);
	setCellWidget(nRow, 17, worstPriceSpinBox);

	QCheckBox *defineOrderCheckBox = new QCheckBox();
	defineOrderCheckBox->setChecked(isDefineOrder);
	setCellWidget(nRow, 18, defineOrderCheckBox);

	QCheckBox *opponentPriceCheckBox = new QCheckBox();
	opponentPriceCheckBox->setChecked(isOpponentPriceType);
	setCellWidget(nRow, 19, opponentPriceCheckBox);

	QCheckBox *cancelFirstCheckBox = new QCheckBox();
	cancelFirstCheckBox->setChecked(isCancelFirstOrder);
	setCellWidget(nRow, 20, cancelFirstCheckBox);

	QCheckBox *cancelSecCheckBox = new QCheckBox();
	cancelSecCheckBox->setChecked(isCancelSecOrder);
	setCellWidget(nRow, 21, cancelSecCheckBox);

	QCheckBox *cycleCheckBox = new QCheckBox();
	cycleCheckBox->setChecked(isCylce);
	setCellWidget(nRow, 22, cycleCheckBox);

	QCheckBox *actionReferTickCheckBox = new QCheckBox();
	actionReferTickCheckBox->setChecked(isActionReferTick);
	setCellWidget(nRow, 23, actionReferTickCheckBox);

	QStringList hedgeList;
	hedgeList << tr("0-投机") << tr("1-套利");
	QComboBox* hedgeCombox = createComboBox(hedgeList);
	int nHedge = hedgeCombox->findText(hedgeFlag);
	hedgeCombox->setCurrentIndex(nHedge);
	setCellWidget(nRow, 24, hedgeCombox);

	QPushButton* pCancelBtn = createButton(tr("删 除"), SLOT(doBtnTableDeleteClicked()));
	setCellWidget(nRow, 25, pCancelBtn);
}

void USTPStrategyWidget::addItem(const int& row, const int& colume, const QString& text)
{
	QTableWidgetItem* item = new QTableWidgetItem(text);
	setItem(row, colume, item);
}

void USTPStrategyWidget::createNewOrder(const int& row, const double& firstBidPrice, const double& firstAskPrice, const double& secBidPrice, const double& secAskPrice)
{
	QString firstName = item(row, 0)->text();
	QString secName = item(row, 1)->text();

	QComboBox* orderTypeCombox = qobject_cast<QComboBox *>(cellWidget(row, 2));
	QStringList items = orderTypeCombox->currentText().split("-");
	int orderType = items.at(0).toInt();
	QString typeName = items.at(1);

	QComboBox* bsCombox = qobject_cast<QComboBox *>(cellWidget(row, 3));
	QStringList bsItems = bsCombox->currentText().split("-");
	char direction = (bsItems.at(0) == QString(tr("0"))) ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;

	QComboBox* offsetCombox = qobject_cast<QComboBox *>(cellWidget(row, 4));
	QStringList offsetItems = offsetCombox->currentText().split("-");
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

	QSpinBox* qtySpin = qobject_cast<QSpinBox *>(cellWidget(row, 5));
	int qty = qtySpin->value();

	QDoubleSpinBox* priceSpin = qobject_cast<QDoubleSpinBox *>(cellWidget(row, 6));
	double price = priceSpin->value();

	QSpinBox* timeFirstSpin = qobject_cast<QSpinBox *>(cellWidget(row, 10));
	int cancelFirstTime = timeFirstSpin->value();

	QSpinBox* timeSecSpin = qobject_cast<QSpinBox *>(cellWidget(row, 11));
	int cancelSecTime = timeSecSpin->value();

	QSpinBox* oneOpenSpin = qobject_cast<QSpinBox *>(cellWidget(row, 12));
	int oneOpenSlip = oneOpenSpin->value();

	QSpinBox* secOpenSpin = qobject_cast<QSpinBox *>(cellWidget(row, 13));
	int secOpenSlip = secOpenSpin->value();

	QSpinBox* cycleSpin = qobject_cast<QSpinBox *>(cellWidget(row, 14));
	int cyclePriceStall = cycleSpin->value();

	QSpinBox* actionReferTickSpin = qobject_cast<QSpinBox *>(cellWidget(row, 15));
	int actionReferTickNum = actionReferTickSpin->value();

	QSpinBox* actionSuperSlipSpin = qobject_cast<QSpinBox *>(cellWidget(row, 16));
	int actionSuperSlipNum = actionSuperSlipSpin->value();

	QDoubleSpinBox* worstPriceSpin = qobject_cast<QDoubleSpinBox *>(cellWidget(row, 17));
	double worstSuperPrice = worstPriceSpin->value();


	QCheckBox* defineOrderCheck = qobject_cast<QCheckBox *>(cellWidget(row, 18));
	bool isDefineOrder = defineOrderCheck->isChecked();

	QCheckBox* oppentPriceCheck = qobject_cast<QCheckBox *>(cellWidget(row, 19));
	bool isOppnentPrice = oppentPriceCheck->isChecked();

	QCheckBox* cancelFirstCheck = qobject_cast<QCheckBox *>(cellWidget(row, 20));
	bool isCancelFirst = cancelFirstCheck->isChecked();

	QCheckBox* cancelSecondCheck = qobject_cast<QCheckBox *>(cellWidget(row, 21));
	bool isCancelSecond = cancelSecondCheck->isChecked();

	QCheckBox* cycleCheck = qobject_cast<QCheckBox *>(cellWidget(row, 22));
	bool isCycle = cycleCheck->isChecked();

	QCheckBox* actionReferCheck = qobject_cast<QCheckBox *>(cellWidget(row, 23));
	bool isActionRefer = actionReferCheck->isChecked();

	QComboBox* hedgeCombox = qobject_cast<QComboBox *>(cellWidget(row, 24));
	QStringList hedgeItems = hedgeCombox->currentText().split("-");
	char hedgeFlag = (hedgeItems.at(0) == tr("0")) ? THOST_FTDC_HF_Speculation : THOST_FTDC_HF_Arbitrage;
	
	if(USTPMutexId::getUpperPrice(firstName) < VALUE_ACCURACY || USTPMutexId::getLowerPrice(firstName) < VALUE_ACCURACY || 
		USTPMutexId::getUpperPrice(secName) < VALUE_ACCURACY || USTPMutexId::getLowerPrice(secName) < VALUE_ACCURACY)
		return;
	createStrategy(orderType, firstName, secName, price, oneOpenSlip, secOpenSlip, qty, direction, offstFlag, hedgeFlag, 
		cancelFirstTime, cancelSecTime, cyclePriceStall, actionReferTickNum, actionSuperSlipNum, worstSuperPrice, isCancelFirst,
		isCancelSecond, isCycle, isOppnentPrice, isDefineOrder, isActionRefer, firstBidPrice, firstAskPrice, secBidPrice, secAskPrice);
}

void USTPStrategyWidget::doSubmitOrder()
{
	QPushButton * senderObj = qobject_cast<QPushButton *>(sender());
	if(senderObj == 0)
		return;
	QModelIndex index = indexAt(QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));
	int selRow = index.row();

	QString firstName = item(selRow, 0)->text();
	QString secName = item(selRow, 1)->text();
	QString complexIns = firstName + tr("|") + secName;
	mOrderKey = complexIns + tr("_") + QString::number(USTPMutexId::getMarketIndex());
	emit onGetComplexMarketPrice(complexIns, mOrderKey, selRow);
}


void USTPStrategyWidget::createStrategy(const int& orderType, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
										const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
										const int& actionSuperNum, const double& worstSuperPrice, bool isAutoFirstCancel, bool isAutoSecCancel, bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isActionRefer,
										const double& firstBidPrice, const double& firstAskPrice, const double& secBidPrice, const double& secAskPrice)
{
	switch (orderType){
		case 0:
			{
				QString label = tr("[盯价套利]") + QString::number(USTPMutexId::getNewOrderIndex());
				USTPStrategyBase* pStrategy = new USTPStarePriceArbitrage(label, firstIns, secIns, orderPriceTick, secPriceSlipPoint, qty, bs, 
					offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, isAutoFirstCancel, isAutoSecCancel, isCycle, firstBidPrice,
					firstAskPrice, secBidPrice, secAskPrice, mOrderWidget, mCancelWidget, this);
				mUserStrategys.insert(label, pStrategy);
			}
			break;
		case 1:
			{
				QString label = tr("[条件套利]") + QString::number(USTPMutexId::getNewOrderIndex());
				USTPStrategyBase* pStrategy = new USTPConditionArbitrage(label, firstIns, secIns, orderPriceTick, firstPriceSlipPoint, secPriceSlipPoint, qty, bs, 
					offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, actionReferNum, actionSuperNum, isAutoFirstCancel, isAutoSecCancel, isCycle, isOppentPrice,
					isDefineOrder, isActionRefer, firstBidPrice, firstAskPrice, secBidPrice, secAskPrice, mOrderWidget, mCancelWidget, this);
				mUserStrategys.insert(label, pStrategy);
			}
			break;
		case 2:
			{
				QString label = tr("[条件对价套利]") + QString::number(USTPMutexId::getNewOrderIndex());
				USTPStrategyBase* pStrategy = new USTPOpponentArbitrage(label,  firstIns, secIns, orderPriceTick, firstPriceSlipPoint, secPriceSlipPoint, qty, bs, 
					offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, actionReferNum, actionSuperNum, isAutoFirstCancel, isAutoSecCancel, isCycle, isOppentPrice,
					isDefineOrder, isActionRefer, firstBidPrice, firstAskPrice, secBidPrice, secAskPrice, mOrderWidget, mCancelWidget, this);
				mUserStrategys.insert(label, pStrategy);
			}
			break;
		case 3:
			{
				QString label = tr("[条件盯价套利]") + QString::number(USTPMutexId::getNewOrderIndex());
				USTPStrategyBase* pStrategy = new USTPConditionStareArbitrage(label, firstIns, secIns, orderPriceTick, firstPriceSlipPoint, secPriceSlipPoint, qty, bs, 
					offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, actionReferNum, actionSuperNum, isAutoFirstCancel, isAutoSecCancel, isCycle, isOppentPrice,
					isDefineOrder, isActionRefer, firstBidPrice, firstAskPrice, secBidPrice, secAskPrice, mOrderWidget, mCancelWidget, this);
				mUserStrategys.insert(label, pStrategy);
			}
			break;
		case 4:
			{
				QString label = tr("[新条件套利]") + QString::number(USTPMutexId::getNewOrderIndex());
				USTPStrategyBase* pStrategy = new USTNewConditionArbitrage(label,  firstIns, secIns, orderPriceTick, firstPriceSlipPoint, secPriceSlipPoint, qty, bs, 
					offset, hedge, cancelFirstTime, cancelSecTime, cycleStall, actionReferNum, actionSuperNum, isAutoFirstCancel, isAutoSecCancel, isCycle, isOppentPrice,
					isDefineOrder, isActionRefer, firstBidPrice, firstAskPrice, secBidPrice, secAskPrice, mOrderWidget, mCancelWidget, this);
				mUserStrategys.insert(label, pStrategy);
			}
			break;
	}
}

void USTPStrategyWidget::doBtnTableDeleteClicked()
{
	QPushButton * senderObj = qobject_cast<QPushButton *>(sender());
	if(senderObj == 0)
		return;
	QModelIndex index = indexAt(QPoint(senderObj->frameGeometry().x(),senderObj->frameGeometry().y()));
	int row = index.row();
	int nColumeLen = columnCount();
	for(int nIndex = 0; nIndex < nColumeLen; nIndex++){
		QTableWidgetItem* showItem = takeItem(row, nIndex);
		delete showItem;
		showItem = NULL;
	}
	removeRow(row);
}

QPushButton *USTPStrategyWidget::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}


QComboBox* USTPStrategyWidget::createComboBox(const QStringList &itemList)
{
	QComboBox *comboBox = new QComboBox;
	foreach(QString item, itemList){
		comboBox->addItem(item);
	}
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	return comboBox;
}

void USTPStrategyWidget::doOrderFinished(const QString& orderLabel, const QString& firstIns, const QString& secIns, const double& orderPriceTick, const int& firstPriceSlipPoint, const int& secPriceSlipPoint, 
										 const int& qty, const char& bs,  const char& offset, const char& hedge, const int& cancelFirstTime, const int& cancelSecTime, const int& cycleStall, const int& actionReferNum, 
										 const int& actionSuperNum, const double& worstPrice, bool isAutoFirstCancel, bool isAutoSecCancel,bool isCycle, bool isOppentPrice, bool isDefineOrder, bool isAllTraded, bool isReferTick, const int& orderType,
										 const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice)
{
	if(mUserStrategys.find(orderLabel) != mUserStrategys.end()){
		USTPStrategyBase* pStrategy = mUserStrategys[orderLabel];
		delete pStrategy;
		pStrategy = NULL;
		mUserStrategys.remove(orderLabel);
		USTPLogger::saveData(QString(tr("[Del Order]  ")) + orderLabel);
		//////////////////////////////////////////////////////////////////////////
		///创建新的循环策略
		//////////////////////////////////////////////////////////////////////////
		if(isAllTraded && isCycle){
			if((THOST_FTDC_D_Buy == bs) && (THOST_FTDC_OF_Open == offset)){
				double nextOrderPrice = orderPriceTick + cycleStall * USTPMutexId::getInsPriceTick(firstIns);
				QString exh = USTPMutexId::getInsExchangeId(firstIns);
				char nextOffsetFlag = (exh == EXH_SHFE) ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Close;
				createStrategy(orderType, firstIns, secIns, nextOrderPrice, firstPriceSlipPoint, secPriceSlipPoint, qty, THOST_FTDC_D_Sell, nextOffsetFlag, hedge, 
					cancelFirstTime, cancelSecTime, cycleStall, actionReferNum, actionSuperNum, worstPrice, isAutoFirstCancel, isAutoSecCancel, isCycle, isOppentPrice,
					isDefineOrder, isReferTick, firstMarketBidPrice, firstMarketAskPrice, secMarketBidPrice, secMarketAskPrice);
			}else if ((THOST_FTDC_D_Sell == bs) && (THOST_FTDC_OF_Open == offset)){
				double nextOrderPrice = orderPriceTick - cycleStall * USTPMutexId::getInsPriceTick(firstIns);
				QString exh = USTPMutexId::getInsExchangeId(firstIns);
				char nextOffsetFlag = (exh == EXH_SHFE) ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Close;
				createStrategy(orderType, firstIns, secIns, nextOrderPrice, firstPriceSlipPoint, secPriceSlipPoint, qty, THOST_FTDC_D_Buy, nextOffsetFlag, hedge, 
					cancelFirstTime, cancelSecTime, cycleStall, actionReferNum, actionSuperNum, worstPrice, isAutoFirstCancel, isAutoSecCancel, isCycle, isOppentPrice,
					isDefineOrder, isReferTick, firstMarketBidPrice, firstMarketAskPrice, secMarketBidPrice, secMarketAskPrice);
			}else if ((THOST_FTDC_D_Buy == bs) && (THOST_FTDC_OF_Close == offset || THOST_FTDC_OF_CloseToday == offset || THOST_FTDC_OF_CloseYesterday == offset)){
				double nextOrderPrice = orderPriceTick + cycleStall * USTPMutexId::getInsPriceTick(firstIns);
				createStrategy(orderType, firstIns, secIns, nextOrderPrice, firstPriceSlipPoint, secPriceSlipPoint, qty, THOST_FTDC_D_Sell, THOST_FTDC_OF_Open, hedge, 
					cancelFirstTime, cancelSecTime, cycleStall, actionReferNum, actionSuperNum, worstPrice, isAutoFirstCancel, isAutoSecCancel, isCycle, isOppentPrice,
					isDefineOrder, isReferTick, firstMarketBidPrice, firstMarketAskPrice, secMarketBidPrice, secMarketAskPrice);
			}else if ((THOST_FTDC_D_Sell == bs) && (THOST_FTDC_OF_Close == offset || THOST_FTDC_OF_CloseToday == offset || THOST_FTDC_OF_CloseYesterday == offset)){
				double nextOrderPrice = orderPriceTick - cycleStall * USTPMutexId::getInsPriceTick(firstIns);
				createStrategy(orderType, firstIns, secIns, nextOrderPrice, firstPriceSlipPoint, secPriceSlipPoint, qty, THOST_FTDC_D_Buy, THOST_FTDC_OF_Open, hedge, 
					cancelFirstTime, cancelSecTime, cycleStall, actionReferNum, actionSuperNum, worstPrice, isAutoFirstCancel, isAutoSecCancel, isCycle, isOppentPrice,
					isDefineOrder, isReferTick, firstMarketBidPrice, firstMarketAskPrice, secMarketBidPrice, secMarketAskPrice);
			}

		}
	}
}

QString USTPStrategyWidget::getList()
{
	int nRowLen = rowCount();
	QString szSaveData;
	for(int row = 0; row < nRowLen; row++){
		QString firstName = item(row, 0)->text();
		QString secName = item(row, 1)->text();

		QComboBox* orderTypeCombox = qobject_cast<QComboBox *>(cellWidget(row, 2));
		QString orderType = orderTypeCombox->currentText();

		QComboBox* bsCombox = qobject_cast<QComboBox *>(cellWidget(row, 3));
		QString direction = bsCombox->currentText();

		QComboBox* offsetCombox = qobject_cast<QComboBox *>(cellWidget(row, 4));
		QString offstFlag = offsetCombox->currentText();

		QSpinBox* qtySpin = qobject_cast<QSpinBox *>(cellWidget(row, 5));
		int qty = qtySpin->value();

		QDoubleSpinBox* priceSpin = qobject_cast<QDoubleSpinBox *>(cellWidget(row, 6));
		double price = priceSpin->value();

		QSpinBox* referTickSpin = qobject_cast<QSpinBox *>(cellWidget(row, 8));
		int referTickName = referTickSpin->value();

		QSpinBox* superPriceSpin = qobject_cast<QSpinBox *>(cellWidget(row, 9));
		int superPriceNum = superPriceSpin->value();

		QSpinBox* timeFirstSpin = qobject_cast<QSpinBox *>(cellWidget(row, 10));
		int cancelFirstTime = timeFirstSpin->value();

		QSpinBox* timeSecSpin = qobject_cast<QSpinBox *>(cellWidget(row, 11));
		int cancelSecTime = timeSecSpin->value();

		QSpinBox* oneOpenSpin = qobject_cast<QSpinBox *>(cellWidget(row, 12));
		int oneOpenSlip = oneOpenSpin->value();

		QSpinBox* secOpenSpin = qobject_cast<QSpinBox *>(cellWidget(row, 13));
		int secOpenSlip = secOpenSpin->value();

		QSpinBox* cycleSpin = qobject_cast<QSpinBox *>(cellWidget(row, 14));
		int cyclePriceStall = cycleSpin->value();

		QSpinBox* actionReferTickSpin = qobject_cast<QSpinBox *>(cellWidget(row, 15));
		int actionReferNum = actionReferTickSpin->value();

		QSpinBox* actionSuperSpin = qobject_cast<QSpinBox *>(cellWidget(row, 16));
		int actionSuperNum = actionSuperSpin->value();

		QDoubleSpinBox* worstPriceSpin = qobject_cast<QDoubleSpinBox *>(cellWidget(row, 17));
		double worstPrice = worstPriceSpin->value();

		QCheckBox* defineOrderCheck = qobject_cast<QCheckBox *>(cellWidget(row, 18));
		bool isDefineOrder = defineOrderCheck->isChecked();

		QCheckBox* oppentPriceCheck = qobject_cast<QCheckBox *>(cellWidget(row, 19));
		bool isOppnentPrice = oppentPriceCheck->isChecked();

		QCheckBox* cancelFirstCheck = qobject_cast<QCheckBox *>(cellWidget(row, 20));
		bool isCancelFirst = cancelFirstCheck->isChecked();

		QCheckBox* cancelSecondCheck = qobject_cast<QCheckBox *>(cellWidget(row, 21));
		bool isCancelSecond = cancelSecondCheck->isChecked();

		QCheckBox* cycleCheck = qobject_cast<QCheckBox *>(cellWidget(row, 22));
		bool isCycle = cycleCheck->isChecked();

		QCheckBox* actionReferCheck = qobject_cast<QCheckBox *>(cellWidget(row, 23));
		bool isActonRefer = actionReferCheck->isChecked();

		QComboBox* hedgeCombox = qobject_cast<QComboBox *>(cellWidget(row, 24));
		QString hedgeFlag = hedgeCombox->currentText();
		//保存数据到文件
		QString szLine = firstName + tr("|") + secName + tr("|") + orderType + tr("|") + direction + tr("|") + 
			offstFlag + tr("|") + QString::number(qty) + tr("|") + QString::number(price, 'g', 3) + tr("|") + 
			QString::number(cancelFirstTime) + tr("|") + QString::number(cancelSecTime) + tr("|") + QString::number(oneOpenSlip) + tr("|") + 
			QString::number(secOpenSlip) +  tr("|") +  QString::number(referTickName) + tr("|") + QString::number(superPriceNum) + tr("|") + 
			QString::number(cyclePriceStall) + tr("|") + QString::number(actionReferNum) + tr("|") + QString::number(actionSuperNum) + tr("|") + QString::number(worstPrice, 'g', 3) +
			tr("|") + QString::number(isDefineOrder ? 1 : 0) + tr("|") + QString::number(isOppnentPrice ? 1 : 0) + tr("|") + QString::number(isCancelFirst ? 1 : 0) +
			tr("|") + QString::number(isCancelSecond ? 1 : 0) + tr("|") + QString::number(isCycle ? 1 : 0) + tr("|") + QString::number(isActonRefer ? 1 : 0) + 
			tr("|") + hedgeFlag;
		if(row == 0)
			szSaveData = szLine;
		else
			szSaveData = szSaveData + QString(tr(",")) + szLine;
	}
	return szSaveData;
}

int USTPStrategyWidget::getPrcision(const double& value)
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

void USTPStrategyWidget::doCellClicked(int row, int column)
{
	mCurrentIndex = row;
}

void USTPStrategyWidget::keyPressEvent(QKeyEvent* event)
{	
	if(mCurrentIndex < 0)
		return;
	if (event->key() == mBidKey){
		QString firstName = item(mCurrentIndex, 0)->text();
		QString secName = item(mCurrentIndex, 1)->text();
		QString complexIns = firstName + tr("|") + secName;
		mMarketKey = complexIns + tr("_") + QString::number(USTPMutexId::getMarketIndex());
		emit onGetComplexInsPrice(complexIns, mMarketKey, 0, mCurrentIndex);
	}else if (event->key() == mAskKey){
		QString firstName = item(mCurrentIndex, 0)->text();
		QString secName = item(mCurrentIndex, 1)->text();
		QString complexIns = firstName + tr("|") + secName;
		mMarketKey = complexIns + tr("_") + QString::number(USTPMutexId::getMarketIndex());
		emit onGetComplexInsPrice(complexIns, mMarketKey, 1, mCurrentIndex);
	}
}

void USTPStrategyWidget::focusInEvent(QFocusEvent* event)
{

}

void USTPStrategyWidget::focusOutEvent(QFocusEvent* event)
{

}

bool USTPStrategyWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == mCancelWidget && event->type() == QEvent::KeyPress){
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == mBidKey){
			if(mCurrentIndex < 0)
				return false;
			QString firstName = item(mCurrentIndex, 0)->text();
			QString secName = item(mCurrentIndex, 1)->text();
			QString complexIns = firstName + tr("|") + secName;
			mMarketKey = complexIns + tr("_") + QString::number(USTPMutexId::getMarketIndex());
			emit onGetComplexInsPrice(complexIns, mMarketKey, 0, mCurrentIndex);
		}else if (keyEvent->key() == mAskKey){
			if(mCurrentIndex < 0)
				return false;
			QString firstName = item(mCurrentIndex, 0)->text();
			QString secName = item(mCurrentIndex, 1)->text();
			QString complexIns = firstName + tr("|") + secName;
			mMarketKey = complexIns + tr("_") + QString::number(USTPMutexId::getMarketIndex());
			emit onGetComplexInsPrice(complexIns, mMarketKey, 1, mCurrentIndex);
		}
		return true;
	}
	return QTableWidget::eventFilter(object,event);
}

void USTPStrategyWidget::doUpdateKey(const int& bidKey, const int& askKey)
{
	mBidKey = bidKey;
	mAskKey = askKey;
}

void USTPStrategyWidget::doTabIndexChanged(int tabIndex)
{	
	if(mCurrentTabIndex == tabIndex){
		USTPMutexId::setCurrentTabIndex(tabIndex);
		mCancelWidget->installEventFilter(this);
	}else{
		mCancelWidget->removeEventFilter(this);
	}
}

void USTPStrategyWidget::doUpdateComplexInsPrice(const QString& reqKey, const int& rowId, const int& direction, const double& firstPrice, const double& secPrice, const double& thirdPrice, 
												 const double& firstMarketBidPrice, const double& firstMarketAskPrice, const double& secMarketBidPrice, const double& secMarketAskPrice)
{
	if(mMarketKey == reqKey){
		QSpinBox* referTickSpin = qobject_cast<QSpinBox *>(cellWidget(rowId, 8));
		int referTickName = referTickSpin->value();
		QSpinBox* superPriceSpin = qobject_cast<QSpinBox *>(cellWidget(rowId, 9));
		int superPriceNum = superPriceSpin->value();
		QString firstName = item(rowId, 0)->text();
		double priceTick = USTPMutexId::getInsPriceTick(firstName);

		QComboBox* bsCombox = qobject_cast<QComboBox *>(cellWidget(rowId, 3));	
		QDoubleSpinBox* priceSpin = qobject_cast<QDoubleSpinBox *>(cellWidget(rowId, 6));
		if(direction == 0){
			double bidBasePrice = 0.0;
			switch (referTickName){
			case 1:
				bidBasePrice = firstPrice;
				break;
			case 2:
				bidBasePrice = firstPrice > secPrice ? secPrice : firstPrice;
				break;
			case 3:
				float tempPrice = firstPrice > secPrice ? secPrice : firstPrice;
				bidBasePrice = tempPrice > thirdPrice ? thirdPrice : tempPrice;
				break;
			}
			double bidPrice = bidBasePrice + superPriceNum * priceTick;
			bsCombox->setCurrentIndex(0);
			priceSpin->setValue(bidPrice);

		}else if(direction == 1){
			double askBasePrice = 0.0;
			switch (referTickName){
			case 1:
				askBasePrice = firstPrice;
				break;
			case 2:
				askBasePrice = firstPrice > secPrice ? firstPrice : secPrice;
				break;
			case 3:
				float tempPrice = firstPrice > secPrice ? firstPrice : secPrice;
				askBasePrice = tempPrice > thirdPrice ? tempPrice : thirdPrice;
				break;
			}
			double askPrice = askBasePrice - superPriceNum * priceTick;
			bsCombox->setCurrentIndex(1);
			priceSpin->setValue(askPrice);
		}
		createNewOrder(rowId, firstMarketBidPrice, firstMarketAskPrice, secMarketBidPrice, secMarketAskPrice);
	}
}

void USTPStrategyWidget::doUpdateMarketInsPrice(const QString& complexIns, const QString& reqKey, const int& rowId, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
							const double& secMarketBidPrice, const double& secMarketAskPrice)
{
	if(mOrderKey == reqKey)
		createNewOrder(rowId, firstMarketBidPrice, firstMarketAskPrice, secMarketBidPrice, secMarketAskPrice);

}

void USTPStrategyWidget::doEnbedOrder(const QString& firstIns, const QString& secIns, const QString& orderStyle, const QString& direction, const QString& offsetFlag, 
									  const QString& hedgeFlag, const double& priceTick, const int& orderQty, bool isEnbed, const double& firstMarketBidPrice, const double& firstMarketAskPrice,
									  const double& secMarketBidPrice, const double& secMarketAskPrice)
{	
	if(USTPMutexId::getCurrentTabIndex() != mCurrentTabIndex)
		return;
		
	if(isEnbed){
		doCreateNewOrder(firstIns, secIns, orderStyle, direction, offsetFlag, hedgeFlag, priceTick, orderQty, 0, CANCEL_DELAY_TIME, SUPER_PRICE_STALL,
			SUPER_PRICE_STALL, REFER_PRICE_NUM, 0, 0, REFER_PRICE_NUM, 0, 0.0, false, false, false, true, false, true);
	}else{
		if(USTPMutexId::getUpperPrice(firstIns) < VALUE_ACCURACY || USTPMutexId::getLowerPrice(firstIns) < VALUE_ACCURACY || 
			USTPMutexId::getUpperPrice(secIns) < VALUE_ACCURACY || USTPMutexId::getLowerPrice(secIns) < VALUE_ACCURACY)
			return;
		QStringList items = orderStyle.split("-");
		int orderType = items.at(0).toInt();

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

		QStringList hedgeItems = hedgeFlag.split("-");
		char hFlag = (hedgeItems.at(0) == tr("0")) ? THOST_FTDC_HF_Speculation : THOST_FTDC_HF_Arbitrage;

		createStrategy(orderType, firstIns, secIns, priceTick, SUPER_PRICE_STALL, SUPER_PRICE_STALL, orderQty, bs, offstFlag, hFlag, 
			0, CANCEL_DELAY_TIME, 0, REFER_PRICE_NUM, 0, 0.0, false, true, false, false, false, true, firstMarketBidPrice, firstMarketAskPrice, 
			secMarketBidPrice, secMarketAskPrice);
	}
}

#include "moc_USTPStrategyWidget.cpp"