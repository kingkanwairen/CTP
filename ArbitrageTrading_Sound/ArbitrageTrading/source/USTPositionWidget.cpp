#include "USTPositionWidget.h"
#include <QtGui/QMessageBox>
#include "USTPCtpLoader.h"
#include "USTPConfig.h"
#include "USTPMutexId.h"

USTPositionWidget::USTPositionWidget(QWidget* parent)
:QTreeWidget(parent)
{	
	mHeaders << POSITION_WIDGET_TAB_HEAD_0 << POSITION_WIDGET_TAB_HEAD_1 << POSITION_WIDGET_TAB_HEAD_2;
	USTPMutexId::getTotalBidPosition(mBidPositions);
	USTPMutexId::getTotalAskPosition(mAskPositions);
	setHeaderLabels(mHeaders);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setAlternatingRowColors(true);
	setRootIsDecorated(false);
	setSortingEnabled(true);
	setFont(QFont("Times", 10, QFont::DemiBold));
	initConnect();
	initPositionShow();
	setMinimumWidth(POSITION_WINDOW_MIN_WIDTH);
}

USTPositionWidget::~USTPositionWidget()
{

}


void USTPositionWidget::doUSTPRtnTrade(const QString& tradeId, const QString& instrumentId, const char& direction, const int& tradeVolume, const double& tradePrice,
									   const char& offsetFlag, const char& hedgeFlag, const QString& brokerId, const QString& exchangeId, const QString& investorId, const QString& orderSysId,
									   const QString& localId, const QString& orderRef, const QString& tradeTime)
{	
	if(direction == '0'){
		if(offsetFlag == '0'){
			if(mBidPositions.find(instrumentId) != mBidPositions.end()){
				mBidPositions[instrumentId] += tradeVolume;
			}else{
				mBidPositions.insert(instrumentId, tradeVolume);
			}
		}else{
			if(mAskPositions.find(instrumentId) != mAskPositions.end()){
				mAskPositions[instrumentId] -= tradeVolume;
				if(mAskPositions[instrumentId] < 0)
					mAskPositions[instrumentId] = 0;
			}
		}
	}else{
		if(offsetFlag == '0'){
			if(mAskPositions.find(instrumentId) != mAskPositions.end()){
				mAskPositions[instrumentId] += tradeVolume;
			}else{
				mAskPositions.insert(instrumentId, tradeVolume);
			}
		}else{
			if(mBidPositions.find(instrumentId) != mBidPositions.end()){
				mBidPositions[instrumentId] -= tradeVolume;
				if(mBidPositions[instrumentId] < 0)
					mBidPositions[instrumentId] = 0;
			}
		}
	}
	updatePositionShow(instrumentId);
}

void USTPositionWidget::initConnect()
{
	connect(USTPCtpLoader::getTradeSpi(), SIGNAL(onUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)),
		this, SLOT(doUSTPRtnTrade(const QString&, const QString&, const char&, const int&, const double&,
		const char&, const char&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&, const QString&)));
}

void USTPositionWidget::initPositionShow()
{
	QList<QString> bidkeys = mBidPositions.keys();
	QList<QString> askkeys = mAskPositions.keys();
	foreach(QString ins, bidkeys){
		updatePositionShow(ins);
	}
	foreach(QString ins, askkeys){
		updatePositionShow(ins);
	}
}

void USTPositionWidget::updatePositionShow(const QString& ins)
{
	QList<QTreeWidgetItem*> itemList = findItems(ins, Qt::MatchRecursive);
	if (itemList.size() == 0){
		QTreeWidgetItem *showItem = new QTreeWidgetItem(this);
		showItem->setText(0, ins);
		if(mBidPositions.find(ins) != mBidPositions.end())
			showItem->setText(1, QString::number(mBidPositions[ins]));
		else
			showItem->setText(1, QString::number(0));

		if (mAskPositions.find(ins) != mAskPositions.end())
			showItem->setText(2, QString::number(mAskPositions[ins]));
		else
			showItem->setText(2, QString::number(0));
		insertTopLevelItem(0, showItem);
	}else{
		QTreeWidgetItem* showItem = itemList.at(0);
		showItem->setText(0, ins);
		if(mBidPositions.find(ins) != mBidPositions.end())
			showItem->setText(1, QString::number(mBidPositions[ins]));
		else
			showItem->setText(1, QString::number(0));

		if (mAskPositions.find(ins) != mAskPositions.end())
			showItem->setText(2, QString::number(mAskPositions[ins]));
		else
			showItem->setText(2, QString::number(0));
	}
}

#include "moc_USTPositionWidget.cpp"