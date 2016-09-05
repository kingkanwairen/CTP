#include "USTPUnilateralWidget.h"
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


USTPUnilateralWidget::USTPUnilateralWidget(USTPOrderWidget* pOrderWidget, USTPCancelWidget* pCancelWidget, USTPSpeMarketWidget* pSpeMarketWidget, QWidget *parent) :
QTableWidget(parent)
{	
	mTradeBidPrice = 0.0;
	mTradeAskPrice = 0.0;
	mOrderWidget = pOrderWidget;
	mCancelWidget = pCancelWidget;
	setColumnCount(UNILATERAL_HEAD_LENGTH);
	QStringList headers;
	headers << UNILATERAL_WIDGET_TAB_HEAD_0 << UNILATERAL_WIDGET_TAB_HEAD_1<< UNILATERAL_WIDGET_TAB_HEAD_2 << UNILATERAL_WIDGET_TAB_HEAD_3<< UNILATERAL_WIDGET_TAB_HEAD_4
		<< UNILATERAL_WIDGET_TAB_HEAD_5 << UNILATERAL_WIDGET_TAB_HEAD_6 << UNILATERAL_WIDGET_TAB_HEAD_7 << UNILATERAL_WIDGET_TAB_HEAD_8;
	setHorizontalHeaderLabels(headers);
	verticalHeader()->setVisible(false);
	horizontalHeader()->setMovable(true);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setEditTriggers(QAbstractItemView::NoEditTriggers); 
	setShowGrid(true);
	initConnect(pSpeMarketWidget, this);
	setMinimumWidth(MARKET_WINDOW_MIN_WIDTH);
}

USTPUnilateralWidget::~USTPUnilateralWidget()
{	
}

void USTPUnilateralWidget::initConnect(USTPSpeMarketWidget* pSpeMarketWidget, QWidget* pWidget)
{	
	connect(this, SIGNAL(onGetUnilateralInsPrice(const QString&, const QString&, const int&)), pSpeMarketWidget, SLOT(doGetUnilateralInsPrice(const QString&, const QString&, const int&)));
	connect(pSpeMarketWidget, SIGNAL(onUpdateUnilateralPrice(const QString&, const QString&, const int&, const double&, const double&)), this, SLOT(doUpdateUnilateralPrice(const QString&,
		const QString&, const int&, const double&, const double&)));
}

bool USTPUnilateralWidget::loadList(const QString& name)
{	
	QStringList orderList = name.split(",");
	for(int nIndex = 0; nIndex < orderList.size(); nIndex++){
		QString eachList = orderList.at(nIndex);
		QStringList cellItems = eachList.split("|");
		if(cellItems.size() < SAVE_UNILATERAL_LENGTH)
			continue;
		doCreateUnilateralOrder(cellItems.at(0), cellItems.at(1), cellItems.at(2), cellItems.at(3).toDouble(), cellItems.at(4).toInt(),
			cellItems.at(5).toInt(), cellItems.at(6).toInt());
	}
	return true;
}

void  USTPUnilateralWidget::doCreateUnilateralOrder(const QString& instrumentId, const QString& direction, const QString& offsetFlag, 
													const double& orderPrice, const int& orderQty, const int& cyclePoint, const int& cycleNum)
{
	int nRow = rowCount();
	insertRow(nRow);
	addItem(nRow, 0, instrumentId);

	QStringList bsList;
	bsList << tr("0-买入") << tr("1-卖出");
	QComboBox* bsCombox = createComboBox(bsList);
	bsCombox->setItemIcon(0, QIcon("../image/red.png"));
	bsCombox->setItemIcon(1, QIcon("../image/green.png"));
	int nBS = bsCombox->findText(direction);
	bsCombox->setCurrentIndex(nBS);
	setCellWidget(nRow, 1, bsCombox);

	QStringList offsetList;
	offsetList << tr("0-开仓") << tr("1-平仓") << tr("2-平今") << tr("3-平昨");
	QComboBox* offsetCombox = createComboBox(offsetList);
	offsetCombox->setItemIcon(0, QIcon("../image/red.png"));
	offsetCombox->setItemIcon(1, QIcon("../image/green.png"));
	offsetCombox->setItemIcon(2, QIcon("../image/green.png"));
	offsetCombox->setItemIcon(3, QIcon("../image/green.png"));
	int nOffset = offsetCombox->findText(offsetFlag);
	offsetCombox->setCurrentIndex(nOffset);
	setCellWidget(nRow, 2, offsetCombox);

	QDoubleSpinBox *orderPriceSpinBox = new QDoubleSpinBox();
	orderPriceSpinBox->setRange(-999999.0, 999999.0); 
	double tick = USTPMutexId::getInsPriceTick(instrumentId);
	orderPriceSpinBox->setDecimals(getPrcision(tick));
	orderPriceSpinBox->setSingleStep(tick);
	orderPriceSpinBox->setValue(orderPrice);
	setCellWidget(nRow, 3, orderPriceSpinBox);

	QSpinBox *qtySpinBox = new QSpinBox();
	qtySpinBox->setRange(1, 1000); 
	qtySpinBox->setSingleStep(1);
	qtySpinBox->setValue(orderQty);
	setCellWidget(nRow, 4, qtySpinBox);

	QSpinBox *cyclePointSpinBox = new QSpinBox();
	cyclePointSpinBox->setRange(0, 99999); 
	cyclePointSpinBox->setSingleStep(1);
	cyclePointSpinBox->setValue(cyclePoint);
	setCellWidget(nRow, 5, cyclePointSpinBox);

	QSpinBox *numSpinBox = new QSpinBox();
	numSpinBox->setRange(0, 99999); 
	numSpinBox->setSingleStep(1);
	numSpinBox->setValue(cycleNum);
	setCellWidget(nRow, 6, numSpinBox);

	QPushButton* pOrderButton = createButton(tr("提 交"), SLOT(doSubmitOrder()));
	setCellWidget(nRow, 7, pOrderButton);

	QPushButton* pCancelBtn = createButton(tr("删 除"), SLOT(doBtnTableDeleteClicked()));
	setCellWidget(nRow, 8, pCancelBtn);
}

void USTPUnilateralWidget::addItem(const int& row, const int& colume, const QString& text)
{
	QTableWidgetItem* item = new QTableWidgetItem(text);
	setItem(row, colume, item);
}

void USTPUnilateralWidget::createNewOrder(const int& row, const double& insBidPrice, const double insAskPrice)
{
	QString insName = item(row, 0)->text();

	QComboBox* bsCombox = qobject_cast<QComboBox *>(cellWidget(row, 1));
	QStringList bsItems = bsCombox->currentText().split("-");
	char direction = (bsItems.at(0) == tr("0")) ? THOST_FTDC_D_Buy : THOST_FTDC_D_Sell;

	QComboBox* offsetCombox = qobject_cast<QComboBox *>(cellWidget(row, 2));
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

	QDoubleSpinBox* orderPriceSpin = qobject_cast<QDoubleSpinBox *>(cellWidget(row, 3));
	double orderPrice = orderPriceSpin->value();

	QSpinBox* orderQtySpin = qobject_cast<QSpinBox *>(cellWidget(row, 4));
	int qty = orderQtySpin->value();

	QSpinBox* cyclePointSpin = qobject_cast<QSpinBox *>(cellWidget(row, 5));
	int cyclePoint = cyclePointSpin->value();

	QSpinBox* cycleNumSpin = qobject_cast<QSpinBox *>(cellWidget(row, 6));
	int cycleNum = cycleNumSpin->value();

	QString label = tr("[单边循环]") + QString::number(USTPMutexId::getNewOrderIndex());
	USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label, insName, direction, offstFlag, orderPrice,
		qty, cyclePoint, 0, cycleNum, insBidPrice, insAskPrice, true, mOrderWidget, mCancelWidget, this);
	mUSTPUnilateralStrategyMap.insert(label, pStrategy);
}

void USTPUnilateralWidget::doSubmitOrder()
{
	QPushButton * senderObj = qobject_cast<QPushButton *>(sender());
	if(senderObj == 0)
		return;
	QModelIndex index = indexAt(QPoint(senderObj->frameGeometry().x(), senderObj->frameGeometry().y()));
	int selRow = index.row();
	mTradeBidPrice = 0.0;
	mTradeAskPrice = 0.0;
	QString insName = item(selRow, 0)->text();
	mTradeInsKey = insName + tr("_") + QString::number(USTPMutexId::getMarketIndex());
	emit onGetUnilateralInsPrice(insName, mTradeInsKey, selRow);
	
}

void USTPUnilateralWidget::doBtnTableDeleteClicked()
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

QPushButton *USTPUnilateralWidget::createButton(const QString &text, const char *member)
{
	QPushButton *button = new QPushButton(text);
	connect(button, SIGNAL(clicked()), this, member);
	return button;
}


QComboBox* USTPUnilateralWidget::createComboBox(const QStringList &itemList)
{
	QComboBox *comboBox = new QComboBox;
	foreach(QString item, itemList){
		comboBox->addItem(item);
	}
	comboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	return comboBox;
}

void USTPUnilateralWidget::doUnilateralFinished(const QString& orderLabel, const QString& insName, const char& direction, const char& offsetFlag, 
												const double& orderPrice, const int& qty, const double& insBidPrice, const double insAskPrice, const int& cyclePoint, 
												const int& cycleNum, const int& totalCycleNum, bool isFirstIns, bool isAllTraded)
{
	if(mUSTPUnilateralStrategyMap.find(orderLabel) != mUSTPUnilateralStrategyMap.end()){
		USTPStrategyBase* pStrategy = mUSTPUnilateralStrategyMap[orderLabel];
		delete pStrategy;
		pStrategy = NULL;
		mUSTPUnilateralStrategyMap.remove(orderLabel);
		USTPLogger::saveData(QString(tr("[Del Order]  ")) + orderLabel);

		//////////////////////////////////////////////////////////////////////////
		///创建新的循环策略
		//////////////////////////////////////////////////////////////////////////
		if(isAllTraded){
			if(isFirstIns){
				QString label = tr("[单边循环]") + QString::number(USTPMutexId::getNewOrderIndex());
				if((THOST_FTDC_D_Buy == direction) && (THOST_FTDC_OF_Open == offsetFlag)){
					double nextOrderPrice = orderPrice + cyclePoint * USTPMutexId::getInsPriceTick(insName);
					QString exh = USTPMutexId::getInsExchangeId(insName);
					char nextOffsetFlag = (exh == EXH_SHFE) ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Close;
					USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label,  insName, THOST_FTDC_D_Sell, nextOffsetFlag, nextOrderPrice,
						qty, cyclePoint, cycleNum, totalCycleNum, insBidPrice, insAskPrice, false, mOrderWidget, mCancelWidget, this);
					mUSTPUnilateralStrategyMap.insert(label, pStrategy);
				}else if ((THOST_FTDC_D_Sell == direction) && (THOST_FTDC_OF_Open == offsetFlag)){
					double nextOrderPrice = orderPrice - cyclePoint * USTPMutexId::getInsPriceTick(insName);
					QString exh = USTPMutexId::getInsExchangeId(insName);
					char nextOffsetFlag = (exh == EXH_SHFE) ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Close;
					USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label,  insName, THOST_FTDC_D_Buy, nextOffsetFlag, nextOrderPrice,
						qty, cyclePoint, cycleNum, totalCycleNum, insBidPrice, insAskPrice, false, mOrderWidget, mCancelWidget, this);
					mUSTPUnilateralStrategyMap.insert(label, pStrategy);
				}else if ((THOST_FTDC_D_Buy == direction) && (THOST_FTDC_OF_Close == offsetFlag || THOST_FTDC_OF_CloseToday == offsetFlag || THOST_FTDC_OF_CloseYesterday == offsetFlag)){
					double nextOrderPrice = orderPrice + cyclePoint * USTPMutexId::getInsPriceTick(insName);
					USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label,  insName, THOST_FTDC_D_Sell, THOST_FTDC_OF_Open, nextOrderPrice,
						qty, cyclePoint, cycleNum, totalCycleNum, insBidPrice, insAskPrice, false, mOrderWidget, mCancelWidget, this);
					mUSTPUnilateralStrategyMap.insert(label, pStrategy);
				}else if ((THOST_FTDC_D_Sell == direction) && (THOST_FTDC_OF_Close == offsetFlag || THOST_FTDC_OF_CloseToday == offsetFlag || THOST_FTDC_OF_CloseYesterday == offsetFlag)){
					double nextOrderPrice = orderPrice - cyclePoint * USTPMutexId::getInsPriceTick(insName);
					USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label,  insName, THOST_FTDC_D_Buy, THOST_FTDC_OF_Open, nextOrderPrice,
						qty, cyclePoint, cycleNum, totalCycleNum, insBidPrice, insAskPrice, false, mOrderWidget, mCancelWidget, this);
					mUSTPUnilateralStrategyMap.insert(label, pStrategy);
				}
			}else{
				int nCycleNum = cycleNum;
				nCycleNum++;
				if(nCycleNum <= totalCycleNum){
					QString label = tr("[单边循环]") + QString::number(USTPMutexId::getNewOrderIndex());
					if((THOST_FTDC_D_Buy == direction) && (THOST_FTDC_OF_Open == offsetFlag)){
						double nextOrderPrice = orderPrice + cyclePoint * USTPMutexId::getInsPriceTick(insName);
						QString exh = USTPMutexId::getInsExchangeId(insName);
						char nextOffsetFlag = (exh == EXH_SHFE) ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Close;
						USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label,  insName, THOST_FTDC_D_Sell, nextOffsetFlag, nextOrderPrice,
							qty, cyclePoint, nCycleNum, totalCycleNum, insBidPrice, insAskPrice, true, mOrderWidget, mCancelWidget, this);
						mUSTPUnilateralStrategyMap.insert(label, pStrategy);
					}else if ((THOST_FTDC_D_Sell == direction) && (THOST_FTDC_OF_Open == offsetFlag)){
						double nextOrderPrice = orderPrice - cyclePoint * USTPMutexId::getInsPriceTick(insName);
						QString exh = USTPMutexId::getInsExchangeId(insName);
						char nextOffsetFlag = (exh == EXH_SHFE) ? THOST_FTDC_OF_CloseToday : THOST_FTDC_OF_Close;
						USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label,  insName, THOST_FTDC_D_Buy, nextOffsetFlag, nextOrderPrice,
							qty, cyclePoint, nCycleNum, totalCycleNum, insBidPrice, insAskPrice, true, mOrderWidget, mCancelWidget, this);
						mUSTPUnilateralStrategyMap.insert(label, pStrategy);
					}else if ((THOST_FTDC_D_Buy == direction) && (THOST_FTDC_OF_Close == offsetFlag || THOST_FTDC_OF_CloseToday == offsetFlag || THOST_FTDC_OF_CloseYesterday == offsetFlag)){
						double nextOrderPrice = orderPrice + cyclePoint * USTPMutexId::getInsPriceTick(insName);
						USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label,  insName, THOST_FTDC_D_Sell, THOST_FTDC_OF_Open, nextOrderPrice,
							qty, cyclePoint, nCycleNum, totalCycleNum, insBidPrice, insAskPrice, true, mOrderWidget, mCancelWidget, this);
						mUSTPUnilateralStrategyMap.insert(label, pStrategy);
					}else if ((THOST_FTDC_D_Sell == direction) && (THOST_FTDC_OF_Close == offsetFlag || THOST_FTDC_OF_CloseToday == offsetFlag || THOST_FTDC_OF_CloseYesterday == offsetFlag)){
						double nextOrderPrice = orderPrice - cyclePoint * USTPMutexId::getInsPriceTick(insName);
						USTPStrategyBase* pStrategy = new USTPUnilateralOrder(label,  insName, THOST_FTDC_D_Buy, THOST_FTDC_OF_Open, nextOrderPrice,
							qty, cyclePoint, nCycleNum, totalCycleNum, insBidPrice, insAskPrice, true, mOrderWidget, mCancelWidget, this);
						mUSTPUnilateralStrategyMap.insert(label, pStrategy);
					}
				}
			}
		}
	}
}

void USTPUnilateralWidget::doUpdateUnilateralPrice(const QString& selIns, const QString& key, const int& indexRow, const double& bidPrice, const double& askPrice)
{
	if(key == mTradeInsKey){
		mTradeBidPrice = bidPrice;
		mTradeAskPrice = askPrice;
		if(mTradeAskPrice > 0.0 && mTradeBidPrice > 0.0)
			createNewOrder(indexRow, mTradeBidPrice, mTradeAskPrice);
	}
}

QString USTPUnilateralWidget::getList()
{
	int nRowLen = rowCount();
	QString szSaveData;
	for(int row = 0; row < nRowLen; row++){
		QString insName = item(row, 0)->text();

		QComboBox* bsCombox = qobject_cast<QComboBox *>(cellWidget(row, 1));
		QString direction = bsCombox->currentText();

		QComboBox* offsetCombox = qobject_cast<QComboBox *>(cellWidget(row, 2));
		QString offstFlag = offsetCombox->currentText();

		QDoubleSpinBox* orderPriceSpin = qobject_cast<QDoubleSpinBox *>(cellWidget(row, 3));
		double orderPrice = orderPriceSpin->value();

		QSpinBox* qtySpin = qobject_cast<QSpinBox *>(cellWidget(row, 4));
		int qty = qtySpin->value();

		QSpinBox* cyclePointSpin = qobject_cast<QSpinBox *>(cellWidget(row, 5));
		int cyclePoint = cyclePointSpin->value();

		QSpinBox* cycleNumSpin = qobject_cast<QSpinBox *>(cellWidget(row, 6));
		int cycleNum = cycleNumSpin->value();
		//保存数据到文件
		QString szLine = insName + tr("|") + direction + tr("|") + offstFlag + tr("|") + QString::number(orderPrice, 'g', 3) + tr("|") + 
			QString::number(qty) + tr("|") + QString::number(cyclePoint) + tr("|") + QString::number(cycleNum);
		if(row == 0)
			szSaveData = szLine;
		else
			szSaveData = szSaveData + QString(tr(",")) + szLine;
	}
	return szSaveData;
}

int USTPUnilateralWidget::getPrcision(const double& value)
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


#include "moc_USTPUnilateralWidget.cpp"