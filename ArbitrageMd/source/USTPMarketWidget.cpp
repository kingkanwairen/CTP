#include "USTPMarketWidget.h"
#include "USTPMdApi.h"
#include "USTPConfig.h"
#include "USTPCtpLoader.h"
#include <QtGui/QHeaderView>
#include <QtCore/QTime>
#include <QtGui/QHeaderView>
#include <Windows.h>

USTPMarketWidget::USTPMarketWidget( QWidget* parent)
:QTableWidget(parent)
{	
	setColumnCount(DEPTH_HEAD_LENGTH);
	mHeaders << DEPTH_WIDGET_TAB_HEAD_0 << DEPTH_WIDGET_TAB_HEAD_1 << DEPTH_WIDGET_TAB_HEAD_2 << DEPTH_WIDGET_TAB_HEAD_3
		<< DEPTH_WIDGET_TAB_HEAD_4 << DEPTH_WIDGET_TAB_HEAD_5 << DEPTH_WIDGET_TAB_HEAD_6 << DEPTH_WIDGET_TAB_HEAD_7 
		<< DEPTH_WIDGET_TAB_HEAD_8 << DEPTH_WIDGET_TAB_HEAD_9 << DEPTH_WIDGET_TAB_HEAD_10;
	setHorizontalHeaderLabels(mHeaders);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	verticalHeader()->setVisible(false);
	horizontalHeader()->setMovable(true);
	horizontalHeader()->setHighlightSections(false); 
	horizontalHeader()->setFont(QFont("Times", 12, QFont::Bold));
	setStyleSheet(  
		"QTableWidget {"  
		"background-color: rgb(20, 20, 20) ;"  
		"}"  
		"QHeaderView::section {"  
		"color: white;"  
		"}"  
		"QHeaderView::section {"  
		"background-color: rgb(80, 80, 6);" 
		"}"  
		);  
	
	setFont(QFont("Times", 12, QFont::DemiBold));
	setColumnHidden(0, true);
	setEditTriggers(QAbstractItemView::NoEditTriggers); 
	setShowGrid(true);
	setMinimumWidth(MARKET_WINDOW_MIN_WIDTH);
	initConnect();
	for(int nIndex = 0; nIndex < DEPTH_HEAD_LENGTH; nIndex++){
		setColumnWidth(nIndex, 100);
	}
}

USTPMarketWidget::~USTPMarketWidget()
{

}

void USTPMarketWidget::initConnect()
{
	connect(USTPCtpLoader::getMdSpi(), SIGNAL(onUSTPMdFrontConnected()), this, SLOT(doUSTPMdFrontConnected()));
}

void USTPMarketWidget::doUSTPMdFrontConnected()
{
	mInss.clear();
}

void USTPMarketWidget::doSubscribeMd(const QStringList& inss)
{	
	foreach (USTPComplexMd* widget, mUSTPMdMap){
		delete widget;
		widget = NULL;
	}
	mUSTPMdMap.clear();

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
	//½âÎöÊý¾Ý
	foreach(QString ins, inss){
		QStringList items = ins.split("+");
		if (items.size() == 3){
			QString oneInsGroup = items.at(0);
			QString secInsGroup = items.at(1);
			QString constantGroup = items.at(1);

			QStringList onsInsItems = oneInsGroup.split("*");
			QString szOneInsFactor = onsInsItems.at(0);
			double fOneInsFactor = szOneInsFactor.toDouble();
			QString szOneIns = onsInsItems.at(1);

			QStringList secInsItems = secInsGroup.split("*");
			QString szSecInsFactor = secInsItems.at(0);
			QString szSecIns = secInsItems.at(1);
			
			QString szSecInsAdjustFactor;
		
			if(szSecInsFactor.startsWith("("))
				szSecInsAdjustFactor = szSecInsFactor.mid(1, szSecInsFactor.size() - 2);
			else
				szSecInsAdjustFactor = szSecInsFactor;

			double fSecInsFactor = szSecInsAdjustFactor.toDouble();
			
			QString szConstValue;
			if(constantGroup.startsWith("("))
				szConstValue = constantGroup.mid(1, constantGroup.size() - 2);
			else
				szConstValue = constantGroup;

			double fConstValue = szConstValue.toDouble();
			
			USTPComplexMd* wdg = new USTPComplexMd(szOneIns, szSecIns, fOneInsFactor, fSecInsFactor, fConstValue, this);
			mUSTPMdMap.insert(ins, wdg);
			QString key = szOneIns + tr("|") + szSecIns;
			doUSTPComplexMd(key, szOneIns, szSecIns, "0.0", "0.0", "0.0", "0.0", "0", "0", "0", "0");
			QMap<QString, QString>::const_iterator iter = mInss.find(szOneIns);
			if(iter == mInss.end()){
				USTPMdApi::subMarketData(szOneIns);
				mInss.insert(szOneIns, szOneIns);
			}
			iter = mInss.find(szSecIns);
			if(iter == mInss.end()){
				USTPMdApi::subMarketData(szSecIns);
				mInss.insert(szSecIns, szSecIns);
			}
		}
	}

}

void USTPMarketWidget::doUSTPComplexMd(const QString& key, const QString& firstIns, const QString& secIns, const QString& bbPrice, const QString& bsPrice, const QString& sbPrice,
									   const QString& ssPrice, const QString& bbQty, const QString& bsQty, const QString& sbQty, const QString& ssQty)
{
	QList<QTableWidgetItem*> itemList = findItems(key, Qt::MatchRecursive);
	if (itemList.size() == 0){
		int nRow = rowCount();
		insertRow(nRow);		
		addItem(nRow, 0, key);
		addItem(nRow, 1, firstIns);
		addItem(nRow, 2, secIns);
		addItem(nRow, 3, bbPrice);
		addItem(nRow, 4, bsPrice);
		addItem(nRow, 5, sbPrice);
		addItem(nRow, 6, ssPrice);
		addItem(nRow, 7, bbQty);
		addItem(nRow, 8, bsQty);
		addItem(nRow, 9, sbQty);
		addItem(nRow, 10, ssQty);
	}
	else{
		QTableWidgetItem* it = itemList.at(0);
		int nSelRow = it->row();
		item(nSelRow, 1)->setText(firstIns);
		item(nSelRow, 2)->setText(secIns);

		updateItemPrice(nSelRow, 3, bbPrice);
		updateItemPrice(nSelRow, 4, bsPrice);
		updateItemPrice(nSelRow, 5, sbPrice);
		updateItemPrice(nSelRow, 6, ssPrice);

		updateItemVolume(nSelRow, 7, bbQty);
		updateItemVolume(nSelRow, 8, bsQty);
		updateItemVolume(nSelRow, 9, sbQty);
		updateItemVolume(nSelRow, 10, ssQty);
	}
}

void USTPMarketWidget::updateItemPrice(const int& row, const int& colume, const QString& price)
{
	QString szOldPrice = item(row, colume)->text();
	double oldPrice = szOldPrice.toDouble();
	double newPrice = price.toDouble();
	if(newPrice >= oldPrice){
		item(row, colume)->setTextColor(QColor(230, 0, 0)); 
	}else if(newPrice < oldPrice){
		item(row, colume)->setTextColor(QColor(0, 230, 0)); 
	}
	item(row, colume)->setText(price);
}

void USTPMarketWidget::updateItemVolume(const int& row, const int& colume, const QString& volume)
{
	QString szOldVolume = item(row, colume)->text();
	int oldVolume = szOldVolume.toInt();
	int newVolume = volume.toInt();
	if(newVolume >= oldVolume){
		item(row, colume)->setTextColor(QColor(230, 0, 0)); 
	}else if(newVolume < oldVolume){
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

#include "moc_USTPMarketWidget.cpp"