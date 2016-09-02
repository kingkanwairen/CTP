#include "USTPOrderWidget.h"
#include <QtGui/QHeaderView>
#include "USTPConfig.h"
#include "USTPCtpLoader.h"

USTPOrderWidget::USTPOrderWidget(QWidget* parent)
:QTableWidget(parent)
{	
	setColumnCount(ORDER_HEAD_LENGTH);
	mHeaders << ORDER_WIDGET_TAB_HEAD_0 << ORDER_WIDGET_TAB_HEAD_1 << ORDER_WIDGET_TAB_HEAD_2
		<< ORDER_WIDGET_TAB_HEAD_3 << ORDER_WIDGET_TAB_HEAD_4 << ORDER_WIDGET_TAB_HEAD_5
		<< ORDER_WIDGET_TAB_HEAD_6 << ORDER_WIDGET_TAB_HEAD_7 << ORDER_WIDGET_TAB_HEAD_8
		<< ORDER_WIDGET_TAB_HEAD_9<< ORDER_WIDGET_TAB_HEAD_10 << ORDER_WIDGET_TAB_HEAD_11;
	setHorizontalHeaderLabels(mHeaders);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setAlternatingRowColors(true);
	setColumnHidden(0, true);
	setEditTriggers(QAbstractItemView::NoEditTriggers); 
	horizontalHeader()->setMovable(true);
	setShowGrid(true);
}

USTPOrderWidget::~USTPOrderWidget()
{

}


QString USTPOrderWidget::getBSText(const char& bs)
{	
	QString rtnBS = "--";
	if (bs == '0'){
		rtnBS = QString(tr("��"));
	}else{
		rtnBS = QString(tr("��"));
	}
	return rtnBS;
}

QString USTPOrderWidget::getPriceTypeText(const char& priceType)
{
	QString rtnType = "--";
	if (priceType == '1'){
		rtnType = QString(tr("�м�"));
	}else if (priceType == '2'){
		rtnType = QString(tr("�޼�"));
	}
	return rtnType;
}

QString USTPOrderWidget::getOffsetFlagText(const char& offsetFlag)
{
	QString rtnOffset = "--";
	if (offsetFlag == '0'){
		rtnOffset = QString(tr("����"));
	}else if (offsetFlag == '1'){
		rtnOffset = QString(tr("ƽ��"));
	}else if (offsetFlag == '2'){
		rtnOffset = QString(tr("ǿƽ"));
	}else if (offsetFlag == '3'){
		rtnOffset = QString(tr("ƽ��"));
	}else if (offsetFlag == '4'){
		rtnOffset = QString(tr("ƽ��"));
	}
	return rtnOffset;
}

QString USTPOrderWidget::getHedgeFlagText(const char& hedgeFlag)
{
	QString rtnHedge = "--";
	if (hedgeFlag == '1'){
		rtnHedge = QString(tr("Ͷ��"));
	}else if (hedgeFlag == '2'){
		rtnHedge = QString(tr("����"));
	}else if (hedgeFlag == '3'){
		rtnHedge = QString(tr("�ױ�"));
	}
	return rtnHedge;
}

QString USTPOrderWidget::getStatusText(const char& status)
{
	QString rtnStatus = "--";
	if (status == '0'){
		rtnStatus = QString(tr("ȫ���ɽ�"));
	}else if (status == '1'){
		rtnStatus = QString(tr("���ֳɽ�������"));
	}else if (status == '2'){
		rtnStatus = QString(tr("���ֳɽ����ڶ���"));
	}else if (status == '3'){
		rtnStatus = QString(tr("δ�ɽ�������"));
	}else if (status == '4'){
		rtnStatus = QString(tr("δ�ɽ����ڶ���"));
	}else if (status == '5'){
		rtnStatus = QString(tr("����"));
	}else if (status == 'a'){
		rtnStatus = QString(tr("δ֪"));
	}else if (status == 'b'){
		rtnStatus = QString(tr("��δ����"));
	}else if (status == 'c'){
		rtnStatus = QString(tr("�Ѵ���"));
	}else if (status == 'D'){
		rtnStatus = QString(tr("�ظ��ı���"));
	}else if (status == 'N'){
		rtnStatus = QString(tr("δ����"));
	}else if (status == 'P'){
		rtnStatus = QString(tr("ƽ��λ����"));
	}else if (status == 'S'){
		rtnStatus = QString(tr("������δȷ��"));
	}else if (status == 'W'){
		rtnStatus = QString(tr("¼�����"));
	}else if (status == 'Z'){
		rtnStatus = QString(tr("�ʽ���"));
	}
	return rtnStatus;
}


void USTPOrderWidget::doUpdateOrderShow(const QString& orderLabel, const QString& requestId, const QString& instrumentId, const char& status, const char& bs, const int&
					   orderVolume, const int& remainVolume, const int& tradeVolume, const char& offsetFlag, const char& priceType, const char& hedge, const double& orderPrice)
{	
	QList<QTableWidgetItem*> itemList = findItems(requestId, Qt::MatchRecursive);
	if (itemList.size() == 0){
		if(status == THOST_FTDC_OST_AllTraded || status == THOST_FTDC_OST_Canceled)
			return;
		setSortingEnabled(false);
		int nRow = rowCount();	
		insertRow(nRow);
		addItem(nRow, 0, requestId);
		addItem(nRow, 1, instrumentId);
		addItem(nRow, 2, getOffsetFlagText(offsetFlag));
		addItem(nRow, 3, orderLabel);
		addItem(nRow, 4, getStatusText(status));
		addItem(nRow, 5, getBSText(bs));
		addItem(nRow, 6, QString::number(orderPrice));
		addItem(nRow, 7, QString::number(orderVolume));
		addItem(nRow, 8, QString::number(remainVolume));
		addItem(nRow, 9, QString::number(tradeVolume));	
		addItem(nRow, 10, getPriceTypeText(priceType));
		addItem(nRow, 11, getHedgeFlagText(hedge));
		setSortingEnabled(true);
		sortItems(0, Qt::DescendingOrder);
	}
	else{
		QTableWidgetItem* it = itemList.at(0);
		int nSelRow = it->row();
		int nColumeLen = columnCount();
		if(status == THOST_FTDC_OST_AllTraded || status == THOST_FTDC_OST_Canceled){
			for(int nIndex = 0; nIndex < nColumeLen; nIndex++){
				QTableWidgetItem* showItem = takeItem(nSelRow, nIndex);
				delete showItem;
				showItem = NULL;
			}
			removeRow(nSelRow);
		}else{
			item(nSelRow, 4)->setText(getStatusText(status));
			item(nSelRow, 6)->setText(QString::number(orderPrice));
			item(nSelRow, 8)->setText(QString::number(remainVolume));
			item(nSelRow, 9)->setText(QString::number(tradeVolume));
		}
	}
}

void USTPOrderWidget::addItem(const int& row, const int& colume, const QString& text)
{
	QTableWidgetItem* item = new QTableWidgetItem(text);
	setItem(row, colume, item);
}


#include "moc_USTPOrderWidget.cpp"