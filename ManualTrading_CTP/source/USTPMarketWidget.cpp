#include "USTPMarketWidget.h"
#include <QtGui/QHeaderView>
#include "USTPMutexId.h"
#include "USTPConfig.h"
#include "USTPMdApi.h"
#include "USTPEnbedDialog.h"

USTPMarketWidget::USTPMarketWidget(QWidget* parent)
:QTableWidget(parent)
{	
	setColumnCount(SPEC_DEPTH_HEAD_LENGTH);
	mHeaders << SPEC_DEPTH_WIDGET_TAB_HEAD_0 << SPEC_DEPTH_WIDGET_TAB_HEAD_1 << SPEC_DEPTH_WIDGET_TAB_HEAD_2 << SPEC_DEPTH_WIDGET_TAB_HEAD_3
		<< SPEC_DEPTH_WIDGET_TAB_HEAD_4 << SPEC_DEPTH_WIDGET_TAB_HEAD_5 << SPEC_DEPTH_WIDGET_TAB_HEAD_6 << SPEC_DEPTH_WIDGET_TAB_HEAD_7 << 
		SPEC_DEPTH_WIDGET_TAB_HEAD_8<< SPEC_DEPTH_WIDGET_TAB_HEAD_9<< SPEC_DEPTH_WIDGET_TAB_HEAD_10;
	setHorizontalHeaderLabels(mHeaders);
	setSelectionBehavior(QAbstractItemView::SelectItems);
	verticalHeader()->setVisible(false);
	verticalHeader()->setDefaultSectionSize(22);
	horizontalHeader()->setMovable(true);
	horizontalHeader()->setHighlightSections(false); 
	horizontalHeader()->setFont(QFont("Times", 11, QFont::Bold));

	setStyleSheet(  
		"QTableWidget {"  
		"background-color: rgb(20, 20, 20) ;"  
		"}"  
		"QHeaderView::section {"  
		"color: white;"  
		"}"  
		"QHeaderView::section {"  
		"background-color: rgb(20, 20, 20);" 
		"}"  
		);  

	setFont(QFont("Times", 11, QFont::DemiBold));
	setEditTriggers(QAbstractItemView::NoEditTriggers); 
	setShowGrid(true);
	initConnect();
	setMinimumHeight(MARKET_WINDOW_MIN_HEIGHT);
}

USTPMarketWidget::~USTPMarketWidget()
{

}

void USTPMarketWidget::initConnect()
{
	connect(this, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(doSelectItem(QTableWidgetItem*)));
	connect(this, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(doCellDoubleClicked(int, int)));
}

void USTPMarketWidget::doSubscribeMd(const QStringList& inss)
{	
	foreach (USTPComplexMd* widget, mUSTPSpeMdMap){
		delete widget;
		widget = NULL;
	}
	mUSTPSpeMdMap.clear();

	int nRowLen = rowCount();
	int nColumeLen = columnCount();
	for(int row = 0; row < nRowLen; row++){
		for(int colume = 0; colume < nColumeLen; colume++){
			QTableWidgetItem *rItem = takeItem(row, colume);
			delete rItem;
			rItem = NULL;
		}
	}
	setRowCount(0);
	clearContents();

	foreach(QString ins, inss){
		doUSTPSpeComplexMd(ins, "0.0", "0", "0.0", "0", "0", "0.0", "0.0", "0.0", "0.0", "0");
		USTPComplexMd* wdg = new USTPComplexMd(ins, this);
		mUSTPSpeMdMap.insert(ins, wdg);
		USTPMdApi::subMarketData(ins);
	}

}

void USTPMarketWidget::doUSTPSpeComplexMd(const QString& selIns, const QString& bidPrice, const QString& bidVolume, const QString& askPrice, const QString& askVolume,
										  const QString& tradeVolume, const QString& lastPrice, const QString& upDownPrice, const QString& preClosePrice, const QString& openPrice, const QString& holdQty)

{
	QList<QTableWidgetItem*> itemList = findItems(selIns, Qt::MatchRecursive);
	if (itemList.size() == 0){
		int nRow = rowCount();
		insertRow(nRow);
		mPriceMap.insert(nRow, upDownPrice.toFloat());
		addItem(nRow, 0, selIns);
		addItem(nRow, 1, preClosePrice);	
		addItem(nRow, 2, openPrice);	
		addItem(nRow, 3, lastPrice);	
		addItem(nRow, 4, bidPrice);
		addItem(nRow, 5, bidVolume);	
		addItem(nRow, 6, askPrice);
		addItem(nRow, 7, askVolume);	
		addItem(nRow, 8, tradeVolume);
		addItem(nRow, 9, upDownPrice);
		addItem(nRow, 10, holdQty);
	}
	else{
		QTableWidgetItem* it = itemList.at(0);
		int nSelRow = it->row();
		mPriceMap[nSelRow] = upDownPrice.toFloat();
		updateItemPrice(nSelRow, 1, preClosePrice);	
		updateItemPrice(nSelRow, 2, openPrice);	
		updateItemPrice(nSelRow, 3, lastPrice);	
		updateItemPrice(nSelRow, 4, bidPrice);
		updateItemVolume(nSelRow, 5, bidVolume);
		updateItemPrice(nSelRow, 6, askPrice);
		updateItemVolume(nSelRow, 7, askVolume);	
		updateItemVolume(nSelRow, 8, tradeVolume);
		updateItemPrice(nSelRow, 9, upDownPrice);
		updateItemVolume(nSelRow, 10, holdQty);
	}
}

void USTPMarketWidget::updateItemPrice(const int& row, const int& colume, const QString& price)
{
	QString szOldPrice = item(row, colume)->text();
	if(mPriceMap[row] > 0.0){
		item(row, colume)->setTextColor(QColor(230, 0, 0)); 
	}else{
		item(row, colume)->setTextColor(QColor(0, 230, 0)); 
	}
	item(row, colume)->setText(price);
}

void USTPMarketWidget::updateItemVolume(const int& row, const int& colume, const QString& volume)
{
	QString szOldVolume = item(row, colume)->text();
	if(mPriceMap[row] > 0.0){
		item(row, colume)->setTextColor(QColor(230, 0, 0)); 
	}else{
		item(row, colume)->setTextColor(QColor(0, 230, 0)); 
	}
	item(row, colume)->setText(volume);
}

void USTPMarketWidget::addItem(const int& row, const int& colume, const QString& text)
{
	QTableWidgetItem* item = new QTableWidgetItem(text);
	item->setTextColor(QColor(250, 250, 250)); 
	setItem(row, colume, item);
}

void USTPMarketWidget::doGetSpeInsPrice(const QString& selIns, const int& direction)
{
	emit onGetSpeDepthMd(selIns, direction);
}

void USTPMarketWidget::doUpdateSpeDepthMd(const QString& selIns, const int& direction, const double& orderPrice)
{
	emit onUpdateSpePrice(selIns, direction, orderPrice);
}


void USTPMarketWidget::doSelectItem(QTableWidgetItem* it)
{
	int nSelRow = it->row();
	int nSelColume = it->column();
	QString selItem = item(nSelRow, 0)->text();
	if(nSelColume == 4 || nSelColume == 5){
		QString selPrice = item(nSelRow, 4)->text();
		emit onClickSelIns(selItem, 1, selPrice.toDouble());
	}else if(nSelColume == 6 || nSelColume == 7){
		QString selPrice = item(nSelRow, 6)->text();
		emit onClickSelIns(selItem, 0, selPrice.toDouble());
	}else{
		emit onClickSelIns(selItem, -1, 0.0);
	}
}

void USTPMarketWidget::doCellDoubleClicked(int row, int column)
{	
	if(column != 4 && column != 6)
		return;
	QString firstIns = item(row, 0)->text();
	QString szPrice = item(row, column)->text();
	double price = szPrice.toDouble();
	int bs = 0;
	if(column == 6)
		bs = 1;
	USTPEnbedDialog dlg(row, firstIns, price, bs, this);
	dlg.exec();
}

void USTPMarketWidget::doCreateEnbedOrder(const QString& selIns, const QString& direction, const QString& offsetFlag, const double& priceTick, const int& orderQty,
										  const int& openSuperPrice, const int& stopProfitNum, const int& stopLossNum, const int& row)
{	
	QString szLastPrice = item(row, 3)->text();
	QString szAskPrice = item(row, 4)->text();
	QString szBidPrice = item(row, 6)->text();
	double lastPrice = szLastPrice.toDouble();
	double askPrice = szAskPrice.toDouble();
	double bidPrice = szBidPrice.toDouble();
	if(lastPrice < VALUE_LIMIT || askPrice < VALUE_LIMIT || bidPrice < VALUE_LIMIT)
		return;
	emit onEnbedOrder(selIns, direction, offsetFlag, priceTick, orderQty, openSuperPrice, stopProfitNum, stopLossNum, bidPrice, askPrice, lastPrice);
}

#include "moc_USTPMarketWidget.cpp"